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

#include "plugin/audit_log_filter/thd_attributes_services.h"
#include "plugin/audit_log_filter/audit_error_log.h"

namespace audit_log_filter {
namespace {

using thd_attributes_t = SERVICE_TYPE_NO_CONST(mysql_thd_attributes);

}  // namespace

ThdAttributesServices::~ThdAttributesServices() { release(); }

bool ThdAttributesServices::acquire() noexcept {
  m_registry = mysql_plugin_registry_acquire();

  if (m_registry == nullptr) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                 "Failed to acquire plugin registry service");
    return false;
  }

  m_registry->acquire("mysql_thd_attributes",
                      reinterpret_cast<my_h_service *>(
                          const_cast<thd_attributes_t **>(&m_thd_attrs)));

  if (m_thd_attrs == nullptr) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                 "Failed to acquire mysql_thd_attributes service");
    release();
    return false;
  }

  return true;
}

void ThdAttributesServices::release() noexcept {
  if (m_registry != nullptr) {
    if (m_thd_attrs != nullptr) {
      m_registry->release(reinterpret_cast<my_h_service>(
          const_cast<thd_attributes_t *>(m_thd_attrs)));
    }

    mysql_plugin_registry_release(m_registry);
  }
}

}  // namespace audit_log_filter
