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
  This component adds to Percona Server for MySQL support of Stored Functions
  and Procedures written in JS language or, more formally, ECMAScript by
  employing Google's V8 engine.

  See README.md for more details.

  This file only contains code which makes our implementation of support for JS
  routines a component. I.e. code related to component definition, its
  initialization/shutdown, preparing for usage of SQL core services our code
  depends on, some glue code exposing our JS support as a service.

  The central part of our implementation of support for JS routines can be
  found in js_lang_core.cc/.h.
*/

// Service that our component implements.
#include <mysql/components/services/language_service.h>

#include <mysql/components/component_implementation.h>

#include "js_lang_common.h"
#include "js_lang_core.h"

REQUIRES_SERVICE_PLACEHOLDER(dynamic_privilege_register);
REQUIRES_SERVICE_PLACEHOLDER(global_grants_check);
REQUIRES_SERVICE_PLACEHOLDER(mysql_charset);
REQUIRES_SERVICE_PLACEHOLDER(mysql_current_thread_reader);
REQUIRES_SERVICE_PLACEHOLDER(mysql_runtime_error);
REQUIRES_SERVICE_PLACEHOLDER(mysql_security_context_options);
REQUIRES_SERVICE_PLACEHOLDER(mysql_stored_program_argument_metadata_query);
REQUIRES_SERVICE_PLACEHOLDER(mysql_stored_program_metadata_query);
REQUIRES_SERVICE_PLACEHOLDER(mysql_stored_program_return_metadata_query);
REQUIRES_SERVICE_PLACEHOLDER(mysql_stored_program_return_value_float);
REQUIRES_SERVICE_PLACEHOLDER(mysql_stored_program_return_value_int);
REQUIRES_SERVICE_PLACEHOLDER(mysql_stored_program_return_value_null);
REQUIRES_SERVICE_PLACEHOLDER(mysql_stored_program_return_value_string);
REQUIRES_SERVICE_PLACEHOLDER(mysql_stored_program_return_value_unsigned_int);
REQUIRES_SERVICE_PLACEHOLDER(mysql_stored_program_runtime_argument_float);
REQUIRES_SERVICE_PLACEHOLDER(mysql_stored_program_runtime_argument_int);
REQUIRES_SERVICE_PLACEHOLDER(mysql_stored_program_runtime_argument_null);
REQUIRES_SERVICE_PLACEHOLDER(mysql_stored_program_runtime_argument_string);
REQUIRES_SERVICE_PLACEHOLDER(
    mysql_stored_program_runtime_argument_unsigned_int);
REQUIRES_SERVICE_PLACEHOLDER(mysql_string_charset_converter);
REQUIRES_SERVICE_PLACEHOLDER(mysql_string_copy_converter);
REQUIRES_SERVICE_PLACEHOLDER(mysql_string_factory);
REQUIRES_SERVICE_PLACEHOLDER(mysql_string_get_data_in_charset);
REQUIRES_SERVICE_PLACEHOLDER(mysql_thd_attributes);
REQUIRES_SERVICE_PLACEHOLDER(mysql_thd_security_context);
REQUIRES_SERVICE_PLACEHOLDER(mysql_thd_store);

/**
  Implementation of External Program Capability Query and External Program
  Execution services.
*/
class Js_lang_service_imp {
 public:
  /**
    Implementation of external_program_capability_query::get() method by our
    component.

    Reports to SQL core that we support "JS" language for external stored
    programs and only it. Fails if queried about any other capabilities.

    @retval False - Success. This was a request if specific language is
                    supported. "*(bool*)value" is set to indicate whether
                    language which name was passed as "property" is
                    supported.
    @retval True  - Failure due to unsupported capability query.
                    Error has been reported.
  */
  static DEFINE_BOOL_METHOD(get, (const char *capability, char *property,
                                  void *value)) {
    // Mimic mysql_stored_program_* services which use case-sensitive
    // comparisons for string key names.
    if (strcmp(capability, "supports_language") != 0) {
      my_error(ER_LANGUAGE_COMPONENT, MYF(0),
               "Unknown capability request from " CURRENT_COMPONENT_NAME_STR
               " component.");
      return true;
    }

    /*
      Use case-insensitive comparison for language name, this is more
      user-friendly and consistent with data-dictionary layer behavior,
      which uppercases external language name before storing it in
      'routines' table.
    */
    *reinterpret_cast<bool *>(value) =
        (strcasecmp(property, LANGUAGE_NAME) == 0);

    return false;
  }

  /**
    Implementation of external_program_execution::init() method by our
    component.

    Create and initialize component's counterpart for sp_head object for
    JS routine (fail for other languages).

    @retval False - Success.
    @retval True  - Failure (Wrong language or routine type, caller is
                    responsible for reporting an error).
  */
  static DEFINE_BOOL_METHOD(init, (stored_program_handle sp,
                                   stored_program_statement_handle,
                                   external_program_handle *lang_sp)) {
    /*
      TODO: Note that if we are to support simultaneous use of several
      components implementing support for different languages, we will need a
      special router component which will serve as default implementation of
      language service API, and route calls among other implementations
      according to language passed to it (e.g. by keeping language => component
      mapping).

      At the moment we simply check that we are not called for routine in
      language other than one this component implements.

      Note that for "init" method caller invokes my_error() in case of failure
      (unlike for "parse" and "execute" methods).
    */
    mysql_cstring_with_length language;
    always_ok(mysql_service_mysql_stored_program_metadata_query->get(
        sp, "sp_language", &language));

    if (strcasecmp(language.str, LANGUAGE_NAME) != 0) {
      // We do not support this language.
      return true;
    }

    uint16_t sql_sp_type;
    always_ok(mysql_service_mysql_stored_program_metadata_query->get(
        sp, "sp_type", &sql_sp_type));

    if (!(sql_sp_type == MYSQL_STORED_PROGRAM_DATA_QUERY_TYPE_FUNCTION ||
          sql_sp_type == MYSQL_STORED_PROGRAM_DATA_QUERY_TYPE_PROCEDURE)) {
      /*
        Even though at the moment SQL-layer doesn't allow stored programs
        other than Functions and Procedures in external language, we still
        play safe and fail in case of attempt to create usupported type
        of program (e.g. trigger) in JS.
      */
      return true;
    }

    *lang_sp =
        reinterpret_cast<external_program_handle>(new Js_sp(sp, sql_sp_type));
    return false;
  }

  /**
    Implementation of external_program_execution::deinit() method by our
    component.

    Destroy component's counterpart for sp_head object for JS routine.
  */
  static DEFINE_BOOL_METHOD(deinit, (MYSQL_THD, external_program_handle lang_sp,
                                     stored_program_handle)) {
    delete reinterpret_cast<Js_sp *>(lang_sp);
    return false;
  }

  /**
    Implementation of external_program_execution::parse() method by our
    component.

    Parse JS routine and do some preparations for its execution.

    @retval False - Success.
    @retval True  - Failure (error has been reported).
  */
  static DEFINE_BOOL_METHOD(parse, (external_program_handle lang_sp,
                                    stored_program_statement_handle)) {
    Js_sp *sp = reinterpret_cast<Js_sp *>(lang_sp);
    return sp->parse();
  }

  /**
    Implementation of external_program_execution::execute() method by our
    component.

    Execute JS routine (which was parsed and prepared for execution earlier).

    @retval False - Success.
    @retval True  - Failure (error has been reported).
  */
  static DEFINE_BOOL_METHOD(execute, (external_program_handle lang_sp,
                                      stored_program_statement_handle)) {
    Js_sp *sp = reinterpret_cast<Js_sp *>(lang_sp);
    return sp->execute();
  }
};

static mysql_service_status_t component_init() {
  /*
    First, let us try to do things which can fail, at least in theory.

    V8 doesn't support V8::Dispose() -> V8::Initialize() state transition,
    so we have to block UNISTALL COMPONENT -> INSTALL COMPONENT case.
  */
  if (Js_isolate::is_used_or_v8_shutdown()) {
    my_error(
        ER_LANGUAGE_COMPONENT, MYF(0),
        "Re-installing the component without server restart is not supported.");
    return 1;
  }

  // Play safe, even though the below can't fail at the moment.
  if (register_create_privilege()) return 1;

  // Registering slot in THD for the component always succeeds (unless OOM)!
  Js_thd::register_slot();

  // Prepare V8 for usage.
  Js_isolate::v8_init();

  return 0;
}

static mysql_service_status_t component_deinit() {
  /*
    Block component shutdown (and V8 deinitialization specifically),
    if there are outstanding Isolates around.

    Note that component shutdown can't happen concurrently with a call
    to any of methods provided by external_program_execution service.
    This is ensured by the fact that these methods are always called
    using my_service wrapper, which does service acquire/release, and
    the fact that component infrastructure blocks unloading of component
    while any service provided by it is acquired (i.e. does its own
    reference counting).

    So the goal of below check is to block component unloading between
    calls to methods of this service, until connections which used
    our component at some point are closed/THDs with associated Js_thd
    contexts/isolates are gone.
  */
  if (Js_isolate::is_used_or_v8_shutdown()) {
    my_error(ER_LANGUAGE_COMPONENT_CANNOT_UNINSTALL, MYF(0));
    return 1;
  }

  // Play safe, even though the below can't fail at the moment.
  if (unregister_create_privilege()) return 1;

  Js_thd::unregister_slot();

  // Shutdown V8.
  Js_isolate::v8_shutdown();

  return 0;
}

// clang-format off
BEGIN_SERVICE_IMPLEMENTATION(js_lang, external_program_capability_query)
  Js_lang_service_imp::get,
END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(js_lang, external_program_execution)
  Js_lang_service_imp::init,
  Js_lang_service_imp::deinit,
  Js_lang_service_imp::parse,
  Js_lang_service_imp::execute,
END_SERVICE_IMPLEMENTATION();

BEGIN_COMPONENT_PROVIDES(js_lang)
  PROVIDES_SERVICE(js_lang, external_program_capability_query),
  PROVIDES_SERVICE(js_lang, external_program_execution),
END_COMPONENT_PROVIDES();

BEGIN_COMPONENT_REQUIRES(js_lang)
  REQUIRES_SERVICE(dynamic_privilege_register),
  REQUIRES_SERVICE(global_grants_check),
  REQUIRES_SERVICE(mysql_charset),
  REQUIRES_SERVICE(mysql_current_thread_reader),
  REQUIRES_SERVICE(mysql_runtime_error),
  REQUIRES_SERVICE(mysql_security_context_options),
  REQUIRES_SERVICE(mysql_stored_program_argument_metadata_query),
  REQUIRES_SERVICE(mysql_stored_program_metadata_query),
  REQUIRES_SERVICE(mysql_stored_program_return_metadata_query),
  REQUIRES_SERVICE(mysql_stored_program_return_value_float),
  REQUIRES_SERVICE(mysql_stored_program_return_value_int),
  REQUIRES_SERVICE(mysql_stored_program_return_value_null),
  REQUIRES_SERVICE(mysql_stored_program_return_value_string),
  REQUIRES_SERVICE(mysql_stored_program_return_value_unsigned_int),
  REQUIRES_SERVICE(mysql_stored_program_runtime_argument_float),
  REQUIRES_SERVICE(mysql_stored_program_runtime_argument_int),
  REQUIRES_SERVICE(mysql_stored_program_runtime_argument_null),
  REQUIRES_SERVICE(mysql_stored_program_runtime_argument_string),
  REQUIRES_SERVICE(mysql_stored_program_runtime_argument_unsigned_int),
  REQUIRES_SERVICE(mysql_string_charset_converter),
  REQUIRES_SERVICE(mysql_string_copy_converter),
  REQUIRES_SERVICE(mysql_string_factory),
  REQUIRES_SERVICE(mysql_string_get_data_in_charset),
  REQUIRES_SERVICE(mysql_thd_attributes),
  REQUIRES_SERVICE(mysql_thd_security_context),
  REQUIRES_SERVICE(mysql_thd_store),
END_COMPONENT_REQUIRES();

BEGIN_COMPONENT_METADATA(js_lang)
  METADATA("mysql.author", "Percona Corporation"),
  METADATA("mysql.license", "GPL"),
END_COMPONENT_METADATA();

DECLARE_COMPONENT(js_lang, CURRENT_COMPONENT_NAME_STR)
  component_init,
  component_deinit,
END_DECLARE_COMPONENT();

DECLARE_LIBRARY_COMPONENTS
  &COMPONENT_REF(js_lang)
END_DECLARE_LIBRARY_COMPONENTS
    // clang-format on
