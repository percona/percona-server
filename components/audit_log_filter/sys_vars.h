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

#include "components/audit_log_filter/component_registry_service.h"
#include "components/audit_log_filter/log_record_formatter/base.h"
#include "components/audit_log_filter/log_writer/base.h"

#include <mysql/components/services/mysql_current_thread_reader.h>

#include <memory>
#include <string>

namespace audit_log_filter {

class AuditRule;

struct SessionFilterRuleCache {
  uint64_t generation;
  uint64_t detach_generation;
  uint64_t removed_filter_generation;
  std::shared_ptr<AuditRule> rule;
};

struct AuditLogReaderContext;
class SysVars;

using log_record_formatter::AuditLogFormatType;
using log_writer::AuditLogCompressionType;
using log_writer::AuditLogEncryptionType;
using log_writer::AuditLogHandlerType;
using log_writer::AuditLogStrategyType;

enum class AuditLogEventModeType { Reduced = 0, Full = 1 };

struct LogBookmark {
  uint64_t id;
  std::string timestamp;
};

class SysVars {
 public:
  /**
   * @brief Initialise system and status variables
   *
   * @return true in case of success, false otherwise
   */
  static bool init() noexcept;

  /**
   * @brief Deinit system and status variables
   */
  static void deinit() noexcept;

  /**
   * @brief Validate system variables settings.
   *
   * @return true in case of success, false otherwise
   */
  [[nodiscard]] static bool validate() noexcept;

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
   * @brief Get database name where to search for config tables.
   *
   * @return Database name
   */
  [[nodiscard]] static const char *get_config_database_name() noexcept;

  /**
   * @brief Get audit log filter handler type.
   *
   * @return Audit log filter handler type, may be one of possible values
   *         of AuditLogHandlerType
   */
  [[nodiscard]] static AuditLogHandlerType get_handler_type() noexcept;

  /**
   * @brief Get audit log filter event mode.
   *
   * @return Audit log filter event mode, may be one of possible values
   *         of AuditLogEventModeType
   */
  [[nodiscard]] static AuditLogEventModeType get_event_mode_type() noexcept;

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
   * @brief Get the syslog messages tag value.
   *
   * @return Syslog tag value
   */
  [[nodiscard]] static const char *get_syslog_tag() noexcept;

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
   * @brief Check if O_DIRECT is enabled for audit log file writes.
   *
   * @return true if O_DIRECT should be used, false otherwise
   */
  [[nodiscard]] static bool get_direct_io() noexcept;

  /**
   * @brief Set filter_id for a session.
   *
   * @param thd MYSQL_THD for current session
   * @param id Filtering rule ID
   */
  static void set_session_filter_id(MYSQL_THD thd, ulong id) noexcept;

  /**
   * @brief Get filter_id for a session.
   *
   * @param thd MYSQL_THD for current session
   *
   * @return Session filter ID, equals 0 in case no filtering rule is assigned
   *         to the session.
   */
  static ulong get_session_filter_id(MYSQL_THD thd) noexcept;

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

  /**
   * @brief Get the cached filter rule for a session.
   *
   * @param thd Connection specific THD instance
   * @return Pointer to session filter rule cache, or nullptr if not yet cached
   */
  static SessionFilterRuleCache *get_session_filter_rule(
      MYSQL_THD thd) noexcept;

  /**
   * @brief Cache a filter rule for a session.
   *
   * @param thd Connection specific THD instance
   * @param generation Current filter rule generation at time of resolution
   * @param detach_generation Current detach generation at time of resolution
   * @param removed_filter_generation Current removed-filter generation at time
   *                                  of resolution
   * @param rule The resolved filter rule (nullptr if no rule applies)
   */
  static void set_session_filter_rule(MYSQL_THD thd, uint64_t generation,
                                      uint64_t detach_generation,
                                      uint64_t removed_filter_generation,
                                      std::shared_ptr<AuditRule> rule) noexcept;

  /**
   * @brief Get the current global filter rule generation counter.
   *
   * @return Current generation value
   */
  static uint64_t get_filter_rule_generation() noexcept;

  /**
   * @brief Increment the global filter rule generation counter.
   *
   * Forces all sessions to re-resolve their filter rule on the next event.
   * Used by flush and remove_filter operations.
   */
  static void bump_filter_rule_generation() noexcept;

  /**
   * @brief Get the current global detach generation counter.
   *
   * @return Current generation value
   */
  static uint64_t get_filter_rule_detach_generation() noexcept;

  /**
   * @brief Increment the global detach generation counter.
   *
   * Forces current sessions to detach from their cached filter until the next
   * connect or change-user operation.
   */
  static void bump_filter_rule_detach_generation() noexcept;

  /**
   * @brief Get the current removed-filter generation counter.
   *
   * @return Current generation value
   */
  static uint64_t get_removed_filter_generation() noexcept;

  /**
   * @brief Mark a filter ID as removed for existing sessions.
   *
   * Sessions still using this cached filter ID will detach on their next
   * non-connection event.
   */
  static void mark_removed_filter_id(uint64_t filter_id) noexcept;

  /**
   * @brief Check whether a filter ID has been removed.
   *
   * @param filter_id Filtering rule ID
   * @param since_generation The session cache's removed-filter generation
   * @return true if the filter was removed after since_generation,
   *         false otherwise
   */
  static bool is_removed_filter_id(uint64_t filter_id,
                                   uint64_t since_generation) noexcept;

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
   * @brief Acquire component registry service.
   *
   * @return component registry service instance
   */
  static comp_registry_srv_t *acquire_comp_registry_srv() noexcept;

  /**
   * @brief Release component registry service.
   */
  static void release_comp_registry_srv() noexcept;

  /**
   * @brief Get component registry service instance.
   *
   * @return component registry service instance
   */
  static comp_registry_srv_t *get_comp_registry_srv() noexcept;
};

}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_SYS_VARS_H_INCLUDED
