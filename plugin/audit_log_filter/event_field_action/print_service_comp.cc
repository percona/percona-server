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

#include "plugin/audit_log_filter/event_field_action/print_service_comp.h"
#include "plugin/audit_log_filter/audit_error_log.h"

#include "plugin/audit_log_filter/audit_log_filter.h"

#include <mysql/components/my_service.h>
#include <mysql/components/services/mysql_audit_print_service_double_data_source.h>
#include <mysql/components/services/mysql_audit_print_service_longlong_data_source.h>
#include <mysql/components/services/mysql_current_thread_reader.h>
#include <mysql/components/services/mysql_string.h>

#include <unordered_map>

namespace audit_log_filter::event_field_action {
namespace {

const std::string_view kElementNameQueryTime{"query_time"};
const std::string_view kElementNameRowsSent{"rows_sent"};
const std::string_view kElementNameRowsExamined{"rows_examined"};
const std::string_view kElementNameBytesReceived{"bytes_received"};
const std::string_view kElementNameBytesSent{"bytes_sent"};

}  // namespace

EventFieldActionPrintServiceComp::EventFieldActionPrintServiceComp(
    std::string tag_name, PrintServiceElementsList elements_list)
    : m_tag_name{std::move(tag_name)},
      m_elements_list{std::move(elements_list)} {}

EventActionType EventFieldActionPrintServiceComp::get_action_type()
    const noexcept {
  return EventActionType::PrintServiceComp;
}

bool EventFieldActionPrintServiceComp::apply(const AuditRecordFieldsList &fields
                                             [[maybe_unused]],
                                             AuditRecordVariant &audit_record,
                                             AuditRule *audit_rule
                                             [[maybe_unused]]) const noexcept {
  auto *comp_reg_srv = get_audit_log_filter_instance()->get_comp_registry_srv();

  my_service<SERVICE_TYPE(mysql_current_thread_reader)> thd_reader_srv(
      "mysql_current_thread_reader", comp_reg_srv);
  my_service<SERVICE_TYPE(mysql_string_charset_converter)> string_converter_srv(
      "mysql_string_charset_converter", comp_reg_srv);
  my_service<SERVICE_TYPE(mysql_charset)> charset_srv("mysql_charset",
                                                      comp_reg_srv);
  my_service<SERVICE_TYPE(mysql_string_factory)> string_factory_srv(
      "mysql_string_factory", comp_reg_srv);
  my_service<SERVICE_TYPE(mysql_audit_print_service_double_data_source)>
      print_double_srv("mysql_audit_print_service_double_data_source",
                       comp_reg_srv);
  my_service<SERVICE_TYPE(mysql_audit_print_service_longlong_data_source)>
      print_longlong_srv("mysql_audit_print_service_longlong_data_source",
                         comp_reg_srv);

  CHARSET_INFO_h utf8 = charset_srv->get_utf8mb4();
  MYSQL_THD thd;

  if (thd_reader_srv->get(&thd)) {
    return false;
  }

  if (thd == nullptr || !print_double_srv.is_valid() ||
      !print_longlong_srv.is_valid()) {
    return false;
  }

  auto *extended_info =
      std::visit([](auto &rec) -> ExtendedInfo * { return &rec.extended_info; },
                 audit_record);
  extended_info->attrs[m_tag_name] = {};

  my_h_string element_name;
  string_factory_srv->create(&element_name);

  for (const auto &element : m_elements_list) {
    string_converter_srv->convert_from_buffer(
        element_name, element.second.data(), element.second.length(), utf8);

    switch (element.first) {
      case ServiceCompElementType::Double: {
        double value = 0;

        if (!print_double_srv->get(thd, element_name, &value)) {
          extended_info->attrs[m_tag_name].emplace_back(element.second,
                                                        std::to_string(value));
        }
      } break;
      case ServiceCompElementType::Longlong: {
        long long value = 0;

        if (!print_longlong_srv->get(thd, element_name, &value)) {
          extended_info->attrs[m_tag_name].emplace_back(element.second,
                                                        std::to_string(value));
        }
      } break;
      default:
        break;
    }
  }

  string_factory_srv->destroy(element_name);

  return true;
}

ServiceCompElementType EventFieldActionPrintServiceComp::string_to_element_type(
    const std::string &type_name) noexcept {
  static const std::unordered_map<std::string, ServiceCompElementType>
      str_to_type{{"double", ServiceCompElementType::Double},
                  {"longlong", ServiceCompElementType::Longlong}};

  const auto it = str_to_type.find(type_name);
  return it != str_to_type.cend() ? it->second
                                  : ServiceCompElementType::Unknown;
}

ServiceCompElementName EventFieldActionPrintServiceComp::string_to_element_name(
    const std::string &element_name) noexcept {
  static const std::unordered_map<std::string, ServiceCompElementName>
      str_to_name{{"query_time", kElementNameQueryTime},
                  {"rows_sent", kElementNameRowsSent},
                  {"rows_examined", kElementNameRowsExamined},
                  {"bytes_received", kElementNameBytesReceived},
                  {"bytes_sent", kElementNameBytesSent}};

  const auto it = str_to_name.find(element_name);
  return it != str_to_name.cend() ? it->second : ServiceCompElementName{};
}

}  // namespace audit_log_filter::event_field_action
