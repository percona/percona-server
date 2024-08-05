/* Copyright (c) 2023, 2024 Percona LLC and/or its affiliates. All rights
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

#include <v8.h>

#include <mysql/components/services/language_service.h>  // stored_program_handle

#include "js_lang_common.h"

/**
  Wrapper class around v8::Isolate which simplifies its lifetime management
  through reference counting pointers by packaging it with its allocator.

  It also contains static methods for V8 initialization/shutdown as well as
  global reference counter for isolates, so we can block component shutdown
  if there are connections around that use isolates and therefore V8.
*/
class Js_isolate {
 public:
  Js_isolate(v8::Isolate *iso, v8::ArrayBuffer::Allocator *alloc)
      : m_isolate(iso), m_allocator(alloc) {}

  ~Js_isolate() {
    m_isolate->Dispose();
    delete m_allocator;
    // Decrement global Isolate reference counter to allow component
    // shutdown if there are no other Isolates around.
    --s_count;
  }

  /**
    Create new Js_isolate and increments global counter of isolates
    accordingly.
  */
  static std::shared_ptr<Js_isolate> create();

  v8::Isolate *get_v8_isolate() const { return m_isolate; }

  /**
    Checks if there are any v8::Isolates around (so V8/component shutdown
    is not allowed) or V8 has been shutdown earlier (so V8/component
    initialization are not allowed).
  */
  static bool is_used_or_v8_shutdown() { return s_count.load() != 0; }

  /** Initialize V8 for use in our component. */
  static void v8_init();

  /**
    Shutdown V8 before shutting down component.

    @note Can't be called if there are still V8 isolates around.
  */
  static void v8_shutdown();

 private:
  v8::Isolate *m_isolate;
  v8::ArrayBuffer::Allocator *m_allocator;

  // Global reference counter which aims to block component shutdown and V8
  // deinitialization while having v8::Isolate around.
  //
  // Set to -1 to indicate that process of component shutdown has started
  // (used for debug purposes) or completed (we rely on this to block
  // UNINSTALL -> INSTALL scenario).
  static std::atomic<int> s_count;
};

/**
  Component's per connection context.

  @note Corresponds to and is associated with core's THD object.
*/
class Js_thd {
 public:
  Js_thd(MYSQL_THD thd) : m_thd(thd) {}
  ~Js_thd() {
    // Play safe. Lock the isolate before deleting context associated with it.
    for (auto &p : m_auth_id_contexts) {
      {
        v8::Locker locker(p.second.isolate_ptr->get_v8_isolate());
        p.second.context.Reset();
      }
    }
  }

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
    /**
      Pointer to isolate for this specific connection and user pair.

      We have to use reference counting pointer here since SQL code imposes
      akward restrictions on Js_thd and Js_sp lifetime - Js_sp object
      representing routine in the connection can outlive connection's Js_thd
      object [sic!].
    */
    std::shared_ptr<Js_isolate> isolate_ptr;

    /**
      JS context for the specific connection and user pair.

      @note Belongs to the Isolate referenced from this context.
    */
    v8::Global<v8::Context> context;

    Auth_id_context(const std::shared_ptr<Js_isolate> &iso_ptr,
                    v8::Local<v8::Context> &ctx)
        : isolate_ptr(iso_ptr), context(iso_ptr->get_v8_isolate(), ctx) {}

    v8::Isolate *get_isolate() const { return isolate_ptr->get_v8_isolate(); }

    v8::Local<v8::Context> get_context() const {
      return v8::Local<v8::Context>::New(isolate_ptr->get_v8_isolate(),
                                         context);
    }
  };

  /**
    Get Auth_id_context representing context for the user account in the
    current connection. Create one if necessary.
  */
  const Auth_id_context *get_or_create_auth_id_context(
      const std::string &auth_id) {
    auto i = m_auth_id_contexts.find(auth_id);
    if (i == m_auth_id_contexts.end()) {
      // Active user didn't run any JS in this connection before.
      // We need to create new Isolate and Context.
      auto isolate_ptr = Js_isolate::create();

      v8::Isolate *isolate = isolate_ptr->get_v8_isolate();

      // Lock the isolate and make it the current one to follow convention.
      v8::Locker locker(isolate);

      v8::Isolate::Scope isolate_scope(isolate);

      v8::HandleScope handle_scope(isolate);

      v8::Local<v8::Context> context = v8::Context::New(isolate);

      auto r = m_auth_id_contexts.try_emplace(auth_id, isolate_ptr, context);

      return &(r.first->second);
    }

    return &(i->second);
  }

  /** Checks if connection executes CREATE FUNCTION or PROCEDURE statement. */
  bool is_create_routine_command() {
    mysql_cstring_with_length sql_command;
    always_ok(mysql_service_mysql_thd_attributes->get(m_thd, "sql_command",
                                                      &sql_command));
    return (strcmp(sql_command.str, "create_procedure") == 0 ||
            strcmp(sql_command.str, "create_spfunction") == 0);
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
  typedef std::function<v8::Local<v8::Value>(v8::Isolate *, uint)>
      get_param_func_t;
  /**
    Get function for getting JS value corresponding to SQL value of
    the specific IN/INOUT parameter of the routine.

    @retval Function wrapped in std::function<> class.
    @retval Empty std::function<> object in case of failure
            (error has been reported already).
  */
  get_param_func_t prepare_get_param_func(uint idx);

  /**
    Type of function for converting JS value and setting it as value of the
    OUT/INOUT parameter or as a returv value of routine.

    These functions take current v8::Context, parameter index and the JS
    value as as arguments. They return false in case of success, and
    true in case of failure (the error has been reported by the function
    in the latter case).
  */
  typedef std::function<bool(v8::Local<v8::Context>, uint,
                             v8::Local<v8::Value>)>
      set_param_func_t;
  /**
    Get function for converting JS value to SQL value and setting it as
    value of the specific OUT/INOUT parameter or as a return value of the
    routine.

    @retval Function wrapped in std::function<> class.
    @retval Empty std::function<> object in case of failure
            (error has been reported already).
  */
  template <class H>
  set_param_func_t prepare_set_param_func(uint idx);

  /**
    Compile wrapper Function object for routine for the specific JS context.

    @param isolate  Isolate in which Function object should be prepared.
    @param context  Context which should be used for preparing Function object.

    @retval Handle for v8::Function object if all went well or empty handle
            in case of failure (the error has been reported already in the
            latter case).
  */
  v8::Local<v8::Function> prepare_func(v8::Isolate *isolate,
                                       v8::Local<v8::Context> &context);

  // Handle for core's stored program object (sp_head).
  stored_program_handle m_sql_sp;
  // Number of routine parameters (cached value from SQL core).
  uint32_t m_arg_count;
  // Routine type. At the moment we only support functions and procedures in JS.
  enum sp_type { FUNCTION, PROCEDURE };
  sp_type m_type;

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
  std::vector<uint16_t> m_out_param_indexes;

  /**
    Map with Function objects for this routine (with user account as a key).

    SQL SECURITY DEFINER routines are always executed under the same account,
    so the are using the same JS context within connection and need only one
    Function object (for connection).

    However, SQL SECURITY INVOKER routines might be executed with different
    user account being active within the same connection. So they might use
    different JS contexts thus require separate Function objects for each
    context/account being used.

    To support correct release of Function handle, we also keep pointer
    to the corresponding Isolate in which this function was allocated.
    Since Js_sp can temporarily outlive Js_thd [sic!] this has to be
    reference counting pointer.
  */
  struct Function_handle_wrapper {
    std::shared_ptr<Js_isolate> isolate_ptr;
    v8::Global<v8::Function> func;

    Function_handle_wrapper(const std::shared_ptr<Js_isolate> &iso_ptr,
                            v8::Isolate *v8_iso, v8::Local<v8::Function> f)
        : isolate_ptr(iso_ptr), func(v8_iso, f) {}
  };
  std::unordered_map<std::string, Function_handle_wrapper> m_funcs;

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

#endif /* COMPONENT_JS_LANG_JS_LANG_CORE_H */
