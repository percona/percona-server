/* Copyright (c) 2024 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; version 2 of
   the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#ifndef PERCONA_TELEMETRY_CONFIG_H
#define PERCONA_TELEMETRY_CONFIG_H

#include <chrono>
#include <memory>
#include <string>

#include <mysql/components/component_implementation.h>
#include <mysql/components/services/component_sys_var_service.h>

class Config {
 public:
  Config(SERVICE_TYPE(component_sys_variable_register) & var_register_service,
         SERVICE_TYPE(component_sys_variable_unregister) &
             var_unregister_service);
  ~Config() = default;

  Config(const Config &rhs) = delete;
  Config(Config &&rhs) = delete;
  Config &operator=(const Config &rhs) = delete;
  Config &operator=(Config &&rhs) = delete;

  bool init();
  bool deinit();

  const std::string &telemetry_storage_dir_path() const noexcept;
  std::chrono::seconds scrape_interval() const noexcept;
  std::chrono::seconds grace_interval() const noexcept;
  std::chrono::seconds history_keep_interval() const noexcept;
  std::chrono::seconds unconditional_history_cleanup_interval() const noexcept;

 private:
  SERVICE_TYPE(component_sys_variable_register) & var_register_service_;
  SERVICE_TYPE(component_sys_variable_unregister) & var_unregister_service_;

  const char *telemetry_root_dir_value_;
  uint scrape_interval_value_;
  uint grace_interval_value_;
  uint history_keep_interval_value_;
};

#endif /* PERCONA_TELEMETRY_CONFIG_H */
