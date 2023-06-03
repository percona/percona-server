/* Copyright (c) 2018, 2019 Francisco Miguel Biete Banon. All rights reserved.
   Copyright (c) 2023 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   51 Franklin Street, Suite 500, Boston, MA 02110-1335 USA */

#include "components/masking_functions/include/component.h"
#include "components/masking_functions/include/udf/udf_registration.h"
#include "components/masking_functions/include/udf/udf_utils.h"

#include <boost/preprocessor/stringize.hpp>

#include <mysql/components/component_implementation.h>

#include <mysql/components/services/component_sys_var_service.h>
#include <mysql/components/services/dynamic_privilege.h>
#include <mysql/components/services/mysql_command_services.h>
#include <mysql/components/services/mysql_current_thread_reader.h>
#include <mysql/components/services/mysql_runtime_error.h>
#include <mysql/components/services/mysql_string.h>
#include <mysql/components/services/security_context.h>
#include <mysql/components/services/udf_metadata.h>
#include <mysql/components/services/udf_registration.h>

// defined as a macro because needed both raw and stringized
#define CURRENT_COMPONENT_NAME masking_functions
#define CURRENT_COMPONENT_NAME_STR BOOST_PP_STRINGIZE(CURRENT_COMPONENT_NAME)

REQUIRES_SERVICE_PLACEHOLDER(udf_registration);
REQUIRES_SERVICE_PLACEHOLDER(mysql_udf_metadata);
REQUIRES_SERVICE_PLACEHOLDER(mysql_runtime_error);
REQUIRES_SERVICE_PLACEHOLDER(mysql_string_converter);
REQUIRES_SERVICE_PLACEHOLDER(mysql_string_character_access);
REQUIRES_SERVICE_PLACEHOLDER(mysql_string_factory);
REQUIRES_SERVICE_PLACEHOLDER(mysql_current_thread_reader);
REQUIRES_SERVICE_PLACEHOLDER(dynamic_privilege_register);
REQUIRES_SERVICE_PLACEHOLDER(global_grants_check);

REQUIRES_SERVICE_PLACEHOLDER_AS(mysql_thd_security_context, thd_security_ctx);
REQUIRES_SERVICE_PLACEHOLDER_AS(mysql_account_database_security_context_lookup,
                                account_db_security_ctx_lookup);
REQUIRES_SERVICE_PLACEHOLDER_AS(mysql_security_context_options,
                                security_ctx_options);

REQUIRES_SERVICE_PLACEHOLDER_AS(mysql_command_options, cmd_options_srv);
REQUIRES_SERVICE_PLACEHOLDER_AS(mysql_command_factory, cmd_factory_srv);
REQUIRES_SERVICE_PLACEHOLDER_AS(mysql_command_query, cmd_query_srv);
REQUIRES_SERVICE_PLACEHOLDER_AS(mysql_command_error_info, cmd_error_info_srv);
REQUIRES_SERVICE_PLACEHOLDER_AS(mysql_command_query_result,
                                cmd_query_result_srv);

constexpr std::string_view privilege_name = "MASKING_DICTIONARIES_ADMIN";

static mysql_service_status_t component_init() {
  DBUG_TRACE;

  // reg_srv = mysql_plugin_registry_acquire();

  // if (mysql::plugins::Charset_service::init(reg_srv)) return 1;

  sql_print_information(
      "Masking Functions Component: Initializing component");

  mysql_service_dynamic_privilege_register->register_privilege(
      privilege_name.data(), privilege_name.size());
  register_udfs();

  return 0;
}

static mysql_service_status_t component_deinit() {
  DBUG_TRACE;

  sql_print_information("Masking Function Component: Deinitializing component");

  unregister_udfs();

  mysql_service_dynamic_privilege_register->register_privilege(
      privilege_name.data(), privilege_name.size());

  return 0;
}

void my_error(int error_id, myf flags, ...) {
  va_list args;
  va_start(args, flags);
  mysql_service_mysql_runtime_error->emit(error_id, flags, args);
  va_end(args);
}

// clang-format off
BEGIN_COMPONENT_PROVIDES(CURRENT_COMPONENT_NAME)
END_COMPONENT_PROVIDES();

BEGIN_COMPONENT_REQUIRES(CURRENT_COMPONENT_NAME)
  REQUIRES_SERVICE(mysql_runtime_error),
  REQUIRES_SERVICE(udf_registration),
  REQUIRES_SERVICE(mysql_udf_metadata),
  REQUIRES_SERVICE(mysql_string_converter),
  REQUIRES_SERVICE(mysql_string_character_access),
  REQUIRES_SERVICE(mysql_string_factory),
  REQUIRES_SERVICE(dynamic_privilege_register),

  REQUIRES_SERVICE_AS(mysql_thd_security_context, thd_security_ctx),
  REQUIRES_SERVICE_AS(mysql_account_database_security_context_lookup, account_db_security_ctx_lookup),
  REQUIRES_SERVICE_AS(mysql_security_context_options, security_ctx_options),

  REQUIRES_SERVICE_AS(mysql_command_options, cmd_options_srv),
  REQUIRES_SERVICE_AS(mysql_command_factory, cmd_factory_srv),
  REQUIRES_SERVICE_AS(mysql_command_query, cmd_query_srv),
  REQUIRES_SERVICE_AS(mysql_command_error_info, cmd_error_info_srv),
  REQUIRES_SERVICE_AS(mysql_command_query_result,
                                cmd_query_result_srv),
  REQUIRES_SERVICE(mysql_current_thread_reader),
  REQUIRES_SERVICE(global_grants_check),
END_COMPONENT_REQUIRES();

BEGIN_COMPONENT_METADATA(CURRENT_COMPONENT_NAME)
  METADATA("mysql.author", "Percona Corporation"),
  METADATA("mysql.license", "GPL"),
END_COMPONENT_METADATA();

DECLARE_COMPONENT(CURRENT_COMPONENT_NAME, CURRENT_COMPONENT_NAME_STR)
  component_init,
  component_deinit,
END_DECLARE_COMPONENT();
// clang-format on

DECLARE_LIBRARY_COMPONENTS &COMPONENT_REF(CURRENT_COMPONENT_NAME)
    END_DECLARE_LIBRARY_COMPONENTS
