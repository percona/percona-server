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

#ifndef AUDIT_LOG_FILTER_H_INCLUDED
#define AUDIT_LOG_FILTER_H_INCLUDED

#include "components/audit_log_filter/audit_error_log.h"
#include "components/audit_log_filter/audit_event_class_internal.h"

#include <mysql/components/services/dynamic_privilege.h>
#include <mysql/components/services/mysql_current_thread_reader.h>
#include <mysql/components/services/security_context.h>

#include "components/audit_log_filter/audit_record.h"

#include <atomic>
#include <memory>

extern SERVICE_TYPE(log_builtins) * log_bi;
extern SERVICE_TYPE(log_builtins_string) * log_bs;

namespace audit_log_filter {
namespace log_writer {
class LogWriterBase;
struct FileRotationResult;
}  // namespace log_writer

class AuditRuleRegistry;
class AuditUdf;
class AuditLogReader;

class AuditLogFilter {
 public:
  AuditLogFilter() = delete;
  AuditLogFilter(std::unique_ptr<AuditRuleRegistry> audit_rules_registry,
                 std::unique_ptr<AuditUdf> audit_udf,
                 std::unique_ptr<log_writer::LogWriterBase> log_writer,
                 std::unique_ptr<AuditLogReader> log_reader);

  /**
   * @brief Init plugin components.
   *
   * @return true in case of success, false otherwise
   */
  bool init() noexcept;

  /**
   * @brief De-init plugin components.
   */
  void deinit() noexcept;

  /**
   * @brief Process audit event.
   *
   * @param event_class Event class
   * @param event Event info
   * @return Event processing status, 0 in case of success or non-zero code
   *         otherwise
   */
  int notify_event(audit_event_class_t event_class, const void *event);

  /**
   * @brief Get UDFs handler instance.
   * @return UDF handler instance
   */
  AuditUdf *get_udf() noexcept;

  /**
   * @brief Get log reader instance.
   *
   * @return Log reader instance
   */
  AuditLogReader *get_log_reader() noexcept;

  /**
   * @brief Send Audit event to log upon plugin initialization.
   */
  void send_audit_start_event() noexcept;

  /**
   * @brief Send NoAudit event to log upon plugin de-initialization.
   */
  void send_audit_stop_event() noexcept;

 public:
  /**
   * @brief Handle filters flush request.
   *
   * @return true in case filters reloaded successfully, false otherwise
   */
  bool on_audit_rule_flush_requested() noexcept;

  /**
   * @brief Handle log files pruning request.
   */
  void on_audit_log_prune_requested() noexcept;

  /**
   * @brief Handle log files rotation request.
   */
  void on_audit_log_rotate_requested(
      log_writer::FileRotationResult *result = nullptr) noexcept;

  /**
   * @brief Handle encryption password pruning request.
   */
  void on_encryption_password_prune_requested() noexcept;

  /**
   * @brief Handle successful log rotation event.
   */
  void on_audit_log_rotated() noexcept;

 private:
  void get_connection_attrs(MYSQL_THD thd, AuditRecordVariant &audit_record);

  /**
   * @brief Get user and host name from connection THD instance
   *
   * @param ctx Security context handle
   * @param user_name Returned user name
   * @param user_host Returned host name
   * @return true in case user and host name are fetched successfully,
   *         false otherwise
   */
  bool get_connection_user(Security_context_handle &ctx, std::string &user_name,
                           std::string &user_host) noexcept;

  /**
   * @brief Check if user has AUDIT_ABORT_EXEMPT privilege assigned
   *
   * @param ctx Security context handle
   * @return true in case user has AUDIT_ABORT_EXEMPT privilege,
   *         false otherwise
   */
  bool check_abort_exempt_privilege(Security_context_handle &ctx) noexcept;

  /**
   * @brief Get user's security context handle
   *
   * @param thd Server thread instance
   * @param ctx Security context handle
   * @return true in case of success, false otherwise
   */
  bool get_security_context(MYSQL_THD thd,
                            Security_context_handle *ctx) noexcept;

 private:
  std::unique_ptr<AuditRuleRegistry> m_audit_rules_registry;
  std::unique_ptr<AuditUdf> m_audit_udf;
  std::unique_ptr<log_writer::LogWriterBase> m_log_writer;
  std::unique_ptr<AuditLogReader> m_log_reader;
  std::atomic_bool m_is_active;

  SERVICE_TYPE(mysql_thd_security_context) * m_security_context_srv;
  SERVICE_TYPE(mysql_security_context_options) * m_security_context_opts_srv;
  SERVICE_TYPE(global_grants_check) * m_grants_check_srv;
};

/**
 * @brief Get audit log filter plugin instance.
 *
 * @return Audit log filter plugin instance
 */
AuditLogFilter *get_audit_log_filter_instance() noexcept;

}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_H_INCLUDED
