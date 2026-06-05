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

#include <picojson/picojson.h>
#include <chrono>
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

#include <shared_mutex>
#include <vector>

#include "jwks.h"
#include "psi_openid_connect.h"

class Id_token;

template <typename T>
using Config_allocator =
    Psi_openid_connect::Allocator<T, Psi_openid_connect::config_memory_key>;

using Config_string =
    std::basic_string<char, std::char_traits<char>,
                      Psi_openid_connect::Config_allocator<char>>;

#if defined(__GNUC__) && !defined(__clang__) && (__GNUC__ < 11)
#define HAVE_TRANSPARENT_COMPARISON 0
#else
#define HAVE_TRANSPARENT_COMPARISON 1
#endif

class Idp_config;
#if HAVE_TRANSPARENT_COMPARISON
struct Config_string_hash {
  using is_transparent = void;
  std::size_t operator()(std::string_view s) const noexcept {
    return std::hash<std::string_view>{}(s);
  }
  std::size_t operator()(const Config_string &s) const noexcept {
    return (*this)(std::string_view{s.data(), s.size()});
  }
};

using Config_string_set =
    std::unordered_set<Config_string, Config_string_hash, std::equal_to<>,
                       Psi_openid_connect::Config_allocator<Config_string>>;
using Config_string_map =
    std::map<Config_string, Config_string, std::less<>,
             Psi_openid_connect::Config_allocator<
                 std::pair<const Config_string, Config_string>>>;
using Config_idp_map =
    std::map<Config_string, Idp_config, std::less<>,
             Psi_openid_connect::Config_allocator<
                 std::pair<const Config_string, Idp_config>>>;
#else
struct Config_string_hash {
  std::size_t operator()(const Config_string &s) const noexcept {
    return std::hash<std::string_view>{}(std::string_view{s.data(), s.size()});
  }
};

using Config_string_set =
    std::unordered_set<Config_string, Config_string_hash,
                       std::equal_to<Config_string>,
                       Psi_openid_connect::Config_allocator<Config_string>>;
using Config_string_map =
    std::map<Config_string, Config_string, std::less<Config_string>,
             Psi_openid_connect::Config_allocator<
                 std::pair<const Config_string, Config_string>>>;
using Config_idp_map =
    std::map<Config_string, Idp_config, std::less<Config_string>,
             Psi_openid_connect::Config_allocator<
                 std::pair<const Config_string, Idp_config>>>;
#endif

/**
 * @brief Statistics for Identity Provider (IDP) configurations.
 */
struct Idp_config_statistics {
  long long configured_idps{0};  ///< Total number of configured IDPs.
  long long configured_idps_using_jwks{
      0};  ///< Number of configured IDPs that use JWKS for key management.
  long long configured_role_maps{
      0};  ///< Total number of group-to-role mappings across all IDPs.
};

/**
 * @class Idp_config
 * @brief Configuration for a single Identity Provider (IDP).
 */
class Idp_config {
 private:
  Config_string issuer_name;    ///< The token's issuer name.
  Jwks jwks;                    ///< Object managing the JSON Web Key Set.
  Config_string group_claim;    ///< Name of the claim in the JWT that contains
                                ///< group information.
  Config_string_set audiences;  ///< Set of allowed audiences for the token.
  Config_string_map roles;      ///< Map of IDP groups to database roles.
  Config_string_map keys;  ///< Map of Key ID (kid) to public key in PEM format.

 public:
  /**
   * @brief Constructs an Idp_config object.
   * @param issuer_name The token's issuer name.
   * @param jwks_url The URL for the JSON Web Key Set.
   * @param group_claim The group claim name.
   * @param audiences A set of allowed audiences.
   * @param roles A map of group-to-role mappings.
   */
  Idp_config(const std::string_view issuer_name,
             const std::string_view jwks_url,
             const std::string_view group_claim, Config_string_set &&audiences,
             Config_string_map &&roles)
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
  std::string get_issuer_name() const noexcept {
    return std::string(issuer_name);
  }

  /**
   * @brief Gets the JWKS URL.
   * @return The JWKS URL.
   */
  const std::string &get_jwks_url() const noexcept { return jwks.get_url(); }

  /**
   * @brief Gets the name of the group claim.
   * @return The group claim name.
   */
  std::string_view get_group_claim() const noexcept { return group_claim; }

  /**
   * @brief Gets the only public key.
   * The "kid" element may be omitted in JOSE header iff only one key is
   * available.
   * @return The only public key in PEM format.
   * @throws std::runtime_error if no keys are available.
   */
  std::string_view get_the_only_pub_key() const {
    if (keys.size() != 1) throw std::runtime_error("incorrect number of keys");
    const auto key{keys.cbegin()};
    return key->second;
  }

  /**
   * @brief Gets a public key by its Key ID (kid).
   * @param kid The Key ID.
   * @return The public key in PEM format.
   * @throws std::out_of_range if the Key ID is not found.
   */
  std::string_view get_pub_key(const std::string_view kid) const {
#if HAVE_TRANSPARENT_COMPARISON
    const auto it = keys.find(kid);
#else
    const auto it = keys.find(Config_string(kid));
#endif

    if (it == keys.end()) throw std::out_of_range("KID not found");
    return it->second;
  }

  /**
   * @brief Shall be audiences claim shall be checked?
   * @return true if audiences were configured, false otherwise.
   */
  bool check_audiences() const noexcept { return !audiences.empty(); }

  /**
   * @brief Checks if a given audience is allowed.
   * @param audience The audience string from the token.
   * @return true if the audience is allowed
   */
  bool is_audience_allowed(const std::string_view audience) const noexcept {
#if HAVE_TRANSPARENT_COMPARISON
    return audiences.contains(audience);
#else
    return audiences.contains(Config_string(audience));
#endif
  }

  /**
   * @brief Gets the mapped database role for an IDP group.
   * @param group The IDP group name.
   * @return The mapped role name, or an empty string if no mapping exists.
   */
  std::string_view get_role(const std::string_view group) const noexcept {
    static Config_string no_role;
#if HAVE_TRANSPARENT_COMPARISON
    const auto it = roles.find(group);
#else
    const auto it = roles.find(Config_string(group));
#endif
    return it == roles.end() ? no_role : it->second;
  }

  /**
   * @brief Swaps the current keys with new keys.
   * @param other An Idp_config object containing the keys to swap in.
   */
  void swap_keys(Idp_config &other) { keys.swap(other.keys); }

  void add_statistics(Idp_config_statistics &statistics) const noexcept {
    statistics.configured_idps++;
    statistics.configured_role_maps += roles.size();
    if (!jwks.get_url().empty()) statistics.configured_idps_using_jwks++;
  }
};

/**
 * @class Idp_configs
 * @brief Manages a collection of Identity Provider configurations.
 */
class Idp_configs {
 private:
  Config_string sysvar_str{};  ///< Value of the configuration system variable.
  Config_idp_map idp_configs{};  ///< Map of IDP names to Idp_config objects.
  Idp_configs() = delete;

  /**
   * @brief Prefixes and their length
   */
  static constexpr std::string_view file_prefix{"FILE://"};
  static constexpr std::string_view json_prefix{"JSON://"};
  static_assert(file_prefix.length() == json_prefix.length(),
                "prefixes must have the same length for correct parsing");
  static constexpr size_t prefix_len{file_prefix.length()};

  /**
   * @brief Timeout duration for acquiring locks when updating configurations.
   * This is used to prevent deadlocks in case of long-running operations
   * while.
   */
  static constexpr std::chrono::seconds lock_timeout{15};

  /**
   * @brief Gets the current Idp_configs instance. The instance is a function
   * local static in order to avoid static initialization order issues.
   * @return A reference to the unique pointer holding the current
   * Idp_configs.
   */
  static std::unique_ptr<Idp_configs> &current(const char *sysvar_str = "");

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
   * @brief Parses the JSON configuration string and loads configurations
   * cache.
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
   * @brief Load mapping of IDP groups to database roles from an IDP JSON
   * object.
   *
   * @param idp_object The picojson::object that contains the IDP
   * configuration.
   * @param idp_name   The name of the IDP
   * @return map of IDP group names to database role names. If no mapping is
   * present the returned map will be empty.
   */
  static Config_string_map load_group_roles(const picojson::object &idp_object,
                                            const std::string &idp_name);

  /**
   * @brief Load the set of allowed audiences from an IDP JSON object.
   *
   * @param idp_object The picojson::object that contains the IDP
   * configuration.
   * @param idp_name   The name of the IDP
   *
   * @return An unordered set containing the allowed audience
   *         strings. If no audiences are configured the set will be empty.
   */
  static Config_string_set load_audiences(const picojson::object &idp_object,
                                          const std::string &idp_name);

  /**
   * @brief Parses the prefix of the configuration system variable.
   * @param prefix The prefix.
   * @return F: if the prefix is FILE, J: if the prefix is JSON, else throws
   * an exception.
   */
  static char parse_prefix(const std::string &prefix);

  /**
   * @brief Reads the configuration from a file.
   * @param path The path to the configuration file.
   * @return The content of the file as a string.
   */
  static std::string read_from_file(const std::string &path);

  /**
   * @brief Parses the configuration system variable.
   * @param variable The value of the configuration variable.
   * configuration, else only the basic checks are done.
   * @param config_json The JSON string containing IDP configurations,
   * or an empty string if parsing fails.
   */
  static void parse_var(const char *variable, std::string &config_json);

  /**
   * @brief Gets the configuration for a specific IDP.
   * @param idp_name The name of the IDP.
   * @return reference to the Idp_config object, or nullptr if not found.
   * @throws std::runtime_error if the IDP is not found in the configuration.
   */
  Idp_config &get_idp(std::string_view idp_name) {
#if HAVE_TRANSPARENT_COMPARISON
    const auto it = idp_configs.find(idp_name);
#else
    const auto it = idp_configs.find(Config_string(idp_name));
#endif
    if (it == idp_configs.end())
      throw std::runtime_error(std::string("IDP not found: ") +
                               std::string(idp_name));
    return it->second;
  }

  /**
   * @brief Swaps the keys of the specified IDP with the keys from another IDP
   * in a thread-safe manner.
   * @param idp_name The name of the first IDP.
   * @param other_idp The second IDP config.
   * @throws std::runtime_error if the IDP is not found in the configuration.
   */
  static void swap_idp_keys(const Config_string &idp_name,
                            Idp_config &other_idp) {
    std::unique_lock lock(mutex());
    current()->get_idp(idp_name).swap_keys(other_idp);
  }

  /**
   * @brief Gets the JWKS URL for a specific IDP in a thread-safe manner.
   * @param idp_name The name of the IDP.
   * @return The JWKS URL for the specified IDP.
   */
  static const std::string get_safe_jwks_url(const Config_string &idp_name) {
    std::shared_lock lock(mutex(), lock_timeout);
    if (!lock.owns_lock())
      throw std::runtime_error("failed to acquire shared lock");
    return current()->get_idp(idp_name).get_jwks_url();
  }

  /**
   * @brief Gets the JWKS URL for a specific IDP in a thread-safe manner.
   */
  static void swap_idp_keys(
      std::vector<std::pair<Config_string, Idp_config>> &configs) {
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
      std::vector<std::pair<Config_string, Idp_config>> &configs) {
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
  static Idp_config_statistics
      statistics;  ///< Statistics about the current configuration.

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
   * @param ext_group The expected external group in the token.
   * @param roles   A string to be populated with the mapped database roles
   * @return The proxy user name
   * (comma-separated) if verification is successful.
   */
  static std::string verify_token(const Id_token &token,
                                  const std::string &idp_name,
                                  const std::string &ext_user,
                                  const std::string &ext_group,
                                  std::string &roles);

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
   *  @note If parsing or loading fails, the system variable will not be
   * updated and an error will be logged.
   */
  static void update(const char *variable, const char **sysvar_ptr) noexcept;

  /**
   *  @brief Parses and loads the configuration according to the value of
   * the variable. To be used on plugin initialization only.
   *  @param variable Value of the variable.
   *  @note If parsing or loading fails, the system variable will not be set
   *        and an error will be logged.
   */
  static void set(const char *variable) noexcept;

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

  static constexpr long long update_keys_error{
      -1};  ///< Return value indicating that an error occurred
  /**
   * @brief Updates the keys for all IDPs by calling JWKS.
   * @return on success: number of updated IDPs, on failure: update_keys_error
   */
  static long long update_keys() noexcept;

  /**
   * @brief Updates the keys for a specific IDP by calling JWKS.
   * @param idp_name name od IDP which keys are to be updated.
   * @return on success: number of updated IDPs (0 or 1),
   * on failure: update_keys_error
   */
  static long long update_keys(const char *idp_name) noexcept;

  /**
   * @brief Collects statistics about the current configuration.
   * @return statistics structure.
   */
  Idp_config_statistics collect_statistics() const {
    Idp_config_statistics new_statistics;
    for (const auto &idp : idp_configs) {
      idp.second.add_statistics(new_statistics);
    }
    return new_statistics;
  }
};

#endif  // AUTH_OIDC_CONFIG_H
