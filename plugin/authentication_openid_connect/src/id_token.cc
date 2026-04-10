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

#include "id_token.h"

#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <string>

#include <jwt-cpp/jwt.h>
#include <jwt-cpp/traits/kazuho-picojson/defaults.h>
#include <jwt-cpp/traits/kazuho-picojson/traits.h>
#include <mysql/plugin_auth_common.h>
#include <mysql_com.h>

#include "config.h"

auto Id_token::get_verifier(const std::string &name, const std::string &key) {
  if (name == "RS256")
    return jwt::verify().allow_algorithm(jwt::algorithm::rs256(key));
  if (name == "RS384")
    return jwt::verify().allow_algorithm(jwt::algorithm::rs384(key));
  if (name == "RS512")
    return jwt::verify().allow_algorithm(jwt::algorithm::rs512(key));
  if (name == "ES256")
    return jwt::verify().allow_algorithm(jwt::algorithm::es256(key));
  if (name == "HS256")
    return jwt::verify().allow_algorithm(jwt::algorithm::hs256(key));

  throw std::runtime_error("Unsupported algorithm: " + name);
}

void Id_token::map_groups_to_roles(
    const Idp_config &idp,
    const jwt::basic_claim<jwt::traits::kazuho_picojson> &groups_claim,
    std::string &roles) {
  // Group-role mapping
  if (groups_claim.get_type() == jwt::json::type::array) {
    bool first{true};
    for (const auto &group : groups_claim.as_array()) {
      const std::string &role{idp.get_role(group.to_str())};
      if (role.empty()) continue;
      if (first)
        first = false;
      else
        roles += ",";
      roles += role;
    }
  } else if (groups_claim.get_type() == jwt::json::type::string) {
    roles = groups_claim.as_string();
  } else
    throw std::runtime_error(
        "cannot parse groups claim in the token, it must be a string or an "
        "array of strings");
}

const char *Id_token::get_error() const { return error.c_str(); }

bool Id_token::read(MYSQL_PLUGIN_VIO *vio) {
  unsigned char *pos(nullptr);
  int len_to_parse = vio->read_packet(vio, &pos);

  // 1. field: capability
  // ensure the packet is long enough to hold the field
  if (len_to_parse <= 1 || pos == nullptr) {
    error = "malformed packet";
    return true;
  }
  // skip the field
  pos++;
  len_to_parse--;

  // 2. field: token length
  // ensure the packet is long enough to hold the field
  len_to_parse -= net_field_length_size(pos);
  if (len_to_parse < 1) {
    error = "malformed packet";
    return true;
  }
  // get token length and move pos to the 3. field: the token
  const uint64_t token_len = net_field_length_ll(&pos);
  // check if the token length is correct
  if (token_len > static_cast<uint64_t>(len_to_parse) || token_len < 1) {
    error = "malformed packet";
    return true;
  }
  token = std::string(reinterpret_cast<char *>(pos), token_len);
  return false;
}

void Id_token::verify(const std::string &ext_user, const Idp_config &idp,
                      std::string &roles) const {
  const auto decoded_token = jwt::decode(token);

  const std::string &pub_key{decoded_token.has_key_id()
                                 ? idp.get_pub_key(decoded_token.get_key_id())
                                 : idp.get_pub_key()};
  const auto verifier =
      get_verifier(decoded_token.get_header_claim("alg").as_string(), pub_key)
          .with_claim("iss", jwt::claim(idp.get_issuer_name()))
          .with_claim("sub", jwt::claim(ext_user));
  // Not explicit here, but verifier verifies both caims and expiration
  verifier.verify(decoded_token);

  // audience check -optional
  if (!idp.is_audience_allowed(
          decoded_token.get_payload_claim("aud").as_string()))
    throw std::runtime_error("invalid audience");

  // groups and roles mapping -optional
  if (const std::string &group_claim_name{idp.get_group_claim()};
      !group_claim_name.empty() &&
      decoded_token.has_payload_claim(group_claim_name))
    map_groups_to_roles(idp, decoded_token.get_payload_claim(group_claim_name),
                        roles);

  // TODO proxying
}
