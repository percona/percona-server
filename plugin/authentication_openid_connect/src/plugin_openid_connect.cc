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

#include <_string.h>
#include <jwt-cpp/jwt.h>
#include <jwt-cpp/traits/kazuho-picojson/defaults.h>
#include <jwt-cpp/traits/kazuho-picojson/traits.h>
#include <cassert>
#include <cstring>
#include <exception>
#include <map>
#include <string>

#include "mysql/components/services/bits/system_variables_bits.h"
#include "mysql/my_loglevel.h"
#include "mysql/service_thd_alloc.h"
#include "mysql_com.h"

#include <mysql/components/service.h>
#include <mysql/components/services/log_builtins.h>
#include <mysql/plugin_auth.h>
#include <mysql/plugin_auth_common.h>
#include <mysqld_error.h>

#include "config.h"

SERVICE_TYPE(registry) * reg_srv(nullptr);
SERVICE_TYPE(log_builtins) * log_bi(nullptr);
SERVICE_TYPE(log_builtins_string) * log_bs(nullptr);

static auto get_verifier(const std::string &name, const std::string &key) {
  if (name == "RS256") {
    return jwt::verify().allow_algorithm(jwt::algorithm::rs256(key));
  } else if (name == "RS384") {
    return jwt::verify().allow_algorithm(jwt::algorithm::rs384(key));
  } else if (name == "RS512") {
    return jwt::verify().allow_algorithm(jwt::algorithm::rs512(key));
  } else if (name == "ES256") {
    return jwt::verify().allow_algorithm(jwt::algorithm::es256(key));
  } else if (name == "HS256") {
    return jwt::verify().allow_algorithm(jwt::algorithm::hs256(key));
  }
  throw std::runtime_error("Unsupported algorithm: " + name);
}

static int auth_oidc_init(MYSQL_PLUGIN plugin_info [[maybe_unused]]) {
  if (init_logging_service_for_plugin(&reg_srv, &log_bi, &log_bs)) return 1;

  return 0;
}

static int auth_oidc_deinit(MYSQL_PLUGIN plugin_info [[maybe_unused]]) {
  deinit_logging_service_for_plugin(&reg_srv, &log_bi, &log_bs);
  return 0;
}

class User_auth_data {
 private:
  std::string idp;
  std::string ext_user;
  std::string error;

 public:
  const std::string &get_idp() const { return idp; }
  const std::string &get_ext_user() const { return ext_user; }
  const char *get_error() const { return error.c_str(); }
  bool init(const MYSQL_SERVER_AUTH_INFO *info) {
    picojson::value auth_json;
    const std::string auth(info->auth_string_length > 0 ? info->auth_string
                                                        : "");

    if (std::string parse_error = picojson::parse(auth_json, auth);
        !parse_error.empty()) {
      error = "invalid IDENTIFIED AS : " + parse_error;
      return true;
    }

    if (!auth_json.is<picojson::object>()) {
      error = "invalid IDENTIFIED AS : not a JSON object";
      return true;
    }

    const auto &obj = auth_json.get<picojson::object>();
    idp = obj.at("identity_provider").get<std::string>();
    ext_user = obj.at("user").get<std::string>();

    return false;
  }
};

class Id_token {
 private:
  std::string token;
  std::string error;

 public:
  const char *get_error() const { return error.c_str(); }
  bool read(MYSQL_PLUGIN_VIO *vio) {
    unsigned char *pos(nullptr);
    int len_to_parse = vio->read_packet(vio, &pos);

    // 1. field: capability
    // ensure the packet is long enough to hold the field
    if (len_to_parse < 1) {
      error = "malformed packet";
      return true;
    }
    // skip the field
    pos++;
    len_to_parse--;

    // 2. field: token length
    // ensure the packet is long enough to hold the field
    len_to_parse -= net_field_length_size(pos);
    if (len_to_parse < 1) {
      error = "malformed packet";
      return true;
    }
    // get token length and move pos to the 3. field: the token
    uint64_t token_len = net_field_length_ll(&pos);
    // check if token length is correct
    if (token_len > static_cast<uint64_t>(len_to_parse)) {
      error = "malformed packet";
      return true;
    }
    token = std::string(reinterpret_cast<char *>(pos), token_len);
    return false;
  }

  void verify(const std::string &ext_user, const std::string &idp_name,
              const std::string &pub_key) const {
    auto decoded_token = jwt::decode(token);
    auto verifier =
        get_verifier(decoded_token.get_header_claim("alg").as_string(), pub_key)
            .with_claim("iss", jwt::claim(idp_name))
            .with_claim("sub", jwt::claim(ext_user));

    verifier.verify(decoded_token);
  }
};

static int auth_oidc_authenticate(MYSQL_PLUGIN_VIO *vio,
                                  MYSQL_SERVER_AUTH_INFO *info) {
  assert(vio);
  assert(info);

  try {
    // Check if the connection is secured, else we cannot trust the token
    MYSQL_PLUGIN_VIO_INFO vio_info{};
    vio->info(vio, &vio_info);
    if (!vio_info.is_tls_established &&
        vio_info.protocol != MYSQL_PLUGIN_VIO_INFO::MYSQL_VIO_SOCKET &&
        vio_info.protocol != MYSQL_PLUGIN_VIO_INFO::MYSQL_VIO_MEMORY) {
      LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                   "unsecure connection, use TLS, socket or memory");
      return CR_ERROR;
    }

    User_auth_data auth_data;
    if (auth_data.init(info)) {
      LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, auth_data.get_error());
      return CR_ERROR;
    }

    Id_token token;
    if (token.read(vio)) {
      LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, token.get_error());
      return CR_ERROR;
    }

    const Idp_config *idp = Idp_configs::get_item(auth_data.get_idp());
    if (idp == nullptr) {
      LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, "IDP not configured");
      return CR_ERROR;
    }

    token.verify(auth_data.get_ext_user(), idp->name, idp->pub_key);
    LogPluginErr(INFORMATION_LEVEL, ER_LOG_PRINTF_MSG,
                 "authentication successful");
    return CR_OK;
  } catch (const std::exception &e) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, e.what());
  } catch (...) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, "authentication failed");
  }

  return CR_ERROR;
}

static int auth_oidc_generate_hash(char *outbuf, unsigned int *buflen,
                                   const char *inbuf, unsigned int inbuflen) {
  /*
    fail if buffer specified by server cannot be copied to output buffer
  */
  if (*buflen < inbuflen) return 1; /* error */
  strncpy(outbuf, inbuf, inbuflen);
  *buflen = strnlen(inbuf, inbuflen);
  return 0; /* success */
}

static int auth_oidc_validate_hash(char *const, unsigned int) {
  return 0; /* success */
}

static int auth_oidc_set_salt(const char *password __attribute__((unused)),
                              unsigned int password_len __attribute__((unused)),
                              unsigned char *salt __attribute__((unused)),
                              unsigned char *salt_len) {
  *salt_len = 0;
  return 0; /* success */
}

static MYSQL_SYSVAR_STR(configuration, Idp_configs::sysvar, PLUGIN_VAR_OPCMDARG,
                        "Configuration of OpenId Connect authentication",
                        Idp_configs::check /* check */,
                        Idp_configs::update /* update */, "" /* default */);

static SYS_VAR *authentication_openid_connect_sysvars[] = {
    MYSQL_SYSVAR(configuration), nullptr};

struct st_mysql_auth auth_oidc_info = {
    MYSQL_AUTHENTICATION_INTERFACE_VERSION,  // int interface_version
    "authentication_openid_connect_client",  // const char *client_auth_plugin
    auth_oidc_authenticate,                  // authentication function
    auth_oidc_generate_hash,                 // generate_authentication_string,
    auth_oidc_validate_hash,                 // validate_authentication_string,
    auth_oidc_set_salt,                      // set_salt,
    0UL,  // const unsigned long authentication_flags
    nullptr};

mysql_declare_plugin(authentication_openid_connect){
    MYSQL_AUTHENTICATION_PLUGIN,          /* type                            */
    &auth_oidc_info,                      /* info                      */
    "authentication_openid_connect",      /* name                            */
    "Percona LLC and/or its affiliates.", /* author                          */
    "OpenID Connect authentication plugin", /* description */
    PLUGIN_LICENSE_GPL,
    auth_oidc_init,                        /* init function (when loaded)     */
    nullptr,                               /* check uninstall function        */
    auth_oidc_deinit,                      /* deinit function (when unloaded) */
    0x0001,                                /* version                         */
    nullptr,                               /* status variables                */
    authentication_openid_connect_sysvars, /* system variables                */
    nullptr,
    0,
} mysql_declare_plugin_end;
