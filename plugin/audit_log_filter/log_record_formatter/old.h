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

#ifndef AUDIT_LOG_FILTER_LOG_RECORD_FORMATTER_OLD_H_INCLUDED
#define AUDIT_LOG_FILTER_LOG_RECORD_FORMATTER_OLD_H_INCLUDED

#include "base.h"

namespace audit_log_filter::log_record_formatter {

template <>
class LogRecordFormatter<AuditLogFormatType::Old>
    : public LogRecordFormatterBaseXml {
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

 private:
  /**
   * @brief Get string representation of extra attributes
   *        for audit log record.
   * @param info Extended record info
   * @return Formatted string
   */
  [[nodiscard]] std::string extra_attrs_to_string(
      const ExtendedInfo &info) const noexcept override;
};

using LogRecordFormatterOld = LogRecordFormatter<AuditLogFormatType::Old>;

}  // namespace audit_log_filter::log_record_formatter

#endif  // AUDIT_LOG_FILTER_LOG_RECORD_FORMATTER_OLD_H_INCLUDED
