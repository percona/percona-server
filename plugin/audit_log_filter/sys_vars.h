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

#ifndef AUDIT_LOG_FILTER_SYS_VARS_H_INCLUDED
#define AUDIT_LOG_FILTER_SYS_VARS_H_INCLUDED

#include "plugin/audit_log_filter/component_registry_service.h"
#include "plugin/audit_log_filter/log_record_formatter/base.h"
#include "plugin/audit_log_filter/log_writer/base.h"

#include <memory>
#include <string>

namespace audit_log_filter {

struct AuditLogReaderContext;
class SysVars;

using log_record_formatter::AuditLogFormatType;
using log_writer::AuditLogCompressionType;
using log_writer::AuditLogEncryptionType;
using log_writer::AuditLogHandlerType;
using log_writer::AuditLogStrategyType;

struct LogBookmark {
  uint64_t id;
  std::string timestamp;
};

class SysVars {
 public:
  /**
   * @brief Get status variables list.
   *
   * @return Status variables list
   */
  [[nodiscard]] static SHOW_VAR *get_status_var_defs() noexcept;

  /**
   * @brief Get system variables list.
   *
   * @return System variables list
   */
  [[nodiscard]] static SYS_VAR **get_sys_var_defs() noexcept;

  /**
   * @brief Validate system variables settings.
   */
  static void validate() noexcept;

  /**
   * @brief Get log file directory name.
   *
   * @return Log file directory name
   */
  [[nodiscard]] static const std::string &get_file_dir() noexcept;

  /**
   * @brief Get log file base name.
   *
   * @return Audit log filter file base name
   */
  [[nodiscard]] static const std::string &get_file_name() noexcept;

  /**
   * @brief Get audit log filter handler type.
   *
   * @return Audit log filter handler type, may be one of possible values
   *         of AuditLogHandlerType
   */
  [[nodiscard]] static AuditLogHandlerType get_handler_type() noexcept;

  /**
   * @brief Get audit log filter format type.
   *
   * @return Audit log filter format type, may be one of possible values
   *         of AuditLogFormatType
   */
  [[nodiscard]] static AuditLogFormatType get_format_type() noexcept;

  /**
   * @brief Get audit log filter file logging strategy.
   *
   * @return Audit log filter file logging strategy, may be one of possible
   *         values of AuditLogStrategyType
   */
  [[nodiscard]] static AuditLogStrategyType get_file_strategy_type() noexcept;

  /**
   * @brief Get size of memory buffer used for logging in bytes.
   *
   * @return Size of memory buffer used for logging in bytes
   */
  [[nodiscard]] static ulonglong get_buffer_size() noexcept;

  /**
   * @brief Get log reader buffer size.
   *
   * @param thd Connection specific THD instance
   * @return Log reader buffer size in bytes
   */
  [[nodiscard]] static ulong get_read_buffer_size(MYSQL_THD thd) noexcept;

  /**
   * @brief Get the maximum size of the audit filter log file in bytes.
   *
   * @return Maximum size of the audit filter log file in bytes
   */
  [[nodiscard]] static ulonglong get_rotate_on_size() noexcept;

  /**
   * @brief Get the maximum combined size above which log files become subject
   *        to pruning.
   *
   * @return Maximum combined size for log files
   */
  [[nodiscard]] static ulonglong get_log_max_size() noexcept;

  /**
   * @brief Get the number of seconds after which log files become subject
   *        to pruning.
   *
   * @return Number of seconds after which log files may be pruned
   */
  [[nodiscard]] static ulonglong get_log_prune_seconds() noexcept;

  /**
   * @brief Get the ident value for syslog.
   *
   * @return Ident value for syslog
   */
  [[nodiscard]] static const char *get_syslog_ident() noexcept;

  /**
   * @brief Get the facility value for syslog.
   *
   * @return Facility value for syslog
   */
  [[nodiscard]] static int get_syslog_facility() noexcept;

  /**
   * @brief Get the priority value for syslog.
   *
   * @return Priority value for syslog
   */
  [[nodiscard]] static int get_syslog_priority() noexcept;

  /**
   * @brief Get audit log compression type.
   *
   * @return Audit log compression type, may be one of possible values
   *         for AuditLogCompressionType
   */
  [[nodiscard]] static AuditLogCompressionType get_compression_type() noexcept;

  /**
   * @brief Get audit log encryption type.
   *
   * @return Audit log encryption type, may be one of possible values
   *         for AuditLogEncryptionType
   */
  [[nodiscard]] static AuditLogEncryptionType get_encryption_type() noexcept;

  /**
   * @brief Set audit log encryption enabled/disabled.
   *
   * @param is_enabled Indicates if audit log encryption is enabled.
   */
  static void set_log_encryption_enabled(bool is_enabled) noexcept;

  /**
   * @brief Check if audit log encryption is enabled.
   *
   * @return true in case audit log encryption is enabled,
   *         false otherwise
   */
  [[nodiscard]] static bool get_log_encryption_enabled() noexcept;

  /**
   * @brief Get the number of days after which archived audit log encryption
   *        passwords are removed.
   *
   * @return number of days after which archived audit log encryption passwords
   *         are removed
   */
  [[nodiscard]] static ulonglong get_password_history_keep_days() noexcept;

  /**
   * @brief Get mean value of randomly generated iterations count used by
   *        password based derivation routine.
   *
   * @return mean value of randomly generated PBKDF iterations count
   */
  [[nodiscard]] static int get_key_derivation_iter_count_mean() noexcept;

  /**
   * @brief Check if a 'time' field is enabled for JSON formatted logs.
   *
   * @return true in case 'time' field containing UNIX timestamp should be
   *             added to log record,
   *         false otherwise
   */
  [[nodiscard]] static bool get_format_unix_timestamp() noexcept;

  /**
   * @brief Set filter_id for a session.
   *
   * @param thd MYSQL_THD for current session
   * @param id Filtering rule ID
   */
  static void set_session_filter_id(MYSQL_THD thd, ulong id) noexcept;

  /**
   * @brief Get value of audit_log_filter_disable variable.
   *
   * @return Value of audit_log_filter_disable variable
   */
  static bool get_log_disabled() noexcept;

  /**
   * @brief Increment counter of events handled by the audit log plugin.
   */
  static void inc_events_total() noexcept;

  /**
   * @brief Get number of events lost in performance logging mode.
   *
   * @return number of lost events
   */
  [[nodiscard]] static uint64_t get_events_lost() noexcept;

  /**
   * @brief Increment counter of events lost in performance logging mode.
   */
  static void inc_events_lost() noexcept;

  /**
   * @brief Increment counter of events handled by the audit log plugin
   *        that were filtered.
   */
  static void inc_events_filtered() noexcept;

  /**
   * @brief Increment counter of events written to the audit log.
   */
  static void inc_events_written() noexcept;

  /**
   * @brief Increment number of times an event had to wait for space
   *        in the audit log buffer.
   */
  static void inc_write_waits() noexcept;

  /**
   * @brief Set size of the largest dropped event in performance logging mode.
   *
   * @param size Size of the dropped event in bytes
   */
  static void update_event_max_drop_size(uint64_t size) noexcept;

  /**
   * @brief Set Audit_log_filter.current_size status variable value.
   *
   * @param size Current log size in bytes
   */
  static void set_current_log_size(uint64_t size) noexcept;

  /**
   * @brief Increase Audit_log_filter.current_size status variable value.
   *
   * @param size Size to add to current value in bytes
   */
  static void update_current_log_size(uint64_t size) noexcept;

  /**
   * @brief Set Audit_log_filter.total_size status variable value.
   *
   * @param size Total logs size in bytes
   */
  static void set_total_log_size(uint64_t size) noexcept;

  /**
   * @brief Increase Audit_log_filter.total_size status variable value.
   *
   * @param size Size to add to current value in bytes
   */
  static void update_total_log_size(uint64_t size) noexcept;

  /**
   * @brief Increment counter for the number of times data is written to log
   *        synchronously bypassing write buffer in asynchronous mode.
   */
  static void inc_direct_writes() noexcept;

  /**
   * @brief Update bookmark to latest event written to log.
   *
   * @param id ID of an audit event
   * @param timestamp timestamp of an audit event
   */
  static void update_log_bookmark(uint64_t id,
                                  const std::string &timestamp) noexcept;

  /**
   * @brief Get bookmark for the latest audit event written to a log.
   *
   * @return Audit event bookmark
   */
  static LogBookmark get_log_bookmark() noexcept;

  /**
   * @brief Get log reader related context object specific for a session.
   *
   * @param thd Connection specific THD instance
   * @return Log reader context
   */
  static AuditLogReaderContext *get_log_reader_context(MYSQL_THD thd) noexcept;

  /**
   * @brief Set log reader related context object specific for a session.
   *
   * @param thd Connection specific THD instance
   * @param context Log reader context
   */
  static void set_log_reader_context(MYSQL_THD thd,
                                     AuditLogReaderContext *context) noexcept;

#ifndef NDEBUG
  /**
   * @brief Get time point from predefined sequence,
   *        used for log rotation testing.
   * @return Time point
   */
  static std::chrono::system_clock::time_point
  get_debug_time_point_for_rotation() noexcept;

  /**
   * @brief Get time point from predefined sequence,
   *        used for log encryption testing.
   * @return Time point
   */
  static std::chrono::system_clock::time_point
  get_debug_time_point_for_encryption() noexcept;
#endif

  /**
   * @brief Get numeric record ID for next log record.
   *
   * @return Record ID
   */
  static uint64_t get_next_record_id() noexcept;

  /**
   * @brief Init record ID sequence number.
   *
   * Set initial value to record sequence number. Initialized to current
   * audit filter log file size during plugin initialization. Incremented by 1
   * for each logged record.
   *
   * @param [in] initial_record_id Initial record sequence number
   */
  static void init_record_id(uint64_t initial_record_id) noexcept;

  /**
   * @brief Store ID of currently active encryption options
   *
   * @param options_id Encryption options ID
   */
  static void set_encryption_options_id(const std::string &options_id) noexcept;

  /**
   * @brief Get ID of currently active encryption options
   *
   * @return Encryption options ID
   */
  static std::string get_encryption_options_id() noexcept;

  /**
   * @brief Get component registry service instance.
   *
   * @return component registry service instance
   */
  static decltype(get_component_registry_service().get())
  get_comp_regystry_srv() noexcept;
};

}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_SYS_VARS_H_INCLUDED
