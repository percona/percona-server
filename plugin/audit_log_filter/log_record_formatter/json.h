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

#ifndef AUDIT_LOG_FILTER_LOG_RECORD_FORMATTER_JSON_H_INCLUDED
#define AUDIT_LOG_FILTER_LOG_RECORD_FORMATTER_JSON_H_INCLUDED

#include "base.h"

#include "plugin/audit_log_filter/audit_record.h"

#include <string_view>

namespace audit_log_filter::log_record_formatter {

template <>
class LogRecordFormatter<AuditLogFormatType::Json>
    : public LogRecordFormatterBase {
 public:
  /**
   * @brief Apply formatting to AuditRecordGeneral audit record.
   *
   * @param [in] audit_record Audit record
   * @return String representing formatted audit record
   */
  [[nodiscard]] AuditRecordString apply(
      const AuditRecordGeneral &audit_record) const noexcept override;

  /**
   * @brief Apply formatting to AuditRecordConnection audit record.
   *
   * @param [in] audit_record Audit record
   * @return String representing formatted audit record
   */
  [[nodiscard]] AuditRecordString apply(
      const AuditRecordConnection &audit_record) const noexcept override;

  /**
   * @brief Apply formatting to AuditRecordTableAccess audit record.
   *
   * @param [in] audit_record Audit record
   * @return String representing formatted audit record
   */
  [[nodiscard]] AuditRecordString apply(
      const AuditRecordTableAccess &audit_record) const noexcept override;

  /**
   * @brief Apply formatting to AuditRecordGlobalVariable audit record.
   *
   * @param [in] audit_record Audit record
   * @return String representing formatted audit record
   */
  [[nodiscard]] AuditRecordString apply(
      const AuditRecordGlobalVariable &audit_record) const noexcept override;

  /**
   * @brief Apply formatting to AuditRecordServerStartup audit record.
   *
   * @param [in] audit_record Audit record
   * @return String representing formatted audit record
   */
  [[nodiscard]] AuditRecordString apply(
      const AuditRecordServerStartup &audit_record) const noexcept override;

  /**
   * @brief Apply formatting to AuditRecordServerShutdown audit record.
   *
   * @param [in] audit_record Audit record
   * @return String representing formatted audit record
   */
  [[nodiscard]] AuditRecordString apply(
      const AuditRecordServerShutdown &audit_record) const noexcept override;

  /**
   * @brief Apply formatting to AuditRecordCommand audit record.
   *
   * @param [in] audit_record Audit record
   * @return String representing formatted audit record
   */
  [[nodiscard]] AuditRecordString apply(
      const AuditRecordCommand &audit_record) const noexcept override;

  /**
   * @brief Apply formatting to AuditRecordQuery audit record.
   *
   * @param [in] audit_record Audit record
   * @return String representing formatted audit record
   */
  [[nodiscard]] AuditRecordString apply(
      const AuditRecordQuery &audit_record) const noexcept override;

  /**
   * @brief Apply formatting to AuditRecordStoredProgram audit record.
   *
   * @param [in] audit_record Audit record
   * @return String representing formatted audit record
   */
  [[nodiscard]] AuditRecordString apply(
      const AuditRecordStoredProgram &audit_record) const noexcept override;

  /**
   * @brief Apply formatting to AuditRecordAuthentication audit record.
   *
   * @param [in] audit_record Audit record
   * @return String representing formatted audit record
   */
  [[nodiscard]] AuditRecordString apply(
      const AuditRecordAuthentication &audit_record) const noexcept override;

  /**
   * @brief Apply formatting to AuditRecordMessage audit record.
   *
   * @param [in] audit_record Audit record
   * @return String representing formatted audit record
   */
  [[nodiscard]] AuditRecordString apply(
      const AuditRecordMessage &audit_record) const noexcept override;

  /**
   * @brief Apply formatting to AuditRecordStartAudit audit record.
   *
   * @param [in] audit_record Audit record
   * @return String representing formatted audit record
   */
  [[nodiscard]] AuditRecordString apply(
      const AuditRecordStartAudit &audit_record) const noexcept override;

  /**
   * @brief Apply formatting to AuditRecordStopAudit audit record.
   *
   * @param [in] audit_record Audit record
   * @return String representing formatted audit record
   */
  [[nodiscard]] AuditRecordString apply(
      const AuditRecordStopAudit &audit_record) const noexcept override;

  /**
   * @brief Get log file header string.
   *
   * @return Log file header string
   */
  [[nodiscard]] std::string get_file_header() const noexcept override;

  /**
   * @brief Get log file footer string.
   *
   * @return Log file footer string
   */
  [[nodiscard]] std::string get_file_footer() const noexcept override;

  /**
   * @brief Get separator added between event records in a log file.
   *
   * @return Event resords separator string
   */
  [[nodiscard]] std::string get_record_separator() const noexcept override;

  /**
   * @brief Insert audit event class and subclass names into record printed to
   *        log. Needed for testing.
   *
   * @param event_class_name Event class name
   * @param event_subclass_name Event subclass name
   * @param record_str String representation of audit record
   */
  void apply_debug_info(std::string_view event_class_name,
                        std::string_view event_subclass_name,
                        std::string &record_str) noexcept override;

 protected:
  /**
   * @brief Get timestamp string representation.
   *
   * @param time_point Time point
   * @return Timestamp string
   */
  [[nodiscard]] std::string make_timestamp(
      std::chrono::system_clock::time_point time_point) const noexcept override;

 private:
  /**
   * @brief Get string representation of audit event subclass name.
   *
   * @param event_subclass Audit event subclass
   * @return String representation of audit event subclass name
   */
  [[nodiscard]] std::string_view event_subclass_to_string(
      mysql_event_general_subclass_t event_subclass) const noexcept override;

  /**
   * @brief Get string representation of audit event subclass name.
   *
   * @param event_subclass Audit event subclass
   * @return String representation of audit event subclass name
   */
  [[nodiscard]] std::string_view event_subclass_to_string(
      mysql_event_connection_subclass_t event_subclass) const noexcept override;

  /**
   * @brief Get string representation of audit event subclass name.
   *
   * @param event_subclass Audit event subclass
   * @return String representation of audit event subclass name
   */
  [[nodiscard]] std::string_view event_subclass_to_string(
      mysql_event_table_access_subclass_t event_subclass)
      const noexcept override;

  /**
   * @brief Get string representation of audit event subclass name.
   *
   * @param event_subclass Audit event subclass
   * @return String representation of audit event subclass name
   */
  [[nodiscard]] std::string_view event_subclass_to_string(
      mysql_event_global_variable_subclass_t event_subclass)
      const noexcept override;

  /**
   * @brief Get string representation of audit event subclass name.
   *
   * @param event_subclass Audit event subclass
   * @return String representation of audit event subclass name
   */
  [[nodiscard]] std::string_view event_subclass_to_string(
      mysql_event_command_subclass_t event_subclass) const noexcept override;

  /**
   * @brief Get string representation of audit event subclass name.
   *
   * @param event_subclass Audit event subclass
   * @return String representation of audit event subclass name
   */
  [[nodiscard]] std::string_view event_subclass_to_string(
      mysql_event_query_subclass_t event_subclass) const noexcept override;

  /**
   * @brief Get string representation of audit event subclass name.
   *
   * @param event_subclass Audit event subclass
   * @return String representation of audit event subclass name
   */
  [[nodiscard]] std::string_view event_subclass_to_string(
      mysql_event_authentication_subclass_t event_subclass)
      const noexcept override;

  /**
   * @brief Get string representation of audit event subclass name.
   *
   * @param event_subclass Audit event subclass
   * @return String representation of audit event subclass name
   */
  [[nodiscard]] std::string_view event_subclass_to_string(
      mysql_event_server_startup_subclass_t event_subclass)
      const noexcept override;

  /**
   * @brief Get string representation of audit event subclass name.
   *
   * @param event_subclass Audit event subclass
   * @return String representation of audit event subclass name
   */
  [[nodiscard]] std::string_view event_subclass_to_string(
      mysql_event_server_shutdown_subclass_t event_subclass)
      const noexcept override;

  /**
   * @brief Get string representation of audit event subclass name.
   *
   * @param event_subclass Audit event subclass
   * @return String representation of audit event subclass name
   */
  [[nodiscard]] std::string_view event_subclass_to_string(
      mysql_event_stored_program_subclass_t event_subclass)
      const noexcept override;

  /**
   * @brief Get string representation of audit event subclass name.
   *
   * @param event_subclass Audit event subclass
   * @return String representation of audit event subclass name
   */
  [[nodiscard]] std::string_view event_subclass_to_string(
      mysql_event_message_subclass_t event_subclass) const noexcept override;

  /**
   * @brief Get string representation of audit event subclass name.
   *
   * @param event_subclass Audit event subclass
   * @return String representation of audit event subclass name
   */
  [[nodiscard]] std::string_view event_subclass_to_string(
      audit_filter_event_subclass_t event_subclass) const noexcept override;

  /**
   * @brief Get string representation of connection type name.
   *
   * @param connection_type Connection type
   * @return String representation of connection type name
   */
  [[nodiscard]] std::string_view connection_type_name_to_string(
      int connection_type) const noexcept override;

  /**
   * @brief Get string representation of shutdown reason.
   *
   * @param reason Shutdown reason
   * @return String representation of shutdown reason
   */
  [[nodiscard]] std::string_view shutdown_reason_to_string(
      mysql_server_shutdown_reason_t reason) const noexcept override;

 private:
  /**
   * @brief Get escape rules.
   *
   * @return Escape rules
   */
  [[nodiscard]] const EscapeRulesContainer &get_escape_rules()
      const noexcept override;

  /**
   * @brief Get JSON string representation of extra attributes
   *        for audit log record.
   * @param info Extended record info
   * @return JSON formatted string
   */
  [[nodiscard]] std::string extra_attrs_to_string(
      const ExtendedInfo &info) const noexcept;
};

using LogRecordFormatterJson = LogRecordFormatter<AuditLogFormatType::Json>;

}  // namespace audit_log_filter::log_record_formatter

#endif  // AUDIT_LOG_FILTER_LOG_RECORD_FORMATTER_JSON_H_INCLUDED
