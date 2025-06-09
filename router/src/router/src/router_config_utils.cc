/*
  Copyright (c) 2025, Oracle and/or its affiliates.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2.0,
  as published by the Free Software Foundation.

  This program is designed to work with certain software (including
  but not limited to OpenSSL) that is licensed under separate terms,
  as designated in a particular file or component or in included license
  documentation.  The authors of MySQL hereby grant you an additional
  permission to link the program and your derivative works with the
  separately licensed software that they have either included with
  the program or referenced in the documentation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "mysqlrouter/router_config_utils.h"

#include "mysql/harness/config_parser.h"
#include "socket_operations.h"

std::string get_configured_router_name(const mysql_harness::Config &config,
                                       const uint32_t default_port) {
  auto section = config.get_default_section();
  if (section.has("name")) {
    return section.get("name");
  }

  std::string port_str = std::to_string(default_port);
  for (const mysql_harness::ConfigSection *section : config.sections()) {
    if (section->name == "http_server" && section->has("port")) {
      port_str = section->get("port");
      break;
    }
  }

  auto socket_ops = mysql_harness::SocketOperations::instance();

  return socket_ops->get_local_hostname() + ":" + port_str;
}