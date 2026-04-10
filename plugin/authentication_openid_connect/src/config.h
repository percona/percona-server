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

#include <chrono>
#include <cstddef>
#include <map>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>

#include <mysql/components/services/bits/system_variables_bits.h>
#include <mysql/plugin.h>
#include <mysql/service_thd_alloc.h>
#include <picojson/picojson.h>

#include <shared_mutex>
#include <vector>

#include "jwks.h"

class Id_token;

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
        roles(std::move(roles)) {}

  /**
   * @brief Loads the public keys from JWKS.
   *
   * @return false if keys were successfully loaded,
   * true if keys were not successfully loaded.
   */
  bool load_keys() noexcept;

  /**
   * @brief Loads the public keys from a JSON array.
   *
   * @param key_array The array containing key definitions.
   * @param from A descriptive string indicating the source (for error
   * messages).
   * @throws std::runtime_error if any key object is malformed or invalid.
   */
  void load_keys(const picojson::array &key_array, const std::string &from);

  /**
   * @brief Gets the issuer name.
   * @return The issuer name string.
   */
  const std::string &get_issuer_name() const noexcept { return issuer_name; }

  /**
   * @brief Gets the JWKS URL.
   * @return The JWKS URL.
   */
  const std::string &get_jwks_url() const noexcept { return jwks.get_url(); }

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

  /**
   * @brief Swaps the current keys with new keys.
   * @param other An Idp_config object containing the keys to swap in.
   */
  void swap_keys(Idp_config &other) { keys.swap(other.keys); }
};

/**
 * @class Idp_configs
 * @brief Manages a collection of Identity Provider configurations.
 */
class Idp_configs {
 private:
  std::string sysvar_str{};  ///< Value of the configuration system variable.
  std::map<std::string, Idp_config>
      idp_configs{};  ///< Map of IDP names to Idp_config objects.
  Idp_configs() = delete;

  /**
   * @brief Length of the
   */
  static constexpr size_t prefix_len{sizeof("FILE://") - 1};

  /**
   * @brief Timeout duration for acquiring locks when updating configurations.
   * This is used to prevent deadlocks in case of long-running operations while.
   */
  static constexpr std::chrono::seconds lock_timeout{5};

  /**
   * @brief Gets the current Idp_configs instance. The instance is a function
   * local static in order to avoid static initialization order issues.
   * @return A reference to the unique pointer holding the current Idp_configs.
   */
  static std::unique_ptr<Idp_configs> &current();

  /**
   * @brief A mutex used for synchronizing access to the
   * configuration. The mutex is a function local static to ensure it is
   * initialized before use and to avoid static initialization order issues.
   * @return A reference to the mutex used
   * @note This mutex should be used to synchronize access to the current
   * configuration.
   */
  static std::shared_timed_mutex &mutex();

  /**
   * @brief Parses the JSON configuration string and loads configurations cache.
   * @param config_json The JSON string containing IDP configurations.
   */
  void load(const std::string &config_json);

  /**
   * @brief Parses the configuration for a single IDP and adds it to the
   * idp_configs map.
   * @param idp_value   The JSON value representing the IDP configuration.
   * @param idp_name    The name of the IDP (used as the key in the map).
   */
  void load_idp(const picojson::value &idp_value,
                const std::string &idp_name) noexcept;

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

  /**
   * @brief Parses the configuration system variable and optionally loads the
   * configuration.
   * @param variable The value of the configuration variable.
   * configuration, else only the basic checks are done.
   * @param config_json The JSON string containing IDP configurations,
   * or an empty string if parsing fails.
   */
  static void parse_var(const char *variable,
                        std::string &config_json) noexcept;

  /**
   * @brief Validates the variable syntax, checks if the variable
   * or content of the file is a valid JSON.
   * @param variable The configuration variable value to validate.
   * @return true if parsing fails, false if valid.
   */
  static bool check(const char *variable) noexcept;

  /**
   *  @brief Parses and loads the configuration according to the new value of
   * the variable. Updates the system variable pointer to the new value stored
   * internally.
   *  @param variable New value of the variable.
   *  @param sysvar_ptr Pointer to the system variable.
   *  @note If parsing or loading fails, the system variable will not be updated
   * and an error will be logged.
   */
  static void update(const char *variable, const char **sysvar_ptr) noexcept;

  /**
   * @brief Gets the configuration for a specific IDP.
   * @param idp_name The name of the IDP.
   * @return reference to the Idp_config object, or nullptr if not found.
   * @throws std::runtime_error if the IDP is not found in the configuration.
   */
  Idp_config &get_idp(const std::string &idp_name) {
    const auto it = idp_configs.find(idp_name);
    if (it == idp_configs.end())
      throw std::runtime_error("IDP not found: " + idp_name);
    return it->second;
  }

  /**
   * @brief Swaps the keys of the specified IDP with the keys from another IDP
   * in a thread-safe manner.
   * @param idp_name The name of the first IDP.
   * @param other_idp The second IDP config.
   * @throws std::runtime_error if the IDP is not found in the configuration.
   */
  static void swap_idp_keys(const std::string &idp_name,
                            Idp_config &other_idp) {
    std::unique_lock lock(mutex());
    current()->get_idp(idp_name).swap_keys(other_idp);
  }

  /**
   * @brief Gets the JWKS URL for a specific IDP in a thread-safe manner.
   * @param idp_name The name of the IDP.
   * @return The JWKS URL for the specified IDP.
   */
  static const std::string &get_safe_jwks_url(const std::string &idp_name) {
    std::shared_lock lock(mutex(), lock_timeout);
    if (!lock.owns_lock())
      throw std::runtime_error("failed to acquire shared lock");
    return current()->get_idp(idp_name).get_jwks_url();
  }

  /**
   * @brief Gets the JWKS URL for a specific IDP in a thread-safe manner.
   */
  static void swap_idp_keys(
      std::vector<std::pair<std::string, Idp_config>> &configs) {
    std::unique_lock lock(mutex(), lock_timeout);
    if (!lock.owns_lock())
      throw std::runtime_error("failed to acquire unique lock");
    for (auto &config : configs) {
      current()->get_idp(config.first).swap_keys(config.second);
    }
  }

  /**
   * @brief Creates temporary IDP configs for further key loading in a
   * thread-safe manner.
   * @param configs A vector to be populated with pairs of IDP names and their
   * corresponding temporary Idp_config objects
   */
  static void create_tmp_configs(
      std::vector<std::pair<std::string, Idp_config>> &configs) {
    std::shared_lock lock(mutex(), lock_timeout);
    if (!lock.owns_lock())
      throw std::runtime_error("failed to acquire shared lock");
    configs.reserve(current()->idp_configs.size());
    for (auto &config : current()->idp_configs) {
      configs.emplace_back(
          config.first,
          Idp_config("", config.second.get_jwks_url(), "", {}, {}));
    }
  }

 public:
  static char *sysvar;  ///< Pointer to the system variable storage.

  /**
   * @brief Constructor for Idp_configs.
   * @param sysvar_str  The value of the system variable string.
   */
  explicit Idp_configs(const char *sysvar_str) : sysvar_str(sysvar_str) {}

  /**
   * @brief Verifies the ID token and extracts user roles based on the IDP
   * configuration.
   * @param token  The ID token to verify.
   * @param idp_name The name of the IDP to use for verification.
   * @param ext_user  The expected external username (subject) in the token.
   * @param roles   A string to be populated with the mapped database roles
   * (comma-separated) if verification is successful.
   */
  static void verify_token(const Id_token &token, const std::string &idp_name,
                           const std::string &ext_user, std::string &roles);

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
   * @return on success: number of updated IDPs (0 or 1),
   * on failure: negative value
   */
  static long long update_keys(const char *idp_name) noexcept;
};

#endif  // AUTH_OIDC_CONFIG_H
