#ifndef MPAL_CONNECTION_H
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
#define MPAL_CONNECTION_H

#include <atomic>
#include <ctime>
#include <mutex>
#include <string>
#include <vector>

#include <ldap.h>

namespace mysql {
namespace plugin {
namespace auth_ldap {

using groups_t = std::vector<std::string>;

class Connection {
 public:
  static const unsigned ZombieAfterSeconds = 120;

  Connection(std::size_t idx, const std::string &ldap_host,
             std::uint16_t ldap_port, const std::string &fallback_host,
             std::uint16_t fallback_port, bool use_ssl, bool use_tls);
  ~Connection();

  Connection(const Connection &) = delete;  // non construction-copyable
  Connection &operator=(const Connection &) = delete;  // non copyable

  void configure(const std::string &ldap_host, std::uint16_t ldap_port,
                 const std::string &fallback_host, std::uint16_t fallback_port,
                 bool use_ssl, bool use_tls);
  bool connect(const std::string &bind_dn, const std::string &bind_pwd);
  std::size_t get_idx_pool() const;
  bool is_snipped() const;
  bool is_zombie();
  void mark_as_busy();
  void mark_as_free();
  void mark_as_snipped();
  std::string search_dn(const std::string &user_name,
                        const std::string &user_search_attr,
                        const std::string &base_dn);
  groups_t search_groups(const std::string &user_name,
                         const std::string &bind_user,
                         const std::string &group_search_attr,
                         const std::string &group_search_filter,
                         const std::string &base_dn);

  static void initialize_global_ldap_parameters(bool enable_debug,
                                                std::string const &ca_path);

 private:
  std::string get_ldap_uri();
  static void log_error(const std::string &str, int ldap_err);
  static void log_warning(const std::string &str, int ldap_err);

  bool available_;
  std::size_t index_;
  std::atomic<bool> snipped_;
  std::string ldap_host_;
  std::uint16_t ldap_port_;
  std::string ldap_fallback_host_;
  std::uint16_t ldap_fallback_port_;
  bool use_ssl_;
  bool use_tls_;

  std::time_t borrowed_ts_;
  std::mutex conn_mutex_;
  LDAP *ldap_;
};
}  // namespace auth_ldap
}  // namespace plugin
}  // namespace mysql
#endif  // MPAL_CONNECTION_H
