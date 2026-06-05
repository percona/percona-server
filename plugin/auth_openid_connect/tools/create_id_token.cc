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

#include <jwt-cpp/jwt.h>

#include <chrono>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <jwt-cpp/traits/kazuho-picojson/defaults.h>
#include <picojson/picojson.h>

struct Args {
  std::string key_path;
  std::string iss = "https://idp-test.com/realms/dummy";
  std::string sub = "idp_user";
  std::string kid;
  std::string name = "IDP User";
  std::string aud;
  int ttl = 360;
  std::string groups_json;
  std::string email = "idp_user@percona.com";
  std::string algorithm = "RS256";
  std::string out = "./id_token.json";
};

// NOLINTNEXTLINE(misc-use-anonymous-namespace)
static std::string read_file(const std::string &path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) {
    throw std::runtime_error("Cannot open file: " + path);
  }

  std::ostringstream ss;
  ss << in.rdbuf();
  return ss.str();
}

// NOLINTNEXTLINE(misc-use-anonymous-namespace)
static std::vector<std::string> parse_groups_json(
    const std::string &groups_json) {
  if (groups_json.empty()) {
    return {};
  }

  picojson::value groups_array;
  if (const std::string err = picojson::parse(groups_array, groups_json);
      !err.empty()) {
    throw std::runtime_error("Invalid groups JSON: " + err);
  }

  if (!groups_array.is<picojson::array>()) {
    throw std::runtime_error("groups must be a JSON array");
  }

  std::vector<std::string> result;
  const auto &arr = groups_array.get<picojson::array>();
  result.reserve(arr.size());

  for (const auto &item : arr) {
    if (!item.is<std::string>()) {
      throw std::runtime_error("groups array must contain only strings");
    }
    result.push_back(item.get<std::string>());
  }

  return result;
}

// NOLINTNEXTLINE(misc-use-anonymous-namespace)
static std::string create_signed_id_token(
    const std::string &private_key_pem, const std::string &issuer,
    const std::string &subject, const std::string &kid,
    const std::string &audience, const std::string &name,
    const std::vector<std::string> &groups, const std::string &email,
    const int ttl, const std::string &algorithm) {
  using namespace std::chrono;
  const auto now = system_clock::now();
  const auto exp = now + static_cast<seconds>(ttl);

  picojson::array groups_array(groups.size());
  for (const auto &group : groups) {
    groups_array.emplace_back(group);
  }

  auto builder = jwt::create()
                     .set_issuer(issuer)
                     .set_subject(subject)
                     .set_issued_at(now)
                     .set_expires_at(exp)
                     .set_payload_claim("name", jwt::claim(name))
                     .set_payload_claim("email", jwt::claim(email))
                     .set_payload_claim("email_verified",
                                        jwt::claim(picojson::value(true)))
                     .set_payload_claim(
                         "groups", jwt::claim(picojson::value(groups_array)));

  if (!kid.empty()) {
    builder.set_key_id(kid);
  }

  if (!audience.empty()) {
    auto trim = [](std::string st) {
      st.erase(st.begin(), std::ranges::find_if(st, [](const unsigned char ch) {
                 return !std::isspace(ch);
               }));
      st.erase(std::find_if(st.rbegin(), st.rend(),
                            [](unsigned char ch) { return !std::isspace(ch); })
                   .base(),
               st.end());
      return st;
    };

    std::vector<std::string> parts;
    std::size_t start = 0;

    while (start < audience.size()) {
      std::size_t pos = audience.find(',', start);
      std::string item = trim(audience.substr(
          start, pos == std::string::npos ? std::string::npos : pos - start));

      if (!item.empty()) {
        parts.push_back(std::move(item));
      }

      if (pos == std::string::npos) {
        break;
      }
      start = pos + 1;
    }

    if (parts.size() == 1) {
      builder.set_audience(parts.front());
    } else if (parts.size() > 1) {
      typename jwt::traits::kazuho_picojson::array_type audiences;
      for (const auto &part : parts) {
        audiences.emplace_back(part);
      }

      builder.set_audience(audiences);
    }
  }

  if (algorithm == "HS256") {
    return builder.sign(
        jwt::algorithm::hs256(private_key_pem  // HMAC signing key
                              ));
  }
  if (algorithm == "HS384") {
    return builder.sign(
        jwt::algorithm::hs384(private_key_pem  // HMAC signing key
                              ));
  }
  if (algorithm == "HS512") {
    return builder.sign(
        jwt::algorithm::hs512(private_key_pem  // HMAC signing key
                              ));
  }
  if (algorithm == "RS256") {
    return builder.sign(jwt::algorithm::rs256(
        "",              // public key -not needed for signing
        private_key_pem  // private key in PEM format
        ));
  }
  if (algorithm == "RS384") {
    return builder.sign(jwt::algorithm::rs384(
        "",              // public key - not needed for signing
        private_key_pem  // private key in PEM format
        ));
  }
  if (algorithm == "RS512") {
    return builder.sign(jwt::algorithm::rs512(
        "",              // public key - not needed for signing
        private_key_pem  // private key in PEM format
        ));
  }
  if (algorithm == "PS256") {
    return builder.sign(jwt::algorithm::ps256(
        "",              // public key -not needed for signing
        private_key_pem  // private key in PEM format
        ));
  }
  if (algorithm == "PS384") {
    return builder.sign(jwt::algorithm::ps384(
        "",              // public key - not needed for signing
        private_key_pem  // private key in PEM format
        ));
  }
  if (algorithm == "PS512") {
    return builder.sign(jwt::algorithm::ps512(
        "",              // public key - not needed for signing
        private_key_pem  // private key in PEM format
        ));
  }
  if (algorithm == "ES256") {
    return builder.sign(jwt::algorithm::es256(
        "",              // public key - not needed for signing
        private_key_pem  // private key in PEM format
        ));
  }
  if (algorithm == "ES384") {
    return builder.sign(jwt::algorithm::es384(
        "",              // public key - not needed for signing
        private_key_pem  // private key in PEM format
        ));
  }
  if (algorithm == "ES512") {
    return builder.sign(jwt::algorithm::es512(
        "",              // public key - not needed for signing
        private_key_pem  // private key in PEM format
        ));
  }

  throw std::runtime_error(
      "Unsupported algorithm or key length, "
      "supported algorithms are: RS, ES, HS, "
      "supported lengths are: 256, 384, 512.");
}

// NOLINTNEXTLINE(misc-use-anonymous-namespace)
static void print_usage(const char *progname) {
  std::cerr
      << "Usage: " << progname << " --key <private-key.pem> [options]\n"
      << "Options:\n"
      << "  --alg, -a   Algorithm (e.g. RS256)\n"
      << "  --aud, -d         Audience\n"
      << "  --email, -e       Email\n"
      << "  --groups, -g      JSON array of groups, e.g. "
         "[\"group1\",\"group2\"]\n"
         "signing key (required)\n"
      << "  --iss, -i         Issuer\n"
      << "  --key, -k         Path to private key in PEM format or HMAC key\n"
      << "  --kid, -z         Key id\n"
      << "  --name, -n        Name\n"
      << "  --oot, -o         Output file, default: \"./id_token.json\"\n"
      << "  --sub, -s         Subject\n"
      << "  --ttl, -t         Time to live in seconds\n";
}

// NOLINTNEXTLINE(misc-use-anonymous-namespace)
static Args parse_args(int argc, char **argv) {
  Args args;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    auto require_value = [&](const std::string &opt) -> std::string {
      if (i + 1 >= argc) {
        throw std::runtime_error("Missing value for option: " + opt);
      }
      return argv[++i];
    };

    if (arg == "--key" || arg == "-k") {
      args.key_path = require_value(arg);
    } else if (arg == "--iss" || arg == "-i") {
      args.iss = require_value(arg);
    } else if (arg == "--sub" || arg == "-s") {
      args.sub = require_value(arg);
    } else if (arg == "--kid" || arg == "-z") {
      args.kid = require_value(arg);
    } else if (arg == "--name" || arg == "-n") {
      args.name = require_value(arg);
    } else if (arg == "--aud" || arg == "-d") {
      args.aud = require_value(arg);
    } else if (arg == "--ttl" || arg == "-t") {
      args.ttl = std::stoi(require_value(arg));
    } else if (arg == "--groups" || arg == "-g") {
      args.groups_json = require_value(arg);
    } else if (arg == "--email" || arg == "-e") {
      args.email = require_value(arg);
    } else if (arg == "--alg" || arg == "-a") {
      args.algorithm = require_value(arg);
    } else if (arg == "--out" || arg == "-o") {
      args.out = require_value(arg);
    } else if (arg == "--help" || arg == "-h") {
      print_usage(argv[0]);
      std::exit(0);
    } else {
      throw std::runtime_error("Unknown argument: " + arg);
    }
  }

  if (args.key_path.empty()) {
    throw std::runtime_error("Missing required argument: --key");
  }

  return args;
}

int main(int argc, char **argv) {
  try {
    const Args args = parse_args(argc, argv);
    const std::string private_key_pem = read_file(args.key_path);
    const std::vector<std::string> groups = parse_groups_json(args.groups_json);

    const std::string token = create_signed_id_token(
        private_key_pem, args.iss, args.sub, args.kid, args.aud, args.name,
        groups, args.email, args.ttl, args.algorithm);

    std::ofstream out(args.out, std::ios::binary);
    if (!out) {
      throw std::runtime_error("Cannot open output file: " + args.out);
    }
    out << token;
    if (!out) {
      throw std::runtime_error("Failed to write token to file: " + args.out);
    }
    return 0;
  } catch (const std::exception &e) {
    std::cout << "ERROR: " << e.what() << '\n';
    return 1;
  }
}