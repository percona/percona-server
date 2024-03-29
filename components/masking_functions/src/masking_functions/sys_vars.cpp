/* Copyright (c) 2024 Percona LLC and/or its affiliates. All rights reserved.

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

#include "masking_functions/sys_vars.hpp"

#include <mysql/components/component_implementation.h>
#include <mysql/components/services/component_sys_var_service.h>
#include <mysql/components/services/log_builtins.h>

#include <mysqld_error.h>

#include <cstring>
#include <string>

extern REQUIRES_SERVICE_PLACEHOLDER(component_sys_variable_register);
extern REQUIRES_SERVICE_PLACEHOLDER(component_sys_variable_unregister);
extern REQUIRES_SERVICE_PLACEHOLDER(log_builtins);

namespace masking_functions::sys_vars {
namespace {

using str_arg_check_type = STR_CHECK_ARG(str);

constexpr std::string_view component_name{"masking_functions"};
constexpr std::string_view masking_database_var_name{"masking_database"};

std::string default_database_name{"mysql"};

bool is_database_name_initialised = false;

char *database_name;

}  // namespace

std::string_view get_dict_database_name() noexcept { return database_name; }

bool register_sys_vars() {
  str_arg_check_type check_db_name{default_database_name.data()};

  if (mysql_service_component_sys_variable_register->register_variable(
          component_name.data(), masking_database_var_name.data(),
          PLUGIN_VAR_STR | PLUGIN_VAR_MEMALLOC | PLUGIN_VAR_RQCMDARG |
              PLUGIN_VAR_READONLY,
          "Specifies the database to use for data masking dictionaries "
          "at server startup.",
          nullptr, nullptr, static_cast<void *>(&check_db_name),
          static_cast<void *>(&database_name)) != 0) {
    return false;
  }
  is_database_name_initialised = true;

  return true;
}

bool unregister_sys_vars() {
  bool is_success = true;

  if (is_database_name_initialised &&
      mysql_service_component_sys_variable_unregister->unregister_variable(
          component_name.data(), masking_database_var_name.data()) != 0) {
    is_success = false;
  }

  return is_success;
}

bool validate() {
  if (database_name == nullptr || strlen(database_name) == 0) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Bad masking_functions.masking_database value");
    return false;
  }

  return true;
}

}  // namespace masking_functions::sys_vars
