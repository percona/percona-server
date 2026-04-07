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
#include <picojson/picojson.h>

#include <ranges>

#include "jwks.h"

/**
 * @class Idp_config
 * @brief Configuration for a single Identity Provider (IDP).
 */
class Idp_config {
 private:
  std::string issuer_name;  ///< The token's issuer name.
  Jwks jwks;                ///< URL for the JSON Web Key Set.
  std::string group_claim;  ///< Name of the claim in the JWT that contains
                            ///< group information.
  std::unordered_set<std::string>
      audiences;  ///< Set of allowed audiences for the token.
  std::map<std::string, std::string>
      roles;  ///< Map of IDP groups to database roles.
  std::map<std::string, std::string>
      keys;  ///< Map of Key ID (kid) to public key in PEM format.

 public:
  /**
   * @brief Constructs an Idp_config object.
   * @param issuer_name The token's issuer name.
   * @param jwks_url The URL for the JSON Web Key Set.
   * @param group_claim The group claim name.
   * @param audiences A set of allowed audiences.
   * @param roles A map of group-to-role mappings.
   */
  Idp_config(const std::string &issuer_name, const std::string &jwks_url,
             const std::string &group_claim,
             std::unordered_set<std::string> &&audiences,
             std::map<std::string, std::string> &&roles)
      : issuer_name(issuer_name),
        jwks(jwks_url),
        group_claim(group_claim),
        audiences(std::move(audiences)),
        roles(std::move(roles)) {
    load_keys(jwks_url);
  }

  /**
   * @brief Constructs an Idp_config object.
   * @param issuer_name The token's issuer name.
   * @param key_array The array of keys from JSON.
   * @param idp_name The name of the IDP, used for error messages when loading
   * keys from the array.
   * @param group_claim The group claim name.
   * @param audiences A set of allowed audiences.
   * @param roles A map of group-to-role mappings.
   */
  Idp_config(const std::string &issuer_name, const picojson::array &key_array,
             const std::string &idp_name, const std::string &group_claim,
             std::unordered_set<std::string> &&audiences,
             std::map<std::string, std::string> &&roles)
      : issuer_name(issuer_name),
        jwks(""),
        group_claim(group_claim),
        audiences(std::move(audiences)),
        roles(std::move(roles)) {
    load_keys(key_array, idp_name);
  }

  bool is_using_jwks() const noexcept { return !jwks.get_url().empty(); }

  void update_keys() {
    if (is_using_jwks()) load_keys(jwks.get_url());
  }

  /**
   * @brief Loads the public keys from a JWKS URL.
   *
   * @param url The URL of the JWKS endpoint.
   * @throws std::runtime_error if the HTTP request fails, JSON parsing fails,
   *                            or key construction fails.
   */
  void load_keys(const std::string &url);

  /**
   * @brief Loads the public keys from a JSON array.
   *
   * @param key_array The array containing key definitions.
   * @param from A descriptive string indicating the source (for error messages).
   * @throws std::runtime_error if any key object is malformed or invalid.
   */
  void load_keys(const picojson::array &key_array, const std::string &from);

  /**
   * @brief Gets the issuer name.
   * @return The issuer name string.
   */
  const std::string &get_issuer_name() const noexcept { return issuer_name; }

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
    const auto key{keys.cbegin()};
    if (key == keys.cend()) throw std::runtime_error("no keys available");
    return key->second;
  }

  /**
   * @brief Gets a public key by its Key ID (kid).
   * @param kid The Key ID.
   * @return The public key in PEM format.
   * @throws std::out_of_range if the Key ID is not found.
   */
  const std::string &get_pub_key(const std::string &kid) const {
    return keys.at(kid);
  }

  /**
   * @brief Checks if a given audience is allowed.
   * @param audience The audience string from the token.
   * @return true if the audience is allowed or if no audiences are configured
   * (allowing all).
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
  std::string config_var{};  ///< The raw configuration variable value.
  std::map<std::string, Idp_config>
      idp_configs{};            ///< Map of IDP names to Idp_config objects.
  static Idp_configs *configs;  ///< Singleton instance of the configuration.
  Idp_configs() {}
  Idp_configs(const std::string &config_var, const std::string &config_json)
      : config_var(config_var) {
    parse_json(config_json);
  }
  static constexpr size_t prefix_len{sizeof("FILE://") - 1};

  /**
   * @brief Parses the JSON configuration string.
   * @param config_json The JSON string containing IDP configurations.
   */
  void parse_json(const std::string &config_json);

  /**
   * @brief Parses the prefix of the configuration system variable.
   * @param prefix The prefix.
   * @return F: if the prefix is FILE, J: if the prefix is JSON, else throws an
   * exception.
   */
  static char parse_prefix(const std::string &prefix);

  /**
   * @brief Reads the configuration from a file.
   * @param path The path to the configuration file.
   * @return The content of the file as a string.
   */
  static std::string read_from_file(const std::string &path);

 public:
  static char *sysvar;  ///< Pointer to the system variable storage.

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
  static Idp_config *get_item(const std::string &idp_name) noexcept {
    if (configs == nullptr) return nullptr;
    std::map<std::string, Idp_config>::iterator it =
        configs->idp_configs.find(idp_name);
    if (it == configs->idp_configs.end()) return nullptr;
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

  /**
   * @brief Updates the keys for all IDPs by calling JWKS.
   * @return on success: number of updated IDPs, on failure: negative value
   */
  static long long update_keys() noexcept;

  /**
   * @brief Updates the keys for a specific IDP by calling JWKS.
   * @param idp_name name od IDP which keys are to be updated.
   * @return on success: number of updated IDPs, on failure: negative value
   */
  static long long update_keys(const char *idp_name) noexcept;
};

#endif  // AUTH_OIDC_CONFIG_H
