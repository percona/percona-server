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
  This header provides some component-wide declarations which are
  used by several other files in this component.
*/
#ifndef COMPONENT_JS_LANG_JS_LANG_COMMON_H
#define COMPONENT_JS_LANG_JS_LANG_COMMON_H

#include <boost/preprocessor/stringize.hpp>
#include <cassert>
#include <string_view>

/*
  Services and helper headers provided by SQL core which our component uses.
*/
#include <mysql/components/services/bits/stored_program_bits.h>
#include <mysql/components/services/dynamic_privilege.h>
#include <mysql/components/services/mysql_current_thread_reader.h>
#include <mysql/components/services/mysql_runtime_error_service.h>
#include <mysql/components/services/mysql_stored_program.h>
#include <mysql/components/services/mysql_string.h>
#include <mysql/components/services/mysql_thd_attributes.h>
#include <mysql/components/services/mysql_thd_store_service.h>
#include <mysql/components/services/security_context.h>
#include <mysql/mysql_lex_string.h>
#include <mysqld_error.h>

/*
  Placeholders for services from SQL core used by our component.
*/
extern REQUIRES_SERVICE_PLACEHOLDER(dynamic_privilege_register);
extern REQUIRES_SERVICE_PLACEHOLDER(global_grants_check);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_charset);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_current_thread_reader);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_runtime_error);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_security_context_options);
extern REQUIRES_SERVICE_PLACEHOLDER(
    mysql_stored_program_argument_metadata_query);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_stored_program_metadata_query);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_stored_program_return_metadata_query);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_stored_program_return_value_float);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_stored_program_return_value_int);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_stored_program_return_value_null);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_stored_program_return_value_string);
extern REQUIRES_SERVICE_PLACEHOLDER(
    mysql_stored_program_return_value_unsigned_int);
extern REQUIRES_SERVICE_PLACEHOLDER(
    mysql_stored_program_runtime_argument_float);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_stored_program_runtime_argument_int);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_stored_program_runtime_argument_null);
extern REQUIRES_SERVICE_PLACEHOLDER(
    mysql_stored_program_runtime_argument_string);
extern REQUIRES_SERVICE_PLACEHOLDER(
    mysql_stored_program_runtime_argument_unsigned_int);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_string_charset_converter);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_string_copy_converter);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_string_factory);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_string_get_data_in_charset);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_thd_attributes);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_thd_security_context);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_thd_store);

/**
  Helper function which is used to absorb results of service calls
  which are not supposed to fail.

  @sa Note about our approach to error handling in  js_lang_core.cc.
*/
static void always_ok(bool service_result) { assert(!service_result); }

/*
  Some global constants and defines used through the component.
*/

// defined as a macro because needed both raw and stringized
#define CURRENT_COMPONENT_NAME js_lang
#define CURRENT_COMPONENT_NAME_STR BOOST_PP_STRINGIZE(COMPONENT_NAME)

// Name/Identifier of the language this component implements.
static constexpr const char *LANGUAGE_NAME = "JS";

// Name of global privilege required from user creating JS routine
// in addition to usual CREATE ROUTINE privilege on the schema.
static constexpr std::string_view CREATE_PRIVILEGE_NAME = "CREATE_JS_ROUTINE";

#endif /* COMPONENT_JS_LANG_JS_LANG_COMMON_H */
