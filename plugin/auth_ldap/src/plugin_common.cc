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

#include "plugin/auth_ldap/include/plugin_common.h"
#include "plugin/auth_ldap/include/auth_ldap_impl.h"
#include "plugin/auth_ldap/include/plugin_log.h"

#include <algorithm>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <list>
#include <string>

int auth_ldap_common_init() { return 0; }

int auth_ldap_common_deinit(mysql::plugin::auth_ldap::Pool *connPool) {
  log_srv_dbg("Destroying LDAP connection pool");
  delete connPool;

  return 0;
}

int auth_ldap_common_authenticate_user(
    MYSQL_PLUGIN_VIO *vio MY_ATTRIBUTE((unused)), MYSQL_SERVER_AUTH_INFO *info,
    const char *password, mysql::plugin::auth_ldap::Pool *pool,
    const char *user_search_attr, const char *group_search_attr,
    const char *group_search_filter, const char *bind_base_dn) {
  std::stringstream log_stream;

  log_srv_dbg("auth_ldap_common_authenticate_user()");

  std::unique_ptr<mysql::plugin::auth_ldap::AuthLDAPImpl> impl =
      std::make_unique<mysql::plugin::auth_ldap::AuthLDAPImpl>(
          str_or_empty(info->user_name), str_or_empty(info->auth_string),
          str_or_empty(user_search_attr), str_or_empty(group_search_filter),
          str_or_empty(group_search_attr), str_or_empty(bind_base_dn), pool);

  // Fin LDAP user dn
  std::string user_dn;
  if (!impl->get_ldap_uid(&user_dn)) {
    log_stream << "LDAP user DN not found for ["
               << str_or_empty(info->user_name) << "]";
    log_srv_warn(log_stream.str());
    return CR_AUTH_USER_CREDENTIALS;
  }

  // Authenticate on ldap
  if (!impl->bind(user_dn, str_or_empty(password))) {
    log_stream << "LDAP user authentication failed for ["
               << str_or_empty(info->user_name) << "] as [" << user_dn << "]";
    log_srv_warn(log_stream.str());
    return CR_AUTH_USER_CREDENTIALS;
  }

  if (strlen(info->authenticated_as) == 0) {
    // Proxy authentication
    std::string user_mysql;
    if (impl->get_mysql_uid(&user_mysql, user_dn)) {
      strcpy(info->authenticated_as, user_mysql.c_str());
    } else {
      log_stream << "MySQL user proxy not found for ["
                 << str_or_empty(info->user_name) << "]";
      log_srv_warn(log_stream.str());
      return CR_AUTH_USER_CREDENTIALS;
    }
  }

  log_stream << "SUCCESS: auth_ldap_common_authenticate_user("
             << str_or_empty(info->user_name) << ") as ["
             << str_or_empty(info->authenticated_as) << "]";
  log_srv_dbg(log_stream.str());

  return CR_OK;
}

int auth_ldap_common_generate_auth_string_hash(char *outbuf,
                                               unsigned int *buflen,
                                               const char *inbuf,
                                               unsigned int inbuflen) {
  /*
    fail if buffer specified by server cannot be copied to output buffer
  */
  if (*buflen < inbuflen) return 1; /* error */
  strncpy(outbuf, inbuf, inbuflen);
  *buflen = strnlen(inbuf, inbuflen);
  return 0; /* success */
}

int auth_ldap_common_validate_auth_string_hash(char *const, unsigned int) {
  return 0; /* success */
}

int auth_ldap_common_set_salt(const char *password __attribute__((unused)),
                              unsigned int password_len __attribute__((unused)),
                              unsigned char *salt __attribute__((unused)),
                              unsigned char *salt_len) {
  *salt_len = 0;
  return 0; /* success */
}
