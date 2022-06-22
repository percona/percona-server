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

#include "plugin/audit_log_filter/sys_vars.h"
#include "plugin/audit_log_filter/audit_error_log.h"
#include "plugin/audit_log_filter/audit_log_filter.h"
#include "plugin/audit_log_filter/sys_var_services.h"

#include "mysql/plugin.h"
#include "sql/sql_const.h"
#include "sql/sql_error.h"
#include "sql/sql_plugin_var.h"

#include <syslog.h>
#include <array>

//#include <boost/lexical_cast/try_lexical_convert.hpp>

namespace audit_log_filter {
namespace {

/*
 * Variable names used during sys vars definition
 */
const char *kComponentName{"audit_log_filter"};
const char *kVarNameFile{"file"};
const char *kVarNameHandler{"handler"};
const char *kVarNameFormat{"format"};
const char *kVarNameStrategy{"strategy"};
const char *kVarNameBufferSize{"buffer_size"};
const char *kVarNameRotateOnSize{"rotate_on_size"};
const char *kVarNameMaxSize{"max_size"};
const char *kVarNamePruneSeconds{"prune_seconds"};
const char *kVarNameFlush{"flush"};
const char *kVarNameSyslogIdent{"syslog_ident"};
const char *kVarNameSyslogFacility{"syslog_facility"};
const char *kVarNameSyslogPriority{"syslog_priority"};

const std::array<const char *, 12> var_names_list{
    kVarNameFile,           kVarNameHandler,     kVarNameFormat,
    kVarNameStrategy,       kVarNameBufferSize,  kVarNameRotateOnSize,
    kVarNameMaxSize,        kVarNameSyslogIdent, kVarNameSyslogFacility,
    kVarNameSyslogPriority, kVarNameFlush,       kVarNamePruneSeconds};

/*
 * TYPE_LIB definition for audit_log_filter.handler
 */
const char *audit_log_filter_handler_names[] = {"FILE", "SYSLOG", nullptr};
TYPE_LIB audit_log_filter_handler_typelib = {
    array_elements(audit_log_filter_handler_names) - 1,
    "audit_log_filter_handler_typelib", audit_log_filter_handler_names,
    nullptr};

/*
 * TYPE_LIB definition for audit_log_filter.format
 */
const char *audit_log_filter_format_names[] = {"NEW", "OLD", "JSON", "CSV",
                                               nullptr};
TYPE_LIB audit_log_filter_format_typelib = {
    array_elements(audit_log_filter_format_names) - 1,
    "audit_log_filter_format_typelib", audit_log_filter_format_names, nullptr};

/*
 * TYPE_LIB definition for audit_log_filter.strategy
 */
const char *audit_log_filter_strategy_names[] = {
    "ASYNCHRONOUS", "PERFORMANCE", "SEMISYNCHRONOUS", "SYNCHRONOUS", nullptr};
TYPE_LIB audit_log_filter_strategy_typelib = {
    array_elements(audit_log_filter_strategy_names) - 1,
    "audit_log_filter_strategy_typelib", audit_log_filter_strategy_names,
    nullptr};

/*
 * TYPE_LIB definition for audit_log_filter.syslog_facility
 */
const int audit_log_filter_syslog_facility_codes[] = {
    LOG_USER,     LOG_AUTHPRIV, LOG_CRON,   LOG_DAEMON, LOG_FTP,    LOG_KERN,
    LOG_LPR,      LOG_MAIL,     LOG_NEWS,
#if (defined LOG_SECURITY)
    LOG_SECURITY,
#endif
    LOG_SYSLOG,   LOG_AUTH,     LOG_UUCP,   LOG_LOCAL0, LOG_LOCAL1, LOG_LOCAL2,
    LOG_LOCAL3,   LOG_LOCAL4,   LOG_LOCAL5, LOG_LOCAL6, LOG_LOCAL7, 0};

const char *audit_log_filter_syslog_facility_names[] = {
    "LOG_USER",     "LOG_AUTHPRIV", "LOG_CRON",   "LOG_DAEMON", "LOG_FTP",
    "LOG_KERN",     "LOG_LPR",      "LOG_MAIL",   "LOG_NEWS",
#if (defined LOG_SECURITY)
    "LOG_SECURITY",
#endif
    "LOG_SYSLOG",   "LOG_AUTH",     "LOG_UUCP",   "LOG_LOCAL0", "LOG_LOCAL1",
    "LOG_LOCAL2",   "LOG_LOCAL3",   "LOG_LOCAL4", "LOG_LOCAL5", "LOG_LOCAL6",
    "LOG_LOCAL7",   nullptr};

TYPE_LIB audit_log_filter_syslog_facility_typelib = {
    array_elements(audit_log_filter_syslog_facility_names) - 1,
    "audit_log_filter_syslog_facility_typelib",
    audit_log_filter_syslog_facility_names, nullptr};

/*
 * TYPE_LIB definition for audit_log_filter.syslog_priority
 */
const int audit_log_filter_syslog_priority_codes[] = {
    LOG_INFO,   LOG_ALERT, LOG_CRIT,  LOG_ERR, LOG_WARNING,
    LOG_NOTICE, LOG_EMERG, LOG_DEBUG, 0};

const char *audit_log_filter_syslog_priority_names[] = {
    "LOG_INFO",   "LOG_ALERT", "LOG_CRIT",  "LOG_ERR", "LOG_WARNING",
    "LOG_NOTICE", "LOG_EMERG", "LOG_DEBUG", 0};

TYPE_LIB audit_log_filter_syslog_priority_typelib = {
    array_elements(audit_log_filter_syslog_priority_names) - 1,
    "audit_log_filter_syslog_priority_typelib",
    audit_log_filter_syslog_priority_names, nullptr};

void flush_update_func(MYSQL_THD, SYS_VAR *, void *val_ptr, const void *save) {
  const auto *val = static_cast<const bool *>(save);
  auto *sys_vars = static_cast<VarWrapper<bool> *>(val_ptr)->get_container();

  if (*val && sys_vars->get_rotate_on_size() == 0) {
    sys_vars->get_mediator()->on_audit_log_flush_requested();
  }
}

void max_size_update_func(MYSQL_THD thd, SYS_VAR *, void *val_ptr,
                          const void *save) {
  const auto *val = static_cast<const ulonglong *>(save);
  *static_cast<VarWrapper<ulonglong> *>(val_ptr) = *val;

  if (*val > 0) {
    if (static_cast<VarWrapper<bool> *>(val_ptr)
            ->get_container()
            ->get_log_prune_seconds() > 0) {
      push_warning(thd, Sql_condition::SL_WARNING,
                   ER_WARN_ADUIT_FILTER_MAX_SIZE_AND_PRUNE_SECONDS, nullptr);
    }

    static_cast<VarWrapper<bool> *>(val_ptr)
        ->get_container()
        ->get_mediator()
        ->on_audit_log_prune_requested();
  }
}

void prune_seconds_update_func(MYSQL_THD thd, SYS_VAR *, void *val_ptr,
                               const void *save) {
  const auto *val = static_cast<const ulonglong *>(save);
  *static_cast<VarWrapper<ulonglong> *>(val_ptr) = *val;

  if (*val > 0) {
    if (static_cast<VarWrapper<bool> *>(val_ptr)
            ->get_container()
            ->get_log_max_size() > 0) {
      push_warning(thd, Sql_condition::SL_WARNING,
                   ER_WARN_ADUIT_FILTER_MAX_SIZE_AND_PRUNE_SECONDS, nullptr);
    }

    static_cast<VarWrapper<bool> *>(val_ptr)
        ->get_container()
        ->get_mediator()
        ->on_audit_log_prune_requested();
  }
}

}  // namespace

SysVars::SysVars(std::unique_ptr<SysVarServices> sys_var_services)
    : AuditBaseComponent(), m_sys_var_services{std::move(sys_var_services)} {
  m_log_flush_requested.set_container(this);
  m_log_max_size.set_container(this);
  m_log_prune_seconds.set_container(this);
}

SysVars::~SysVars() {
  auto *sys_var_srv = m_sys_var_services->get_sys_var_unreg();

  for (const auto &var_name : var_names_list) {
    if (sys_var_srv->unregister_variable(kComponentName, var_name)) {
      LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                      "Failed to unregister %s.%s variable", kComponentName,
                      var_name);
    }
  }
}

bool SysVars::init() noexcept {
  using ulonglong_arg_check_t = INTEGRAL_CHECK_ARG(ulonglong);
  using str_arg_check_t = STR_CHECK_ARG(str);
  using enum_arg_check_t = ENUM_CHECK_ARG(enum);
  using bool_arg_check_t = BOOL_CHECK_ARG(bool);

  auto *sys_var_srv = m_sys_var_services->get_sys_var_reg();

  /*
   * The audit_log_filter.file variable is used to specify the filename that’s
   * going to store the audit log. It can contain the path relative to the
   * datadir or absolute path.
   */
  str_arg_check_t file_arg_check{strdup("audit_filter.log")};

  if (sys_var_srv->register_variable(
          kComponentName, kVarNameFile,
          PLUGIN_VAR_STR | PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_READONLY |
              PLUGIN_VAR_MEMALLOC,
          "The name of the log file.", nullptr, nullptr,
          static_cast<void *>(&file_arg_check),
          static_cast<void *>(&m_file_name))) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to init %s.%s variable", kComponentName,
                    kVarNameFile);
    return false;
  }

  /*
   * The audit_log_filter.handler variable is used to configure where the
   * audit log will be written. If it is set to FILE, the log will be written
   * into a file specified by audit_log_filter.file variable. If it is set to
   * SYSLOG, the audit log will be written to syslog.
   */
  enum_arg_check_t handler_arg_check{
      static_cast<ulong>(AuditLogHandlerType::File),
      &audit_log_filter_handler_typelib};

  if (sys_var_srv->register_variable(
          kComponentName, kVarNameHandler,
          PLUGIN_VAR_ENUM | PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_READONLY,
          "The audit log filter handler.", nullptr, nullptr,
          static_cast<void *>(&handler_arg_check),
          static_cast<void *>(&m_handler_type))) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to init %s.%s variable", kComponentName,
                    kVarNameHandler);
    return false;
  }

  /*
   * The audit_log_filter.format variable is used to specify the audit filter
   * log format. The audit log filter plugin supports four log formats:
   * OLD, NEW, JSON, and CSV. OLD and NEW formats are based on XML, where
   * the former outputs log record properties as XML attributes and the latter
   * as XML tags.
   */
  enum_arg_check_t format_arg_check{static_cast<ulong>(AuditLogFormatType::New),
                                    &audit_log_filter_format_typelib};

  if (sys_var_srv->register_variable(
          kComponentName, kVarNameFormat,
          PLUGIN_VAR_ENUM | PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_READONLY,
          "The audit log filter file format.", nullptr, nullptr,
          static_cast<void *>(&format_arg_check),
          static_cast<void *>(&m_format_type))) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to init %s.%s variable", kComponentName,
                    kVarNameFormat);
    return false;
  }

  /*
   * The audit_log_filter.strategy variable is used to specify the audit log
   * filter strategy, possible values are:
   * ASYNCHRONOUS - (default) log using memory buffer, do not drop messages
   *                if buffer is full
   * PERFORMANCE - log using memory buffer, drop messages if buffer is full
   * SEMISYNCHRONOUS - log directly to file, do not flush and sync every event
   * SYNCHRONOUS - log directly to file, flush and sync every event.
   *
   * This variable has effect only when audit_log_handler is set to FILE.
   */
  enum_arg_check_t strategy_arg_check{
      static_cast<ulong>(AuditLogStrategyType::Asynchronous),
      &audit_log_filter_strategy_typelib};

  if (sys_var_srv->register_variable(
          kComponentName, kVarNameStrategy,
          PLUGIN_VAR_ENUM | PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_READONLY,
          "The logging method used by the audit log filter plugin, "
          "if FILE handler is used.",
          nullptr, nullptr, static_cast<void *>(&strategy_arg_check),
          static_cast<void *>(&m_file_stategy_type))) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to init %s.%s variable", kComponentName,
                    kVarNameStrategy);
    return false;
  }

  /*
   * The audit_log_filter.buffer_size variable can be used to specify the size
   * of memory buffer used for logging, used when audit_log_filter.strategy
   * variable is set to ASYNCHRONOUS or PERFORMANCE values. This variable has
   * effect only when audit_log_filter.handler is set to FILE.
   */
  ulonglong_arg_check_t buffer_size_arg_check{1048576UL, 4096UL, ULLONG_MAX,
                                              4096UL};

  if (sys_var_srv->register_variable(
          kComponentName, kVarNameBufferSize,
          PLUGIN_VAR_LONGLONG | PLUGIN_VAR_UNSIGNED | PLUGIN_VAR_RQCMDARG |
              PLUGIN_VAR_READONLY,
          "The size of the buffer for asynchronous logging, "
          "if FILE handler is used.",
          nullptr, nullptr, static_cast<void *>(&buffer_size_arg_check),
          static_cast<void *>(&m_buffer_size))) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to init %s.%s variable", kComponentName,
                    kVarNameStrategy);
    return false;
  }

  /*
   * The audit_log_filter.rotate_on_size variable specifies the maximum size
   * of the audit log file. Upon reaching this size, the audit log will be
   * rotated. For this variable to take effect, set the audit_log_filter.handler
   * variable to FILE and the audit_log_filter.rotations variable to a value
   * greater than zero.
   */
  ulonglong_arg_check_t rotate_on_size_arg_check{0UL, 0UL, ULLONG_MAX, 4096UL};

  if (sys_var_srv->register_variable(
          kComponentName, kVarNameRotateOnSize,
          PLUGIN_VAR_LONGLONG | PLUGIN_VAR_UNSIGNED | PLUGIN_VAR_RQCMDARG,
          "Maximum size of the log to start the rotation, "
          "if FILE handler is used.",
          nullptr, nullptr, static_cast<void *>(&rotate_on_size_arg_check),
          static_cast<void *>(&m_rotate_on_size))) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to init %s.%s variable", kComponentName,
                    kVarNameRotateOnSize);
    return false;
  }

  /*
   * A value greater than 0 enables size-based pruning. The value is the
   * combined size above which audit log files become subject to pruning.
   */
  ulonglong_arg_check_t max_size_arg_check{0UL, 0UL, ULLONG_MAX, 4096UL};

  if (sys_var_srv->register_variable(
          kComponentName, kVarNameMaxSize,
          PLUGIN_VAR_LONGLONG | PLUGIN_VAR_UNSIGNED | PLUGIN_VAR_OPCMDARG,
          "The maximum combined size of log files in bytes after which log "
          "files become subject to pruning.",
          nullptr, max_size_update_func,
          static_cast<void *>(&max_size_arg_check),
          static_cast<void *>(&m_log_max_size))) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to init %s.%s variable", kComponentName,
                    kVarNameMaxSize);
    return false;
  }

  /*
   * A value greater than 0 enables age-based pruning. The value is the number
   * of seconds after which log files become subject to pruning.
   */
  ulonglong_arg_check_t prune_seconds_arg_check{0UL, 0UL, ULLONG_MAX, 0UL};

  if (sys_var_srv->register_variable(
          kComponentName, kVarNamePruneSeconds,
          PLUGIN_VAR_LONGLONG | PLUGIN_VAR_UNSIGNED | PLUGIN_VAR_OPCMDARG,
          "The maximum log file age in seconds after which log file "
          "become subject to pruning.",
          nullptr, prune_seconds_update_func,
          static_cast<void *>(&prune_seconds_arg_check),
          static_cast<void *>(&m_log_prune_seconds))) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to init %s.%s variable", kComponentName,
                    kVarNamePruneSeconds);
    return false;
  }

  /*
   * When this variable is set to ON log file will be closed and reopened.
   * This can be used for manual log rotation.
   */
  bool_arg_check_t flush_arg_check{false};

  if (sys_var_srv->register_variable(
          kComponentName, kVarNameFlush, PLUGIN_VAR_BOOL | PLUGIN_VAR_NOCMDARG,
          "Close and reopen log file when set to ON.", nullptr,
          flush_update_func, static_cast<void *>(&flush_arg_check),
          static_cast<void *>(&m_log_flush_requested))) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to init %s.%s variable", kComponentName,
                    kVarNameFlush);
    return false;
  }

  /*
   * The audit_log_filter.syslog_ident variable is used to specify the ident
   * value for syslog.
   */
  str_arg_check_t syslog_ident_arg_check{strdup("percona-audit-event-filter")};

  if (sys_var_srv->register_variable(
          kComponentName, kVarNameSyslogIdent,
          PLUGIN_VAR_STR | PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_READONLY |
              PLUGIN_VAR_MEMALLOC,
          "The string that will be prepended to each log message, "
          "if SYSLOG handler is used.",
          nullptr, nullptr, static_cast<void *>(&syslog_ident_arg_check),
          static_cast<void *>(&m_syslog_ident))) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to init %s.%s variable", kComponentName,
                    kVarNameSyslogIdent);
    return false;
  }

  /*
   * The audit_log_filter.syslog_facility variable is used to specify the
   * facility value for syslog.
   */
  enum_arg_check_t syslog_facility_arg_check{
      0, &audit_log_filter_syslog_facility_typelib};

  if (sys_var_srv->register_variable(
          kComponentName, kVarNameSyslogFacility,
          PLUGIN_VAR_ENUM | PLUGIN_VAR_RQCMDARG,
          "The syslog facility to assign to messages, if SYSLOG handler is "
          "used.",
          nullptr, nullptr, static_cast<void *>(&syslog_facility_arg_check),
          static_cast<void *>(&m_syslog_facility))) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to init %s.%s variable", kComponentName,
                    kVarNameSyslogFacility);
    return false;
  }

  /*
   * The audit_log_filter.syslog_priority variable is used to specify the
   * priority value for syslog. This variable has the same meaning as the
   * appropriate parameter described in the syslog(3) manual.
   */
  enum_arg_check_t syslog_priority_arg_check{
      0, &audit_log_filter_syslog_priority_typelib};

  if (sys_var_srv->register_variable(
          kComponentName, kVarNameSyslogPriority,
          PLUGIN_VAR_ENUM | PLUGIN_VAR_RQCMDARG,
          "Priority to be assigned to all messages written to syslog.", nullptr,
          nullptr, static_cast<void *>(&syslog_priority_arg_check),
          static_cast<void *>(&m_syslog_priority))) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to init %s.%s variable", kComponentName,
                    kVarNameSyslogPriority);
    return false;
  }

  validate();

  return true;
}

void SysVars::validate() const noexcept {
  if (get_log_max_size() > 0 && get_log_prune_seconds() > 0) {
    LogPluginErr(WARNING_LEVEL,
                 ER_WARN_ADUIT_FILTER_MAX_SIZE_AND_PRUNE_SECONDS_LOG);
  }
}

// TODO: support for
// sys vars
//  MYSQL_SYSVAR(record_buffer),
//  MYSQL_SYSVAR(query_stack),
//  audit_log_compression
//  audit_log_current_session
//  audit_log_disable
//  audit_log_encryption
//  audit_log_filter_id
//  audit_log_password_history_keep_days
//  audit_log_prune_seconds
//  audit_log_read_buffer_size
//
// status vars
//  Audit_log_current_size
//  Audit_log_event_max_drop_size
//  Audit_log_events
//  Audit_log_events_filtered
//  Audit_log_events_lost
//  Audit_log_events_written
//  Audit_log_total_size
//  Audit_log_write_waits

int SysVars::get_syslog_facility() const noexcept {
  return audit_log_filter_syslog_facility_codes[m_syslog_facility];
}

int SysVars::get_syslog_priority() const noexcept {
  return audit_log_filter_syslog_priority_codes[m_syslog_priority];
}

}  // namespace audit_log_filter
