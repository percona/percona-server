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

#include "plugin/audit_log_filter/audit_error_log.h"

#include "plugin/audit_log_filter/audit_filter.h"
#include "plugin/audit_log_filter/audit_log_filter.h"
#include "plugin/audit_log_filter/audit_log_reader.h"
#include "plugin/audit_log_filter/audit_rule.h"
#include "plugin/audit_log_filter/audit_rule_registry.h"
#include "plugin/audit_log_filter/audit_udf.h"
#include "plugin/audit_log_filter/log_record_formatter.h"
#include "plugin/audit_log_filter/log_writer.h"
#include "plugin/audit_log_filter/sys_vars.h"

#include <mysql/components/services/mysql_connection_attributes_iterator.h>
#include <mysql/components/services/security_context.h>

#include "mysql/plugin.h"
#include "sql/sql_class.h"

#include <array>
#include <memory>
#include <variant>

#define PLUGIN_VERSION 0x0100

#ifdef WIN32
#define PLUGIN_EXPORT extern "C" __declspec(dllexport)
#else
#define PLUGIN_EXPORT extern "C"
#endif

SERVICE_TYPE(log_builtins) *log_bi = nullptr;
SERVICE_TYPE(log_builtins_string) *log_bs = nullptr;

namespace audit_log_filter {
namespace {
AuditLogFilter *audit_log_filter = nullptr;

void my_plugin_perror() noexcept {
  char errbuf[MYSYS_STRERROR_SIZE];
  my_strerror(errbuf, sizeof(errbuf), errno);
  LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG, "Error: %s", errbuf);
}

}  // namespace

AuditLogFilter *get_audit_log_filter_instance() noexcept {
  return audit_log_filter;
}

/*
 * Audit UDF functions
 */

#define DECLARE_AUDIT_UDF_INIT(NAME)                                      \
  PLUGIN_EXPORT                                                           \
  bool NAME##_udf_init(UDF_INIT *initid, UDF_ARGS *args, char *message) { \
    return audit_log_filter::AuditUdf::NAME##_udf_init(                   \
        audit_log_filter->get_udf(), initid, args, message);              \
  }

#define DECLARE_AUDIT_UDF_FUNC(NAME)                                           \
  PLUGIN_EXPORT                                                                \
  char *NAME##_udf(UDF_INIT *initid, UDF_ARGS *args, char *result,             \
                   unsigned long *length, unsigned char *is_null,              \
                   unsigned char *error) {                                     \
    return audit_log_filter::AuditUdf::NAME##_udf(audit_log_filter->get_udf(), \
                                                  initid, args, result,        \
                                                  length, is_null, error);     \
  }

#define DECLARE_AUDIT_UDF_DEINIT(NAME)                     \
  PLUGIN_EXPORT                                            \
  void NAME##_udf_deinit(UDF_INIT *initid) {               \
    audit_log_filter::AuditUdf::NAME##_udf_deinit(initid); \
  }

#define DECLARE_AUDIT_UDF(NAME) \
  DECLARE_AUDIT_UDF_INIT(NAME)  \
  DECLARE_AUDIT_UDF_FUNC(NAME)  \
  DECLARE_AUDIT_UDF_DEINIT(NAME)

DECLARE_AUDIT_UDF(audit_log_filter_set_filter)
DECLARE_AUDIT_UDF(audit_log_filter_remove_filter)
DECLARE_AUDIT_UDF(audit_log_filter_set_user)
DECLARE_AUDIT_UDF(audit_log_filter_remove_user)
DECLARE_AUDIT_UDF(audit_log_filter_flush)
DECLARE_AUDIT_UDF(audit_log_read)
DECLARE_AUDIT_UDF(audit_log_read_bookmark)

#define DECLARE_AUDIT_UDF_INFO(NAME) \
  UdfFuncInfo { #NAME, &NAME##_udf, &NAME##_udf_init, &NAME##_udf_deinit }

static std::array udfs_list{
    DECLARE_AUDIT_UDF_INFO(audit_log_filter_set_filter),
    DECLARE_AUDIT_UDF_INFO(audit_log_filter_remove_filter),
    DECLARE_AUDIT_UDF_INFO(audit_log_filter_set_user),
    DECLARE_AUDIT_UDF_INFO(audit_log_filter_remove_user),
    DECLARE_AUDIT_UDF_INFO(audit_log_filter_flush),
    DECLARE_AUDIT_UDF_INFO(audit_log_read),
    DECLARE_AUDIT_UDF_INFO(audit_log_read_bookmark)};

/**
 * @brief Initialize the plugin at server start or plugin installation.
 *
 * @param plugin_info Pointer to plugin info structure
 * @return Initialization status, 0 in case of success or non zero
 *         code otherwise
 */
int audit_log_filter_init(MYSQL_PLUGIN plugin_info [[maybe_unused]]) {
  LogPluginErr(INFORMATION_LEVEL, ER_LOG_PRINTF_MSG,
               "Initializing Audit Event Filter...");

  SysVars::validate();

  auto comp_registry_srv = get_component_registry_service();

  if (comp_registry_srv == nullptr) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                 "Failed to acquire components registry service");
    return 1;
  }

  auto audit_udf = std::make_unique<AuditUdf>(comp_registry_srv.get());

  if (audit_udf == nullptr) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                 "Failed to create UDFs handler instance");
    return 1;
  }

  if (!audit_udf->init(udfs_list.begin(), udfs_list.end())) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, "Failed to init UDFs");
    return 1;
  }

  auto audit_rule_registry =
      std::make_unique<AuditRuleRegistry>(comp_registry_srv.get());

  if (audit_rule_registry == nullptr) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                 "Failed to create audit rule registry instance");
    return 1;
  }

  if (!audit_rule_registry->load()) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                 "Failed to load filtering rules");
    return 1;
  }

  auto formatter = get_log_record_formatter(SysVars::get_format_type());

  if (formatter == nullptr) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                 "Failed to init record formatter");
    return 1;
  }

  auto log_writer = get_log_writer(std::move(formatter));

  if (log_writer == nullptr) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                 "Failed to create log writer instance");
    return 1;
  }

  if (!log_writer->open()) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG, "Cannot open log writer");
    my_plugin_perror();
    return 1;
  }

  auto log_reader = std::make_unique<AuditLogReader>(comp_registry_srv.get());

  if (log_reader == nullptr) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                 "Failed to create log reader instance");
    return 1;
  }

  if (!log_reader->init()) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG, "Cannot open log reader");
    my_plugin_perror();
    return 1;
  }

  audit_log_filter = new AuditLogFilter(
      std::move(comp_registry_srv), std::move(audit_rule_registry),
      std::move(audit_udf), std::move(log_writer), std::move(log_reader));

  if (SysVars::get_log_disabled()) {
    LogPluginErr(WARNING_LEVEL, ER_WARN_AUDIT_LOG_FILTER_DISABLED);
  }

  return 0;
}

/**
 * @brief Terminate the plugin at server shutdown or plugin deinstallation.
 *
 * @param arg Plugin descriptor pointer
 * @return Plugin deinit status, 0 in case of success or non zero
 *         code otherwise
 */
int audit_log_filter_deinit(void *arg [[maybe_unused]]) {
  LogPluginErr(INFORMATION_LEVEL, ER_LOG_PRINTF_MSG,
               "Uninstalled Audit Event Filter");

  if (audit_log_filter == nullptr) {
    return 0;
  }

  delete audit_log_filter;
  audit_log_filter = nullptr;

  return 0;
}

/**
 * @brief Process audit event.
 *
 * @param thd Connection specific THD instance
 * @param event_class Event class
 * @param event Event info
 * @return Event processing status, 0 in case of success or non-zero code
 *         otherwise
 */
int audit_log_notify(MYSQL_THD thd, mysql_event_class_t event_class,
                     const void *event) {
  return audit_log_filter->notify_event(thd, event_class, event);
}

AuditLogFilter::AuditLogFilter(
    comp_registry_srv_container_t comp_registry_srv,
    std::unique_ptr<AuditRuleRegistry> audit_rules_registry,
    std::unique_ptr<AuditUdf> audit_udf,
    std::unique_ptr<log_writer::LogWriterBase> log_writer,
    std::unique_ptr<AuditLogReader> log_reader)
    : m_comp_registry_srv{std::move(comp_registry_srv)},
      m_audit_rules_registry{std::move(audit_rules_registry)},
      m_audit_udf{std::move(audit_udf)},
      m_log_writer{std::move(log_writer)},
      m_filter{std::make_unique<AuditEventFilter>()},
      m_log_reader{std::move(log_reader)} {
  m_audit_udf->set_mediator(this);
}

int AuditLogFilter::notify_event(MYSQL_THD thd, mysql_event_class_t event_class,
                                 const void *event) {
  if (SysVars::get_log_disabled()) {
    return 0;
  }

  SysVars::inc_events_total();

  std::string user_name;
  std::string user_host;

  if (!get_connection_user(thd, user_name, user_host)) {
    return 0;
  }

  // Get connection specific filtering rule
  std::string rule_name;

  if (!m_audit_rules_registry->lookup_rule_name(user_name, user_host,
                                                rule_name)) {
    return 0;
  }

  auto *filter_rule = m_audit_rules_registry->get_rule(rule_name);

  if (filter_rule == nullptr) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to find '%s' filtering rule", rule_name.c_str());
    return 0;
  }

  SysVars::set_session_filter_id(thd, filter_rule->get_filter_id());

  // Get actual event info based on event class
  AuditRecordVariant audit_record = get_audit_record(event_class, event);

  if (std::holds_alternative<AuditRecordUnknown>(audit_record)) {
    LogPluginErrMsg(WARNING_LEVEL, ER_LOG_PRINTF_MSG,
                    "Unsupported audit event class with ID %i received",
                    event_class);
    return 0;
  }

  auto ev_name = std::visit(
      [](const auto &rec) -> std::string_view { return rec.event_class_name; },
      audit_record);

  // Apply filtering rule
  AuditAction filter_result = m_filter->apply(filter_rule, audit_record);

  if (filter_result == AuditAction::Skip) {
    SysVars::inc_events_filtered();
    return 0;
  }

  if (filter_result == AuditAction::Block) {
    LogPluginErrMsg(INFORMATION_LEVEL, ER_LOG_PRINTF_MSG,
                    "Blocked audit event '%s' with class %i", ev_name.data(),
                    event_class);
    return 1;
  }

  if (event_class == mysql_event_class_t::MYSQL_AUDIT_CONNECTION_CLASS) {
    get_connection_attrs(thd, audit_record);
  }

  m_log_writer->write(audit_record);
  SysVars::inc_events_written();

  return 0;
}

bool AuditLogFilter::on_audit_rule_flush_requested() noexcept {
  const bool is_flushed = m_audit_rules_registry->load();

  DBUG_EXECUTE_IF("audit_log_filter_rotate_after_audit_rules_flush",
                  { m_log_writer->rotate(); });

  return is_flushed;
}

void AuditLogFilter::on_audit_log_flush_requested() noexcept {
  m_log_writer->flush();
}

void AuditLogFilter::on_audit_log_prune_requested() noexcept {
  m_log_writer->prune();
}

void AuditLogFilter::on_audit_log_rotated() noexcept { m_log_reader->init(); }

void AuditLogFilter::get_connection_attrs(MYSQL_THD thd,
                                          AuditRecordVariant &audit_record) {
  my_service<SERVICE_TYPE(mysql_connection_attributes_iterator)> attrs_service(
      "mysql_connection_attributes_iterator", m_comp_registry_srv.get());

  if (!attrs_service.is_valid()) {
    return;
  }

  my_h_connection_attributes_iterator iterator;
  MYSQL_LEX_CSTRING attr_name{nullptr, 0};
  MYSQL_LEX_CSTRING attr_value{nullptr, 0};
  const char *charset_string = nullptr;

  if (attrs_service->init(thd, &iterator)) {
    return;
  }

  auto &info =
      std::visit([](auto &rec) -> ExtendedInfo & { return rec.extended_info; },
                 audit_record);

  while (!attrs_service->get(thd, &iterator, &attr_name.str, &attr_name.length,
                             &attr_value.str, &attr_value.length,
                             &charset_string)) {
    info.attrs.emplace(std::string{attr_name.str, attr_name.length},
                       std::string{attr_value.str, attr_value.length});
  }

  attrs_service->deinit(iterator);
}

bool AuditLogFilter::get_connection_user(
    MYSQL_THD thd, std::string &user_name, std::string &user_host) noexcept {
  my_service<SERVICE_TYPE(mysql_thd_security_context)> security_context_service(
      "mysql_thd_security_context", m_comp_registry_srv.get());
  my_service<SERVICE_TYPE(mysql_security_context_options)>
      security_context_opts_service(
        "mysql_security_context_options", m_comp_registry_srv.get());

  if (!security_context_service.is_valid() ||
      !security_context_opts_service.is_valid()) {
    return false;
  }

  Security_context_handle ctx;

  if (security_context_service->get(thd, &ctx)) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                 "Can not get security context");
    return false;
  }

  MYSQL_LEX_CSTRING user{"", 0};
  MYSQL_LEX_CSTRING host{"", 0};

  if (security_context_opts_service->get(ctx, "priv_user", &user)) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                 "Can not get user name from security context");
    return false;
  }

  if (security_context_opts_service->get(ctx, "priv_host", &host)) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                 "Can not get user host from security context");
    return false;
  }

  if (user.length == 0 || host.length == 0) {
    return false;
  }

  user_name = user.str;
  user_host = host.str;

  return true;
}

comp_registry_srv_t *AuditLogFilter::get_comp_registry_srv() noexcept {
  return m_comp_registry_srv.get();
}

AuditLogReader *AuditLogFilter::get_log_reader() noexcept {
  return m_log_reader.get();
}

}  // namespace audit_log_filter

static void MY_ATTRIBUTE((constructor)) audit_log_filter_so_init() noexcept {}

/**
 * @brief Plugin type-specific descriptor
 */
static st_mysql_audit audit_log_descriptor = {
    MYSQL_AUDIT_INTERFACE_VERSION,      /* interface version    */
    nullptr,                            /* release_thd function */
    audit_log_filter::audit_log_notify, /* notify function      */
    {                                   /* class mask           */
     static_cast<unsigned long>(MYSQL_AUDIT_GENERAL_ALL),
     static_cast<unsigned long>(MYSQL_AUDIT_CONNECTION_ALL),
     static_cast<unsigned long>(MYSQL_AUDIT_PARSE_ALL), 0,
     static_cast<unsigned long>(MYSQL_AUDIT_TABLE_ACCESS_ALL),
     static_cast<unsigned long>(MYSQL_AUDIT_GLOBAL_VARIABLE_ALL),
     static_cast<unsigned long>(MYSQL_AUDIT_SERVER_STARTUP_ALL),
     static_cast<unsigned long>(MYSQL_AUDIT_SERVER_SHUTDOWN_ALL),
     static_cast<unsigned long>(MYSQL_AUDIT_COMMAND_ALL),
     static_cast<unsigned long>(MYSQL_AUDIT_QUERY_ALL),
     static_cast<unsigned long>(MYSQL_AUDIT_STORED_PROGRAM_ALL),
     static_cast<unsigned long>(MYSQL_AUDIT_AUTHENTICATION_ALL),
     static_cast<unsigned long>(MYSQL_AUDIT_MESSAGE_ALL)}};

/**
 * @brief Plugin library descriptor
 */
mysql_declare_plugin(audit_log){
    MYSQL_AUDIT_PLUGIN,                   /* type                     */
    &audit_log_descriptor,                /* descriptor               */
    "audit_log_filter",                   /* name                     */
    "Percona LLC and/or its affiliates.", /* author                   */
    "Audit log",                          /* description              */
    PLUGIN_LICENSE_GPL,
    audit_log_filter::audit_log_filter_init, /* init function            */
    nullptr,
    audit_log_filter::audit_log_filter_deinit, /* deinit function          */
    PLUGIN_VERSION,                            /* version                  */
    audit_log_filter::SysVars::get_status_var_defs(),
    audit_log_filter::SysVars::get_sys_var_defs(),
    nullptr,
    0,
} mysql_declare_plugin_end;
