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
#include <mysql/components/services/event_tracking_message_service.h>
#include <mysql/components/services/event_tracking_parse_service.h>
#include <mysql/components/services/event_tracking_query_service.h>
#include <mysql/components/services/event_tracking_stored_program_service.h>
#include <mysql/components/services/event_tracking_table_access_service.h>

#include <algorithm>
#include <array>
#include <cstring>
#include <unordered_map>

namespace audit_log_filter {
namespace {
constexpr std::string_view kClassNameGeneral{"general"};
constexpr std::string_view kClassNameConnection{"connection"};
constexpr std::string_view kClassNameAuthorization{"authorization"};
constexpr std::string_view kClassNameTableAccess{"table_access"};
constexpr std::string_view kClassNameGlobalVariable{"global_variable"};
constexpr std::string_view kClassNameCommand{"command"};
constexpr std::string_view kClassNameQuery{"query"};
constexpr std::string_view kClassNameStoredProgram{"stored_program"};
constexpr std::string_view kClassNameAuthentication{"authentication"};
constexpr std::string_view kClassNameMessage{"message"};
constexpr std::string_view kClassNameParse{"parse"};
constexpr std::string_view kClassNameInternalAudit{"audit"};

constexpr std::string_view kSubclassNameGeneralLog{"log"};
constexpr std::string_view kSubclassNameGeneralError{"error"};
constexpr std::string_view kSubclassNameGeneralResult{"result"};
constexpr std::string_view kSubclassNameGeneralStatus{"status"};
constexpr std::string_view kSubclassNameUser{"user"};
constexpr std::string_view kSubclassNameRead{"read"};
constexpr std::string_view kSubclassNameInsert{"insert"};
constexpr std::string_view kSubclassNameUpdate{"update"};
constexpr std::string_view kSubclassNameDelete{"delete"};
constexpr std::string_view kSubclassNameGet{"get"};
constexpr std::string_view kSubclassNameSet{"set"};
constexpr std::string_view kSubclassNameEnd{"end"};
constexpr std::string_view kSubclassNameStart{"start"};
constexpr std::string_view kSubclassNameNestedStart{"nested_start"};
constexpr std::string_view kSubclassNameStatusEnd{"status_end"};
constexpr std::string_view kSubclassNameNestedStatusEnd{"nested_status_end"};
constexpr std::string_view kSubclassNameExecute{"execute"};
constexpr std::string_view kSubclassNameFlush{"flush"};
constexpr std::string_view kSubclassNameAuthidCreate{"authid_create"};
constexpr std::string_view kSubclassNameCredentialChange{"credential_change"};
constexpr std::string_view kSubclassNameAuthidRename{"authid_rename"};
constexpr std::string_view kSubclassNameAuthidDrop{"authid_drop"};
constexpr std::string_view kSubclassNameConnect{"connect"};
constexpr std::string_view kSubclassNameDisconnect{"disconnect"};
constexpr std::string_view kSubclassNameChangeUser{"change_user"};
constexpr std::string_view kSubclassNamePreAuthenticate{"pre_authenticate"};
constexpr std::string_view kSubclassNameMessageInternal{"internal"};
constexpr std::string_view kSubclassNameInternalStartup{"startup"};
constexpr std::string_view kSubclassNameInternalShutdown{"shutdown"};
constexpr std::string_view kSubclassNameParsePreparse{"preparse"};
constexpr std::string_view kSubclassNameParsePostparse{"postparse"};

template <typename Container>
bool contains_string_view(std::string_view value,
                          const Container &valid_values) {
  return std::find(valid_values.cbegin(), valid_values.cend(), value) !=
         valid_values.cend();
}

template <typename>
struct UnsupportedFieldValueType : std::false_type {};

constexpr std::string_view kNameUnknown{"unknown"};

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
    case EVENT_TRACKING_PARSE_PREPARSE:
      return kSubclassNameParsePreparse;
    case EVENT_TRACKING_PARSE_POSTPARSE:
      return kSubclassNameParsePostparse;
    default:
      assert(false);
  }

  return kNameUnknown;
}

std::string_view event_subclass_to_string(
    const internal_event_tracking_audit_data *event) {
  switch (event->event_subclass) {
    case INTERNAL_EVENT_TRACKING_AUDIT_AUDIT:
      return kSubclassNameInternalStartup;
    case INTERNAL_EVENT_TRACKING_AUDIT_NOAUDIT:
      return kSubclassNameInternalShutdown;
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

inline uint64_t mysql_cstring_len_to_uint64(
    const mysql_cstring_with_length *str) {
  return str != nullptr ? static_cast<uint64_t>(str->length) : 0;
}

}  // namespace

std::string field_value_to_string(const AuditRecordFieldValue &value) {
  return std::visit(
      [](const auto &v) -> std::string {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, std::string>)
          return v;
        else
          return std::to_string(v);
      },
      value);
}

bool field_value_matches(const AuditRecordFieldValue &value,
                         const std::string &expected) {
  return std::visit(
      [&expected](const auto &v) -> bool {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, std::string>) {
          return v == expected;
        } else if constexpr (std::is_same_v<T, int64_t>) {
          try {
            size_t pos = 0;
            int64_t num = std::stoll(expected, &pos);
            return pos == expected.size() && v == num;
          } catch (...) {
            return false;
          }
        } else if constexpr (std::is_same_v<T, uint64_t>) {
          try {
            size_t pos = 0;
            uint64_t num = std::stoull(expected, &pos);
            return pos == expected.size() && v == num;
          } catch (...) {
            return false;
          }
        } else {
          static_assert(UnsupportedFieldValueType<T>::value,
                        "Unsupported AuditRecordFieldValue alternative");
        }
      },
      value);
}

std::optional<AuditRecordVariant> get_audit_record(
    audit_event_class_t event_class, const void *event) {
  switch (event_class) {
    case audit_event_class_t::AUDIT_SERVER_STARTUP_CLASS:
    case audit_event_class_t::AUDIT_SERVER_SHUTDOWN_CLASS:
      return std::nullopt;
    case audit_event_class_t::AUDIT_GENERAL_CLASS: {
      return AuditRecordVariant{AuditRecordGeneral{
          event_class_to_string(event_class),
          event_subclass_to_string(
              static_cast<const mysql_event_tracking_general_data *>(event)),
          event_class,
          static_cast<const mysql_event_tracking_general_data *>(event),
          {}}};
    }
    case audit_event_class_t::AUDIT_CONNECTION_CLASS: {
      return AuditRecordVariant{AuditRecordConnection{
          event_class_to_string(event_class),
          event_subclass_to_string(
              static_cast<const mysql_event_tracking_connection_data *>(event)),
          event_class,
          static_cast<const mysql_event_tracking_connection_data *>(event),
          {}}};
    }
    case audit_event_class_t::AUDIT_TABLE_ACCESS_CLASS: {
      return AuditRecordVariant{AuditRecordTableAccess{
          event_class_to_string(event_class),
          event_subclass_to_string(
              static_cast<const mysql_event_tracking_table_access_data *>(
                  event)),
          event_class,
          static_cast<const mysql_event_tracking_table_access_data *>(event),
          {}}};
    }
    case audit_event_class_t::AUDIT_GLOBAL_VARIABLE_CLASS: {
      return AuditRecordVariant{AuditRecordGlobalVariable{
          event_class_to_string(event_class),
          event_subclass_to_string(
              static_cast<const mysql_event_tracking_global_variable_data *>(
                  event)),
          event_class,
          static_cast<const mysql_event_tracking_global_variable_data *>(event),
          {}}};
    }
    case audit_event_class_t::AUDIT_COMMAND_CLASS: {
      return AuditRecordVariant{AuditRecordCommand{
          event_class_to_string(event_class),
          event_subclass_to_string(
              static_cast<const mysql_event_tracking_command_data *>(event)),
          event_class,
          static_cast<const mysql_event_tracking_command_data *>(event),
          {}}};
    }
    case audit_event_class_t::AUDIT_QUERY_CLASS: {
      return AuditRecordVariant{AuditRecordQuery{
          event_class_to_string(event_class),
          event_subclass_to_string(
              static_cast<const mysql_event_tracking_query_data *>(event)),
          event_class,
          static_cast<const mysql_event_tracking_query_data *>(event),
          {}}};
    }
    case audit_event_class_t::AUDIT_STORED_PROGRAM_CLASS: {
      return AuditRecordVariant{AuditRecordStoredProgram{
          event_class_to_string(event_class),
          event_subclass_to_string(
              static_cast<const mysql_event_tracking_stored_program_data *>(
                  event)),
          event_class,
          static_cast<const mysql_event_tracking_stored_program_data *>(event),
          {}}};
    }
    case audit_event_class_t::AUDIT_AUTHENTICATION_CLASS: {
      return AuditRecordVariant{AuditRecordAuthentication{
          event_class_to_string(event_class),
          event_subclass_to_string(
              static_cast<const mysql_event_tracking_authentication_data *>(
                  event)),
          event_class,
          static_cast<const mysql_event_tracking_authentication_data *>(event),
          {}}};
    }
    case audit_event_class_t::AUDIT_MESSAGE_CLASS: {
      return AuditRecordVariant{AuditRecordMessage{
          event_class_to_string(event_class),
          event_subclass_to_string(
              static_cast<const mysql_event_tracking_message_data *>(event)),
          event_class,
          static_cast<const mysql_event_tracking_message_data *>(event),
          {}}};
    }
    case audit_event_class_t::AUDIT_PARSE_CLASS: {
      return AuditRecordVariant{AuditRecordParse{
          event_class_to_string(event_class),
          event_subclass_to_string(
              static_cast<const mysql_event_tracking_parse_data *>(event)),
          event_class,
          static_cast<const mysql_event_tracking_parse_data *>(event),
          {}}};
    }
    case audit_event_class_t::AUDIT_INTERNAL_AUDIT_CLASS: {
      return AuditRecordVariant{AuditRecordAudit{
          kClassNameInternalAudit,
          event_subclass_to_string(
              static_cast<const internal_event_tracking_audit_data *>(event)),
          audit_event_class_t::AUDIT_INTERNAL_AUDIT_CLASS,
          static_cast<const internal_event_tracking_audit_data *>(event),
          {}}};
    }
    default:
      break;
  }

  assert(false);

  return AuditRecordVariant{
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

bool is_valid_connection_type_value(std::string_view value) {
  return value.size() == 1 && value[0] >= '0' && value[0] <= '5';
}

AuditRecordFieldsList get_audit_record_fields(
    const AuditRecordGeneral &record) {
  const auto *event = record.event;
  const auto &extra = record.extended_info;

  return {
      {"general_error_code", static_cast<int64_t>(event->error_code)},
      // alias for general_connection_id
      {"general_thread_id", static_cast<uint64_t>(event->connection_id)},
      {"general_connection_id", static_cast<uint64_t>(event->connection_id)},
      {"general_user.str", extra.user},
      {"general_user.length", static_cast<uint64_t>(extra.user.length())},
      {"general_command.str", extra.command},
      {"general_command.length", static_cast<uint64_t>(extra.command.length())},
      {"general_query.str", extra.query},
      {"general_query.length", static_cast<uint64_t>(extra.query.length())},
      {"general_host.str", extra.host},
      {"general_host.length", static_cast<uint64_t>(extra.host.length())},
      {"general_sql_command.str", extra.sql_command},
      {"general_sql_command.length",
       static_cast<uint64_t>(extra.sql_command.length())},
      {"general_external_user.str", extra.external_user},
      {"general_external_user.length",
       static_cast<uint64_t>(extra.external_user.length())},
      {"general_ip.str", extra.ip},
      {"general_ip.length", static_cast<uint64_t>(extra.ip.length())},
  };
}

AuditRecordFieldsList get_audit_record_fields(
    const AuditRecordConnection &record) {
  const auto *event = record.event;
  return {
      {"status", static_cast<int64_t>(event->status)},
      {"connection_id", static_cast<uint64_t>(event->connection_id)},
      {"user.str", mysql_cstring_to_string(&event->user)},
      {"user.length", mysql_cstring_len_to_uint64(&event->user)},
      {"priv_user.str", mysql_cstring_to_string(&event->priv_user)},
      {"priv_user.length", mysql_cstring_len_to_uint64(&event->priv_user)},
      {"external_user.str", mysql_cstring_to_string(&event->external_user)},
      {"external_user.length",
       mysql_cstring_len_to_uint64(&event->external_user)},
      {"proxy_user.str", mysql_cstring_to_string(&event->proxy_user)},
      {"proxy_user.length", mysql_cstring_len_to_uint64(&event->proxy_user)},
      {"host.str", mysql_cstring_to_string(&event->host)},
      {"host.length", mysql_cstring_len_to_uint64(&event->host)},
      {"ip.str", mysql_cstring_to_string(&event->ip)},
      {"ip.length", mysql_cstring_len_to_uint64(&event->ip)},
      {"database.str", mysql_cstring_to_string(&event->database)},
      {"database.length", mysql_cstring_len_to_uint64(&event->database)},
      {"connection_type", static_cast<int64_t>(event->connection_type)},
  };
}

AuditRecordFieldsList get_audit_record_fields(
    const AuditRecordTableAccess &record) {
  const auto *event = record.event;
  const auto &extra = record.extended_info;

  return {
      {"connection_id", static_cast<uint64_t>(event->connection_id)},
      {"sql_command_id", static_cast<int64_t>(extra.sql_command_id)},
      {"query.str", extra.query},
      {"query.length", static_cast<uint64_t>(extra.query.length())},
      {"table_database.str", mysql_cstring_to_string(&event->table_database)},
      {"table_database.length",
       mysql_cstring_len_to_uint64(&event->table_database)},
      {"table_name.str", mysql_cstring_to_string(&event->table_name)},
      {"table_name.length", mysql_cstring_len_to_uint64(&event->table_name)},
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

bool is_event_class_allowed_in_reduced_mode(std::string_view class_name) {
  static constexpr std::array<std::string_view, 4> allowed_classes{{
      kClassNameGeneral,
      kClassNameConnection,
      kClassNameTableAccess,
      kClassNameMessage,
  }};
  return contains_string_view(class_name, allowed_classes);
}

bool is_event_subclass_allowed_in_reduced_mode(std::string_view class_name,
                                               std::string_view subclass_name) {
  if (class_name == kClassNameGeneral) {
    return subclass_name == kSubclassNameGeneralStatus;
  }
  if (class_name == kClassNameConnection) {
    static constexpr std::array<std::string_view, 3> allowed{{
        kSubclassNameConnect,
        kSubclassNameDisconnect,
        kSubclassNameChangeUser,
    }};
    return contains_string_view(subclass_name, allowed);
  }
  if (class_name == kClassNameTableAccess) {
    static constexpr std::array<std::string_view, 4> allowed{{
        kSubclassNameRead,
        kSubclassNameInsert,
        kSubclassNameUpdate,
        kSubclassNameDelete,
    }};
    return contains_string_view(subclass_name, allowed);
  }
  if (class_name == kClassNameMessage) {
    static constexpr std::array<std::string_view, 2> allowed{{
        kSubclassNameMessageInternal,
        kSubclassNameUser,
    }};
    return contains_string_view(subclass_name, allowed);
  }
  return false;
}

bool is_valid_event_class_name(std::string_view class_name) {
  // Filter definitions intentionally accept only the supported subset of
  // class names. That excludes authorization, internal audit lifecycle,
  // and server startup/shutdown lifecycle classes.
  static constexpr std::array<std::string_view, 10> valid_classes{{
      kClassNameGeneral,
      kClassNameConnection,
      kClassNameTableAccess,
      kClassNameGlobalVariable,
      kClassNameCommand,
      kClassNameQuery,
      kClassNameStoredProgram,
      kClassNameAuthentication,
      kClassNameMessage,
      kClassNameParse,
  }};
  return contains_string_view(class_name, valid_classes);
}

bool is_valid_event_subclass_name(std::string_view class_name,
                                  std::string_view subclass_name) {
  if (class_name == kClassNameGeneral) {
    static constexpr std::array<std::string_view, 4> valid_subclasses{{
        kSubclassNameGeneralLog,
        kSubclassNameGeneralError,
        kSubclassNameGeneralResult,
        kSubclassNameGeneralStatus,
    }};
    return contains_string_view(subclass_name, valid_subclasses);
  }

  if (class_name == kClassNameConnection) {
    static constexpr std::array<std::string_view, 4> valid_subclasses{{
        kSubclassNameConnect,
        kSubclassNameDisconnect,
        kSubclassNameChangeUser,
        kSubclassNamePreAuthenticate,
    }};
    return contains_string_view(subclass_name, valid_subclasses);
  }

  if (class_name == kClassNameTableAccess) {
    static constexpr std::array<std::string_view, 4> valid_subclasses{{
        kSubclassNameRead,
        kSubclassNameInsert,
        kSubclassNameUpdate,
        kSubclassNameDelete,
    }};
    return contains_string_view(subclass_name, valid_subclasses);
  }

  if (class_name == kClassNameGlobalVariable) {
    static constexpr std::array<std::string_view, 2> valid_subclasses{{
        kSubclassNameGet,
        kSubclassNameSet,
    }};
    return contains_string_view(subclass_name, valid_subclasses);
  }

  if (class_name == kClassNameCommand) {
    static constexpr std::array<std::string_view, 2> valid_subclasses{{
        kSubclassNameStart,
        kSubclassNameEnd,
    }};
    return contains_string_view(subclass_name, valid_subclasses);
  }

  if (class_name == kClassNameQuery) {
    static constexpr std::array<std::string_view, 4> valid_subclasses{{
        kSubclassNameStart,
        kSubclassNameNestedStart,
        kSubclassNameStatusEnd,
        kSubclassNameNestedStatusEnd,
    }};
    return contains_string_view(subclass_name, valid_subclasses);
  }

  if (class_name == kClassNameStoredProgram) {
    static constexpr std::array<std::string_view, 1> valid_subclasses{{
        kSubclassNameExecute,
    }};
    return contains_string_view(subclass_name, valid_subclasses);
  }

  if (class_name == kClassNameAuthentication) {
    static constexpr std::array<std::string_view, 5> valid_subclasses{{
        kSubclassNameFlush,
        kSubclassNameAuthidCreate,
        kSubclassNameCredentialChange,
        kSubclassNameAuthidRename,
        kSubclassNameAuthidDrop,
    }};
    return contains_string_view(subclass_name, valid_subclasses);
  }

  if (class_name == kClassNameMessage) {
    static constexpr std::array<std::string_view, 2> valid_subclasses{{
        kSubclassNameMessageInternal,
        kSubclassNameUser,
    }};
    return contains_string_view(subclass_name, valid_subclasses);
  }

  if (class_name == kClassNameParse) {
    static constexpr std::array<std::string_view, 2> valid_subclasses{{
        kSubclassNameParsePreparse,
        kSubclassNameParsePostparse,
    }};
    return contains_string_view(subclass_name, valid_subclasses);
  }

  return false;
}

bool is_valid_event_field_name(std::string_view event_class_name,
                               std::string_view field_name) {
  using FieldVec = std::vector<std::string_view>;
  static const std::unordered_map<std::string_view, FieldVec> valid_fields_map{
      {kClassNameGeneral,
       {"general_error_code", "general_thread_id", "general_connection_id",
        "general_user.str", "general_user.length", "general_command.str",
        "general_command.length", "general_query.str", "general_query.length",
        "general_host.str", "general_host.length", "general_sql_command.str",
        "general_sql_command.length", "general_external_user.str",
        "general_external_user.length", "general_ip.str", "general_ip.length"}},
      {kClassNameConnection,
       {"status", "connection_id", "user.str", "user.length", "priv_user.str",
        "priv_user.length", "external_user.str", "external_user.length",
        "proxy_user.str", "proxy_user.length", "host.str", "host.length",
        "ip.str", "ip.length", "database.str", "database.length",
        "connection_type"}},
      {kClassNameTableAccess,
       {"connection_id", "sql_command_id", "query.str", "query.length",
        "table_database.str", "table_database.length", "table_name.str",
        "table_name.length"}},
      {kClassNameGlobalVariable,
       {"connection_id", "variable_name.str", "variable_name.length",
        "variable_value.str", "variable_value.length"}},
      {kClassNameCommand,
       {"status", "connection_id", "command.str", "command.length"}},
      {kClassNameQuery,
       {"status", "connection_id", "sql_command_id", "query.str",
        "query.length", "query_charset"}},
      {kClassNameStoredProgram,
       {"connection_id", "database.str", "database.length", "name.str",
        "name.length"}},
      {kClassNameAuthentication,
       {"status", "connection_id", "user.str", "user.length", "host.str",
        "host.length"}},
      {kClassNameMessage,
       {"connection_id", "component.str", "component.length", "producer.str",
        "producer.length", "message.str", "message.length"}},
      {kClassNameParse,
       {"connection_id", "flags", "query.str", "query.length",
        "rewritten_query.str", "rewritten_query.length"}},
  };

  const auto class_it = valid_fields_map.find(event_class_name);
  if (class_it == valid_fields_map.cend()) {
    return false;
  }

  return contains_string_view(field_name, class_it->second);
}

EventFieldValueType get_event_field_value_type(
    std::string_view event_class_name, std::string_view field_name) {
  using FieldTypeMap = std::map<std::string_view, EventFieldValueType>;
  static const std::unordered_map<std::string_view, FieldTypeMap>
      field_type_map{
          {kClassNameConnection,
           {{"status", EventFieldValueType::SignedInteger},
            {"connection_id", EventFieldValueType::UnsignedInteger},
            {"user.length", EventFieldValueType::UnsignedInteger},
            {"priv_user.length", EventFieldValueType::UnsignedInteger},
            {"external_user.length", EventFieldValueType::UnsignedInteger},
            {"proxy_user.length", EventFieldValueType::UnsignedInteger},
            {"host.length", EventFieldValueType::UnsignedInteger},
            {"ip.length", EventFieldValueType::UnsignedInteger},
            {"database.length", EventFieldValueType::UnsignedInteger},
            {"connection_type", EventFieldValueType::SignedInteger}}},
          {kClassNameGeneral,
           {{"general_error_code", EventFieldValueType::SignedInteger},
            {"general_thread_id", EventFieldValueType::UnsignedInteger},
            {"general_connection_id", EventFieldValueType::UnsignedInteger},
            {"general_user.length", EventFieldValueType::UnsignedInteger},
            {"general_command.length", EventFieldValueType::UnsignedInteger},
            {"general_query.length", EventFieldValueType::UnsignedInteger},
            {"general_host.length", EventFieldValueType::UnsignedInteger},
            {"general_sql_command.length",
             EventFieldValueType::UnsignedInteger},
            {"general_external_user.length",
             EventFieldValueType::UnsignedInteger},
            {"general_ip.length", EventFieldValueType::UnsignedInteger}}},
          {kClassNameTableAccess,
           {{"connection_id", EventFieldValueType::UnsignedInteger},
            {"sql_command_id", EventFieldValueType::SignedInteger},
            {"query.length", EventFieldValueType::UnsignedInteger},
            {"table_database.length", EventFieldValueType::UnsignedInteger},
            {"table_name.length", EventFieldValueType::UnsignedInteger}}},
      };

  const auto class_it = field_type_map.find(event_class_name);
  if (class_it == field_type_map.cend()) {
    return EventFieldValueType::String;
  }

  const auto field_it = class_it->second.find(field_name);
  if (field_it == class_it->second.cend()) {
    return EventFieldValueType::String;
  }

  return field_it->second;
}

}  // namespace audit_log_filter
