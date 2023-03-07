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

#include "plugin/audit_log_filter/log_record_formatter/old.h"
#include "plugin/audit_log_filter/audit_record.h"

#include <chrono>
#include <sstream>

namespace audit_log_filter::log_record_formatter {

AuditRecordString LogRecordFormatterOld::apply(
    const AuditRecordGeneral &audit_record) const noexcept {
  std::stringstream result;
  std::chrono::system_clock::time_point tp{
      std::chrono::seconds{audit_record.event->general_time}};

  result << "  <AUDIT_RECORD\n"
         << "    NAME=\""
         << make_escaped_string(&audit_record.event->general_command) << "\"\n"
         << "    RECORD_ID=\"" << make_record_id(tp) << "\"\n"
         << "    TIMESTAMP=\"" << make_timestamp(tp) << "\"\n"
         << "    COMMAND_CLASS=\""
         << make_escaped_string(&audit_record.event->general_sql_command)
         << "\"\n"
         << "    CONNECTION_ID=\"" << audit_record.event->general_thread_id
         << "\"\n"
         << "    HOST=\""
         << make_escaped_string(&audit_record.event->general_host) << "\"\n"
         << "    IP=\"" << make_escaped_string(&audit_record.event->general_ip)
         << "\"\n"
         << "    USER=\""
         << make_escaped_string(&audit_record.event->general_user) << "\"\n"
         << "    OS_LOGIN=\""
         << make_escaped_string(&audit_record.event->general_external_user)
         << "\"\n"
         << "    SQLTEXT=\""
         << (audit_record.extended_info.digest.empty()
                 ? make_escaped_string(&audit_record.event->general_query)
                 : make_escaped_string(audit_record.extended_info.digest))
         << "\"\n"
         << "    STATUS=\"" << audit_record.event->general_error_code
         << "\"/>\n";

  return result.str();
}

AuditRecordString LogRecordFormatterOld::apply(
    const AuditRecordConnection &audit_record) const noexcept {
  std::stringstream result;
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();

  result << "  <AUDIT_RECORD\n"
         << "    NAME=\""
         << event_subclass_to_string(audit_record.event->event_subclass)
         << "\"\n"
         << "    RECORD_ID=\"" << make_record_id(tp) << "\"\n"
         << "    TIMESTAMP=\"" << make_timestamp(tp) << "\"\n"
         << "    COMMAND_CLASS=\""
         << event_class_to_string(audit_record.event_class) << "\"\n"
         << "    CONNECTION_ID=\"" << audit_record.event->connection_id
         << "\"\n"
         << "    HOST=\"" << make_escaped_string(&audit_record.event->host)
         << "\"\n"
         << "    IP=\"" << make_escaped_string(&audit_record.event->ip)
         << "\"\n"
         << "    USER=\"" << make_escaped_string(&audit_record.event->user)
         << "\"\n"
         << "    OS_LOGIN=\""
         << make_escaped_string(&audit_record.event->external_user) << "\"\n"
         << "    PRIV_USER=\""
         << make_escaped_string(&audit_record.event->priv_user) << "\"\n"
         << "    PROXY_USER=\""
         << make_escaped_string(&audit_record.event->proxy_user) << "\"\n"
         << "    DB=\"" << make_escaped_string(&audit_record.event->database)
         << "\"\n"
         << "    STATUS=\"" << audit_record.event->status << "\"\n"
         << "    CONNECTION_TYPE=\""
         << connection_type_name_to_string(audit_record.event->connection_type)
         << "\"/>\n";

  return result.str();
}

AuditRecordString LogRecordFormatterOld::apply(
    const AuditRecordTableAccess &audit_record) const noexcept {
  std::stringstream result;
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();

  result << "  <AUDIT_RECORD\n"
         << "    NAME=\""
         << event_subclass_to_string(audit_record.event->event_subclass)
         << "\"\n"
         << "    RECORD_ID=\"" << make_record_id(tp) << "\"\n"
         << "    TIMESTAMP=\"" << make_timestamp(tp) << "\"\n"
         << "    COMMAND_CLASS=\""
         << sql_command_id_to_string(audit_record.event->sql_command_id)
         << "\"\n"
         << "    CONNECTION_ID=\"" << audit_record.event->connection_id
         << "\"\n"
         << "    SQLTEXT=\""
         << (audit_record.extended_info.digest.empty()
                 ? make_escaped_string(&audit_record.event->query)
                 : make_escaped_string(audit_record.extended_info.digest))
         << "\"\n"
         << "    DB=\""
         << make_escaped_string(&audit_record.event->table_database) << "\"\n"
         << "    TABLE=\""
         << make_escaped_string(&audit_record.event->table_name) << "\"/>\n";

  return result.str();
}

AuditRecordString LogRecordFormatterOld::apply(
    const AuditRecordGlobalVariable &audit_record) const noexcept {
  std::stringstream result;
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();

  result << "  <AUDIT_RECORD\n"
         << "    NAME=\""
         << event_subclass_to_string(audit_record.event->event_subclass)
         << "\"\n"
         << "    RECORD_ID=\"" << make_record_id(tp) << "\"\n"
         << "    TIMESTAMP=\"" << make_timestamp(tp) << "\"\n"
         << "    COMMAND_CLASS=\""
         << sql_command_id_to_string(audit_record.event->sql_command_id)
         << "\"\n"
         << "    CONNECTION_ID=\"" << audit_record.event->connection_id
         << "\"\n"
         << "    VARIABLE_NAME=\""
         << make_escaped_string(&audit_record.event->variable_name) << "\"\n"
         << "    VARIABLE_VALUE=\""
         << make_escaped_string(&audit_record.event->variable_value)
         << "\"/>\n";

  return result.str();
}

AuditRecordString LogRecordFormatterOld::apply(
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

  result << "  <AUDIT_RECORD\n"
         << "    NAME=\"" << event_class_to_string(audit_record.event_class)
         << "\"\n"
         << "    RECORD_ID=\"" << make_record_id(tp) << "\"\n"
         << "    TIMESTAMP=\"" << make_timestamp(tp) << "\"\n"
         << "    STARTUP_OPTIONS=\"" << make_escaped_string(startup_options_str)
         << "\"/>\n";

  return result.str();
}

AuditRecordString LogRecordFormatterOld::apply(
    const AuditRecordServerShutdown &audit_record) const noexcept {
  std::stringstream result;
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();

  result << "  <AUDIT_RECORD\n"
         << "    NAME=\"" << event_class_to_string(audit_record.event_class)
         << "\"\n"
         << "    RECORD_ID=\"" << make_record_id(tp) << "\"\n"
         << "    TIMESTAMP=\"" << make_timestamp(tp) << "\"\n"
         << "    STATUS=\"" << audit_record.event->exit_code << "\"\n"
         << "    SHUTDOWN_REASON=\""
         << shutdown_reason_to_string(audit_record.event->reason) << "\"/>\n";

  return result.str();
}

AuditRecordString LogRecordFormatterOld::apply(
    const AuditRecordCommand &audit_record) const noexcept {
  std::stringstream result;
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();

  result << "  <AUDIT_RECORD\n"
         << "    NAME=\""
         << event_subclass_to_string(audit_record.event->event_subclass)
         << "\"\n"
         << "    RECORD_ID=\"" << make_record_id(tp) << "\"\n"
         << "    TIMESTAMP=\"" << make_timestamp(tp) << "\"\n"
         << "    STATUS=\"" << audit_record.event->status << "\"\n"
         << "    CONNECTION_ID=\"" << audit_record.event->connection_id
         << "\"\n"
         << "    COMMAND_CLASS=\""
         << command_id_to_string(audit_record.event->command_id) << "\"/>\n";

  return result.str();
}

AuditRecordString LogRecordFormatterOld::apply(
    const AuditRecordQuery &audit_record) const noexcept {
  std::stringstream result;
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();

  result << "  <AUDIT_RECORD\n"
         << "    NAME=\""
         << event_subclass_to_string(audit_record.event->event_subclass)
         << "\"\n"
         << "    RECORD_ID=\"" << make_record_id(tp) << "\"\n"
         << "    TIMESTAMP=\"" << make_timestamp(tp) << "\"\n"
         << "    STATUS=\"" << audit_record.event->status << "\"\n"
         << "    CONNECTION_ID=\"" << audit_record.event->connection_id
         << "\"\n"
         << "    COMMAND_CLASS=\""
         << sql_command_id_to_string(audit_record.event->sql_command_id)
         << "\"\n"
         << "    SQLTEXT=\""
         << (audit_record.extended_info.digest.empty()
                 ? make_escaped_string(&audit_record.event->query)
                 : make_escaped_string(audit_record.extended_info.digest))
         << "\"/>\n";

  return result.str();
}

AuditRecordString LogRecordFormatterOld::apply(
    const AuditRecordStoredProgram &audit_record) const noexcept {
  std::stringstream result;
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();

  result << "  <AUDIT_RECORD\n"
         << "    NAME=\"" << event_class_to_string(audit_record.event_class)
         << "\"\n"
         << "    RECORD_ID=\"" << make_record_id(tp) << "\"\n"
         << "    TIMESTAMP=\"" << make_timestamp(tp) << "\"\n"
         << "    CONNECTION_ID=\"" << audit_record.event->connection_id
         << "\"\n"
         << "    COMMAND_CLASS=\""
         << sql_command_id_to_string(audit_record.event->sql_command_id)
         << "\"\n"
         << "    SQLTEXT=\""
         << (audit_record.extended_info.digest.empty()
                 ? make_escaped_string(&audit_record.event->query)
                 : make_escaped_string(audit_record.extended_info.digest))
         << "\"\n"
         << "    DB=\"" << make_escaped_string(&audit_record.event->database)
         << "\"\n"
         << "    STORED_PROGRAM=\""
         << make_escaped_string(&audit_record.event->name) << "\"/>\n";

  return result.str();
}

AuditRecordString LogRecordFormatterOld::apply(
    const AuditRecordAuthentication &audit_record) const noexcept {
  std::stringstream result;
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();

  result << "  <AUDIT_RECORD\n"
         << "    NAME=\""
         << event_subclass_to_string(audit_record.event->event_subclass)
         << "\"\n"
         << "    RECORD_ID=\"" << make_record_id(tp) << "\"\n"
         << "    TIMESTAMP=\"" << make_timestamp(tp) << "\"\n"
         << "    CONNECTION_ID=\"" << audit_record.event->connection_id
         << "\"\n"
         << "    COMMAND_CLASS=\""
         << sql_command_id_to_string(audit_record.event->sql_command_id)
         << "\"\n"
         << "    SQLTEXT=\""
         << (audit_record.extended_info.digest.empty()
                 ? make_escaped_string(&audit_record.event->query)
                 : make_escaped_string(audit_record.extended_info.digest))
         << "\"\n"
         << "    STATUS=\"" << audit_record.event->status << "\"\n"
         << "    USER=\"" << make_escaped_string(&audit_record.event->user)
         << "\"\n"
         << "    HOST=\"" << make_escaped_string(&audit_record.event->host)
         << "\"\n"
         << "    AUTH_PLUGIN=\""
         << make_escaped_string(&audit_record.event->authentication_plugin)
         << "\"\n"
         << "    NEW_USER=\""
         << make_escaped_string(&audit_record.event->new_user) << "\"\n"
         << "    NEW_HOST=\""
         << make_escaped_string(&audit_record.event->new_host) << "\"/>\n";

  return result.str();
}

AuditRecordString LogRecordFormatterOld::apply(
    const AuditRecordMessage &audit_record) const noexcept {
  std::stringstream result;
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();

  result << "  <AUDIT_RECORD\n"
         << "    NAME=\"" << event_class_to_string(audit_record.event_class)
         << "\"\n"
         << "    RECORD_ID=\"" << make_record_id(tp) << "\"\n"
         << "    TIMESTAMP=\"" << make_timestamp(tp) << "\"\n"
         << "    COMPONENT=\""
         << make_escaped_string(&audit_record.event->component) << "\"\n"
         << "    PRODUCER=\""
         << make_escaped_string(&audit_record.event->producer) << "\"\n"
         << "    MESSAGE=\""
         << make_escaped_string(&audit_record.event->message) << "\"\n"
         << "    MESSAGE_ATTRIBUTES=\"";

  for (size_t i = 0; i < audit_record.event->key_value_map_length; ++i) {
    result << make_escaped_string(&audit_record.event->key_value_map[i].key)
           << "=";
    if (audit_record.event->key_value_map[i].value_type ==
        MYSQL_AUDIT_MESSAGE_VALUE_TYPE_STR) {
      result << make_escaped_string(
          &audit_record.event->key_value_map[i].value.str);
    } else if (audit_record.event->key_value_map[i].value_type ==
               MYSQL_AUDIT_MESSAGE_VALUE_TYPE_NUM) {
      result << audit_record.event->key_value_map[i].value.num;
    }

    result << " ";
  }

  result << "\"/>\n";

  return result.str();
}

AuditRecordString LogRecordFormatterOld::apply(
    const AuditRecordStartAudit &audit_record) const noexcept {
  std::stringstream result;
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();

  result << "  <AUDIT_RECORD\n"
         << "    NAME=\""
         << event_subclass_to_string(audit_record.event->event_subclass)
         << "\"\n"
         << "    RECORD_ID=\"" << make_record_id(tp) << "\"\n"
         << "    TIMESTAMP=\"" << make_timestamp(tp) << "\"\n"
         << "    SERVER_ID=\"" << audit_record.event->server_id << "\"/>\n";

  return result.str();
}

AuditRecordString LogRecordFormatterOld::apply(
    const AuditRecordStopAudit &audit_record) const noexcept {
  std::stringstream result;
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();

  result << "  <AUDIT_RECORD\n"
         << "    NAME=\""
         << event_subclass_to_string(audit_record.event->event_subclass)
         << "\"\n"
         << "    RECORD_ID=\"" << make_record_id(tp) << "\"\n"
         << "    TIMESTAMP=\"" << make_timestamp(tp) << "\"\n"
         << "    SERVER_ID=\"" << audit_record.event->server_id << "\"/>\n";

  return result.str();
}

void LogRecordFormatterOld::apply_debug_info(
    std::string_view event_class_name, std::string_view event_subclass_name,
    std::string &record_str) noexcept {
  assert(!record_str.empty());

  std::stringstream debug_info;
  debug_info << "    EVENT_CLASS_NAME=\"" << event_class_name << "\"\n"
             << "    EVENT_SUBCLASS_NAME=\"" << event_subclass_name << "\"\n";

  std::string insert_after_tag{"<AUDIT_RECORD\n"};
  auto tag_begin = record_str.find(insert_after_tag, 0);
  record_str.insert(tag_begin + insert_after_tag.length(), debug_info.str());
}

std::string LogRecordFormatterOld::extra_attrs_to_string(
    const ExtendedInfo &info [[maybe_unused]]) const noexcept {
  return "";
}

}  // namespace audit_log_filter::log_record_formatter
