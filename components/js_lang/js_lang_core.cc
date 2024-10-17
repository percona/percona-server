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

#include <libplatform/libplatform.h>

#include "js_lang_common.h"

std::unique_ptr<v8::Platform> platform;

std::atomic<int> Js_isolate::s_count = 0;

std::shared_ptr<Js_isolate> Js_isolate::create() {
  v8::Isolate::CreateParams create_params;
  v8::Isolate *isolate;

  // Increment global Isolate reference counter to block component
  // shutdown while we creating Isolate and as long as it is around.
  //
  // Component infrastructure ensures that methods of language service and
  // thus this code can't be called after component shutdown starts.
  assert(s_count.load() >= 0);
  ++s_count;

  create_params.array_buffer_allocator =
      v8::ArrayBuffer::Allocator::NewDefaultAllocator();

  isolate = v8::Isolate::New(create_params);
  return std::make_shared<Js_isolate>(isolate,
                                      create_params.array_buffer_allocator);
}

void Js_isolate::v8_init() {
  // We would like to enforce strict mode for code of JS routines right
  // from the start.
  v8::V8::SetFlagsFromString("--use_strict");

  // v8::V8::InitializeICUDefaultLocation(argv[0]);
  // v8::V8::InitializeExternalStartupData(argv[0]);
  platform = v8::platform::NewDefaultPlatform();
  v8::V8::InitializePlatform(platform.get());
  // The below call will simply crash the process if something is wrong.
  (void)v8::V8::Initialize();
}

void Js_isolate::v8_shutdown() {
  // We should have checked that there are no used isolates around earlier.
  assert(s_count.load() == 0);

  // Set flag indicating that component shutdown has been started for
  // debug/assertion purposes.
  //
  // We also piggy-back on this flag to block attempts to install this
  // component without server restart again. It is not possible due to
  // V8 not supporting re-initialization.
  s_count.store(-1);

  // Shutdown V8.
  v8::V8::Dispose();
  v8::V8::DisposePlatform();
  platform.reset();
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

Js_sp::Js_sp(stored_program_handle sp, uint16_t sql_sp_type) : m_sql_sp(sp) {
  assert(sql_sp_type == MYSQL_STORED_PROGRAM_DATA_QUERY_TYPE_FUNCTION ||
         sql_sp_type == MYSQL_STORED_PROGRAM_DATA_QUERY_TYPE_PROCEDURE);
  m_type = (sql_sp_type == MYSQL_STORED_PROGRAM_DATA_QUERY_TYPE_FUNCTION)
               ? Js_sp::FUNCTION
               : Js_sp::PROCEDURE;

  always_ok(mysql_service_mysql_stored_program_metadata_query->get(
      sp, "argument_count", &m_arg_count));
}

Js_sp::~Js_sp() {
  for (auto &p : m_funcs) {
    v8::Locker locker(p.second.isolate_ptr->get_v8_isolate());
    p.second.func.Reset();
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

  for (uint i = 0; i < m_arg_count; ++i) {
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

  m_wrapped_body.append(body.str);

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

v8::Local<v8::Function> Js_sp::prepare_func(v8::Isolate *isolate,
                                            v8::Local<v8::Context> &context) {
  v8::EscapableHandleScope handle_scope(isolate);

  v8::TryCatch try_catch(isolate);

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

  if (!v8::Script::Compile(context, source).ToLocal(&script)) {
    v8::String::Utf8Value exception(isolate, try_catch.Exception());
    my_error(ER_LANGUAGE_COMPONENT, MYF(0),
             *exception ? *exception : "Unknown compilation error.");
    return v8::Local<v8::Function>();
  }

  v8::Local<v8::Value> func_result;
  if (!script->Run(context).ToLocal(&func_result)) {
    v8::String::Utf8Value exception(isolate, try_catch.Exception());
    my_error(ER_LANGUAGE_COMPONENT, MYF(0),
             *exception ? *exception : "Unknown wrapper preparation error.");
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

  v8::Isolate *isolate = auth_id_ctx->get_isolate();

  // Lock the isolate to follow convention.
  v8::Locker locker(isolate);

  // Make this isolate the current one.
  v8::Isolate::Scope isolate_scope(isolate);

  // Create a stack-allocated handle scope.
  v8::HandleScope handle_scope(isolate);

  v8::Local<v8::Context> context = auth_id_ctx->get_context();

  // Enter the context for compiling and getting the wrapper function.
  v8::Context::Scope context_scope(context);

  /*
    Compile and execute script to get wrapper function object. Later we
    will use this function's Call() method for running our routine.

    For SQL SECURITY DEFINER routines this will be the only Function
    object we will use.
    SQL SECURITY INVOKER routines might be called with different user
    account active within the same connection. So we might need to build
    more Function objects for this account's JS context.
  */
  v8::Local<v8::Function> func = prepare_func(isolate, context);

  if (func.IsEmpty()) return true;

  m_funcs.try_emplace(auth_id, auth_id_ctx->isolate_ptr, isolate, func);

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

  v8::Isolate *isolate = auth_id_ctx->get_isolate();

  v8::Locker locker(isolate);

  // Make this isolate the current one.
  v8::Isolate::Scope isolate_scope(isolate);

  // Create a stack-allocated handle scope.
  v8::HandleScope handle_scope(isolate);

  v8::Local<v8::Context> context = auth_id_ctx->get_context();

  // Enter the context for executing the function.
  v8::Context::Scope context_scope(context);

  /*
    Get Function object for the routine in this context.

    Again, object for active account and its context might be missing
    in case of SQL SECURITY INVOKER routine, so prepare a new one if
    necessary.
  */
  v8::Local<v8::Function> func;

  auto j = m_funcs.find(auth_id);
  if (j == m_funcs.end()) {
    func = prepare_func(isolate, context);
    if (func.IsEmpty()) return true;
    m_funcs.try_emplace(auth_id, auth_id_ctx->isolate_ptr, isolate, func);
  } else {
    func = v8::Local<v8::Function>::New(isolate, j->second.func);
  }

  v8::TryCatch try_catch(isolate);

  // Build array with program parameters.
  v8::Local<v8::Value> *arg_arr = nullptr;

  if (m_arg_count) {
    arg_arr = new v8::Local<v8::Value>[m_arg_count];
    for (uint i = 0; i < m_arg_count; ++i) {
      arg_arr[i] = m_get_param_func[i](isolate, i);

      if (arg_arr[i].IsEmpty()) {
        // It is responsibility of get_param_func function to
        // report error to user.
        delete[] arg_arr;
        return true;
      }
    }
  }

  // Call the wrapper function passing parameters to it.
  v8::Local<v8::Value> result;
  if (!func->Call(context, context->Global(), m_arg_count, arg_arr)
           .ToLocal(&result)) {
    v8::String::Utf8Value exception(isolate, try_catch.Exception());
    my_error(ER_LANGUAGE_COMPONENT, MYF(0),
             *exception ? *exception : "Unknown execution error");
    delete[] arg_arr;
    return true;
  }
  delete[] arg_arr;

  if (m_type == Js_sp::FUNCTION) {
    if (m_set_retval_func(context, 0, result)) return true;
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
      uint res_arr_idx = 1;
      for (auto param_idx : m_out_param_indexes) {
        if (m_set_out_param_func[res_arr_idx - 1](
                context, param_idx,
                // The array returned must have an element for each OUT param.
                arr_result->Get(context, res_arr_idx).ToLocalChecked())) {
          // OUT param setter function is responsible for reporting error
          // in case of its failure.
          return true;
        }
        ++res_arr_idx;
      }
    }
  }

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
