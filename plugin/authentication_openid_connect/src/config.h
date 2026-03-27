/*
(C) 2026 Percona LLC and/or its affiliates

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/

#ifndef AUTH_OIDC_CONFIG_H
#define AUTH_OIDC_CONFIG_H

#include "mysql/components/services/bits/system_variables_bits.h"
#include "mysql/plugin.h"
#include "mysql/service_thd_alloc.h"

#include <picojson/picojson.h>
#include <map>
#include <string>

struct Idp_config {
  std::string name{};
  std::string use{};
  std::string pub_key{};
};

class Idp_configs {
 private:
  std::string config_var{};
  std::map<std::string, Idp_config> idp_configs{};
  static const Idp_configs *config;
  Idp_configs() {}
  Idp_configs(const std::string &config_var, const std::string &config_json)
      : config_var(config_var) {
    parse_json(config_json);
  }
  void parse_json(const std::string &config_json);
  static std::string get_str(const picojson::object &obj,
                             const std::string &key, const std::string &from,
                             bool is_mandatory = true);

 public:
  static char *sysvar;
  static Idp_configs *parse_var(const std::string &config_var);
  static const Idp_config *get_item(const std::string &idp_name) {
    if (config == nullptr) return nullptr;
    auto it = config->idp_configs.find(idp_name);
    if (it == config->idp_configs.end()) return nullptr;
    return &(it->second);
  }

  static int check(MYSQL_THD thd [[maybe_unused]],
                   SYS_VAR *var [[maybe_unused]], void *save,
                   st_mysql_value *value);
  static void update(MYSQL_THD thd [[maybe_unused]],
                     SYS_VAR *var [[maybe_unused]], void *var_ptr,
                     const void *save);
};

#endif  // AUTH_OIDC_CONFIG_H
