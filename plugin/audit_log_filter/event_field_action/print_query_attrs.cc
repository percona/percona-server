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

#include "plugin/audit_log_filter/event_field_action/print_query_attrs.h"
#include "plugin/audit_log_filter/audit_error_log.h"

#include "plugin/audit_log_filter/sys_vars.h"

#include <mysql/components/my_service.h>
#include <mysql/components/services/mysql_current_thread_reader.h>
#include <mysql/components/services/mysql_query_attributes.h>
#include <mysql/components/services/mysql_string.h>

#include "sql/sql_class.h"

#include <scope_guard.h>

namespace audit_log_filter::event_field_action {

EventFieldActionPrintQueryAttrs::EventFieldActionPrintQueryAttrs(
    std::string tag_name, QueryAttrsList attrs_list)
    : m_tag_name{std::move(tag_name)}, m_attrs_list{std::move(attrs_list)} {}

EventActionType EventFieldActionPrintQueryAttrs::get_action_type()
    const noexcept {
  return EventActionType::PrintQueryAttrs;
}

bool EventFieldActionPrintQueryAttrs::apply(const AuditRecordFieldsList &fields
                                            [[maybe_unused]],
                                            AuditRecordVariant &audit_record,
                                            AuditRule *audit_rule
                                            [[maybe_unused]]) const noexcept {
  auto *comp_reg_srv = SysVars::get_comp_registry_srv();

  my_service<SERVICE_TYPE(mysql_current_thread_reader)> thd_reader_srv(
      "mysql_current_thread_reader", comp_reg_srv);
  my_service<SERVICE_TYPE(mysql_string_charset_converter)> string_converter_srv(
      "mysql_string_charset_converter", comp_reg_srv);
  my_service<SERVICE_TYPE(mysql_charset)> charset_srv("mysql_charset",
                                                      comp_reg_srv);
  my_service<SERVICE_TYPE(mysql_string_factory)> string_factory_srv(
      "mysql_string_factory", comp_reg_srv);

  my_service<SERVICE_TYPE(mysql_query_attributes_iterator)> attrs_iterator_srv(
      "mysql_query_attributes_iterator", comp_reg_srv);
  my_service<SERVICE_TYPE(mysql_query_attribute_isnull)> isnull_srv(
      "mysql_query_attribute_isnull", comp_reg_srv);
  my_service<SERVICE_TYPE(mysql_query_attribute_string)> attr_string_srv(
      "mysql_query_attribute_string", comp_reg_srv);

  CHARSET_INFO_h utf8 = charset_srv->get_utf8mb4();
  MYSQL_THD thd;

  if (thd_reader_srv->get(&thd)) {
    return false;
  }

  if (thd == nullptr) {
    return false;
  }

  mysqlh_query_attributes_iterator iter;

  if (attrs_iterator_srv->create(thd, nullptr, &iter)) {
    return false;
  }

  auto *extended_info =
      std::visit([](auto &rec) -> ExtendedInfo * { return &rec.extended_info; },
                 audit_record);
  extended_info->attrs[m_tag_name] = {};

  bool is_null = false;
  char buff_attr_name[1024];
  char buff_attr_value[1024];

  auto cleanup_iter =
      create_scope_guard([&] { attrs_iterator_srv->release(iter); });

  do {
    my_h_string attr_name;

    if (attrs_iterator_srv->get_name(iter, &attr_name)) {
      return false;
    }

    auto cleanup_name =
        create_scope_guard([&] { string_factory_srv->destroy(attr_name); });

    if (isnull_srv->get(iter, &is_null)) {
      return false;
    }

    string_converter_srv->convert_to_buffer(attr_name, buff_attr_name,
                                            sizeof(buff_attr_name), utf8);

    if (std::find(m_attrs_list.cbegin(), m_attrs_list.cend(), buff_attr_name) !=
        m_attrs_list.cend()) {
      if (!is_null) {
        my_h_string attr_value;

        if (attr_string_srv->get(iter, &attr_value)) {
          return false;
        }

        string_converter_srv->convert_to_buffer(attr_value, buff_attr_value,
                                                sizeof(buff_attr_value), utf8);

        string_factory_srv->destroy(attr_value);
      }

      extended_info->attrs[m_tag_name].emplace_back(
          buff_attr_name, (is_null ? "" : buff_attr_value));
    }
  } while (!attrs_iterator_srv->next(iter));

  return true;
}

}  // namespace audit_log_filter::event_field_action
