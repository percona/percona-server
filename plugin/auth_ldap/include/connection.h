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
#include <list>
#include <mutex>
#include <string>

#include <ldap.h>

namespace mysql {
namespace plugin {
namespace auth_ldap {
class Connection {
 public:
  static const unsigned ZombieAfterSeconds = 120;

  Connection(std::size_t idx, const std::string &ldap_host,
             std::uint16_t ldap_port, bool use_ssl, bool use_tls,
             const std::string &ca_path);
  ~Connection();

 public:
  Connection(const Connection &) = delete;  // non construction-copyable
  Connection &operator=(const Connection &) = delete;  // non copyable

 public:
  void configure(const std::string &ldap_host, std::uint16_t ldap_port,
                 bool use_ssl, bool use_tls, const std::string &ca_path);
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
  std::list<std::string> search_groups(const std::string &user_name,
                                       const std::string &bind_user,
                                       const std::string &group_search_attr,
                                       const std::string &group_search_filter,
                                       const std::string &base_dn);

 private:
  std::string get_ldap_uri();
  void log_error(const std::string &str, int ldap_err);
  void log_warning(const std::string &str, int ldap_err);

 private:
  bool available_;
  std::size_t index_;
  std::atomic<bool> snipped_;
  std::string ldap_host_;
  std::uint16_t ldap_port_;
  bool use_ssl_;
  bool use_tls_;
  std::string ca_path_;

 private:
  std::time_t borrowed_ts_;
  std::mutex conn_mutex_;
  LDAP *ldap_;
};
}  // namespace auth_ldap
}  // namespace plugin
}  // namespace mysql
#endif  // MPAL_CONNECTION_H
