/* Copyright (c) 2023, 2025 Percona LLC and/or its affiliates. All rights
   reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; version 2 of
   the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

/*
  This and corresponding header file contain classes and routines which are
  the central part of our implementation of support for JS routines.

  Note about error handling and Out-Of-Memory conditions
  ------------------------------------------------------

  In general case, this component tries to gracefully handle potential errors
  from calls to services provided by SQL core and V8 engine.
  However, in some trivial cases, when we know that service/V8 call can't fail
  in practice (e.g. because it is simple accessor to members of some object)
  we have chosen to ignore instead, even though it theoretically possible
  according to API. This allows to avoid clobbering our code with dead error
  handling branches.

  Code in SQL core is not very consistent regarding OOM handling. In some
  places it tries to catch std::bad_alloc/to handle NULL return value from
  malloc() and in some other places it does not. Failures to insert data
  into container/to append to strings are also often ignored.
  And V8 simply crashes process on OOM in most cases.

  Taking into account the above this component doesn't try to handle OOM
  gracefully either (i.e. will crash on OOM).
*/

#include "js_lang_core.h"

#include <sstream>

#include <scope_guard.h>

#include "js_lang_common.h"

std::unique_ptr<v8::Platform> Js_v8::s_platform;

std::atomic<int> Js_v8::s_ref_count = 0;

void Js_v8::init() {
  // We would like to enforce strict mode for code of JS routines right
  // from the start.
  v8::V8::SetFlagsFromString("--use_strict --no-concurrent_sparkplug");

  // v8::V8::InitializeICUDefaultLocation(argv[0]);
  // v8::V8::InitializeExternalStartupData(argv[0]);
  s_platform = v8::platform::NewDefaultPlatform();
  v8::V8::InitializePlatform(s_platform.get());
  // The below call will simply crash the process if something is wrong.
  (void)v8::V8::Initialize();
}

void Js_v8::shutdown() {
  // We should have checked that there are no used isolates around earlier.
  assert(s_ref_count.load() == 0);

  // Set flag indicating that component shutdown has been started for
  // debug/assertion purposes.
  //
  // We also piggy-back on this flag to block attempts to install this
  // component without server restart again. It is not possible due to
  // V8 not supporting re-initialization.
  s_ref_count.store(-1);

  // Shutdown V8.
  v8::V8::Dispose();
  v8::V8::DisposePlatform();
  s_platform.reset();
}

bool Js_isolate::Memory_manager::check_mem_limit_at_arr_buff_alloc(
    size_t length) {
  /*
    raise_max_mem_size_exceeded() code assumes that it can be only called
    from the same thread as executes JS (i.e. connection thread) and not
    from some background thread. It needs to be adjusted to use atomic flag
    if this assumption is ever broken.
  */
  check_thd();

  /*
    In theory, we could do a better check taking into account current heap
    size, similar to one in check_mem_limit_at_GC(), here instead.

    However:
    1) Technically, we are not supposed to call back V8 from allocator
       methods (though both PLv8 and isolated-vm projects seem to do this
       just fine). Calling TerminateExecution() is probably safe.
    2) Code gets akwardly complicated if we try to do this nicely:
       We probably want to avoid calling v8::Isolate::GetHeapStatistics()
       on each allocation. We also either need to avoid
       v8::Isolate::LowMemoryNotification() in this case, or handle
       situation when check_mem_limit_at_arr_buff_alloc() causes immediate GC,
       which calls check_mem_limit_at_GC() in its turn (so we have sort of
       recursion).

    So, instead, we simply check that memory allocated for ArrayBuffer's
    doesn't exceed the limit, and let the check at GC time to detect
    situation when sum of memory allocated for ArrayBuffer's and on V8
    heap exceeds the limit. What can happen in the worst case is that we
    will exceed the memory limit until GC time, but this can happen anyway
    in our implementation.

    Note that m_arr_buff_allocated value can be updated concurrently by
    Free() call invoked from within background thread. This makes the
    check a bit racy, but it still should be OK.
  */
  if (m_arr_buff_allocated + length > m_max_mem_size) {
    m_js_isolate->raise_max_mem_size_exceeded();
    return true;
  }
  return false;
}

void Js_isolate::Memory_manager::check_mem_limit_at_GC() {
  /*
    The below code assumes that it can be only called from the same thread
    as executes JS (i.e. connection thread) and not from some background
    thread. It needs to be adjusted if this assumption is ever broken
    (GetHeapStatistics() and TerminateExecution() are thread-safe, but
    LowMemoryNotification() is likely not and neither reads/updates to
    our Mem_limit_manager members).
  */
  check_thd();

  v8::HeapStatistics heap_stats;
  m_js_isolate->m_isolate->GetHeapStatistics(&heap_stats);

  // Piggy-back on GC callback to update global memory usage counters.
  update_global_mem_stats(heap_stats);

  const size_t total_size = heap_stats.used_heap_size() + m_arr_buff_allocated;

  if (total_size > m_max_mem_size) {
    m_js_isolate->raise_max_mem_size_exceeded();
  } else {
    /*
      Check if we have approached dangerously close to the memory limit
      and ask V8 to do free up some memory if yes. Since this will
      trigger major GC which is expensive we try to avoid doing this
      if it was done during one of the previous callback invokations
      and we didn't return back to normality yet.

      TODO: In future we might consider to use memory pressure API instead.
            It is thread-safe and allows to trigger GCs in a more gradual
            fashion (first moderate pressure, then critical).
    */
    const size_t danger_zone_mem_size = m_max_mem_size * 0.9;
    if ((total_size > danger_zone_mem_size) &&
        (m_last_danger_zone_check_mem_size <= danger_zone_mem_size)) {
      m_js_isolate->m_isolate->LowMemoryNotification();
    }
  }
  m_last_danger_zone_check_mem_size = total_size;
}

/**
  Maximum size of buffer for Typed Array for which V8 applies special
  optimization and allocates buffer on V8 heap at its creation time
  (see JSTypedArray::kMaxSizeInHeap in V8 sources).
*/
constexpr size_t TYPED_ARRAY_IN_HEAP_MAX_SIZE = 64;

void *Js_isolate::Memory_manager::Allocate(size_t length) {
  /*
    It is unsafe to return nullptr from Allocate() in two cases:

    1) If we were asked to allocate memory for tiny array buffer.

       V8 optimizes tiny Typed Arrays by allocating their buffer on V8
       heap instead of calling allocator. However, when someone requests
       pointer to underlying buffer for such an array, V8 switches it from
       on heap buffer to allocated buffer. Allocator is called and failure
       to allocate the buffer in such a scenario results in V8 OOM/fatal
       abort (see JSTypedArray::GetBuffer() in V8 sources).

       Hence, we do not return nullptr for tiny allocation requests. This
       should be fine as we still mark isolate as exceeding memory limit
       and request its termination in this case.

    2) v8::ArrayBuffer::New() which is used by our code handling parameter
       values doesn't handle nullptr gracefully either and results in V8
       OOM/fatal abort in this case as well.

       We are working around this case by ensuring that we never invoke
       v8::ArrayBuffer::New() when limit could be exceeded. This is done
       by simply checking that we have enough space before invoking this
       method. Of course it is ugly, but probably better than allowing
       to allocate arbitrary large array buffers.

    For other allocation requests it is OK to return nullptr as V8 should
    handle such situations gracefully.
  */
  if (check_mem_limit_at_arr_buff_alloc(length) &&
      length > TYPED_ARRAY_IN_HEAP_MAX_SIZE)
    return nullptr;

  m_arr_buff_allocated += length;
  return m_arr_buff_allocator->Allocate(length);
}

void *Js_isolate::Memory_manager::AllocateUninitialized(size_t length) {
  // See comment in Allocate() for explanation why we can't return nullptr
  // for tiny allocations.
  if (check_mem_limit_at_arr_buff_alloc(length) &&
      length > TYPED_ARRAY_IN_HEAP_MAX_SIZE)
    return nullptr;

  m_arr_buff_allocated += length;
  return m_arr_buff_allocator->AllocateUninitialized(length);
}

void Js_isolate::Memory_manager::Free(void *data, size_t length) {
  // Note that this method can be called from a background thread doing
  // GC sweeping, hence we have to use atomic for m_arr_buff_allocated.
  m_arr_buff_allocated -= length;
  m_arr_buff_allocator->Free(data, length);
}

void *Js_isolate::Memory_manager::Reallocate(void *data, size_t old_length,
                                             size_t new_length) {
  ssize_t length_diff =
      static_cast<ssize_t>(new_length) - static_cast<ssize_t>(old_length);

  if (length_diff > 0 && check_mem_limit_at_arr_buff_alloc(length_diff))
    return nullptr;

  m_arr_buff_allocated += length_diff;
  return m_arr_buff_allocator->Reallocate(data, old_length, new_length);
}

Js_isolate *Js_isolate::create() {
  v8::Isolate::CreateParams create_params;

  // Increment global Isolate reference counter to block component
  // shutdown while we creating Isolate and as long as it is around.
  //
  // Component infrastructure ensures that methods of language service and
  // thus this code can't be called after component shutdown starts.
  Js_v8::inc_ref_count();

  // We create Js_isolate before creation of V8 Isolate object as the
  // latter need to be passed reference to Js_isolate::Memory_manager.
  Js_isolate *result = new Js_isolate();

  // Tell V8 to use our custom wrapper around the standard V8 ArrayBuffer
  // allocator, which does memory limit enforcement.
  create_params.array_buffer_allocator = &(result->m_memory_manager);
  /*
    V8 is bad at handling exceeding heap limit gracefully, instead it is
    likely to abort the whole process in this case.

    Handling OOMs/exceeding memory limits gracefully is simply not among V8's
    design goals. Their primary customer is Chrome, which runs JS in separate
    isolated processes, for which "let us crash on reaching the limit"
    behavior doesn't cause problems.

    By default, we try to avoid such aborts and do not set V8 memory limit
    explicitly, relying on its generous default (typically > 1Gb).
    We only do best-effort attempt to respect max_mem_size memory limit by
    checking if it is exceeded periodically (at GC time).

    Users can set smaller V8 limit by setting max_mem_size_hard_limit_factor
    to non-0 value if they are willing to risk server aborts or if such aborts
    are preferrable to exceeding memory limits.

    TODO: Consider having a hardened mode for JS execution in which it will
          be run in separate worker process (like it is done in Chrome).
  */

  if (Js_isolate::Memory_manager::s_max_mem_size_hard_limit_factor > 0) {
    create_params.constraints.ConfigureDefaultsFromHeapSize(
        0, result->m_memory_manager.m_max_mem_size *
               Js_isolate::Memory_manager::s_max_mem_size_hard_limit_factor);
  }

  v8::Isolate *isolate = v8::Isolate::New(create_params);

  result->m_isolate = isolate;

  // Install callback to be called at the end of GC that checks if
  // memory limit has been exceeded, and terminates isolate if yes.
  auto gc_epilogue_cb_lambda = [](v8::Isolate *, v8::GCType type,
                                  v8::GCCallbackFlags, void *data) -> void {
    // V8 documentation says that callback won't be called again if it
    // triggers GC, so we don't have to care about recursion.
    //
    // Do the check only during normal minor and major GC.
    if (type == v8::GCType::kGCTypeScavenge ||
        type == v8::GCType::kGCTypeMarkSweepCompact) {
      Js_isolate *js_isolate = static_cast<Js_isolate *>(data);
      js_isolate->m_memory_manager.check_mem_limit_at_GC();
    }
    return;
  };
  isolate->AddGCEpilogueCallback(gc_epilogue_cb_lambda, result);

  // If we have approached V8 heap limit (which is max_mem_size * X times),
  // probably it is too late already! (Perhaps we are allocating too fast?).
  //
  // Still let us install callback to be called in this case to give  a chance
  // to a graceful termination.
  auto near_heap_limit_cb_lambda = [](void *data, size_t current_heap_limit,
                                      size_t) -> size_t {
    Js_isolate *js_isolate = static_cast<Js_isolate *>(data);
    js_isolate->raise_max_mem_size_exceeded();
    // Increase V8 heap limit temporarily to give a chance to
    // a graceful termination handling/unwinding.
    return current_heap_limit + 1024 * 1024;
  };
  isolate->AddNearHeapLimitCallback(near_heap_limit_cb_lambda, result);

  return result;
}

void Js_isolate::Memory_manager::update_global_mem_stats(
    v8::HeapStatistics &stats) {
  // Let us aggregate current memory usage counters for the isolate into
  // global memory usage counters. To get actual global values, deduct values
  // which were added to global counters on previous call of this method for
  // the isolate.
  s_total_heap_size += (stats.total_heap_size() - m_old_stats.total_heap_size);
  s_used_heap_size += (stats.used_heap_size() - m_old_stats.used_heap_size);
  // Using value from v8::HeapStatistics rather than directly from
  // Memory_manager sounds more future-proof. For example, in future we might
  // add size of SQL result objects to the amount of externally allocated
  // memory (by reporting it to V8 using appropriate API).
  s_external_memory_size +=
      (stats.external_memory() - m_old_stats.external_memory_size);

  // Save added counter values so they can be deducted on future calls or
  // isolate destruction.
  m_old_stats.total_heap_size = stats.total_heap_size();
  m_old_stats.used_heap_size = stats.used_heap_size();
  m_old_stats.external_memory_size = stats.external_memory();
}

void Js_isolate::Memory_manager::update_global_mem_stats() {
  v8::HeapStatistics stats;
  m_js_isolate->m_isolate->GetHeapStatistics(&stats);
  update_global_mem_stats(stats);
}

std::string Js_isolate::Memory_manager::get_mem_stats_json(
    Memory_manager *mem_mgr) {
  Json_string_buffer string_buffer;
  Json_writer json_writer(string_buffer);

  json_writer.StartObject();

  // Add information about memory usage by current connection/
  // user pair's Isolate.
  json_writer.Key(STRING_WITH_LEN("local"));

  if (mem_mgr != nullptr) {
    // Case when Js_isolate and Memory_manager for current connection/
    // user pair exist.

    // Get current memory usage stats from V8.
    v8::HeapStatistics stats;
    mem_mgr->m_js_isolate->m_isolate->GetHeapStatistics(&stats);

    // Update global counters to avoid so they do not look
    // too outdated in resulting JSON.
    mem_mgr->update_global_mem_stats(stats);

    json_writer.StartObject();

    json_writer.Key(STRING_WITH_LEN("totalHeapSize"));
    json_writer.Uint64(stats.total_heap_size());

    json_writer.Key(STRING_WITH_LEN("usedHeapSize"));
    json_writer.Uint64(stats.used_heap_size());

    json_writer.Key(STRING_WITH_LEN("externalMemorySize"));
    json_writer.Uint64(stats.external_memory());

    json_writer.EndObject();
  } else {
    // Handle the case when there is no isolate for current
    // connection/user pair.
    json_writer.Null();
  }

  // Add information about memory usage by all isolates.
  json_writer.Key(STRING_WITH_LEN("global"));

  json_writer.StartObject();

  json_writer.Key(STRING_WITH_LEN("totalHeapSize"));
  json_writer.Uint64(s_total_heap_size.load(std::memory_order_relaxed));

  json_writer.Key(STRING_WITH_LEN("usedHeapSize"));
  json_writer.Uint64(s_used_heap_size.load(std::memory_order_relaxed));

  json_writer.Key(STRING_WITH_LEN("externalMemorySize"));
  json_writer.Uint64(s_external_memory_size.load(std::memory_order_relaxed));

  // Number of contexts is the same as number isolates at this point.
  json_writer.Key(STRING_WITH_LEN("contexts"));
  json_writer.Uint(Js_v8::get_isolate_count());

  json_writer.EndObject();

  json_writer.EndObject();

  return std::string(string_buffer.GetString(), string_buffer.GetSize());
}

std::atomic<size_t> Js_isolate::Memory_manager::s_total_heap_size = 0;
std::atomic<size_t> Js_isolate::Memory_manager::s_used_heap_size = 0;
std::atomic<size_t> Js_isolate::Memory_manager::s_external_memory_size = 0;

int Js_isolate::Memory_manager::show_total_heap_size(MYSQL_THD, SHOW_VAR *var,
                                                     char *buff) {
  var->type = SHOW_LONGLONG;
  var->value = buff;
  auto *value = reinterpret_cast<ulonglong *>(buff);
  *value = s_total_heap_size.load(std::memory_order_relaxed);
  return 0;
}

int Js_isolate::Memory_manager::show_used_heap_size(MYSQL_THD, SHOW_VAR *var,
                                                    char *buff) {
  var->type = SHOW_LONGLONG;
  var->value = buff;
  auto *value = reinterpret_cast<ulonglong *>(buff);
  *value = s_used_heap_size.load(std::memory_order_relaxed);
  return 0;
}

int Js_isolate::Memory_manager::show_external_memory_size(MYSQL_THD,
                                                          SHOW_VAR *var,
                                                          char *buff) {
  var->type = SHOW_LONGLONG;
  var->value = buff;
  auto *value = reinterpret_cast<ulonglong *>(buff);
  *value = s_external_memory_size.load(std::memory_order_relaxed);
  return 0;
}

static int show_contexts(MYSQL_THD, SHOW_VAR *var, char *buff) {
  var->type = SHOW_LONG;
  var->value = buff;
  auto *value = reinterpret_cast<long *>(buff);
  // Number of contexts is the same as number isolates at this point.
  *value = Js_v8::get_isolate_count();
  return 0;
}

SHOW_VAR *Js_isolate::get_status_vars_defs() {
  // TODO: Consider adding per-isolate/connection/user statistics in future.
  //       These values probably should be exposed through P_S instead of
  //       status variable (usage of P_S is the general trend and allows
  //       to fight with information leakage more easily).
  static SHOW_VAR status_vars[] = {
      {"Js_lang_total_heap_size",
       reinterpret_cast<char *>(&Memory_manager::show_total_heap_size),
       SHOW_FUNC, SHOW_SCOPE_GLOBAL},
      {"Js_lang_used_heap_size",
       reinterpret_cast<char *>(&Memory_manager::show_used_heap_size),
       SHOW_FUNC, SHOW_SCOPE_GLOBAL},
      {"Js_lang_external_memory_size",
       reinterpret_cast<char *>(&Memory_manager::show_external_memory_size),
       SHOW_FUNC, SHOW_SCOPE_GLOBAL},
      // TODO: In future we might introduce Js_lang_isolates as well.
      {"Js_lang_contexts", reinterpret_cast<char *>(&show_contexts), SHOW_FUNC,
       SHOW_SCOPE_GLOBAL},
      {nullptr, nullptr, SHOW_UNDEF, SHOW_SCOPE_UNDEF}};

  return status_vars;
}

constexpr unsigned int MAX_MEM_SIZE_DEFAULT = 8 * 1024 * 1024;
constexpr unsigned int MAX_MEM_SIZE_MIN = 3 * 1024 * 1024;

unsigned int Js_isolate::Memory_manager::s_max_mem_size = MAX_MEM_SIZE_DEFAULT;
unsigned int Js_isolate::Memory_manager::s_max_mem_size_hard_limit_factor = 0;

bool Js_isolate::register_vars() {
  /*
    Register variables for limiting of per Isolate memory consumption.

    Note that while minimum value for max_mem_size setting might seem to be
    fairly big it was determined by expiriments. V8 tends to overrides/
    increases smaller values anyway.

    TODO: Possibly change the default after gathering more feedback from users.
  */
  INTEGRAL_CHECK_ARG(uint) max_mem_size_check, max_mem_size_hl_factor_check;

  max_mem_size_check.def_val = MAX_MEM_SIZE_DEFAULT;
  max_mem_size_check.min_val = MAX_MEM_SIZE_MIN;
  max_mem_size_check.max_val = 1024 * 1024 * 1024;
  max_mem_size_check.blk_sz = 1024;

  if (mysql_service_component_sys_variable_register->register_variable(
          CURRENT_COMPONENT_NAME_STR, MAX_MEM_SIZE_VAR_NAME,
          PLUGIN_VAR_INT | PLUGIN_VAR_UNSIGNED,
          "Soft limit for JS memory size for each user and connection", nullptr,
          nullptr, (void *)&max_mem_size_check,
          (void *)&Memory_manager::s_max_mem_size)) {
    // Registration of system variable is non-trivial, and can fail for
    // reasons other than OOM, so we play safe and try to handle error
    // gracefully here.
    my_error(ER_LANGUAGE_COMPONENT, MYF(0),
             "Can't register " MAX_MEM_SIZE_VAR_NAME
             " system variable for " CURRENT_COMPONENT_NAME_STR " component.");
    return true;
  }

  /*
    We do not enable hard JS memory limit by default, as hitting it causes
    process abort and, in general case, it is fairly easy to do so without
    triggering the best-effort limit first. We also do not allow turning it on/
    changing this setting at runtime to reduce misuse.
  */
  max_mem_size_hl_factor_check.def_val = 0;
  max_mem_size_hl_factor_check.min_val = 0;
  max_mem_size_hl_factor_check.max_val = 1024;
  max_mem_size_hl_factor_check.blk_sz = 0;

  if (mysql_service_component_sys_variable_register->register_variable(
          CURRENT_COMPONENT_NAME_STR, MAX_MEM_SIZE_HARD_LIMIT_FACTOR_VAR_NAME,
          PLUGIN_VAR_INT | PLUGIN_VAR_UNSIGNED | PLUGIN_VAR_READONLY,
          "Multiplier defining hard limit for JS memory size for each"
          "user and connection (0 means that V8 default is used)",
          nullptr, nullptr, (void *)&max_mem_size_hl_factor_check,
          (void *)&Memory_manager::s_max_mem_size_hard_limit_factor)) {
    // See above.
    my_error(ER_LANGUAGE_COMPONENT, MYF(0),
             "Can't register " MAX_MEM_SIZE_HARD_LIMIT_FACTOR_VAR_NAME
             " system variable for " CURRENT_COMPONENT_NAME_STR " component.");
    // We can't do much if the below call fails.
    (void)mysql_service_component_sys_variable_unregister->unregister_variable(
        CURRENT_COMPONENT_NAME_STR, MAX_MEM_SIZE_VAR_NAME);
    return true;
  }

  if (mysql_service_status_variable_registration->register_variable(
          get_status_vars_defs())) {
    // Registration of status variables can't fail unless OOM.
    // In this case error should have been reported already.
    //
    // We can't do much if the below calls fail.
    (void)mysql_service_component_sys_variable_unregister->unregister_variable(
        CURRENT_COMPONENT_NAME_STR, MAX_MEM_SIZE_HARD_LIMIT_FACTOR_VAR_NAME);
    (void)mysql_service_component_sys_variable_unregister->unregister_variable(
        CURRENT_COMPONENT_NAME_STR, MAX_MEM_SIZE_VAR_NAME);
    return true;
  }

  return false;
}

bool Js_isolate::unregister_vars() {
  if (mysql_service_component_sys_variable_unregister->unregister_variable(
          CURRENT_COMPONENT_NAME_STR, MAX_MEM_SIZE_VAR_NAME)) {
    // The above should not fail normally. Still we play safe.
    my_error(ER_LANGUAGE_COMPONENT, MYF(0),
             "Can't unregister " MAX_MEM_SIZE_VAR_NAME
             " system variable for " CURRENT_COMPONENT_NAME_STR " component.");
    return true;
  }

  // Unregistration of status variables can't fail.
  always_ok(mysql_service_status_variable_registration->unregister_variable(
      get_status_vars_defs()));

  return false;
}

bool Js_isolate::check_if_arr_buff_alloc_will_exceed_mem_limit(size_t length) {
  /*
    Unlike in check_if_heap_alloc_will_exceed_mem_limit() case here it is
    important not to be too optimistic, as it would lead to aborts in later
    calls to v8::ArrayBuffer::New() when it will get nullptr from allocator.
  */
  Js_thd *js_thd = Js_thd::get_current_js_thd();
  std::string auth_id = js_thd->get_current_auth_id();
  auto auth_id_ctx = js_thd->get_auth_id_context(auth_id);
  Js_isolate *that = auth_id_ctx->get_js_isolate();
  return that->m_memory_manager.check_mem_limit_at_arr_buff_alloc(length);
}

bool Js_isolate::check_if_heap_alloc_will_exceed_mem_limit(size_t length) {
  /*
    We do pretty naive and too optimistic check here. But that should be
    OK taking into account context in which this method used.

    OTOH since this method is called for each CHAR/VARCHAR/TEXT argument
    we try to keep it as lightweight as possible (hence MAX_MEM_SIZE_MIN
    check).

    TODO: Consider doing something more complex but still cheap - like
          summing up lengths of all arguments.
  */
  if (length >= MAX_MEM_SIZE_MIN) {
    Js_thd *js_thd = Js_thd::get_current_js_thd();
    std::string auth_id = js_thd->get_current_auth_id();
    auto auth_id_ctx = js_thd->get_auth_id_context(auth_id);
    Js_isolate *that = auth_id_ctx->get_js_isolate();

    if (that->m_memory_manager.check_mem_limit_at_heap_alloc(length)) {
      return true;
    }
  }
  return false;
}

mysql_thd_store_slot Js_thd::s_thd_slot = nullptr;

void Js_thd::register_slot() {
  auto free_fn_lambda = [](void *resource) {
    delete reinterpret_cast<Js_thd *>(resource);
    return 0;
  };
  always_ok(mysql_service_mysql_thd_store->register_slot(
      CURRENT_COMPONENT_NAME_STR, free_fn_lambda, &s_thd_slot));
}

void Js_thd::unregister_slot() {
  always_ok(mysql_service_mysql_thd_store->unregister_slot(s_thd_slot));
  s_thd_slot = nullptr;
}

void Js_thd::Auth_id_context::report_error(v8::TryCatch &try_catch,
                                           const char *fallback_error_msg) {
  Js_isolate *js_isolate = get_js_isolate();

  v8::Isolate *isolate = js_isolate->get_v8_isolate();

  v8::HandleScope handle_scope(isolate);

  v8::Local<v8::Context> context = get_context();

  // User string representation of exception as error message.
  v8::String::Utf8Value exception_str(isolate, try_catch.Exception());
  const char *error_msg_str =
      *exception_str ? *exception_str : fallback_error_msg;

  // Report error with this message to SQL core/user.
  my_error(ER_LANGUAGE_COMPONENT, MYF(0), error_msg_str);

  // Also save it the context so it can be retrieved using UDF
  m_last_error = error_msg_str;

  // Also make this message part of extended info.
  //
  // TODO: Consider supporting returning the same info as JSON,
  //       so it easier to use it in tooling.
  std::stringstream error_info_str;
  error_info_str << "Error: " << error_msg_str;

  // Append additional info from v8::Message class if possible.
  v8::Local<v8::Message> message = try_catch.Message();
  if (!message.IsEmpty()) {
    // Add info about SQL program and line/column where the error has occurred
    // (take into account that line/column numbers might be not available).
    v8::String::Utf8Value rname(isolate, message->GetScriptResourceName());

    error_info_str << "\nAt: " << (*rname ? *rname : "<unknown>");

    int line_no = message->GetLineNumber(context).FromMaybe(-1);
    int start_col = message->GetStartColumn(context).FromMaybe(-1);
    int end_col = message->GetEndColumn(context).FromMaybe(-1);

    // Line numbers are 1-based
    if (line_no > 0) {
      error_info_str << ":" << line_no;
      // Column numbers are 0-based.
      if (start_col >= 0 && end_col >= 0) {
        error_info_str << ":" << start_col;
        if (end_col > start_col + 1) error_info_str << "-" << end_col;
      }
    }

    // If available add problematic source line with an optional ^^...^^
    // underline indicating its exact part.
    //
    // TODO: Consider also adding a few surrounding lines to give more context?
    //       Can be non-trivial as it will require splitting source into lines
    //       and taking into account wrapping code.
    v8::Local<v8::String> source_line_val;
    if (message->GetSourceLine(context).ToLocal(&source_line_val) &&
        source_line_val->Length() > 0) {
      // We assume that converions of JS string to UTF8 can't fail
      // (as OOM will abort the process).
      v8::String::Utf8Value source_line_str(isolate, source_line_val);
      error_info_str << "\nLine: " << *source_line_str;
      if (start_col >= 0 && end_col >= 0) {
        error_info_str << "\n      ";
        int i = 0;
        while (i < start_col) {
          error_info_str << ' ';
          ++i;
        }
        // Add at least one '^'.
        do {
          error_info_str << '^';
          ++i;
        } while (i < end_col);
      }
    }
  }

  // Also add stack trace for the error if it is available and has string
  // type (in theory, JS code can override to be any value).
  v8::Local<v8::Value> stack_trace_val;
  if (try_catch.StackTrace(context).ToLocal(&stack_trace_val) &&
      stack_trace_val->IsString() &&
      stack_trace_val.As<v8::String>()->Length() > 0) {
    // Again, we assume that converion of JS string to UTF8 can't fail.
    v8::String::Utf8Value stack_trace_str(isolate, stack_trace_val);
    error_info_str << "\nStack:\n" << *stack_trace_str;
  }
  m_last_error_info = error_info_str.str();
}

int Js_thd::Auth_id_context::clear_last_error() {
  int result = m_last_error.has_value() ? 1 : 0;
  m_last_error.reset();
  m_last_error_info.reset();
  return result;
}

Js_sp::Js_sp(stored_program_handle sp, uint16_t sql_sp_type) : m_sql_sp(sp) {
  assert(sql_sp_type == MYSQL_STORED_PROGRAM_DATA_QUERY_TYPE_FUNCTION ||
         sql_sp_type == MYSQL_STORED_PROGRAM_DATA_QUERY_TYPE_PROCEDURE);
  m_type = (sql_sp_type == MYSQL_STORED_PROGRAM_DATA_QUERY_TYPE_FUNCTION)
               ? Js_sp::FUNCTION
               : Js_sp::PROCEDURE;

  always_ok(mysql_service_mysql_stored_program_metadata_query->get(
      sp, "argument_count", &m_arg_count));

  // Construct "resource name" for routine, to be used by error reporting.
  mysql_cstring_with_length qname;
  always_ok(mysql_service_mysql_stored_program_metadata_query->get(
      sp, "qualified_name", &qname));

  m_resource_name.append("SQL ");
  m_resource_name.append(m_type == Js_sp::FUNCTION ? "FUNCTION "
                                                   : "PROCEDURE ");
  m_resource_name.append(qname.str, qname.length);
}

Js_sp::~Js_sp() {
  Js_thd *js_thd = Js_thd::get_current_js_thd();

  // Js_thd context might be already gone at this point.
  if (js_thd != nullptr) {
    js_thd->erase_sp(m_sql_sp);
  }
}

void Js_sp::prepare_wrapped_body_and_out_param_indexes() {
  mysql_cstring_with_length body;

  always_ok(mysql_service_mysql_stored_program_metadata_query->get(
      m_sql_sp, "sp_body", &body));

  // Comma separated list of INOUT and OUT parameter names.
  // Empty if this is FUNCTION or PROCEDURE without such parameters.
  std::string out_params_list;

  m_wrapped_body.append("((");

  for (size_t i = 0; i < m_arg_count; ++i) {
    if (i != 0) m_wrapped_body.append(", ");

    // We do not support JS routines with parameter names which are not
    // valid JS identifiers.
    //
    // MySQL rules for identifiers and thus parameter names are somewhat
    // more relaxed than JS rules.
    //
    // For example, MySQL identifier can start with digit (which JS doesn't
    // allow). If quoted identifier is used as parameter name in MySQL then
    // it can be any non-empty sequence consisting of Unicode BMP characters
    // other than U+0000 which doesn't end with space. This includes JS
    // reserved words. And some of JS reserved words can be used as MySQL
    // parameter names even without quoting (because they are not reserved
    // in SQL).
    //
    // Instead of detecting bad parameter names here, we rely on discovering
    // them later when wrapped routine body is compiled. The error produced
    // in this case might be cryptic but this probably acceptable for such
    // a corner case.
    //
    // TODO: Consider trying to produce better error message by checking
    //       parameter name when routine is created. This can be achieved
    //       by trying to compile, for each parameter separately, small
    //       piece of code which uses the name as identifier.
    //       Alternative is to require parameter names to start with letter,
    //       but this might be too limiting (what about '$' or '_' ?) and
    //       doesn't solve problem with reserved words.
    const char *arg_name;
    always_ok(mysql_service_mysql_stored_program_argument_metadata_query->get(
        m_sql_sp, i, "argument_name", &arg_name));

    m_wrapped_body.append(arg_name);

    if (m_type == Js_sp::PROCEDURE) {
      // MySQL only supports OUT and INOUT parameters in procedures.
      bool is_out_param;
      always_ok(mysql_service_mysql_stored_program_argument_metadata_query->get(
          m_sql_sp, i, "out_variable", &is_out_param));

      if (is_out_param) {
        // Name of second and later parameters needs to be separated from
        // the preceding one.
        if (!out_params_list.empty()) out_params_list.append(", ");
        out_params_list.append(arg_name);
        m_out_param_indexes.push_back(i);
      }
    }
  }
  m_wrapped_body.append(") => {");

  // For PROCEDURES in case of INOUT/OUT parameters we need to wrap JS code
  // passed to us in additional function/layer, so we can return values of
  // these parameters to our component as an array.
  //
  // We also do this to be able to detect and error out in situation when
  // one tries to return value from PROCEDURE.
  assert(out_params_list.empty() || m_type == Js_sp::PROCEDURE);
  if (m_type == Js_sp::PROCEDURE)
    m_wrapped_body.append("let js_lang_int_res = (() => {");

  // Add line break, so users won't see introductory wrapper code in
  // extended error info.
  m_wrapped_body.append("\n");

  m_wrapped_body.append(body.str);

  // Another line break, so users won't see finalizing wrapper code in
  // extended error info.
  m_wrapped_body.append("\n");

  // Finalize extra wrapping for PROCEDURE and pack IN/OUT parameter values
  // in an array to be returned.
  if (m_type == Js_sp::PROCEDURE) {
    m_wrapped_body.append("})();");
    m_wrapped_body.append("return [js_lang_int_res");
    if (!out_params_list.empty()) {
      m_wrapped_body.append(",");
      m_wrapped_body.append(out_params_list);
    }
    m_wrapped_body.append("];");
  }

  m_wrapped_body.append("})");
}

v8::Local<v8::Function> Js_sp::prepare_func(
    Js_thd::Auth_id_context *auth_id_ctx) {
  Js_isolate *js_isolate = auth_id_ctx->get_js_isolate();

  v8::Isolate *isolate = js_isolate->get_v8_isolate();

  // Caller should have locked and made this Isolate current one already.
  assert(v8::Locker::IsLocked(isolate));
  assert(isolate->IsCurrent());

  v8::EscapableHandleScope handle_scope(isolate);

  v8::Local<v8::Context> context = auth_id_ctx->get_context();

  // Enter the context for compiling and getting the wrapper function.
  v8::Context::Scope context_scope(context);

  v8::TryCatch try_catch(isolate);

  // Set extended routine name as script origin for our JS code
  // (it is used for error reporting).
  //
  // It can never exceed JS string length limit, so the below call can fail
  // only in case of OOM. Since V8 handles OOM by aborting the process we
  // do not try to handle it gracefully here either.
  //
  // Pass -1 as resource line offset to account for extra line at the start
  // of wrapped code. Doing this allows to have correct line numbers in
  // v8::Message objects and stack traces.
  v8::ScriptOrigin origin(
      v8::String::NewFromUtf8(isolate, m_resource_name.c_str(),
                              v8::NewStringType::kNormal,
                              m_resource_name.length())
          .ToLocalChecked(),
      -1);

  // Make v8::Function out of the wrapped code so we can use its Call()
  // method to execute the code with parameters passed in the array.

  v8::Local<v8::String> source;
  if (!v8::String::NewFromUtf8(isolate, m_wrapped_body.c_str(),
                               v8::NewStringType::kNormal,
                               m_wrapped_body.length())
           .ToLocal(&source)) {
    // Wrapped routine body can exceed V8 string length limit.
    // Though, in practice, InnoDB will complain in the error log about
    // very big rows/types if routine body is huge enough, even when it
    // is still below this limit.
    my_error(ER_LANGUAGE_COMPONENT, MYF(0),
             "Routine body exceeds V8 string length");
    return v8::Local<v8::Function>();
  }

  v8::Local<v8::Script> script;

  if (!v8::Script::Compile(context, source, &origin).ToLocal(&script)) {
    auth_id_ctx->report_error(try_catch, "Unknown compilation error.");
    return v8::Local<v8::Function>();
  }

  v8::Local<v8::Value> func_result;
  if (!script->Run(context).ToLocal(&func_result)) {
    auth_id_ctx->report_error(try_catch, "Unknown wrapper preparation error.");
    return v8::Local<v8::Function>();
  }

  if (!func_result->IsFunction()) {
    my_error(ER_LANGUAGE_COMPONENT, MYF(0),
             "Unable to compile wrapper function.");
    return v8::Local<v8::Function>();
  }

  return handle_scope.Escape(func_result.As<v8::Function>());
}

bool Js_sp::parse() {
  Js_thd *js_thd = Js_thd::get_or_create_current_js_thd();

  /*
    If this parse() call happens during CREATE FUNCTION/PROCEDURE for the
    routine we check if current user (i.e. user creating routine) has a global
    CREATE_JS_ROUTINE privilege in addition to usual CREATE_ROUTINE privilege
    on schema level required and checked by SQL core.
  */
  if (js_thd->is_create_routine_command() &&
      !js_thd->current_user_has_create_js_privilege()) {
    my_error(ER_SPECIFIC_ACCESS_DENIED_ERROR, MYF(0),
             CREATE_PRIVILEGE_NAME.data());
    return true;
  }

  // Prepare text of wrapper function for JS code.
  prepare_wrapped_body_and_out_param_indexes();

  /*
    Get Isolate and JS context for current connection and active user.
    Create one if needed.

    For security reasons, we use separate Isolates for execution of routines
    under different users within the same connection (this can happen e.g.
    due to calls of SQL SECURITY DEFINER routines).

    We use separate JS context for each connection. Additionally, we use
    separate contexts for execution of routines under different users within
    the same connection.

    Routines which are executed under the same user in the same connection
    use the same JS context.

    We played with idea of having separate context for each routine call.
    While it is nice from security point of view, it turned out to introduce
    too much performance overhead for smaller functions (e.g. for simple
    function calls calculating 50! we observed 10x slowdown caused by this).

    TODO: Consider supporting usage of different contexts for the same
          connection/account depending on system variable value or routine
          attribute.
  */
  std::string auth_id = js_thd->get_current_auth_id();

  auto auth_id_ctx = js_thd->get_or_create_auth_id_context(auth_id);

  Js_isolate *js_isolate = auth_id_ctx->get_js_isolate();

  v8::Isolate *isolate = js_isolate->get_v8_isolate();

  /*
    We need to destroy isolate and report error to user if we exceed memory
    limit. Since it needs to be done after we stop using/unlock isolate it
    is convenient to do this using scope guard handler.
  */
  auto mem_limit_guard = create_scope_guard([=]() {
    if (js_isolate->is_max_mem_size_exceeded()) {
      js_thd->erase_auth_id_context(auth_id);
      my_error(ER_LANGUAGE_COMPONENT, MYF(0), "Memory limit exceeded.");
    }
  });

  // Lock the isolate to follow convention.
  v8::Locker locker(isolate);

  // Make this isolate the current one.
  v8::Isolate::Scope isolate_scope(isolate);

  // Create a stack-allocated handle scope.
  v8::HandleScope handle_scope(isolate);

  /*
    Compile and execute script to get wrapper function object. Later we
    will use this function's Call() method for running our routine.

    For SQL SECURITY DEFINER routines this will be the only Function
    object we will use.
    SQL SECURITY INVOKER routines might be called with different user
    account active within the same connection. So we might need to build
    more Function objects for this account's JS context.
  */
  v8::Local<v8::Function> func = prepare_func(auth_id_ctx);

  // When memory limit is exceeded we want to fail even if we were able
  // prepare function. Scope guard will take care of isolate and error
  // reporting.
  if (js_isolate->is_max_mem_size_exceeded() || func.IsEmpty()) return true;

  auth_id_ctx->add_function(m_sql_sp, func);

  // Prepare functions for getting parameter values.
  if (prepare_get_param_funcs()) return true;
  // And setting out parameter and return values.
  if (m_type == Js_sp::FUNCTION) {
    if (prepare_set_retval_func()) return true;
  } else if (has_out_param()) {
    if (prepare_set_out_param_funcs()) return true;
  }

  return false;
}

bool Js_sp::execute() {
  Js_thd *js_thd = Js_thd::get_or_create_current_js_thd();

  /*
    Get Isolate and JS context for current connection and active user.
    For SQL SECURITY INVOKER routines such Isolate/Context might not exist yet
    (since earlier parse() call might have happened under different user, so
    create one if needed.

    TODO: Consider optimizing case for SQL SECURITY DEFINER routines?
          This can be non-trivial due to Js_thd vs Js_sp lifetime issues
          and case when Isolate have been destroyed due to OOM.
  */
  std::string auth_id = js_thd->get_current_auth_id();

  auto auth_id_ctx = js_thd->get_or_create_auth_id_context(auth_id);

  Js_isolate *js_isolate = auth_id_ctx->get_js_isolate();

  /*
    We need to destroy isolate and report error to user if we exceed memory
    limit. Since it needs to be done after we stop using/unlock isolate it
    is convenient to do this using scope guard handler.
  */
  auto mem_limit_guard = create_scope_guard([=]() {
    if (js_isolate->is_max_mem_size_exceeded()) {
      js_thd->erase_auth_id_context(auth_id);
      my_error(ER_LANGUAGE_COMPONENT, MYF(0), "Memory limit exceeded.");
    }
  });

  v8::Isolate *isolate = js_isolate->get_v8_isolate();

  v8::Locker locker(isolate);

  // Make this isolate the current one.
  v8::Isolate::Scope isolate_scope(isolate);

  // Create a stack-allocated handle scope.
  v8::HandleScope handle_scope(isolate);

  /*
    Get Function object for the routine in this context.

    Again, object for active account and its context might be missing
    in case of SQL SECURITY INVOKER routine, so prepare a new one if
    necessary.
  */
  v8::Local<v8::Function> func = auth_id_ctx->get_function(m_sql_sp);

  if (func.IsEmpty()) {
    func = prepare_func(auth_id_ctx);
    // When memory limit is exceeded we want to fail even if we were able
    // prepare function. Scope guard will take care of isolate and error
    // reporting.
    if (js_isolate->is_max_mem_size_exceeded() || func.IsEmpty()) return true;
    auth_id_ctx->add_function(m_sql_sp, func);
  }

  v8::Local<v8::Context> context = auth_id_ctx->get_context();

  // Enter the context for executing the function.
  v8::Context::Scope context_scope(context);

  v8::TryCatch try_catch(isolate);

  // Build array with program parameters.
  v8::Local<v8::Value> *arg_arr = nullptr;

  auto arg_arr_guard = create_scope_guard([&arg_arr]() { delete[] arg_arr; });

  if (m_arg_count) {
    arg_arr = new v8::Local<v8::Value>[m_arg_count];
    for (size_t i = 0; i < m_arg_count; ++i) {
      arg_arr[i] = m_get_param_func[i](isolate, i);

      // When memory limit is exceeded we want to fail even if we were able
      // to get parameter value. Scope guard will take care of isolate and
      // error reporting.
      if (js_isolate->is_max_mem_size_exceeded()) return true;

      if (arg_arr[i].IsEmpty()) {
        // It is responsibility of get_param_func function to
        // report error to user.
        return true;
      }
    }
  }

  /*
    We about to execute JS code, which can take time. Make this execution
    KILLable by installing a handler to be called when connection/statement
    is killed (or MAX_EXECUTION_TIME limit is reached).
    This handler aborts execution of JS code in the connection by calling
    v8::Isolate::TerminateExecution() method. It gets pointer to Isolate
    to be aborted as a parameter.
  */
  js_thd->install_kill_handler(isolate);

  // But before proceeding, we need to check if we were killed before
  // handler has been installed and we have not noticed this.
  bool killed_earlier = js_thd->is_killed();

  v8::MaybeLocal<v8::Value> maybe_result;
  if (!killed_earlier) {
    // Call the wrapper function passing parameters to it.
    maybe_result = func->Call(context, context->Global(), m_arg_count, arg_arr);
  }

  js_thd->reset_kill_handler();

  // When memory limit is exceeded we want to fail even if we were able to
  // execute call/get return value. Scope guard will take care of isolate
  // and error reporting.
  if (js_isolate->is_max_mem_size_exceeded()) return true;

  if (killed_earlier || js_thd->is_killed() ||
      isolate->IsExecutionTerminating() /* Extra-safety. */) {
    isolate->CancelTerminateExecution();
    // It is responsibility of SQL core to report an error in case when
    // connection or query was killed.
    return true;
  }

  v8::Local<v8::Value> result;
  if (!maybe_result.ToLocal(&result)) {
    auth_id_ctx->report_error(try_catch, "Unknown execution error");
    return true;
  }

  if (m_type == Js_sp::FUNCTION) {
    if (m_set_retval_func(context, 0, result) ||
        // When memory limit is exceeded we want to fail even if we were
        // able to store return value.
        js_isolate->is_max_mem_size_exceeded())
      return true;
  } else {
    // Values of INOUT and OUT parameters need to be unpacked from the array
    // returned as result of function execution to corresponding parameters
    // in runtime context.
    //
    // We also bark if one tries to return value other than undefined from
    // PROCEDURE using return statement.
    //
    // TODO: Discuss with reviewer, I am not quite convinced it is worth
    // doing it taking into account that extra wrapping + creation of array
    // costs something.

    // Execution of wrapper function in case of PROCEDURE must return
    // an array or throw (which is handled above).
    assert(result->IsArray());
    v8::Local<v8::Array> arr_result = result.As<v8::Array>();

    // The array returned must have the first element representing return
    // value.
    if (!arr_result->Get(context, 0).ToLocalChecked()->IsUndefined()) {
      my_error(ER_LANGUAGE_COMPONENT, MYF(0),
               "Returning value from PROCEDURE is not allowed");
      return true;
    }

    if (has_out_param()) {
      size_t res_arr_idx = 1;
      for (auto param_idx : m_out_param_indexes) {
        if (m_set_out_param_func[res_arr_idx - 1](
                context, param_idx,
                // The array returned must have an element for each OUT param.
                arr_result->Get(context, res_arr_idx).ToLocalChecked())) {
          // OUT param setter function is responsible for reporting error
          // in case of its failure.
          return true;
        }
        // When memory limit is exceeded we want to fail even if we
        // were able to store OUT parameter value.
        if (js_isolate->is_max_mem_size_exceeded()) return true;
        ++res_arr_idx;
      }
    }
  }

  // Memory limit should not be exceeded here. Still we play safe.
  mem_limit_guard.release();

  return false;
}

bool register_create_privilege() {
  if (mysql_service_dynamic_privilege_register->register_privilege(
          CREATE_PRIVILEGE_NAME.data(), CREATE_PRIVILEGE_NAME.length())) {
    // With current implementation of the dynamic_privilege_register
    // service this should never happen. But we try to play safe since what
    // the service does is non-trivial and implementation might change.
    my_error(ER_LANGUAGE_COMPONENT, MYF(0),
             "Can't register privilege for " CURRENT_COMPONENT_NAME_STR
             " component.");
    return true;
  }
  return false;
}

bool unregister_create_privilege() {
  if (mysql_service_dynamic_privilege_register->unregister_privilege(
          CREATE_PRIVILEGE_NAME.data(), CREATE_PRIVILEGE_NAME.length())) {
    // With current implementation of the dynamic_privilege_register
    // service this should never happen. But we try to play safe since what
    // the service does is non-trivial and implementation might change.
    my_error(ER_LANGUAGE_COMPONENT, MYF(0),
             "Can't unregister privilege for " CURRENT_COMPONENT_NAME_STR
             " component.");
    return true;
  }
  return false;
}

bool register_vars() {
  if (Js_isolate::register_vars()) return true;
  if (Js_console::register_sys_var()) return true;
  return false;
}

bool unregister_vars() {
  if (Js_isolate::unregister_vars() || Js_console::unregister_sys_var())
    return true;
  return false;
}
