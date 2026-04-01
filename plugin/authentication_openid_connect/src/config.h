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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
*/

#ifndef AUTH_OIDC_CONFIG_H
#define AUTH_OIDC_CONFIG_H


#include <map>
#include <stdexcept>
#include <string>
#include <unordered_set>

#include <mysql/components/services/bits/system_variables_bits.h>
#include <mysql/plugin.h>
#include <mysql/service_thd_alloc.h>

/**
 * @class Idp_config
 * @brief Configuration for a single Identity Provider (IDP).
 */
class Idp_config {
 private:
  std::string jwks_uri{}; ///< URI for the JSON Web Key Set.
  std::string group_claim{}; ///< Name of the claim in the JWT that contains group information.
  // map kid -> key in PEM format
  std::map<std::string, std::string> pub_keys{}; ///< Map of Key IDs to PEM-encoded public keys.
  std::unordered_set<std::string> audiences{}; ///< Set of allowed audiences for the token.
  std::map<std::string, std::string> roles{}; ///< Map of IDP groups to database roles.

 public:

  /**
   * @brief Constructs an Idp_config object.
   * @param jwks_uri The JWKS URI.
   * @param group_claim The group claim name.
   * @param pub_keys A map of public keys.
   * @param audiences A set of allowed audiences.
   * @param roles A map of group-to-role mappings.
   */
  Idp_config(const std::string &jwks_uri, const std::string &group_claim,
           std::map<std::string, std::string> &&pub_keys,
           std::unordered_set<std::string> &&audiences,
           std::map<std::string, std::string> &&roles)
    : jwks_uri(jwks_uri),
      group_claim(group_claim),
      pub_keys(std::move(pub_keys)),
      audiences(std::move(audiences)),
      roles(std::move(roles)) {}


  /**
   * @brief Gets the JWKS URI.
   * @return The JWKS URI string.
   */
  const std::string &get_jwks_uri() const noexcept { return jwks_uri; }

  /**
   * @brief Gets the name of the group claim.
   * @return The group claim name.
   */
  const std::string &get_group_claim() const noexcept { return group_claim; }

  /**
   * @brief Gets the first available public key.
   * @return The first public key in PEM format.
   * @throws std::runtime_error if no keys are available.
   */
  const std::string &get_pub_key() const {
    if (pub_keys.empty()) throw std::runtime_error("no IDP public key");
    return pub_keys.begin()->second;
  }

  /**
   * @brief Gets a public key by its Key ID (kid).
   * @param kid The Key ID.
   * @return The public key in PEM format.
   * @throws std::out_of_range if the Key ID is not found.
   */
  const std::string &get_pub_key(const std::string &kid) const {
    return pub_keys.at(kid);
  }

  /**
   * @brief Checks if a given audience is allowed.
   * @param audience The audience string from the token.
   * @return true if the audience is allowed or if no audiences are configured (allowing all).
   */
  bool is_audience_allowed(const std::string &audience) const noexcept {
    return audiences.empty() || audiences.contains(audience);
  }

  /**
   * @brief Gets the mapped database role for an IDP group.
   * @param group The IDP group name.
   * @return The mapped role name, or an empty string if no mapping exists.
   */
  const std::string &get_role(const std::string &group) const noexcept {
    static std::string no_role;
    const auto it = roles.find(group);
    return it == roles.end() ? no_role : it->second;
  }
};

/**
 * @class Idp_configs
 * @brief Manages a collection of Identity Provider configurations.
 */
class Idp_configs {
 private:
  std::string config_var{}; ///< The raw configuration variable value.
  std::map<std::string, Idp_config> idp_configs{}; ///< Map of IDP names to Idp_config objects.
  static const Idp_configs *config; ///< Singleton instance of the configuration.
  Idp_configs() {}
  Idp_configs(const std::string &config_var, const std::string &config_json)
      : config_var(config_var) {
    parse_json(config_json);
  }

  /**
   * @brief Parses the JSON configuration string.
   * @param config_json The JSON string containing IDP configurations.
   */
  void parse_json(const std::string &config_json);

 public:
  static char *sysvar; ///< Pointer to the system variable storage.

  /**
   * @brief Parses the configuration system variable.
   * @param config_var The value of the configuration variable.
   * @return A pointer to the newly created Idp_configs instance.
   */
  static Idp_configs *parse_var(const std::string &config_var) noexcept;

  /**
   * @brief Gets the configuration for a specific IDP.
   * @param idp_name The name of the IDP.
   * @return A pointer to the Idp_config object, or nullptr if not found.
   */
  static const Idp_config *get_item(const std::string &idp_name) noexcept {
    if (config == nullptr) return nullptr;
    const auto it = config->idp_configs.find(idp_name);
    if (it == config->idp_configs.end()) return nullptr;
    return &(it->second);
  }

  /**
   * @brief Check function for the MySQL system variable.
   * @return 0 for success, non-zero for error.
   */
  static int check(MYSQL_THD thd [[maybe_unused]],
                   SYS_VAR *var [[maybe_unused]], void *save,
                   st_mysql_value *value);

  /**
   * @brief Update function for the MySQL system variable.
   */
  static void update(MYSQL_THD thd [[maybe_unused]],
                     SYS_VAR *var [[maybe_unused]], void *var_ptr,
                     const void *save);
};

#endif  // AUTH_OIDC_CONFIG_H
