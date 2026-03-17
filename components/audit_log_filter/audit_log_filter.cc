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

#define ALLOW_COMPONENT_INCLUDE // for my_io.h and plugin.h
#include "components/audit_log_filter/audit_log_filter.h"
#include "components/audit_log_filter/audit_filter.h"
#include "components/audit_log_filter/audit_keyring.h"
#include "components/audit_log_filter/audit_log_reader.h"
#include "components/audit_log_filter/audit_psi_info.h"
#include "components/audit_log_filter/audit_rule_registry.h"
#include "components/audit_log_filter/audit_udf.h"
#include "components/audit_log_filter/log_record_formatter.h"
#include "components/audit_log_filter/log_writer.h"
#include "components/audit_log_filter/log_writer/file_handle.h"
#include "components/audit_log_filter/sys_vars.h"

#include <mysql/components/component_implementation.h>
#include <mysql/components/service_implementation.h>

#include <mysql/components/services/mysql_connection_attributes_iterator.h>
#include <mysql/components/services/mysql_thd_attributes.h>

#include <mysql/components/services/event_tracking_authentication_service.h>
#include <mysql/components/services/event_tracking_command_service.h>
#include <mysql/components/services/event_tracking_connection_service.h>
#include <mysql/components/services/event_tracking_general_service.h>
#include <mysql/components/services/event_tracking_global_variable_service.h>
#include <mysql/components/services/event_tracking_lifecycle_service.h>
#include <mysql/components/services/event_tracking_message_service.h>
#include <mysql/components/services/event_tracking_parse_service.h>
#include <mysql/components/services/event_tracking_query_service.h>
#include <mysql/components/services/event_tracking_stored_program_service.h>
#include <mysql/components/services/event_tracking_table_access_service.h>

#include <mysql/psi/mysql_memory.h>

#include "sql/mysqld.h"
#include "sql/sql_class.h"

#include <scope_guard.h>
#include <array>
#include <exception>
#include <memory>
#include <variant>

#ifdef WIN32
#define PLUGIN_EXPORT extern "C" __declspec(dllexport)
#else
#define PLUGIN_EXPORT extern "C"
#endif

/** Dependencies */
REQUIRES_SERVICE_PLACEHOLDER(log_builtins);
REQUIRES_SERVICE_PLACEHOLDER(log_builtins_string);
REQUIRES_SERVICE_PLACEHOLDER(mysql_current_thread_reader);
REQUIRES_SERVICE_PLACEHOLDER(mysql_thd_attributes);
REQUIRES_SERVICE_PLACEHOLDER(mysql_string_get_data_in_charset);
REQUIRES_SERVICE_PLACEHOLDER(mysql_string_factory);
REQUIRES_PSI_MEMORY_SERVICE_PLACEHOLDER;

SERVICE_TYPE(log_builtins) *log_bi = nullptr;
SERVICE_TYPE(log_builtins_string) *log_bs = nullptr;

namespace audit_log_filter {
namespace {

std::unique_ptr<AuditLogFilter> audit_log_filter;

class EventsConsumer {
 public:
  static mysql_service_status_t notify(
      const mysql_event_tracking_authentication_data *event_data) {
    return audit_log_filter->notify_event(
        audit_event_class_t::AUDIT_AUTHENTICATION_CLASS,
        static_cast<const void *>(event_data));
  }
  static mysql_service_status_t notify(
      const mysql_event_tracking_command_data *event_data) {
    return audit_log_filter->notify_event(
        audit_event_class_t::AUDIT_COMMAND_CLASS,
        static_cast<const void *>(event_data));
  }
  static mysql_service_status_t notify(
      const mysql_event_tracking_connection_data *event_data) {
    return audit_log_filter->notify_event(
        audit_event_class_t::AUDIT_CONNECTION_CLASS,
        static_cast<const void *>(event_data));
  }
  static mysql_service_status_t notify(
      const mysql_event_tracking_general_data *event_data) {
    return audit_log_filter->notify_event(
        audit_event_class_t::AUDIT_GENERAL_CLASS,
        static_cast<const void *>(event_data));
  }
  static mysql_service_status_t notify(
      const mysql_event_tracking_global_variable_data *event_data) {
    return audit_log_filter->notify_event(
        audit_event_class_t::AUDIT_GLOBAL_VARIABLE_CLASS,
        static_cast<const void *>(event_data));
  }
  static mysql_service_status_t notify(
      const mysql_event_tracking_startup_data *event_data) {
    return audit_log_filter->notify_event(
        audit_event_class_t::AUDIT_SERVER_STARTUP_CLASS,
        static_cast<const void *>(event_data));
  }
  static mysql_service_status_t notify(
      const mysql_event_tracking_shutdown_data *event_data) {
    return audit_log_filter->notify_event(
        audit_event_class_t::AUDIT_SERVER_SHUTDOWN_CLASS,
        static_cast<const void *>(event_data));
  }
  static mysql_service_status_t notify(
      const mysql_event_tracking_message_data *event_data) {
    return audit_log_filter->notify_event(
        audit_event_class_t::AUDIT_MESSAGE_CLASS,
        static_cast<const void *>(event_data));
  }
  static mysql_service_status_t notify(
      mysql_event_tracking_parse_data *event_data) {
    return audit_log_filter->notify_event(
        audit_event_class_t::AUDIT_PARSE_CLASS,
        static_cast<const void *>(event_data));
  }
  static mysql_service_status_t notify(
      const mysql_event_tracking_query_data *event_data) {
    return audit_log_filter->notify_event(
        audit_event_class_t::AUDIT_QUERY_CLASS,
        static_cast<const void *>(event_data));
  }
  static mysql_service_status_t notify(
      const mysql_event_tracking_stored_program_data *event_data) {
    return audit_log_filter->notify_event(
        audit_event_class_t::AUDIT_STORED_PROGRAM_CLASS,
        static_cast<const void *>(event_data));
  }
  static mysql_service_status_t notify(
      const mysql_event_tracking_table_access_data *event_data) {
    return audit_log_filter->notify_event(
        audit_event_class_t::AUDIT_TABLE_ACCESS_CLASS,
        static_cast<const void *>(event_data));
  }
};

bool init_abort_exempt_privilege() {
  my_service<SERVICE_TYPE(dynamic_privilege_register)> reg_priv_srv(
      "dynamic_privilege_register", SysVars::get_comp_registry_srv());

  if (reg_priv_srv.is_valid() &&
      reg_priv_srv->register_privilege(STRING_WITH_LEN("AUDIT_ABORT_EXEMPT")) ==
          0) {
    return true;
  }

  return false;
}

void deinit_abort_exempt_privilege() {
  my_service<SERVICE_TYPE(dynamic_privilege_register)> reg_priv_srv(
      "dynamic_privilege_register", SysVars::get_comp_registry_srv());

  if (reg_priv_srv.is_valid()) {
    reg_priv_srv->unregister_privilege(STRING_WITH_LEN("AUDIT_ABORT_EXEMPT"));
  }
}

}  // namespace

AuditLogFilter *get_audit_log_filter_instance() noexcept {
  return audit_log_filter.get();
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

#define DECLARE_AUDIT_UDF_STR_FUNC(NAME)                                       \
  PLUGIN_EXPORT                                                                \
  char *NAME##_udf(UDF_INIT *initid, UDF_ARGS *args, char *result,             \
                   unsigned long *length, unsigned char *is_null,              \
                   unsigned char *error) {                                     \
    return audit_log_filter::AuditUdf::NAME##_udf(audit_log_filter->get_udf(), \
                                                  initid, args, result,        \
                                                  length, is_null, error);     \
  }

#define DECLARE_AUDIT_UDF_INT_FUNC(NAME)                               \
  PLUGIN_EXPORT                                                        \
  long long NAME##_udf(UDF_INIT *initid, UDF_ARGS *args,               \
                       unsigned char *is_null, unsigned char *error) { \
    return audit_log_filter::AuditUdf::NAME##_udf(                     \
        audit_log_filter->get_udf(), initid, args, is_null, error);    \
  }

#define DECLARE_AUDIT_UDF_DEINIT(NAME)                     \
  PLUGIN_EXPORT                                            \
  void NAME##_udf_deinit(UDF_INIT *initid) {               \
    audit_log_filter::AuditUdf::NAME##_udf_deinit(initid); \
  }

#define DECLARE_AUDIT_STR_UDF(NAME) \
  DECLARE_AUDIT_UDF_INIT(NAME)      \
  DECLARE_AUDIT_UDF_STR_FUNC(NAME)  \
  DECLARE_AUDIT_UDF_DEINIT(NAME)

#define DECLARE_AUDIT_INT_UDF(NAME) \
  DECLARE_AUDIT_UDF_INIT(NAME)      \
  DECLARE_AUDIT_UDF_INT_FUNC(NAME)  \
  DECLARE_AUDIT_UDF_DEINIT(NAME)

DECLARE_AUDIT_STR_UDF(audit_log_filter_set_filter)
DECLARE_AUDIT_STR_UDF(audit_log_filter_remove_filter)
DECLARE_AUDIT_STR_UDF(audit_log_filter_set_user)
DECLARE_AUDIT_STR_UDF(audit_log_filter_remove_user)
DECLARE_AUDIT_STR_UDF(audit_log_filter_flush)
DECLARE_AUDIT_STR_UDF(audit_log_read)
DECLARE_AUDIT_STR_UDF(audit_log_read_bookmark)
DECLARE_AUDIT_STR_UDF(audit_log_rotate)
DECLARE_AUDIT_STR_UDF(audit_log_encryption_password_get)
DECLARE_AUDIT_STR_UDF(audit_log_encryption_password_set)
DECLARE_AUDIT_INT_UDF(audit_log_session_filter_id)

#define DECLARE_AUDIT_STR_UDF_INFO(NAME)                          \
  UdfFuncInfo {                                                   \
#NAME, STRING_RESULT, &NAME##_udf, nullptr, &NAME##_udf_init, \
        &NAME##_udf_deinit                                        \
  }
#define DECLARE_AUDIT_INT_UDF_INFO(NAME)                       \
  UdfFuncInfo {                                                \
#NAME, INT_RESULT, nullptr, &NAME##_udf, &NAME##_udf_init, \
        &NAME##_udf_deinit                                     \
  }

static std::array udfs_list{
    DECLARE_AUDIT_STR_UDF_INFO(audit_log_filter_set_filter),
    DECLARE_AUDIT_STR_UDF_INFO(audit_log_filter_remove_filter),
    DECLARE_AUDIT_STR_UDF_INFO(audit_log_filter_set_user),
    DECLARE_AUDIT_STR_UDF_INFO(audit_log_filter_remove_user),
    DECLARE_AUDIT_STR_UDF_INFO(audit_log_filter_flush),
    DECLARE_AUDIT_STR_UDF_INFO(audit_log_read),
    DECLARE_AUDIT_STR_UDF_INFO(audit_log_read_bookmark),
    DECLARE_AUDIT_STR_UDF_INFO(audit_log_rotate),
    DECLARE_AUDIT_STR_UDF_INFO(audit_log_encryption_password_get),
    DECLARE_AUDIT_STR_UDF_INFO(audit_log_encryption_password_set),
    DECLARE_AUDIT_INT_UDF_INFO(audit_log_session_filter_id)};

/**
 * @brief Initialize the component at server start or component installation.
 *
 * @return Initialization status, 0 in case of success or non zero
 *         code otherwise
 */
mysql_service_status_t audit_log_filter_init() try {
  auto *all_mem_info = get_all_memory_info();
  mysql_memory_register(AUDIT_LOG_FILTER_PSI_CATEGORY, all_mem_info,
                        sizeof(*all_mem_info) / sizeof(PSI_memory_info));

  log_bi = mysql_service_log_builtins;
  log_bs = mysql_service_log_builtins_string;

  const auto *comp_registry_srv = SysVars::acquire_comp_registry_srv();

  auto comp_scope_guard = create_scope_guard([&] {
    if (comp_registry_srv != nullptr) {
      SysVars::deinit();
      SysVars::release_comp_registry_srv();
    }
  });

  LogComponentErr(INFORMATION_LEVEL, ER_AUDIT_INIT_STARTED);

  if (!SysVars::init() || !SysVars::validate()) {
    return 1;
  }

  auto is_keyring_initialized = audit_keyring::check_keyring_initialized();

  if (is_keyring_initialized &&
      !audit_keyring::check_generate_initial_encryption_options()) {
    LogComponentErr(ERROR_LEVEL, ER_AUDIT_INIT_ENCRYPTION_PASSWORD_FAILURE);
    return 1;
  }

  SysVars::set_log_encryption_enabled(is_keyring_initialized &&
                                      SysVars::get_encryption_type() !=
                                          AuditLogEncryptionType::None);

  if (!init_abort_exempt_privilege()) {
    LogComponentErr(ERROR_LEVEL, ER_AUDIT_INIT_PRIV_FAILURE);
    return 1;
  }

  auto audit_udf = std::make_unique<AuditUdf>();

  if (audit_udf == nullptr) {
    LogComponentErr(ERROR_LEVEL, ER_AUDIT_INIT_UDF_CREATE_FAILURE);
    return 1;
  }

  if (!audit_udf->init(udfs_list.begin(), udfs_list.end())) {
    LogComponentErr(ERROR_LEVEL, ER_AUDIT_INIT_UDF_INIT_FAILURE);
    return 1;
  }

  auto audit_rule_registry = std::make_unique<AuditRuleRegistry>();

  if (audit_rule_registry == nullptr) {
    LogComponentErr(ERROR_LEVEL, ER_AUDIT_INIT_FILTERS_INIT_FAILURE);
    return 1;
  }

  auto formatter = get_log_record_formatter(SysVars::get_format_type());

  if (formatter == nullptr) {
    LogComponentErr(ERROR_LEVEL, ER_AUDIT_INIT_FORMATTER_INIT_FAILURE);
    return 1;
  }

  auto log_writer = get_log_writer(std::move(formatter));

  if (log_writer == nullptr || !log_writer->init()) {
    LogComponentErr(ERROR_LEVEL, ER_AUDIT_INIT_WRITER_INIT_FAILURE);
    return 1;
  }

  if (!log_writer->open()) {
    char errbuf[MYSYS_STRERROR_SIZE];
    my_strerror(errbuf, sizeof(errbuf), errno);
    LogComponentErr(ERROR_LEVEL, ER_AUDIT_INIT_WRITER_OPEN_FAILURE, errbuf);
    return 1;
  }

  auto log_reader = std::make_unique<AuditLogReader>();

  if (log_reader == nullptr) {
    LogComponentErr(ERROR_LEVEL, ER_AUDIT_INIT_READER_INIT_FAILURE);
    return 1;
  }

  log_reader->reset();

  auto local_audit_log_filter = std::make_unique<AuditLogFilter>(
      std::move(audit_rule_registry), std::move(audit_udf),
      std::move(log_writer), std::move(log_reader));

  if (!local_audit_log_filter->init()) {
    LogComponentErr(ERROR_LEVEL, ER_AUDIT_INIT_FAILURE);
    return 1;
  }

  if (SysVars::get_log_disabled()) {
    LogComponentErr(WARNING_LEVEL, ER_AUDIT_INIT_DISABLED_WARN);
  } else {
    local_audit_log_filter->send_audit_start_event();
  }

  // Disarm the scope guard only after all potentially-throwing operations
  // succeeded, so that SysVars cleanup still runs on exception.
  comp_registry_srv = nullptr;
  audit_log_filter = std::move(local_audit_log_filter);
  return 0;
} catch (const std::exception &e) {
  LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                  "Exception in audit_log_filter_init: %s", e.what());
  return 1;
} catch (...) {
  LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                  "Unknown exception in audit_log_filter_init");
  return 1;
}

/**
 * @brief Terminate the component at server shutdown or component
 *        deinstallation.
 *
 * @return Plugin deinit status, 0 in case of success or non zero
 *         code otherwise
 */
mysql_service_status_t audit_log_filter_deinit() try {
  if (audit_log_filter == nullptr) {
    return 0;
  }

  audit_log_filter->send_audit_stop_event();
  audit_log_filter->deinit();

  deinit_abort_exempt_privilege();

  LogComponentErr(INFORMATION_LEVEL, ER_AUDIT_DEINIT_DONE);

  SysVars::deinit();
  SysVars::release_comp_registry_srv();

  audit_log_filter.reset();

  return 0;
} catch (const std::exception &e) {
  LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                  "Exception in audit_log_filter_deinit: %s", e.what());
  return 1;
} catch (...) {
  LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                  "Unknown exception in audit_log_filter_deinit");
  return 1;
}

AuditLogFilter::AuditLogFilter(
    std::unique_ptr<AuditRuleRegistry> audit_rules_registry,
    std::unique_ptr<AuditUdf> audit_udf,
    std::unique_ptr<log_writer::LogWriterBase> log_writer,
    std::unique_ptr<AuditLogReader> log_reader)
    : m_audit_rules_registry{std::move(audit_rules_registry)},
      m_audit_udf{std::move(audit_udf)},
      m_log_writer{std::move(log_writer)},
      m_log_reader{std::move(log_reader)},
      m_is_active{true} {}

bool AuditLogFilter::init() noexcept {
  const auto *reg_srv = SysVars::get_comp_registry_srv();

  if (reg_srv->acquire(
          "mysql_thd_security_context",
          reinterpret_cast<my_h_service *>(
              const_cast<SERVICE_TYPE_NO_CONST(mysql_thd_security_context) **>(
                  &m_security_context_srv))) == 1 ||
      reg_srv->acquire("mysql_security_context_options",
                       reinterpret_cast<my_h_service *>(
                           const_cast<SERVICE_TYPE_NO_CONST(
                               mysql_security_context_options) **>(
                               &m_security_context_opts_srv))) == 1 ||
      reg_srv->acquire(
          "global_grants_check",
          reinterpret_cast<my_h_service *>(
              const_cast<SERVICE_TYPE_NO_CONST(global_grants_check) **>(
                  &m_grants_check_srv))) == 1) {
    return false;
  }

  return m_security_context_srv != nullptr &&
         m_security_context_opts_srv != nullptr &&
         m_grants_check_srv != nullptr;
}

void AuditLogFilter::deinit() noexcept {
  m_is_active = false;
  m_audit_udf->deinit();
  m_log_writer->close();

  const auto *reg_srv = SysVars::get_comp_registry_srv();
  reg_srv->release(reinterpret_cast<my_h_service>(
      const_cast<SERVICE_TYPE_NO_CONST(mysql_thd_security_context) *>(
          m_security_context_srv)));
  reg_srv->release(reinterpret_cast<my_h_service>(
      const_cast<SERVICE_TYPE_NO_CONST(mysql_security_context_options) *>(
          m_security_context_opts_srv)));
  reg_srv->release(reinterpret_cast<my_h_service>(
      const_cast<SERVICE_TYPE_NO_CONST(global_grants_check) *>(
          m_grants_check_srv)));
}

int AuditLogFilter::notify_event(audit_event_class_t event_class,
                                 const void *event_data) try {
  if (SysVars::get_log_disabled() || !m_is_active) {
    return 0;
  }

  if (event_class == audit_event_class_t::AUDIT_SERVER_STARTUP_CLASS ||
      event_class == audit_event_class_t::AUDIT_SERVER_SHUTDOWN_CLASS) {
    return 0;
  }

  MYSQL_THD thd = nullptr;
  if (mysql_service_mysql_current_thread_reader->get(&thd) == 1 ||
      thd == nullptr) {
    return 0;
  }

  SysVars::inc_events_total();

  Security_context_handle sctx{};
  std::string user_name;
  std::string user_host;

  if (!get_security_context(thd, &sctx) ||
      !get_connection_user(sctx, user_name, user_host)) {
    return 0;
  }

  // Get connection specific filtering rule
  std::string rule_name;

  if (!m_audit_rules_registry->lookup_rule_name(user_name, user_host,
                                                rule_name)) {
    SysVars::set_session_filter_id(thd, 0);
    return 0;
  }

  auto filter_rule = m_audit_rules_registry->get_rule(rule_name);

  if (filter_rule == nullptr) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to find '%s' filtering rule", rule_name.c_str());
    return 0;
  }

  SysVars::set_session_filter_id(thd, filter_rule->get_filter_id());

  // Get actual event info based on event class
  auto audit_record = get_audit_record(event_class, event_data);
  if (!audit_record.has_value()) {
    return 0;
  }
  auto &record = *audit_record;

  if (std::holds_alternative<AuditRecordUnknown>(record)) {
    LogComponentErr(WARNING_LEVEL, ER_LOG_PRINTF_MSG,
                    "Unsupported audit event class with ID %i received",
                    event_class);
    return 0;
  }

  if (auto rec = std::get_if<AuditRecordGeneral>(&record)) {
    set_extended_info(thd, sctx, *rec);
  }

  if (auto rec = std::get_if<AuditRecordTableAccess>(&record)) {
    set_extended_info(thd, sctx, *rec);
  }

  // Apply filtering rule
  AuditAction filter_result =
      AuditEventFilter::apply(filter_rule.get(), record);

  if (filter_result == AuditAction::Skip) {
    SysVars::inc_events_filtered();
    return 0;
  }

  if (filter_result == AuditAction::Block &&
      !check_abort_exempt_privilege(sctx)) {
    auto ev_name = std::visit(
        [](const auto &rec) -> std::string_view {
          return rec.event_class_name;
        },
        record);
    LogComponentErr(INFORMATION_LEVEL, ER_LOG_PRINTF_MSG,
                    "Blocked audit event '%s' with class %i", ev_name.data(),
                    event_class);
    return 1;
  }

  if (event_class == audit_event_class_t::AUDIT_CONNECTION_CLASS) {
    get_connection_attrs(thd, record);
  }

  try {
    m_log_writer->write(record);
  } catch (...) {
    SysVars::inc_events_lost();
    throw;
  }
  SysVars::inc_events_written();

  return 0;
} catch (const std::exception &e) {
  LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                  "Caught exception in notify_event: %s", e.what());
  return 0;
} catch (...) {
  LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                  "Caught unknown exception in notify_event");
  return 0;
}

void AuditLogFilter::send_audit_start_event() noexcept try {
  auto event = internal_event_tracking_audit_data{
      INTERNAL_EVENT_TRACKING_AUDIT_AUDIT, static_cast<uint32>(::server_id), 0};

  MYSQL_THD thd = nullptr;
  if (mysql_service_mysql_current_thread_reader->get(&thd) == 0 &&
      thd != nullptr) {
    event.connection_id = static_cast<unsigned long>(thd->thread_id());
  }

  auto audit_record =
      get_audit_record(audit_event_class_t::AUDIT_INTERNAL_AUDIT_CLASS, &event);
  if (!audit_record.has_value()) {
    return;
  }

  Security_context_handle sctx{};
  if (thd != nullptr && m_security_context_srv != nullptr &&
      m_security_context_opts_srv != nullptr &&
      m_security_context_srv->get(thd, &sctx) == 0 && sctx != nullptr) {
    auto load_security_context_option = [this, &sctx](const char *name,
                                                      std::string &value) {
      MYSQL_LEX_CSTRING raw_value{"", 0};
      if (m_security_context_opts_srv->get(sctx, name, &raw_value) == 0 &&
          raw_value.str != nullptr && raw_value.length > 0) {
        value.assign(raw_value.str, raw_value.length);
      } else {
        value.clear();
      }
    };

    if (auto *record = std::get_if<AuditRecordAudit>(&*audit_record)) {
      load_security_context_option("user", record->extended_info.user);
      load_security_context_option("host", record->extended_info.host);
      load_security_context_option("ip", record->extended_info.ip);
      load_security_context_option("external_user",
                                   record->extended_info.external_user);
      load_security_context_option("proxy_user",
                                   record->extended_info.proxy_user);
    }
  }

  m_log_writer->write(*audit_record);
} catch (...) {
  LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                  "Caught exception in send_audit_start_event");
}

void AuditLogFilter::send_audit_stop_event() noexcept try {
  auto event =
      internal_event_tracking_audit_data{INTERNAL_EVENT_TRACKING_AUDIT_NOAUDIT,
                                         static_cast<uint32>(::server_id), 0};

  MYSQL_THD thd = nullptr;
  if (mysql_service_mysql_current_thread_reader->get(&thd) == 0 &&
      thd != nullptr) {
    event.connection_id = static_cast<unsigned long>(thd->thread_id());
  }

  auto audit_record =
      get_audit_record(audit_event_class_t::AUDIT_INTERNAL_AUDIT_CLASS, &event);
  if (!audit_record.has_value()) {
    return;
  }

  m_log_writer->write(*audit_record);
} catch (...) {
  LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                  "Caught exception in send_audit_stop_event");
}

bool AuditLogFilter::on_audit_rule_flush_requested() noexcept try {
  if (!m_is_active) {
    return false;
  }

  const bool is_flushed = m_audit_rules_registry->load();

  DBUG_EXECUTE_IF("audit_log_filter_rotate_after_audit_rules_flush",
                  { m_log_writer->rotate(nullptr); });

  return is_flushed;
} catch (...) {
  LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                  "Caught exception in on_audit_rule_flush_requested");
  return false;
}

void AuditLogFilter::on_audit_log_prune_requested() noexcept {
  if (m_is_active) {
    m_log_writer->prune();
  }
}

void AuditLogFilter::on_audit_log_rotate_requested(
    log_writer::FileRotationResult *result) noexcept {
  if (m_is_active) {
    m_log_writer->rotate(result);
    m_log_writer->prune();
  }
}

void AuditLogFilter::on_encryption_password_prune_requested() noexcept try {
  if (m_is_active && SysVars::get_password_history_keep_days() > 0 &&
      audit_keyring::check_keyring_initialized()) {
    std::vector<std::string> log_names;
    if (log_writer::FileHandle::get_log_names_list(
            SysVars::get_file_dir(), SysVars::get_file_name(), log_names)) {
      audit_keyring::prune_encryption_options(
          SysVars::get_password_history_keep_days(), log_names);
    }
  }
} catch (...) {
  LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                  "Caught exception in on_encryption_password_prune_requested");
}

void AuditLogFilter::on_audit_log_rotated() noexcept {
  if (m_is_active) {
    m_log_reader->reset();
  }
}

void AuditLogFilter::get_connection_attrs(MYSQL_THD thd,
                                          AuditRecordVariant &audit_record) {
  my_service<SERVICE_TYPE(mysql_connection_attributes_iterator)> attrs_service(
      "mysql_connection_attributes_iterator", SysVars::get_comp_registry_srv());

  if (!attrs_service.is_valid()) {
    return;
  }

  my_h_connection_attributes_iterator iterator = nullptr;
  MYSQL_LEX_CSTRING attr_name{nullptr, 0};
  MYSQL_LEX_CSTRING attr_value{nullptr, 0};
  const char *charset_string = nullptr;

  if (attrs_service->init(thd, &iterator) == 1) {
    return;
  }

  auto &info =
      std::visit([](auto &rec) -> ExtendedInfo & { return rec.extended_info; },
                 audit_record);

  info.attrs["connection_attributes"] = {};

  while (attrs_service->get(thd, &iterator, &attr_name.str, &attr_name.length,
                            &attr_value.str, &attr_value.length,
                            &charset_string) == 0) {
    info.attrs["connection_attributes"].emplace_back(
        std::string{attr_name.str, attr_name.length},
        std::string{attr_value.str, attr_value.length});
  }

  attrs_service->deinit(iterator);
}

bool AuditLogFilter::get_connection_user(Security_context_handle &ctx,
                                         std::string &user_name,
                                         std::string &user_host) noexcept try {
  MYSQL_LEX_CSTRING user{"", 0};
  MYSQL_LEX_CSTRING host{"", 0};

  if (m_security_context_opts_srv->get(ctx, "priv_user", &user) == 1) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Can not get user name from security context");
    return false;
  }

  if (m_security_context_opts_srv->get(ctx, "priv_host", &host) == 1) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Can not get user host from security context");
    return false;
  }

  if (user.length == 0) {
    user_name.clear();
  } else {
    user_name.assign(user.str, user.length);
  }
  if (host.length == 0) {
    user_host.clear();
  } else {
    user_host.assign(host.str, host.length);
  }

  return true;
} catch (...) {
  return false;
}

bool AuditLogFilter::check_abort_exempt_privilege(
    Security_context_handle &ctx) noexcept {
  bool has_system_user_grant =
      m_grants_check_srv->has_global_grant(ctx, STRING_WITH_LEN("SYSTEM_USER"));
  bool has_abort_exempt_grant = m_grants_check_srv->has_global_grant(
      ctx, STRING_WITH_LEN("AUDIT_ABORT_EXEMPT"));

  return has_system_user_grant && has_abort_exempt_grant;
}

bool AuditLogFilter::get_security_context(
    MYSQL_THD thd, Security_context_handle *ctx) noexcept {
  if (m_security_context_srv->get(thd, ctx) == 1) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Cannot get security context");
    return false;
  }

  return true;
}

bool AuditLogFilter::get_security_context_option(Security_context_handle &ctx,
                                                 const std::string &name,
                                                 std::string &value) noexcept
    try {
  MYSQL_LEX_CSTRING val{"", 0};
  // calls mysql_security_context_imp::get
  if (m_security_context_opts_srv->get(ctx, name.c_str(), &val) != 0) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Can not get %s from security context", name.c_str());
    return false;
  }

  if (val.length == 0 || val.str == nullptr) {
    value.clear();
  } else {
    value.assign(val.str, val.length);
  }

  return true;
} catch (...) {
  return false;
}

std::string AuditLogFilter::get_sql_text(MYSQL_THD thd) {
  if (!thd) return std::string();

  my_h_string hstr = nullptr;
  int rc = mysql_service_mysql_thd_attributes->get(thd, "sql_text", &hstr);

  if (rc != 0 || hstr == nullptr) return std::string();

  const char *raw_buffer = nullptr;
  size_t raw_length = 0;
  CHARSET_INFO_h raw_charset = nullptr;

  if (mysql_service_mysql_string_get_data_in_charset->get_data(
          hstr, &raw_buffer, &raw_length, &raw_charset)) {
    mysql_service_mysql_string_factory->destroy(hstr);
    return std::string();
  }

  std::string result(raw_buffer, raw_length);
  mysql_service_mysql_string_factory->destroy(hstr);
  return result;
}

bool AuditLogFilter::set_extended_info(MYSQL_THD thd,
                                       Security_context_handle sctx,
                                       AuditRecordGeneral &record) {
  if (!thd) return false;

  /** The mysql_event_tracking_general_data struct includes error_code,
      connection_id, user, host, and ip.
    Note that the user field is formatted as "root[root] @ localhost
    [127.0.0.1]", which is inconsistent with other event types. Therefore, we
    rely on user, host, and ip provided by the services. For
    AuditRecordGeneral we need to retrieve query, sql_command, command, user,
    host, ip, external_user, and proxy_user from the corresponding services. */
  auto &extra = record.extended_info;

  extra.query = get_sql_text(thd);

  mysql_cstring_with_length sql_command;
  if (!mysql_service_mysql_thd_attributes->get(thd, "sql_command",
                                               &sql_command)) {
    extra.sql_command = {sql_command.str, sql_command.length};
  }

  // required only for AuditRecordGeneral
  mysql_cstring_with_length command;
  if (!mysql_service_mysql_thd_attributes->get(thd, "command", &command)) {
    extra.command = {command.str, command.length};
  }

  if (!sctx) return false;

  get_security_context_option(sctx, "user", extra.user);
  get_security_context_option(sctx, "host", extra.host);
  get_security_context_option(sctx, "ip", extra.ip);
  get_security_context_option(sctx, "external_user", extra.external_user);
  get_security_context_option(sctx, "proxy_user", extra.proxy_user);

  return true;
}

bool AuditLogFilter::set_extended_info(MYSQL_THD thd,
                                       Security_context_handle sctx,
                                       AuditRecordTableAccess &record) {
  if (!thd) return false;

  /** The mysql_event_tracking_table_access_data struct contains only
    connection_id, table_database, and table_name. To obtain the other fields
    for AuditRecordTableAccess — query, sql_command, user, host, ip,
    external_user, and proxy_user — we need to retrieve them from the
    corresponding services. */
  auto &extra = record.extended_info;

  extra.query = get_sql_text(thd);

  mysql_cstring_with_length sql_command;
  if (!mysql_service_mysql_thd_attributes->get(thd, "sql_command",
                                               &sql_command)) {
    extra.sql_command = {sql_command.str, sql_command.length};
  }

  // required only for AuditRecordTableAccess
  uint32_t sql_command_id = 0;
  if (!mysql_service_mysql_thd_attributes->get(thd, "sql_command_id",
                                               &sql_command_id)) {
    extra.sql_command_id = static_cast<enum_sql_command>(sql_command_id);
  }

  if (!sctx) return false;

  get_security_context_option(sctx, "user", extra.user);
  get_security_context_option(sctx, "host", extra.host);
  get_security_context_option(sctx, "ip", extra.ip);
  get_security_context_option(sctx, "external_user", extra.external_user);
  get_security_context_option(sctx, "proxy_user", extra.proxy_user);

  return true;
}

AuditUdf *AuditLogFilter::get_udf() noexcept { return m_audit_udf.get(); }

AuditLogReader *AuditLogFilter::get_log_reader() noexcept {
  return m_log_reader.get();
}

}  // namespace audit_log_filter

BEGIN_SERVICE_IMPLEMENTATION(component_audit_log_filter,
                             event_tracking_authentication)
audit_log_filter::EventsConsumer::notify END_SERVICE_IMPLEMENTATION();
BEGIN_SERVICE_IMPLEMENTATION(component_audit_log_filter, event_tracking_command)
audit_log_filter::EventsConsumer::notify END_SERVICE_IMPLEMENTATION();
BEGIN_SERVICE_IMPLEMENTATION(component_audit_log_filter,
                             event_tracking_connection)
audit_log_filter::EventsConsumer::notify END_SERVICE_IMPLEMENTATION();
BEGIN_SERVICE_IMPLEMENTATION(component_audit_log_filter, event_tracking_general)
audit_log_filter::EventsConsumer::notify END_SERVICE_IMPLEMENTATION();
BEGIN_SERVICE_IMPLEMENTATION(component_audit_log_filter,
                             event_tracking_global_variable)
audit_log_filter::EventsConsumer::notify END_SERVICE_IMPLEMENTATION();
BEGIN_SERVICE_IMPLEMENTATION(component_audit_log_filter,
                             event_tracking_lifecycle)
audit_log_filter::EventsConsumer::notify,
    audit_log_filter::EventsConsumer::notify END_SERVICE_IMPLEMENTATION();
BEGIN_SERVICE_IMPLEMENTATION(component_audit_log_filter, event_tracking_message)
audit_log_filter::EventsConsumer::notify END_SERVICE_IMPLEMENTATION();
BEGIN_SERVICE_IMPLEMENTATION(component_audit_log_filter, event_tracking_parse)
audit_log_filter::EventsConsumer::notify END_SERVICE_IMPLEMENTATION();
BEGIN_SERVICE_IMPLEMENTATION(component_audit_log_filter, event_tracking_query)
audit_log_filter::EventsConsumer::notify END_SERVICE_IMPLEMENTATION();
BEGIN_SERVICE_IMPLEMENTATION(component_audit_log_filter,
                             event_tracking_stored_program)
audit_log_filter::EventsConsumer::notify END_SERVICE_IMPLEMENTATION();
BEGIN_SERVICE_IMPLEMENTATION(component_audit_log_filter,
                             event_tracking_table_access)
audit_log_filter::EventsConsumer::notify END_SERVICE_IMPLEMENTATION();

BEGIN_COMPONENT_PROVIDES(component_audit_log_filter)
PROVIDES_SERVICE(component_audit_log_filter, event_tracking_authentication),
    PROVIDES_SERVICE(component_audit_log_filter, event_tracking_command),
    PROVIDES_SERVICE(component_audit_log_filter, event_tracking_connection),
    PROVIDES_SERVICE(component_audit_log_filter, event_tracking_general),
    PROVIDES_SERVICE(component_audit_log_filter,
                     event_tracking_global_variable),
    PROVIDES_SERVICE(component_audit_log_filter, event_tracking_lifecycle),
    PROVIDES_SERVICE(component_audit_log_filter, event_tracking_message),
    PROVIDES_SERVICE(component_audit_log_filter, event_tracking_parse),
    PROVIDES_SERVICE(component_audit_log_filter, event_tracking_query),
    PROVIDES_SERVICE(component_audit_log_filter, event_tracking_stored_program),
    PROVIDES_SERVICE(component_audit_log_filter, event_tracking_table_access),
    END_COMPONENT_PROVIDES();

BEGIN_COMPONENT_REQUIRES(component_audit_log_filter)
REQUIRES_SERVICE(registry), REQUIRES_SERVICE(log_builtins),
    REQUIRES_SERVICE(log_builtins_string),
    REQUIRES_SERVICE(mysql_thd_attributes),
    REQUIRES_SERVICE(mysql_current_thread_reader),
    REQUIRES_SERVICE(mysql_string_get_data_in_charset),
    REQUIRES_SERVICE(mysql_string_factory), REQUIRES_PSI_MEMORY_SERVICE,
    END_COMPONENT_REQUIRES();

BEGIN_COMPONENT_METADATA(component_audit_log_filter)
METADATA("mysql.author", "Percona Corporation"),
    METADATA("mysql.license", "GPL"), END_COMPONENT_METADATA();

DECLARE_COMPONENT(component_audit_log_filter, "component_audit_log_filter")
audit_log_filter::audit_log_filter_init,
    audit_log_filter::audit_log_filter_deinit END_DECLARE_COMPONENT();

DECLARE_LIBRARY_COMPONENTS &COMPONENT_REF(component_audit_log_filter)
    END_DECLARE_LIBRARY_COMPONENTS
