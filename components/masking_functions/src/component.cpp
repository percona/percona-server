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

#include <cassert>
#include <cstdarg>
#include <exception>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include <boost/preprocessor/stringize.hpp>

#include <my_dbug.h>
#include <my_inttypes.h>
#include <mysqld_error.h>

#include <mysql/my_loglevel.h>

#include <mysql/components/component_implementation.h>
#include <mysql/components/service.h>

#include <mysql/components/services/component_sys_var_service.h>  // IWYU pragma: keep
#include <mysql/components/services/log_builtins.h>
#include <mysql/components/services/mysql_command_services.h>  // IWYU pragma: keep
#include <mysql/components/services/mysql_current_thread_reader.h>  // IWYU pragma: keep
#include <mysql/components/services/mysql_runtime_error.h>
#include <mysql/components/services/mysql_string.h>  // IWYU pragma: keep
#include <mysql/components/services/psi_thread.h>
#include <mysql/components/services/security_context.h>  // IWYU pragma: keep
#include <mysql/components/services/udf_metadata.h>      // IWYU pragma: keep
#include <mysql/components/services/udf_registration.h>  // IWYU pragma: keep

#include <mysqlpp/udf_error_reporter.hpp>

#include "sql/debug_sync.h"  // IWYU pragma: keep

#include "masking_functions/command_service_tuple.hpp"
#include "masking_functions/component_sys_variable_service_tuple.hpp"
#include "masking_functions/default_sql_context_builder.hpp"
#include "masking_functions/dictionary_flusher_thread.hpp"
#include "masking_functions/primitive_singleton.hpp"
#include "masking_functions/query_builder.hpp"
#include "masking_functions/registration_routines.hpp"
#include "masking_functions/string_service_tuple.hpp"
#include "masking_functions/sys_vars.hpp"
#include "masking_functions/term_cache.hpp"
#include "masking_functions/term_cache_core.hpp"

// defined as a macro because needed both raw and stringized
#define CURRENT_COMPONENT_NAME masking_functions
#define CURRENT_COMPONENT_NAME_STR BOOST_PP_STRINGIZE(CURRENT_COMPONENT_NAME)

REQUIRES_SERVICE_PLACEHOLDER(mysql_charset);
REQUIRES_SERVICE_PLACEHOLDER(mysql_string_factory);
REQUIRES_SERVICE_PLACEHOLDER(mysql_string_charset_converter);
REQUIRES_SERVICE_PLACEHOLDER(mysql_string_get_data_in_charset);
REQUIRES_SERVICE_PLACEHOLDER(mysql_string_append);
REQUIRES_SERVICE_PLACEHOLDER(mysql_string_character_access);
REQUIRES_SERVICE_PLACEHOLDER(mysql_string_byte_access);
REQUIRES_SERVICE_PLACEHOLDER(mysql_string_reset);
REQUIRES_SERVICE_PLACEHOLDER(mysql_string_substr);
REQUIRES_SERVICE_PLACEHOLDER(mysql_string_compare);

REQUIRES_SERVICE_PLACEHOLDER(mysql_command_query);
REQUIRES_SERVICE_PLACEHOLDER(mysql_command_query_result);
REQUIRES_SERVICE_PLACEHOLDER(mysql_command_field_info);
REQUIRES_SERVICE_PLACEHOLDER(mysql_command_options);
REQUIRES_SERVICE_PLACEHOLDER(mysql_command_factory);
REQUIRES_SERVICE_PLACEHOLDER(mysql_command_error_info);
REQUIRES_SERVICE_PLACEHOLDER(mysql_command_thread);

REQUIRES_PSI_THREAD_SERVICE_PLACEHOLDER;

REQUIRES_SERVICE_PLACEHOLDER(udf_registration);
REQUIRES_SERVICE_PLACEHOLDER(dynamic_privilege_register);

REQUIRES_SERVICE_PLACEHOLDER(mysql_udf_metadata);

REQUIRES_SERVICE_PLACEHOLDER(mysql_current_thread_reader);
REQUIRES_SERVICE_PLACEHOLDER(mysql_thd_security_context);
REQUIRES_SERVICE_PLACEHOLDER(global_grants_check);
REQUIRES_SERVICE_PLACEHOLDER(component_sys_variable_register);
REQUIRES_SERVICE_PLACEHOLDER(component_sys_variable_unregister);

REQUIRES_SERVICE_PLACEHOLDER(log_builtins);
REQUIRES_SERVICE_PLACEHOLDER(log_builtins_string);

REQUIRES_SERVICE_PLACEHOLDER(mysql_runtime_error);

SERVICE_TYPE(log_builtins) * log_bi;
SERVICE_TYPE(log_builtins_string) * log_bs;

namespace {

mysql_service_status_t component_init();
mysql_service_status_t component_deinit();

void masking_functions_my_error(int error_id, myf flags, ...) {
  va_list args;
  va_start(args, flags);
  mysql_service_mysql_runtime_error->emit(error_id, flags, args);
  va_end(args);
}

mysql_service_status_t component_init() {
  mysql_service_status_t initialization_result{0};

  log_bi = mysql_service_log_builtins;
  log_bs = mysql_service_log_builtins_string;

  masking_functions::primitive_singleton<
      masking_functions::string_service_tuple>::instance() =
      masking_functions::string_service_tuple{
          // TODO: convert this to designated initializers in c++20
          mysql_service_mysql_charset,
          mysql_service_mysql_string_factory,
          mysql_service_mysql_string_charset_converter,
          mysql_service_mysql_string_get_data_in_charset,
          mysql_service_mysql_string_append,
          mysql_service_mysql_string_character_access,
          mysql_service_mysql_string_byte_access,
          mysql_service_mysql_string_reset,
          mysql_service_mysql_string_substr,
          mysql_service_mysql_string_compare};
  masking_functions::primitive_singleton<
      masking_functions::command_service_tuple>::instance() =
      masking_functions::command_service_tuple{
          // TODO: convert this to designated initializers in c++20
          mysql_service_mysql_command_query,
          mysql_service_mysql_command_query_result,
          mysql_service_mysql_command_field_info,
          mysql_service_mysql_command_options,
          mysql_service_mysql_command_factory,
          mysql_service_mysql_command_error_info,
          mysql_service_mysql_command_thread};
  masking_functions::primitive_singleton<
      masking_functions::component_sys_variable_service_tuple>::instance() =
      masking_functions::component_sys_variable_service_tuple{
          // TODO: convert this to designated initializers in c++20
          mysql_service_component_sys_variable_register,
          mysql_service_component_sys_variable_unregister};

  // here we use a custom error reporting function
  // 'masking_functions_my_error()' based on the
  // 'mysql_service_mysql_runtime_error' service instead of the standard
  // 'my_error()' from 'mysys' to get rid of the 'mysys' dependency for this
  // component
  mysqlpp::udf_error_reporter::instance() = &masking_functions_my_error;

  try {
    if (!masking_functions::register_dynamic_privileges()) {
      throw std::runtime_error{"Cannot register dynamic privilege"};
    }

    if (!masking_functions::register_sys_vars()) {
      throw std::runtime_error{"Cannot register system variables"};
    }

    std::string check_error_message;
    if (!masking_functions::check_sys_vars(check_error_message)) {
      throw std::runtime_error{check_error_message};
    }

    if (!masking_functions::register_udfs()) {
      throw std::runtime_error{"Cannot register UDFs"};
    }

    auto sql_query_builder{std::make_unique<masking_functions::query_builder>(
        masking_functions::get_dict_database_name())};
    masking_functions::primitive_singleton<
        masking_functions::query_builder_ptr>::instance() =
        std::move(sql_query_builder);

    const auto &command_services = masking_functions::primitive_singleton<
        masking_functions::command_service_tuple>::instance();

    auto default_sql_ctx_builder{
        std::make_shared<masking_functions::default_sql_context_builder>(
            command_services)};

    // here we create an instance of the 'term_cache_core' class that will
    // be shared between 'primary_cache' (an instance of the 'term_cache'
    // created with this core and 'default_sql_context_builder') and another
    // instance of the 'term_cache' class created inside background thread
    // handler function with this core and 'static_sql_context_builder'
    auto cache_core{std::make_shared<masking_functions::term_cache_core>()};

    auto primary_cache{std::make_unique<masking_functions::term_cache>(
        cache_core, default_sql_ctx_builder)};
    masking_functions::primitive_singleton<
        masking_functions::term_cache_ptr>::instance() =
        std::move(primary_cache);

    const auto flush_interval_seconds{
        masking_functions::get_flush_interval_seconds()};
    if (flush_interval_seconds > 0U) {
      auto flusher{
          std::make_unique<masking_functions::dictionary_flusher_thread>(
              cache_core, flush_interval_seconds)};

      masking_functions::primitive_singleton<
          masking_functions::dictionary_flusher_thread_ptr>::instance() =
          std::move(flusher);

      // NOLINTNEXTLINE(cppcoreguidelines-avoid-do-while)
      DBUG_EXECUTE_IF("enable_masking_functions_flusher_create_sync", {
        MYSQL_THD extracted_thd{nullptr};
        mysql_service_mysql_current_thread_reader->get(&extracted_thd);
        assert(extracted_thd != nullptr);
        DEBUG_SYNC(extracted_thd, "masking_functions_after_flusher_create");
      });
    }

    LogComponentErr(INFORMATION_LEVEL, ER_LOG_PRINTF_MSG,
                    "Component successfully initialized");
  } catch (const std::exception &e) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, e.what());
    component_deinit();
    initialization_result = 1;
  }
  return initialization_result;
}

mysql_service_status_t component_deinit() {
  int result = 0;
  auto &flusher{masking_functions::primitive_singleton<
      masking_functions::dictionary_flusher_thread_ptr>::instance()};

  // the destruction of the 'flusher' object will also trigger graceful
  // background thread termination ('dictionary_flusher_thread' destructor
  // will set its state to 'stopped' and then will join the thread)
  flusher.reset();

  masking_functions::primitive_singleton<
      masking_functions::term_cache_ptr>::instance()
      .reset();

  if (!masking_functions::unregister_udfs()) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, "Cannot unregister UDFs");
    result = 1;
  }

  if (!masking_functions::unregister_sys_vars()) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Cannot unregister system variables");
    result = 1;
  }

  if (!masking_functions::unregister_dynamic_privileges()) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Cannot unregister dynamic privilege");
    result = 1;
  }

  if (result == 0) {
    LogComponentErr(INFORMATION_LEVEL, ER_LOG_PRINTF_MSG,
                    "Component successfully deinitialized");
    log_bi = nullptr;
    log_bs = nullptr;
  }
  return result;
}

}  // anonymous namespace

// clang-format off

// NOLINTBEGIN(cppcoreguidelines-pro-type-const-cast)
// NOLINTBEGIN(bugprone-multi-level-implicit-pointer-conversion)
// NOLINTBEGIN(bugprone-casting-through-void)
// NOLINTBEGIN(misc-use-anonymous-namespace)
BEGIN_COMPONENT_PROVIDES(CURRENT_COMPONENT_NAME)
END_COMPONENT_PROVIDES();

BEGIN_COMPONENT_REQUIRES(CURRENT_COMPONENT_NAME)
  REQUIRES_SERVICE(mysql_charset),
  REQUIRES_SERVICE(mysql_string_factory),
  REQUIRES_SERVICE(mysql_string_charset_converter),
  REQUIRES_SERVICE(mysql_string_get_data_in_charset),
  REQUIRES_SERVICE(mysql_string_append),
  REQUIRES_SERVICE(mysql_string_character_access),
  REQUIRES_SERVICE(mysql_string_byte_access),
  REQUIRES_SERVICE(mysql_string_reset),
  REQUIRES_SERVICE(mysql_string_substr),
  REQUIRES_SERVICE(mysql_string_compare),

  REQUIRES_PSI_THREAD_SERVICE,

  REQUIRES_SERVICE(mysql_command_query),
  REQUIRES_SERVICE(mysql_command_query_result),
  REQUIRES_SERVICE(mysql_command_field_info),
  REQUIRES_SERVICE(mysql_command_options),
  REQUIRES_SERVICE(mysql_command_factory),
  REQUIRES_SERVICE(mysql_command_error_info),
  REQUIRES_SERVICE(mysql_command_thread),

  REQUIRES_SERVICE(udf_registration),
  REQUIRES_SERVICE(dynamic_privilege_register),

  REQUIRES_SERVICE(mysql_udf_metadata),

  REQUIRES_SERVICE(mysql_current_thread_reader),
  REQUIRES_SERVICE(mysql_thd_security_context),
  REQUIRES_SERVICE(global_grants_check),
  REQUIRES_SERVICE(component_sys_variable_register),
  REQUIRES_SERVICE(component_sys_variable_unregister),

  REQUIRES_SERVICE(log_builtins),
  REQUIRES_SERVICE(log_builtins_string),

  REQUIRES_SERVICE(mysql_runtime_error),
END_COMPONENT_REQUIRES();

BEGIN_COMPONENT_METADATA(CURRENT_COMPONENT_NAME)
  METADATA("mysql.author", "Percona Corporation"),
  METADATA("mysql.license", "GPL"),
END_COMPONENT_METADATA();

DECLARE_COMPONENT(CURRENT_COMPONENT_NAME, CURRENT_COMPONENT_NAME_STR)
  component_init,
  component_deinit,
END_DECLARE_COMPONENT();

DECLARE_LIBRARY_COMPONENTS
  &COMPONENT_REF(CURRENT_COMPONENT_NAME)
END_DECLARE_LIBRARY_COMPONENTS
// NOLINTEND(misc-use-anonymous-namespace)
// NOLINTEND(bugprone-casting-through-void)
// NOLINTEND(bugprone-multi-level-implicit-pointer-conversion)
// NOLINTEND(cppcoreguidelines-pro-type-const-cast)

    // clang-format on
