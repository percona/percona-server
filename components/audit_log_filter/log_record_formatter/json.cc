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

#include "components/audit_log_filter/log_record_formatter/json.h"
#include "components/audit_log_filter/sys_vars.h"

#include "my_dbug.h"

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

#include <cassert>
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

const std::string_view kAuditEventNameParseRewriteNone{"no_rewrite"};
const std::string_view kAuditEventNameParseRewriteQueryRewritten{
    "query_rewritten"};
const std::string_view kAuditEventNameParseRewritePreparedStatement{
    "prepared_statement"};

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

auto make_unix_timestamp(
    const std::chrono::system_clock::time_point time_point) noexcept {
  return std::chrono::duration_cast<std::chrono::microseconds>(
             time_point.time_since_epoch())
      .count();
}

}  // namespace

std::string_view LogRecordFormatterJson::event_subclass_to_string(
    const mysql_event_tracking_general_data *event) const noexcept {
  switch (event->event_subclass) {
    case EVENT_TRACKING_GENERAL_LOG:
      return kAuditEventNameGeneralLog;
    case EVENT_TRACKING_GENERAL_ERROR:
      return kAuditEventNameGeneralError;
    case EVENT_TRACKING_GENERAL_RESULT:
      return kAuditEventNameGeneralResult;
    case EVENT_TRACKING_GENERAL_STATUS:
      return kAuditEventNameGeneralStatus;
    default:
      assert(false);
  }

  return kAuditNameUnknown;
}

std::string_view LogRecordFormatterJson::event_subclass_to_string(
    const mysql_event_tracking_table_access_data *event) const noexcept {
  switch (event->event_subclass) {
    case EVENT_TRACKING_TABLE_ACCESS_READ:
      return kAuditEventNameAccessRead;
    case EVENT_TRACKING_TABLE_ACCESS_INSERT:
      return kAuditEventNameAccessInsert;
    case EVENT_TRACKING_TABLE_ACCESS_UPDATE:
      return kAuditEventNameAccessUpdate;
    case EVENT_TRACKING_TABLE_ACCESS_DELETE:
      return kAuditEventNameAccessDelete;
    default:
      assert(false);
  }

  return kAuditNameUnknown;
}

std::string_view LogRecordFormatterJson::event_subclass_to_string(
    const mysql_event_tracking_connection_data *event) const noexcept {
  switch (event->event_subclass) {
    case EVENT_TRACKING_CONNECTION_CONNECT:
      return kAuditEventNameConnect;
    case EVENT_TRACKING_CONNECTION_DISCONNECT:
      return kAuditEventNameDisconnect;
    case EVENT_TRACKING_CONNECTION_CHANGE_USER:
      return kAuditEventNameChangeUser;
    case EVENT_TRACKING_CONNECTION_PRE_AUTHENTICATE:
      return kAuditEventNamePreAuth;
    default:
      assert(false);
  }

  return kAuditNameUnknown;
}

std::string_view LogRecordFormatterJson::event_subclass_to_string(
    const mysql_event_tracking_global_variable_data *event) const noexcept {
  switch (event->event_subclass) {
    case EVENT_TRACKING_GLOBAL_VARIABLE_GET:
      return kAuditEventNameGlobalVariableGet;
    case EVENT_TRACKING_GLOBAL_VARIABLE_SET:
      return kAuditEventNameGlobalVariableSet;
    default:
      assert(false);
  }

  return kAuditNameUnknown;
}

std::string_view LogRecordFormatterJson::event_subclass_to_string(
    const mysql_event_tracking_command_data *event) const noexcept {
  switch (event->event_subclass) {
    case EVENT_TRACKING_COMMAND_START:
      return kAuditEventNameCommandStart;
    case EVENT_TRACKING_COMMAND_END:
      return kAuditEventNameCommandEnd;
    default:
      assert(false);
  }

  return kAuditNameUnknown;
}

std::string_view LogRecordFormatterJson::event_subclass_to_string(
    const mysql_event_tracking_query_data *event) const noexcept {
  switch (event->event_subclass) {
    case EVENT_TRACKING_QUERY_START:
      return kAuditEventNameQueryStart;
    case EVENT_TRACKING_QUERY_NESTED_START:
      return kAuditEventNameQueryNestedStart;
    case EVENT_TRACKING_QUERY_STATUS_END:
      return kAuditEventNameQueryStatusEnd;
    case EVENT_TRACKING_QUERY_NESTED_STATUS_END:
      return kAuditEventNameQueryNestedStatusEnd;
    default:
      assert(false);
  }

  return kAuditNameUnknown;
}

std::string_view LogRecordFormatterJson::event_subclass_to_string(
    const mysql_event_tracking_authentication_data *event) const noexcept {
  switch (event->event_subclass) {
    case EVENT_TRACKING_AUTHENTICATION_FLUSH:
      return kAuditEventNameAuthFlush;
    case EVENT_TRACKING_AUTHENTICATION_AUTHID_CREATE:
      return kAuditEventNameAuthAuthidCreate;
    case EVENT_TRACKING_AUTHENTICATION_CREDENTIAL_CHANGE:
      return kAuditEventNameAuthCredentialChange;
    case EVENT_TRACKING_AUTHENTICATION_AUTHID_RENAME:
      return kAuditEventNameAuthAuthidRename;
    case EVENT_TRACKING_AUTHENTICATION_AUTHID_DROP:
      return kAuditEventNameAuthAuthidDrop;
    default:
      assert(false);
  }

  return kAuditNameUnknown;
}

std::string_view LogRecordFormatterJson::event_subclass_to_string(
    const mysql_event_tracking_startup_data *event) const noexcept {
  switch (event->event_subclass) {
    case EVENT_TRACKING_STARTUP_STARTUP:
      return kAuditEventNameServerStartupStartup;
    default:
      assert(false);
  }

  return kAuditNameUnknown;
}

std::string_view LogRecordFormatterJson::event_subclass_to_string(
    const mysql_event_tracking_shutdown_data *event) const noexcept {
  switch (event->event_subclass) {
    case EVENT_TRACKING_SHUTDOWN_SHUTDOWN:
      return kAuditEventNameServerShutdownShutdown;
    default:
      assert(false);
  }

  return kAuditNameUnknown;
}

std::string_view LogRecordFormatterJson::event_subclass_to_string(
    const mysql_event_tracking_stored_program_data *event) const noexcept {
  switch (event->event_subclass) {
    case EVENT_TRACKING_STORED_PROGRAM_EXECUTE:
      return kAuditEventNameStoredProgramExecute;
    default:
      assert(false);
  }

  return kAuditNameUnknown;
}

std::string_view LogRecordFormatterJson::event_subclass_to_string(
    const mysql_event_tracking_message_data *event) const noexcept {
  switch (event->event_subclass) {
    case EVENT_TRACKING_MESSAGE_INTERNAL:
      return kAuditEventNameMessageInternal;
    case EVENT_TRACKING_MESSAGE_USER:
      return kAuditEventNameMessageUser;
    default:
      assert(false);
  }

  return kAuditNameUnknown;
}

std::string_view LogRecordFormatterJson::event_subclass_to_string(
    const mysql_event_tracking_parse_data *event) const noexcept {
  switch (event->event_subclass) {
    case EVENT_TRACKING_PARSE_REWRITE_NONE:
      return kAuditEventNameParseRewriteNone;
    case EVENT_TRACKING_PARSE_REWRITE_QUERY_REWRITTEN:
      return kAuditEventNameParseRewriteQueryRewritten;
    case EVENT_TRACKING_PARSE_REWRITE_IS_PREPARED_STATEMENT:
      return kAuditEventNameParseRewritePreparedStatement;
    default:
      assert(false);
  }

  return kAuditNameUnknown;
}

std::string_view LogRecordFormatterJson::event_subclass_to_string(
    const internal_event_tracking_audit_data *event) const noexcept {
  switch (event->event_subclass) {
    case INTERNAL_EVENT_TRACKING_AUDIT_AUDIT:
      return kAuditEventNameAuditStart;
    case INTERNAL_EVENT_TRACKING_AUDIT_NOAUDIT:
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
    mysql_event_tracking_shutdown_reason_t reason) const noexcept {
  switch (reason) {
    case EVENT_TRACKING_SHUTDOWN_REASON_SHUTDOWN:
      return kAuditNameShutdownReasonShutdown;
    case EVENT_TRACKING_SHUTDOWN_REASON_ABORT:
      return kAuditNameShutdownReasonAbort;
    default:
      assert(false);
  }

  return kAuditNameUnknown;
}

AuditRecordString LogRecordFormatterJson::apply(
    const AuditRecordGeneral &audit_record) const noexcept {
  std::stringstream result;
  const auto time_now = std::chrono::system_clock::now();
  const auto timestamp = make_timestamp(time_now);
  const auto rec_id = make_record_id();

  /* clang-format off */
  result << "  {\n"
         << R"(    "timestamp": ")" << timestamp << "\",\n";

  if (SysVars::get_format_unix_timestamp()) {
    result << R"(    "time": )" << make_unix_timestamp(time_now) << ",\n";
  }

  const auto escaped_user = make_escaped_string(&audit_record.event->user);
  const auto escaped_host = make_escaped_string(&audit_record.event->host);
  const auto escaped_ip = make_escaped_string(&audit_record.event->ip);

  result << R"(    "id": )" << rec_id << ",\n"
         << R"(    "class": "general",)" << "\n"
         << R"(    "event": ")" << event_subclass_to_string(audit_record.event) << "\",\n"
         << R"(    "connection_id": )" << audit_record.event->connection_id << ",\n"
         << R"(    "account": { "user": ")" << escaped_user << R"(", "host": ")" << escaped_host << R"(" },)" << "\n"
         << R"(    "login": { "user": ")" << escaped_user << R"(", "ip": ")" << escaped_ip << R"(", "proxy": "")" << " },\n"
         << R"(    "general_data": { "status": )" << audit_record.event->error_code << " }"
         << extra_attrs_to_string(audit_record.extended_info) << "\n  }";
  /* clang-format on */

  SysVars::update_log_bookmark(rec_id, timestamp);

  return result.str();
}

AuditRecordString LogRecordFormatterJson::apply(
    const AuditRecordConnection &audit_record) const noexcept {
  std::stringstream result;
  const auto time_now = std::chrono::system_clock::now();
  const auto timestamp = make_timestamp(time_now);
  const auto rec_id = make_record_id();

  /* clang-format off */
  result << "  {\n"
         << R"(    "timestamp": ")" << timestamp << "\",\n";

  if (SysVars::get_format_unix_timestamp()) {
    result << R"(    "time": )" << make_unix_timestamp(time_now) << ",\n";
  }

  const auto escaped_user = make_escaped_string(&audit_record.event->user);
  const auto escaped_host = make_escaped_string(&audit_record.event->host);
  const auto escaped_ip = make_escaped_string(&audit_record.event->ip);
  const auto escaped_external_user = make_escaped_string(&audit_record.event->external_user);
  const auto escaped_proxy_user = make_escaped_string(&audit_record.event->proxy_user);

  result << R"(    "id": )" << rec_id << ",\n"
         << R"(    "class": "connection",)" << "\n"
         << R"(    "event": ")" << event_subclass_to_string(audit_record.event) << "\",\n"
         << R"(    "connection_id": )" << audit_record.event->connection_id << ",\n"
         << R"(    "account": { "user": ")" << escaped_user << R"(", "host": ")" << escaped_host << R"(" },)" << "\n"
         << R"(    "login": { "user": ")" << escaped_user << R"(", "os": ")" << escaped_external_user << R"(", "ip": ")" << escaped_ip << R"(", "proxy": ")" << escaped_proxy_user << R"(" },)" << "\n"
         << R"(    "connection_data": {)" << "\n"
         << R"(      "connection_type": ")" << connection_type_name_to_string(audit_record.event->connection_type) << "\",\n"
         << R"(      "status": )" << audit_record.event->status << ",\n"
         << R"(      "db": ")" << make_escaped_string(&audit_record.event->database) << "\"}"
         << extra_attrs_to_string(audit_record.extended_info) << "\n  }";
  /* clang-format on */

  SysVars::update_log_bookmark(rec_id, timestamp);

  return result.str();
}

AuditRecordString LogRecordFormatterJson::apply(
    const AuditRecordTableAccess &audit_record) const noexcept {
  std::stringstream result;
  const auto time_now = std::chrono::system_clock::now();
  const auto timestamp = make_timestamp(time_now);
  const auto rec_id = make_record_id();

  /* clang-format off */
  result << "  {\n"
         << R"(    "timestamp": ")" << timestamp << "\",\n";

  if (SysVars::get_format_unix_timestamp()) {
    result << R"(    "time": )" << make_unix_timestamp(time_now) << ",\n";
  }

  result << R"(    "id": )" << rec_id << ",\n"
         << R"(    "class": "table_access",)" << "\n"
         << R"(    "event": ")" << event_subclass_to_string(audit_record.event) << "\",\n"
         << R"(    "connection_id": )" << audit_record.event->connection_id << ",\n"
         << R"(    "table_access_data": {)" << "\n"
         << R"(      "db": ")" << make_escaped_string(&audit_record.event->table_database) << "\",\n"
         << R"(      "table": ")" << make_escaped_string(&audit_record.event->table_name) << "\"}"
         << extra_attrs_to_string(audit_record.extended_info) << "\n  }";
  /* clang-format on */

  SysVars::update_log_bookmark(rec_id, timestamp);

  return result.str();
}

AuditRecordString LogRecordFormatterJson::apply(
    const AuditRecordGlobalVariable &audit_record) const noexcept {
  std::stringstream result;
  const auto time_now = std::chrono::system_clock::now();
  const auto timestamp = make_timestamp(time_now);
  const auto rec_id = make_record_id();

  /* clang-format off */
  result << "  {\n"
         << R"(    "timestamp": ")" << timestamp << "\",\n";

  if (SysVars::get_format_unix_timestamp()) {
    result << R"(    "time": )" << make_unix_timestamp(time_now) << ",\n";
  }

  result << R"(    "id": )" << rec_id << ",\n"
         << R"(    "class": "global_variable",)" << "\n"
         << R"(    "event": ")" << event_subclass_to_string(audit_record.event) << "\",\n"
         << R"(    "connection_id": )" << audit_record.event->connection_id << ",\n"
         << R"(    "global_variable_data": {)" << "\n"
         << R"(      "name": ")" << make_escaped_string(&audit_record.event->variable_name) << "\",\n"
         << R"(      "value": ")" << make_escaped_string(&audit_record.event->variable_value) << "\",\n"
         << R"(      "sql_command": ")" << make_escaped_string(audit_record.event->sql_command) << "\"}"
         << extra_attrs_to_string(audit_record.extended_info) << "\n  }";
  /* clang-format on */

  SysVars::update_log_bookmark(rec_id, timestamp);

  return result.str();
}

AuditRecordString LogRecordFormatterJson::apply(
    const AuditRecordServerStartup &audit_record) const noexcept {
  std::stringstream result;
  const auto time_now = std::chrono::system_clock::now();
  const auto timestamp = make_timestamp(time_now);
  const auto rec_id = make_record_id();

  /* clang-format off */
  result << "  {\n"
         << R"(    "timestamp": ")" << timestamp << "\",\n";

  if (SysVars::get_format_unix_timestamp()) {
    result << R"(    "time": )" << make_unix_timestamp(time_now) << ",\n";
  }

  result << R"(    "id": )" << rec_id << ",\n"
         << R"(    "class": "server_startup",)" << "\n"
         << R"(    "event": ")" << event_subclass_to_string(audit_record.event) << "\",\n"
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
  /* clang-format on */

  SysVars::update_log_bookmark(rec_id, timestamp);

  return result.str();
}

AuditRecordString LogRecordFormatterJson::apply(
    const AuditRecordServerShutdown &audit_record) const noexcept {
  std::stringstream result;
  const auto time_now = std::chrono::system_clock::now();
  const auto timestamp = make_timestamp(time_now);
  const auto rec_id = make_record_id();

  /* clang-format off */
  result << "  {\n"
         << R"(    "timestamp": ")" << timestamp << "\",\n";

  if (SysVars::get_format_unix_timestamp()) {
    result << R"(    "time": )" << make_unix_timestamp(time_now) << ",\n";
  }

  result << R"(    "id": )" << rec_id << ",\n"
         << R"(    "class": "server_shutdown",)" << "\n"
         << R"(    "event": ")" << event_subclass_to_string(audit_record.event) << "\",\n"
         << R"(    "server_shutdown_data": {)" << "\n"
         << R"(      "status": )" << audit_record.event->exit_code << ",\n"
         << R"(      "reason": ")" << shutdown_reason_to_string(audit_record.event->reason) << "\"}"
         << extra_attrs_to_string(audit_record.extended_info) << "\n  }";
  /* clang-format on */

  SysVars::update_log_bookmark(rec_id, timestamp);

  return result.str();
}

AuditRecordString LogRecordFormatterJson::apply(
    const AuditRecordCommand &audit_record) const noexcept {
  std::stringstream result;
  const auto time_now = std::chrono::system_clock::now();
  const auto timestamp = make_timestamp(time_now);
  const auto rec_id = make_record_id();

  /* clang-format off */
  result << "  {\n"
         << R"(    "timestamp": ")" << timestamp << "\",\n";

  if (SysVars::get_format_unix_timestamp()) {
    result << R"(    "time": )" << make_unix_timestamp(time_now) << ",\n";
  }

  result << R"(    "id": )" << rec_id << ",\n"
         << R"(    "class": "command",)" << "\n"
         << R"(    "event": ")" << event_subclass_to_string(audit_record.event) << "\",\n"
         << R"(    "connection_id": )" << audit_record.event->connection_id << ",\n"
         << R"(    "command_data": {)" << "\n"
         << R"(      "name": ")" << event_subclass_to_string(audit_record.event) << "\",\n"
         << R"(      "status": )" << audit_record.event->status << ",\n"
         << R"(      "command": ")" << make_escaped_string(&audit_record.event->command) << "\"}"
         << extra_attrs_to_string(audit_record.extended_info) << "\n  }";
  /* clang-format on */

  SysVars::update_log_bookmark(rec_id, timestamp);

  return result.str();
}

AuditRecordString LogRecordFormatterJson::apply(
    const AuditRecordQuery &audit_record) const noexcept {
  std::stringstream result;
  const auto time_now = std::chrono::system_clock::now();
  const auto timestamp = make_timestamp(time_now);
  const auto rec_id = make_record_id();

  /* clang-format off */
  result << "  {\n"
         << R"(    "timestamp": ")" << timestamp << "\",\n";

  if (SysVars::get_format_unix_timestamp()) {
    result << R"(    "time": )" << make_unix_timestamp(time_now) << ",\n";
  }

  result << R"(    "id": )" << rec_id << ",\n"
         << R"(    "class": "query",)" << "\n"
         << R"(    "event": ")" << event_subclass_to_string(audit_record.event) << "\",\n"
         << R"(    "connection_id": )" << audit_record.event->connection_id << ",\n"
         << R"(    "query_data": {)" << "\n"
         << R"(      "query": ")" << (audit_record.extended_info.digest.empty()
                                        ? make_escaped_string(&audit_record.event->query)
                                        : make_escaped_string(audit_record.extended_info.digest)) << "\",\n"
         << R"(      "status": )" << audit_record.event->status << ",\n"
         << R"(      "sql_command": ")" << make_escaped_string(audit_record.event->sql_command) << "\"}"
         << extra_attrs_to_string(audit_record.extended_info)
         << "\n  }";
  /* clang-format on */

  SysVars::update_log_bookmark(rec_id, timestamp);

  return result.str();
}

AuditRecordString LogRecordFormatterJson::apply(
    const AuditRecordStoredProgram &audit_record) const noexcept {
  std::stringstream result;
  const auto time_now = std::chrono::system_clock::now();
  const auto timestamp = make_timestamp(time_now);
  const auto rec_id = make_record_id();

  /* clang-format off */
  result << "  {\n"
         << R"(    "timestamp": ")" << timestamp << "\",\n";

  if (SysVars::get_format_unix_timestamp()) {
    result << R"(    "time": )" << make_unix_timestamp(time_now) << ",\n";
  }

  result << R"(    "id": )" << rec_id << ",\n"
         << R"(    "class": "stored_program",)" << "\n"
         << R"(    "event": ")" << event_subclass_to_string(audit_record.event) << "\",\n"
         << R"(    "connection_id": )" << audit_record.event->connection_id << ",\n"
         << R"(    "stored_program_data": {)" << "\n"
         << R"(      "name": ")" << make_escaped_string(&audit_record.event->name) << "\",\n"
         << R"(      "db": ")" << make_escaped_string(&audit_record.event->database) << "\"}"
         << extra_attrs_to_string(audit_record.extended_info)
         << "\n  }";
  /* clang-format on */

  SysVars::update_log_bookmark(rec_id, timestamp);

  return result.str();
}

AuditRecordString LogRecordFormatterJson::apply(
    const AuditRecordAuthentication &audit_record) const noexcept {
  std::stringstream result;
  const auto time_now = std::chrono::system_clock::now();
  const auto timestamp = make_timestamp(time_now);
  const auto rec_id = make_record_id();

  /* clang-format off */
  result << "  {\n"
         << R"(    "timestamp": ")" << timestamp << "\",\n";

  if (SysVars::get_format_unix_timestamp()) {
    result << R"(    "time": )" << make_unix_timestamp(time_now) << ",\n";
  }

  const auto escaped_user = make_escaped_string(&audit_record.event->user);
  const auto escaped_host = make_escaped_string(&audit_record.event->host);

  result << R"(    "id": )" << rec_id << ",\n"
         << R"(    "class": "authentication",)" << "\n"
         << R"(    "event": ")" << event_subclass_to_string(audit_record.event) << "\",\n"
         << R"(    "connection_id": )" << audit_record.event->connection_id << ",\n"
         << R"(    "account": { "user": ")" << escaped_user << R"(", "host": ")" << escaped_host << R"(" },)" << "\n"
         << R"(    "authentication_data": { "status": )" << audit_record.event->status << " }"
         << extra_attrs_to_string(audit_record.extended_info)
         << "\n  }";
  /* clang-format on */

  SysVars::update_log_bookmark(rec_id, timestamp);

  return result.str();
}

AuditRecordString LogRecordFormatterJson::apply(
    const AuditRecordMessage &audit_record) const noexcept {
  std::stringstream result;
  const auto time_now = std::chrono::system_clock::now();
  const auto timestamp = make_timestamp(time_now);
  const auto rec_id = make_record_id();

  /* clang-format off */
  result << "  {\n"
         << R"(    "timestamp": ")" << timestamp << "\",\n";

  if (SysVars::get_format_unix_timestamp()) {
    result << R"(    "time": )" << make_unix_timestamp(time_now) << ",\n";
  }

  result << R"(    "id": )" << rec_id << ",\n"
         << R"(    "class": "message",)" << "\n"
         << R"(    "event": ")" << event_subclass_to_string(audit_record.event) << "\",\n"
         << R"(    "connection_id": )" << audit_record.event->connection_id << ",\n"
         << R"(    "message_data": {)" << "\n"
         << R"(      "component": ")" << make_escaped_string(&audit_record.event->component) << "\",\n"
         << R"(      "producer": ")" << make_escaped_string(&audit_record.event->producer) << "\",\n"
         << R"(      "message": ")" << make_escaped_string(&audit_record.event->message) << "\",\n"
         << R"(      "message_attributes": {)" << "\n";

  for (size_t i = 0; i < audit_record.event->key_value_map_length; ++i) {
    result << ((i == 0) ? "" : ",\n") << "        \""
           << make_escaped_string(&audit_record.event->key_value_map[i].key)
           << "\": ";
    if (audit_record.event->key_value_map[i].value_type ==
        EVENT_TRACKING_MESSAGE_VALUE_TYPE_STR) {
      result << "\""
             << make_escaped_string(
                    &audit_record.event->key_value_map[i].value.str)
             << "\"";
    } else if (audit_record.event->key_value_map[i].value_type ==
               EVENT_TRACKING_MESSAGE_VALUE_TYPE_NUM) {
      result << audit_record.event->key_value_map[i].value.num;
    } else {
      result << "\"\"";
    }
  }

  result << "\n      }\n"
         << "    }" << extra_attrs_to_string(audit_record.extended_info)
         << "\n  }";
  /* clang-format on */

  SysVars::update_log_bookmark(rec_id, timestamp);

  return result.str();
}

AuditRecordString LogRecordFormatterJson::apply(
    const AuditRecordParse &audit_record) const noexcept {
  std::stringstream result;
  const auto time_now = std::chrono::system_clock::now();
  const auto timestamp = make_timestamp(time_now);
  const auto rec_id = make_record_id();

  /* clang-format off */
  result << "  {\n"
         << R"(    "timestamp": ")" << timestamp << "\",\n";

  if (SysVars::get_format_unix_timestamp()) {
    result << R"(    "time": )" << make_unix_timestamp(time_now) << ",\n";
  }

  result << R"(    "id": )" << rec_id << ",\n"
         << R"(    "class": "parse",)" << "\n"
         << R"(    "event": ")" << event_subclass_to_string(audit_record.event) << "\",\n"
         << R"(    "connection_id": )" << audit_record.event->connection_id << ",\n"
         << R"(    "parse_data": {)" << "\n"
         << R"(      "flags": )" << (audit_record.event->flags != nullptr ? *audit_record.event->flags : 0) << ",\n"
         << R"(      "query": ")" << (audit_record.extended_info.digest.empty()
                                          ? make_escaped_string(&audit_record.event->query)
                                          : make_escaped_string(audit_record.extended_info.digest)) << "\",\n"
         << R"(      "rewritten_query": ")" << make_escaped_string(audit_record.event->rewritten_query) << "\"}"
         << extra_attrs_to_string(audit_record.extended_info) << "\n  }";
  /* clang-format on */

  SysVars::update_log_bookmark(rec_id, timestamp);

  return result.str();
}

AuditRecordString LogRecordFormatterJson::apply(
    const AuditRecordAudit &audit_record) const noexcept {
  std::stringstream result;
  const auto time_now = std::chrono::system_clock::now();
  const auto timestamp = make_timestamp(time_now);
  const auto rec_id = make_record_id();

  /* clang-format off */
  result << "  {\n"
         << R"(    "timestamp": ")" << timestamp << "\",\n";

  if (SysVars::get_format_unix_timestamp()) {
    result << R"(    "time": )" << make_unix_timestamp(time_now) << ",\n";
  }

  result << R"(    "id": )" << rec_id << ",\n"
         << R"(    "class": "audit",)" << "\n"
         << R"(    "event": ")" << event_subclass_to_string(audit_record.event) << "\",\n"
         << R"(    "server_id": )" << audit_record.event->server_id
         << extra_attrs_to_string(audit_record.extended_info) << "\n  }";
  /* clang-format on */

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
  std::time_t tp = std::chrono::system_clock::to_time_t(time_point);

  DBUG_EXECUTE_IF("audit_log_filter_debug_timestamp", {
    tp = std::chrono::system_clock::to_time_t(
        SysVars::get_debug_time_point_for_rotation());
  });

  std::stringstream timestamp;
  timestamp << std::put_time(std::localtime(&tp), "%F %T");

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
