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


#include <map>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#include "mysql/components/services/bits/system_variables_bits.h"
#include "mysql/plugin.h"
#include "mysql/service_thd_alloc.h"

class Idp_config {
 private:
  std::string jwks_uri{};
  std::string group_claim{};
  // map kid -> key in PEM format
  std::map<std::string, std::string> pub_keys{};
  std::unordered_set<std::string> audiences{};
  std::map<std::string, std::string> roles{};

 public:

  Idp_config(const std::string &jwks_uri, const std::string &group_claim,
           std::map<std::string, std::string> &&pub_keys,
           std::unordered_set<std::string> &&audiences,
           std::map<std::string, std::string> &&roles)
    : jwks_uri(jwks_uri),
      group_claim(group_claim),
      pub_keys(std::move(pub_keys)),
      audiences(std::move(audiences)),
      roles(std::move(roles)) {}


  const std::string &get_jwks_uri() const noexcept { return jwks_uri; }
  const std::string &get_group_claim() const noexcept { return group_claim; }
  const std::string &get_pub_key() const {
    if (pub_keys.empty()) throw std::runtime_error("no IDP public key");
    return pub_keys.begin()->second;
  }
  const std::string &get_pub_key(const std::string &kid) const {
    return pub_keys.at(kid);
  }
  bool is_audience_allowed(const std::string &audience) const noexcept {
    return audiences.empty() || audiences.contains(audience);
  }
  const std::string &get_role(const std::string &group) const noexcept {
    static constexpr std::string no_role;
    auto it = roles.find(group);
    return it == roles.end() ? no_role : it->second;
  }
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
