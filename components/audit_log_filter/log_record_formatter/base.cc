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

#include "components/audit_log_filter/log_record_formatter/base.h"

#include "components/audit_log_filter/sys_vars.h"

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
#include <iomanip>
#include <iostream>
#include <unordered_map>

namespace audit_log_filter::log_record_formatter {
namespace {

const std::string_view kAuditEventNameGeneral{"General"};
const std::string_view kAuditEventNameConnection{"Connection"};
const std::string_view kAuditEventNameAuthorization{"Authorization"};
const std::string_view kAuditEventNameTableAccess{"Table Access"};
const std::string_view kAuditEventNameGlobalVariable{"Global Variable"};
const std::string_view kAuditEventNameServerStartup{"Server Startup"};
const std::string_view kAuditEventNameServerShutdown{"Server Shutdown"};
const std::string_view kAuditEventNameCommand{"Command"};
const std::string_view kAuditEventNameQuery{"Query"};
const std::string_view kAuditEventNameStoredProgram{"Stored Program"};
const std::string_view kAuditEventNameAuthentication{"Authentication"};
const std::string_view kAuditEventNameMessage{"Message"};
const std::string_view kAuditEventNameParse{"Parse"};
const std::string_view kAuditEventNameAudit{"Audit"};

const std::string_view kAuditEventNameGeneralLog{"Log"};
const std::string_view kAuditEventNameGeneralError{"Error"};
const std::string_view kAuditEventNameGeneralResult{"Result"};
const std::string_view kAuditEventNameGeneralStatus{"Status"};

const std::string_view kAuditEventNameConnect{"Connect"};
const std::string_view kAuditEventNameDisconnect{"Disconnect"};
const std::string_view kAuditEventNameChangeUser{"Change user"};
const std::string_view kAuditEventNamePreAuth{"Pre Authenticate"};

const std::string_view kAuditConnectionTypeNameUndef{"Undefined"};
const std::string_view kAuditConnectionTypeNameTcpip{"TCP/IP"};
const std::string_view kAuditConnectionTypeNameSock{"Socket"};
const std::string_view kAuditConnectionTypeNamePipe{"Named pipe"};
const std::string_view kAuditConnectionTypeNameSsl{"SSL"};
const std::string_view kAuditConnectionTypeNameShared{"Shared memory"};

const std::string_view kAuditEventNameAccessRead{"TableRead"};
const std::string_view kAuditEventNameAccessInsert{"TableInsert"};
const std::string_view kAuditEventNameAccessUpdate{"TableUpdate"};
const std::string_view kAuditEventNameAccessDelete{"TableDelete"};

const std::string_view kAuditEventNameGlobalVariableGet{"Variable Get"};
const std::string_view kAuditEventNameGlobalVariableSet{"Variable Set"};

const std::string_view kAuditNameShutdownReasonShutdown{"Shutdown"};
const std::string_view kAuditNameShutdownReasonAbort{"Abort"};

const std::string_view kAuditEventNameCommandStart{"Command Start"};
const std::string_view kAuditEventNameCommandEnd{"Command End"};

const std::string_view kAuditEventNameQueryStart{"Query Start"};
const std::string_view kAuditEventNameQueryNestedStart{"Query Nested Start"};
const std::string_view kAuditEventNameQueryStatusEnd{"Query Status End"};
const std::string_view kAuditEventNameQueryNestedStatusEnd{
    "Query Nested Status End"};

const std::string_view kAuditEventNameAuthFlush{"Auth Flush"};
const std::string_view kAuditEventNameAuthAuthidCreate{"Auth Authid Create"};
const std::string_view kAuditEventNameAuthCredentialChange{
    "Auth Credential Change"};
const std::string_view kAuditEventNameAuthAuthidRename{"Auth Authid Rename"};
const std::string_view kAuditEventNameAuthAuthidDrop{"Auth Authid Drop"};

const std::string_view kAuditEventNameServerStartupStartup{"Startup"};

const std::string_view kAuditEventNameServerShutdownShutdown{"Shutdown"};

const std::string_view kAuditEventNameStoredProgramExecute{"Execute"};

const std::string_view kAuditEventNameMessageInternal{"Internal"};
const std::string_view kAuditEventNameMessageUser{"User"};

const std::string_view kAuditEventNameParseRewriteNone{"No Rewrite"};
const std::string_view kAuditEventNameParseRewriteQueryRewritten{
    "Query Rewritten"};
const std::string_view kAuditEventNameParseRewritePreparedStatement{
    "Prepared Statement"};

const std::string_view kAuditEventNameAuditStart{"Audit"};
const std::string_view kAuditEventNameAuditStop{"NoAudit"};

const std::string_view kAuditNameUnknown{"unknown"};

}  // namespace

std::string LogRecordFormatterBase::make_record_id(
    const std::chrono::system_clock::time_point time_point) const noexcept {
  std::stringstream id;
  id << SysVars::get_next_record_id() << "_" << make_timestamp(time_point);

  return id.str();
}

uint64_t LogRecordFormatterBase::make_record_id() const noexcept {
  return SysVars::get_next_record_id();
}

std::string LogRecordFormatterBase::make_escaped_string(
    const std::string &in) const noexcept {
  std::string out;
  const auto &escape_rules = get_escape_rules();

  for (const char &c : in) {
    const auto it = escape_rules.find(c);
    if (it == escape_rules.end()) {
      out.append(&c, 1);
    } else {
      out.append(it->second);
    }
  }

  return out;
}

std::string LogRecordFormatterBase::make_escaped_string(
    const mysql_cstring_with_length *in) const noexcept {
  std::string out;

  if (in != nullptr && in->str != nullptr && in->length != 0) {
    const auto &escape_rules = get_escape_rules();

    for (size_t i = 0; i < in->length; ++i) {
      const auto it = escape_rules.find(in->str[i]);
      if (it == escape_rules.end()) {
        out.append(&in->str[i], 1);
      } else {
        out.append(it->second);
      }
    }
  }

  return out;
}

std::string_view LogRecordFormatterBase::event_class_to_string(
    audit_event_class_t event_class) noexcept {
  switch (event_class) {
    case audit_event_class_t::AUDIT_GENERAL_CLASS:
      return kAuditEventNameGeneral;
    case audit_event_class_t::AUDIT_CONNECTION_CLASS:
      return kAuditEventNameConnection;
    case audit_event_class_t::AUDIT_AUTHORIZATION_CLASS:
      return kAuditEventNameAuthorization;
    case audit_event_class_t::AUDIT_TABLE_ACCESS_CLASS:
      return kAuditEventNameTableAccess;
    case audit_event_class_t::AUDIT_GLOBAL_VARIABLE_CLASS:
      return kAuditEventNameGlobalVariable;
    case audit_event_class_t::AUDIT_SERVER_STARTUP_CLASS:
      return kAuditEventNameServerStartup;
    case audit_event_class_t::AUDIT_SERVER_SHUTDOWN_CLASS:
      return kAuditEventNameServerShutdown;
    case audit_event_class_t::AUDIT_COMMAND_CLASS:
      return kAuditEventNameCommand;
    case audit_event_class_t::AUDIT_QUERY_CLASS:
      return kAuditEventNameQuery;
    case audit_event_class_t::AUDIT_STORED_PROGRAM_CLASS:
      return kAuditEventNameStoredProgram;
    case audit_event_class_t::AUDIT_AUTHENTICATION_CLASS:
      return kAuditEventNameAuthentication;
    case audit_event_class_t::AUDIT_MESSAGE_CLASS:
      return kAuditEventNameMessage;
    case audit_event_class_t::AUDIT_PARSE_CLASS:
      return kAuditEventNameParse;
    case audit_event_class_t::AUDIT_INTERNAL_AUDIT_CLASS:
      return kAuditEventNameAudit;
    default:
      assert(false);
  }

  return kAuditNameUnknown;
}

std::string_view LogRecordFormatterBase::event_subclass_to_string(
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

std::string_view LogRecordFormatterBase::event_subclass_to_string(
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

std::string_view LogRecordFormatterBase::event_subclass_to_string(
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

std::string_view LogRecordFormatterBase::event_subclass_to_string(
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

std::string_view LogRecordFormatterBase::event_subclass_to_string(
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

std::string_view LogRecordFormatterBase::event_subclass_to_string(
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

std::string_view LogRecordFormatterBase::event_subclass_to_string(
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

std::string_view LogRecordFormatterBase::event_subclass_to_string(
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

std::string_view LogRecordFormatterBase::event_subclass_to_string(
    const mysql_event_tracking_startup_data *event) const noexcept {
  switch (event->event_subclass) {
    case EVENT_TRACKING_STARTUP_STARTUP:
      return kAuditEventNameServerStartupStartup;
    default:
      assert(false);
  }

  return kAuditNameUnknown;
}

std::string_view LogRecordFormatterBase::event_subclass_to_string(
    const mysql_event_tracking_shutdown_data *event) const noexcept {
  switch (event->event_subclass) {
    case EVENT_TRACKING_SHUTDOWN_SHUTDOWN:
      return kAuditEventNameServerShutdownShutdown;
    default:
      assert(false);
  }

  return kAuditNameUnknown;
}

std::string_view LogRecordFormatterBase::event_subclass_to_string(
    const mysql_event_tracking_stored_program_data *event) const noexcept {
  switch (event->event_subclass) {
    case EVENT_TRACKING_STORED_PROGRAM_EXECUTE:
      return kAuditEventNameStoredProgramExecute;
    default:
      assert(false);
  }

  return kAuditNameUnknown;
}

std::string_view LogRecordFormatterBase::event_subclass_to_string(
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

std::string_view LogRecordFormatterBase::event_subclass_to_string(
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

std::string_view LogRecordFormatterBase::connection_type_name_to_string(
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

std::string_view LogRecordFormatterBase::shutdown_reason_to_string(
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

AuditRecordString LogRecordFormatterBase::apply(
    const AuditRecordUnknown &audit_record [[maybe_unused]]) const noexcept {
  // Should not happen
  assert(false);
  return "";
}

std::string LogRecordFormatterBaseXml::get_file_header() const noexcept {
  return "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<AUDIT>\n";
}

std::string LogRecordFormatterBaseXml::get_file_footer() const noexcept {
  return "</AUDIT>\n";
}

std::string LogRecordFormatterBaseXml::get_record_separator() const noexcept {
  return "";
}

const EscapeRulesContainer &LogRecordFormatterBaseXml::get_escape_rules()
    const noexcept {
  // Although most control sequences aren't supported in XML 1.0, we are better
  // off printing them anyway instead of the original control characters
  static const EscapeRulesContainer escape_rules = {
      {0, "?"},      {1, "&#1;"},     {2, "&#2;"},     {3, "&#3;"},
      {4, "&#4;"},   {5, "&#5;"},     {6, "&#6;"},     {7, "&#7;"},
      {8, "&#8;"},   {'\t', "&#9;"},  {'\n', "&#10;"}, {11, "&#11;"},
      {12, "&#12;"}, {'\r', "&#13;"}, {14, "&#14;"},   {15, "&#15;"},
      {16, "&#16;"}, {17, "&#17;"},   {18, "&#18;"},   {19, "&#19;"},
      {20, "&#20;"}, {21, "&#21;"},   {22, "&#22;"},   {23, "&#23;"},
      {24, "&#24;"}, {25, "&#25;"},   {26, "&#26;"},   {27, "&#27;"},
      {28, "&#28;"}, {29, "&#29;"},   {30, "&#30;"},   {31, "&#31;"},
      {'<', "&lt;"}, {'>', "&gt;"},   {'&', "&amp;"},  {'"', "&quot;"}};

  return escape_rules;
}

std::string LogRecordFormatterBaseXml::make_timestamp(
    const std::chrono::system_clock::time_point time_point) const noexcept {
  std::time_t t = std::chrono::system_clock::to_time_t(time_point);
  std::stringstream timestamp;
  timestamp << std::put_time(std::localtime(&t), "%FT%T");

  return timestamp.str();
}

}  // namespace audit_log_filter::log_record_formatter
