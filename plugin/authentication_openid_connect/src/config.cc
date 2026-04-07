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

#include <ctype.h>
#include <stddef.h>
#include <exception>
#include <fstream>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>

#include <mysql/components/services/bits/system_variables_bits.h>
#include <mysql/components/services/log_builtins.h>
#include <mysql/my_loglevel.h>
#include <mysql/plugin.h>
#include <mysql/service_thd_alloc.h>
#include <mysqld_error.h>
#include <picojson/picojson.h>

#include "jwk.h"
#include "jwks.h"

Idp_configs *Idp_configs::configs(nullptr);
char *Idp_configs::sysvar(nullptr);

// Declaration to access the name of the SYS_VAR
struct SYS_VAR {
  MYSQL_PLUGIN_VAR_HEADER;
};

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

int Idp_configs::check(MYSQL_THD thd [[maybe_unused]],
                       SYS_VAR *var [[maybe_unused]], void *save,
                       st_mysql_value *value) {
  int value_len(0);
  const Idp_configs *new_config(
      parse_var(value->val_str(value, nullptr, &value_len)));
  if (new_config == nullptr) return 1;
  // need to pass Idp_config* via void* save to retrieve it in update()
  *static_cast<const Idp_configs **>(save) = new_config;
  // TODO what if sth fails between check and update? -> avoid memory leak
  return 0;
}

void Idp_configs::update(MYSQL_THD thd [[maybe_unused]],
                         SYS_VAR *var [[maybe_unused]], void *var_ptr,
                         const void *save) {
  const Idp_configs *prev_config(configs);
  // passed as void* from check()
  configs = *static_cast<Idp_configs *const *>(save);
  *static_cast<const char **>(var_ptr) = configs->config_var.c_str();
  if (prev_config != nullptr) {
    delete prev_config;
  }
}

long long Idp_configs::update_keys() noexcept {
  long long updated_keys{0};
  if (configs == nullptr) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                 "IDP configuration is missing");
    return -1;
  }
  try {
    for (auto &config : configs->idp_configs | std::views::values) {
      if (!config.is_using_jwks()) continue;
      config.update_keys();
    }
  } catch (std::exception &e) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, e.what());
    return -2;
  }
  return updated_keys;
}

long long Idp_configs::update_keys(const char *idp_name) noexcept {
  try {
    Idp_config *config = get_item(idp_name);
    if (config == nullptr) return -1;
    if (!config->is_using_jwks()) return 0;
    config->update_keys();
  } catch (std::exception &e) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, e.what());
    return -2;
  }
  return 1;
}

void Idp_configs::parse_json(const std::string &config_json) {
  picojson::value json_obj;
  if (const std::string err = picojson::parse(json_obj, config_json);
      !err.empty())
    throw std::runtime_error(err);

  if (!json_obj.is<picojson::object>())
    throw std::runtime_error("incorrect configuration structure");

  for (const picojson::object &obj = json_obj.get<picojson::object>();
       const auto &[idp_name, idp_value] : obj) {
    if (!idp_value.is<picojson::object>())
      throw std::runtime_error("incorrect IdP definition of " + idp_name);
    const picojson::object &idp_object = idp_value.get<picojson::object>();

    const std::string &issuer_name{
        json_get<std::string>(idp_object, "issuer-name", idp_name)};

    std::unordered_set<std::string> audiences{};
    const picojson::array &audience_array{
        json_get<picojson::array>(idp_object, "audiences", idp_name, false)};
    for (const auto &audience : audience_array) {
      audiences.insert(audience.get<std::string>());
    }

    const std::string &group_claim{
        json_get<std::string>(idp_object, "group-claim", idp_name, false)};

    std::map<std::string, std::string> roles;

    const picojson::array &roles_array{
        json_get<picojson::array>(idp_object, "group-role", idp_name)};
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

    const std::string &jwks_url{
        json_get<std::string>(idp_object, "jwks-url", idp_name, false)};

    Idp_config &config{
        idp_configs
            .emplace(idp_name,
                     Idp_config(issuer_name, jwks_url, group_claim,
                                std::move(audiences), std::move(roles)))
            .first->second};

    if (jwks_url.empty()) {
      const picojson::array &key_array{
          json_get<picojson::array>(idp_object, "keys", idp_name)};
      config.load_keys(key_array, idp_name);
    } else config.load_keys(jwks_url);
  }
}

void Idp_config::load_keys(const std::string &url) {
  std::string body = Jwks::http_get(url);
  picojson::value root;
  const std::string err = picojson::parse(root, body);
  if (!err.empty()) {
    throw std::runtime_error("JWST: invalid JSON: " + err);
  }

  if (!root.is<picojson::object>()) {
    throw std::runtime_error("JWST: JSON root is not an object");
  }
  const picojson::object &root_object = root.get<picojson::object>();

  const picojson::array &key_array{
      json_get<picojson::array>(root_object, "keys", url)};

  load_keys(key_array, url);
}

void Idp_config::load_keys(const picojson::array &key_array,
                           const std::string &from) {
  // SECURITY: Clear old keys first to prevent accepting compromised keys.
  // If loading fails partway through, it's better to have no keys than to
  // risk accepting tokens signed with potentially compromised keys.
  // This follows the principle of "fail secure" - better to deny access
  // than to allow potentially unauthorized access.
  keys.clear();

  for (const auto &key_value : key_array) {
    if (!key_value.is<picojson::object>())
      throw std::runtime_error("incorrect keys definition of " + from);

    const picojson::object &key_object{key_value.get<picojson::object>()};
    const std::string &kty{json_get<std::string>(key_object, "kty", from)};
    const std::string &kid{json_get<std::string>(key_object, "kid", from)};

    std::string pem_key;
    if (kty == "RSA") {
      Rsa_jwk rsa_jwk(json_get<std::string>(key_object, "n", from),
                      json_get<std::string>(key_object, "e", from));
      pem_key = rsa_jwk.to_pem();
    } else if (kty == "EC") {
      Ec_jwk ec_jwk(json_get<std::string>(key_object, "crv", from),
                    json_get<std::string>(key_object, "x", from),
                    json_get<std::string>(key_object, "y", from));
      pem_key = ec_jwk.to_pem();
    } else
      throw std::runtime_error(std::string("invalid kty in ") + from);

    keys[kid] = std::move(pem_key);
  }
}

char Idp_configs::parse_prefix(const std::string &prefix) {
  static constexpr std::string_view file_prefix{"FILE://"};
  static constexpr std::string_view json_prefix{"JSON://"};
  static_assert(prefix_len > 0);

  if (prefix.size() != prefix_len) return 0;

  // Case-insensitive prefix check
  if (prefix[0] == file_prefix[0]) {
    for (size_t i = 0; i < prefix_len; ++i) {
      if (std::toupper(prefix[i]) != file_prefix[i]) return 0;
    }
    return 'F';
  }
  if (prefix[0] == json_prefix[0]) {
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

Idp_configs *Idp_configs::parse_var(const std::string &config_var) noexcept {
  // nothing changed
  // if (config_var == sysvar) return nullptr;

  try {
    if (config_var.size() < prefix_len)
      throw std::runtime_error("invalid prefix");
    const std::string prefix = config_var.substr(0, prefix_len);
    std::string config_json;
    switch (parse_prefix(prefix)) {
      case 'F':
        config_json = read_from_file(config_var.substr(prefix_len - 1));
        break;
      case 'J':
        config_json = config_var.substr(prefix_len);
        break;
      default:
        throw std::runtime_error("invalid prefix");
    }

    return new Idp_configs(config_var, config_json);
  } catch (const std::exception &e) {
    std::string error_msg("configuration error: ");
    error_msg += e.what();
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, error_msg.c_str());
  } catch (...) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, "configuration error");
  }
  return nullptr;
}