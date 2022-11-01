/* Copyright (c) 2019 Francisco Miguel Biete Banon. All rights reserved.
   Copyright (c) 2022, Percona Inc. All Rights Reserved.

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

#include "plugin/auth_ldap/include/auth_ldap_impl.h"
#include "plugin/auth_ldap/include/connection.h"
#include "plugin/auth_ldap/include/plugin_log.h"

#include <algorithm>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

namespace mysql {
namespace plugin {
namespace auth_ldap {

static std::map<std::string, std::string> calc_group_mappings(
    std::string const &mapping) {
  std::map<std::string, std::string> group_mapping;
  std::vector<std::string> roles;
  boost::algorithm::split(roles, mapping, boost::is_any_of(","));
  for (auto const &role : roles) {
    std::vector<std::string> r;
    boost::algorithm::split(r, role, boost::is_any_of("="));
    if (r.size() == 1) {
      group_mapping[role] = role;
    } else {
      group_mapping[r[0]] = r[1];
    }
  }
  return group_mapping;
}

AuthLDAPImpl::AuthLDAPImpl(const std::string &user_name,
                           const std::string &auth_string,
                           const std::string &user_search_attr,
                           const std::string &group_search_filter,
                           const std::string &group_search_attr,
                           const std::string &bind_base_dn,
                           const std::string &mapping, Pool *pool)
    : pool_(pool),
      user_search_attr_(user_search_attr),
      group_search_attr_(group_search_attr),
      group_search_filter_(group_search_filter),
      bind_base_dn_(bind_base_dn),
      user_name_(user_name),
      group_role_mapping_(calc_group_mappings(mapping)) {
  std::vector<std::string> parts;
  boost::algorithm::split(parts, auth_string, boost::is_any_of("#"));
  user_auth_string_ = boost::algorithm::trim_copy(parts[0]);
  if (parts.size() == 2) {
    std::string raw_groupmappings_ = boost::algorithm::trim_copy(parts[1]);
    if (!raw_groupmappings_.empty()) calc_mappings(raw_groupmappings_);
  }
}

AuthLDAPImpl::~AuthLDAPImpl() {}

bool AuthLDAPImpl::bind_internal(const std::string &user_dn,
                                 const std::string &password,
                                 Pool::pool_ptr_t *conn_out) {
  log_srv_dbg("AuthLDAPImpl::bind()");
  bool success = false;
  std::ostringstream log_stream;

  auto conn = pool_->borrow_connection(false);
  if (conn == nullptr) return false;

  std::string resp;
  if (conn->connect(user_dn, password, resp) == Connection::status::SUCCESS) {
    log_stream << "User authentication success: [" << user_dn << "]";
    success = true;
  } else {
    log_stream << "User authentication failed: [" << user_dn << "]";
  }
  log_srv_dbg(log_stream.str());

  if (conn_out == nullptr || !success) {
    pool_->return_connection(conn);
  } else {
    *conn_out = conn;
  }

  return success;
}

bool AuthLDAPImpl::bind_internal(sasl_ctx &ctx, std::string const &user_dn,
                                 Pool::pool_ptr_t *conn_out) {
  log_srv_dbg("AuthLDAPImpl::bind()");
  bool success = false;
  std::ostringstream log_stream;

  auto conn = pool_->borrow_connection(false);
  if (conn == nullptr) return false;

  bool first = true;

  Connection::status res;

  do {
    auto cdata = ctx.get_client_data();
    std::string sdata;
    if (first) {
      res = conn->connect(user_dn, cdata, sdata, ctx.sasl_method);
      first = false;
    } else {
      res = conn->connect_step(user_dn, cdata, sdata, ctx.sasl_method);
    }
    ctx.send_server_data(sdata);
    if (res == Connection::status::IN_PROGRESS) {
      log_srv_dbg("LDAP SASL bind in progress");
    }
  } while (res == Connection::status::IN_PROGRESS);

  if (res == Connection::status::SUCCESS) {
    log_stream << "SASL User authentication success: [" << user_dn << "]";
    log_srv_dbg(log_stream.str());
    success = true;
  } else {
    log_stream << "SASL User authentication failed: [" << user_dn << "]";
    log_srv_warn(log_stream.str());
  }

  if (conn_out == nullptr || !success) {
    pool_->return_connection(conn);
  } else {
    *conn_out = conn;
  }

  return success;
}  // namespace auth_ldap

bool AuthLDAPImpl::bind(const std::string &user_dn,
                        const std::string &password) {
  return bind_internal(user_dn, password, nullptr);
}

bool AuthLDAPImpl::bind_and_get_mysql_uid(sasl_ctx &ctx,
                                          const std::string &user_dn,
                                          std::string *user_mysql,
                                          std::string *roles_mysql) {
  // If we have a separate bind_root_dn configured, we'll use that
  // Otherwise we'll try to query roles with the actual user, it should work
  // in most cases
  Pool::pool_ptr_t conn;
  if (!bind_internal(ctx, user_dn, &conn)) {
    return false;
  }

  bool ret = get_mysql_uid(user_mysql, roles_mysql, user_dn, &conn);
  pool_->return_connection(conn);

  return ret;
}

bool AuthLDAPImpl::bind_and_get_mysql_uid(const std::string &user_dn,
                                          const std::string &password,
                                          std::string *user_mysql,
                                          std::string *roles_mysql) {
  // If we have a separate bind_root_dn configured, we'll use that
  // Otherwise we'll try to query roles with the actual user, it should work
  // in most cases
  Pool::pool_ptr_t conn;
  if (!bind_internal(user_dn, password, &conn)) {
    return false;
  }

  bool ret = get_mysql_uid(user_mysql, roles_mysql, user_dn, &conn);
  pool_->return_connection(conn);

  return ret;
}

bool AuthLDAPImpl::get_ldap_uid(std::string *user_dn) {
  log_srv_dbg("AuthLDAPImpl::get_ldap_uid()");

  if (user_auth_string_.empty()) {
    *user_dn = search_ldap_uid();
  } else {
    *user_dn = calc_ldap_uid();
  }

  if (user_dn->empty()) {
    std::ostringstream log_stream;
    log_stream << "User not found for user_name: [" << user_name_
               << "] user_search_attr: [" << user_search_attr_
               << "] bind_base_dn: [" << bind_base_dn_ << "]";
    log_srv_warn(log_stream.str());
  }

  return !user_dn->empty();
}

bool AuthLDAPImpl::get_mysql_uid(std::string *user_mysql,
                                 std::string *roles_mysql,
                                 const std::string &user_dn,
                                 Pool::pool_ptr_t *use_conn) {
  log_srv_dbg("AuthLDAPImpl::get_mysql_uid()");
  if (user_dn.empty()) return false;

  auto ldap_groups = search_ldap_groups(user_dn, use_conn);

  if (user_mysql != nullptr) {
    if (ldap_groups.size() == 0) return false;
    *user_mysql = calc_mysql_user(ldap_groups);
    if (user_mysql->empty()) return false;
  }

  *roles_mysql = calc_mysql_roles(ldap_groups);

  return true;
}

std::string AuthLDAPImpl::calc_ldap_uid() {
  log_srv_dbg("AuthLDAPImpl::calc_ldap_uid()");
  std::string uid;
  std::stringstream log_stream;

  if (user_auth_string_[0] == '+') {
    uid = user_search_attr_ + "=" + user_name_ + "," +
          user_auth_string_.substr(1);
    log_stream << "Calculated user_dn: ";
  } else {
    uid = user_auth_string_;
    log_stream << "Full user_dn specified: ";
  }
  log_stream << uid;
  log_srv_dbg(log_stream.str());

  return uid;
}

void AuthLDAPImpl::calc_mappings(const std::string &group_str) {
  std::vector<std::string> parts2;
  boost::algorithm::split(parts2, group_str, boost::is_any_of(","));
  for (const std::string &s : parts2) {
    t_group_mapping map;
    if (s.find("=") != std::string::npos) {
      std::vector<std::string> parts3;
      boost::algorithm::split(parts3, s, boost::is_any_of("="));
      map.mysql_user = parts3[1];
      if (parts3[0].find("+") != std::string::npos) {
        std::vector<std::string> parts4;
        boost::algorithm::split(parts4, parts3[0], boost::is_any_of("+"));
        map.ldap_groups = parts4;
      } else {
        map.ldap_groups.push_back(parts3[0]);
      }
    } else {
      map.mysql_user = s;
      map.ldap_groups.push_back(s);
    }
    mappings_.push_back(map);
  }
}

std::string AuthLDAPImpl::calc_mysql_roles(const groups_t &groups) {
  log_srv_dbg("AuthLDAPImpl::calc_mysql_roles()");
  std::string roles;
  for (const auto &group : groups) {
    auto it = group_role_mapping_.find(group);
    if (it != group_role_mapping_.end()) {
      if (!roles.empty()) roles += ",";
      roles += it->second;
    }
  }
  return roles;
}

std::string AuthLDAPImpl::calc_mysql_user(const groups_t &groups) {
  log_srv_dbg("AuthLDAPImpl::calc_mysql_user()");
  for (const t_group_mapping &map : mappings_) {
    if (matched_map(map, groups)) {
      return map.mysql_user;
    }
  }
  log_srv_dbg("MySQL mapping not found for existing LDAP groups");
  return "";
}

/**
 * All the groups in a map are present in ldap: MATCH!
 */
bool AuthLDAPImpl::matched_map(const t_group_mapping &map,
                               const groups_t &groups) {
  log_srv_dbg("AuthLDAPImpl::matched_map()");
  bool matched = true;
  std::ostringstream log_stream;

  log_stream << "Check map ";
  std::copy(map.ldap_groups.begin(), map.ldap_groups.end(),
            std::ostream_iterator<std::string>(log_stream, ","));
  log_stream << " in AD ";
  std::copy(groups.begin(), groups.end(),
            std::ostream_iterator<std::string>(log_stream, ","));
  log_stream << " -> " << map.mysql_user;
  log_srv_dbg(log_stream.str());

  for (const std::string &s : map.ldap_groups) {
    if (std::find(groups.begin(), groups.end(), s) == std::end(groups))
      matched = false;
  }

  return matched;
}

groups_t AuthLDAPImpl::search_ldap_groups(const std::string &user_dn,
                                          Pool::pool_ptr_t *use_conn) {
  log_srv_dbg("AuthLDAPImpl::search_ldap_groups");
  groups_t list;

  std::shared_ptr<Connection> conn =
      use_conn != nullptr ? *use_conn : pool_->borrow_connection(true);
  if (conn == nullptr) return list;

  list = conn->search_groups(user_name_, user_dn, group_search_attr_,
                             group_search_filter_, bind_base_dn_);

  if (use_conn == nullptr) {
    pool_->return_connection(conn);
  }

  return list;
}

std::string AuthLDAPImpl::search_ldap_uid() {
  log_srv_dbg("AuthLDAPImpl::search_ldap_uid()");
  std::string uid;

  auto conn = pool_->borrow_connection();
  if (conn == nullptr) return uid;

  uid = conn->search_dn(user_name_, user_search_attr_, bind_base_dn_);

  pool_->return_connection(conn);

  if (uid.empty()) {
    std::stringstream log_stream;
    log_stream << "User not found in LDAP user_name: [" << user_name_
               << "] user_search_attr: [" << user_search_attr_
               << "] bind_base_dn: [" << bind_base_dn_ << "]";
    log_srv_dbg(log_stream.str());
  }
  return uid;
}

}  // namespace auth_ldap
}  // namespace plugin
}  // namespace mysql
