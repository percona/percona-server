#ifndef _PLUGIN_LOG_MPALDAP_H
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
#define _PLUGIN_LOG_MPALDAP_H

#include "plugin/auth_ldap/include/log_client.h"

extern mysql::plugin::auth_ldap::Ldap_logger *g_logger_server;

#define log_srv_dbg \
  g_logger_server->log<mysql::plugin::auth_ldap::ldap_log_type::LDAP_LOG_DBG>
#define log_srv_info \
  g_logger_server->log<mysql::plugin::auth_ldap::ldap_log_type::LDAP_LOG_INFO>
#define log_srv_warn \
  g_logger_server    \
      ->log<mysql::plugin::auth_ldap::ldap_log_type::LDAP_LOG_WARNING>
#define log_srv_error \
  g_logger_server->log<mysql::plugin::auth_ldap::ldap_log_type::LDAP_LOG_ERROR>

#endif  // _PLUGIN_LOG_MPALDAP_H
