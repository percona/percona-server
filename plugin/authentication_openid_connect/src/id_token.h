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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
*/

/**
 * @file id_token.h
 * @brief Header for the Id_token class, which handles OpenID Connect ID token verification.
 */

#ifndef ID_TOKEN_H
#define ID_TOKEN_H

#include <string>

#include <jwt-cpp/jwt.h>
#include <mysql/plugin_auth.h>
#include <mysql/plugin_auth_common.h>

class Idp_config;

/**
 * @class Id_token
 * @brief Represents an OpenID Connect ID token and provides methods to read and verify it.
 */
class Id_token {
 private:
  std::string token; ///< The raw JWT token string.
  std::string error; ///< Stores error messages if verification or reading fails.

  /**
   * @brief Returns a JWT verifier for a given algorithm and public key.
   * @param name The name of the algorithm (e.g., "RS256").
   * @param key The public key in PEM format.
   * @return A jwt::verifier object configured with the specified algorithm and key.
   * @throws std::runtime_error if the algorithm is not supported.
   */
  static auto get_verifier(const std::string &name, const std::string &key);

  /**
   * @brief Maps OpenID Connect groups to database roles based on IDP configuration.
   * @param idp The identity provider configuration.
   * @param groups_claim The claim containing group information from the JWT.
   * @param roles A string to which the mapped roles will be appended (comma-separated).
   * @throws std::runtime_error if the groups claim format is invalid.
   */
  static void map_groups_to_roles(const Idp_config *idp,
      const jwt::basic_claim<jwt::traits::kazuho_picojson>& groups_claim,
                                  std::string &roles);

 public:
  /**
   * @brief Gets the last error message.
   * @return A pointer to the error message string.
   */
  const char *get_error() const;

  /**
   * @brief Reads the ID token from the MySQL client-server communication channel.
   * @param vio The VIO (Virtual I/O) object for communication.
   * @return true if an error occurred while reading, false otherwise.
   */
  bool read(MYSQL_PLUGIN_VIO *vio);

  /**
   * @brief Verifies the ID token against IDP configuration and user information.
   * @param ext_user The expected external username (subject).
   * @param idp Pointer to the Idp_config object containing verification parameters.
   * @param roles String to be populated with roles mapped from the token's groups.
   * @throws std::runtime_error if verification fails or token is invalid.
   */
  void verify(const std::string &ext_user, const Idp_config *idp,
              std::string &roles) const;
};

#endif // ID_TOKEN_H
