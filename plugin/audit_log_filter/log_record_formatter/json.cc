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

#include "plugin/audit_log_filter/log_record_formatter/json.h"
#include "plugin/audit_log_filter/sys_vars.h"

#include <chrono>
#include <iomanip>
#include <sstream>

namespace audit_log_filter::log_record_formatter {
namespace {

const std::string_view kAuditNameUnknown{"unknown"};

const std::string_view kAuditEventNameGeneralLog{"log"};
const std::string_view kAuditEventNameGeneralError{"error"};
const std::string_view kAuditEventNameGeneralResult{"result"};
const std::string_view kAuditEventNameGeneralStatus{"status"};

const std::string_view kAuditEventNameAccessRead{"read"};
const std::string_view kAuditEventNameAccessInsert{"insert"};
const std::string_view kAuditEventNameAccessUpdate{"update"};
const std::string_view kAuditEventNameAccessDelete{"delete"};

const std::string_view kAuditEventNameConnect{"connect"};
const std::string_view kAuditEventNameDisconnect{"disconnect"};
const std::string_view kAuditEventNameChangeUser{"change_user"};
const std::string_view kAuditEventNamePreAuth{"pre_authenticate"};

const std::string_view kAuditEventNameGlobalVariableGet{"variable_get"};
const std::string_view kAuditEventNameGlobalVariableSet{"variable_set"};

const std::string_view kAuditEventNameCommandStart{"command_start"};
const std::string_view kAuditEventNameCommandEnd{"command_end"};

const std::string_view kAuditEventNameQueryStart{"query_start"};
const std::string_view kAuditEventNameQueryNestedStart{"query_nested_start"};
const std::string_view kAuditEventNameQueryStatusEnd{"query_status_end"};
const std::string_view kAuditEventNameQueryNestedStatusEnd{
    "query_nested_status_end"};

const std::string_view kAuditEventNameAuthFlush{"auth_flush"};
const std::string_view kAuditEventNameAuthAuthidCreate{"auth_authid_create"};
const std::string_view kAuditEventNameAuthCredentialChange{
    "auth_credential_change"};
const std::string_view kAuditEventNameAuthAuthidRename{"auth_authid_rename"};
const std::string_view kAuditEventNameAuthAuthidDrop{"auth_authid_drop"};

const std::string_view kAuditEventNameServerStartupStartup{"startup"};

const std::string_view kAuditEventNameServerShutdownShutdown{"shutdown"};

const std::string_view kAuditEventNameStoredProgramExecute{"execute"};

const std::string_view kAuditEventNameMessageInternal{"internal"};
const std::string_view kAuditEventNameMessageUser{"user"};

const std::string_view kAuditNameShutdownReasonShutdown{"shutdown"};
const std::string_view kAuditNameShutdownReasonAbort{"abort"};

const std::string_view kAuditConnectionTypeNameUndef{"undefined"};
const std::string_view kAuditConnectionTypeNameTcpip{"tcp/ip"};
const std::string_view kAuditConnectionTypeNameSock{"socket"};
const std::string_view kAuditConnectionTypeNamePipe{"named_pipe"};
const std::string_view kAuditConnectionTypeNameSsl{"ssl"};
const std::string_view kAuditConnectionTypeNameShared{"shared_memory"};

const std::string_view kAuditEventNameAuditStart{"audit"};
const std::string_view kAuditEventNameAuditStop{"noaudit"};

}  // namespace

std::string_view LogRecordFormatterJson::event_subclass_to_string(
    mysql_event_general_subclass_t event_subclass) const noexcept {
  switch (event_subclass) {
    case MYSQL_AUDIT_GENERAL_LOG:
      return kAuditEventNameGeneralLog;
    case MYSQL_AUDIT_GENERAL_ERROR:
      return kAuditEventNameGeneralError;
    case MYSQL_AUDIT_GENERAL_RESULT:
      return kAuditEventNameGeneralResult;
    case MYSQL_AUDIT_GENERAL_STATUS:
      return kAuditEventNameGeneralStatus;
    default:
      assert(false);
  }

  return kAuditNameUnknown;
}

std::string_view LogRecordFormatterJson::event_subclass_to_string(
    mysql_event_table_access_subclass_t event_subclass) const noexcept {
  switch (event_subclass) {
    case MYSQL_AUDIT_TABLE_ACCESS_READ:
      return kAuditEventNameAccessRead;
    case MYSQL_AUDIT_TABLE_ACCESS_INSERT:
      return kAuditEventNameAccessInsert;
    case MYSQL_AUDIT_TABLE_ACCESS_UPDATE:
      return kAuditEventNameAccessUpdate;
    case MYSQL_AUDIT_TABLE_ACCESS_DELETE:
      return kAuditEventNameAccessDelete;
    default:
      assert(false);
  }

  return kAuditNameUnknown;
}

std::string_view LogRecordFormatterJson::event_subclass_to_string(
    mysql_event_connection_subclass_t event_subclass) const noexcept {
  switch (event_subclass) {
    case MYSQL_AUDIT_CONNECTION_CONNECT:
      return kAuditEventNameConnect;
    case MYSQL_AUDIT_CONNECTION_DISCONNECT:
      return kAuditEventNameDisconnect;
    case MYSQL_AUDIT_CONNECTION_CHANGE_USER:
      return kAuditEventNameChangeUser;
    case MYSQL_AUDIT_CONNECTION_PRE_AUTHENTICATE:
      return kAuditEventNamePreAuth;
    default:
      assert(false);
  }

  return kAuditNameUnknown;
}

std::string_view LogRecordFormatterJson::event_subclass_to_string(
    mysql_event_global_variable_subclass_t event_subclass) const noexcept {
  switch (event_subclass) {
    case MYSQL_AUDIT_GLOBAL_VARIABLE_GET:
      return kAuditEventNameGlobalVariableGet;
    case MYSQL_AUDIT_GLOBAL_VARIABLE_SET:
      return kAuditEventNameGlobalVariableSet;
    default:
      assert(false);
  }

  return kAuditNameUnknown;
}

std::string_view LogRecordFormatterJson::event_subclass_to_string(
    mysql_event_command_subclass_t event_subclass) const noexcept {
  switch (event_subclass) {
    case MYSQL_AUDIT_COMMAND_START:
      return kAuditEventNameCommandStart;
    case MYSQL_AUDIT_COMMAND_END:
      return kAuditEventNameCommandEnd;
    default:
      assert(false);
  }

  return kAuditNameUnknown;
}

std::string_view LogRecordFormatterJson::event_subclass_to_string(
    mysql_event_query_subclass_t event_subclass) const noexcept {
  switch (event_subclass) {
    case MYSQL_AUDIT_QUERY_START:
      return kAuditEventNameQueryStart;
    case MYSQL_AUDIT_QUERY_NESTED_START:
      return kAuditEventNameQueryNestedStart;
    case MYSQL_AUDIT_QUERY_STATUS_END:
      return kAuditEventNameQueryStatusEnd;
    case MYSQL_AUDIT_QUERY_NESTED_STATUS_END:
      return kAuditEventNameQueryNestedStatusEnd;
    default:
      assert(false);
  }

  return kAuditNameUnknown;
}

std::string_view LogRecordFormatterJson::event_subclass_to_string(
    mysql_event_authentication_subclass_t event_subclass) const noexcept {
  switch (event_subclass) {
    case MYSQL_AUDIT_AUTHENTICATION_FLUSH:
      return kAuditEventNameAuthFlush;
    case MYSQL_AUDIT_AUTHENTICATION_AUTHID_CREATE:
      return kAuditEventNameAuthAuthidCreate;
    case MYSQL_AUDIT_AUTHENTICATION_CREDENTIAL_CHANGE:
      return kAuditEventNameAuthCredentialChange;
    case MYSQL_AUDIT_AUTHENTICATION_AUTHID_RENAME:
      return kAuditEventNameAuthAuthidRename;
    case MYSQL_AUDIT_AUTHENTICATION_AUTHID_DROP:
      return kAuditEventNameAuthAuthidDrop;
    default:
      assert(false);
  }

  return kAuditNameUnknown;
}

std::string_view LogRecordFormatterJson::event_subclass_to_string(
    mysql_event_server_startup_subclass_t event_subclass) const noexcept {
  switch (event_subclass) {
    case MYSQL_AUDIT_SERVER_STARTUP_STARTUP:
      return kAuditEventNameServerStartupStartup;
    default:
      assert(false);
  }

  return kAuditNameUnknown;
}

std::string_view LogRecordFormatterJson::event_subclass_to_string(
    mysql_event_server_shutdown_subclass_t event_subclass) const noexcept {
  switch (event_subclass) {
    case MYSQL_AUDIT_SERVER_SHUTDOWN_SHUTDOWN:
      return kAuditEventNameServerShutdownShutdown;
    default:
      assert(false);
  }

  return kAuditNameUnknown;
}

std::string_view LogRecordFormatterJson::event_subclass_to_string(
    mysql_event_stored_program_subclass_t event_subclass) const noexcept {
  switch (event_subclass) {
    case MYSQL_AUDIT_STORED_PROGRAM_EXECUTE:
      return kAuditEventNameStoredProgramExecute;
    default:
      assert(false);
  }

  return kAuditNameUnknown;
}

std::string_view LogRecordFormatterJson::event_subclass_to_string(
    mysql_event_message_subclass_t event_subclass) const noexcept {
  switch (event_subclass) {
    case MYSQL_AUDIT_MESSAGE_INTERNAL:
      return kAuditEventNameMessageInternal;
    case MYSQL_AUDIT_MESSAGE_USER:
      return kAuditEventNameMessageUser;
    default:
      assert(false);
  }

  return kAuditNameUnknown;
}

std::string_view LogRecordFormatterJson::event_subclass_to_string(
    audit_filter_event_subclass_t event_subclass) const noexcept {
  switch (event_subclass) {
    case audit_filter_event_subclass_t::AUDIT_FILTER_INTERNAL_AUDIT:
      return kAuditEventNameAuditStart;
    case audit_filter_event_subclass_t::AUDIT_FILTER_INTERNAL_NOAUDIT:
      return kAuditEventNameAuditStop;
    default:
      assert(false);
  }

  return kAuditNameUnknown;
}

std::string_view LogRecordFormatterJson::connection_type_name_to_string(
    int connection_type) const noexcept {
  switch (connection_type) {
    case 0:
      return kAuditConnectionTypeNameUndef;
    case 1:
      return kAuditConnectionTypeNameTcpip;
    case 2:
      return kAuditConnectionTypeNameSock;
    case 3:
      return kAuditConnectionTypeNamePipe;
    case 4:
      return kAuditConnectionTypeNameSsl;
    case 5:
      return kAuditConnectionTypeNameShared;
    default:
      assert(false);
  }

  return kAuditNameUnknown;
}

std::string_view LogRecordFormatterJson::shutdown_reason_to_string(
    mysql_server_shutdown_reason_t reason) const noexcept {
  switch (reason) {
    case MYSQL_AUDIT_SERVER_SHUTDOWN_REASON_SHUTDOWN:
      return kAuditNameShutdownReasonShutdown;
    case MYSQL_AUDIT_SERVER_SHUTDOWN_REASON_ABORT:
      return kAuditNameShutdownReasonAbort;
    default:
      assert(false);
  }

  return kAuditNameUnknown;
}

AuditRecordString LogRecordFormatterJson::apply(
    const AuditRecordGeneral &audit_record) const noexcept {
  std::stringstream result;
  const auto timestamp = make_timestamp(std::chrono::system_clock::time_point{
      std::chrono::seconds{audit_record.event->general_time}});
  const auto rec_id = make_record_id();

  result << "  {\n"
         << R"(    "timestamp": ")" << timestamp << "\",\n"
         << R"(    "id": )" << rec_id << ",\n"
         << R"(    "class": "general",)"
         << "\n"
         << R"(    "event": ")"
         << event_subclass_to_string(audit_record.event->event_subclass)
         << "\",\n"
         << R"(    "connection_id": )" << audit_record.event->general_thread_id
         << ",\n"
         << R"(    "account": { "user": ")"
         << make_escaped_string(&audit_record.event->general_user)
         << R"(", "host": ")"
         << make_escaped_string(&audit_record.event->general_host) << R"(" },)"
         << "\n"
         << R"(    "login": { "user": ")"
         << make_escaped_string(&audit_record.event->general_user)
         << R"(", "os": ")"
         << make_escaped_string(&audit_record.event->general_external_user)
         << R"(", "ip": ")"
         << make_escaped_string(&audit_record.event->general_ip)
         << R"(", "proxy": "")"
         << " },\n"
         << R"(    "general_data": {)"
         << "\n"
         << R"(      "command": ")"
         << make_escaped_string(&audit_record.event->general_command) << "\",\n"
         << R"(      "sql_command": ")"
         << make_escaped_string(&audit_record.event->general_sql_command)
         << "\",\n"
         << R"(      "query": ")"
         << (audit_record.extended_info.digest.empty()
                 ? make_escaped_string(&audit_record.event->general_query)
                 : make_escaped_string(audit_record.extended_info.digest))
         << "\",\n"
         << R"(      "status": )" << audit_record.event->general_error_code
         << "}" << extra_attrs_to_string(audit_record.extended_info) << "\n  }";

  SysVars::update_log_bookmark(rec_id, timestamp);

  return result.str();
}

AuditRecordString LogRecordFormatterJson::apply(
    const AuditRecordConnection &audit_record) const noexcept {
  std::stringstream result;
  const auto timestamp = make_timestamp(std::chrono::system_clock::now());
  const auto rec_id = make_record_id();

  result << "  {\n"
         << R"(    "timestamp": ")" << timestamp << "\",\n"
         << R"(    "id": )" << rec_id << ",\n"
         << R"(    "class": "connection",)"
         << "\n"
         << R"(    "event": ")"
         << event_subclass_to_string(audit_record.event->event_subclass)
         << "\",\n"
         << R"(    "connection_id": )" << audit_record.event->connection_id
         << ",\n"
         << R"(    "account": { "user": ")"
         << make_escaped_string(&audit_record.event->user) << R"(", "host": ")"
         << make_escaped_string(&audit_record.event->host) << R"(" },)"
         << "\n"
         << R"(    "login": { "user": ")"
         << make_escaped_string(&audit_record.event->user) << R"(", "os": ")"
         << make_escaped_string(&audit_record.event->external_user)
         << R"(", "ip": ")" << make_escaped_string(&audit_record.event->ip)
         << R"(", "proxy": ")"
         << make_escaped_string(&audit_record.event->proxy_user) << R"(" },)"
         << "\n"
         << R"(    "connection_data": {)"
         << "\n"
         << R"(      "connection_type": ")"
         << connection_type_name_to_string(audit_record.event->connection_type)
         << "\",\n"
         << R"(      "status": )" << audit_record.event->status << ",\n"
         << R"(      "db": ")"
         << make_escaped_string(&audit_record.event->database) << "\""
         << "    }" << extra_attrs_to_string(audit_record.extended_info)
         << "\n  }";

  SysVars::update_log_bookmark(rec_id, timestamp);

  return result.str();
}

AuditRecordString LogRecordFormatterJson::apply(
    const AuditRecordTableAccess &audit_record) const noexcept {
  std::stringstream result;
  const auto timestamp = make_timestamp(std::chrono::system_clock::now());
  const auto rec_id = make_record_id();

  result << "  {\n"
         << R"(    "timestamp": ")" << timestamp << "\",\n"
         << R"(    "id": )" << rec_id << ",\n"
         << R"(    "class": "table_access",)"
         << "\n"
         << R"(    "event": ")"
         << event_subclass_to_string(audit_record.event->event_subclass)
         << "\",\n"
         << R"(    "connection_id": )" << audit_record.event->connection_id
         << ",\n"
         << R"(    "table_access_data": {)"
         << "\n"
         << R"(      "db": ")"
         << make_escaped_string(&audit_record.event->table_database) << "\",\n"
         << R"(      "table": ")"
         << make_escaped_string(&audit_record.event->table_name) << "\",\n"
         << R"(      "query": ")"
         << (audit_record.extended_info.digest.empty()
                 ? make_escaped_string(&audit_record.event->query)
                 : make_escaped_string(audit_record.extended_info.digest))
         << "\",\n"
         << R"(      "sql_command": ")"
         << sql_command_id_to_string(audit_record.event->sql_command_id)
         << "\"}" << extra_attrs_to_string(audit_record.extended_info)
         << "\n  }";

  SysVars::update_log_bookmark(rec_id, timestamp);

  return result.str();
}

AuditRecordString LogRecordFormatterJson::apply(
    const AuditRecordGlobalVariable &audit_record) const noexcept {
  std::stringstream result;
  const auto timestamp = make_timestamp(std::chrono::system_clock::now());
  const auto rec_id = make_record_id();

  result << "  {\n"
         << R"(    "timestamp": ")" << timestamp << "\",\n"
         << R"(    "id": )" << rec_id << ",\n"
         << R"(    "class": "global_variable",)"
         << "\n"
         << R"(    "event": ")"
         << event_subclass_to_string(audit_record.event->event_subclass)
         << "\",\n"
         << R"(    "connection_id": )" << audit_record.event->connection_id
         << ",\n"
         << R"(    "global_variable_data": {)"
         << "\n"
         << R"(      "name": ")"
         << make_escaped_string(&audit_record.event->variable_name) << "\",\n"
         << R"(      "value": ")"
         << make_escaped_string(&audit_record.event->variable_value) << "\",\n"
         << R"(      "sql_command": ")"
         << sql_command_id_to_string(audit_record.event->sql_command_id)
         << "\"}" << extra_attrs_to_string(audit_record.extended_info)
         << "\n  }";

  SysVars::update_log_bookmark(rec_id, timestamp);

  return result.str();
}

AuditRecordString LogRecordFormatterJson::apply(
    const AuditRecordServerStartup &audit_record) const noexcept {
  std::stringstream result;
  const auto timestamp = make_timestamp(std::chrono::system_clock::now());
  const auto rec_id = make_record_id();

  result << "  {\n"
         << R"(    "timestamp": ")" << timestamp << "\",\n"
         << R"(    "id": )" << rec_id << ",\n"
         << R"(    "class": "server_startup",)"
         << "\n"
         << R"(    "event": ")"
         << event_subclass_to_string(audit_record.event->event_subclass)
         << "\",\n"
         << R"(    "args": [)"
         << "\n";
  for (unsigned int i = 0; i < audit_record.event->argc; ++i) {
    if (audit_record.event->argv[i] != nullptr) {
      result << ((i == 0) ? "" : ",\n") << R"(      ")"
             << make_escaped_string(audit_record.event->argv[i]) << "\"";
    }
  }

  result << "\n     ]" << extra_attrs_to_string(audit_record.extended_info)
         << "\n  }";

  SysVars::update_log_bookmark(rec_id, timestamp);

  return result.str();
}

AuditRecordString LogRecordFormatterJson::apply(
    const AuditRecordServerShutdown &audit_record) const noexcept {
  std::stringstream result;
  const auto timestamp = make_timestamp(std::chrono::system_clock::now());
  const auto rec_id = make_record_id();

  result << "  {\n"
         << R"(    "timestamp": ")" << timestamp << "\",\n"
         << R"(    "id": )" << rec_id << ",\n"
         << R"(    "class": "server_shutdown",)"
         << "\n"
         << R"(    "event": ")"
         << event_subclass_to_string(audit_record.event->event_subclass)
         << "\",\n"
         << R"(    "server_shutdown_data": {)"
         << "\n"
         << R"(      "status": )" << audit_record.event->exit_code << ",\n"
         << R"(      "reason": ")"
         << shutdown_reason_to_string(audit_record.event->reason) << "\"}"
         << extra_attrs_to_string(audit_record.extended_info) << "\n  }";

  SysVars::update_log_bookmark(rec_id, timestamp);

  return result.str();
}

AuditRecordString LogRecordFormatterJson::apply(
    const AuditRecordCommand &audit_record) const noexcept {
  std::stringstream result;
  const auto timestamp = make_timestamp(std::chrono::system_clock::now());
  const auto rec_id = make_record_id();

  result << "  {\n"
         << R"(    "timestamp": ")" << timestamp << "\",\n"
         << R"(    "id": )" << rec_id << ",\n"
         << R"(    "class": "command",)"
         << "\n"
         << R"(    "event": ")"
         << event_subclass_to_string(audit_record.event->event_subclass)
         << "\",\n"
         << R"(    "connection_id": )" << audit_record.event->connection_id
         << ",\n"
         << R"(    "command_data": {)"
         << "\n"
         << R"(      "name": ")"
         << event_subclass_to_string(audit_record.event->event_subclass)
         << "\",\n"
         << R"(      "status": )" << audit_record.event->status << ",\n"
         << R"(      "command": ")"
         << command_id_to_string(audit_record.event->command_id) << "\"}"
         << extra_attrs_to_string(audit_record.extended_info) << "\n  }";

  SysVars::update_log_bookmark(rec_id, timestamp);

  return result.str();
}

AuditRecordString LogRecordFormatterJson::apply(
    const AuditRecordQuery &audit_record) const noexcept {
  std::stringstream result;
  const auto timestamp = make_timestamp(std::chrono::system_clock::now());
  const auto rec_id = make_record_id();

  result << "  {\n"
         << R"(    "timestamp": ")" << timestamp << "\",\n"
         << R"(    "id": )" << rec_id << ",\n"
         << R"(    "class": "query",)"
         << "\n"
         << R"(    "event": ")"
         << event_subclass_to_string(audit_record.event->event_subclass)
         << "\",\n"
         << R"(    "connection_id": )" << audit_record.event->connection_id
         << ",\n"
         << R"(    "query_data": {)"
         << "\n"
         << R"(      "query": ")"
         << (audit_record.extended_info.digest.empty()
                 ? make_escaped_string(&audit_record.event->query)
                 : make_escaped_string(audit_record.extended_info.digest))
         << "\",\n"
         << R"(      "status": )" << audit_record.event->status << ",\n"
         << R"(      "sql_command": ")"
         << sql_command_id_to_string(audit_record.event->sql_command_id)
         << "\"}" << extra_attrs_to_string(audit_record.extended_info)
         << "\n  }";

  SysVars::update_log_bookmark(rec_id, timestamp);

  return result.str();
}

AuditRecordString LogRecordFormatterJson::apply(
    const AuditRecordStoredProgram &audit_record) const noexcept {
  std::stringstream result;
  const auto timestamp = make_timestamp(std::chrono::system_clock::now());
  const auto rec_id = make_record_id();

  result << "  {\n"
         << R"(    "timestamp": ")" << timestamp << "\",\n"
         << R"(    "id": )" << rec_id << ",\n"
         << R"(    "class": "stored_program",)"
         << "\n"
         << R"(    "event": ")"
         << event_subclass_to_string(audit_record.event->event_subclass)
         << "\",\n"
         << R"(    "connection_id": )" << audit_record.event->connection_id
         << ",\n"
         << R"(    "stored_program_data": {)"
         << "\n"
         << R"(      "name": ")"
         << make_escaped_string(&audit_record.event->name) << "\",\n"
         << R"(      "db": ")"
         << make_escaped_string(&audit_record.event->database) << "\",\n"
         << R"(      "query": ")"
         << (audit_record.extended_info.digest.empty()
                 ? make_escaped_string(&audit_record.event->query)
                 : make_escaped_string(audit_record.extended_info.digest))
         << "\",\n"
         << R"(      "sql_command": ")"
         << sql_command_id_to_string(audit_record.event->sql_command_id)
         << "\"}" << extra_attrs_to_string(audit_record.extended_info)
         << "\n  }";

  SysVars::update_log_bookmark(rec_id, timestamp);

  return result.str();
}

AuditRecordString LogRecordFormatterJson::apply(
    const AuditRecordAuthentication &audit_record) const noexcept {
  std::stringstream result;
  const auto timestamp = make_timestamp(std::chrono::system_clock::now());
  const auto rec_id = make_record_id();

  result << "  {\n"
         << R"(    "timestamp": ")" << timestamp << "\",\n"
         << R"(    "id": )" << rec_id << ",\n"
         << R"(    "class": "authentication",)"
         << "\n"
         << R"(    "event": ")"
         << event_subclass_to_string(audit_record.event->event_subclass)
         << "\",\n"
         << R"(    "connection_id": )" << audit_record.event->connection_id
         << ",\n"
         << R"(    "account": { "user": ")"
         << make_escaped_string(&audit_record.event->user) << R"(", "host": ")"
         << make_escaped_string(&audit_record.event->host) << R"(" },)"
         << "\n"
         << R"(    "authentication_data": {)"
         << "\n"
         << R"(      "auth_plugin": ")"
         << make_escaped_string(&audit_record.event->authentication_plugin)
         << "\",\n"
         << R"(      "new_account": { "user": ")"
         << make_escaped_string(&audit_record.event->new_user)
         << R"(", "host": ")"
         << make_escaped_string(&audit_record.event->new_host) << R"(" },)"
         << "\n"
         << R"(      "status": )" << audit_record.event->status << ",\n"
         << R"(      "query": ")"
         << (audit_record.extended_info.digest.empty()
                 ? make_escaped_string(&audit_record.event->query)
                 : make_escaped_string(audit_record.extended_info.digest))
         << "\",\n"
         << R"(      "sql_command": ")"
         << sql_command_id_to_string(audit_record.event->sql_command_id)
         << "\"}" << extra_attrs_to_string(audit_record.extended_info)
         << "\n  }";

  SysVars::update_log_bookmark(rec_id, timestamp);

  return result.str();
}

AuditRecordString LogRecordFormatterJson::apply(
    const AuditRecordMessage &audit_record) const noexcept {
  std::stringstream result;
  const auto timestamp = make_timestamp(std::chrono::system_clock::now());
  const auto rec_id = make_record_id();

  result << "  {\n"
         << R"(    "timestamp": ")" << timestamp << "\",\n"
         << R"(    "id": )" << rec_id << ",\n"
         << R"(    "class": "message",)"
         << "\n"
         << R"(    "event": ")"
         << event_subclass_to_string(audit_record.event->event_subclass)
         << "\",\n"
         << R"(    "message_data": {)"
         << "\n"
         << R"(      "component": ")"
         << make_escaped_string(&audit_record.event->component) << "\",\n"
         << R"(      "producer": ")"
         << make_escaped_string(&audit_record.event->producer) << "\",\n"
         << R"(      "message": ")"
         << make_escaped_string(&audit_record.event->message) << "\",\n"
         << R"(      "message_attributes": {)"
         << "\n";

  for (size_t i = 0; i < audit_record.event->key_value_map_length; ++i) {
    result << ((i == 0) ? "" : ",\n") << "        \""
           << make_escaped_string(&audit_record.event->key_value_map[i].key)
           << "\": ";
    if (audit_record.event->key_value_map[i].value_type ==
        MYSQL_AUDIT_MESSAGE_VALUE_TYPE_STR) {
      result << "\""
             << make_escaped_string(
                    &audit_record.event->key_value_map[i].value.str)
             << "\"";
    } else if (audit_record.event->key_value_map[i].value_type ==
               MYSQL_AUDIT_MESSAGE_VALUE_TYPE_NUM) {
      result << audit_record.event->key_value_map[i].value.num;
    } else {
      result << "\"\"";
    }
  }

  result << "\n      }\n"
         << "    }" << extra_attrs_to_string(audit_record.extended_info)
         << "\n  }";

  SysVars::update_log_bookmark(rec_id, timestamp);

  return result.str();
}

AuditRecordString LogRecordFormatterJson::apply(
    const AuditRecordStartAudit &audit_record) const noexcept {
  std::stringstream result;
  const auto timestamp = make_timestamp(std::chrono::system_clock::now());
  const auto rec_id = make_record_id();

  result << "  {\n"
         << R"(    "timestamp": ")" << timestamp << "\",\n"
         << R"(    "id": )" << rec_id << ",\n"
         << R"(    "class": ")"
         << event_subclass_to_string(audit_record.event->event_subclass)
         << "\",\n"
         << R"(    "server_id": )" << audit_record.event->server_id
         << extra_attrs_to_string(audit_record.extended_info) << "\n  }";

  SysVars::update_log_bookmark(rec_id, timestamp);

  return result.str();
}

AuditRecordString LogRecordFormatterJson::apply(
    const AuditRecordStopAudit &audit_record) const noexcept {
  std::stringstream result;
  const auto timestamp = make_timestamp(std::chrono::system_clock::now());
  const auto rec_id = make_record_id();

  result << "  {\n"
         << R"(    "timestamp": ")" << timestamp << "\",\n"
         << R"(    "id": )" << rec_id << ",\n"
         << R"(    "class": ")"
         << event_subclass_to_string(audit_record.event->event_subclass)
         << "\",\n"
         << R"(    "server_id": )" << audit_record.event->server_id
         << extra_attrs_to_string(audit_record.extended_info) << "\n  }";

  SysVars::update_log_bookmark(rec_id, timestamp);

  return result.str();
}

std::string LogRecordFormatterJson::get_file_header() const noexcept {
  return "[\n";
}

std::string LogRecordFormatterJson::get_file_footer() const noexcept {
  return "\n]\n";
}

std::string LogRecordFormatterJson::get_record_separator() const noexcept {
  return ",\n";
}

const EscapeRulesContainer &LogRecordFormatterJson::get_escape_rules()
    const noexcept {
  static const EscapeRulesContainer escape_rules = {
      {0, "\\u0000"},  {1, "\\u0001"},  {2, "\\u0002"},  {3, "\\u0003"},
      {4, "\\u0004"},  {5, "\\u0005"},  {6, "\\u0006"},  {7, "\\u0007"},
      {'\b', "\\b"},   {'\t', "\\t"},   {'\n', "\\n"},   {11, "\\u000B"},
      {'\f', "\\f"},   {'\r', "\\r"},   {14, "\\u000E"}, {15, "\\u000F"},
      {16, "\\u0010"}, {17, "\\u0011"}, {18, "\\u0012"}, {19, "\\u0013"},
      {20, "\\u0014"}, {21, "\\u0015"}, {22, "\\u0016"}, {23, "\\u0017"},
      {24, "\\u0018"}, {25, "\\u0019"}, {26, "\\u001A"}, {27, "\\u001B"},
      {28, "\\u001C"}, {29, "\\u001D"}, {30, "\\u001E"}, {31, "\\u001F"},
      {'\\', "\\\\"},  {'"', "\\\""},   {'/', "\\/"}};

  return escape_rules;
}

std::string LogRecordFormatterJson::make_timestamp(
    const std::chrono::system_clock::time_point time_point) const noexcept {
  std::time_t t = std::chrono::system_clock::to_time_t(time_point);

  DBUG_EXECUTE_IF("audit_log_filter_debug_timestamp", {
    t = std::chrono::system_clock::to_time_t(
        SysVars::get_debug_time_point_for_rotation());
  });

  std::stringstream timestamp;
  timestamp << std::put_time(std::localtime(&t), "%F %T");

  return timestamp.str();
}

void LogRecordFormatterJson::apply_debug_info(
    std::string_view event_class_name, std::string_view event_subclass_name,
    std::string &record_str) noexcept {
  assert(!record_str.empty());

  std::stringstream debug_info;
  debug_info << R"(  "event_class_name": ")" << event_class_name << "\",\n"
             << R"(  "event_subclass_name": ")" << event_subclass_name
             << "\",\n";

  std::string insert_after_tag{"{\n"};
  auto tag_begin = record_str.find(insert_after_tag, 0);
  record_str.insert(tag_begin + insert_after_tag.length(), debug_info.str());
}

std::string LogRecordFormatterJson::extra_attrs_to_string(
    const ExtendedInfo &info) const noexcept {
  std::stringstream result;

  for (const auto &pair : info.attrs) {
    result << ",\n"
           << "    \"" << pair.first << "\": {\n";

    bool is_first_attr = true;
    for (const auto &name_value : pair.second) {
      result << (is_first_attr ? "" : ",\n") << "      \""
             << make_escaped_string(name_value.first) << "\": \""
             << make_escaped_string(name_value.second) << "\"";
      is_first_attr = false;
    }

    result << "}";
  }

  return result.str();
}

}  // namespace audit_log_filter::log_record_formatter
