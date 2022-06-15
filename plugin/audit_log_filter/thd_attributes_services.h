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

#ifndef AUDIT_LOG_FILTER_THD_ATTRIBUTES_SERVICES_H_INCLUDED
#define AUDIT_LOG_FILTER_THD_ATTRIBUTES_SERVICES_H_INCLUDED

#include <mysql/components/services/mysql_thd_attributes.h>
#include <mysql/service_plugin_registry.h>

namespace audit_log_filter {

class ThdAttributesServices {
  using registry_srv_t = SERVICE_TYPE(registry);
  using mysql_thd_attributes_srv_t = SERVICE_TYPE(mysql_thd_attributes);

 public:
  ~ThdAttributesServices();

  /**
   * @brief Get component registry service.
   *
   * @return Component registry service
   */
  [[nodiscard]] registry_srv_t *get_reg_srv() noexcept { return m_registry; }

  /**
   * @brief Get THD attributes access service.
   *
   * @return THD attributes access service
   */
  [[nodiscard]] mysql_thd_attributes_srv_t *get_thd_attrs_srv() noexcept {
    return m_thd_attrs;
  }

  /**
   * @brief Acquire THD attributes access service.
   *
   * @return true in case of success, false otherwise
   */
  bool acquire() noexcept;

  /**
   * @brief Release THD attributes access service.
   */
  void release() noexcept;

 private:
  registry_srv_t *m_registry;
  mysql_thd_attributes_srv_t *m_thd_attrs;
};

}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_THD_ATTRIBUTES_SERVICES_H_INCLUDED
