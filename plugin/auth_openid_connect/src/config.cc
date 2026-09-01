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

#include "config.h"

#include <picojson/picojson.h>
#include <exception>
#include <fstream>
#include <mutex>
#include <ranges>
#include <shared_mutex>
#include <stdexcept>
#include <string_view>

#include <mysql/components/services/bits/system_variables_bits.h>
#include <mysql/components/services/log_builtins.h>
#include <mysql/my_loglevel.h>
#include <mysql/plugin.h>
#include <mysqld_error.h>

#include "id_token.h"
#include "jwk.h"
#include "jwks.h"
#include "mysql/components/services/bits/thd.h"

char *Idp_configs::sysvar(nullptr);
Idp_config_statistics Idp_configs::statistics;

static constexpr std::string_view key_keys{"keys"};

/**
 * @brief Retrieves a value from a picojson object by key.
 *
 * This template function attempts to extract a value of type T from the given
 * picojson object using the specified key. If the key is not found or the
 * value is not of the expected type, it handles the error based on the
 * is_mandatory flag.
 *
 * @tparam T The expected type of the value to retrieve (e.g., std::string,
 * int).
 * @param obj The picojson object to search in.
 * @param key The key to look for in the object.
 * @param from A descriptive string indicating where the object comes from
 *             (used in error messages).
 * @param is_mandatory If true, throws an exception if the key is missing or
 *                     the type doesn't match; if false, returns a default
 * value.
 * @return A const reference to the retrieved value if found and of correct
 * type, or a default value if is_mandatory is false and the key/type is
 * invalid.
 * @throws std::runtime_error If is_mandatory is true and the key is not found
 *                            or the value is not of type T.
 *
 * @note The default value returned when is_mandatory is false is a static
 *       default-constructed instance of type T.
 */
template <typename T>
static const T &json_get(const picojson::object &obj, const std::string &key,
                         const std::string &from,
                         const bool is_mandatory = true) {
  const auto it = obj.find(key);
  if (it == obj.end() || !it->second.is<T>()) {
    static const T def;
    if (is_mandatory)
      throw std::runtime_error("missing " + key + " in " + from);
    return def;
  }
  return it->second.get<T>();
}

static inline std::string json_parse(picojson::value &json_document,
                                     std::string_view json_txt) {
  std::string err;
  auto end =
      picojson::parse(json_document, json_txt.begin(), json_txt.end(), &err);
  // Syntax error in the JSON document itself
  if (!err.empty()) return err;
  // Skip any trailing whitespace
  while (end != json_txt.end() &&
         std::isspace(static_cast<unsigned char>(*end)) != 0)
    ++end;
  // There is extra non‑whitespace content after the parsed JSON
  if (end != json_txt.end()) return "extra content after correct JSON document";
  return {};
}

int Idp_configs::check(MYSQL_THD thd [[maybe_unused]],
                       SYS_VAR *var [[maybe_unused]], void *save,
                       st_mysql_value *value) {
  int value_len{0};
  const char *value_str{value->val_str(value, nullptr, &value_len)};
  if (value_str == nullptr || check(value_str) || value_len == 0) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                 "invalid value for system variable");
    return 1;
  }
  *static_cast<const char **>(save) = value_str;
  return 0;
}

void Idp_configs::update(MYSQL_THD thd [[maybe_unused]],
                         SYS_VAR *var [[maybe_unused]], void *var_ptr,
                         const void *save) {
  update(*static_cast<char *const *>(save),
         static_cast<const char **>(var_ptr));
}

long long Idp_configs::update_keys() noexcept {
  long long no_updated_keys{0};
  try {
    std::vector<std::pair<Config_string, Idp_config>> configs;
    create_tmp_configs(configs);
    for (auto &[key, val] : configs) {
      if (!val.load_keys()) ++no_updated_keys;
    }
    swap_idp_keys(configs);
  } catch (std::exception &e) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, e.what());
    return update_keys_error;
  }
  return no_updated_keys;
}

long long Idp_configs::update_keys(const char *idp_name) noexcept {
  try {
    // This is done in 3 steps to gain max performance with thread safety
    // 1) get JWKS URL with shared lock
    // 2) load new keys (takes longest time) without a lock
    // 3) swaps the keys with unique lock
    // Note 1: if the load fails, the keys container in tmp is empty and
    // effectively the keys will be removed from IDP. Note 2: during 2 the IDP
    // config may be removed, then 3 fails and the function returns an error.
    // That is not effective, but it is an edge case.
    const Config_string jwks_url{get_safe_jwks_url(idp_name)};
    if (jwks_url.empty()) return 0;
    Idp_config config("", jwks_url, "", {}, {});
    const long long result = config.load_keys() ? 0 : 1;
    swap_idp_keys(idp_name, config);
    return result;
  } catch (std::exception &e) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, e.what());
    return update_keys_error;
  }
}

std::unique_ptr<Idp_configs> &Idp_configs::current(const char *sysvar_str) {
  static std::unique_ptr<Idp_configs> current;
  // create an empty configuration if
  if (current == nullptr) current = std::make_unique<Idp_configs>(sysvar_str);
  return current;
}

std::shared_timed_mutex &Idp_configs::mutex() {
  static std::shared_timed_mutex mutex;
  return mutex;
}

void Idp_configs::load(const std::string &config_json) {
  picojson::value json_obj;
  if (const std::string err = json_parse(json_obj, config_json); !err.empty())
    throw std::runtime_error(err);

  if (!json_obj.is<picojson::object>())
    throw std::runtime_error("incorrect configuration structure");

  for (const picojson::object &obj = json_obj.get<picojson::object>();
       const auto &[idp_name, idp_value] : obj)
    load_idp(idp_value, idp_name);
}

/* Ignore -Wdangling-reference for functions accessing picojson objects
 * encapsulated in other picojson. This is safe because parent picojson objects
 * are valid for all life of the functions and this way we avoid copying.
 * Use #if as this check is done only by gcc >= 13
 */
#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ >= 13
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdangling-reference"
#endif

Config_string_map Idp_configs::load_group_roles(
    const picojson::object &idp_object, const std::string &idp_name) {
  static constexpr std::string_view key_group_role{"group-role"};

  Config_string_map roles;

  const picojson::array &roles_array{json_get<picojson::array>(
      idp_object, std::string(key_group_role), idp_name, false)};
  for (const auto &group_role : roles_array) {
    if (!group_role.is<picojson::object>())
      throw std::runtime_error("incorrect group role mapping in " + idp_name);
    const auto &group_role_object{group_role.get<picojson::object>()};
    const auto &group_role_pair{group_role_object.begin()};

    if (group_role_pair == group_role_object.end() ||
        !group_role_pair->second.is<std::string>())
      throw std::runtime_error("incorrect group role mapping in " + idp_name);

    roles.emplace(group_role_pair->first,
                  group_role_pair->second.get<std::string>());
  }
  return roles;
}

Config_string_set Idp_configs::load_audiences(
    const picojson::object &idp_object, const std::string &idp_name) {
  static constexpr std::string_view key_audiences{"audiences"};
  Config_string_set audiences{};
  const picojson::array &audience_array{json_get<picojson::array>(
      idp_object, std::string(key_audiences), idp_name, false)};
  for (const auto &audience : audience_array) {
    audiences.insert(Config_string(audience.get<std::string>()));
  }
  return audiences;
}

void Idp_configs::load_idp(const picojson::value &idp_value,
                           const std::string &idp_name) noexcept {
  static constexpr std::string_view key_issuer_name{"issuer-name"};
  static constexpr std::string_view key_group_claim{"group-claim"};
  static constexpr std::string_view key_jwks_url{"jwks-url"};
  // we catch all exceptions here, so if one IDP is misconfigured,
  // configuration of other IDPs can be loaded
  try {
    if (!idp_value.is<picojson::object>())
      throw std::runtime_error("incorrect IdP definition of " + idp_name);
    const picojson::object &idp_object = idp_value.get<picojson::object>();

    const std::string &jwks_url{json_get<std::string>(
        idp_object, std::string(key_jwks_url), idp_name, false)};

    Idp_config &config{
        idp_configs
            .emplace(idp_name,
                     Idp_config(json_get<std::string>(
                                    idp_object, std::string(key_issuer_name),
                                    idp_name),
                                jwks_url,
                                json_get<std::string>(
                                    idp_object, std::string(key_group_claim),
                                    idp_name, false),
                                load_audiences(idp_object, idp_name),
                                load_group_roles(idp_object, idp_name)))
            .first->second};

    if (jwks_url.empty()) {
      const picojson::array &key_array{json_get<picojson::array>(
          idp_object, std::string(key_keys), idp_name)};
      config.load_keys(key_array, idp_name);
    } else if (config.load_keys()) {
      const std::string message{
          "configuration of " + idp_name +
          " successfully parsed, but failed to load keys"};
      LogPluginErr(WARNING_LEVEL, ER_LOG_PRINTF_MSG, message.c_str());
      return;
    }
    const std::string message{"configuration of " + idp_name +
                              " successfully parsed"};
    LogPluginErr(INFORMATION_LEVEL, ER_LOG_PRINTF_MSG, message.c_str());
  } catch (const std::exception &e) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, e.what());
  } catch (...) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                 "unknown error while parsing IDP configuration");
  }
}

bool Idp_config::load_keys() noexcept {
  try {
    if (jwks.get_url().empty()) return true;
    const std::string body = jwks.http_get();
    picojson::value root;
    const std::string err = json_parse(root, body);
    if (!err.empty()) {
      throw std::runtime_error("JWKS: invalid JSON: " + err);
    }

    if (!root.is<picojson::object>()) {
      throw std::runtime_error("JWKS: JSON root is not an object");
    }
    const picojson::object &root_object = root.get<picojson::object>();

    const picojson::array &key_array{json_get<picojson::array>(
        root_object, std::string(key_keys), jwks.get_url())};

    load_keys(key_array, jwks.get_url());
  }
  // SECURITY: Remove the keys on any error to prevent accepting compromised
  // keys. If loading fails partway through, it's better to have no keys than to
  // risk accepting tokens signed with potentially compromised keys.
  // This follows the principle of "fail secure" - better to deny access
  // than to allow potentially unauthorized access.
  catch (const std::exception &e) {
    keys.clear();
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, e.what());
    return true;
  } catch (...) {
    keys.clear();
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                 "unknown error while loading keys from JWKS");
    return true;
  }
  return false;
}

void Idp_config::load_keys(const picojson::array &key_array,
                           const std::string &from) {
  static const std::string key_kty{"kty"};
  static const std::string key_kid{"kid"};
  static constexpr std::string_view key_kty_rsa{"RSA"};
  static constexpr std::string_view key_kty_ec{"EC"};
  static const std::string key_rsa_n{"n"};
  static const std::string key_rsa_e{"e"};
  static const std::string key_ec_crv{"crv"};
  static const std::string key_ec_x{"x"};
  static const std::string key_ec_y{"y"};

  // for reload case
  keys.clear();

  for (const auto &key_value : key_array) {
    if (!key_value.is<picojson::object>())
      throw std::runtime_error("incorrect keys definition of " + from);

    const picojson::object &key_object{key_value.get<picojson::object>()};
    const std::string &kty{json_get<std::string>(key_object, key_kty, from)};
    const Config_string kid{json_get<std::string>(key_object, key_kid, from)};

    Config_string pem_key;
    if (kty == key_kty_rsa) {
      Rsa_jwk rsa_jwk(json_get<std::string>(key_object, key_rsa_n, from),
                      json_get<std::string>(key_object, key_rsa_e, from));
      pem_key = rsa_jwk.to_pem();
    } else if (kty == key_kty_ec) {
      Ec_jwk ec_jwk(json_get<std::string>(key_object, key_ec_crv, from),
                    json_get<std::string>(key_object, key_ec_x, from),
                    json_get<std::string>(key_object, key_ec_y, from));
      pem_key = ec_jwk.to_pem();
    } else
      throw std::runtime_error(std::string("invalid kty in ") + from);

    keys[kid] = std::move(pem_key);
  }
  const std::string message{"public keys from " + from + " loaded"};
  LogPluginErr(INFORMATION_LEVEL, ER_LOG_PRINTF_MSG, message.c_str());
}
#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ >= 13
#pragma GCC diagnostic pop
#endif

char Idp_configs::parse_prefix(const std::string &prefix) {
  static_assert(prefix_len > 0);

  if (prefix.size() != prefix_len) return 0;

  // Case-insensitive prefix check
  if (std::toupper(prefix[0]) == file_prefix[0]) {
    for (size_t i = 0; i < prefix_len; ++i) {
      if (std::toupper(prefix[i]) != file_prefix[i]) return 0;
    }
    return 'F';
  }
  if (std::toupper(prefix[0]) == json_prefix[0]) {
    for (size_t i = 0; i < prefix_len; ++i) {
      if (std::toupper(prefix[i]) != json_prefix[i]) return 0;
    }
    return 'J';
  }

  return 0;
}

std::string Idp_configs::read_from_file(const std::string &path) {
  std::ifstream file(path, std::ios::binary | std::ios::ate);
  if (!file) {
    throw std::runtime_error("cannot open config file: " + path);
  }

  const auto size = file.tellg();
  if (size < 0) {
    throw std::runtime_error("cannot determine size of config file: " + path);
  }

  std::string content(size, '\0');
  file.seekg(0, std::ios::beg);
  if (!file.read(content.data(), size)) {
    throw std::runtime_error("cannot read config file: " + path);
  }

  return content;
}

void Idp_configs::parse_var(const char *variable, std::string &config_json) {
  const std::string config_var{variable};
  if (config_var.size() < prefix_len)
    throw std::runtime_error(
        "sysvar too short, expected prefix FILE:// or JSON://");
  const std::string prefix{config_var.substr(0, prefix_len)};
  switch (parse_prefix(prefix)) {
    case 'F':
      config_json = read_from_file(config_var.substr(prefix_len));
      break;
    case 'J':
      config_json = config_var.substr(prefix_len);
      break;
    default:
      throw std::runtime_error(
          "invalid sysvar prefix, expected FILE:// or JSON://");
  }
}

bool Idp_configs::check(const char *variable) noexcept {
  try {
    std::string config_json;
    parse_var(variable, config_json);
    picojson::value json_obj;
    const std::string err{json_parse(json_obj, config_json)};
    if (err.empty()) return false;
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, err.c_str());
  } catch (const std::exception &e) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, e.what());
  } catch (...) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                 "unknown error while checking sysvar");
  }
  return true;
}

void Idp_configs::update(const char *variable,
                         const char **sysvar_ptr) noexcept {
  // This function updates the configuration which must be done in read-only
  // mode. In order to minimize the locking time, a new configuration is created
  // and loaded out of the locks. If success, the lock is set for a short time
  // of swaping old and new configuration.
  try {
    auto new_configs = std::make_unique<Idp_configs>(variable);
    std::string config_json;
    parse_var(variable, config_json);
    new_configs->load(config_json);
    Idp_config_statistics new_statistics{new_configs->collect_statistics()};
    const std::unique_lock lock(mutex(), lock_timeout);
    if (!lock.owns_lock())
      throw std::runtime_error(
          "failed to acquire unique lock on configuration");
    current().swap(new_configs);
    *sysvar_ptr = current()->sysvar_str.c_str();
    statistics = new_statistics;
  } catch (const std::exception &e) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, e.what());
  } catch (...) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                 "failed to update configuration");
  }
}

void Idp_configs::set(const char *variable) noexcept {
  // This function sets the configuration on plugin init, so it is safe to set
  // the config directly
  try {
    std::string config_json;
    parse_var(variable, config_json);
    current(variable)->load(config_json);
    statistics = current()->collect_statistics();
  } catch (const std::exception &e) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, e.what());
  } catch (...) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                 "failed to update configuration");
  }
}

std::string Idp_configs::verify_token(
    const Id_token &token, const std::string &idp_name,
    const std::string &ext_user,
    const std::vector<std::pair<std::string, std::string>> &groups_to_proxied,
    std::string &roles) {
  // No change to the configuration is allowed while verifying the token,
  // use lock
  const std::shared_lock lock(mutex(), lock_timeout);
  if (!lock.owns_lock())
    throw std::runtime_error("failed to acquire shared lock on configuration");
  return token.verify(ext_user, groups_to_proxied, current()->get_idp(idp_name),
                      roles);
}
