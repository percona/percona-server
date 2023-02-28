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

#include "plugin/audit_log_filter/audit_table/audit_log_user.h"
#include "plugin/audit_log_filter/audit_error_log.h"
#include "plugin/audit_log_filter/sys_vars.h"

#include "my_dbug.h"

namespace audit_log_filter::audit_table {
namespace {
inline constexpr const char *kAuditDbName = "mysql";
inline constexpr const char *kAuditUserTableName = "audit_log_user";

/*
 * The audit_log_user table columns description
 */
const size_t kAuditLogUserUsername = 0;
const size_t kAuditLogUserUserhost = 1;
const size_t kAuditLogUserFiltername = 2;
const TA_table_field_def columns_audit_log_user[] = {
    {kAuditLogUserUsername, "USERNAME", 8, TA_TYPE_VARCHAR, false,
     kAuditFieldLengthUsername},
    {kAuditLogUserUserhost, "USERHOST", 8, TA_TYPE_VARCHAR, false,
     kAuditFieldLengthUserhost},
    {kAuditLogUserFiltername, "FILTERNAME", 10, TA_TYPE_VARCHAR, false,
     kAuditFieldLengthFiltername}};
const size_t kAuditLogUserFieldsCount = 3;

const TA_index_field_def key_filter_name_cols[] = {
    {"FILTERNAME", 10, true}, {"USERNAME", 8, true}, {"USERHOST", 8, true}};
const size_t kKeyFilterNameNumcol = 3;
const char *kKeyFilterNameName = "FILTER_NAME";
const size_t kKeyFilterNameNameLength = 11;

const TA_index_field_def key_primary_cols[] = {{"USERNAME", 8, true},
                                               {"USERHOST", 8, true}};
const size_t kKeyPrimaryNumcol = 2;
const char *kKeyPrimaryName = "PRIMARY";
const size_t kKeyPrimaryLength = 7;

}  // namespace

const char *AuditLogUser::get_table_db_name() noexcept { return kAuditDbName; }

const char *AuditLogUser::get_table_name() noexcept {
  return kAuditUserTableName;
}

const TA_table_field_def *AuditLogUser::get_table_field_def() noexcept {
  return columns_audit_log_user;
}

size_t AuditLogUser::get_table_field_count() noexcept {
  return kAuditLogUserFieldsCount;
}

TableResult AuditLogUser::index_scan_locate_record_by_rule_name(
    TableAccessContext *ta_context, TA_key *key,
    const std::string &rule_name) noexcept {
  my_service<SERVICE_TYPE(table_access_index_v1)> index_srv(
      "table_access_index_v1", SysVars::get_comp_regystry_srv());
  my_service<SERVICE_TYPE(mysql_charset)> charset_srv(
      "mysql_charset", SysVars::get_comp_regystry_srv());
  my_service<SERVICE_TYPE(mysql_string_factory)> string_srv(
      "mysql_string_factory", SysVars::get_comp_regystry_srv());
  my_service<SERVICE_TYPE(mysql_string_charset_converter)> string_convert_srv(
      "mysql_string_charset_converter", SysVars::get_comp_regystry_srv());
  my_service<SERVICE_TYPE(field_varchar_access_v1)> varchar_srv(
      "field_varchar_access_v1", SysVars::get_comp_regystry_srv());

  if (index_srv->init(ta_context->ta_session, ta_context->ta_table,
                      kKeyFilterNameName, kKeyFilterNameNameLength,
                      key_filter_name_cols, kKeyFilterNameNumcol, key)) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to init index access of %s table",
                    get_table_name());
    return TableResult::Fail;
  }

  CHARSET_INFO_h utf8 = charset_srv->get_utf8mb4();
  HStringContainer filter_name_value{string_srv};
  string_convert_srv->convert_from_buffer(
      filter_name_value.get(), rule_name.c_str(), rule_name.length(), utf8);

  varchar_srv->set(ta_context->ta_session, ta_context->ta_table,
                   kAuditLogUserFiltername, filter_name_value.get());

  int rc = index_srv->read_map(ta_context->ta_session, ta_context->ta_table, 1,
                               *key);

  return rc == 0 ? TableResult::Found : TableResult::NotFound;
}

TableResult AuditLogUser::index_scan_locate_record_by_user_name_host(
    TableAccessContext *ta_context, TA_key *key, const std::string &user_name,
    const std::string &user_host) noexcept {
  my_service<SERVICE_TYPE(table_access_index_v1)> index_srv(
      "table_access_index_v1", SysVars::get_comp_regystry_srv());
  my_service<SERVICE_TYPE(mysql_charset)> charset_srv(
      "mysql_charset", SysVars::get_comp_regystry_srv());
  my_service<SERVICE_TYPE(mysql_string_factory)> string_srv(
      "mysql_string_factory", SysVars::get_comp_regystry_srv());
  my_service<SERVICE_TYPE(mysql_string_charset_converter)> string_convert_srv(
      "mysql_string_charset_converter", SysVars::get_comp_regystry_srv());
  my_service<SERVICE_TYPE(field_varchar_access_v1)> varchar_srv(
      "field_varchar_access_v1", SysVars::get_comp_regystry_srv());

  if (index_srv->init(ta_context->ta_session, ta_context->ta_table,
                      kKeyPrimaryName, kKeyPrimaryLength, key_primary_cols,
                      kKeyPrimaryNumcol, key)) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to init index access of %s table",
                    get_table_name());
    return TableResult::Fail;
  }

  CHARSET_INFO_h utf8 = charset_srv->get_utf8mb4();

  HStringContainer user_name_value{string_srv};
  string_convert_srv->convert_from_buffer(
      user_name_value.get(), user_name.c_str(), user_name.length(), utf8);

  HStringContainer user_host_value{string_srv};
  string_convert_srv->convert_from_buffer(
      user_host_value.get(), user_host.c_str(), user_host.length(), utf8);

  varchar_srv->set(ta_context->ta_session, ta_context->ta_table,
                   kAuditLogUserUsername, user_name_value.get());
  varchar_srv->set(ta_context->ta_session, ta_context->ta_table,
                   kAuditLogUserUserhost, user_host_value.get());

  int rc = index_srv->read_map(ta_context->ta_session, ta_context->ta_table,
                               kKeyPrimaryNumcol, *key);

  return rc == 0 ? TableResult::Found : TableResult::NotFound;
}

void AuditLogUser::index_scan_end(TableAccessContext *ta_context,
                                  TA_key key) noexcept {
  if (key != nullptr) {
    my_service<SERVICE_TYPE(table_access_index_v1)> index_srv(
        "table_access_index_v1", SysVars::get_comp_regystry_srv());
    index_srv->end(ta_context->ta_session, ta_context->ta_table, key);
  }
}

TableResult AuditLogUser::load_users(AuditUsersContainer &container) noexcept {
  container.clear();

  auto ta_context = open_table();

  if (ta_context == nullptr) {
    return TableResult::MissingTable;
  }

  my_service<SERVICE_TYPE(mysql_charset)> charset_srv(
      "mysql_charset", SysVars::get_comp_regystry_srv());
  my_service<SERVICE_TYPE(mysql_string_factory)> string_srv(
      "mysql_string_factory", SysVars::get_comp_regystry_srv());
  my_service<SERVICE_TYPE(mysql_string_charset_converter)> string_convert_srv(
      "mysql_string_charset_converter", SysVars::get_comp_regystry_srv());
  my_service<SERVICE_TYPE(field_varchar_access_v1)> varchar_srv(
      "field_varchar_access_v1", SysVars::get_comp_regystry_srv());
  my_service<SERVICE_TYPE(table_access_scan_v1)> scan_srv(
      "table_access_scan_v1", SysVars::get_comp_regystry_srv());

  if (scan_srv->init(ta_context->ta_session, ta_context->ta_table)) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to init full scan of %s table", get_table_name());
    return TableResult::Fail;
  }

  CHARSET_INFO_h utf8 = charset_srv->get_utf8mb4();

  char buff_user_name_value[kAuditFieldLengthUsername + 1];
  char buff_user_host_value[kAuditFieldLengthUserhost + 1];
  char buff_user_filter_name_value[kAuditFieldLengthFiltername + 1];
  HStringContainer user_name_value{string_srv};
  HStringContainer user_host_value{string_srv};
  HStringContainer user_filter_name_value{string_srv};

  while (true) {
    if (scan_srv->next(ta_context->ta_session, ta_context->ta_table)) {
      break;
    }

    if (varchar_srv->get(ta_context->ta_session, ta_context->ta_table,
                         kAuditLogUserUsername, user_name_value.get())) {
      LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                      "Failed to read %s.username", get_table_name());
      return TableResult::Fail;
    }
    if (varchar_srv->get(ta_context->ta_session, ta_context->ta_table,
                         kAuditLogUserUserhost, user_host_value.get())) {
      LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                      "Failed to read %s.userhost", get_table_name());
      return TableResult::Fail;
    }
    if (varchar_srv->get(ta_context->ta_session, ta_context->ta_table,
                         kAuditLogUserFiltername,
                         user_filter_name_value.get())) {
      LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                      "Failed to read %s.filtername", get_table_name());
      return TableResult::Fail;
    }

    string_convert_srv->convert_to_buffer(user_name_value.get(),
                                          buff_user_name_value,
                                          sizeof(buff_user_name_value), utf8);
    string_convert_srv->convert_to_buffer(user_host_value.get(),
                                          buff_user_host_value,
                                          sizeof(buff_user_host_value), utf8);
    string_convert_srv->convert_to_buffer(
        user_filter_name_value.get(), buff_user_filter_name_value,
        sizeof(buff_user_filter_name_value), utf8);

    container.insert({{buff_user_name_value, buff_user_host_value},
                      buff_user_filter_name_value});
  }

  if (scan_srv->end(ta_context->ta_session, ta_context->ta_table)) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to end full scan of %s table", get_table_name());
    return TableResult::Fail;
  }

  return TableResult::Ok;
}

TableResult AuditLogUser::delete_user_by_filter(
    const std::string &rule_name) noexcept {
  DBUG_EXECUTE_IF("udf_audit_log_user_delete_user_by_filter_failure",
                  return TableResult::Fail;);

  auto ta_context = open_table();

  if (ta_context == nullptr) {
    return TableResult::Fail;
  }

  TA_key filter_name_key = nullptr;
  auto scan_result = index_scan_locate_record_by_rule_name(
      ta_context.get(), &filter_name_key, rule_name);

  if (scan_result == TableResult::Fail) {
    return scan_result;
  }

  if (scan_result == TableResult::NotFound) {
    index_scan_end(ta_context.get(), filter_name_key);
    return TableResult::Ok;
  }

  my_service<SERVICE_TYPE(table_access_index_v1)> index_srv(
      "table_access_index_v1", SysVars::get_comp_regystry_srv());
  my_service<SERVICE_TYPE(table_access_update_v1)> table_update_srv(
      "table_access_update_v1", SysVars::get_comp_regystry_srv());
  my_service<SERVICE_TYPE(table_access_v1)> table_access_srv(
      "table_access_v1", SysVars::get_comp_regystry_srv());

  int rc = 0;
  while (rc == 0) {
    if (table_update_srv->delete_row(ta_context->ta_session,
                                     ta_context->ta_table) != 0) {
      LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                      "Failed to delete record for filter '%s'",
                      rule_name.c_str());
      index_scan_end(ta_context.get(), filter_name_key);
      return TableResult::Fail;
    }

    // Find next record
    rc = index_srv->next_same(ta_context->ta_session, ta_context->ta_table,
                              filter_name_key);
  }

  if (table_access_srv->commit(ta_context->ta_session)) {
    index_scan_end(ta_context.get(), filter_name_key);

    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to delete record for filter '%s', commit failed",
                    rule_name.c_str());
    return TableResult::Fail;
  }

  index_scan_end(ta_context.get(), filter_name_key);

  return TableResult::Ok;
}

TableResult AuditLogUser::delete_user_by_name_host(
    const std::string &user_name, const std::string &user_host) noexcept {
  DBUG_EXECUTE_IF("udf_audit_log_user_delete_user_by_name_host_failure",
                  return TableResult::Fail;);

  auto ta_context = open_table();

  if (ta_context == nullptr) {
    return TableResult::Fail;
  }

  TA_key user_host_key = nullptr;
  auto scan_result = index_scan_locate_record_by_user_name_host(
      ta_context.get(), &user_host_key, user_name, user_host);

  if (scan_result == TableResult::Fail) {
    return scan_result;
  }

  my_service<SERVICE_TYPE(table_access_update_v1)> table_update_srv(
      "table_access_update_v1", SysVars::get_comp_regystry_srv());
  my_service<SERVICE_TYPE(table_access_v1)> table_access_srv(
      "table_access_v1", SysVars::get_comp_regystry_srv());

  if (scan_result == TableResult::Found &&
      table_update_srv->delete_row(ta_context->ta_session,
                                   ta_context->ta_table) != 0) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to delete record for user '%s@%s'",
                    user_name.c_str(), user_host.c_str());
    index_scan_end(ta_context.get(), user_host_key);
    return TableResult::Fail;
  }

  if (table_access_srv->commit(ta_context->ta_session)) {
    index_scan_end(ta_context.get(), user_host_key);

    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to delete record for user '%s@%s', commit failed",
                    user_name.c_str(), user_host.c_str());
    return TableResult::Fail;
  }

  index_scan_end(ta_context.get(), user_host_key);

  return TableResult::Ok;
}

TableResult AuditLogUser::set_update_filter(
    const std::string &user_name, const std::string &user_host,
    const std::string &filter_name) noexcept {
  DBUG_EXECUTE_IF("udf_audit_log_user_set_update_filter_failure",
                  return TableResult::Fail;);

  auto ta_context = open_table();

  if (ta_context == nullptr) {
    return TableResult::Fail;
  }

  TA_key user_host_key = nullptr;
  auto scan_result = index_scan_locate_record_by_user_name_host(
      ta_context.get(), &user_host_key, user_name, user_host);

  if (scan_result == TableResult::Fail) {
    return scan_result;
  }

  my_service<SERVICE_TYPE(mysql_charset)> charset_srv(
      "mysql_charset", SysVars::get_comp_regystry_srv());
  my_service<SERVICE_TYPE(mysql_string_factory)> string_srv(
      "mysql_string_factory", SysVars::get_comp_regystry_srv());
  my_service<SERVICE_TYPE(mysql_string_charset_converter)> string_convert_srv(
      "mysql_string_charset_converter", SysVars::get_comp_regystry_srv());
  my_service<SERVICE_TYPE(field_varchar_access_v1)> varchar_srv(
      "field_varchar_access_v1", SysVars::get_comp_regystry_srv());
  my_service<SERVICE_TYPE(table_access_update_v1)> table_update_srv(
      "table_access_update_v1", SysVars::get_comp_regystry_srv());
  my_service<SERVICE_TYPE(table_access_v1)> table_access_srv(
      "table_access_v1", SysVars::get_comp_regystry_srv());

  CHARSET_INFO_h utf8 = charset_srv->get_utf8mb4();
  HStringContainer filter_name_value{string_srv};
  string_convert_srv->convert_from_buffer(
      filter_name_value.get(), filter_name.c_str(), filter_name.length(), utf8);
  varchar_srv->set(ta_context->ta_session, ta_context->ta_table,
                   kAuditLogUserFiltername, filter_name_value.get());

  int rc = 0;

  if (scan_result == TableResult::Found) {
    rc = table_update_srv->update(ta_context->ta_session, ta_context->ta_table);
  } else {
    rc = table_update_srv->insert(ta_context->ta_session, ta_context->ta_table);
  }

  if (rc != 0) {
    index_scan_end(ta_context.get(), user_host_key);

    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to assign '%s' filtering rule for user '%s@%s'",
                    filter_name.c_str(), user_name.c_str(), user_host.c_str());
    return TableResult::Fail;
  }

  if (table_access_srv->commit(ta_context->ta_session)) {
    index_scan_end(ta_context.get(), user_host_key);

    LogPluginErrMsg(
        ERROR_LEVEL, ER_LOG_PRINTF_MSG,
        "Failed to assign '%s' filtering rule for user '%s@%s', commit failed",
        filter_name.c_str(), user_name.c_str(), user_host.c_str());
    return TableResult::Fail;
  }

  index_scan_end(ta_context.get(), user_host_key);

  return TableResult::Ok;
}

}  // namespace audit_log_filter::audit_table
