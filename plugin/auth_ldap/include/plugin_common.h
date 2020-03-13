#ifndef _PLUGIN_COMMON_MPALDAP_H
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
#define _PLUGIN_COMMON_MPALDAP_H

#include "m_string.h"
#include "my_dbug.h"
#include "my_sys.h"
#include "mysql/components/my_service.h"
#include "mysql/components/service_implementation.h"
#include "mysql/plugin_auth.h"
#include "mysqld_error.h"

#include "plugin/auth_ldap/include/pool.h"

inline const char *str_or_empty(const char *x) { return x == nullptr ? "" : x; }

int auth_ldap_common_init();
int auth_ldap_common_deinit(mysql::plugin::auth_ldap::Pool *connPool);

int auth_ldap_common_authenticate_user(
    MYSQL_PLUGIN_VIO *vio, MYSQL_SERVER_AUTH_INFO *info, const char *password,
    mysql::plugin::auth_ldap::Pool *pool, const char *user_search_attr,
    const char *group_search_attr, const char *group_search_filter,
    const char *bind_base_dn);

int auth_ldap_common_generate_auth_string_hash(char *outbuf,
                                               unsigned int *buflen,
                                               const char *inbuf,
                                               unsigned int inbuflen);

int auth_ldap_common_validate_auth_string_hash(char *const buf,
                                               unsigned int len);

int auth_ldap_common_set_salt(const char *password, unsigned int password_len,
                              unsigned char *salt, unsigned char *salt_len);

#endif  // _PLUGIN_COMMON_MPALDAP_H
