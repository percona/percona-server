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

#include "components/audit_log_filter/audit_record.h"
#include "components/audit_log_filter/audit_error_log.h"

#include <mysql/components/services/event_tracking_authentication_service.h>
#include <mysql/components/services/event_tracking_command_service.h>
#include <mysql/components/services/event_tracking_connection_service.h>
#include <mysql/components/services/event_tracking_general_service.h>
#include <mysql/components/services/event_tracking_global_variable_service.h>
#include <mysql/components/services/event_tracking_lifecycle_service.h>
#include <mysql/components/services/event_tracking_message_service.h>
#include <mysql/components/services/event_tracking_parse_service.h>
#include <mysql/components/services/event_tracking_query_service.h>
#include <mysql/components/services/event_tracking_stored_program_service.h>
#include <mysql/components/services/event_tracking_table_access_service.h>

#include <cstring>
#include <unordered_map>

namespace audit_log_filter {
namespace {
const std::string_view kClassNameGeneral{"general"};
const std::string_view kClassNameConnection{"connection"};
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
const std::string_view kClassNameParse{"parse"};
const std::string_view kClassNameInternalAudit{"audit"};

const std::string_view kSubclassNameGeneralLog{"log"};
const std::string_view kSubclassNameGeneralError{"error"};
const std::string_view kSubclassNameGeneralResult{"result"};
const std::string_view kSubclassNameGeneralStatus{"status"};
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
const std::string_view kSubclassNameMessageInternal{"internal"};
const std::string_view kSubclassNameInternalAudit{"audit"};
const std::string_view kSubclassNameInternalNoAudit{"noaudit"};
const std::string_view kSubclassNameParseRewriteNone{"rewrite_none"};
const std::string_view kSubclassNameParseRewriteQueryRewritten{
    "rewrite_query_rewritten"};
const std::string_view kSubclassNameParseRewritePreparedStatement{
    "rewrite_prepared_statement"};

const std::string_view kNameUnknown{"unknown"};

std::string_view event_class_to_string(audit_event_class_t event_class) {
  switch (event_class) {
    case audit_event_class_t::AUDIT_GENERAL_CLASS:
      return kClassNameGeneral;
    case audit_event_class_t::AUDIT_CONNECTION_CLASS:
      return kClassNameConnection;
    case audit_event_class_t::AUDIT_AUTHORIZATION_CLASS:
      return kClassNameAuthorization;
    case audit_event_class_t::AUDIT_TABLE_ACCESS_CLASS:
      return kClassNameTableAccess;
    case audit_event_class_t::AUDIT_GLOBAL_VARIABLE_CLASS:
      return kClassNameGlobalVariable;
    case audit_event_class_t::AUDIT_SERVER_STARTUP_CLASS:
      return kClassNameServerStartup;
    case audit_event_class_t::AUDIT_SERVER_SHUTDOWN_CLASS:
      return kClassNameServerShutdown;
    case audit_event_class_t::AUDIT_COMMAND_CLASS:
      return kClassNameCommand;
    case audit_event_class_t::AUDIT_QUERY_CLASS:
      return kClassNameQuery;
    case audit_event_class_t::AUDIT_STORED_PROGRAM_CLASS:
      return kClassNameStoredProgram;
    case audit_event_class_t::AUDIT_AUTHENTICATION_CLASS:
      return kClassNameAuthentication;
    case audit_event_class_t::AUDIT_MESSAGE_CLASS:
      return kClassNameMessage;
    case audit_event_class_t::AUDIT_PARSE_CLASS:
      return kClassNameParse;
    default:
      assert(false);
  }

  return kNameUnknown;
}

std::string_view event_subclass_to_string(
    const mysql_event_tracking_general_data *event) {
  switch (event->event_subclass) {
    case EVENT_TRACKING_GENERAL_LOG:
      return kSubclassNameGeneralLog;
    case EVENT_TRACKING_GENERAL_ERROR:
      return kSubclassNameGeneralError;
    case EVENT_TRACKING_GENERAL_RESULT:
      return kSubclassNameGeneralResult;
    case EVENT_TRACKING_GENERAL_STATUS:
      return kSubclassNameGeneralStatus;
    default:
      assert(false);
  }

  return kNameUnknown;
}

std::string_view event_subclass_to_string(
    const mysql_event_tracking_connection_data *event) {
  switch (event->event_subclass) {
    case EVENT_TRACKING_CONNECTION_CONNECT:
      return kSubclassNameConnect;
    case EVENT_TRACKING_CONNECTION_DISCONNECT:
      return kSubclassNameDisconnect;
    case EVENT_TRACKING_CONNECTION_CHANGE_USER:
      return kSubclassNameChangeUser;
    case EVENT_TRACKING_CONNECTION_PRE_AUTHENTICATE:
      return kSubclassNamePreAuthenticate;
    default:
      assert(false);
  }

  return kNameUnknown;
}

std::string_view event_subclass_to_string(
    const mysql_event_tracking_table_access_data *event) {
  switch (event->event_subclass) {
    case EVENT_TRACKING_TABLE_ACCESS_READ:
      return kSubclassNameRead;
    case EVENT_TRACKING_TABLE_ACCESS_INSERT:
      return kSubclassNameInsert;
    case EVENT_TRACKING_TABLE_ACCESS_UPDATE:
      return kSubclassNameUpdate;
    case EVENT_TRACKING_TABLE_ACCESS_DELETE:
      return kSubclassNameDelete;
    default:
      assert(false);
  }

  return kNameUnknown;
}

std::string_view event_subclass_to_string(
    const mysql_event_tracking_global_variable_data *event) {
  switch (event->event_subclass) {
    case EVENT_TRACKING_GLOBAL_VARIABLE_GET:
      return kSubclassNameGet;
    case EVENT_TRACKING_GLOBAL_VARIABLE_SET:
      return kSubclassNameSet;
    default:
      assert(false);
  }

  return kNameUnknown;
}

std::string_view event_subclass_to_string(
    const mysql_event_tracking_startup_data *event) {
  switch (event->event_subclass) {
    case EVENT_TRACKING_STARTUP_STARTUP:
      return kSubclassNameStartup;
    default:
      assert(false);
  }

  return kNameUnknown;
}

std::string_view event_subclass_to_string(
    const mysql_event_tracking_shutdown_data *event) {
  switch (event->event_subclass) {
    case EVENT_TRACKING_SHUTDOWN_SHUTDOWN:
      return kSubclassNameShutdown;
    default:
      assert(false);
  }

  return kNameUnknown;
}

std::string_view event_subclass_to_string(
    const mysql_event_tracking_command_data *event) {
  switch (event->event_subclass) {
    case EVENT_TRACKING_COMMAND_START:
      return kSubclassNameStart;
    case EVENT_TRACKING_COMMAND_END:
      return kSubclassNameEnd;
    default:
      assert(false);
  }

  return kNameUnknown;
}

std::string_view event_subclass_to_string(
    const mysql_event_tracking_query_data *event) {
  switch (event->event_subclass) {
    case EVENT_TRACKING_QUERY_START:
      return kSubclassNameStart;
    case EVENT_TRACKING_QUERY_NESTED_START:
      return kSubclassNameNestedStart;
    case EVENT_TRACKING_QUERY_STATUS_END:
      return kSubclassNameStatusEnd;
    case EVENT_TRACKING_QUERY_NESTED_STATUS_END:
      return kSubclassNameNestedStatusEnd;
    default:
      assert(false);
  }

  return kNameUnknown;
}

std::string_view event_subclass_to_string(
    const mysql_event_tracking_stored_program_data *event) {
  switch (event->event_subclass) {
    case EVENT_TRACKING_STORED_PROGRAM_EXECUTE:
      return kSubclassNameExecute;
    default:
      assert(false);
  }

  return kNameUnknown;
}

std::string_view event_subclass_to_string(
    const mysql_event_tracking_authentication_data *event) {
  switch (event->event_subclass) {
    case EVENT_TRACKING_AUTHENTICATION_FLUSH:
      return kSubclassNameFlush;
    case EVENT_TRACKING_AUTHENTICATION_AUTHID_CREATE:
      return kSubclassNameAuthidCreate;
    case EVENT_TRACKING_AUTHENTICATION_CREDENTIAL_CHANGE:
      return kSubclassNameCredentialChange;
    case EVENT_TRACKING_AUTHENTICATION_AUTHID_RENAME:
      return kSubclassNameAuthidRename;
    case EVENT_TRACKING_AUTHENTICATION_AUTHID_DROP:
      return kSubclassNameAuthidDrop;
    default:
      assert(false);
  }

  return kNameUnknown;
}

std::string_view event_subclass_to_string(
    const mysql_event_tracking_message_data *event) {
  switch (event->event_subclass) {
    case EVENT_TRACKING_MESSAGE_INTERNAL:
      return kSubclassNameMessageInternal;
    case EVENT_TRACKING_MESSAGE_USER:
      return kSubclassNameUser;
    default:
      assert(false);
  }

  return kNameUnknown;
}

std::string_view event_subclass_to_string(
    const mysql_event_tracking_parse_data *event) {
  switch (event->event_subclass) {
    case EVENT_TRACKING_PARSE_REWRITE_NONE:
      return kSubclassNameParseRewriteNone;
    case EVENT_TRACKING_PARSE_REWRITE_QUERY_REWRITTEN:
      return kSubclassNameParseRewriteQueryRewritten;
    case EVENT_TRACKING_PARSE_REWRITE_IS_PREPARED_STATEMENT:
      return kSubclassNameParseRewritePreparedStatement;
    default:
      assert(false);
  }

  return kNameUnknown;
}

std::string_view event_subclass_to_string(
    const internal_event_tracking_audit_data *event) {
  switch (event->event_subclass) {
    case INTERNAL_EVENT_TRACKING_AUDIT_AUDIT:
      return kSubclassNameInternalAudit;
    case INTERNAL_EVENT_TRACKING_AUDIT_NOAUDIT:
      return kSubclassNameInternalNoAudit;
    default:
      assert(false);
  }

  return kNameUnknown;
}

inline std::string mysql_cstring_to_string(
    const mysql_cstring_with_length *str) {
  return str != nullptr && str->str != nullptr && std::strlen(str->str) > 0
             ? str->str
             : "";
}

inline std::string mysql_cstring_len_to_string(
    const mysql_cstring_with_length *str) {
  return str != nullptr ? std::to_string(str->length) : "0";
}

}  // namespace

AuditRecordVariant get_audit_record(audit_event_class_t event_class,
                                    const void *event) {
  switch (event_class) {
    case audit_event_class_t::AUDIT_GENERAL_CLASS: {
      return AuditRecordVariant{
          std::in_place_index<0>,
          AuditRecordGeneral{
              event_class_to_string(event_class),
              event_subclass_to_string(
                  static_cast<const mysql_event_tracking_general_data *>(
                      event)),
              event_class,
              static_cast<const mysql_event_tracking_general_data *>(event),
              {}}};
    }
    case audit_event_class_t::AUDIT_CONNECTION_CLASS: {
      return AuditRecordVariant{
          std::in_place_index<1>,
          AuditRecordConnection{
              event_class_to_string(event_class),
              event_subclass_to_string(
                  static_cast<const mysql_event_tracking_connection_data *>(
                      event)),
              event_class,
              static_cast<const mysql_event_tracking_connection_data *>(event),
              {}}};
    }
    case audit_event_class_t::AUDIT_TABLE_ACCESS_CLASS: {
      return AuditRecordVariant{
          std::in_place_index<2>,
          AuditRecordTableAccess{
              event_class_to_string(event_class),
              event_subclass_to_string(
                  static_cast<const mysql_event_tracking_table_access_data *>(
                      event)),
              event_class,
              static_cast<const mysql_event_tracking_table_access_data *>(
                  event),
              {}}};
    }
    case audit_event_class_t::AUDIT_GLOBAL_VARIABLE_CLASS: {
      return AuditRecordVariant{
          std::in_place_index<3>,
          AuditRecordGlobalVariable{
              event_class_to_string(event_class),
              event_subclass_to_string(
                  static_cast<const mysql_event_tracking_global_variable_data
                                  *>(event)),
              event_class,
              static_cast<const mysql_event_tracking_global_variable_data *>(
                  event),
              {}}};
    }
    case audit_event_class_t::AUDIT_SERVER_STARTUP_CLASS: {
      return AuditRecordVariant{
          std::in_place_index<4>,
          AuditRecordServerStartup{
              event_class_to_string(event_class),
              event_subclass_to_string(
                  static_cast<const mysql_event_tracking_startup_data *>(
                      event)),
              event_class,
              static_cast<const mysql_event_tracking_startup_data *>(event),
              {}}};
    }
    case audit_event_class_t::AUDIT_SERVER_SHUTDOWN_CLASS: {
      return AuditRecordVariant{
          std::in_place_index<5>,
          AuditRecordServerShutdown{
              event_class_to_string(event_class),
              event_subclass_to_string(
                  static_cast<const mysql_event_tracking_shutdown_data *>(
                      event)),
              event_class,
              static_cast<const mysql_event_tracking_shutdown_data *>(event),
              {}}};
    }
    case audit_event_class_t::AUDIT_COMMAND_CLASS: {
      return AuditRecordVariant{
          std::in_place_index<6>,
          AuditRecordCommand{
              event_class_to_string(event_class),
              event_subclass_to_string(
                  static_cast<const mysql_event_tracking_command_data *>(
                      event)),
              event_class,
              static_cast<const mysql_event_tracking_command_data *>(event),
              {}}};
    }
    case audit_event_class_t::AUDIT_QUERY_CLASS: {
      return AuditRecordVariant{
          std::in_place_index<7>,
          AuditRecordQuery{
              event_class_to_string(event_class),
              event_subclass_to_string(
                  static_cast<const mysql_event_tracking_query_data *>(event)),
              event_class,
              static_cast<const mysql_event_tracking_query_data *>(event),
              {}}};
    }
    case audit_event_class_t::AUDIT_STORED_PROGRAM_CLASS: {
      return AuditRecordVariant{
          std::in_place_index<8>,
          AuditRecordStoredProgram{
              event_class_to_string(event_class),
              event_subclass_to_string(
                  static_cast<const mysql_event_tracking_stored_program_data *>(
                      event)),
              event_class,
              static_cast<const mysql_event_tracking_stored_program_data *>(
                  event),
              {}}};
    }
    case audit_event_class_t::AUDIT_AUTHENTICATION_CLASS: {
      return AuditRecordVariant{
          std::in_place_index<9>,
          AuditRecordAuthentication{
              event_class_to_string(event_class),
              event_subclass_to_string(
                  static_cast<const mysql_event_tracking_authentication_data *>(
                      event)),
              event_class,
              static_cast<const mysql_event_tracking_authentication_data *>(
                  event),
              {}}};
    }
    case audit_event_class_t::AUDIT_MESSAGE_CLASS: {
      return AuditRecordVariant{
          std::in_place_index<10>,
          AuditRecordMessage{
              event_class_to_string(event_class),
              event_subclass_to_string(
                  static_cast<const mysql_event_tracking_message_data *>(
                      event)),
              event_class,
              static_cast<const mysql_event_tracking_message_data *>(event),
              {}}};
    }
    case audit_event_class_t::AUDIT_PARSE_CLASS: {
      return AuditRecordVariant{
          std::in_place_index<11>,
          AuditRecordParse{
              event_class_to_string(event_class),
              event_subclass_to_string(
                  static_cast<const mysql_event_tracking_parse_data *>(event)),
              event_class,
              static_cast<const mysql_event_tracking_parse_data *>(event),
              {}}};
    }
    case audit_event_class_t::AUDIT_INTERNAL_AUDIT_CLASS: {
      return AuditRecordVariant{
          std::in_place_index<12>,
          AuditRecordAudit{
              kClassNameInternalAudit,
              event_subclass_to_string(
                  static_cast<const internal_event_tracking_audit_data *>(
                      event)),
              audit_event_class_t::AUDIT_INTERNAL_AUDIT_CLASS,
              static_cast<const internal_event_tracking_audit_data *>(event),
              {}}};
    }
    default:
      break;
  }

  assert(false);

  return AuditRecordVariant{
      std::in_place_index<13>,
      AuditRecordUnknown{kNameUnknown,
                         kNameUnknown,
                         audit_event_class_t::AUDIT_INTERNAL_UNKNOWN_CLASS,
                         event,
                         {}}};
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
      {"general_error_code", std::to_string(event->error_code)},
      {"general_connection_id", std::to_string(event->connection_id)},
      {"general_user.str", mysql_cstring_to_string(&event->user)},
      {"general_user.length", mysql_cstring_len_to_string(&event->user)},
      {"general_host.str", mysql_cstring_to_string(&event->host)},
      {"general_host.length", mysql_cstring_len_to_string(&event->host)},
      {"general_ip.str", mysql_cstring_to_string(&event->ip)},
      {"general_ip.length", mysql_cstring_len_to_string(&event->ip)},
  };
}

AuditRecordFieldsList get_audit_record_fields(
    const AuditRecordConnection &record) {
  const auto *event = record.event;
  return {
      {"status", std::to_string(event->status)},
      {"connection_id", std::to_string(event->connection_id)},
      {"user.str", mysql_cstring_to_string(&event->user)},
      {"user.length", mysql_cstring_len_to_string(&event->user)},
      {"priv_user.str", mysql_cstring_to_string(&event->priv_user)},
      {"priv_user.length", mysql_cstring_len_to_string(&event->priv_user)},
      {"external_user.str", mysql_cstring_to_string(&event->external_user)},
      {"external_user.length",
       mysql_cstring_len_to_string(&event->external_user)},
      {"proxy_user.str", mysql_cstring_to_string(&event->proxy_user)},
      {"proxy_user.length", mysql_cstring_len_to_string(&event->proxy_user)},
      {"host.str", mysql_cstring_to_string(&event->host)},
      {"host.length", mysql_cstring_len_to_string(&event->host)},
      {"ip.str", mysql_cstring_to_string(&event->ip)},
      {"ip.length", mysql_cstring_len_to_string(&event->ip)},
      {"database.str", mysql_cstring_to_string(&event->database)},
      {"database.length", mysql_cstring_len_to_string(&event->database)},
      {"connection_type", std::to_string(event->connection_type)},
  };
}

AuditRecordFieldsList get_audit_record_fields(
    const AuditRecordTableAccess &record) {
  const auto *event = record.event;
  return {
      {"connection_id", std::to_string(event->connection_id)},
      {"table_database.str", mysql_cstring_to_string(&event->table_database)},
      {"table_database.length",
       mysql_cstring_len_to_string(&event->table_database)},
      {"table_name.str", mysql_cstring_to_string(&event->table_name)},
      {"table_name.length", mysql_cstring_len_to_string(&event->table_name)},
  };
}

AuditRecordFieldsList get_audit_record_fields(
    const AuditRecordGlobalVariable &record) {
  const auto *event = record.event;
  return {
      {"connection_id", std::to_string(event->connection_id)},
      {"variable_name.str", mysql_cstring_to_string(&event->variable_name)},
      {"variable_name.length",
       mysql_cstring_len_to_string(&event->variable_name)},
      {"variable_value.str", mysql_cstring_to_string(&event->variable_value)},
      {"variable_value.length",
       mysql_cstring_len_to_string(&event->variable_value)},
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
      {"command.str", mysql_cstring_to_string(&event->command)},
      {"command.length", mysql_cstring_len_to_string(&event->command)},
  };
}

AuditRecordFieldsList get_audit_record_fields(const AuditRecordQuery &record) {
  const auto *event = record.event;
  return {
      {"status", std::to_string(event->status)},
      {"connection_id", std::to_string(event->connection_id)},
      {"sql_command_id", std::string(event->sql_command)},
      {"query.str", mysql_cstring_to_string(&event->query)},
      {"query.length", mysql_cstring_len_to_string(&event->query)},
      {"query_charset", std::string(event->query_charset)},
  };
}

AuditRecordFieldsList get_audit_record_fields(
    const AuditRecordStoredProgram &record) {
  const auto *event = record.event;
  return {
      {"connection_id", std::to_string(event->connection_id)},
      {"database.str", mysql_cstring_to_string(&event->database)},
      {"database.length", mysql_cstring_len_to_string(&event->database)},
      {"name.str", mysql_cstring_to_string(&event->name)},
      {"name.length", mysql_cstring_len_to_string(&event->name)},
  };
}

AuditRecordFieldsList get_audit_record_fields(
    const AuditRecordAuthentication &record) {
  const auto *event = record.event;
  return {
      {"status", std::to_string(event->status)},
      {"connection_id", std::to_string(event->connection_id)},
      {"user.str", mysql_cstring_to_string(&event->user)},
      {"user.length", mysql_cstring_len_to_string(&event->user)},
      {"host.str", mysql_cstring_to_string(&event->host)},
      {"host.length", mysql_cstring_len_to_string(&event->host)},
  };
}

AuditRecordFieldsList get_audit_record_fields(
    const AuditRecordMessage &record) {
  const auto *event = record.event;
  return {
      {"connection_id", std::to_string(event->connection_id)},
      {"component.str", mysql_cstring_to_string(&event->component)},
      {"component.length", mysql_cstring_len_to_string(&event->component)},
      {"producer.str", mysql_cstring_to_string(&event->producer)},
      {"producer.length", mysql_cstring_len_to_string(&event->producer)},
      {"message.str", mysql_cstring_to_string(&event->message)},
      {"message.length", mysql_cstring_len_to_string(&event->message)},
  };
}

AuditRecordFieldsList get_audit_record_fields(const AuditRecordParse &record) {
  const auto *event = record.event;
  return {
      {"connection_id", std::to_string(event->connection_id)},
      {"flags", std::to_string(event->flags != nullptr ? *event->flags : 0)},
      {"query.str", mysql_cstring_to_string(&event->query)},
      {"query.length", mysql_cstring_len_to_string(&event->query)},
      {"rewritten_query.str", mysql_cstring_to_string(event->rewritten_query)},
      {"rewritten_query.length",
       mysql_cstring_len_to_string(event->rewritten_query)},
  };
}

AuditRecordFieldsList get_audit_record_fields(const AuditRecordAudit &record
                                              [[maybe_unused]]) {
  const auto *event = record.event;
  return {{"server_id", std::to_string(event->server_id)}};
}

AuditRecordFieldsList get_audit_record_fields(const AuditRecordUnknown &record
                                              [[maybe_unused]]) {
  return {};
}

}  // namespace audit_log_filter
