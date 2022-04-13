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

#include "plugin/audit_log_filter/audit_table/audit_log_filter.h"
#include "plugin/audit_log_filter/audit_error_log.h"

#include <memory>

namespace audit_log_filter::audit_table {
namespace {
inline constexpr const char *kAuditDbName = "mysql";
inline constexpr const char *kAuditFilterTableName = "audit_log_filter";

/*
 * The audit_log_filter table columns description
 */
const size_t kAuditLogFilterFilterId = 0;
const size_t kAuditLogFilterName = 1;
const size_t kAuditLogFilterFilter = 2;
const TA_table_field_def columns_audit_log_filter[] = {
    {kAuditLogFilterFilterId, "FILTER_ID", 9, TA_TYPE_INTEGER, false, 0},
    {kAuditLogFilterName, "NAME", 4, TA_TYPE_VARCHAR, false,
     kAuditFieldLengthFiltername},
    {kAuditLogFilterFilter, "FILTER", 6, TA_TYPE_JSON, false, 0}};

const size_t kAuditLogFilterFieldsCount = 3;

/*
 * Primary key info
 */
const TA_index_field_def key_filter_primary_cols[] = {{"FILTER_ID", 9, false}};
const size_t kKeyFilterPrimaryNumcol = 1;
const char *kKeyFilterPrimaryName = "PRIMARY";
const size_t kKeyFilterPrimaryNameLength = 7;

/*
 * Filter name key info
 */
const TA_index_field_def key_filter_name_cols[] = {{"NAME", 4, true}};
const size_t kKeyFilterNameNumcol = 1;
const char *kKeyFilterNameName = "FILTER_NAME";
const size_t kKeyFilterNameNameLength = 11;

}  // namespace

AuditLogFilter::AuditLogFilter(TableAccessServices *table_access_services)
    : AuditTableBase{table_access_services} {}

const char *AuditLogFilter::get_table_db_name() noexcept {
  return kAuditDbName;
}

const char *AuditLogFilter::get_table_name() noexcept {
  return kAuditFilterTableName;
}

const TA_table_field_def *AuditLogFilter::get_table_field_def() noexcept {
  return columns_audit_log_filter;
}

size_t AuditLogFilter::get_table_field_count() noexcept {
  return kAuditLogFilterFieldsCount;
}

TableResult AuditLogFilter::index_scan_locate_record_by_rule_name(
    TableAccessContext *ta_context, TA_key *key,
    const std::string &rule_name) noexcept {
  if (get_ta_srv()->get_ta_index_srv()->init(
          ta_context->ta_session, ta_context->ta_table, kKeyFilterNameName,
          kKeyFilterNameNameLength, key_filter_name_cols, kKeyFilterNameNumcol,
          key) != 0) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to init index scan of %s table", get_table_name());
    return TableResult::Fail;
  }

  CHARSET_INFO_h utf8 = get_ta_srv()->get_charset_srv()->get_utf8mb4();
  HStringContainer filter_name_value{get_ta_srv()->get_string_factory_srv()};
  get_ta_srv()->get_string_converter_srv()->convert_from_buffer(
      filter_name_value.get(), rule_name.c_str(), rule_name.length(), utf8);

  get_ta_srv()->get_fa_varchar_srv()->set(
      ta_context->ta_session, ta_context->ta_table, kAuditLogFilterName,
      filter_name_value.get());

  int rc = get_ta_srv()->get_ta_index_srv()->read_map(
      ta_context->ta_session, ta_context->ta_table, kKeyFilterNameNumcol, *key);

  return rc == 0 ? TableResult::Found : TableResult::NotFound;
}

void AuditLogFilter::index_scan_end(TableAccessContext *ta_context,
                                    TA_key key) noexcept {
  if (key != nullptr) {
    get_ta_srv()->get_ta_index_srv()->end(ta_context->ta_session,
                                          ta_context->ta_table, key);
  }
}

TableResult AuditLogFilter::get_next_pk_value(TableAccessContext *ta_context,
                                              long long &next_pk) noexcept {
  TA_key filter_id_key = nullptr;
  next_pk = 0;

  if (get_ta_srv()->get_ta_index_srv()->init(
          ta_context->ta_session, ta_context->ta_table, kKeyFilterPrimaryName,
          kKeyFilterPrimaryNameLength, key_filter_primary_cols,
          kKeyFilterPrimaryNumcol, &filter_id_key) != 0) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to init index scan of %s table", get_table_name());
    return TableResult::Fail;
  }

  int rc = get_ta_srv()->get_ta_index_srv()->first(
      ta_context->ta_session, ta_context->ta_table, filter_id_key);

  // TODO: Find an optimal way for determining next PK value
  while (rc == 0) {
    long long found_filter_id = 0;

    if (get_ta_srv()->get_fa_int_srv()->get(
            ta_context->ta_session, ta_context->ta_table,
            kAuditLogFilterFilterId, &found_filter_id)) {
      LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                      "Failed to read %s.filter_id", get_table_name());
      index_scan_end(ta_context, filter_id_key);
      return TableResult::Fail;
    }

    next_pk = found_filter_id + 1;

    rc = get_ta_srv()->get_ta_index_srv()->next(
        ta_context->ta_session, ta_context->ta_table, filter_id_key);
  }

  index_scan_end(ta_context, filter_id_key);

  return TableResult::Ok;
}

TableResult AuditLogFilter::load_filters(
    AuditRulesContainer &container) noexcept {
  container.clear();

  auto ta_context = open_table();

  if (ta_context == nullptr) {
    return TableResult::Fail;
  }

  if (get_ta_srv()->get_ta_scan_srv()->init(ta_context->ta_session,
                                            ta_context->ta_table)) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to init full scan of %s table", get_table_name());
    return TableResult::Fail;
  }

  CHARSET_INFO_h utf8 = get_ta_srv()->get_charset_srv()->get_utf8mb4();

  long long filter_id = 0;
  char buff_filter_name_value[kAuditFieldLengthFiltername + 1];
  char buff_filter_filter_value[kAuditFieldLengthFilter + 1];
  HStringContainer filter_name_value{get_ta_srv()->get_string_factory_srv()};
  HStringContainer filter_filter_value{get_ta_srv()->get_string_factory_srv()};

  while (true) {
    if (get_ta_srv()->get_ta_scan_srv()->next(ta_context->ta_session,
                                              ta_context->ta_table)) {
      LogPluginErrMsg(INFORMATION_LEVEL, ER_LOG_PRINTF_MSG,
                      "Nothing more to read from %s", get_table_name());
      break;
    }

    if (get_ta_srv()->get_fa_int_srv()->get(
            ta_context->ta_session, ta_context->ta_table,
            kAuditLogFilterFilterId, &filter_id)) {
      LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                      "Failed to read %s.filter_id", get_table_name());
      return TableResult::Fail;
    }
    if (get_ta_srv()->get_fa_varchar_srv()->get(
            ta_context->ta_session, ta_context->ta_table, kAuditLogFilterName,
            filter_name_value.get())) {
      LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                      "Failed to read %s.filter", get_table_name());
      return TableResult::Fail;
    }
    if (get_ta_srv()->get_fa_any_srv()->get(
            ta_context->ta_session, ta_context->ta_table, kAuditLogFilterFilter,
            filter_filter_value.get())) {
      LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                      "Failed to read %s.filter", get_table_name());
      return TableResult::Fail;
    }

    get_ta_srv()->get_string_converter_srv()->convert_to_buffer(
        filter_name_value.get(), buff_filter_name_value,
        sizeof(buff_filter_name_value), utf8);
    get_ta_srv()->get_string_converter_srv()->convert_to_buffer(
        filter_filter_value.get(), buff_filter_filter_value,
        sizeof(buff_filter_filter_value), utf8);

    LogPluginErrMsg(INFORMATION_LEVEL, ER_LOG_PRINTF_MSG,
                    "%s filter_id: %lld, name: %s, filter: %s",
                    get_table_name(), filter_id, buff_filter_name_value,
                    buff_filter_filter_value);

    AuditRule rule{static_cast<uint64_t>(filter_id), buff_filter_name_value,
                   buff_filter_filter_value};

    if (rule.check_valid()) {
      container.insert({buff_filter_name_value, std::move(rule)});
    } else {
      LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                      "audit_log_filter name: %s, filter: %s has wrong format",
                      buff_filter_name_value, buff_filter_filter_value);
    }
  }

  if (get_ta_srv()->get_ta_scan_srv()->end(ta_context->ta_session,
                                           ta_context->ta_table)) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to end full scan of %s table", get_table_name());
    return TableResult::Fail;
  }

  return TableResult::Ok;
}

TableResult AuditLogFilter::check_name_exists(
    const std::string &rule_name) noexcept {
  DBUG_EXECUTE_IF("udf_audit_log_filter_check_name_failure",
                  return TableResult::Fail;);

  auto ta_context = open_table();

  if (ta_context == nullptr) {
    return TableResult::Fail;
  }

  TA_key filter_name_key = nullptr;
  auto scan_result = index_scan_locate_record_by_rule_name(
      ta_context.get(), &filter_name_key, rule_name);

  if (scan_result != TableResult::Fail) {
    index_scan_end(ta_context.get(), filter_name_key);
  }

  return scan_result;
}

TableResult AuditLogFilter::insert_filter(
    const std::string &rule_name, const std::string &rule_definition) noexcept {
  DBUG_EXECUTE_IF("udf_audit_log_filter_insertion_failure",
                  return TableResult::Fail;);

  auto ta_context = open_table();

  if (ta_context == nullptr) {
    return TableResult::Fail;
  }

  long long next_id_value = 0;
  auto pk_result = get_next_pk_value(ta_context.get(), next_id_value);

  if (pk_result == TableResult::Fail) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to fetch next filter_id value");
    return TableResult::Fail;
  }

  CHARSET_INFO_h utf8 = get_ta_srv()->get_charset_srv()->get_utf8mb4();
  HStringContainer filter_name_value{get_ta_srv()->get_string_factory_srv()};
  HStringContainer filter_definition_value{
      get_ta_srv()->get_string_factory_srv()};
  get_ta_srv()->get_string_converter_srv()->convert_from_buffer(
      filter_name_value.get(), rule_name.c_str(), rule_name.length(), utf8);
  get_ta_srv()->get_string_converter_srv()->convert_from_buffer(
      filter_definition_value.get(), rule_definition.c_str(),
      rule_definition.length(), utf8);

  get_ta_srv()->get_fa_int_srv()->set(ta_context->ta_session,
                                      ta_context->ta_table,
                                      kAuditLogFilterFilterId, next_id_value);
  get_ta_srv()->get_fa_varchar_srv()->set(
      ta_context->ta_session, ta_context->ta_table, kAuditLogFilterName,
      filter_name_value.get());
  get_ta_srv()->get_fa_varchar_srv()->set(
      ta_context->ta_session, ta_context->ta_table, kAuditLogFilterFilter,
      filter_definition_value.get());

  int rc = get_ta_srv()->get_ta_update_srv()->insert(ta_context->ta_session,
                                                     ta_context->ta_table);

  if (rc != 0) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to insert filtering rule '%s', '%s'",
                    rule_name.c_str(), rule_definition.c_str());
    return TableResult::Fail;
  }

  if (get_ta_srv()->get_ta_srv()->commit(ta_context->ta_session)) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to insert filtering rule '%s', '%s', commit failed",
                    rule_name.c_str(), rule_definition.c_str());
    return TableResult::Fail;
  }

  return TableResult::Ok;
}

TableResult AuditLogFilter::delete_filter(
    const std::string &rule_name) noexcept {
  DBUG_EXECUTE_IF("udf_audit_log_filter_delete_filter_failure",
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

  auto rc = get_ta_srv()->get_ta_update_srv()->delete_row(
      ta_context->ta_session, ta_context->ta_table);

  if (rc != 0) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to delete filter with the name '%s'",
                    rule_name.c_str());
    index_scan_end(ta_context.get(), filter_name_key);
    return TableResult::Fail;
  }

  if (get_ta_srv()->get_ta_srv()->commit(ta_context->ta_session)) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to delete filter with the name '%s', commit failed",
                    rule_name.c_str());
    index_scan_end(ta_context.get(), filter_name_key);
    return TableResult::Fail;
  }

  index_scan_end(ta_context.get(), filter_name_key);

  return TableResult::Ok;
}

}  // namespace audit_log_filter::audit_table
