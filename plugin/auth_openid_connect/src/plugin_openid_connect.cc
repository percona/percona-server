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

#include <curl/curl.h>
#include <jwt-cpp/traits/kazuho-picojson/traits.h>
#include <picojson/picojson.h>
#include <stddef.h>
#include <cassert>
#include <exception>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include <mysql/components/service.h>
#include <mysql/components/services/bits/system_variables_bits.h>
#include <mysql/components/services/log_builtins.h>
#include <mysql/my_loglevel.h>
#include <mysql/plugin.h>
#include <mysql/plugin_auth.h>
#include <mysql/plugin_auth_common.h>
#include <mysqld_error.h>

#include "config.h"
#include "id_token.h"
#include "psi_openid_connect.h"

static SERVICE_TYPE(registry) * reg_srv(nullptr);
SERVICE_TYPE(log_builtins) * log_bi(nullptr);
SERVICE_TYPE(log_builtins_string) * log_bs(nullptr);

class Connection_status {
 private:
  bool authenticated{false};
  bool proxied{false};
  bool using_roles{false};

  void add_status() const {
    static std::mutex lock;
    std::lock_guard guard{lock};
    if (authenticated)
      ++users_authenticated;
    else
      ++users_denied;

    if (proxied) ++users_proxied;

    if (using_roles) ++users_with_roles;
  }

 public:
  ~Connection_status() { add_status(); }
  void set_authenticated() { authenticated = true; }
  void set_proxied() { proxied = true; }
  void set_using_roles() { using_roles = true; }
  static long long users_authenticated;
  static long long users_denied;
  static long long users_proxied;
  static long long users_with_roles;
  static void reset() {
    users_authenticated = 0;
    users_denied = 0;
    users_proxied = 0;
    users_with_roles = 0;
  }
};

long long Connection_status::users_authenticated{0};
long long Connection_status::users_denied{0};
long long Connection_status::users_proxied{0};
long long Connection_status::users_with_roles{0};

/**
 * @brief Initializes the OpenID Connect authentication plugin.
 * @param plugin_info Pointer to the plugin information.
 * @return 0 for success, 1 for error.
 */
static int auth_oidc_init(MYSQL_PLUGIN plugin_info [[maybe_unused]]) {
  Psi_openid_connect::init();
  if (init_logging_service_for_plugin(&reg_srv, &log_bi, &log_bs)) return 1;
  if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, "curl_global_init failed");
    deinit_logging_service_for_plugin(&reg_srv, &log_bi, &log_bs);
    return 1;
  }

  if (Idp_configs::sysvar != nullptr && Idp_configs::sysvar[0] != '\0') {
    if (Idp_configs::check(Idp_configs::sysvar)) {
      LogPluginErr(WARNING_LEVEL, ER_LOG_PRINTF_MSG,
                   "Invalid value of auth_openid_connect_configuration");
    } else {
      Idp_configs::set(Idp_configs::sysvar);
    }
  } else {
    LogPluginErr(INFORMATION_LEVEL, ER_LOG_PRINTF_MSG,
                 "auth_openid_connect_configuration not set.");
  }
  Connection_status::reset();
  return 0;
}

/**
 * @brief Deinitializes the OpenID Connect authentication plugin.
 * @param plugin_info Pointer to the plugin information.
 * @return 0 for success.
 */
static int auth_oidc_deinit(MYSQL_PLUGIN plugin_info [[maybe_unused]]) {
  deinit_logging_service_for_plugin(&reg_srv, &log_bi, &log_bs);
  curl_global_cleanup();
  return 0;
}

/**
 * @class User_auth_data
 * @brief Holds user-specific authentication data extracted from the 'IDENTIFIED
 * AS' clause.
 */
class User_auth_data {
 private:
  std::string idp;        ///< Name of the identity provider.
  std::string ext_user;   ///< External username (subject) in the IDP.
  std::string ext_group;  ///< External group in the IDP.
  std::string error;      ///< Error message if initialization fails.

 public:
  /** @return The IDP name. */
  const std::string &get_idp() const { return idp; }
  /** @return The external user name. */
  const std::string &get_ext_user() const { return ext_user; }
  /** @return The external group name. */
  const std::string &get_ext_group() const { return ext_group; }
  /** @return The error message. */
  const char *get_error() const { return error.c_str(); }

  /**
   * @brief Initializes the User_auth_data from the MySQL auth info.
   *        Sets error message if authentication string is incorrect.
   * @param info Pointer to the MYSQL_SERVER_AUTH_INFO structure.
   * @return true if an error occurred, false otherwise.
   */
  // clang-tidy is absolutely wrong here,
  // the method cannot be static as uses non-static members!
  // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
  bool init(const MYSQL_SERVER_AUTH_INFO *info) {
    picojson::value auth_json;
    const std::string auth(info->auth_string_length > 0 ? info->auth_string
                                                        : "");

    if (const std::string parse_error = picojson::parse(auth_json, auth);
        !parse_error.empty()) {
      error = "invalid IDENTIFIED AS : " + parse_error;
      return true;
    }

    if (!auth_json.is<picojson::object>()) {
      error = "invalid IDENTIFIED AS : not a JSON object";
      return true;
    }

    const auto &obj = auth_json.get<picojson::object>();
    const auto idp_it = obj.find("identity_provider");
    if (idp_it == obj.end() || !idp_it->second.is<std::string>()) {
      error = "missing or invalid identity_provider in IDENTIFIED AS";
      return true;
    }
    idp = idp_it->second.get<std::string>();
    if (const auto it = obj.find("user"); it != obj.end()) {
      if (!it->second.is<std::string>()) {
        error = "invalid user in IDENTIFIED AS";
        return true;
      }
      ext_user = it->second.get<std::string>();
    }
    if (const auto it = obj.find("group"); it != obj.end()) {
      if (!it->second.is<std::string>()) {
        error = "invalid group in IDENTIFIED AS";
        return true;
      }
      ext_group = it->second.get<std::string>();
    }
    return false;
  }
};

/**
 * @brief The main authentication function for the OpenID Connect plugin.
 * @param vio The VIO (Virtual I/O) object for communication with the client.
 * @param info The server authentication information.
 * @return CR_OK, CR_ERROR, or other MySQL authentication status codes.
 */
static int auth_oidc_authenticate(MYSQL_PLUGIN_VIO *vio,
                                  MYSQL_SERVER_AUTH_INFO *info) noexcept {
  assert(vio);
  assert(info);

  try {
    // Ensure that if auth_oidc_authenticate() returns other than CR_OK,
    // the users_denied counter is incremented
    auto status = std::make_unique<Connection_status>();

    // Check if the connection is secured, else we cannot trust the token
    MYSQL_PLUGIN_VIO_INFO vio_info{};
    vio->info(vio, &vio_info);
    if (!vio_info.is_tls_established &&
        vio_info.protocol != MYSQL_PLUGIN_VIO_INFO::MYSQL_VIO_SOCKET &&
        vio_info.protocol != MYSQL_PLUGIN_VIO_INFO::MYSQL_VIO_MEMORY) {
      LogPluginErr(INFORMATION_LEVEL, ER_LOG_PRINTF_MSG,
                   "unsecure connection, use TLS, socket or memory");
      return CR_ERROR;
    }

    User_auth_data auth_data;
    if (auth_data.init(info)) {
      LogPluginErr(INFORMATION_LEVEL, ER_LOG_PRINTF_MSG, auth_data.get_error());
      return CR_ERROR;
    }

    Id_token token;
    if (token.read(vio)) {
      LogPluginErr(INFORMATION_LEVEL, ER_LOG_PRINTF_MSG, token.get_error());
      return CR_ERROR;
    }

    std::string roles;
    auto authenticated_as = Idp_configs::verify_token(
        token, auth_data.get_idp(), auth_data.get_ext_user(),
        auth_data.get_ext_group(), roles);

    if (!authenticated_as.empty()) {
      if (size_t buf_size{std::size(info->authenticated_as)};
          authenticated_as.size() + 1 > buf_size)
        LogPluginErr(INFORMATION_LEVEL, ER_LOG_PRINTF_MSG,
                     "authenticated as name is too long, ignoring user name");
      else {
        std::snprintf(info->authenticated_as, buf_size, "%s",
                      authenticated_as.c_str());
        status->set_proxied();
      }
    }

    if (size_t role_buf_size{std::size(info->external_roles)};
        roles.size() + 1 >= role_buf_size)
      LogPluginErr(WARNING_LEVEL, ER_LOG_PRINTF_MSG,
                   "too many roles, ignoring roles");
    else if (!roles.empty()) {
      std::snprintf(info->external_roles, role_buf_size, "%s", roles.c_str());
      status->set_using_roles();
    }

    LogPluginErr(INFORMATION_LEVEL, ER_LOG_PRINTF_MSG,
                 "authentication successful");
    status->set_authenticated();
    return CR_OK;
  } catch (const std::exception &e) {
    LogPluginErr(INFORMATION_LEVEL, ER_LOG_PRINTF_MSG, e.what());
  } catch (...) {
    LogPluginErr(INFORMATION_LEVEL, ER_LOG_PRINTF_MSG, "authentication failed");
  }

  return CR_ERROR;
}

static int auth_oidc_generate_hash(char *outbuf, unsigned int *buflen,
                                   const char *inbuf,
                                   unsigned int inbuflen) noexcept {
  /*
    fail if the buffer specified by the server cannot be copied to the output
    buffer
  */
  if (*buflen < inbuflen) return 1; /* error */
  strncpy(outbuf, inbuf, inbuflen);
  *buflen = strnlen(inbuf, inbuflen);
  return 0; /* success */
}

static int auth_oidc_validate_hash(char *const, unsigned int) noexcept {
  return 0; /* success */
}

static int auth_oidc_set_salt(const char *password [[maybe_unused]],
                              unsigned int password_len [[maybe_unused]],
                              unsigned char *salt [[maybe_unused]],
                              unsigned char *salt_len) noexcept {
  *salt_len = 0;
  return 0; /* success */
}

static MYSQL_SYSVAR_STR(configuration, Idp_configs::sysvar,
                        PLUGIN_VAR_OPCMDARG | PLUGIN_VAR_STR,
                        "Configuration of OpenId Connect authentication",
                        Idp_configs::check,   // check
                        Idp_configs::update,  // update
                        ""                    // default
);

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
static SYS_VAR *auth_openid_connect_sysvars[] = {MYSQL_SYSVAR(configuration),
                                                 nullptr};

static SHOW_VAR auth_openid_connect_statvars[] = {
    {"auth_openid_connect_users_authenticated",
     reinterpret_cast<char *>(&Connection_status::users_authenticated),
     SHOW_LONGLONG, SHOW_SCOPE_GLOBAL},
    {"auth_openid_connect_users_denied",
     reinterpret_cast<char *>(&Connection_status::users_denied), SHOW_LONGLONG,
     SHOW_SCOPE_GLOBAL},
    {"auth_openid_connect_users_proxied",
     reinterpret_cast<char *>(&Connection_status::users_proxied), SHOW_LONGLONG,
     SHOW_SCOPE_GLOBAL},
    {"auth_openid_connect_users_with_roles",
     reinterpret_cast<char *>(&Connection_status::users_with_roles),
     SHOW_LONGLONG, SHOW_SCOPE_GLOBAL},
    {"auth_openid_connect_configured_idps",
     reinterpret_cast<char *>(&Idp_configs::statistics.configured_idps),
     SHOW_LONGLONG, SHOW_SCOPE_GLOBAL},
    {"auth_openid_connect_configured_idps_using_jwks",
     reinterpret_cast<char *>(
         &Idp_configs::statistics.configured_idps_using_jwks),
     SHOW_LONGLONG, SHOW_SCOPE_GLOBAL},
    {"auth_openid_connect_configured_group_role_maps",
     reinterpret_cast<char *>(&Idp_configs::statistics.configured_role_maps),
     SHOW_LONGLONG, SHOW_SCOPE_GLOBAL},
    {nullptr, nullptr, SHOW_UNDEF, SHOW_SCOPE_GLOBAL}};

/**
 * @brief MySQL authentication plugin interface for OpenID Connect.
 *
 * Defines the plugin interface including authentication, hashing, and
 * validation function pointers.
 */
// NOLINTNEXTLINE(misc-use-internal-linkage)
st_mysql_auth auth_oidc_info = {
    MYSQL_AUTHENTICATION_INTERFACE_VERSION,  // int interface_version
    "authentication_openid_connect_client",  // const char *client_auth_plugin
    auth_oidc_authenticate,                  // authentication function
    auth_oidc_generate_hash,                 // generate_authentication_string,
    auth_oidc_validate_hash,                 // validate_authentication_string,
    auth_oidc_set_salt,                      // set_salt,
    0UL,  // const unsigned long authentication_flags
    nullptr};

// NOLINTNEXTLINE(misc-use-internal-linkage)
mysql_declare_plugin(auth_openid_connect){
    MYSQL_AUTHENTICATION_PLUGIN,             // type
    &auth_oidc_info,                         // info
    "auth_openid_connect",                   // name
    "Percona LLC and/or its affiliates.",    // author
    "OpenID Connect authentication plugin",  // description
    PLUGIN_LICENSE_GPL,                      // license
    auth_oidc_init,                          // init function (when loaded)
    nullptr,                                 // check uninstall function
    auth_oidc_deinit,                        // deinit function (when unloaded)
    0x0001,                                  // version
    auth_openid_connect_statvars,            // status variables
    auth_openid_connect_sysvars,             // system variables
    nullptr,                                 // reserved
    0,                                       // flags
} mysql_declare_plugin_end;
