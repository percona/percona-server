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
  This and corresponding .cc file contain classes and routines which are
  the central part of our implementation of support for JS routines.
*/

#ifndef COMPONENT_JS_LANG_JS_LANG_CORE_H
#define COMPONENT_JS_LANG_JS_LANG_CORE_H

#include <atomic>
#include <cstdint>
#include <functional>
#include <unordered_map>

#include <libplatform/libplatform.h>
#include <v8.h>

#include <mysql/components/services/language_service.h>  // stored_program_handle

#include "js_lang_common.h"
#include "js_lang_console.h"

/**
  Representation of V8 engine in our component.

  @note Since there can be only one V8 engine in an application it
  contains static methods/members only. Could be made a singleton,
  but we decided to avoid extra complexity related to this for now.
*/
class Js_v8 {
 public:
  /** Initialize V8 for use in our component. */
  static void init();

  /**
    Shutdown V8 before shutting down component.

    @note Can't be called if V8 is in use/there are still V8 isolates around.
  */
  static void shutdown();

  /**
    Checks if V8 is in use/there are any v8::Isolates around (so V8/component
    shutdown is not allowed) or V8 has been shutdown earlier (so V8/component
    initialization are not allowed).
  */
  static bool is_used_or_shutdown() { return s_ref_count.load() != 0; }

  /** Get number of v8::Isolate object around. */
  static unsigned int get_isolate_count() {
    auto ref_count = s_ref_count.load(std::memory_order_relaxed);
    // Play safe, do not return special -1 value to the caller.
    return (ref_count >= 0) ? ref_count : 0;
  }

  /**
    Pump isolate's foreground message loop, i.e. process tasks posted to
    isolate's foreground task queue (for example, some GC steps, which
    were added by threads doing background GC activity).
  */
  static void pump_message_loop(v8::Isolate *iso) {
    while (v8::platform::PumpMessageLoop(
        s_platform.get(), iso, v8::platform::MessageLoopBehavior::kDoNotWait))
      continue;
  }

  /**
    Notify V8 platform that we are about to dispose isolate,
    so it can properly tear down its structures associated with
    the isolate (e.g. drain the task queue for it).
  */
  static void notify_isolate_shutdown(v8::Isolate *iso) {
    v8::platform::NotifyIsolateShutdown(s_platform.get(), iso);
  }

 private:
  // Increment/decrement V8 usage/isolate global reference counter.
  static void inc_ref_count() {
    assert(s_ref_count.load() >= 0);
    ++s_ref_count;
  }
  static void dec_ref_count() { --s_ref_count; }

  // Js_isolate methods need access to inc/dec_ref_count;
  friend class Js_isolate;

  // V8 object representing Platform we are on.
  static std::unique_ptr<v8::Platform> s_platform;

  // Global reference counter which aims to block component shutdown and V8
  // deinitialization while V8 is being used/having v8::Isolate around.
  //
  // Set to -1 to indicate that process of component shutdown has started
  // (used for debug purposes) or completed (we rely on this to block
  // UNINSTALL -> INSTALL scenario).
  static std::atomic<int> s_ref_count;
};

/**
  Wrapper class around v8::Isolate which simplifies its lifetime management
  by packaging it with its allocator and other Isolate specific data.
*/
class Js_isolate {
 private:
  Js_isolate() : m_memory_manager(this) {}

 public:
  ~Js_isolate() {
    // Let V8 prepare for isolate disposal.
    Js_v8::notify_isolate_shutdown(m_isolate);
    m_isolate->Dispose();
    // Destroy allocator we got from V8 before allowing V8 shutdown.
    m_memory_manager.m_arr_buff_allocator.reset();
    // Decrement global V8/Isolate reference counter to allow component
    // shutdown if there are no other Isolates around.
    Js_v8::dec_ref_count();
  }

  // Block default copy/move semantics.
  Js_isolate(Js_isolate const &rhs) = delete;
  Js_isolate(Js_isolate &&rhs) = delete;
  Js_isolate &operator=(Js_isolate const &rhs) = delete;
  Js_isolate &operator=(Js_isolate &&rhs) = delete;

  /**
    Create new Js_isolate and increments global counter of isolates
    accordingly.
  */
  static Js_isolate *create();

  v8::Isolate *get_v8_isolate() const { return m_isolate; }

  bool is_max_mem_size_exceeded() const {
    return m_memory_manager.m_is_max_mem_size_exceeded;
  }

  /**
    Mark isolate as exceeded memory limit and request execution
    of JS in it to be aborted.
  */
  void raise_max_mem_size_exceeded() {
    m_memory_manager.m_is_max_mem_size_exceeded = true;
    m_isolate->TerminateExecution();
  }

  /**
    Get current memory usage counters for this isolate and aggregate them
    into global memory usage counters.
  */
  void update_global_mem_stats() { m_memory_manager.update_global_mem_stats(); }

  /**
    Get information about memory usage by the isolate for current connection/
    user pair (passed as parameter) and by all isolates in the system as JSON
    object.

    @param js_iso - Pointer to Js_isolate object for current connection/
                    user pair if exists. nullptr - if there is no such object
                    (e.g. connection was never used for running JS code).
  */
  static std::string get_mem_stats_json(Js_isolate *js_iso) {
    return Memory_manager::get_mem_stats_json(
        (js_iso != nullptr) ? &(js_iso->m_memory_manager) : nullptr);
  }

  /**
    Helper which allows to check in advance if buffer we are about allocate
    for ArrayBuffer object will fit into the memory limit. If no we mark
    isolate as exceeding the limit and request JS execution to be aborted.

    @note Concurrent background GC freeing buffers in allocator should not
          be a problem for this call. It migh fail in cases when it could
          have succeeded, but will never do vice versa.

    @return True if limit is about to be exceeded and isolate was marked
            marked as such. False otherwise.
  */
  static bool check_if_arr_buff_alloc_will_exceed_mem_limit(size_t length);

  /**
    Helper which does naive and optimistic check if string object we are about
    to allocate will fit into the memory limit. If no we mark isolate as
    exceeding the limit and request JS execution to be aborted.

    @note This function is really optimistic (not even best-efforts!) and
          should not be relied up in cases when we must ensure that the
          upcoming allocation fits into the limit.

    @return True if limit is about to be exceeded and isolate was marked
            marked as such. False otherwise.
  */
  static bool check_if_heap_alloc_will_exceed_mem_limit(size_t length);

  /** Get definitions of memory usage status variables. */
  static SHOW_VAR *get_status_vars_defs();

  /**
    Register/unregister system variable which controls per-isolate maximum
    memory size as well as global status variables for memory usage.

    @retval False - Success.
    @retval True  - Failure (error has been reported).
  */
  static bool register_vars();
  static bool unregister_vars();

 private:
  v8::Isolate *m_isolate{nullptr};

  /**
    Helper class which is responsible for memory accounting and limit
    enforcement for the isolate.

    Implements v8::ArrayBuffer::Allocator interface to be able properly
    enforce memory limits for JS ArrayBuffer objects. This is done by
    wrapping default V8 ArrayBuffer allocator implementation with memory
    accounting/tracking functionality.
  */
  class Memory_manager : public v8::ArrayBuffer::Allocator {
   public:
    Memory_manager(Js_isolate *js_iso)
        : m_js_isolate(js_iso),
          m_arr_buff_allocator(
              v8::ArrayBuffer::Allocator::NewDefaultAllocator()),
          m_max_mem_size(s_max_mem_size) {
#ifndef NDEBUG
      always_ok(mysql_service_mysql_current_thread_reader->get(&m_mysql_thd));
#endif
    }

    ~Memory_manager() {
      // Since V8 isolate has been deleted at this point it is good time
      // to decrease global memory usage counters.
      s_total_heap_size -= m_old_stats.total_heap_size;
      s_used_heap_size -= m_old_stats.used_heap_size;
      s_external_memory_size -= m_old_stats.external_memory_size;
    }

    // Block default copy/move semantics.
    Memory_manager(Memory_manager const &rhs) = delete;
    Memory_manager(Memory_manager &&rhs) = delete;
    Memory_manager &operator=(Memory_manager const &rhs) = delete;
    Memory_manager &operator=(Memory_manager &&rhs) = delete;

    void check_thd() {
#ifndef NDEBUG
      MYSQL_THD mysql_thd;
      always_ok(mysql_service_mysql_current_thread_reader->get(&mysql_thd));
      assert(m_mysql_thd == mysql_thd);
#endif
    }

    /**
      Check if allocation of new memory buffer will exceed memory limit,
      mark isolate as exceeding memory limit and request JS execution abort
      in it if yes.

      @retval False - limit is not exceeded.
      @retval True  - limit is exceeded.
    */
    bool check_mem_limit_at_arr_buff_alloc(size_t length);

    /**
      Check if we about to allocate on heap object that is bigger than memory
      limit, mark isolate as exceeding memory limit and request JS execution
      abort in it if yes.

      @retval False - objects is smaller than memory limit (the limit still
                      might be exceeded though).
      @retval True  - object exceeds than memory limit.
    */
    bool check_mem_limit_at_heap_alloc(size_t length) {
      if (length > m_max_mem_size) {
        m_js_isolate->raise_max_mem_size_exceeded();
        return true;
      }
      return false;
    }

    /**
      At minor/major GC time check if memory limit for isolate is exceeded,
      mark it as such and request JS execution abort in it if yes.
    */
    void check_mem_limit_at_GC();

    /* Start of v8::ArrayBuffer::Allocator interface implementation. */

    void *Allocate(size_t length) override;

    void *AllocateUninitialized(size_t length) override;

    void Free(void *data, size_t length) override;

    void *Reallocate(void *data, size_t old_length, size_t new_length) override;

    /* End of v8::ArrayBuffer::Allocator interface implementation. */

    /**
      Aggregate current memory usage counters for this isolate (which are
      passed in parameter) into global memory usage counters.
    */
    void update_global_mem_stats(v8::HeapStatistics &stats);

    /**
      Get current memory usage counters for this isolate and aggregate them
      into global memory usage counters.
    */
    void update_global_mem_stats();

    /**
      Get information about memory usage by the isolate for current connection/
      user pair (its Memory_manager passed as parameter) and by all isolates in
      the system as a JSON object.

      @param mem_mgr - Pointer to Memory_manager object for current connection/
                       user pair's isolate if exists.
                       nullptr - if there is no such object (e.g. if connection
                       was never used for running JS code).
    */
    static std::string get_mem_stats_json(Memory_manager *mem_mgr);

    /* Helpers implementing memory usage status variables. */
    static int show_total_heap_size(MYSQL_THD, SHOW_VAR *var, char *buff);
    static int show_used_heap_size(MYSQL_THD, SHOW_VAR *var, char *buff);
    static int show_external_memory_size(MYSQL_THD, SHOW_VAR *var, char *buff);

    /** Pointer to isolate to which this memory manager belongs. */
    Js_isolate *m_js_isolate;

#ifndef NDEBUG
    /**
      Opaque handle of THD object which uses this isolate/memory manager,
      used as an approximation of physical thread id by debug assertions.
    */
    MYSQL_THD m_mysql_thd;
#endif

    /**
      V8 default ArrayBuffer::Allocator to which we forward ArrayBuffer
      buffer allocation requests.
    */
    std::unique_ptr<v8::ArrayBuffer::Allocator> m_arr_buff_allocator;

    /**
      Amount of memory allocated for ArrayBuffer buffers outside of V8 heap
      for the isolate.

      @note Needs to be atomic as these buffers can be freed from the
            background threads.
    */
    std::atomic<size_t> m_arr_buff_allocated{0};

    /**
      Total amount of memory which was used by the isolate during last GC.
      It is used to determine if we have approached dangerously close to
      memory limit since the last GC, or it was already the case at the
      last GC.
    */
    size_t m_last_danger_zone_check_mem_size{0};

    /**
      Maximum memory size limit which is enforced for this isolate/memory
      manager.

      @note This limit is set at Isolate creation time and then used by checks
            for the rest of its life. We cannot use value of global dynamic
            variable directly for this, as change in its value will bring it
            out of sync with V8 heap limit which set at Isolate creation time,
            which might lead to crashes.
    */
    const size_t m_max_mem_size;

    /**
      Indicates if per-isolate maximum memory size has been exceeded,
      and therefore JS execution in it should be aborted and isolate
      should be destroyed.
    */
    bool m_is_max_mem_size_exceeded{false};

    /**
      Global dynamic variable for maximum memory size soft limit to set for
      new isolates.
    */
    static unsigned int s_max_mem_size;

    /**
      Global read-only (i.e. start-up settable) variable for hard memory limit
      in V8 engine.
      0 - indicates that no V8 limit is set explicitly and the default V8 limit
      is used (typically > 1Gb),
      non-0  - indicates that V8 limit is set explicitly. The value used as a
      multiplier which we use to produce limit value from m_max_mem_size.
    */
    static unsigned int s_max_mem_size_hard_limit_factor;

    /**
      Memory usage counter values for this isolate which already have been
      agreggated into global memory usage counters.
    */
    struct {
      size_t total_heap_size;
      size_t used_heap_size;
      size_t external_memory_size;
    } m_old_stats{0, 0, 0};

    /**
      Global counters of memory usage to be shown as status variables.

      TODO: Consider partitioning these counters if updating them ever
            becomes performance bottleneck (though it is unlikely).
    */
    static std::atomic<size_t> s_total_heap_size;
    static std::atomic<size_t> s_used_heap_size;
    static std::atomic<size_t> s_external_memory_size;
  } m_memory_manager;
};

/**
  Component's per connection context.

  @note Corresponds to and is associated with core's THD object.
*/
class Js_thd {
 public:
  explicit Js_thd(MYSQL_THD thd) : m_thd(thd) {}
  ~Js_thd() {}

  // Block default copy/move semantics.
  Js_thd(Js_thd const &rhs) = delete;
  Js_thd(Js_thd &&rhs) = delete;
  Js_thd &operator=(Js_thd const &rhs) = delete;
  Js_thd &operator=(Js_thd &&rhs) = delete;

  /**
    Register slot for keeping Js_thd value associated with current connection/
    its THD object using mysql_thd_store service.
  */
  static void register_slot();
  /**
    Unregister slot used for keeping Js_thd value associated with current
    connection/its THD object.
  */
  static void unregister_slot();

  /** Get Js_thd for current connection/THD, create if necessary. */
  static Js_thd *get_or_create_current_js_thd() {
    MYSQL_THD thd;
    always_ok(mysql_service_mysql_current_thread_reader->get(&thd));

    void *opaque = mysql_service_mysql_thd_store->get(thd, s_thd_slot);
    Js_thd *js_thd = reinterpret_cast<Js_thd *>(opaque);
    if (js_thd == nullptr) {
      js_thd = new Js_thd(thd);
      always_ok(mysql_service_mysql_thd_store->set(thd, s_thd_slot, js_thd));
    }
    return js_thd;
  }

  /** Get Js_thd for current connection/THD, return nullptr if absent. */
  static Js_thd *get_current_js_thd() {
    MYSQL_THD thd;
    always_ok(mysql_service_mysql_current_thread_reader->get(&thd));

    void *opaque = mysql_service_mysql_thd_store->get(thd, s_thd_slot);
    return reinterpret_cast<Js_thd *>(opaque);
  }

  /**
    Return user@host pair representing user account currently active in the
    connection (it can be different than user that was authenticated at connect
    time if we are executing SQL SECURITY DEFINER routine, for example).
  */
  std::string get_current_auth_id() {
    Security_context_handle sctx;
    always_ok(mysql_service_mysql_thd_security_context->get(m_thd, &sctx));

    MYSQL_LEX_CSTRING user, host;
    always_ok(mysql_service_mysql_security_context_options->get(
        sctx, "priv_user", &user));
    always_ok(mysql_service_mysql_security_context_options->get(
        sctx, "priv_host", &host));

    std::string result;
    result.append(user.str);
    result.append("@");
    result.append(host.str);

    return result;
  }

  /**
    Per user account connection's context.

    Within the same connection JS code can be executed under different users
    (e.g. due to SQL SECURITY DEFINER/INVOKER clauses).

    To prevent information leakage, we use different Isolates to execute
    JS code under different users.

    TODO: At the moment we use an individual isolate for each connection and
    user pair, but we might change this in future if we discover that with
    such approach too much memory is consumed (current test show that execution
    of the whole js_lang_basic test causes heap to grown a bit below 2Mb).

    OTOH V8 Isolates can only be used by a single thread at each moment.
    So having a single isolate in the server per user sounds like not the best
    idea from concurrency point of view. So perhaps we should have a few
    isolates (for each user) partitioned between connection? (But then the
    question of freeing Isolates which are not in use for a long time raises).

    Also we use separate JS context for each connection (and separate contexts
    for execution of routines under different users within the same connection).

    Routines which are executed under the same user in the same connection
    share the same JS context.

    TODO: Make it a class once support for KILLability/OOM handling is added.
  */
  struct Auth_id_context {
    /** JS console for the specific connection and user pair. */
    Js_console console;

    Auth_id_context(Js_isolate *js_iso, v8::Local<v8::Context> &ctx)
        : m_js_isolate(js_iso),
          m_context(m_js_isolate->get_v8_isolate(), ctx) {}

    ~Auth_id_context() {
      // Follow the protocol.
      // Lock the isolate before deleting objects associated with it.
      {
        v8::Locker locker(m_js_isolate->get_v8_isolate());
        m_funcs.clear();
        m_context.Reset();
        m_unhandled_rejected_promises.clear();
      }
      delete m_js_isolate;
    }

    Js_isolate *get_js_isolate() const { return m_js_isolate; }

    v8::Local<v8::Context> get_context() const {
      return v8::Local<v8::Context>::New(m_js_isolate->get_v8_isolate(),
                                         m_context);
    }

    /**
      Get JS Function object which corresponds to SQL-layer stored routine
      handler/object in this context (for SQL INVOKER routines we can use
      several different Auth_id_context objects for the same SQL-layer
      stored routine handler/object).

      @return Handle for JS Function object, empty handle if no such object
              found in this context.
    */
    v8::Local<v8::Function> get_function(stored_program_handle sql_sp) const {
      auto j = m_funcs.find(sql_sp);

      if (j == m_funcs.end()) return v8::Local<v8::Function>();

      return v8::Local<v8::Function>::New(m_js_isolate->get_v8_isolate(),
                                          j->second);
    }

    /**
      Set JS Function object which corresponds to SQL-layer stored
      routine handler/object in this context.
    */
    void add_function(stored_program_handle sql_sp,
                      v8::Local<v8::Function> func) {
      m_funcs.try_emplace(sql_sp, m_js_isolate->get_v8_isolate(), func);
    }

    /**
      Delete Function object which corresponds to SQL-layer stored routine
      handler/object in this context if exists.
    */
    void erase_function(stored_program_handle sql_sp) { m_funcs.erase(sql_sp); }

    const std::optional<std::string> &get_last_error() const {
      return m_last_error;
    }

    const std::optional<std::string> &get_last_error_info() const {
      return m_last_error_info;
    }

    /**
      Report error from JS parsing or execution that was caught by exception
      handler to client (SQL core to be exact). Also update information about
      the last JS error which occurred for this context.

      @param try_catch            Exception handler which caught the error.
      @param fallback_error_msg   Error message to be used if we fail to get
                                  one from exception handler.
    */
    void report_error(v8::TryCatch &try_catch, const char *fallback_error_msg);

    /**
      Reset information about last JS error for the specific connection and
      user pair as if no error happened.

      @retval 1 - if there was a JS error, information about which was reset.
      @retval 0 - if there was no JS error for this connection and user.
    */
    int clear_last_error();

    /**
      Callback to notify about promise rejection with no handler, or
      about addition of handler to rejected promise.
    */
    static void promise_reject_cb(v8::PromiseRejectMessage reject_msg);

    /**
      Check if call to JS routine for user/connection left any unhandled
      rejected promises. Report error if yes. Prepare for next calls by
      resetting the unhandled rejected promises list.

      @retval False - there are no unhandled rejected promises.
      @retval True -  there was unhandled rejected promise, error has been
                      reported to SQL-layer/user.
    */
    bool process_unhandled_rejected_promises();

   private:
    /**
      Build string representation of extended information about JS error/
      exception which occurred for the user/connection context.

      @param error_msg_str    Error message string.
      @oaram context          Handle for V8::Context where error has happened.
      @param message          Handle V8 object representing error message
                              (can be empty).
      @param stack_trace_val  Handle for v8::Value representing stack-trace
                              for the error (can be empty).
    */
    std::string build_error_info(const std::string &error_msg_str,
                                 v8::Local<v8::Context> context,
                                 v8::Local<v8::Message> message,
                                 v8::Local<v8::Value> stack_trace_val);

   private:
    /**
      Pointer to isolate for this specific connection and user pair.

      @note Since Js_sp objects can outlive connection Js_thd context
            this member can't be cached by the former.
    */
    Js_isolate *m_js_isolate;

    /**
      JS context for the specific connection and user pair.

      @note Belongs to the Isolate referenced from this context.
    */
    v8::Global<v8::Context> m_context;

    /**
      Map with Function objects for routines to be executed in the JS
      context and Isolate for the specific connection and user pair.
      With SQL core routine handle being a key.

      Note that even SQL SECURITY DEFINER routines are always executed under
      the same account, and thus are using the same JS context within
      connection and need only one Function object (for connection), the
      pointer to this object can't be stored in Js_sp objects easily,
      as the latter can outlive Isolate associated with the connection
      (unless we do some kind of reference tracking and their invalidation).

      SQL SECURITY INVOKER routines might be executed with different user
      account being active within the same connection. So they might use
      different JS contexts and thus require separate Function objects
      for each context/account being used.
    */
    std::unordered_map<stored_program_handle, v8::Global<v8::Function>> m_funcs;

    /**
      Message for the last JS error which happened for the specific connection
      and user pair.
    */
    std::optional<std::string> m_last_error;
    /**
      Extended information about the last JS error which happened for the
      specific connection and user pair.
    */
    std::optional<std::string> m_last_error_info;

    /**
      Helper struct which bundles persistent handle for unhandled rejected
      promise with error message and extended info to be reported about it.
    */
    struct Rejected_promise {
      v8::Global<v8::Promise> promise;
      std::string error;
      std::string error_info;

      Rejected_promise(v8::Isolate *iso, v8::Local<v8::Promise> p,
                       std::string &&err, std::string &&err_info)
          : promise(iso, p), error(err), error_info(err_info) {}

      // This struct is movable since v8::Global is.
      Rejected_promise(Rejected_promise &&) = default;
      Rejected_promise &operator=(Rejected_promise &&) = default;

      // This struct is not copyable since v8::Global is not.
      Rejected_promise(const Rejected_promise &) = delete;
      Rejected_promise &operator=(const Rejected_promise &) = delete;
    };

    /**
      Rejected promises without handlers.

      @note We can't use associative container to speed up look up of
            promises as v8::Global only support equality comparison.
            OTOH, normally, there should not be many entries in the
            below vector so it should not matter much.
    */
    std::vector<Rejected_promise> m_unhandled_rejected_promises;
    /**
      Flag indicating that calls to promise rejection callbacks should be
      ignored since we are already in the middle of executing one.
    */
    bool m_ignore_promise_rejects{false};
  };

  /**
    Get Auth_id_context representing context for the user account in the
    current connection. Create one if necessary.
  */
  Auth_id_context *get_or_create_auth_id_context(const std::string &auth_id) {
    auto i = m_auth_id_contexts.find(auth_id);
    if (i == m_auth_id_contexts.end()) {
      // Active user didn't run any JS in this connection before.
      // We need to create new Isolate and Context.
      Js_isolate *js_isolate = Js_isolate::create();

      v8::Isolate *isolate = js_isolate->get_v8_isolate();

      // Lock the isolate and make it the current one to follow convention.
      v8::Locker locker(isolate);

      v8::Isolate::Scope isolate_scope(isolate);

      v8::HandleScope handle_scope(isolate);

      // Create JS context with our custom built-ins.
      //
      // First, create context using global object template.
      v8::Local<v8::Context> context = v8::Context::New(isolate);

      // Then, setup our custom 'console' object.
      Js_console::prepare_object(context);

      // After both Isolate and Context have been created it is good
      // time to update global counters of memory usage.
      // This operation is not cheap, but creation of isolate is
      // not cheap either!
      js_isolate->update_global_mem_stats();

      auto r = m_auth_id_contexts.try_emplace(auth_id, js_isolate, context);

      return &(r.first->second);
    }

    return &(i->second);
  }

  /**
    Get Auth_id_context representing context for the user account in the
    current connection. Return nullptr if there is no such context.
  */
  Auth_id_context *get_auth_id_context(const std::string &auth_id) {
    auto i = m_auth_id_contexts.find(auth_id);

    if (i == m_auth_id_contexts.end()) return nullptr;

    return &(i->second);
  }

  /**
    Get Auth_id_context representing context for the user account that is active
    in the current connection. Return nullptr if there is no such context.
  */
  static Auth_id_context *get_current_auth_id_context() {
    Js_thd *js_thd = Js_thd::get_current_js_thd();

    // Handle case when JS was never run in this connection.
    if (js_thd == nullptr) return nullptr;

    std::string auth_id = js_thd->get_current_auth_id();

    return js_thd->get_auth_id_context(auth_id);
  }

  /** Checks if connection executes CREATE FUNCTION or PROCEDURE statement. */
  bool is_create_routine_command() {
    mysql_cstring_with_length sql_command;
    always_ok(mysql_service_mysql_thd_attributes->get(m_thd, "sql_command",
                                                      &sql_command));

    constexpr size_t CREATE_ROUTINE_COM_MIN_LENGTH = 16;
    static_assert(
        CREATE_ROUTINE_COM_MIN_LENGTH ==
        std::min(strlen("create_procedure"), strlen("create_spfunction")));

    return (sql_command.length >= CREATE_ROUTINE_COM_MIN_LENGTH &&
            (strcmp(sql_command.str, "create_procedure") == 0 ||
             strcmp(sql_command.str, "create_spfunction") == 0));
  }

  /**
    Check if current active user account/security context has
    CREATE_JS_PRIVILEGE privilege.
  */
  bool current_user_has_create_js_privilege() {
    Security_context_handle sctx;
    always_ok(mysql_service_mysql_thd_security_context->get(m_thd, &sctx));

    return mysql_service_global_grants_check->has_global_grant(
        sctx, CREATE_PRIVILEGE_NAME.data(), CREATE_PRIVILEGE_NAME.length());
  }

  /**
    Install a handler for the connection that will ensure that JS code
    execution in isolate will be aborted if connection/statement in it
    gets killed.
  */
  void install_kill_handler(v8::Isolate *isolate) {
    auto kill_handler_lambda = [](MYSQL_THD /* killed_thd */,
                                  uint16_t /* state */, void *data) {
      reinterpret_cast<v8::Isolate *>(data)->TerminateExecution();
    };

    always_ok(mysql_service_mysql_thd_kill_handler->set(
        m_thd, kill_handler_lambda, isolate));
  }

  void reset_kill_handler() {
    always_ok(
        mysql_service_mysql_thd_kill_handler->set(m_thd, nullptr, nullptr));
  }

  /** Check if connection or statement that it executes is killed. */
  bool is_killed() {
    uint16_t status;
    mysql_service_mysql_thd_attributes->get(m_thd, "thd_status", &status);
    return status != STATUS_SESSION_OK;
  }

  /**
    Erase information associated with SQL-layer routine handle/object from
    this Js_thd and all its Auth_id_context objects.
  */
  void erase_sp(stored_program_handle sp) {
    for (auto &p : m_auth_id_contexts) {
      v8::Locker locker(p.second.get_js_isolate()->get_v8_isolate());
      p.second.erase_function(sp);
    }
  }

  void erase_auth_id_context(const std::string &auth_id) {
    m_auth_id_contexts.erase(auth_id);
  }

 private:
  // Opaque handle for corresponding THD object.
  MYSQL_THD m_thd;

  // Slot to associate Js_thd context with core's THD.
  static mysql_thd_store_slot s_thd_slot;

  // Map with per user-account contexts for the connection.
  std::unordered_map<std::string, Auth_id_context> m_auth_id_contexts;
};

/**
  Representation of stored program instance in our component.

  @note Corresponds to core's sp_head object and is used to store
  component specific data about stored program, as well as cache data
  from sp_head/sp_pcontext to reduce overhead from service
  calls to the server core.

  @note Note that since core's sp_head objects are associated with
        specific connection (unless they are for trigger), each
        object of this class is also associated with specific
        connection.
  @note However, due to the way SQL core implements per-component
        THD store, Js_sp object can temporarily outlive Js_thd object
        for its connection!
*/
class Js_sp {
 public:
  Js_sp(stored_program_handle sp, uint16_t sql_sp_type);

  ~Js_sp();

  // Block default copy/move semantics.
  Js_sp(Js_sp const &rhs) = delete;
  Js_sp(Js_sp &&rhs) = delete;
  Js_sp &operator=(Js_sp const &rhs) = delete;
  Js_sp &operator=(Js_sp &&rhs) = delete;

  /**
    Parse JS routine and prepare for its execution by building and compiling
    wrapper function around its JS code. Also setup functions for getting/
    setting values of parameters and return value.

    @note When called during CREATE FUNCTION/PROCEDURE statement also checks
          if current user has CREATE_JS_ROUTINE privilege, and fails
          otherwise.

    @retval False - Success.
    @retval True  - Failure (error has been reported).
  */
  bool parse();

  /**
    Execute JS routine, by invoking wrapper function.

    Use getter functions, which we set up during parse() phase, to get JS
    values to be passed as wrapper function parameters from SQL routine params
    (obtained from SQL routine runtime context).
    Use setter functions, which we set up at the same phase, to convert
    and store JS values returned by wrapper function to SQL routine OUT
    params values/return value.

    @retval False - Success.
    @retval True  - Failure (error has been reported).
  */
  bool execute();

 private:
  /**
    Prepare code of wrapper function for our JS routine.
    Also fill array with indexes of OUT/INOUT parameters in the array of all
    routine parameters.
  */
  void prepare_wrapped_body_and_out_param_indexes();

  /** Check if routine has any OUT or INOUT parameters. */
  bool has_out_param() const { return !m_out_param_indexes.empty(); }

  /**
    Setup functions for getting JS values corresponding to SQL values of
    IN/INOUT parameters of the routine.

    @retval False - Success.
    @retval True  - Failure (error has been reported).
  */
  bool prepare_get_param_funcs();
  /**
    Setup function for converting JS return value to SQL value and
    setting it as return value of the routine.

    @retval False - Success.
    @retval True  - Failure (error has been reported).
  */
  bool prepare_set_retval_func();
  /**
    Setup functions for converting JS values to SQL values and setting
    it as values of OUT/INOUT parameters of the routine.

    @retval False - Success.
    @retval True  - Failure (error has been reported).
  */
  bool prepare_set_out_param_funcs();

  /**
    Type of function for getting JS values corresponding to SQL values of
    IN/INOUT parameters of the routine

    These functions take current Isolate and parameter index as arguments.
    They return handle for JS value in case of success, and empty handle
    in case of failure (the error has been reported by the function in the
    latter case).
  */
  using get_param_func_t =
      std::function<v8::Local<v8::Value>(v8::Isolate *, size_t)>;

  /**
    Get function for getting JS value corresponding to SQL value of
    the specific IN/INOUT parameter of the routine.

    @retval Function wrapped in std::function<> class.
    @retval Empty std::function<> object in case of failure
            (error has been reported already).
  */
  get_param_func_t prepare_get_param_func(size_t idx);

  /**
    Type of function for converting JS value and setting it as value of the
    OUT/INOUT parameter or as a returv value of routine.

    These functions take current v8::Context, parameter index and the JS
    value as as arguments. They return false in case of success, and
    true in case of failure (the error has been reported by the function
    in the latter case).
  */
  using set_param_func_t =
      std::function<bool(v8::Local<v8::Context>, size_t, v8::Local<v8::Value>)>;

  /**
    Get function for converting JS value to SQL value and setting it as
    value of the specific OUT/INOUT parameter or as a return value of the
    routine.

    @retval Function wrapped in std::function<> class.
    @retval Empty std::function<> object in case of failure
            (error has been reported already).
  */
  template <class H>
  set_param_func_t prepare_set_param_func(size_t idx);

  /**
    Compile wrapper Function object for routine for the Auth_id_context (and
    thus JS) context.

    @param auth_id_ctx Auth_id_context for which Function object needs to be
                       prepared.

    @retval Handle for v8::Function object if all went well or empty handle
            in case of failure (the error has been reported already in the
            latter case).
  */
  v8::Local<v8::Function> prepare_func(Js_thd::Auth_id_context *auth_id_ctx);

  // Handle for core's stored program object (sp_head).
  stored_program_handle m_sql_sp;
  // Number of routine parameters (cached value from SQL core).
  uint32_t m_arg_count;
  // Routine type. At the moment we only support functions and procedures in JS.
  enum sp_type { FUNCTION, PROCEDURE };
  sp_type m_type;
  // "Resource name" for routine, used for error reporting.
  std::string m_resource_name;

  /**
    Wrapped body of stored program which was passed in CREATE FUNCTION/
    PROCEDURE statement.

    We do this to make a JS function with the same parameter names as
    SQL program has. After that we can compile this code to produce
    v8::Function object which can be called from our componets with
    parameters passed as an array.
  */
  std::string m_wrapped_body;
  // Indexes of OUT/INOUT parameters in array of all routine parameters.
  std::vector<size_t> m_out_param_indexes;

  // Parameter and return value getters/setters.
  std::vector<get_param_func_t> m_get_param_func;
  set_param_func_t m_set_retval_func;
  std::vector<set_param_func_t> m_set_out_param_func;
};

/**
  Register global dynamic CREATE_JS_ROUTINE privilege.

  @retval False - Success.
  @retval True  - Failure (error has been reported).
*/
bool register_create_privilege();
/**
  Unregister global dynamic CREATE_JS_ROUTINE privilege.

  @retval False - Success.
  @retval True  - Failure (error has been reported).
*/
bool unregister_create_privilege();

/**
  Register auxiliary UDFs providing access to additional
  information about JS component or context.

  @retval False - Success.
  @retval True  - Failure (error has been reported).
*/
bool register_udfs();

/**
  Unregister auxiliary UDFs providing access to additional
  information about JS component or context.

  @retval False - Success.
  @retval True  - Failure (error has been reported).
*/
bool unregister_udfs();

/**
  Register system variables allowing to control some of JS routines behavior.
  Also register status variables allowing to get some information about their
  execution.

  @retval False - Success.
  @retval True  - Failure (error has been reported).
*/
bool register_vars();

/**
  Unregister system variables allowing to control some of JS routines behavior.
  Unregister status variables allowing to get some information about their
  execution.

  @retval False - Success.
  @retval True  - Failure (error has been reported).
*/
bool unregister_vars();

#endif /* COMPONENT_JS_LANG_JS_LANG_CORE_H */
