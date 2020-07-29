#ifndef _MPAL_POOL_H
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
#define _MPAL_POOL_H

#include <boost/dynamic_bitset.hpp>
#include <mutex>
#include <vector>

#include "plugin/auth_ldap/include/connection.h"

namespace mysql {
namespace plugin {
namespace auth_ldap {
class Pool {
 public:
  Pool(std::size_t pool_initial_size, std::size_t pool_max_size,
       const std::string &ldap_host, std::uint16_t ldap_port, bool use_ssl,
       bool use_tls, const std::string &ca_path, const std::string &bind_dn,
       const std::string &bind_pwd);
  ~Pool();

 public:
  Pool(const Pool &) = delete;             // non construction-copyable
  Pool &operator=(const Pool &) = delete;  // non copyable
 public:
  using pool_ptr_t = std::shared_ptr<Connection>;
  pool_ptr_t borrow_connection(bool default_connect = true);
  void debug_info();
  void return_connection(pool_ptr_t conn);
  void reconfigure(std::size_t new_pool_initial_size,
                   std::size_t new_pool_max_size, const std::string &ldap_host,
                   std::uint16_t ldap_port, bool use_ssl, bool use_tls,
                   const std::string &ca_path, const std::string &bind_dn,
                   const std::string &bind_pwd);
  void zombie_control();

 private:
  int find_first_free();
  pool_ptr_t get_connection(int idx, bool default_connect);
  void mark_as_busy(size_t idx);
  void mark_as_free(size_t idx);

 private:
  std::size_t pool_initial_size_;
  std::size_t pool_max_size_;
  std::string ldap_host_;
  std::uint16_t ldap_port_;
  bool use_ssl_;
  bool use_tls_;
  std::string ca_path_;
  std::string bind_dn_;
  std::string bind_pwd_;
  using bs_used_t = boost::dynamic_bitset<>;
  bs_used_t bs_used_;
  using connection_vec_t = std::vector<pool_ptr_t>;
  connection_vec_t v_connections_;
  std::mutex pool_mutex_;
};
}  // namespace auth_ldap
}  // namespace plugin
}  // namespace mysql
#endif  // _MPAL_POOL_H
