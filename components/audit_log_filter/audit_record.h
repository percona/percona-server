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

#include <map>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

struct mysql_event_tracking_general_data;
struct mysql_event_tracking_connection_data;
struct mysql_event_tracking_table_access_data;
struct mysql_event_tracking_global_variable_data;
struct mysql_event_tracking_startup_data;
struct mysql_event_tracking_shutdown_data;
struct mysql_event_tracking_command_data;
struct mysql_event_tracking_query_data;
struct mysql_event_tracking_stored_program_data;
struct mysql_event_tracking_authentication_data;
struct mysql_event_tracking_message_data;
struct mysql_event_tracking_parse_data;

namespace audit_log_filter {

using AuditRecordFieldsList = std::map<std::string, std::string>;

constexpr std::string_view CONNECTION_TYPE_FIELD_NAME = "connection_type";

struct ExtendedInfo {
  std::string digest;
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

struct AuditRecordServerStartup {
  std::string_view event_class_name;
  std::string_view event_subclass_name;
  audit_event_class_t event_class;
  const mysql_event_tracking_startup_data *event;
  ExtendedInfo extended_info;
};

struct AuditRecordServerShutdown {
  std::string_view event_class_name;
  std::string_view event_subclass_name;
  audit_event_class_t event_class;
  const mysql_event_tracking_shutdown_data *event;
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
                 AuditRecordServerStartup, AuditRecordServerShutdown,
                 AuditRecordCommand, AuditRecordQuery, AuditRecordStoredProgram,
                 AuditRecordAuthentication, AuditRecordMessage,
                 AuditRecordParse, AuditRecordAudit, AuditRecordUnknown>;

/**
 * @brief Get AuditRecordVariant instance representing received audit event.
 *
 * @param event_class Received audit event class
 * @param event Received audit event
 * @return An instance of AuditRecordVariant representing audit event
 */
AuditRecordVariant get_audit_record(audit_event_class_t event_class,
                                    const void *event);

/**
 * @brief Convert connection_type pseudo-constant to numeric value.
 *
 * @param type Connection type
 */
void update_connection_type_pseudo_to_numeric(std::string &type);

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
 * @brief Get fields list from AuditRecordServerStartup event record.
 *
 * @param record Audit event record
 * @return Fields list, @ref AuditRecordFieldsList
 */
AuditRecordFieldsList get_audit_record_fields(
    const AuditRecordServerStartup &record);

/**
 * @brief Get fields list from AuditRecordServerShutdown event record.
 *
 * @param record Audit event record
 * @return Fields list, @ref AuditRecordFieldsList
 */
AuditRecordFieldsList get_audit_record_fields(
    const AuditRecordServerShutdown &record);

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

}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_RECORD_H_INCLUDED
