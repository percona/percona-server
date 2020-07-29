/* Copyright (c) 2019 Francisco Miguel Biete Banon. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   51 Franklin Street, Suite 500, Boston, MA 02110-1335 USA */

#include "plugin/auth_ldap/include/plugin_simple.h"
#include "plugin/auth_ldap/include/connection.h"
#include "plugin/auth_ldap/include/plugin_common.h"
#include "plugin/auth_ldap/include/plugin_log.h"
#include "plugin/auth_ldap/include/plugin_variables.h"
#include "plugin/auth_ldap/include/pool.h"

#include <condition_variable>
#include <mutex>

static const auto password_question = "\5";

mysql::plugin::auth_ldap::Ldap_logger *g_logger_server;

MYSQL_PLUGIN auth_ldap_simple_plugin_info;

namespace {
std::mutex active_m;
std::condition_variable active_cv;
int active_connections = 0;
}  // namespace

// Declaration to access the name of the SYS_VAR
struct SYS_VAR {
  MYSQL_PLUGIN_VAR_HEADER;
};

template <typename Copy_type>
void update_sysvar(THD *, SYS_VAR *var, void *var_ptr, const void *value) {
  // Update the value
  *(Copy_type *)var_ptr = *(Copy_type *)const_cast<void *>(value);

  if (strcmp(var->name, "authentication_ldap_simple_log_status") == 0)
    g_logger_server->set_log_level(
        static_cast<mysql::plugin::auth_ldap::ldap_log_level>(log_status));
  else {
    connPool->reconfigure(init_pool_size, max_pool_size,
                          str_or_empty(server_host), server_port, ssl, tls,
                          str_or_empty(ca_path), str_or_empty(bind_root_dn),
                          str_or_empty(bind_root_pwd));
    connPool->debug_info();
  }
}

static int auth_ldap_simple_init(MYSQL_PLUGIN plugin_info) {
  g_logger_server = new mysql::plugin::auth_ldap::Ldap_logger();
  g_logger_server->set_log_level(
      static_cast<mysql::plugin::auth_ldap::ldap_log_level>(log_status));
  log_srv_dbg("Ldap_logger initialized");

  auth_ldap_common_init();
  log_srv_dbg("auth_ldap_simple_init()");

  log_srv_dbg("Creating LDAP connection pool");
  connPool = new mysql::plugin::auth_ldap::Pool(
      init_pool_size, max_pool_size, str_or_empty(server_host), server_port,
      ssl, tls, str_or_empty(ca_path), str_or_empty(bind_root_dn),
      str_or_empty(bind_root_pwd));
  connPool->debug_info();

  auth_ldap_simple_plugin_info = plugin_info;
  log_srv_info("Plugin initialized");

  {
    std::unique_lock<std::mutex> l{active_m};
    active_connections = 0;
  }

  return 0;
}

static int auth_ldap_simple_deinit(MYSQL_PLUGIN plugin_info
                                   __attribute__((unused))) {
  log_srv_dbg("auth_ldap_simple_deinit()");

  {
    std::unique_lock<std::mutex> l{active_m};
    active_cv.wait(l, [&] { return active_connections <= 0; });
    if (active_connections < 0) {
      return 0;
    }
    active_connections--;
  }

  auth_ldap_common_deinit(connPool);

  delete g_logger_server;
  auth_ldap_simple_plugin_info = nullptr;
  return 0;
}

int mpaldap_simple_authenticate(MYSQL_PLUGIN_VIO *vio,
                                MYSQL_SERVER_AUTH_INFO *info) {
  {
    std::unique_lock<std::mutex> l{active_m};
    if (active_connections < 0) {
      // plugin was uninstalled
      return CR_ERROR;
    }
    active_connections++;
    active_cv.notify_one();
  }

  log_srv_dbg("mpaldap_simple_authenticate()");

  // mysql_clear_password
  unsigned char *password;

  // send the password question
  if (vio->write_packet(vio,
                        static_cast<const unsigned char *>(
                            static_cast<const void *>(password_question)),
                        1)) {
    log_srv_error("Failed to write password question");
    std::unique_lock<std::mutex> l{active_m};
    active_connections--;
    active_cv.notify_one();
    return CR_ERROR;
  }

  // read the password
  if ((vio->read_packet(vio, &password)) < 0) {
    log_srv_error("Failed to read password packet");
    std::unique_lock<std::mutex> l{active_m};
    active_connections--;
    active_cv.notify_one();
    return CR_ERROR;
  }

  info->password_used = PASSWORD_USED_YES;
  auto ret = auth_ldap_common_authenticate_user(
      vio, info, static_cast<char *>(static_cast<void *>(password)), connPool,
      user_search_attr, group_search_attr, group_search_filter, bind_base_dn);

  {
    std::unique_lock<std::mutex> l{active_m};
    active_connections--;
    active_cv.notify_one();
  }

  return ret;
}

// Plugin declaration
struct st_mysql_auth mpaldap_simple_handler = {
    MYSQL_AUTHENTICATION_INTERFACE_VERSION,  // int interface_version
    "mysql_clear_password",                  // const char *client_auth_plugin
    &mpaldap_simple_authenticate,            // authentication function
    &auth_ldap_common_generate_auth_string_hash,  // generate_authentication_string,
    &auth_ldap_common_validate_auth_string_hash,  // validate_authentication_string,
    &auth_ldap_common_set_salt,                   // set_salt,
    0UL,  // const unsigned long authentication_flags
    nullptr};

mysql_declare_plugin(auth_ldap_simple) {
  MYSQL_AUTHENTICATION_PLUGIN,             /* plugin type */
      &mpaldap_simple_handler,             /* type-specific descriptor */
      MPALDAP_SIMPLE_PLUGIN_NAME,          /* plugin name */
      "Francisco Miguel Biete Banon",      /* author */
      "LDAP Simple authentication plugin", /* description */
      PLUGIN_LICENSE_GPL,                  /* license type */
      &auth_ldap_simple_init,              /* init function */
      &auth_ldap_simple_deinit,            /* deinit function */
      nullptr,                             /* no check function */
      0x0100,                              /* version = 1.0 */
      nullptr,                             /* no status variables */
      mpaldap_sysvars,                     /* system variables */
      nullptr                              /* no reserved information */
#if MYSQL_PLUGIN_INTERFACE_VERSION >= 0x103
      ,
      0 /* no flags */
#endif
}
mysql_declare_plugin_end;
