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

#include "plugin/audit_log_filter/log_record_formatter/new.h"
#include "plugin/audit_log_filter/audit_record.h"

#include <chrono>
#include <sstream>

namespace audit_log_filter::log_record_formatter {

AuditRecordString LogRecordFormatterNew::apply(
    const AuditRecordGeneral &audit_record) const noexcept {
  std::stringstream result;
  std::chrono::system_clock::time_point tp{
      std::chrono::seconds{audit_record.event->general_time}};

  result << "  <AUDIT_RECORD>\n"
         << "    <NAME>"
         << make_escaped_string(&audit_record.event->general_command)
         << "</NAME>\n"
         << "    <RECORD_ID>" << make_record_id(tp) << "</RECORD_ID>\n"
         << "    <TIMESTAMP>" << make_timestamp(tp) << "</TIMESTAMP>\n"
         << "    <COMMAND_CLASS>"
         << make_escaped_string(&audit_record.event->general_sql_command)
         << "</COMMAND_CLASS>\n"
         << "    <CONNECTION_ID>" << audit_record.event->general_thread_id
         << "</CONNECTION_ID>\n"
         << "    <HOST>"
         << make_escaped_string(&audit_record.event->general_host)
         << "</HOST>\n"
         << "    <IP>" << make_escaped_string(&audit_record.event->general_ip)
         << "</IP>\n"
         << "    <USER>"
         << make_escaped_string(&audit_record.event->general_user)
         << "</USER>\n"
         << "    <OS_LOGIN>"
         << make_escaped_string(&audit_record.event->general_external_user)
         << "</OS_LOGIN>\n"
         << "    <SQLTEXT>"
         << (audit_record.extended_info.digest.empty()
                 ? make_escaped_string(&audit_record.event->general_query)
                 : make_escaped_string(audit_record.extended_info.digest))
         << "</SQLTEXT>\n"
         << "    <STATUS>" << audit_record.event->general_error_code
         << "</STATUS>\n"
         << "  </AUDIT_RECORD>\n";

  return result.str();
}

AuditRecordString LogRecordFormatterNew::apply(
    const AuditRecordConnection &audit_record) const noexcept {
  std::stringstream result;
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();

  result << "  <AUDIT_RECORD>\n"
         << "    <NAME>"
         << event_subclass_to_string(audit_record.event->event_subclass)
         << "</NAME>\n"
         << "    <RECORD_ID>" << make_record_id(tp) << "</RECORD_ID>\n"
         << "    <TIMESTAMP>" << make_timestamp(tp) << "</TIMESTAMP>\n"
         << "    <COMMAND_CLASS>"
         << event_class_to_string(audit_record.event_class)
         << "</COMMAND_CLASS>\n"
         << "    <CONNECTION_ID>" << audit_record.event->connection_id
         << "</CONNECTION_ID>\n"
         << "    <HOST>" << make_escaped_string(&audit_record.event->host)
         << "</HOST>\n"
         << "    <IP>" << make_escaped_string(&audit_record.event->ip)
         << "</IP>\n"
         << "    <USER>" << make_escaped_string(&audit_record.event->user)
         << "</USER>\n"
         << "    <OS_LOGIN>"
         << make_escaped_string(&audit_record.event->external_user)
         << "</OS_LOGIN>\n"
         << "    <PRIV_USER>"
         << make_escaped_string(&audit_record.event->priv_user)
         << "</PRIV_USER>\n"
         << "    <PROXY_USER>"
         << make_escaped_string(&audit_record.event->proxy_user)
         << "</PROXY_USER>\n"
         << "    <DB>" << make_escaped_string(&audit_record.event->database)
         << "</DB>\n"
         << "    <STATUS>" << audit_record.event->status << "</STATUS>\n"
         << "    <CONNECTION_TYPE>"
         << connection_type_name_to_string(audit_record.event->connection_type)
         << "</CONNECTION_TYPE>\n"
         << extra_attrs_to_string(audit_record.extended_info) << "\n"
         << "  </AUDIT_RECORD>\n";

  return result.str();
}

AuditRecordString LogRecordFormatterNew::apply(
    const AuditRecordTableAccess &audit_record) const noexcept {
  std::stringstream result;
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();

  result << "  <AUDIT_RECORD>\n"
         << "    <NAME>"
         << event_subclass_to_string(audit_record.event->event_subclass)
         << "</NAME>\n"
         << "    <RECORD_ID>" << make_record_id(tp) << "</RECORD_ID>\n"
         << "    <TIMESTAMP>" << make_timestamp(tp) << "</TIMESTAMP>\n"
         << "    <COMMAND_CLASS>"
         << sql_command_id_to_string(audit_record.event->sql_command_id)
         << "</COMMAND_CLASS>\n"
         << "    <CONNECTION_ID>" << audit_record.event->connection_id
         << "</CONNECTION_ID>\n"
         << "    <SQLTEXT>"
         << (audit_record.extended_info.digest.empty()
                 ? make_escaped_string(&audit_record.event->query)
                 : make_escaped_string(audit_record.extended_info.digest))
         << "</SQLTEXT>\n"
         << "    <DB>"
         << make_escaped_string(&audit_record.event->table_database)
         << "</DB>\n"
         << "    <TABLE>"
         << make_escaped_string(&audit_record.event->table_name) << "</TABLE>\n"
         << "  </AUDIT_RECORD>\n";

  return result.str();
}

AuditRecordString LogRecordFormatterNew::apply(
    const AuditRecordGlobalVariable &audit_record) const noexcept {
  std::stringstream result;
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();

  result << "  <AUDIT_RECORD>\n"
         << "    <NAME>"
         << event_subclass_to_string(audit_record.event->event_subclass)
         << "</NAME>\n"
         << "    <RECORD_ID>" << make_record_id(tp) << "</RECORD_ID>\n"
         << "    <TIMESTAMP>" << make_timestamp(tp) << "</TIMESTAMP>\n"
         << "    <COMMAND_CLASS>"
         << sql_command_id_to_string(audit_record.event->sql_command_id)
         << "</COMMAND_CLASS>\n"
         << "    <CONNECTION_ID>" << audit_record.event->connection_id
         << "</CONNECTION_ID>\n"
         << "    <VARIABLE_NAME>"
         << make_escaped_string(&audit_record.event->variable_name)
         << "</VARIABLE_NAME>\n"
         << "    <VARIABLE_VALUE>"
         << make_escaped_string(&audit_record.event->variable_value)
         << "</VARIABLE_VALUE>\n"
         << "  </AUDIT_RECORD>\n";

  return result.str();
}

AuditRecordString LogRecordFormatterNew::apply(
    const AuditRecordServerStartup &audit_record) const noexcept {
  std::stringstream result;
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
  std::stringstream startup_options;

  for (unsigned int i = 0; i < audit_record.event->argc; ++i) {
    if (audit_record.event->argv[i] != nullptr) {
      startup_options << audit_record.event->argv[i] << " ";
    }
  }

  std::string startup_options_str = startup_options.str();
  startup_options_str.pop_back();

  result << "  <AUDIT_RECORD>\n"
         << "    <NAME>" << event_class_to_string(audit_record.event_class)
         << "</NAME>\n"
         << "    <RECORD_ID>" << make_record_id(tp) << "</RECORD_ID>\n"
         << "    <TIMESTAMP>" << make_timestamp(tp) << "</TIMESTAMP>\n"
         << "    <STARTUP_OPTIONS>" << make_escaped_string(startup_options_str)
         << "</STARTUP_OPTIONS>\n"
         << "  </AUDIT_RECORD>\n";

  return result.str();
}

AuditRecordString LogRecordFormatterNew::apply(
    const AuditRecordServerShutdown &audit_record) const noexcept {
  std::stringstream result;
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();

  result << "  <AUDIT_RECORD>\n"
         << "    <NAME>" << event_class_to_string(audit_record.event_class)
         << "</NAME>\n"
         << "    <RECORD_ID>" << make_record_id(tp) << "</RECORD_ID>\n"
         << "    <TIMESTAMP>" << make_timestamp(tp) << "</TIMESTAMP>\n"
         << "    <STATUS>" << audit_record.event->exit_code << "</STATUS>\n"
         << "    <SHUTDOWN_REASON>"
         << shutdown_reason_to_string(audit_record.event->reason)
         << "</SHUTDOWN_REASON>\n"
         << "  </AUDIT_RECORD>\n";

  return result.str();
}

AuditRecordString LogRecordFormatterNew::apply(
    const AuditRecordCommand &audit_record) const noexcept {
  std::stringstream result;
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();

  result << "  <AUDIT_RECORD>\n"
         << "    <NAME>"
         << event_subclass_to_string(audit_record.event->event_subclass)
         << "</NAME>\n"
         << "    <RECORD_ID>" << make_record_id(tp) << "</RECORD_ID>\n"
         << "    <TIMESTAMP>" << make_timestamp(tp) << "</TIMESTAMP>\n"
         << "    <STATUS>" << audit_record.event->status << "</STATUS>\n"
         << "    <CONNECTION_ID>" << audit_record.event->connection_id
         << "</CONNECTION_ID>\n"
         << "    <COMMAND_CLASS>"
         << command_id_to_string(audit_record.event->command_id)
         << "</COMMAND_CLASS>\n"
         << "  </AUDIT_RECORD>\n";

  return result.str();
}

AuditRecordString LogRecordFormatterNew::apply(
    const AuditRecordQuery &audit_record) const noexcept {
  std::stringstream result;
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();

  result << "  <AUDIT_RECORD>\n"
         << "    <NAME>"
         << event_subclass_to_string(audit_record.event->event_subclass)
         << "</NAME>\n"
         << "    <RECORD_ID>" << make_record_id(tp) << "</RECORD_ID>\n"
         << "    <TIMESTAMP>" << make_timestamp(tp) << "</TIMESTAMP>\n"
         << "    <STATUS>" << audit_record.event->status << "</STATUS>\n"
         << "    <CONNECTION_ID>" << audit_record.event->connection_id
         << "</CONNECTION_ID>\n"
         << "    <COMMAND_CLASS>"
         << sql_command_id_to_string(audit_record.event->sql_command_id)
         << "</COMMAND_CLASS>\n"
         << "    <SQLTEXT>"
         << (audit_record.extended_info.digest.empty()
                 ? make_escaped_string(&audit_record.event->query)
                 : make_escaped_string(audit_record.extended_info.digest))
         << "</SQLTEXT>\n"
         << "  </AUDIT_RECORD>\n";

  return result.str();
}

AuditRecordString LogRecordFormatterNew::apply(
    const AuditRecordStoredProgram &audit_record) const noexcept {
  std::stringstream result;
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();

  result << "  <AUDIT_RECORD>\n"
         << "    <NAME>" << event_class_to_string(audit_record.event_class)
         << "</NAME>\n"
         << "    <RECORD_ID>" << make_record_id(tp) << "</RECORD_ID>\n"
         << "    <TIMESTAMP>" << make_timestamp(tp) << "</TIMESTAMP>\n"
         << "    <CONNECTION_ID>" << audit_record.event->connection_id
         << "</CONNECTION_ID>\n"
         << "    <COMMAND_CLASS>"
         << sql_command_id_to_string(audit_record.event->sql_command_id)
         << "</COMMAND_CLASS>\n"
         << "    <SQLTEXT>"
         << (audit_record.extended_info.digest.empty()
                 ? make_escaped_string(&audit_record.event->query)
                 : make_escaped_string(audit_record.extended_info.digest))
         << "</SQLTEXT>\n"
         << "    <DB>" << make_escaped_string(&audit_record.event->database)
         << "</DB>\n"
         << "    <STORED_PROGRAM>"
         << make_escaped_string(&audit_record.event->name)
         << "</STORED_PROGRAM>\n"
         << "  </AUDIT_RECORD>\n";

  return result.str();
}

AuditRecordString LogRecordFormatterNew::apply(
    const AuditRecordAuthentication &audit_record) const noexcept {
  std::stringstream result;
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();

  result << "  <AUDIT_RECORD>\n"
         << "    <NAME>"
         << event_subclass_to_string(audit_record.event->event_subclass)
         << "</NAME>\n"
         << "    <RECORD_ID>" << make_record_id(tp) << "</RECORD_ID>\n"
         << "    <TIMESTAMP>" << make_timestamp(tp) << "</TIMESTAMP>\n"
         << "    <CONNECTION_ID>" << audit_record.event->connection_id
         << "</CONNECTION_ID>\n"
         << "    <COMMAND_CLASS>"
         << sql_command_id_to_string(audit_record.event->sql_command_id)
         << "</COMMAND_CLASS>\n"
         << "    <SQLTEXT>"
         << (audit_record.extended_info.digest.empty()
                 ? make_escaped_string(&audit_record.event->query)
                 : make_escaped_string(audit_record.extended_info.digest))
         << "</SQLTEXT>\n"
         << "    <STATUS>" << audit_record.event->status << "</STATUS>\n"
         << "    <USER>" << make_escaped_string(&audit_record.event->user)
         << "</USER>\n"
         << "    <HOST>" << make_escaped_string(&audit_record.event->host)
         << "</HOST>\n"
         << "    <AUTH_PLUGIN>"
         << make_escaped_string(&audit_record.event->authentication_plugin)
         << "</AUTH_PLUGIN>\n"
         << "    <NEW_USER>"
         << make_escaped_string(&audit_record.event->new_user)
         << "</NEW_USER>\n"
         << "    <NEW_HOST>"
         << make_escaped_string(&audit_record.event->new_host)
         << "</NEW_HOST>\n"
         << "  </AUDIT_RECORD>\n";

  return result.str();
}

AuditRecordString LogRecordFormatterNew::apply(
    const AuditRecordMessage &audit_record) const noexcept {
  std::stringstream result;
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();

  result << "  <AUDIT_RECORD>\n"
         << "    <NAME>" << event_class_to_string(audit_record.event_class)
         << "</NAME>\n"
         << "    <RECORD_ID>" << make_record_id(tp) << "</RECORD_ID>\n"
         << "    <TIMESTAMP>" << make_timestamp(tp) << "</TIMESTAMP>\n"
         << "    <COMPONENT>"
         << make_escaped_string(&audit_record.event->component)
         << "</COMPONENT>\n"
         << "    <PRODUCER>"
         << make_escaped_string(&audit_record.event->producer)
         << "</PRODUCER>\n"
         << "    <MESSAGE>" << make_escaped_string(&audit_record.event->message)
         << "</MESSAGE>\n"
         << "    <MESSAGE_ATTRIBUTES>\n";

  for (size_t i = 0; i < audit_record.event->key_value_map_length; ++i) {
    result << "      <ATTRIBUTE>\n"
           << "        <NAME>"
           << make_escaped_string(&audit_record.event->key_value_map[i].key)
           << "</NAME>\n";
    if (audit_record.event->key_value_map[i].value_type ==
        MYSQL_AUDIT_MESSAGE_VALUE_TYPE_STR) {
      result << "        <VALUE>"
             << make_escaped_string(
                    &audit_record.event->key_value_map[i].value.str)
             << "</VALUE>\n";
    } else if (audit_record.event->key_value_map[i].value_type ==
               MYSQL_AUDIT_MESSAGE_VALUE_TYPE_NUM) {
      result << "        <VALUE>"
             << audit_record.event->key_value_map[i].value.num << "</VALUE>\n";
    } else {
      result << "        <VALUE></VALUE>";
    }
    result << "      </ATTRIBUTE>\n";
  }

  result << "    </MESSAGE_ATTRIBUTES>\n"
         << "  </AUDIT_RECORD>\n";

  return result.str();
}

AuditRecordString LogRecordFormatterNew::apply(
    const AuditRecordStartAudit &audit_record) const noexcept {
  std::stringstream result;
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();

  result << "  <AUDIT_RECORD>\n"
         << "    <NAME>"
         << event_subclass_to_string(audit_record.event->event_subclass)
         << "</NAME>\n"
         << "    <RECORD_ID>" << make_record_id(tp) << "</RECORD_ID>\n"
         << "    <TIMESTAMP>" << make_timestamp(tp) << "</TIMESTAMP>\n"
         << "    <SERVER_ID>" << audit_record.event->server_id
         << "</SERVER_ID>\n"
         << "  </AUDIT_RECORD>\n";

  return result.str();
}

AuditRecordString LogRecordFormatterNew::apply(
    const AuditRecordStopAudit &audit_record) const noexcept {
  std::stringstream result;
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();

  result << "  <AUDIT_RECORD>\n"
         << "    <NAME>"
         << event_subclass_to_string(audit_record.event->event_subclass)
         << "</NAME>\n"
         << "    <RECORD_ID>" << make_record_id(tp) << "</RECORD_ID>\n"
         << "    <TIMESTAMP>" << make_timestamp(tp) << "</TIMESTAMP>\n"
         << "    <SERVER_ID>" << audit_record.event->server_id
         << "</SERVER_ID>\n"
         << "  </AUDIT_RECORD>\n";

  return result.str();
}

void LogRecordFormatterNew::apply_debug_info(
    std::string_view event_class_name, std::string_view event_subclass_name,
    std::string &record_str) noexcept {
  assert(!record_str.empty());

  std::stringstream debug_info;
  debug_info << "    <EVENT_CLASS_NAME>" << event_class_name
             << "</EVENT_CLASS_NAME>\n"
             << "    <EVENT_SUBCLASS_NAME>" << event_subclass_name
             << "</EVENT_SUBCLASS_NAME>\n";

  std::string insert_after_tag{"<AUDIT_RECORD>\n"};
  auto tag_begin = record_str.find(insert_after_tag, 0);
  record_str.insert(tag_begin + insert_after_tag.length(), debug_info.str());
}

std::string LogRecordFormatterNew::extra_attrs_to_string(
    const ExtendedInfo &info) const noexcept {
  std::stringstream result;
  auto attrs_it = info.attrs.find("connection_attributes");

  if (attrs_it != info.attrs.cend()) {
    result << "    <CONNECTION_ATTRIBUTES>\n";

    for (const auto &name_value : attrs_it->second) {
      result << "      <ATTRIBUTE>\n"
             << "        <NAME>" << make_escaped_string(name_value.first)
             << "</NAME>\n"
             << "        <VALUE>" << make_escaped_string(name_value.second)
             << "</VALUE>\n"
             << "      </ATTRIBUTE>\n";
    }

    result << "    </CONNECTION_ATTRIBUTES>";
  }

  return result.str();
}

}  // namespace audit_log_filter::log_record_formatter
