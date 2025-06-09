/*
  Copyright (c) 2021, 2025, Oracle and/or its affiliates.

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

#include "mrs/authentication/mysql_handler.h"

#include <cassert>
#include <utility>

#include "mysql/harness/logging/logging.h"

IMPORT_LOG_FUNCTIONS()

namespace mrs {
namespace authentication {

std::string to_string(const std::set<UniversalId> &ids) {
  std::string result;
  for (const auto &id : ids) {
    if (!result.empty()) result += ",";
    result += id.to_string();
  }
  return result;
}

MysqlHandler::MysqlHandler(const AuthApp &entry,
                           collector::MysqlCacheManager *cache_manager,
                           QueryFactory *qf)
    : WwwAuthenticationHandler(entry, qf), cache_manager_{cache_manager} {
  log_debug("MySQLHandler for service %s", to_string(entry_).c_str());
}

const std::string &MysqlHandler::get_handler_name() const {
  static const std::string k_app_name{
      "MySQL internal authentication application"};

  return k_app_name;
}

std::set<UniversalId> MysqlHandler::get_service_ids() const {
  return entry_.service_ids;
}

UniversalId MysqlHandler::get_id() const { return entry_.id; }

bool MysqlHandler::verify_credential(const Credentials &credentials,
                                     SqlSessionCached *out_cache,
                                     AuthUser *out_user) {
  assert(out_cache);
  assert(out_user);
  const bool k_do_not_allow_update = false;
  try {
    // The MySQL account may be different for different host,
    // even if they use the same user-name.
    //
    // This potential problem should be documented.
    pre_authorize_account(this, credentials.user);

    if (credentials.password.empty()) return false;

    auto default_auth_user =
        out_cache->get()->get_connection_parameters().conn_opts;
    out_cache->get()->change_user(credentials.user, credentials.password, "");
    out_user->vendor_user_id =
        (*out_cache->get()->query_one("SELECT CURRENT_USER();"))[0];

    // Restore default user
    out_cache->get()->change_user(default_auth_user.username,
                                  default_auth_user.password, "");

    out_user->app_id = entry_.id;

    auto ret = um_.user_get(out_user, out_cache, k_do_not_allow_update);
    out_user->is_mysql_auth = true;
    out_user->name = credentials.user;
    out_user->mysql_password = credentials.password;
    return ret;
  } catch (const std::exception &) {
    return false;
  }
}

}  // namespace authentication
}  // namespace mrs
