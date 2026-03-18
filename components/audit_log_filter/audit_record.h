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

#ifndef AUDIT_LOG_FILTER_RECORD_H_INCLUDED
#define AUDIT_LOG_FILTER_RECORD_H_INCLUDED

#include "components/audit_log_filter/audit_event_class_internal.h"
#include "my_sqlcommand.h"  // enum_sql_command

#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

struct mysql_event_tracking_general_data;
struct mysql_event_tracking_connection_data;
struct mysql_event_tracking_table_access_data;
struct mysql_event_tracking_global_variable_data;
struct mysql_event_tracking_command_data;
struct mysql_event_tracking_query_data;
struct mysql_event_tracking_stored_program_data;
struct mysql_event_tracking_authentication_data;
struct mysql_event_tracking_message_data;
struct mysql_event_tracking_parse_data;

namespace audit_log_filter {

using AuditRecordFieldValue = std::variant<std::string, int64_t, uint64_t>;
using AuditRecordFieldsList = std::map<std::string, AuditRecordFieldValue>;

enum class EventFieldValueType { String, SignedInteger, UnsignedInteger };

constexpr std::string_view CONNECTION_TYPE_FIELD_NAME = "connection_type";

struct ExtendedInfo {
  std::string digest;
  std::string user;
  std::string host;
  std::string ip;
  std::string external_user;
  std::string proxy_user;
  std::string command;
  std::string sql_command;
  enum_sql_command sql_command_id;
  std::string query;
  std::map<std::string, std::vector<std::pair<std::string, std::string>>> attrs;
};

struct AuditRecordGeneral {
  std::string_view event_class_name;
  std::string_view event_subclass_name;
  audit_event_class_t event_class;
  const mysql_event_tracking_general_data *event;
  ExtendedInfo extended_info;
};

struct AuditRecordConnection {
  std::string_view event_class_name;
  std::string_view event_subclass_name;
  audit_event_class_t event_class;
  const mysql_event_tracking_connection_data *event;
  ExtendedInfo extended_info;
};

struct AuditRecordTableAccess {
  std::string_view event_class_name;
  std::string_view event_subclass_name;
  audit_event_class_t event_class;
  const mysql_event_tracking_table_access_data *event;
  ExtendedInfo extended_info;
};

struct AuditRecordGlobalVariable {
  std::string_view event_class_name;
  std::string_view event_subclass_name;
  audit_event_class_t event_class;
  const mysql_event_tracking_global_variable_data *event;
  ExtendedInfo extended_info;
};

struct AuditRecordCommand {
  std::string_view event_class_name;
  std::string_view event_subclass_name;
  audit_event_class_t event_class;
  const mysql_event_tracking_command_data *event;
  ExtendedInfo extended_info;
};

struct AuditRecordQuery {
  std::string_view event_class_name;
  std::string_view event_subclass_name;
  audit_event_class_t event_class;
  const mysql_event_tracking_query_data *event;
  ExtendedInfo extended_info;
};

struct AuditRecordStoredProgram {
  std::string_view event_class_name;
  std::string_view event_subclass_name;
  audit_event_class_t event_class;
  const mysql_event_tracking_stored_program_data *event;
  ExtendedInfo extended_info;
};

struct AuditRecordAuthentication {
  std::string_view event_class_name;
  std::string_view event_subclass_name;
  audit_event_class_t event_class;
  const mysql_event_tracking_authentication_data *event;
  ExtendedInfo extended_info;
};

struct AuditRecordMessage {
  std::string_view event_class_name;
  std::string_view event_subclass_name;
  audit_event_class_t event_class;
  const mysql_event_tracking_message_data *event;
  ExtendedInfo extended_info;
};

struct AuditRecordParse {
  std::string_view event_class_name;
  std::string_view event_subclass_name;
  audit_event_class_t event_class;
  const mysql_event_tracking_parse_data *event;
  ExtendedInfo extended_info;
};

struct AuditRecordAudit {
  std::string_view event_class_name;
  std::string_view event_subclass_name;
  audit_event_class_t event_class;
  const internal_event_tracking_audit_data *event;
  ExtendedInfo extended_info;
};

struct AuditRecordUnknown {
  std::string_view event_class_name;
  std::string_view event_subclass_name;
  audit_event_class_t event_class;
  const void *event;
  ExtendedInfo extended_info;
};

using AuditRecordVariant =
    std::variant<AuditRecordGeneral, AuditRecordConnection,
                 AuditRecordTableAccess, AuditRecordGlobalVariable,
                 AuditRecordCommand, AuditRecordQuery, AuditRecordStoredProgram,
                 AuditRecordAuthentication, AuditRecordMessage,
                 AuditRecordParse, AuditRecordAudit, AuditRecordUnknown>;

/**
 * @brief Get AuditRecordVariant instance representing received audit event.
 *
 * @param event_class Received audit event class
 * @param event Received audit event
 * @return An instance of AuditRecordVariant representing audit event,
 *         or std::nullopt when the event should be ignored
 */
std::optional<AuditRecordVariant> get_audit_record(
    audit_event_class_t event_class, const void *event);

/**
 * @brief Convert connection_type pseudo-constant to numeric value.
 *
 * @param type Connection type
 */
void update_connection_type_pseudo_to_numeric(std::string &type);

/**
 * @brief Check if connection_type value is valid.
 *
 * Valid values are "0" through "5" (after pseudo-to-numeric conversion).
 *
 * @param value Connection type value to validate
 * @return true if the value is a valid connection type, false otherwise
 */
bool is_valid_connection_type_value(std::string_view value);

/**
 * @brief Get fields list from AuditRecordGeneral event record.
 *
 * @param record Audit event record
 * @return Fields list, @ref AuditRecordFieldsList
 */
AuditRecordFieldsList get_audit_record_fields(const AuditRecordGeneral &record);

/**
 * @brief Get fields list from AuditRecordConnection event record.
 *
 * @param record Audit event record
 * @return Fields list, @ref AuditRecordFieldsList
 */
AuditRecordFieldsList get_audit_record_fields(
    const AuditRecordConnection &record);

/**
 * @brief Get fields list from AuditRecordTableAccess event record.
 *
 * @param record Audit event record
 * @return Fields list, @ref AuditRecordFieldsList
 */
AuditRecordFieldsList get_audit_record_fields(
    const AuditRecordTableAccess &record);

/**
 * @brief Get fields list from AuditRecordGlobalVariable event record.
 *
 * @param record Audit event record
 * @return Fields list, @ref AuditRecordFieldsList
 */
AuditRecordFieldsList get_audit_record_fields(
    const AuditRecordGlobalVariable &record);

/**
 * @brief Get fields list from AuditRecordCommand event record.
 *
 * @param record Audit event record
 * @return Fields list, @ref AuditRecordFieldsList
 */
AuditRecordFieldsList get_audit_record_fields(const AuditRecordCommand &record);

/**
 * @brief Get fields list from AuditRecordQuery event record.
 *
 * @param record Audit event record
 * @return Fields list, @ref AuditRecordFieldsList
 */
AuditRecordFieldsList get_audit_record_fields(const AuditRecordQuery &record);

/**
 * @brief Get fields list from AuditRecordStoredProgram event record.
 *
 * @param record Audit event record
 * @return Fields list, @ref AuditRecordFieldsList
 */
AuditRecordFieldsList get_audit_record_fields(
    const AuditRecordStoredProgram &record);

/**
 * @brief Get fields list from AuditRecordAuthentication event record.
 *
 * @param record Audit event record
 * @return Fields list, @ref AuditRecordFieldsList
 */
AuditRecordFieldsList get_audit_record_fields(
    const AuditRecordAuthentication &record);

/**
 * @brief Get fields list from AuditRecordMessage event record.
 *
 * @param record Audit event record
 * @return Fields list, @ref AuditRecordFieldsList
 */
AuditRecordFieldsList get_audit_record_fields(const AuditRecordMessage &record);

/**
 * @brief Get fields list from AuditRecordParse event record.
 *
 * @param record Audit event record
 * @return Fields list, @ref AuditRecordFieldsList
 */
AuditRecordFieldsList get_audit_record_fields(const AuditRecordParse &record);

/**
 * @brief Get fields list from AuditRecordAudit event record.
 *
 * @param record Audit event record
 * @return Fields list, @ref AuditRecordFieldsList
 */
AuditRecordFieldsList get_audit_record_fields(const AuditRecordAudit &record);

/**
 * @brief Get fields list from AuditRecordUnknown event record.
 *
 * @param record Audit event record
 * @return Fields list, @ref AuditRecordFieldsList
 */
AuditRecordFieldsList get_audit_record_fields(const AuditRecordUnknown &record);

/**
 * @brief Convert an AuditRecordFieldValue to its string representation.
 */
std::string field_value_to_string(const AuditRecordFieldValue &value);

/**
 * @brief Compare an AuditRecordFieldValue against a string expected value.
 *
 * For string alternatives the comparison is direct. For integer alternatives
 * the expected string is parsed to the matching integer type first; returns
 * false on parse failure.
 */
bool field_value_matches(const AuditRecordFieldValue &value,
                         const std::string &expected);

/**
 * @brief Check if an event class has at least one subclass allowed in
 *        REDUCED event mode.
 *
 * @param class_name Event class name to validate (e.g. "connection")
 * @return true if the class has any allowed subclass in REDUCED mode,
 *         false otherwise
 */
bool is_event_class_allowed_in_reduced_mode(std::string_view class_name);

/**
 * @brief Check if a specific event subclass is allowed in REDUCED event mode.
 *
 * @param class_name Event class name (e.g. "connection")
 * @param subclass_name Event subclass name (e.g. "connect")
 * @return true if the event is allowed in REDUCED mode, false otherwise
 */
bool is_event_subclass_allowed_in_reduced_mode(std::string_view class_name,
                                               std::string_view subclass_name);

/**
 * @brief Check if an event class name is supported by filter definitions.
 *
 * @param class_name Event class name to validate (e.g. "connection")
 * @return true if the class name is accepted by filter-rule validation,
 *         false otherwise
 */
bool is_valid_event_class_name(std::string_view class_name);

/**
 * @brief Check if an event subclass name is valid for a filterable class.
 *
 * @param class_name Event class name (e.g. "connection")
 * @param subclass_name Event subclass name to validate (e.g. "connect")
 * @return true if the subclass name is recognized for the class by
 *         filter-rule validation,
 *         false otherwise
 */
bool is_valid_event_subclass_name(std::string_view class_name,
                                  std::string_view subclass_name);

/**
 * @brief Check if a field name is valid for a filterable event class.
 *
 * @param event_class_name Audit event class name (e.g. "table_access")
 * @param field_name Field name to validate (e.g. "table_name.str")
 * @return true if the field name is recognized for the event class by
 *         filter-rule validation,
 *         false otherwise
 */
bool is_valid_event_field_name(std::string_view event_class_name,
                               std::string_view field_name);

/**
 * @brief Get the expected value type for a field in a filterable event class.
 *
 * @param event_class_name Audit event class name (e.g. "table_access")
 * @param field_name Field name to check (e.g. "connection_id")
 * @return EventFieldValueType indicating String, SignedInteger, or
 *         UnsignedInteger
 */
EventFieldValueType get_event_field_value_type(
    std::string_view event_class_name, std::string_view field_name);

}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_RECORD_H_INCLUDED
