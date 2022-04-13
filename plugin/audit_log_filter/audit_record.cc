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

#include "plugin/audit_log_filter/audit_record.h"
#include "plugin/audit_log_filter/audit_error_log.h"

#include <cstring>
#include <unordered_map>

namespace audit_log_filter {
namespace {
const std::string_view kClassNameGeneral{"general"};
const std::string_view kClassNameConnection{"connection"};
const std::string_view kClassNameParse{"parse"};
const std::string_view kClassNameAuthorization{"authorization"};
const std::string_view kClassNameTableAccess{"table_access"};
const std::string_view kClassNameGlobalVariable{"global_variable"};
const std::string_view kClassNameServerStartup{"server_startup"};
const std::string_view kClassNameServerShutdown{"server_shutdown"};
const std::string_view kClassNameCommand{"command"};
const std::string_view kClassNameQuery{"query"};
const std::string_view kClassNameStoredProgram{"stored_program"};
const std::string_view kClassNameAuthentication{"authentication"};
const std::string_view kClassNameMessage{"message"};

const std::string_view kSubclassNameGeneralLog{"log"};
const std::string_view kSubclassNameGeneralError{"error"};
const std::string_view kSubclassNameGeneralResult{"result"};
const std::string_view kSubclassNameGeneralStatus{"status"};
const std::string_view kSubclassNamePreparse{"preparse"};
const std::string_view kSubclassNamePostparse{"postparse"};
const std::string_view kSubclassNameUser{"user"};
const std::string_view kSubclassNameRead{"read"};
const std::string_view kSubclassNameInsert{"insert"};
const std::string_view kSubclassNameUpdate{"update"};
const std::string_view kSubclassNameDelete{"delete"};
const std::string_view kSubclassNameGet{"get"};
const std::string_view kSubclassNameSet{"set"};
const std::string_view kSubclassNameStartup{"startup"};
const std::string_view kSubclassNameShutdown{"shutdown"};
const std::string_view kSubclassNameEnd{"end"};
const std::string_view kSubclassNameStart{"start"};
const std::string_view kSubclassNameNestedStart{"nested_start"};
const std::string_view kSubclassNameStatusEnd{"status_end"};
const std::string_view kSubclassNameNestedStatusEnd{"nested_status_end"};
const std::string_view kSubclassNameExecute{"execute"};
const std::string_view kSubclassNameFlush{"flush"};
const std::string_view kSubclassNameAuthidCreate{"authid_create"};
const std::string_view kSubclassNameCredentialChange{"credential_change"};
const std::string_view kSubclassNameAuthidRename{"authid_rename"};
const std::string_view kSubclassNameAuthidDrop{"authid_drop"};
const std::string_view kSubclassNameConnect{"connect"};
const std::string_view kSubclassNameDisconnect{"disconnect"};
const std::string_view kSubclassNameChangeUser{"change_user"};
const std::string_view kSubclassNamePreAuthenticate{"pre_authenticate"};
const std::string_view kSubclassNameInternal{"internal"};

const std::string_view kNameUnknown{"unknown"};

std::string_view event_class_to_string(mysql_event_class_t event_class) {
  switch (event_class) {
    case MYSQL_AUDIT_GENERAL_CLASS:
      return kClassNameGeneral;
    case MYSQL_AUDIT_CONNECTION_CLASS:
      return kClassNameConnection;
    case MYSQL_AUDIT_PARSE_CLASS:
      return kClassNameParse;
    case MYSQL_AUDIT_AUTHORIZATION_CLASS:
      return kClassNameAuthorization;
    case MYSQL_AUDIT_TABLE_ACCESS_CLASS:
      return kClassNameTableAccess;
    case MYSQL_AUDIT_GLOBAL_VARIABLE_CLASS:
      return kClassNameGlobalVariable;
    case MYSQL_AUDIT_SERVER_STARTUP_CLASS:
      return kClassNameServerStartup;
    case MYSQL_AUDIT_SERVER_SHUTDOWN_CLASS:
      return kClassNameServerShutdown;
    case MYSQL_AUDIT_COMMAND_CLASS:
      return kClassNameCommand;
    case MYSQL_AUDIT_QUERY_CLASS:
      return kClassNameQuery;
    case MYSQL_AUDIT_STORED_PROGRAM_CLASS:
      return kClassNameStoredProgram;
    case MYSQL_AUDIT_AUTHENTICATION_CLASS:
      return kClassNameAuthentication;
    case MYSQL_AUDIT_MESSAGE_CLASS:
      return kClassNameMessage;
    default:
      assert(false);
  }

  return kNameUnknown;
}

std::string_view event_subclass_to_string(
    mysql_event_general_subclass_t event_subclass) {
  switch (event_subclass) {
    case MYSQL_AUDIT_GENERAL_LOG:
      return kSubclassNameGeneralLog;
    case MYSQL_AUDIT_GENERAL_ERROR:
      return kSubclassNameGeneralError;
    case MYSQL_AUDIT_GENERAL_RESULT:
      return kSubclassNameGeneralResult;
    case MYSQL_AUDIT_GENERAL_STATUS:
      return kSubclassNameGeneralStatus;
    default:
      assert(false);
  }

  return kNameUnknown;
}

std::string_view event_subclass_to_string(
    mysql_event_parse_subclass_t event_subclass) {
  switch (event_subclass) {
    case MYSQL_AUDIT_PARSE_PREPARSE:
      return kSubclassNamePreparse;
    case MYSQL_AUDIT_PARSE_POSTPARSE:
      return kSubclassNamePostparse;
    default:
      assert(false);
  }

  return kNameUnknown;
}

std::string_view event_subclass_to_string(
    mysql_event_connection_subclass_t event_subclass) {
  switch (event_subclass) {
    case MYSQL_AUDIT_CONNECTION_CONNECT:
      return kSubclassNameConnect;
    case MYSQL_AUDIT_CONNECTION_DISCONNECT:
      return kSubclassNameDisconnect;
    case MYSQL_AUDIT_CONNECTION_CHANGE_USER:
      return kSubclassNameChangeUser;
    case MYSQL_AUDIT_CONNECTION_PRE_AUTHENTICATE:
      return kSubclassNamePreAuthenticate;
    default:
      assert(false);
  }

  return kNameUnknown;
}

std::string_view event_subclass_to_string(
    mysql_event_table_access_subclass_t event_subclass) {
  switch (event_subclass) {
    case MYSQL_AUDIT_TABLE_ACCESS_READ:
      return kSubclassNameRead;
    case MYSQL_AUDIT_TABLE_ACCESS_INSERT:
      return kSubclassNameInsert;
    case MYSQL_AUDIT_TABLE_ACCESS_UPDATE:
      return kSubclassNameUpdate;
    case MYSQL_AUDIT_TABLE_ACCESS_DELETE:
      return kSubclassNameDelete;
    default:
      assert(false);
  }

  return kNameUnknown;
}

std::string_view event_subclass_to_string(
    mysql_event_global_variable_subclass_t event_subclass) {
  switch (event_subclass) {
    case MYSQL_AUDIT_GLOBAL_VARIABLE_GET:
      return kSubclassNameGet;
    case MYSQL_AUDIT_GLOBAL_VARIABLE_SET:
      return kSubclassNameSet;
    default:
      assert(false);
  }

  return kNameUnknown;
}

std::string_view event_subclass_to_string(
    mysql_event_server_startup_subclass_t event_subclass) {
  switch (event_subclass) {
    case MYSQL_AUDIT_SERVER_STARTUP_STARTUP:
      return kSubclassNameStartup;
    default:
      assert(false);
  }

  return kNameUnknown;
}

std::string_view event_subclass_to_string(
    mysql_event_server_shutdown_subclass_t event_subclass) {
  switch (event_subclass) {
    case MYSQL_AUDIT_SERVER_SHUTDOWN_SHUTDOWN:
      return kSubclassNameShutdown;
    default:
      assert(false);
  }

  return kNameUnknown;
}

std::string_view event_subclass_to_string(
    mysql_event_command_subclass_t event_subclass) {
  switch (event_subclass) {
    case MYSQL_AUDIT_COMMAND_START:
      return kSubclassNameStart;
    case MYSQL_AUDIT_COMMAND_END:
      return kSubclassNameEnd;
    default:
      assert(false);
  }

  return kNameUnknown;
}

std::string_view event_subclass_to_string(
    mysql_event_query_subclass_t event_subclass) {
  switch (event_subclass) {
    case MYSQL_AUDIT_QUERY_START:
      return kSubclassNameStart;
    case MYSQL_AUDIT_QUERY_NESTED_START:
      return kSubclassNameNestedStart;
    case MYSQL_AUDIT_QUERY_STATUS_END:
      return kSubclassNameStatusEnd;
    case MYSQL_AUDIT_QUERY_NESTED_STATUS_END:
      return kSubclassNameNestedStatusEnd;
    default:
      assert(false);
  }

  return kNameUnknown;
}

std::string_view event_subclass_to_string(
    mysql_event_stored_program_subclass_t event_subclass) {
  switch (event_subclass) {
    case MYSQL_AUDIT_STORED_PROGRAM_EXECUTE:
      return kSubclassNameExecute;
    default:
      assert(false);
  }

  return kNameUnknown;
}

std::string_view event_subclass_to_string(
    mysql_event_authentication_subclass_t event_subclass) {
  switch (event_subclass) {
    case MYSQL_AUDIT_AUTHENTICATION_FLUSH:
      return kSubclassNameFlush;
    case MYSQL_AUDIT_AUTHENTICATION_AUTHID_CREATE:
      return kSubclassNameAuthidCreate;
    case MYSQL_AUDIT_AUTHENTICATION_CREDENTIAL_CHANGE:
      return kSubclassNameCredentialChange;
    case MYSQL_AUDIT_AUTHENTICATION_AUTHID_RENAME:
      return kSubclassNameAuthidRename;
    case MYSQL_AUDIT_AUTHENTICATION_AUTHID_DROP:
      return kSubclassNameAuthidDrop;
    default:
      assert(false);
  }

  return kNameUnknown;
}

std::string_view event_subclass_to_string(
    mysql_event_message_subclass_t event_subclass) {
  switch (event_subclass) {
    case MYSQL_AUDIT_MESSAGE_INTERNAL:
      return kSubclassNameInternal;
    case MYSQL_AUDIT_MESSAGE_USER:
      return kSubclassNameUser;
    default:
      assert(false);
  }

  return kNameUnknown;
}

inline std::string lexcstring_to_string(const MYSQL_LEX_CSTRING *cstring) {
  return cstring != nullptr && cstring->str != nullptr &&
                 std::strlen(cstring->str) > 0
             ? cstring->str
             : "";
}

inline std::string lexcstring_len_to_string(const MYSQL_LEX_CSTRING *cstring) {
  return cstring != nullptr ? std::to_string(cstring->length) : "0";
}

}  // namespace

AuditRecordVariant get_audit_record(mysql_event_class_t event_class,
                                    const void *event) {
  switch (event_class) {
    case MYSQL_AUDIT_GENERAL_CLASS: {
      return AuditRecordVariant{
          std::in_place_index<0>,
          AuditRecordGeneral{event_class_to_string(event_class),
                             event_subclass_to_string(
                                 static_cast<const mysql_event_general *>(event)
                                     ->event_subclass),
                             event_class,
                             static_cast<const mysql_event_general *>(event),
                             {}}};
    }
    case MYSQL_AUDIT_CONNECTION_CLASS: {
      return AuditRecordVariant{
          std::in_place_index<1>,
          AuditRecordConnection{
              event_class_to_string(event_class),
              event_subclass_to_string(
                  static_cast<const mysql_event_connection *>(event)
                      ->event_subclass),
              event_class,
              static_cast<const mysql_event_connection *>(event),
              {}}};
    }
    case MYSQL_AUDIT_PARSE_CLASS: {
      return AuditRecordVariant{
          std::in_place_index<2>,
          AuditRecordParse{event_class_to_string(event_class),
                           event_subclass_to_string(
                               static_cast<const mysql_event_parse *>(event)
                                   ->event_subclass),
                           event_class,
                           static_cast<const mysql_event_parse *>(event),
                           {}}};
    }
    case MYSQL_AUDIT_TABLE_ACCESS_CLASS: {
      return AuditRecordVariant{
          std::in_place_index<3>,
          AuditRecordTableAccess{
              event_class_to_string(event_class),
              event_subclass_to_string(
                  static_cast<const mysql_event_table_access *>(event)
                      ->event_subclass),
              event_class,
              static_cast<const mysql_event_table_access *>(event),
              {}}};
    }
    case MYSQL_AUDIT_GLOBAL_VARIABLE_CLASS: {
      return AuditRecordVariant{
          std::in_place_index<4>,
          AuditRecordGlobalVariable{
              event_class_to_string(event_class),
              event_subclass_to_string(
                  static_cast<const mysql_event_global_variable *>(event)
                      ->event_subclass),
              event_class,
              static_cast<const mysql_event_global_variable *>(event),
              {}}};
    }
    case MYSQL_AUDIT_SERVER_STARTUP_CLASS: {
      return AuditRecordVariant{
          std::in_place_index<5>,
          AuditRecordServerStartup{
              event_class_to_string(event_class),
              event_subclass_to_string(
                  static_cast<const mysql_event_server_startup *>(event)
                      ->event_subclass),
              event_class,
              static_cast<const mysql_event_server_startup *>(event),
              {}}};
    }
    case MYSQL_AUDIT_SERVER_SHUTDOWN_CLASS: {
      return AuditRecordVariant{
          std::in_place_index<6>,
          AuditRecordServerShutdown{
              event_class_to_string(event_class),
              event_subclass_to_string(
                  static_cast<const mysql_event_server_shutdown *>(event)
                      ->event_subclass),
              event_class,
              static_cast<const mysql_event_server_shutdown *>(event),
              {}}};
    }
    case MYSQL_AUDIT_COMMAND_CLASS: {
      return AuditRecordVariant{
          std::in_place_index<7>,
          AuditRecordCommand{event_class_to_string(event_class),
                             event_subclass_to_string(
                                 static_cast<const mysql_event_command *>(event)
                                     ->event_subclass),
                             event_class,
                             static_cast<const mysql_event_command *>(event),
                             {}}};
    }
    case MYSQL_AUDIT_QUERY_CLASS: {
      return AuditRecordVariant{
          std::in_place_index<8>,
          AuditRecordQuery{event_class_to_string(event_class),
                           event_subclass_to_string(
                               static_cast<const mysql_event_query *>(event)
                                   ->event_subclass),
                           event_class,
                           static_cast<const mysql_event_query *>(event),
                           {}}};
    }
    case MYSQL_AUDIT_STORED_PROGRAM_CLASS: {
      return AuditRecordVariant{
          std::in_place_index<9>,
          AuditRecordStoredProgram{
              event_class_to_string(event_class),
              event_subclass_to_string(
                  static_cast<const mysql_event_stored_program *>(event)
                      ->event_subclass),
              event_class,
              static_cast<const mysql_event_stored_program *>(event),
              {}}};
    }
    case MYSQL_AUDIT_AUTHENTICATION_CLASS: {
      return AuditRecordVariant{
          std::in_place_index<10>,
          AuditRecordAuthentication{
              event_class_to_string(event_class),
              event_subclass_to_string(
                  static_cast<const mysql_event_authentication *>(event)
                      ->event_subclass),
              event_class,
              static_cast<const mysql_event_authentication *>(event),
              {}}};
    }
    case MYSQL_AUDIT_MESSAGE_CLASS: {
      return AuditRecordVariant{
          std::in_place_index<11>,
          AuditRecordMessage{event_class_to_string(event_class),
                             event_subclass_to_string(
                                 static_cast<const mysql_event_message *>(event)
                                     ->event_subclass),
                             event_class,
                             static_cast<const mysql_event_message *>(event),
                             {}}};
    }
    default:
      break;
  }

  assert(false);

  return AuditRecordVariant{
      std::in_place_index<12>,
      AuditRecordUnknown{kNameUnknown, kNameUnknown, event_class, event, {}}};
}

void update_connection_type_pseudo_to_numeric(std::string &type) {
  static const std::unordered_map<std::string, std::string>
      connection_type_pseudo{
          {"::undefined", "0"},  {"::tcp/ip", "1"}, {"::socket", "2"},
          {"::named_pipe", "3"}, {"::ssl", "4"},    {"::shared_memory", "5"},
      };

  const auto it = connection_type_pseudo.find(type);
  if (it != connection_type_pseudo.cend()) {
    type = it->second;
  }
}

AuditRecordFieldsList get_audit_record_fields(
    const AuditRecordGeneral &record) {
  const auto *event = record.event;
  return {
      {"general_error_code", std::to_string(event->general_error_code)},
      {"general_thread_id", std::to_string(event->general_thread_id)},
      {"general_user.str", lexcstring_to_string(&event->general_user)},
      {"general_user.length", lexcstring_len_to_string(&event->general_user)},
      {"general_command.str", lexcstring_to_string(&event->general_command)},
      {"general_command.length",
       lexcstring_len_to_string(&event->general_command)},
      {"general_query.str", lexcstring_to_string(&event->general_query)},
      {"general_query.length", lexcstring_len_to_string(&event->general_query)},
      {"general_host.str", lexcstring_to_string(&event->general_host)},
      {"general_host.length", lexcstring_len_to_string(&event->general_host)},
      {"general_sql_command.str",
       lexcstring_to_string(&event->general_sql_command)},
      {"general_sql_command.length",
       lexcstring_len_to_string(&event->general_sql_command)},
      {"general_external_user.str",
       lexcstring_to_string(&event->general_external_user)},
      {"general_external_user.length",
       lexcstring_len_to_string(&event->general_external_user)},
      {"general_ip.str", lexcstring_to_string(&event->general_ip)},
      {"general_ip.length", lexcstring_len_to_string(&event->general_ip)},
  };
}

AuditRecordFieldsList get_audit_record_fields(
    const AuditRecordConnection &record) {
  const auto *event = record.event;
  return {
      {"status", std::to_string(event->status)},
      {"connection_id", std::to_string(event->connection_id)},
      {"user.str", lexcstring_to_string(&event->user)},
      {"user.length", lexcstring_len_to_string(&event->user)},
      {"priv_user.str", lexcstring_to_string(&event->priv_user)},
      {"priv_user.length", lexcstring_len_to_string(&event->priv_user)},
      {"external_user.str", lexcstring_to_string(&event->external_user)},
      {"external_user.length", lexcstring_len_to_string(&event->external_user)},
      {"proxy_user.str", lexcstring_to_string(&event->proxy_user)},
      {"proxy_user.length", lexcstring_len_to_string(&event->proxy_user)},
      {"host.str", lexcstring_to_string(&event->host)},
      {"host.length", lexcstring_len_to_string(&event->host)},
      {"ip.str", lexcstring_to_string(&event->ip)},
      {"ip.length", lexcstring_len_to_string(&event->ip)},
      {"database.str", lexcstring_to_string(&event->database)},
      {"database.length", lexcstring_len_to_string(&event->database)},
      {"connection_type", std::to_string(event->connection_type)},
  };
}

AuditRecordFieldsList get_audit_record_fields(const AuditRecordParse &record) {
  const auto *event = record.event;
  return {
      {"query.str", lexcstring_to_string(&event->query)},
      {"query.length", lexcstring_len_to_string(&event->query)},
      {"rewritten_query.str", lexcstring_to_string(event->rewritten_query)},
      {"rewritten_query.length",
       lexcstring_len_to_string(event->rewritten_query)},
  };
}

AuditRecordFieldsList get_audit_record_fields(
    const AuditRecordTableAccess &record) {
  const auto *event = record.event;
  return {
      {"connection_id", std::to_string(event->connection_id)},
      {"sql_command_id", std::to_string(event->sql_command_id)},
      {"query.str", lexcstring_to_string(&event->query)},
      {"query.length", lexcstring_len_to_string(&event->query)},
      {"table_database.str", lexcstring_to_string(&event->table_database)},
      {"table_database.length",
       lexcstring_len_to_string(&event->table_database)},
      {"table_name.str", lexcstring_to_string(&event->table_name)},
      {"table_name.length", lexcstring_len_to_string(&event->table_name)},
  };
}

AuditRecordFieldsList get_audit_record_fields(
    const AuditRecordGlobalVariable &record) {
  const auto *event = record.event;
  return {
      {"connection_id", std::to_string(event->connection_id)},
      {"sql_command_id", std::to_string(event->sql_command_id)},
      {"variable_name.str", lexcstring_to_string(&event->variable_name)},
      {"variable_name.length", lexcstring_len_to_string(&event->variable_name)},
      {"variable_value.str", lexcstring_to_string(&event->variable_value)},
      {"variable_value.length",
       lexcstring_len_to_string(&event->variable_value)},
  };
}

AuditRecordFieldsList get_audit_record_fields(
    const AuditRecordServerStartup &record [[maybe_unused]]) {
  return {};
}

AuditRecordFieldsList get_audit_record_fields(
    const AuditRecordServerShutdown &record) {
  const auto *event = record.event;
  return {
      {"exit_code", std::to_string(event->exit_code)},
      {"reason", std::to_string(event->reason)},
  };
}

AuditRecordFieldsList get_audit_record_fields(
    const AuditRecordCommand &record) {
  const auto *event = record.event;
  return {
      {"status", std::to_string(event->status)},
      {"connection_id", std::to_string(event->connection_id)},
      {"command_id", std::to_string(event->command_id)},
  };
}

AuditRecordFieldsList get_audit_record_fields(const AuditRecordQuery &record) {
  const auto *event = record.event;
  return {
      {"status", std::to_string(event->status)},
      {"connection_id", std::to_string(event->connection_id)},
      {"sql_command_id", std::to_string(event->sql_command_id)},
      {"query.str", lexcstring_to_string(&event->query)},
      {"query.length", lexcstring_len_to_string(&event->query)},
  };
}

AuditRecordFieldsList get_audit_record_fields(
    const AuditRecordStoredProgram &record) {
  const auto *event = record.event;
  return {
      {"connection_id", std::to_string(event->connection_id)},
      {"sql_command_id", std::to_string(event->sql_command_id)},
      {"query.str", lexcstring_to_string(&event->query)},
      {"query.length", lexcstring_len_to_string(&event->query)},
      {"database.str", lexcstring_to_string(&event->database)},
      {"database.length", lexcstring_len_to_string(&event->database)},
      {"name.str", lexcstring_to_string(&event->name)},
      {"name.length", lexcstring_len_to_string(&event->name)},
  };
}

AuditRecordFieldsList get_audit_record_fields(
    const AuditRecordAuthentication &record) {
  const auto *event = record.event;
  return {
      {"status", std::to_string(event->status)},
      {"connection_id", std::to_string(event->connection_id)},
      {"sql_command_id", std::to_string(event->sql_command_id)},
      {"query.str", lexcstring_to_string(&event->query)},
      {"query.length", lexcstring_len_to_string(&event->query)},
      {"user.str", lexcstring_to_string(&event->user)},
      {"user.length", lexcstring_len_to_string(&event->user)},
      {"host.str", lexcstring_to_string(&event->host)},
      {"host.length", lexcstring_len_to_string(&event->host)},
      {"authentication_plugin.str",
       lexcstring_to_string(&event->authentication_plugin)},
      {"authentication_plugin.length",
       lexcstring_len_to_string(&event->authentication_plugin)},
      {"new_user.str", lexcstring_to_string(&event->new_user)},
      {"new_user.length", lexcstring_len_to_string(&event->new_user)},
      {"new_host.str", lexcstring_to_string(&event->new_host)},
      {"new_host.length", lexcstring_len_to_string(&event->new_host)},
      {"is_role", std::to_string(event->is_role)},
  };
}

AuditRecordFieldsList get_audit_record_fields(
    const AuditRecordMessage &record) {
  const auto *event = record.event;
  return {
      {"component.str", lexcstring_to_string(&event->component)},
      {"component.length", lexcstring_len_to_string(&event->component)},
      {"producer.str", lexcstring_to_string(&event->producer)},
      {"producer.length", lexcstring_len_to_string(&event->producer)},
      {"message.str", lexcstring_to_string(&event->message)},
      {"message.length", lexcstring_len_to_string(&event->message)},
  };
}

AuditRecordFieldsList get_audit_record_fields(const AuditRecordUnknown &record
                                              [[maybe_unused]]) {
  return {};
}

}  // namespace audit_log_filter
