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

#include "plugin/audit_log_filter/log_record_formatter/base.h"

#include "plugin/audit_log_filter/sys_vars.h"

#include <iomanip>
#include <iostream>

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
    const MYSQL_LEX_CSTRING *in) const noexcept {
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
    mysql_event_class_t event_class) noexcept {
  switch (event_class) {
    case MYSQL_AUDIT_GENERAL_CLASS:
      return kAuditEventNameGeneral;
    case MYSQL_AUDIT_CONNECTION_CLASS:
      return kAuditEventNameConnection;
    case MYSQL_AUDIT_AUTHORIZATION_CLASS:
      return kAuditEventNameAuthorization;
    case MYSQL_AUDIT_TABLE_ACCESS_CLASS:
      return kAuditEventNameTableAccess;
    case MYSQL_AUDIT_GLOBAL_VARIABLE_CLASS:
      return kAuditEventNameGlobalVariable;
    case MYSQL_AUDIT_SERVER_STARTUP_CLASS:
      return kAuditEventNameServerStartup;
    case MYSQL_AUDIT_SERVER_SHUTDOWN_CLASS:
      return kAuditEventNameServerShutdown;
    case MYSQL_AUDIT_COMMAND_CLASS:
      return kAuditEventNameCommand;
    case MYSQL_AUDIT_QUERY_CLASS:
      return kAuditEventNameQuery;
    case MYSQL_AUDIT_STORED_PROGRAM_CLASS:
      return kAuditEventNameStoredProgram;
    case MYSQL_AUDIT_AUTHENTICATION_CLASS:
      return kAuditEventNameAuthentication;
    case MYSQL_AUDIT_MESSAGE_CLASS:
      return kAuditEventNameMessage;
    default:
      assert(false);
  }

  return kAuditNameUnknown;
}

std::string_view LogRecordFormatterBase::event_subclass_to_string(
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

std::string_view LogRecordFormatterBase::event_subclass_to_string(
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

std::string_view LogRecordFormatterBase::event_subclass_to_string(
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

std::string_view LogRecordFormatterBase::event_subclass_to_string(
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

std::string_view LogRecordFormatterBase::event_subclass_to_string(
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

std::string_view LogRecordFormatterBase::event_subclass_to_string(
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

std::string_view LogRecordFormatterBase::event_subclass_to_string(
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

std::string_view LogRecordFormatterBase::event_subclass_to_string(
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

std::string_view LogRecordFormatterBase::event_subclass_to_string(
    mysql_event_server_startup_subclass_t event_subclass) const noexcept {
  switch (event_subclass) {
    case MYSQL_AUDIT_SERVER_STARTUP_STARTUP:
      return kAuditEventNameServerStartupStartup;
    default:
      assert(false);
  }

  return kAuditNameUnknown;
}

std::string_view LogRecordFormatterBase::event_subclass_to_string(
    mysql_event_server_shutdown_subclass_t event_subclass) const noexcept {
  switch (event_subclass) {
    case MYSQL_AUDIT_SERVER_SHUTDOWN_SHUTDOWN:
      return kAuditEventNameServerShutdownShutdown;
    default:
      assert(false);
  }

  return kAuditNameUnknown;
}

std::string_view LogRecordFormatterBase::event_subclass_to_string(
    mysql_event_stored_program_subclass_t event_subclass) const noexcept {
  switch (event_subclass) {
    case MYSQL_AUDIT_STORED_PROGRAM_EXECUTE:
      return kAuditEventNameStoredProgramExecute;
    default:
      assert(false);
  }

  return kAuditNameUnknown;
}

std::string_view LogRecordFormatterBase::event_subclass_to_string(
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

std::string_view LogRecordFormatterBase::sql_command_id_to_string(
    enum_sql_command_t sql_command_id) noexcept {
  static const std::unordered_map<enum_sql_command_t, std::string_view>
      name_map{
          {SQLCOM_SELECT, "select"},
          {SQLCOM_CREATE_TABLE, "create_table"},
          {SQLCOM_CREATE_INDEX, "create_index"},
          {SQLCOM_ALTER_TABLE, "alter_table"},
          {SQLCOM_UPDATE, "update"},
          {SQLCOM_INSERT, "insert"},
          {SQLCOM_INSERT_SELECT, "insert_select"},
          {SQLCOM_DELETE, "delete"},
          {SQLCOM_TRUNCATE, "truncate"},
          {SQLCOM_DROP_TABLE, "drop_table"},
          {SQLCOM_DROP_INDEX, "drop_index"},
          {SQLCOM_SHOW_DATABASES, "show_databases"},
          {SQLCOM_SHOW_TABLES, "show_tables"},
          {SQLCOM_SHOW_FIELDS, "show_fields"},
          {SQLCOM_SHOW_KEYS, "show_keys"},
          {SQLCOM_SHOW_VARIABLES, "show_variables"},
          {SQLCOM_SHOW_STATUS, "show_status"},
          {SQLCOM_SHOW_ENGINE_LOGS, "show_engine_logs"},
          {SQLCOM_SHOW_ENGINE_STATUS, "show_engine_status"},
          {SQLCOM_SHOW_ENGINE_MUTEX, "show_engine_mutex"},
          {SQLCOM_SHOW_PROCESSLIST, "show_processlist"},
          {SQLCOM_SHOW_MASTER_STAT, "show_master_stat"},
          {SQLCOM_SHOW_SLAVE_STAT, "show_slave_stat"},
          {SQLCOM_SHOW_GRANTS, "show_grants"},
          {SQLCOM_SHOW_CREATE, "show_create"},
          {SQLCOM_SHOW_CHARSETS, "show_charsets"},
          {SQLCOM_SHOW_COLLATIONS, "show_collations"},
          {SQLCOM_SHOW_CREATE_DB, "show_create_db"},
          {SQLCOM_SHOW_TABLE_STATUS, "show_table_status"},
          {SQLCOM_SHOW_TRIGGERS, "show_triggers"},
          {SQLCOM_LOAD, "load"},
          {SQLCOM_SET_OPTION, "set_option"},
          {SQLCOM_LOCK_TABLES, "lock_tables"},
          {SQLCOM_UNLOCK_TABLES, "unlock_tables"},
          {SQLCOM_GRANT, "grant"},
          {SQLCOM_CHANGE_DB, "change_db"},
          {SQLCOM_CREATE_DB, "create_db"},
          {SQLCOM_DROP_DB, "drop_db"},
          {SQLCOM_ALTER_DB, "alter_db"},
          {SQLCOM_REPAIR, "repair"},
          {SQLCOM_REPLACE, "replace"},
          {SQLCOM_REPLACE_SELECT, "replace_select"},
          {SQLCOM_CREATE_FUNCTION, "create_function"},
          {SQLCOM_DROP_FUNCTION, "drop_function"},
          {SQLCOM_REVOKE, "revoke"},
          {SQLCOM_OPTIMIZE, "optimize"},
          {SQLCOM_CHECK, "check"},
          {SQLCOM_ASSIGN_TO_KEYCACHE, "assign_to_keycache"},
          {SQLCOM_PRELOAD_KEYS, "preload_keys"},
          {SQLCOM_FLUSH, "flush"},
          {SQLCOM_KILL, "kill"},
          {SQLCOM_ANALYZE, "analyze"},
          {SQLCOM_ROLLBACK, "rollback"},
          {SQLCOM_ROLLBACK_TO_SAVEPOINT, "rollback_to_savepoint"},
          {SQLCOM_COMMIT, "commit"},
          {SQLCOM_SAVEPOINT, "savepoint"},
          {SQLCOM_RELEASE_SAVEPOINT, "release_savepoint"},
          {SQLCOM_SLAVE_START, "slave_start"},
          {SQLCOM_SLAVE_STOP, "slave_stop"},
          {SQLCOM_START_GROUP_REPLICATION, "start_group_replication"},
          {SQLCOM_STOP_GROUP_REPLICATION, "stop_group_replication"},
          {SQLCOM_BEGIN, "begin"},
          {SQLCOM_CHANGE_MASTER, "change_master"},
          {SQLCOM_CHANGE_REPLICATION_FILTER, "change_replication_filter"},
          {SQLCOM_RENAME_TABLE, "rename_table"},
          {SQLCOM_RESET, "reset"},
          {SQLCOM_PURGE, "purge"},
          {SQLCOM_PURGE_BEFORE, "purge_before"},
          {SQLCOM_SHOW_BINLOGS, "show_binlogs"},
          {SQLCOM_SHOW_OPEN_TABLES, "show_open_tables"},
          {SQLCOM_HA_OPEN, "ha_open"},
          {SQLCOM_HA_CLOSE, "ha_close"},
          {SQLCOM_HA_READ, "ha_read"},
          {SQLCOM_SHOW_SLAVE_HOSTS, "show_slave_hosts"},
          {SQLCOM_DELETE_MULTI, "delete_multi"},
          {SQLCOM_UPDATE_MULTI, "update_multi"},
          {SQLCOM_SHOW_BINLOG_EVENTS, "show_binlog_events"},
          {SQLCOM_DO, "do"},
          {SQLCOM_SHOW_WARNS, "show_warns"},
          {SQLCOM_EMPTY_QUERY, "empty_query"},
          {SQLCOM_SHOW_ERRORS, "show_errors"},
          {SQLCOM_SHOW_STORAGE_ENGINES, "show_storage_engines"},
          {SQLCOM_SHOW_PRIVILEGES, "show_privileges"},
          {SQLCOM_HELP, "help"},
          {SQLCOM_CREATE_USER, "create_user"},
          {SQLCOM_DROP_USER, "drop_user"},
          {SQLCOM_RENAME_USER, "rename_user"},
          {SQLCOM_REVOKE_ALL, "revoke_all"},
          {SQLCOM_CHECKSUM, "checksum"},
          {SQLCOM_CREATE_PROCEDURE, "create_procedure"},
          {SQLCOM_CREATE_SPFUNCTION, "create_spfunction"},
          {SQLCOM_CALL, "call"},
          {SQLCOM_DROP_PROCEDURE, "drop_procedure"},
          {SQLCOM_ALTER_PROCEDURE, "alter_procedure"},
          {SQLCOM_ALTER_FUNCTION, "alter_function"},
          {SQLCOM_SHOW_CREATE_PROC, "show_create_proc"},
          {SQLCOM_SHOW_CREATE_FUNC, "show_create_func"},
          {SQLCOM_SHOW_STATUS_PROC, "show_status_proc"},
          {SQLCOM_SHOW_STATUS_FUNC, "show_status_func"},
          {SQLCOM_PREPARE, "prepare"},
          {SQLCOM_EXECUTE, "execute"},
          {SQLCOM_DEALLOCATE_PREPARE, "deallocate_prepare"},
          {SQLCOM_CREATE_VIEW, "create_view"},
          {SQLCOM_DROP_VIEW, "drop_view"},
          {SQLCOM_CREATE_TRIGGER, "create_trigger"},
          {SQLCOM_DROP_TRIGGER, "drop_trigger"},
          {SQLCOM_XA_START, "xa_start"},
          {SQLCOM_XA_END, "xa_end"},
          {SQLCOM_XA_PREPARE, "xa_prepare"},
          {SQLCOM_XA_COMMIT, "xa_commit"},
          {SQLCOM_XA_ROLLBACK, "xa_rollback"},
          {SQLCOM_XA_RECOVER, "xa_recover"},
          {SQLCOM_SHOW_PROC_CODE, "show_proc_code"},
          {SQLCOM_SHOW_FUNC_CODE, "show_func_code"},
          {SQLCOM_ALTER_TABLESPACE, "alter_tablespace"},
          {SQLCOM_INSTALL_PLUGIN, "install_plugin"},
          {SQLCOM_UNINSTALL_PLUGIN, "uninstall_plugin"},
          {SQLCOM_BINLOG_BASE64_EVENT, "binlog_base64_event"},
          {SQLCOM_SHOW_PLUGINS, "show_plugins"},
          {SQLCOM_CREATE_SERVER, "create_server"},
          {SQLCOM_DROP_SERVER, "drop_server"},
          {SQLCOM_ALTER_SERVER, "alter_server"},
          {SQLCOM_CREATE_EVENT, "create_event"},
          {SQLCOM_ALTER_EVENT, "alter_event"},
          {SQLCOM_DROP_EVENT, "drop_event"},
          {SQLCOM_SHOW_CREATE_EVENT, "show_create_event"},
          {SQLCOM_SHOW_EVENTS, "show_events"},
          {SQLCOM_SHOW_CREATE_TRIGGER, "show_create_trigger"},
          {SQLCOM_SHOW_PROFILE, "show_profile"},
          {SQLCOM_SHOW_PROFILES, "show_profiles"},
          {SQLCOM_SIGNAL, "signal"},
          {SQLCOM_RESIGNAL, "resignal"},
          {SQLCOM_SHOW_RELAYLOG_EVENTS, "show_relaylog_events"},
          {SQLCOM_GET_DIAGNOSTICS, "get_diagnostics"},
          {SQLCOM_ALTER_USER, "alter_user"},
          {SQLCOM_EXPLAIN_OTHER, "explain_other"},
          {SQLCOM_SHOW_CREATE_USER, "show_create_user"},
          {SQLCOM_SHUTDOWN, "shutdown"},
          {SQLCOM_SET_PASSWORD, "set_password"},
          {SQLCOM_ALTER_INSTANCE, "alter_instance"},
          {SQLCOM_INSTALL_COMPONENT, "install_component"},
          {SQLCOM_UNINSTALL_COMPONENT, "uninstall_component"},
          {SQLCOM_CREATE_ROLE, "create_role"},
          {SQLCOM_DROP_ROLE, "drop_role"},
          {SQLCOM_SET_ROLE, "set_role"},
          {SQLCOM_GRANT_ROLE, "grant_role"},
          {SQLCOM_REVOKE_ROLE, "revoke_role"},
          {SQLCOM_ALTER_USER_DEFAULT_ROLE, "alter_user_default_role"},
          {SQLCOM_IMPORT, "import"},
          {SQLCOM_CREATE_RESOURCE_GROUP, "create_resource_group"},
          {SQLCOM_ALTER_RESOURCE_GROUP, "alter_resource_group"},
          {SQLCOM_DROP_RESOURCE_GROUP, "drop_resource_group"},
          {SQLCOM_SET_RESOURCE_GROUP, "set_resource_group"},
          {SQLCOM_CLONE, "clone"},
          {SQLCOM_LOCK_INSTANCE, "lock_instance"},
          {SQLCOM_UNLOCK_INSTANCE, "unlock_instance"},
          {SQLCOM_RESTART_SERVER, "restart_server"},
          {SQLCOM_CREATE_SRS, "create_srs"},
          {SQLCOM_DROP_SRS, "drop_srs"},
          {SQLCOM_SHOW_USER_STATS, "show_user_stats"},
          {SQLCOM_SHOW_TABLE_STATS, "show_table_stats"},
          {SQLCOM_SHOW_INDEX_STATS, "show_index_stats"},
          {SQLCOM_SHOW_CLIENT_STATS, "show_client_stats"},
          {SQLCOM_SHOW_THREAD_STATS, "show_thread_stats"},
          {SQLCOM_LOCK_TABLES_FOR_BACKUP, "lock_tables_for_backup"},
          {SQLCOM_CREATE_COMPRESSION_DICTIONARY,
           "create_compression_dictionary"},
          {SQLCOM_DROP_COMPRESSION_DICTIONARY, "drop_compression_dictionary"},
      };

  const auto it = name_map.find(sql_command_id);

  if (it != name_map.end()) {
    return it->second;
  }

  return {""};
}

std::string_view LogRecordFormatterBase::command_id_to_string(
    enum_server_command_t command_id) noexcept {
  static const std::unordered_map<enum_server_command_t, std::string_view>
      name_map{{COM_SLEEP, "slep"},
               {COM_QUIT, "quit"},
               {COM_INIT_DB, "init_db"},
               {COM_QUERY, "query"},
               {COM_FIELD_LIST, "field_list"},
               {COM_CREATE_DB, "create_db"},
               {COM_DROP_DB, "drop_db"},
               {COM_REFRESH, "refresh"},
               {COM_DEPRECATED_1, "deprecated"},
               {COM_STATISTICS, "statistics"},
               {COM_PROCESS_INFO, "process_info"},
               {COM_CONNECT, "connect"},
               {COM_PROCESS_KILL, "process_kill"},
               {COM_DEBUG, "debug"},
               {COM_PING, "ping"},
               {COM_TIME, "time"},
               {COM_DELAYED_INSERT, "delayed_insert"},
               {COM_CHANGE_USER, "change_user"},
               {COM_BINLOG_DUMP, "binlog_dump"},
               {COM_TABLE_DUMP, "table_dump"},
               {COM_CONNECT_OUT, "connect_out"},
               {COM_REGISTER_SLAVE, "register_slave"},
               {COM_STMT_PREPARE, "stmt_prepare"},
               {COM_STMT_EXECUTE, "stmt_execute"},
               {COM_STMT_SEND_LONG_DATA, "stmt_send_long_data"},
               {COM_STMT_CLOSE, "stmt_close"},
               {COM_STMT_RESET, "stmt_reset"},
               {COM_SET_OPTION, "set_option"},
               {COM_STMT_FETCH, "stmt_fetch"},
               {COM_DAEMON, "daemon"},
               {COM_BINLOG_DUMP_GTID, "binlog_dump_gtid"},
               {COM_RESET_CONNECTION, "reset_connection"},
               {COM_CLONE, "clone"},
               {COM_SUBSCRIBE_GROUP_REPLICATION_STREAM,
                "subscribe_group_replication_stream"}};

  const auto it = name_map.find(command_id);

  if (it != name_map.end()) {
    return it->second;
  }

  return {""};
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
