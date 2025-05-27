/* Copyright (c) 2023 Percona LLC and/or its affiliates. All rights reserved.

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

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include <mysql_com.h>  // needed only for MYSQL_ERRMSG_SIZE

#include <mysql/components/services/mysql_command_services.h>

#include "masking_functions/command_service_tuple.hpp"
#include "masking_functions/sql_context.hpp"

namespace {

MYSQL_H to_mysql_h(void *ptr) noexcept { return static_cast<MYSQL_H>(ptr); }

}  // anonymous namespace

namespace masking_functions {

void sql_context::deleter::operator()(void *ptr) const noexcept {
  if (ptr != nullptr) (*services->factory->close)(to_mysql_h(ptr));
}

sql_context::sql_context(
    const command_service_tuple &services,
    sql_context_registry_access initialization_registry_locking_mode,
    sql_context_registry_access operation_registry_locking_mode,
    bool initialize_thread)
    : impl_{nullptr, deleter{&services}} {
  MYSQL_H local_mysql_h = nullptr;
  if ((*get_services().factory->init)(&local_mysql_h) != 0) {
    raise_with_error_message("Couldn't initialize server handle");
  }
  assert(local_mysql_h != nullptr);
  impl_.reset(local_mysql_h);

  if (initialize_thread) {
    if ((*get_services().options->set)(
            local_mysql_h, MYSQL_COMMAND_LOCAL_THD_HANDLE, nullptr) != 0) {
      raise_with_error_message("Couldn't set local THD handle");
    }
  }

  // setting MYSQL_NO_LOCK_REGISTRY is needed for cases when we destroy
  // 'sql_context' from the 'UNINSTALL COMPONENT' handler (component's
  // 'deinit()' function)
  const bool initialization_no_lock_registry_option_value{
      initialization_registry_locking_mode ==
      sql_context_registry_access::non_locking};
  if ((*get_services().options->set)(
          local_mysql_h, MYSQL_NO_LOCK_REGISTRY,
          &initialization_no_lock_registry_option_value) != 0) {
    raise_with_error_message(
        "Couldn't set initialization registry locking mode");
  }

  // setting MYSQL_COMMAND_PROTOCOL to nullptr will be translated to the
  // default value "local"
  if ((*get_services().options->set)(local_mysql_h, MYSQL_COMMAND_PROTOCOL,
                                     nullptr) != 0) {
    raise_with_error_message("Couldn't set protocol");
  }

  // nullptr here will be translated into MYSQL_SESSION_USER ("mysql.session")
  if ((*get_services().options->set)(local_mysql_h, MYSQL_COMMAND_USER_NAME,
                                     nullptr) != 0) {
    raise_with_error_message("Couldn't set username");
  }

  // setting MYSQL_COMMAND_HOST_NAME to nullptr will be translated to the
  // default MYSQL_SYS_HOST ("localhost")
  if ((*get_services().options->set)(local_mysql_h, MYSQL_COMMAND_HOST_NAME,
                                     nullptr) != 0) {
    raise_with_error_message("Couldn't set hostname");
  }

  if ((*get_services().factory->connect)(local_mysql_h) != 0) {
    raise_with_error_message("Couldn't establish server connection");
  }

  // In order to make sure that internal INSERT / DELETE queries which
  // manipulate 'mysql.masking_dictionaries' are not affected by the global
  // value of '@@global.autocommit' (we want all operations to be committed
  // immediately), we are setting the value of the 'autocommit' session
  // variable here explicitly to 'ON'.
  if ((*get_services().factory->autocommit)(local_mysql_h, true) != 0) {
    raise_with_error_message("Couldn't set autocommit");
  }

  if (operation_registry_locking_mode != initialization_registry_locking_mode) {
    const bool operation_no_lock_registry_option_value{
        operation_registry_locking_mode ==
        sql_context_registry_access::non_locking};
    if ((*get_services().options->set)(
            local_mysql_h, MYSQL_NO_LOCK_REGISTRY,
            &operation_no_lock_registry_option_value) != 0) {
      raise_with_error_message("Couldn't set operation registry locking mode");
    }
  }
}

void sql_context::reset() {
  if ((*get_services().factory->reset)(to_mysql_h(impl_.get())) != 0) {
    raise_with_error_message("Couldn't reset connection");
  }
}

bool sql_context::execute_dml(std::string_view query) {
  // NOLINTNEXTLINE(readability-qualified-auto,llvm-qualified-auto)
  const auto casted_impl{to_mysql_h(impl_.get())};
  if ((*get_services().query->query)(casted_impl, query.data(),
                                     query.length()) != 0) {
    raise_with_error_message("Error while executing SQL DML query");
  }
  std::uint64_t row_count = 0;
  if ((*get_services().query->affected_rows)(casted_impl, &row_count) != 0) {
    raise_with_error_message("Couldn't get number of affected rows");
  }
  return row_count > 0;
}

void sql_context::execute_select_internal(
    std::string_view query, std::size_t expected_number_of_fields,
    const row_internal_callback &callback) {
  // NOLINTNEXTLINE(readability-qualified-auto,llvm-qualified-auto)
  const auto casted_impl{to_mysql_h(impl_.get())};
  if ((*get_services().query->query)(casted_impl, query.data(),
                                     query.length()) != 0) {
    raise_with_error_message("Error while executing SQL select query");
  }

  unsigned int actual_number_of_fields = 0;
  if ((*get_services().field_info->field_count)(
          casted_impl, &actual_number_of_fields) != 0) {
    raise_with_error_message("Couldn't get number of fields");
  }

  if (actual_number_of_fields != expected_number_of_fields) {
    raise_with_error_message(
        "Mismatch between actual and expected number of fields");
  }

  MYSQL_RES_H mysql_res = nullptr;
  if ((*get_services().query_result->store_result)(casted_impl, &mysql_res) !=
      0) {
    raise_with_error_message("Couldn't store MySQL result");
  }
  if (mysql_res == nullptr) {
    raise_with_error_message("Couldn't create MySQL result handler");
  }

  auto mysql_res_deleter = [deleter = get_services().query_result->free_result](
                               MYSQL_RES_H handler) {
    if (handler != nullptr) (*deleter)(handler);
  };
  using mysql_res_type = std::remove_pointer_t<MYSQL_RES_H>;
  using mysql_res_ptr =
      std::unique_ptr<mysql_res_type, decltype(mysql_res_deleter)>;

  mysql_res_ptr mysql_res_guard(mysql_res, std::move(mysql_res_deleter));
  std::uint64_t row_count = 0;
  // As the 'affected_rows()' method of the 'mysql_command_query' MySQL
  // service is implemented via 'mysql_affected_rows()' MySQL client
  // function, it is OK to use it for SELECT statements as well, because
  // in this case it will work like 'mysql_num_rows()'.
  if ((*get_services().query->affected_rows)(casted_impl, &row_count) != 0)
    raise_with_error_message("Couldn't query row count");

  for (std::uint64_t i = 0; i < row_count; ++i) {
    MYSQL_ROW_H field_values = nullptr;
    ulong *field_value_lengths = nullptr;  // NOLINT(misc-include-cleaner)

    if ((*get_services().query_result->fetch_row)(mysql_res, &field_values) !=
        0)
      raise_with_error_message("Couldn't fetch row");
    if ((*get_services().query_result->fetch_lengths)(
            mysql_res, &field_value_lengths) != 0)
      raise_with_error_message("Couldn't fetch length");

    callback(field_values, field_value_lengths);
  }
}

[[noreturn]] void sql_context::raise_with_error_message(
    std::string_view prefix) {
  std::string message{prefix};

  // despite the fact that sql_error service method expects 'char **ptr' as an
  // output parameter, it does not do '*ptr = some_internal_string', instead it
  // expects this double pointer to point to a pointer to a valid buffer and
  // does 'memcpy(*ptr, ...)'

  // unfortunately, there is no way to specify the size of the buffer -
  // however, as this buffer is filled with the strings coming from the
  // 'mysql_error()' client API function, its max size is known to be
  // MYSQL_ERRMSG_SIZE
  unsigned int error_number{0};
  using error_message_buffer_type = std::array<char, MYSQL_ERRMSG_SIZE>;
  error_message_buffer_type error_message_buffer;
  char *error_message_buffer_ptr{std::data(error_message_buffer)};
  // NOLINTNEXTLINE(readability-qualified-auto,llvm-qualified-auto)
  const auto casted_impl{to_mysql_h(impl_.get())};
  if (casted_impl != nullptr &&
      (*get_services().error_info->sql_errno)(casted_impl, &error_number) ==
          0 &&
      (*get_services().error_info->sql_error)(casted_impl,
                                              &error_message_buffer_ptr) == 0) {
    message += "(errno = ";
    message += std::to_string(error_number);
    message += " \"";
    if (error_message_buffer_ptr != nullptr) {
      message += error_message_buffer_ptr;
    }
    message += "\")";
  }
  throw std::runtime_error{message};
}

}  // namespace masking_functions
