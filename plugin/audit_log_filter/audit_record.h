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

#include "mysql/plugin_audit.h"

#include <string_view>
#include <variant>

namespace audit_log_filter {

struct AuditRecordGeneral {
  std::string_view event_class_name;
  std::string_view event_subclass_name;
  mysql_event_class_t event_class;
  const mysql_event_general *event;
};

struct AuditRecordConnection {
  std::string_view event_class_name;
  std::string_view event_subclass_name;
  mysql_event_class_t event_class;
  const mysql_event_connection *event;
};

struct AuditRecordParse {
  std::string_view event_class_name;
  std::string_view event_subclass_name;
  mysql_event_class_t event_class;
  const mysql_event_parse *event;
};

struct AuditRecordTableAccess {
  std::string_view event_class_name;
  std::string_view event_subclass_name;
  mysql_event_class_t event_class;
  const mysql_event_table_access *event;
};

struct AuditRecordGlobalVariable {
  std::string_view event_class_name;
  std::string_view event_subclass_name;
  mysql_event_class_t event_class;
  const mysql_event_global_variable *event;
};

struct AuditRecordServerStartup {
  std::string_view event_class_name;
  std::string_view event_subclass_name;
  mysql_event_class_t event_class;
  const mysql_event_server_startup *event;
};

struct AuditRecordServerShutdown {
  std::string_view event_class_name;
  std::string_view event_subclass_name;
  mysql_event_class_t event_class;
  const mysql_event_server_shutdown *event;
};

struct AuditRecordCommand {
  std::string_view event_class_name;
  std::string_view event_subclass_name;
  mysql_event_class_t event_class;
  const mysql_event_command *event;
};

struct AuditRecordQuery {
  std::string_view event_class_name;
  std::string_view event_subclass_name;
  mysql_event_class_t event_class;
  const mysql_event_query *event;
};

struct AuditRecordStoredProgram {
  std::string_view event_class_name;
  std::string_view event_subclass_name;
  mysql_event_class_t event_class;
  const mysql_event_stored_program *event;
};

struct AuditRecordAuthentication {
  std::string_view event_class_name;
  std::string_view event_subclass_name;
  mysql_event_class_t event_class;
  const mysql_event_authentication *event;
};

struct AuditRecordMessage {
  std::string_view event_class_name;
  std::string_view event_subclass_name;
  mysql_event_class_t event_class;
  const mysql_event_message *event;
};

struct AuditRecordUnknown {
  std::string_view event_class_name;
  std::string_view event_subclass_name;
  mysql_event_class_t event_class;
  const void *event;
};

using AuditRecordVariant =
    std::variant<AuditRecordGeneral, AuditRecordConnection, AuditRecordParse,
                 AuditRecordTableAccess, AuditRecordGlobalVariable,
                 AuditRecordServerStartup, AuditRecordServerShutdown,
                 AuditRecordCommand, AuditRecordQuery, AuditRecordStoredProgram,
                 AuditRecordAuthentication, AuditRecordMessage,
                 AuditRecordUnknown>;

/**
 * @brief Get AuditRecordVariant instance representing received audit event.
 *
 * @param event_class Received audit event class
 * @param event Received audit event
 * @return An instance of AuditRecordVariant representing audit event
 */
AuditRecordVariant get_audit_record(mysql_event_class_t event_class,
                                    const void *event);

}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_RECORD_H_INCLUDED
