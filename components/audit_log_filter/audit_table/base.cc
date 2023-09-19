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
#include "components/audit_log_filter/audit_error_log.h"
#include "components/audit_log_filter/sys_vars.h"

namespace audit_log_filter::audit_table {
namespace {

const size_t kAccessedTableCount = 1;

}  // namespace

AuditTableBase::AuditTableBase(std::string db_name)
    : m_db_name{std::move(db_name)} {}

TableAccessContext::~TableAccessContext() {
  ta_table = nullptr;
  table_ticket = 0;

  if (ta_session != nullptr) {
    my_service<SERVICE_TYPE(table_access_factory_v1)> ta_factory_srv(
        "table_access_factory_v1", SysVars::get_comp_registry_srv());
    ta_factory_srv->destroy(ta_session);
    ta_session = nullptr;
  }

  thd = nullptr;
}

std::unique_ptr<TableAccessContext> AuditTableBase::open_table() noexcept {
  auto ta_context = std::make_unique<TableAccessContext>();

  if (ta_context == nullptr) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to init table access context");
    return nullptr;
  }

  my_service<SERVICE_TYPE(mysql_current_thread_reader)> thd_reader_srv(
      "mysql_current_thread_reader", SysVars::get_comp_registry_srv());
  my_service<SERVICE_TYPE(table_access_factory_v1)> ta_factory_srv(
      "table_access_factory_v1", SysVars::get_comp_registry_srv());
  my_service<SERVICE_TYPE(table_access_v1)> table_access_srv(
      "table_access_v1", SysVars::get_comp_registry_srv());

  thd_reader_srv->get(&ta_context->thd);

  ta_context->ta_session =
      ta_factory_srv->create(ta_context->thd, kAccessedTableCount);
  if (ta_context->ta_session == nullptr) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to init table access service");
    return nullptr;
  }

  ta_context->table_ticket = table_access_srv->add(
      ta_context->ta_session, m_db_name.c_str(), m_db_name.length(),
      get_table_name(), strlen(get_table_name()), TA_WRITE);

  if (table_access_srv->begin(ta_context->ta_session) != 0) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to start table access transaction");
    return nullptr;
  }

  ta_context->ta_table =
      table_access_srv->get(ta_context->ta_session, ta_context->table_ticket);

  if (ta_context->ta_table == nullptr) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to get an opened %s table", get_table_name());
    return nullptr;
  }

  if (table_access_srv->check(ta_context->ta_session, ta_context->ta_table,
                              get_table_field_def(),
                              get_table_field_count()) != 0) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to check %s table fields", get_table_name());
    return nullptr;
  }

  return ta_context;
}

}  // namespace audit_log_filter::audit_table
