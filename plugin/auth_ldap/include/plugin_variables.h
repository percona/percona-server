#ifndef _PLUGIN_VARIABLES_MPALDAP_H
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
#define _PLUGIN_VARIABLES_MPALDAP_H

#include "plugin/auth_ldap/include/plugin_common.h"
#include "plugin/auth_ldap/include/pool.h"

struct SYS_VAR;

// If we define static variables in a header file we will have a copy of that
// variable in each of the compilation units that include that header. Usually
// not something we want, but perfect in this case to avoid duplicate variable
// definitions for the simple and sasl plugins.

// Static plugin variables
static char *auth_method_name;
static char *bind_base_dn;
static char *bind_root_dn;
static char *bind_root_pwd;
static char *ca_path;
static char *group_search_attr;
static char *group_search_filter;
static unsigned int init_pool_size;
static int log_status;
static unsigned int max_pool_size;
static char *server_host;
static unsigned int server_port;
static bool ssl;
static bool tls;
static char *user_search_attr;

static mysql::plugin::auth_ldap::Pool *connPool;

template <typename Copy_type>
void update_sysvar(THD *, SYS_VAR *var, void *tgt, const void *save);

// System Variables
static MYSQL_SYSVAR_STR(auth_method_name, auth_method_name,
                        PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_MEMALLOC,
                        "The authentication method name", nullptr /* check */,
                        &update_sysvar<char *> /* update */,
                        "SIMPLE" /* default */);
static MYSQL_SYSVAR_STR(bind_base_dn, bind_base_dn,
                        PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_MEMALLOC,
                        "The base distinguished name (DN)", nullptr /* check */,
                        &update_sysvar<char *> /* update */,
                        nullptr /* default */);
static MYSQL_SYSVAR_STR(bind_root_dn, bind_root_dn,
                        PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_MEMALLOC,
                        "The root distinguished name (DN)", nullptr /* check */,
                        &update_sysvar<char *> /* update */,
                        nullptr /* default */);
static MYSQL_SYSVAR_STR(bind_root_pwd, bind_root_pwd,
                        PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_MEMALLOC,
                        "The password for the "
                        "root distinguished name",
                        nullptr /* check */,
                        &update_sysvar<char *> /* update */,
                        nullptr /* default */);
static MYSQL_SYSVAR_STR(ca_path, ca_path,
                        PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_MEMALLOC,
                        "The absolute path of "
                        "the certificate authority file",
                        nullptr /* check */,
                        &update_sysvar<char *> /* update */,
                        nullptr /* default */);
static MYSQL_SYSVAR_STR(group_search_attr, group_search_attr,
                        PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_MEMALLOC,
                        "The name of the attribute that specifies "
                        "group names in LDAP directory entries",
                        nullptr /* check */,
                        &update_sysvar<char *> /* update */,
                        "cn" /* default */);
static MYSQL_SYSVAR_STR(group_search_filter, group_search_filter,
                        PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_MEMALLOC,
                        "The custom group search filter", nullptr /* check */,
                        &update_sysvar<char *> /* update */,
                        "(|(&(objectClass=posixGroup)(memberUid={UA}))(&("
                        "objectClass=group)(member={UD})))" /* default */);
static MYSQL_SYSVAR_UINT(init_pool_size, init_pool_size, PLUGIN_VAR_RQCMDARG,
                         "The initial size of "
                         "the pool of connections to the LDAP server",
                         nullptr /* check */,
                         &update_sysvar<unsigned int> /* update */,
                         10 /* default */, 1 /*minimum */, 32767 /* maximum */,
                         0 /* blocksize */);
static MYSQL_SYSVAR_UINT(max_pool_size, max_pool_size, PLUGIN_VAR_RQCMDARG,
                         "The maximum size of "
                         "the pool of connections to the LDAP server",
                         nullptr /* check */,
                         &update_sysvar<unsigned int> /* update */,
                         1000 /* default */, 0 /*minimum */,
                         32767 /* maximum */, 0 /* blocksize */);
static MYSQL_SYSVAR_INT(log_status, log_status, PLUGIN_VAR_RQCMDARG,
                        "The logging level", nullptr /* check */,
                        &update_sysvar<unsigned int> /* update */,
                        1 /* default */, 1 /*minimum */, 5 /* maximum */,
                        0 /* blocksize */);
static MYSQL_SYSVAR_STR(server_host, server_host,
                        PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_MEMALLOC,
                        "The LDAP server host", nullptr /* check */,
                        &update_sysvar<char *> /* update */,
                        nullptr /* default */);
static MYSQL_SYSVAR_UINT(server_port, server_port, PLUGIN_VAR_RQCMDARG,
                         "The LDAP server TCP/IP port number",
                         nullptr /* check */,
                         &update_sysvar<unsigned int> /* update */,
                         389 /* default */, 1 /*minimum */, 32376 /* maximum */,
                         0 /* blocksize */);
static MYSQL_SYSVAR_BOOL(
    ssl, ssl, PLUGIN_VAR_RQCMDARG,
    "Whether connections "
    "by the plugin to the LDAP server are using the SSL protocol (ldaps://)",
    nullptr /* check */, &update_sysvar<bool> /* update */,
    false /* default */);
static MYSQL_SYSVAR_BOOL(
    tls, tls, PLUGIN_VAR_RQCMDARG,
    "Whether connections "
    "by the plugin to the LDAP server are secured with STARTTLS (ldap://)",
    nullptr /* check */, &update_sysvar<bool> /* update */,
    false /* default */);
static MYSQL_SYSVAR_STR(user_search_attr, user_search_attr,
                        PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_MEMALLOC,
                        "The name of the attribute that specifies "
                        "user names in LDAP directory entries",
                        nullptr /* check */,
                        &update_sysvar<char *> /* update */,
                        "uid" /* default */);

static SYS_VAR *mpaldap_sysvars[] = {MYSQL_SYSVAR(auth_method_name),
                                     MYSQL_SYSVAR(bind_base_dn),
                                     MYSQL_SYSVAR(bind_root_dn),
                                     MYSQL_SYSVAR(bind_root_pwd),
                                     MYSQL_SYSVAR(ca_path),
                                     MYSQL_SYSVAR(group_search_attr),
                                     MYSQL_SYSVAR(group_search_filter),
                                     MYSQL_SYSVAR(init_pool_size),
                                     MYSQL_SYSVAR(log_status),
                                     MYSQL_SYSVAR(max_pool_size),
                                     MYSQL_SYSVAR(server_host),
                                     MYSQL_SYSVAR(server_port),
                                     MYSQL_SYSVAR(ssl),
                                     MYSQL_SYSVAR(tls),
                                     MYSQL_SYSVAR(user_search_attr),
                                     nullptr};

#endif  // _PLUGIN_VARIABLES_MPALDAP_H
