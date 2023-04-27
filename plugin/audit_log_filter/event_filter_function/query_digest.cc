/* Copyright (c) 2022 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#include "plugin/audit_log_filter/event_filter_function/query_digest.h"

#include "plugin/audit_log_filter/sys_vars.h"

#include <mysql/components/my_service.h>
#include <mysql/components/services/mysql_current_thread_reader.h>
#include <mysql/components/services/mysql_string.h>
#include <mysql/components/services/mysql_thd_attributes.h>

namespace audit_log_filter::event_filter_function {

EventFilterFunction<EventFilterFunctionType::QueryDigest>::EventFilterFunction(
    FunctionArgsList args)
    : EventFilterFunctionBase{std::move(args)} {}

bool EventFilterFunctionQueryDigest::validate_args(
    const FunctionArgsList &args,
    const FunctionReturnType expected_return_type) noexcept {
  if (args.size() > 1 ||
      (!args.empty() && args[0].arg_type != FunctionArgType::String)) {
    return false;
  }

  if ((expected_return_type == FunctionReturnType::String && !args.empty()) ||
      (expected_return_type == FunctionReturnType::Bool && args.empty())) {
    return false;
  }

  return true;
}

std::string EventFilterFunctionQueryDigest::get_query_digest() const noexcept {
  auto *comp_registry_srv = SysVars::get_comp_registry_srv();

  my_service<SERVICE_TYPE(mysql_charset)> charset_srv("mysql_charset",
                                                      comp_registry_srv);
  my_service<SERVICE_TYPE(mysql_string_factory)> string_factory_srv(
      "mysql_string_factory", comp_registry_srv);
  my_service<SERVICE_TYPE(mysql_string_charset_converter)> string_converter_srv(
      "mysql_string_charset_converter", comp_registry_srv);
  my_service<SERVICE_TYPE(mysql_current_thread_reader)> current_thd_srv(
      "mysql_current_thread_reader", comp_registry_srv);
  my_service<SERVICE_TYPE(mysql_thd_attributes)> thd_attrs_srv(
      "mysql_thd_attributes", comp_registry_srv);

  CHARSET_INFO_h utf8 = charset_srv->get_utf8mb4();

  my_h_string digest;
  string_factory_srv->create(&digest);

  MYSQL_THD thd;
  current_thd_srv->get(&thd);

  char buff_digest[1024];
  std::string result;

  if (!thd_attrs_srv->get(thd, "query_digest",
                          reinterpret_cast<void *>(&digest))) {
    string_converter_srv->convert_to_buffer(digest, buff_digest,
                                            sizeof(buff_digest), utf8);
    result.append(buff_digest);
  }

  string_factory_srv->destroy(digest);

  return result;
}

bool EventFilterFunctionQueryDigest::exec(const AuditRecordFieldsList &fields,
                                          bool &result) noexcept {
  /*
   * With an argument, query_digest returns a Boolean indicating whether the
   * argument is equal to the current statement digest
   */
  if (!has_args()) {
    // The query_digest() overload returning bool result was used
    // in a wrong context
    return false;
  }

  std::string expected_digest = arg_to_string<0>(fields);
  auto digest = get_query_digest();

  result = expected_digest == digest;

  return true;
}

bool EventFilterFunctionQueryDigest::exec(const AuditRecordFieldsList &fields
                                          [[maybe_unused]],
                                          std::string &result) noexcept {
  /*
   * With no argument, query_digest returns the statement digest value
   * corresponding to the statement literal text in the current event
   */
  auto digest = get_query_digest();
  result = std::move(digest);

  return true;
}

}  // namespace audit_log_filter::event_filter_function
