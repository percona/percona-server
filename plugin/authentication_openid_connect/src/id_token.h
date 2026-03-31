/*
(C) 2026 Percona LLC and/or its affiliates

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/

#ifndef ID_TOKEN_H
#define ID_TOKEN_H

#include <mysql/plugin_auth.h>
#include <mysql/plugin_auth_common.h>
#include <jwt-cpp/jwt.h>
#include <string>

class Idp_config;

class Id_token {
 private:
  std::string token;
  std::string error;

  static auto get_verifier(const std::string &name, const std::string &key);

  static void map_groups_to_roles(const Idp_config *idp,
      const jwt::basic_claim<jwt::traits::kazuho_picojson>& groups_claim,
                                  std::string &roles);

 public:
  const char *get_error() const;
  bool read(MYSQL_PLUGIN_VIO *vio);

  void verify(const std::string &ext_user, const std::string &idp_name,
              const Idp_config *idp, std::string &roles) const;
};

#endif // ID_TOKEN_H
