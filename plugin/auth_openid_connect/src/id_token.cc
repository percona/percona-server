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

#include <algorithm>
#include <stdexcept>
#include <string>
#include <string_view>

/* Clang‑tidy wants to reorder those headers, but jwt‑cpp requires
 * traits header included before jwt.h */
// clang-format off
#include <jwt-cpp/traits/kazuho-picojson/defaults.h>
#include <jwt-cpp/traits/kazuho-picojson/traits.h>
#include <jwt-cpp/jwt.h>
// clang-format on
#include <mysql/plugin_auth_common.h>
#include <mysql_com.h>
#include <picojson/picojson.h>

#include "config.h"

auto Id_token::get_verifier(const std::string &name, const std::string &key) {
  constexpr std::string_view rs256("RS256");
  constexpr std::string_view rs384("RS384");
  constexpr std::string_view rs512("RS512");
  constexpr std::string_view es256("ES256");
  constexpr std::string_view es384("ES384");
  constexpr std::string_view es512("ES512");
  constexpr std::string_view ps256("PS256");
  constexpr std::string_view ps384("PS384");
  constexpr std::string_view ps512("PS512");

  if (name == rs256)
    return jwt::verify().allow_algorithm(jwt::algorithm::rs256(key));
  if (name == rs384)
    return jwt::verify().allow_algorithm(jwt::algorithm::rs384(key));
  if (name == rs512)
    return jwt::verify().allow_algorithm(jwt::algorithm::rs512(key));
  if (name == es256)
    return jwt::verify().allow_algorithm(jwt::algorithm::es256(key));
  if (name == es384)
    return jwt::verify().allow_algorithm(jwt::algorithm::es384(key));
  if (name == es512)
    return jwt::verify().allow_algorithm(jwt::algorithm::es512(key));
  if (name == ps256)
    return jwt::verify().allow_algorithm(jwt::algorithm::ps256(key));
  if (name == ps384)
    return jwt::verify().allow_algorithm(jwt::algorithm::ps384(key));
  if (name == ps512)
    return jwt::verify().allow_algorithm(jwt::algorithm::ps512(key));

  throw std::runtime_error("Unsupported algorithm: " + name);
}

void Id_token::verify_group_member(
    const jwt::basic_claim<jwt::traits::kazuho_picojson> &groups_claim,
    const std::string &group) {
  if (groups_claim.get_type() == jwt::json::type::array) {
    const auto groups{groups_claim.as_array()};
    if (!std::ranges::any_of(groups, [&](const picojson::value &claim_group) {
          return claim_group.to_str() == group;
        }))
      throw std::runtime_error("user is not a member of the required group");

  } else if (groups_claim.get_type() == jwt::json::type::string) {
    if (groups_claim.as_string() != group)
      throw std::runtime_error("user is not a member of the required group");
  } else {
    throw std::runtime_error(
        "cannot parse groups claim in the token, it must be a string or an "
        "array of strings");
  }
}

std::string Id_token::get_first_group(
    const jwt::basic_claim<jwt::traits::kazuho_picojson> &groups_claim) {
  if (groups_claim.get_type() == jwt::json::type::array) {
    const auto groups{groups_claim.as_array()};
    if (groups.empty()) throw std::runtime_error("empty groups claim");
    return groups.front().to_str();
  }

  if (groups_claim.get_type() == jwt::json::type::string)
    return groups_claim.as_string();

  throw std::runtime_error(
      "cannot parse groups claim in the token, it must be a string or an "
      "array of strings");
}

void Id_token::map_groups_to_roles(
    const Idp_config &idp,
    const jwt::basic_claim<jwt::traits::kazuho_picojson> &groups_claim,
    std::string &roles) {
  // Group-role mapping
  if (groups_claim.get_type() == jwt::json::type::array) {
    bool first{true};
    for (const auto &group : groups_claim.as_array()) {
      const std::string_view role{idp.get_role(group.to_str())};
      if (role.empty()) continue;
      if (first)
        first = false;
      else
        roles += ",";
      roles += role;
    }
  } else if (groups_claim.get_type() == jwt::json::type::string) {
    roles = idp.get_role(groups_claim.as_string());
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

std::string Id_token::verify(const std::string &ext_user,
                             const std::string &ext_group,
                             const Idp_config &idp, std::string &roles) const {
  const auto decoded_token = jwt::decode(token);
  const auto issuer{decoded_token.get_issuer()};
  if (issuer != idp.get_issuer_name())
    throw std::runtime_error("Token not issued by " + idp.get_issuer_name());
  const std::string pub_key{decoded_token.has_key_id()
                                ? idp.get_pub_key(decoded_token.get_key_id())
                                : idp.get_the_only_pub_key()};
  // We verify "sub" only if "user" is specified in the AUTHENTICATED AS
  // clause, so proxying is possible
  const auto verifier =
      ext_user.empty()
          ? get_verifier(decoded_token.get_header_claim("alg").as_string(),
                         pub_key)
                .with_claim("iss", jwt::claim(issuer))
          : get_verifier(decoded_token.get_header_claim("alg").as_string(),
                         pub_key)
                .with_claim("iss", jwt::claim(issuer))
                .with_claim("sub", jwt::claim(ext_user));
  // Not explicit here, but verifier verifies both claims and expiration
  verifier.verify(decoded_token);

  // audience check -optional
  if (idp.check_audiences()) verify_audiences(idp, decoded_token);

  // Dealing with groups
  const std::string group_claim_name{idp.get_group_claim()};

  // If the user is empty: proxying
  if (ext_user.empty())
    return handle_proxying(decoded_token, ext_group, group_claim_name);

  // groups and roles mapping -optional and only if there is not proxying
  if (!group_claim_name.empty() &&
      decoded_token.has_payload_claim(group_claim_name))
    map_groups_to_roles(idp, decoded_token.get_payload_claim(group_claim_name),
                        roles);

  return "";
}

void Id_token::verify_audiences(
    const Idp_config &idp,
    const jwt::decoded_jwt<jwt::traits::kazuho_picojson> &decoded_token) {
  if (!decoded_token.has_payload_claim("aud"))
    throw std::runtime_error("missing audience");

  const auto audiences{decoded_token.get_payload_claim("aud")};
  bool authorized{false};

  // there is a single audience given as string
  if (audiences.get_type() == jwt::json::type::string) {
    authorized = idp.is_audience_allowed(
        decoded_token.get_payload_claim("aud").as_string());
  }
  // there are multiple audiences given as array
  else if (audiences.get_type() == jwt::json::type::array) {
    for (const auto &audience : audiences.as_array()) {
      if (idp.is_audience_allowed(audience.to_str())) {
        authorized = true;
        break;
      }
    }
  }

  if (!authorized) throw std::runtime_error("audience not authorized");
}

std::string Id_token::handle_proxying(
    const jwt::decoded_jwt<jwt::traits::kazuho_picojson> &decoded_token,
    const std::string &ext_group, const std::string &group_claim_name) {
  if (group_claim_name.empty())
    throw std::runtime_error(
        "groups claim not configured, proxying not possible");

  if (!decoded_token.has_payload_claim(group_claim_name))
    throw std::runtime_error("token does not contain groups claim");

  // Proxying occurs
  if (ext_group.empty())
    // group not specified, get the first group
    return get_first_group(decoded_token.get_payload_claim(group_claim_name));

  // group specified,  verify that the user is member of the group
  verify_group_member(decoded_token.get_payload_claim(group_claim_name),
                      ext_group);
  return ext_group;
}