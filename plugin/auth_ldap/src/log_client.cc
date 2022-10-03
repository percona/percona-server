/* Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.
   Copyright (c) 2022, Percona Inc. All Rights Reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "plugin/auth_ldap/include/log_client.h"
#include "include/mysql/services.h"
#ifdef PLUGIN_SIMPLE
#include "plugin/auth_ldap/include/plugin_simple.h"
#endif
#ifdef PLUGIN_SASL
#include "plugin/auth_ldap/include/plugin_sasl.h"
#endif

namespace mysql {
namespace plugin {
namespace auth_ldap {
Ldap_logger::Ldap_logger() {
  m_log_level = LDAP_LOG_LEVEL_NONE;
  m_log_writer = new Ldap_log_writer_error();
}

Ldap_logger::~Ldap_logger() {
  if (m_log_writer) {
    delete m_log_writer;
  }
}

void Ldap_logger::set_log_level(ldap_log_level level) { m_log_level = level; }

void Ldap_log_writer_error::write(ldap_log_type::ldap_type level,
                                  const std::string &data) {
  plugin_log_level plevel = MY_INFORMATION_LEVEL;
  switch (level) {
    case ldap_log_type::LDAP_LOG_LDAP_DBG:
    case ldap_log_type::LDAP_LOG_DBG:
    case ldap_log_type::LDAP_LOG_INFO:
      plevel = MY_INFORMATION_LEVEL;
      break;
    case ldap_log_type::LDAP_LOG_WARNING:
      plevel = MY_WARNING_LEVEL;
      break;
    case ldap_log_type::LDAP_LOG_ERROR:
      plevel = MY_ERROR_LEVEL;
      break;
  };
  my_plugin_log_message(
#ifdef PLUGIN_SIMPLE
      &auth_ldap_simple_plugin_info
#endif
#ifdef PLUGIN_SASL
          & auth_ldap_sasl_plugin_info
#endif
      ,
      plevel, "%s", data.c_str());
}

/**
This class writes error into default error streams.
We needed this constructor because of template class usage.
*/
Ldap_log_writer_error::Ldap_log_writer_error() {}

/**
 */
Ldap_log_writer_error::~Ldap_log_writer_error() {}
}  // namespace auth_ldap
}  // namespace plugin
}  // namespace mysql
