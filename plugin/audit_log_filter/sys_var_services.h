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

#ifndef AUDIT_LOG_FILTER_SYS_VAR_SERVICES_H_INCLUDED
#define AUDIT_LOG_FILTER_SYS_VAR_SERVICES_H_INCLUDED

#include <mysql/components/services/component_sys_var_service.h>
#include <mysql/service_plugin_registry.h>

namespace audit_log_filter {

class SysVarServices {
  using registry_srv_t = SERVICE_TYPE(registry);
  using sys_variable_register_srv_t =
      SERVICE_TYPE(component_sys_variable_register);
  using sys_variable_unregister_srv_t =
      SERVICE_TYPE(component_sys_variable_unregister);

 public:
  ~SysVarServices();

  /**
   * @brief Get system variable registering service.
   *
   * @return System variable registering service
   */
  [[nodiscard]] sys_variable_register_srv_t *get_sys_var_reg() noexcept {
    return m_sys_var_reg;
  }

  /**
   * @brief Get system variable unregistering service.
   *
   * @return System variable unregistering service
   */
  [[nodiscard]] sys_variable_unregister_srv_t *get_sys_var_unreg() noexcept {
    return m_sys_var_unreg;
  }

  /**
   * @brief Acquire system variables handling services.
   *
   * @return true in case of success, false otherwise
   */
  bool acquire() noexcept;

 private:
  /**
   * @brief Release system variables handling services.
   */
  void release() noexcept;

 private:
  registry_srv_t *m_registry;
  sys_variable_register_srv_t *m_sys_var_reg;
  sys_variable_unregister_srv_t *m_sys_var_unreg;
};

}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_SYS_VAR_SERVICES_H_INCLUDED
