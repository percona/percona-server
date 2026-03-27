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

#define ALLOW_COMPONENT_INCLUDE  // for my_io.h and plugin.h
#include "components/audit_log_filter/log_record_formatter/new.h"
#include "components/audit_log_filter/audit_record.h"

#include <mysql/components/services/defs/event_tracking_authentication_defs.h>
#include <mysql/components/services/defs/event_tracking_command_defs.h>
#include <mysql/components/services/defs/event_tracking_connection_defs.h>
#include <mysql/components/services/defs/event_tracking_general_defs.h>
#include <mysql/components/services/defs/event_tracking_global_variable_defs.h>
#include <mysql/components/services/defs/event_tracking_message_defs.h>
#include <mysql/components/services/defs/event_tracking_parse_defs.h>
#include <mysql/components/services/defs/event_tracking_query_defs.h>
#include <mysql/components/services/defs/event_tracking_stored_program_defs.h>
#include <mysql/components/services/defs/event_tracking_table_access_defs.h>
#include <sql/mysqld.h>

#include <cassert>
#include <chrono>
#include <sstream>

namespace audit_log_filter::log_record_formatter {

namespace {

std::string make_xml_element(std::string_view tag, std::string_view value) {
  std::string result{"<"};
  result.append(tag);
  if (value.empty()) {
    result.append("/>");
  } else {
    result.append(">");
    result.append(value);
    result.append("</");
    result.append(tag);
    result.append(">");
  }
  return result;
}

}  // namespace

AuditRecordString LogRecordFormatterNew::apply(
    const AuditRecordGeneral &audit_record) const noexcept {
  std::stringstream result;
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();

  const auto &sqltext = (audit_record.extended_info.digest.empty()
                             ? audit_record.extended_info.query
                             : audit_record.extended_info.digest);

  /* clang-format off */
  result << " <AUDIT_RECORD>\n"
         << "  <TIMESTAMP>" << make_timestamp(tp) << "</TIMESTAMP>\n"
         << "  <RECORD_ID>" << make_record_id(tp) << "</RECORD_ID>\n"
         << "  <NAME>" << audit_record.extended_info.command << "</NAME>\n"
         << "  <CONNECTION_ID>" << audit_record.event->connection_id << "</CONNECTION_ID>\n"
         << "  <STATUS>" << audit_record.event->error_code << "</STATUS>\n"
         << "  <STATUS_CODE>" << (audit_record.event->error_code ? 1 : 0) << "</STATUS_CODE>\n"
         << "  " << make_xml_element("USER", make_escaped_string(&audit_record.event->user)) << "\n"
         << "  " << make_xml_element("OS_LOGIN", make_escaped_string(audit_record.extended_info.external_user)) << "\n"
         << "  " << make_xml_element("HOST", make_escaped_string(&audit_record.event->host)) << "\n"
         << "  " << make_xml_element("IP", make_escaped_string(&audit_record.event->ip)) << "\n"
         << "  " << make_xml_element("COMMAND_CLASS", audit_record.extended_info.sql_command) << "\n";

  if (!sqltext.empty()) {
    result << "  <SQLTEXT>" << make_escaped_string(sqltext) << "</SQLTEXT>\n";
  }

  result << " </AUDIT_RECORD>\n";;
  /* clang-format on */

  return result.str();
}

AuditRecordString LogRecordFormatterNew::apply(
    const AuditRecordConnection &audit_record) const noexcept {
  std::stringstream result;
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();

  /* clang-format off */
  result << " <AUDIT_RECORD>\n"
         << "  <TIMESTAMP>" << make_timestamp(tp) << "</TIMESTAMP>\n"
         << "  <RECORD_ID>" << make_record_id(tp) << "</RECORD_ID>\n"
         << "  <NAME>" << event_subclass_to_string(audit_record.event) << "</NAME>\n"
         << "  <CONNECTION_ID>" << audit_record.event->connection_id << "</CONNECTION_ID>\n"
         << "  <STATUS>" << audit_record.event->status << "</STATUS>\n"
         << "  <STATUS_CODE>" << (audit_record.event->status ? 1 : 0) << "</STATUS_CODE>\n"
         << "  " << make_xml_element("USER", make_escaped_string(&audit_record.event->user)) << "\n"
         << "  " << make_xml_element("OS_LOGIN", make_escaped_string(&audit_record.event->external_user)) << "\n"
         << "  " << make_xml_element("HOST", make_escaped_string(&audit_record.event->host)) << "\n"
         << "  " << make_xml_element("IP", make_escaped_string(&audit_record.event->ip)) << "\n"
         << "  <COMMAND_CLASS>connect</COMMAND_CLASS>\n"
         << "  <CONNECTION_TYPE>" << connection_type_name_to_string(audit_record.event->connection_type) << "</CONNECTION_TYPE>\n";

  if (audit_record.event->event_subclass == EVENT_TRACKING_CONNECTION_CONNECT) {
    result << extra_attrs_to_string(audit_record.extended_info)
           << "  " << make_xml_element("PRIV_USER", make_escaped_string(&audit_record.event->priv_user)) << "\n"
           << "  " << make_xml_element("PROXY_USER", make_escaped_string(&audit_record.event->proxy_user)) << "\n"
           << "  " << make_xml_element("DB", make_escaped_string(&audit_record.event->database)) << "\n";
  }

  result << " </AUDIT_RECORD>\n";
  /* clang-format on */

  return result.str();
}

AuditRecordString LogRecordFormatterNew::apply(
    const AuditRecordTableAccess &audit_record) const noexcept {
  std::stringstream result;
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();

  /* clang-format off */
  result << " <AUDIT_RECORD>\n"
         << "  <TIMESTAMP>" << make_timestamp(tp) << "</TIMESTAMP>\n"
         << "  <RECORD_ID>" << make_record_id(tp) << "</RECORD_ID>\n"
         << "  <NAME>" << event_subclass_to_string(audit_record.event) << "</NAME>\n"
         << "  <CONNECTION_ID>" << audit_record.event->connection_id << "</CONNECTION_ID>\n"
         << user_info_to_string(audit_record.extended_info)
         << "  " << make_xml_element("OS_LOGIN", make_escaped_string(audit_record.extended_info.external_user)) << "\n"
         << "  " << make_xml_element("HOST", make_escaped_string(audit_record.extended_info.host)) << "\n"
         << "  " << make_xml_element("IP", make_escaped_string(audit_record.extended_info.ip)) << "\n"
         << "  " << make_xml_element("COMMAND_CLASS", audit_record.extended_info.sql_command) << "\n"
         << "  <SQLTEXT>" << (audit_record.extended_info.digest.empty() ? make_escaped_string(audit_record.extended_info.query)
                                                                          : make_escaped_string(audit_record.extended_info.digest)) << "</SQLTEXT>\n"
         << "  <DB>" << make_escaped_string(&audit_record.event->table_database) << "</DB>\n"
         << "  <TABLE>" << make_escaped_string(&audit_record.event->table_name) << "</TABLE>\n"
         << " </AUDIT_RECORD>\n";
  /* clang-format on */

  return result.str();
}

AuditRecordString LogRecordFormatterNew::apply(
    const AuditRecordGlobalVariable &audit_record) const noexcept {
  std::stringstream result;
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();

  /* clang-format off */
  result << " <AUDIT_RECORD>\n"
         << "  <NAME>" << event_subclass_to_string(audit_record.event) << "</NAME>\n"
         << "  <RECORD_ID>" << make_record_id(tp) << "</RECORD_ID>\n"
         << "  <TIMESTAMP>" << make_timestamp(tp) << "</TIMESTAMP>\n"
         << "  <COMMAND_CLASS>" << make_escaped_string(audit_record.event->sql_command) << "</COMMAND_CLASS>\n"
         << "  <CONNECTION_ID>" << audit_record.event->connection_id << "</CONNECTION_ID>\n"
         << "  <VARIABLE_NAME>" << make_escaped_string(&audit_record.event->variable_name) << "</VARIABLE_NAME>\n"
         << "  <VARIABLE_VALUE>" << make_escaped_string(&audit_record.event->variable_value) << "</VARIABLE_VALUE>\n"
         << " </AUDIT_RECORD>\n";
  /* clang-format on */

  return result.str();
}

AuditRecordString LogRecordFormatterNew::apply(
    const AuditRecordCommand &audit_record) const noexcept {
  std::stringstream result;
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();

  /* clang-format off */
  result << " <AUDIT_RECORD>\n"
         << "  <NAME>" << event_subclass_to_string(audit_record.event) << "</NAME>\n"
         << "  <RECORD_ID>" << make_record_id(tp) << "</RECORD_ID>\n"
         << "  <TIMESTAMP>" << make_timestamp(tp) << "</TIMESTAMP>\n"
         << "  <STATUS>" << audit_record.event->status << "</STATUS>\n"
         << "  <CONNECTION_ID>" << audit_record.event->connection_id << "</CONNECTION_ID>\n"
         << "  <COMMAND_CLASS>" << make_escaped_string(&audit_record.event->command) << "</COMMAND_CLASS>\n"
         << " </AUDIT_RECORD>\n";
  /* clang-format on */

  return result.str();
}

AuditRecordString LogRecordFormatterNew::apply(
    const AuditRecordQuery &audit_record) const noexcept {
  std::stringstream result;
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();

  /* clang-format off */
  result << " <AUDIT_RECORD>\n"
         << "  <NAME>" << event_subclass_to_string(audit_record.event) << "</NAME>\n"
         << "  <RECORD_ID>" << make_record_id(tp) << "</RECORD_ID>\n"
         << "  <TIMESTAMP>" << make_timestamp(tp) << "</TIMESTAMP>\n"
         << "  <STATUS>" << audit_record.event->status << "</STATUS>\n"
         << "  <CONNECTION_ID>" << audit_record.event->connection_id << "</CONNECTION_ID>\n"
         << "  <COMMAND_CLASS>" << make_escaped_string(audit_record.event->sql_command) << "</COMMAND_CLASS>\n"
         << "  <SQLTEXT>" << (audit_record.extended_info.digest.empty() ? make_escaped_string(&audit_record.event->query)
                                                                          : make_escaped_string(audit_record.extended_info.digest)) << "</SQLTEXT>\n"
         << " </AUDIT_RECORD>\n";
  /* clang-format on */

  return result.str();
}

AuditRecordString LogRecordFormatterNew::apply(
    const AuditRecordStoredProgram &audit_record) const noexcept {
  std::stringstream result;
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();

  /* clang-format off */
  result << " <AUDIT_RECORD>\n"
         << "  <NAME>" << event_subclass_to_string(audit_record.event) << "</NAME>\n"
         << "  <RECORD_ID>" << make_record_id(tp) << "</RECORD_ID>\n"
         << "  <TIMESTAMP>" << make_timestamp(tp) << "</TIMESTAMP>\n"
         << "  <COMMAND_CLASS>" << event_class_to_string(audit_record.event_class) << "</COMMAND_CLASS>\n"
         << "  <CONNECTION_ID>" << audit_record.event->connection_id << "</CONNECTION_ID>\n"
         << "  <DB>" << make_escaped_string(&audit_record.event->database) << "</DB>\n"
         << "  <STORED_PROGRAM>" << make_escaped_string(&audit_record.event->name) << "</STORED_PROGRAM>\n"
         << " </AUDIT_RECORD>\n";
  /* clang-format on */

  return result.str();
}

AuditRecordString LogRecordFormatterNew::apply(
    const AuditRecordAuthentication &audit_record) const noexcept {
  std::stringstream result;
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();

  /* clang-format off */
  result << " <AUDIT_RECORD>\n"
         << "  <NAME>" << event_subclass_to_string(audit_record.event) << "</NAME>\n"
         << "  <RECORD_ID>" << make_record_id(tp) << "</RECORD_ID>\n"
         << "  <TIMESTAMP>" << make_timestamp(tp) << "</TIMESTAMP>\n"
         << "  <COMMAND_CLASS>" << event_class_to_string(audit_record.event_class) << "</COMMAND_CLASS>\n"
         << "  <CONNECTION_ID>" << audit_record.event->connection_id << "</CONNECTION_ID>\n"
         << "  <STATUS>" << audit_record.event->status << "</STATUS>\n"
         << "  <USER>" << make_escaped_string(&audit_record.event->user) << "</USER>\n"
         << "  <HOST>" << make_escaped_string(&audit_record.event->host) << "</HOST>\n"
         << " </AUDIT_RECORD>\n";
  /* clang-format on */

  return result.str();
}

AuditRecordString LogRecordFormatterNew::apply(
    const AuditRecordMessage &audit_record) const noexcept {
  std::stringstream result;
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
  const auto escaped_component =
      make_escaped_string(&audit_record.event->component);
  const auto escaped_producer =
      make_escaped_string(&audit_record.event->producer);
  const auto escaped_message =
      make_escaped_string(&audit_record.event->message);
  const auto escaped_host =
      make_escaped_string(audit_record.extended_info.host);
  const auto escaped_ip = make_escaped_string(audit_record.extended_info.ip);
  const auto escaped_external_user =
      make_escaped_string(audit_record.extended_info.external_user);

  /* clang-format off */
  result << " <AUDIT_RECORD>\n"
         << "  <TIMESTAMP>" << make_timestamp(tp) << "</TIMESTAMP>\n"
         << "  <RECORD_ID>" << make_record_id(tp) << "</RECORD_ID>\n"
         << "  <NAME>" << event_class_to_string(audit_record.event_class) << "</NAME>\n"
         << "  <CONNECTION_ID>" << audit_record.event->connection_id << "</CONNECTION_ID>\n"
         << "  <STATUS>0</STATUS>\n"
         << "  <STATUS_CODE>0</STATUS_CODE>\n"
         << user_info_to_string(audit_record.extended_info)
         << "  " << make_xml_element("OS_LOGIN", escaped_external_user) << "\n"
         << "  " << make_xml_element("HOST", escaped_host) << "\n"
         << "  " << make_xml_element("IP", escaped_ip) << "\n"
         << "  " << make_xml_element("COMMAND_CLASS", audit_record.event_subclass_name) << "\n"
         << "  <COMPONENT>" << escaped_component << "</COMPONENT>\n"
         << "  <PRODUCER>" << escaped_producer << "</PRODUCER>\n"
         << "  <MESSAGE>" << escaped_message << "</MESSAGE>\n"
         << "  <MAP>\n";

  for (size_t i = 0; i < audit_record.event->key_value_map_length; ++i) {
    result << "   <ELEMENT>\n"
           << "    <KEY>" << make_escaped_string(&audit_record.event->key_value_map[i].key) << "</KEY>\n";
    if (audit_record.event->key_value_map[i].value_type == EVENT_TRACKING_MESSAGE_VALUE_TYPE_STR) {
      result << "    <VALUE>" << make_escaped_string(&audit_record.event->key_value_map[i].value.str) << "</VALUE>\n";
    } else if (audit_record.event->key_value_map[i].value_type == EVENT_TRACKING_MESSAGE_VALUE_TYPE_NUM) {
      result << "    <VALUE>" << audit_record.event->key_value_map[i].value.num << "</VALUE>\n";
    } else {
      result << "    <VALUE></VALUE>\n";
    }
    result << "   </ELEMENT>\n";
  }

  result << "  </MAP>\n"
         << " </AUDIT_RECORD>\n";
  /* clang-format on */

  return result.str();
}

AuditRecordString LogRecordFormatterNew::apply(
    const AuditRecordParse &audit_record) const noexcept {
  std::stringstream result;
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();

  /* clang-format off */
  result << " <AUDIT_RECORD>\n"
         << "  <NAME>" << event_subclass_to_string(audit_record.event) << "</NAME>\n"
         << "  <RECORD_ID>" << make_record_id(tp) << "</RECORD_ID>\n"
         << "  <TIMESTAMP>" << make_timestamp(tp) << "</TIMESTAMP>\n"
         << "  <COMMAND_CLASS>" << event_class_to_string(audit_record.event_class) << "</COMMAND_CLASS>\n"
         << "  <CONNECTION_ID>" << audit_record.event->connection_id << "</CONNECTION_ID>\n"
         << "  <FLAGS>" << (audit_record.event->flags != nullptr ? *audit_record.event->flags : 0) << "</FLAGS>\n"
         << "  <SQLTEXT>" << (audit_record.extended_info.digest.empty() ? make_escaped_string(&audit_record.event->query)
                                                                          : make_escaped_string(audit_record.extended_info.digest)) << "</SQLTEXT>\n"
         << "  <REWRITTEN_QUERY>" << make_escaped_string(audit_record.event->rewritten_query) << "</REWRITTEN_QUERY>\n"
         << " </AUDIT_RECORD>\n";
  /* clang-format on */

  return result.str();
}

AuditRecordString LogRecordFormatterNew::apply(
    const AuditRecordAudit &audit_record) const noexcept {
  std::stringstream result;
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();

  /* clang-format off */
  result << " <AUDIT_RECORD>\n"
         << "  <TIMESTAMP>" << make_timestamp(tp) << "</TIMESTAMP>\n"
         << "  <RECORD_ID>" << make_record_id(tp) << "</RECORD_ID>\n"
         << "  <NAME>" << event_subclass_to_string(audit_record.event) << "</NAME>\n"
         << "  <SERVER_ID>" << audit_record.event->server_id << "</SERVER_ID>\n";


  if (audit_record.event->event_subclass == INTERNAL_EVENT_TRACKING_AUDIT_AUDIT)
  {
    std::stringstream startup_options;

    for (int i = 0; i < orig_argc; ++i) {
      if (orig_argv[i]) {
        startup_options << orig_argv[i] << " ";
      }
    }

    std::string startup_options_str = startup_options.str();
    startup_options_str.pop_back();

    result << "  <VERSION>1</VERSION>\n"
           << "  <STARTUP_OPTIONS>" << make_escaped_string(startup_options_str) << "</STARTUP_OPTIONS>\n"
           << "  <OS_VERSION>" << MACHINE_TYPE << "-" << SYSTEM_TYPE << "</OS_VERSION>\n"
           << "  <MYSQL_VERSION>" << server_version << "</MYSQL_VERSION>\n";
  }

  result << " </AUDIT_RECORD>\n";
  /* clang-format on */

  return result.str();
}

void LogRecordFormatterNew::apply_debug_info(
    std::string_view event_class_name, std::string_view event_subclass_name,
    std::string &record_str) noexcept {
  assert(!record_str.empty());

  /* clang-format off */
  std::stringstream debug_info;
  debug_info << "  <EVENT_CLASS_NAME>" << event_class_name << "</EVENT_CLASS_NAME>\n"
             << "  <EVENT_SUBCLASS_NAME>" << event_subclass_name << "</EVENT_SUBCLASS_NAME>\n";
  /* clang-format on */

  std::string insert_after_tag{"<AUDIT_RECORD>\n"};
  auto tag_begin = record_str.find(insert_after_tag, 0);
  record_str.insert(tag_begin + insert_after_tag.length(), debug_info.str());
}

std::string LogRecordFormatterNew::extra_attrs_to_string(
    const ExtendedInfo &info) const noexcept {
  std::stringstream result;
  auto attrs_it = info.attrs.find("connection_attributes");

  /* clang-format off */
  if (attrs_it != info.attrs.cend()) {
    result << "  <CONNECTION_ATTRIBUTES>\n";

    for (const auto &name_value : attrs_it->second) {
      result << "   <ATTRIBUTE>\n"
             << "    <NAME>" << make_escaped_string(name_value.first) << "</NAME>\n"
             << "    <VALUE>" << make_escaped_string(name_value.second) << "</VALUE>\n"
             << "   </ATTRIBUTE>\n";
    }

    result << "  </CONNECTION_ATTRIBUTES>\n";
  }
  /* clang-format on */

  return result.str();
}

std::string LogRecordFormatterNew::user_info_to_string(
    const ExtendedInfo &info) const {
  std::stringstream result;

  result << "  <USER>" << make_escaped_string(info.user) << "["
         << make_escaped_string(info.user) << "] @ "
         << make_escaped_string(info.host) << " ["
         << make_escaped_string(info.ip) << "]</USER>\n";

  return result.str();
}

}  // namespace audit_log_filter::log_record_formatter
