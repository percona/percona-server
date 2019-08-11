#ifndef _AUTH_LDAP_IMPL_MPALDAP_H
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
#define _AUTH_LDAP_IMPL_MPALDAP_H

#include "plugin/auth_ldap/include/pool.h"

#include <list>
#include <string>
#include <vector>

namespace mysql {
namespace plugin {
namespace auth_ldap {
struct t_group_mapping {
  std::vector<std::string> ldap_groups;
  std::string mysql_user;
};

class AuthLDAPImpl {
 public:
  AuthLDAPImpl(const std::string &user_name, const std::string &auth_string,
               const std::string &user_search_attr,
               const std::string &group_search_filter,
               const std::string &group_search_attr,
               const std::string &bind_base_dn, Pool *pool);
  ~AuthLDAPImpl();

  bool bind(const std::string &user_dn, const std::string &password);
  bool get_ldap_uid(std::string *user_dn);

  bool get_mysql_uid(std::string *user_mysql, const std::string &user_dn);

 private:
  std::string calc_ldap_uid();

  void calc_mappings(const std::string &group_str);
  std::string calc_mysql_user(const std::list<std::string> &groups);

  bool matched_map(const t_group_mapping &map,
                   const std::list<std::string> &groups);

  std::list<std::string> search_ldap_groups(const std::string &user_dn);

  std::string search_ldap_uid();

 private:
  Pool *pool_;
  // Static attributes across all requests
  std::string user_search_attr_;
  std::string group_search_attr_;
  std::string group_search_filter_;
  std::string bind_base_dn_;
  // Dynamic attributes per request
  std::string user_name_;
  std::string user_auth_string_;
  std::vector<t_group_mapping> mappings_;
};
}  // namespace auth_ldap
}  // namespace plugin
}  // namespace mysql

#endif  // _AUTH_LDAP_IMPL_MPALDAP_H
