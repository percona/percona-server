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

#include "base.h"
#include "plugin/audit_log_filter/audit_error_log.h"

namespace audit_log_filter::audit_table {
namespace {

const size_t kAccessedTableCount = 1;

}  // namespace

AuditTableBase::AuditTableBase(TableAccessServices *table_access_services)
    : m_table_access_services{table_access_services} {}

std::unique_ptr<TableAccessContext> AuditTableBase::open_table() noexcept {
  auto ta_context =
      std::make_unique<TableAccessContext>(m_table_access_services);

  if (ta_context == nullptr) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                 "Failed to init table access context");
    return nullptr;
  }

  m_table_access_services->get_current_thd_srv()->get(&ta_context->thd);

  ta_context->ta_session =
      m_table_access_services->get_ta_factory_srv()->create(
          ta_context->thd, kAccessedTableCount);
  if (ta_context->ta_session == nullptr) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                 "Failed to init table access service");
    return nullptr;
  }

  ta_context->table_ticket = m_table_access_services->get_ta_srv()->add(
      ta_context->ta_session, get_table_db_name(), strlen(get_table_db_name()),
      get_table_name(), strlen(get_table_name()), TA_WRITE);

  if (m_table_access_services->get_ta_srv()->begin(ta_context->ta_session) !=
      0) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                 "Failed to start table access transaction");
    return nullptr;
  }

  ta_context->ta_table = m_table_access_services->get_ta_srv()->get(
      ta_context->ta_session, ta_context->table_ticket);

  if (ta_context->ta_table == nullptr) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to get an opened %s table", get_table_name());
    return nullptr;
  }

  if (m_table_access_services->get_ta_srv()->check(
          ta_context->ta_session, ta_context->ta_table, get_table_field_def(),
          get_table_field_count()) != 0) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to check %s table fields", get_table_name());
    return nullptr;
  }

  return ta_context;
}

}  // namespace audit_log_filter::audit_table
