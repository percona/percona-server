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

#include <cassert>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <type_traits>

#include "masking_functions/command_service_tuple.hpp"
#include "masking_functions/sql_context.hpp"

namespace {

MYSQL_H to_mysql_h(void *p) noexcept { return static_cast<MYSQL_H>(p); }

constexpr const char default_command_user_name[] = "root";

}  // anonymous namespace

namespace masking_functions {

void sql_context::deleter::operator()(void *ptr) const noexcept {
  if (ptr != nullptr) (*services->factory->close)(to_mysql_h(ptr));
}

sql_context::sql_context(const command_service_tuple &services)
    : impl_{nullptr, deleter{&services}} {
  MYSQL_H local_mysql_h = nullptr;
  if ((*get_services().factory->init)(&local_mysql_h) != 0) {
    throw std::runtime_error{"Couldn't initialize server handle"};
  }
  assert(local_mysql_h != nullptr);
  impl_.reset(local_mysql_h);

  // setting MYSQL_COMMAND_PROTOCOL to nullptr will be translated to the
  // default value "local"
  if ((*get_services().options->set)(local_mysql_h, MYSQL_COMMAND_PROTOCOL,
                                     nullptr) != 0) {
    throw std::runtime_error{"Couldn't set protocol"};
  }

  // setting MYSQL_COMMAND_USER_NAME to default_command_user_name here
  // as the default MYSQL_SESSION_USER ("mysql.session") does not have
  // access to the mysql.masking_dictionaries
  if ((*get_services().options->set)(local_mysql_h, MYSQL_COMMAND_USER_NAME,
                                     default_command_user_name) != 0) {
    throw std::runtime_error{"Couldn't set username"};
  }

  // setting MYSQL_COMMAND_HOST_NAME to nullptr will be translated to the
  // default MYSQL_SYS_HOST ("localhost")
  if ((*get_services().options->set)(local_mysql_h, MYSQL_COMMAND_HOST_NAME,
                                     nullptr) != 0) {
    throw std::runtime_error{"Couldn't set hostname"};
  }

  if ((*get_services().factory->connect)(local_mysql_h) != 0) {
    throw std::runtime_error{"Couldn't establish server connection"};
  }

  // In order to make sure that internal INSERT / DELETE queries which
  // manipulate 'mysql.masking_dictionaries' are not affected by the global
  // value of '@@global.autocommit' (we want all operations to be committed
  // immediately), we are setting the value of the 'autocommit' session
  // variable here explicitly to 'ON'.
  if ((*get_services().factory->autocommit)(to_mysql_h(impl_.get()), true)) {
    throw std::runtime_error{"Couldn't set autocommit"};
  }
}

bool sql_context::execute_dml(std::string_view query) {
  if ((*get_services().query->query)(to_mysql_h(impl_.get()), query.data(),
                                     query.length()) != 0) {
    throw std::runtime_error{"Error while executing SQL DML query"};
  }
  std::uint64_t row_count = 0;
  if ((*get_services().query->affected_rows)(to_mysql_h(impl_.get()),
                                             &row_count) != 0) {
    throw std::runtime_error{"Couldn't get number of affected rows"};
  }
  return row_count > 0;
}

void sql_context::execute_select_internal(
    std::string_view query, std::size_t expected_number_of_fields,
    const row_internal_callback &callback) {
  if ((*get_services().query->query)(to_mysql_h(impl_.get()), query.data(),
                                     query.length()) != 0) {
    throw std::runtime_error{"Error while executing SQL select query"};
  }

  unsigned int actual_number_of_fields = 0;
  if ((*get_services().field_info->field_count)(
          to_mysql_h(impl_.get()), &actual_number_of_fields) != 0) {
    throw std::runtime_error{"Couldn't get number of fields"};
  }

  if (actual_number_of_fields != expected_number_of_fields) {
    throw std::runtime_error{
        "Micmatch between actual and expected number of fields"};
  }

  MYSQL_RES_H mysql_res = nullptr;
  if ((*get_services().query_result->store_result)(to_mysql_h(impl_.get()),
                                                   &mysql_res) != 0) {
    throw std::runtime_error{"Couldn't store MySQL result"};
  }
  if (mysql_res == nullptr) {
    throw std::runtime_error{"Couldn't create MySQL result handler"};
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
  // service is implementted via 'mysql_affected_rows()' MySQL client
  // function, it is OK to use it for SELECT statements as well, because
  // in this case it will work like 'mysql_num_rows()'.
  if ((*get_services().query->affected_rows)(to_mysql_h(impl_.get()),
                                             &row_count) != 0)
    throw std::runtime_error{"Couldn't query row count"};

  for (auto i = row_count; i > 0; --i) {
    MYSQL_ROW_H field_values = nullptr;
    ulong *field_value_lengths = nullptr;

    if ((*get_services().query_result->fetch_row)(mysql_res, &field_values) !=
        0)
      throw std::runtime_error{"Couldn't fetch length"};
    if ((*get_services().query_result->fetch_lengths)(
            mysql_res, &field_value_lengths) != 0)
      throw std::runtime_error{"Couldn't fetch length"};

    callback(field_values, field_value_lengths);
  }
}

}  // namespace masking_functions
