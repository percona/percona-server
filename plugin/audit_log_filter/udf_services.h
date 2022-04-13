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

#ifndef AUDIT_LOG_FILTER_UDF_SERVICES_H_INCLUDED
#define AUDIT_LOG_FILTER_UDF_SERVICES_H_INCLUDED

#include <mysql/components/services/udf_metadata.h>
#include <mysql/components/services/udf_registration.h>
#include <mysql/service_plugin_registry.h>

namespace audit_log_filter {

class UdfServices {
  using registry_srv_t = SERVICE_TYPE(registry);
  using udf_registration_srv_t = SERVICE_TYPE(udf_registration);
  using udf_metadata_srv_t = SERVICE_TYPE(mysql_udf_metadata);

 public:
  ~UdfServices();

  /**
   * @brief Get UDF registration service.
   *
   * @return UDF registration service
   */
  [[nodiscard]] udf_registration_srv_t *get_udf_reg() noexcept {
    return m_udf_reg;
  }

  /**
   * @brief Get UDF metadata handling service.
   *
   * @return UDF metadata handling service
   */
  [[nodiscard]] udf_metadata_srv_t *get_udf_metadata() noexcept {
    return m_udf_metadata;
  }

  /**
   * @brief Acquire UDF handling services.
   *
   * @return true in case of success, false otherwise
   */
  bool acquire() noexcept;

  /**
   * @brief Release UDF handling services.
   */
  void release() noexcept;

 private:
  registry_srv_t *m_registry;
  udf_registration_srv_t *m_udf_reg;
  udf_metadata_srv_t *m_udf_metadata;
};

}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_UDF_SERVICES_H_INCLUDED
