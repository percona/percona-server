/*
  Copyright (c) 2015, 2021, Oracle and/or its affiliates.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2.0,
  as published by the Free Software Foundation.

  This program is also distributed with certain software (including
  but not limited to OpenSSL) that is licensed under separate terms,
  as designated in a particular file or component or in included license
  documentation.  The authors of MySQL hereby grant you an additional
  permission to link the program and your derivative works with the
  separately licensed software that they have included with MySQL.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "plugin_config.h"

#include <algorithm>  // transform
#include <array>
#include <cinttypes>
#include <initializer_list>
#include <stdexcept>  // invalid_argument
#include <string>
#include <vector>

#include "context.h"
#include "hostname_validator.h"
#include "mysql/harness/config_option.h"
#include "mysql/harness/config_parser.h"
#include "mysql/harness/logging/logging.h"
#include "mysql/harness/string_utils.h"  // trim
#include "mysql_router_thread.h"         // kDefaultStackSizeInKiloByte
#include "mysqlrouter/routing.h"         // AccessMode
#include "mysqlrouter/routing_component.h"
#include "mysqlrouter/uri.h"
#include "mysqlrouter/utils.h"  // is_valid_socket_name
#include "ssl_mode.h"
#include "tcp_address.h"

using namespace std::string_view_literals;
IMPORT_LOG_FUNCTIONS()

static std::string get_log_prefix(const mysql_harness::ConfigSection *section,
                                  const mysql_harness::ConfigOption &option) {
  // get_section_name() knows about the options from the DEFAULT section
  // and returns "" in case the option isn't set
  std::string section_name = section->get_section_name(option.name());
  if (section_name.empty()) {
    section_name = section->key.empty() ? section->name
                                        : section->name + ":" + section->key;
  }

  return "option " + option.name() + " in [" + section_name + "]";
}

static int get_option_tcp_port(const mysql_harness::ConfigSection *section,
                               const mysql_harness::ConfigOption &option) {
  auto res = option.get_option_string(section);
  if (!res) {
    throw std::invalid_argument(res.error().message());
  }

  auto value = std::move(res.value());

  if (!value.empty()) {
    char *rest;
    errno = 0;
    auto result = std::strtol(value.c_str(), &rest, 10);

    if (errno > 0 || *rest != '\0' || result > UINT16_MAX || result < 1) {
      std::ostringstream os;
      os << get_log_prefix(section, option)
         << " needs value between 1 and 65535 inclusive";
      if (!value.empty()) {
        os << ", was '" << value << "'";
      }
      throw std::invalid_argument(os.str());
    }

    return static_cast<int>(result);
  }

  return -1;
}

static Protocol::Type get_protocol(const mysql_harness::ConfigSection *section,
                                   const mysql_harness::ConfigOption &option) {
  auto res = option.get_option_string(section);

  if (res == stdx::make_unexpected(
                 make_error_code(mysql_harness::option_errc::not_found))) {
    return Protocol::get_default();
  }

  // all other cases are treated as "empty string"
  auto name = res.value_or("");

  std::transform(name.begin(), name.end(), name.begin(), ::tolower);

  return Protocol::get_by_name(name);
}

static routing::AccessMode get_option_mode(
    const mysql_harness::ConfigSection *section,
    const mysql_harness::ConfigOption &option) {
  auto res = option.get_option_string(section);
  if (!res) {
    if (res.error() == mysql_harness::option_errc::not_found) {
      return routing::AccessMode::kUndefined;
    } else if (res.error() == mysql_harness::option_errc::empty) {
      throw std::invalid_argument(get_log_prefix(section, option) + " " +
                                  res.error().message());
    } else {
      throw std::invalid_argument(option.name() + " ... " +
                                  res.error().message());
    }
  }

  std::string value = std::move(res.value());

  std::transform(value.begin(), value.end(), value.begin(), ::tolower);

  // if the mode is given it still needs to be valid
  routing::AccessMode result = routing::get_access_mode(value);
  if (result == routing::AccessMode::kUndefined) {
    const std::string valid = routing::get_access_mode_names();
    throw std::invalid_argument(get_log_prefix(section, option) +
                                " is invalid; valid are " + valid + " (was '" +
                                value + "')");
  }
  return result;
}

static routing::RoutingStrategy get_option_routing_strategy(
    const mysql_harness::ConfigSection *section,
    const mysql_harness::ConfigOption &option, routing::AccessMode mode,
    bool is_metadata_cache) {
  auto res = option.get_option_string(section);
  if (!res) {
    if (res.error() == mysql_harness::option_errc::not_found) {
      // routing_strategy option is not given
      // this is fine as long as mode is set which means that we deal with an
      // old configuration which we still want to support

      if (mode == routing::AccessMode::kUndefined) {
        throw std::invalid_argument(get_log_prefix(section, option) +
                                    " is required");
      }

      /** @brief `mode` option read from configuration section */
      return routing::RoutingStrategy::kUndefined;
    } else {
      throw std::invalid_argument(get_log_prefix(section, option) + " " +
                                  res.error().message());
    }
  }

  std::string value = std::move(res.value());
  std::transform(value.begin(), value.end(), value.begin(), ::tolower);

  auto result = routing::get_routing_strategy(value);
  if (result == routing::RoutingStrategy::kUndefined ||
      ((result == routing::RoutingStrategy::kRoundRobinWithFallback) &&
       !is_metadata_cache)) {
    const std::string valid =
        routing::get_routing_strategy_names(is_metadata_cache);
    throw std::invalid_argument(get_log_prefix(section, option) +
                                " is invalid; valid are " + valid + " (was '" +
                                value + "')");
  }
  return result;
}

static std::string get_option_destinations(
    const mysql_harness::ConfigSection *section,
    const mysql_harness::ConfigOption &option, const Protocol::Type &,
    bool &metadata_cache) {
  auto res = option.get_option_string(section);

  if (!res) {
    if (res.error() == mysql_harness::option_errc::not_found) {
      throw std::invalid_argument(get_log_prefix(section, option) + " " +
                                  "is required");
    } else {
      throw std::invalid_argument(get_log_prefix(section, option) + " " +
                                  res.error().message());
    }
  }

  std::string value = std::move(res.value());
  try {
    // disable root-less paths like mailto:foo@example.org to stay
    // backward compatible with
    //
    //   localhost:1234,localhost:1235
    //
    // which parse into:
    //
    //   scheme: localhost
    //   path: 1234,localhost:1235
    auto uri = mysqlrouter::URI(value,  // raises URIError when URI is invalid
                                false   // allow_path_rootless
    );
    if (uri.scheme == "metadata-cache") {
      metadata_cache = true;
    } else {
      throw std::invalid_argument(get_log_prefix(section, option) +
                                  " has an invalid URI scheme '" + uri.scheme +
                                  "' for URI " + value);
    }
    return value;
  } catch (const mysqlrouter::URIError &) {
    for (auto part : mysql_harness::split_string(value, ',')) {
      mysql_harness::trim(part);
      if (part.empty()) {
        throw std::invalid_argument(
            get_log_prefix(section, option) +
            ": empty address found in destination list (was '" + value + "')");
      }

      auto make_res = mysql_harness::make_tcp_address(part);

      if (!make_res) {
        throw std::invalid_argument(get_log_prefix(section, option) +
                                    ": address in destination list '" + part +
                                    "' is invalid");
      }

      auto address = make_res->address();

      if (!mysql_harness::is_valid_ip_address(address) &&
          !mysql_harness::is_valid_hostname(address)) {
        throw std::invalid_argument(get_log_prefix(section, option) +
                                    " has an invalid destination address '" +
                                    address + "'");
      }
    }
  }

  return value;
}

static mysql_harness::Path get_option_named_socket(
    const mysql_harness::ConfigSection *section,
    const mysql_harness::ConfigOption &option) {
  auto res = option.get_option_string(section);
  if (!res) {
    throw std::invalid_argument(res.error().message());
  }

  std::string value = std::move(res.value());
  std::string error;
  if (!mysqlrouter::is_valid_socket_name(value, error)) {
    throw std::invalid_argument(error);
  }

  if (value.empty()) {
    return mysql_harness::Path();
  }
  return mysql_harness::Path(value);
}

static mysql_harness::TCPAddress get_option_tcp_address(
    const mysql_harness::ConfigSection *section,
    const mysql_harness::ConfigOption &option, bool require_port,
    int default_port) {
  auto res = option.get_option_string(section);

  if (!res) {
    throw std::invalid_argument(res.error().message());
  }

  std::string value = std::move(res.value());
  if (value.empty()) {
    return mysql_harness::TCPAddress{};
  }

  auto make_res = mysql_harness::make_tcp_address(value);
  if (!make_res) {
    throw std::invalid_argument(get_log_prefix(section, option) + ": '" +
                                value + "' is not a valid endpoint");
  }

  auto address = make_res->address();
  uint16_t port = make_res->port();

  if (port <= 0) {
    if (default_port > 0) {
      port = static_cast<uint16_t>(default_port);
    } else if (require_port) {
      throw std::runtime_error("TCP port missing");
    }
  }

  if (!(mysql_harness::is_valid_hostname(address) ||
        mysql_harness::is_valid_ip_address(address))) {
    throw std::invalid_argument(get_log_prefix(section, option) + ": '" +
                                address + "' in '" + value +
                                "' is not a valid IP-address or hostname");
  }

  return {address, port};
}

template <typename T>
static T get_uint_option(const mysql_harness::ConfigSection *section,
                         const mysql_harness::ConfigOption &option,
                         T min_value = 0,
                         T max_value = std::numeric_limits<T>::max()) {
  auto res = option.get_option_string(section);
  if (!res) {
    throw std::invalid_argument(res.error().message());
  }

  return mysql_harness::option_as_uint(
      res.value(), get_log_prefix(section, option), min_value, max_value);
}

static SslMode get_option_ssl_mode(
    const mysql_harness::ConfigSection *section,
    const mysql_harness::ConfigOption &option,
    std::initializer_list<SslMode> allowed_ssl_modes) {
  auto res = option.get_option_string(section);
  if (!res) {
    throw std::invalid_argument(res.error().message());
  }

  // convert name to upper-case to get case-insenstive comparision.
  auto name = res.value();
  std::transform(name.begin(), name.end(), name.begin(), ::toupper);

  // check if the mode is allowed
  const auto it =
      std::find_if(allowed_ssl_modes.begin(), allowed_ssl_modes.end(),
                   [name](auto const &allowed_ssl_mode) {
                     return name == ssl_mode_to_string(allowed_ssl_mode);
                   });
  if (it != allowed_ssl_modes.end()) {
    return *it;
  }

  // build list of allowed modes, but don't mention the default case.
  std::string allowed_names;
  for (const auto &allowed_ssl_mode : allowed_ssl_modes) {
    if (allowed_ssl_mode == SslMode::kDefault) continue;

    if (!allowed_names.empty()) {
      allowed_names.append(",");
    }

    allowed_names += ssl_mode_to_string(allowed_ssl_mode);
  }

  throw std::invalid_argument("invalid value '" + res.value() + "' for " +
                              option.name() +
                              ". Allowed are: " + allowed_names + ".");
}

/**
 * get the name for a SslVerify.
 *
 * @param verify a SslVerify value
 *
 * @returns name of a SslVerify
 * @retval nullptr if verify is unknown.
 */
static const char *ssl_verify_to_string(SslVerify verify) {
  switch (verify) {
    case SslVerify::kVerifyCa:
      return "VERIFY_CA";
    case SslVerify::kVerifyIdentity:
      return "VERIFY_IDENTITY";
    case SslVerify::kDisabled:
      return "DISABLED";
  }

  return nullptr;
}

static SslVerify get_option_ssl_verify(
    const mysql_harness::ConfigSection *section,
    const mysql_harness::ConfigOption &option,
    std::initializer_list<SslVerify> allowed_ssl_verifies) {
  auto res = option.get_option_string(section);
  if (!res) {
    throw std::invalid_argument(res.error().message());
  }

  // convert name to upper-case to get case-insenstive comparision.
  auto name = res.value();
  std::transform(name.begin(), name.end(), name.begin(), ::toupper);

  const auto it =
      std::find_if(allowed_ssl_verifies.begin(), allowed_ssl_verifies.end(),
                   [name](auto const &allowed_ssl_verify) {
                     return name == ssl_verify_to_string(allowed_ssl_verify);
                   });
  if (it != allowed_ssl_verifies.end()) {
    return *it;
  }

  std::string allowed_names;
  for (const auto &allowed_ssl_verify : allowed_ssl_verifies) {
    if (!allowed_names.empty()) {
      allowed_names.append(",");
    }

    allowed_names += ssl_verify_to_string(allowed_ssl_verify);
  }

  throw std::invalid_argument("invalid value '" + res.value() + "' for " +
                              option.name() +
                              ". Allowed are: " + allowed_names + ".");
}

static std::string get_option_string(
    const mysql_harness::ConfigSection *section,
    const mysql_harness::ConfigOption &option) {
  auto res = option.get_option_string(section);
  if (!res) {
    throw std::invalid_argument(res.error().message());
  }

  return res.value();
}

uint16_t get_option_max_connections(
    const mysql_harness::ConfigSection *section) {
  const auto result = get_uint_option<uint16_t>(
      section, mysql_harness::ConfigOption(
                   "max_connections"sv,
                   std::to_string(routing::kDefaultMaxConnections)));

  auto &routing_component = MySQLRoutingComponent::get_instance();

  if (result != routing::kDefaultMaxConnections &&
      result > routing_component.max_total_connections()) {
    log_warning(
        "Value configured for max_connections > max_total_connections (%u "
        "> %" PRIu64 "). Will have no effect.",
        result, routing_component.max_total_connections());
  }

  return result;
}

/** @brief Constructor
 *
 * @param section from configuration file provided as ConfigSection
 */
RoutingPluginConfig::RoutingPluginConfig(
    const mysql_harness::ConfigSection *section)
    : metadata_cache_(false),
      protocol(
          get_protocol(section, mysql_harness::ConfigOption("protocol"sv))),
      destinations(get_option_destinations(
          section, mysql_harness::ConfigOption("destinations"sv), protocol,
          metadata_cache_)),
      bind_port(get_option_tcp_port(
          section, mysql_harness::ConfigOption("bind_port"sv, ""sv))),
      bind_address(get_option_tcp_address(
          section,
          mysql_harness::ConfigOption("bind_address"sv,
                                      routing::kDefaultBindAddress),
          false, bind_port)),
      named_socket(get_option_named_socket(
          section, mysql_harness::ConfigOption("socket"sv, ""sv))),
      connect_timeout(get_uint_option<uint16_t>(
          section,
          mysql_harness::ConfigOption(
              "connect_timeout"sv,
              std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
                                 routing::kDefaultDestinationConnectionTimeout)
                                 .count())),
          1)),
      mode(get_option_mode(section, mysql_harness::ConfigOption("mode"sv))),
      routing_strategy(get_option_routing_strategy(
          section, mysql_harness::ConfigOption("routing_strategy"sv), mode,
          metadata_cache_)),
      max_connections(get_option_max_connections(section)),
      max_connect_errors(get_uint_option<uint32_t>(
          section,
          mysql_harness::ConfigOption(
              "max_connect_errors"sv,
              std::to_string(routing::kDefaultMaxConnectErrors)),
          1, UINT32_MAX)),
      client_connect_timeout(get_uint_option<uint32_t>(
          section,
          mysql_harness::ConfigOption(
              "client_connect_timeout"sv,
              std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
                                 routing::kDefaultClientConnectTimeout)
                                 .count())),
          2, 31536000)),
      net_buffer_length(get_uint_option<uint32_t>(
          section,
          mysql_harness::ConfigOption(
              "net_buffer_length"sv,
              std::to_string(routing::kDefaultNetBufferLength)),
          1024, 1048576)),
      thread_stack_size(get_uint_option<uint32_t>(
          section,
          mysql_harness::ConfigOption(
              "thread_stack_size"sv,
              std::to_string(mysql_harness::kDefaultStackSizeInKiloBytes)),
          1, 65535)),
      source_ssl_mode{get_option_ssl_mode(
          section, mysql_harness::ConfigOption("client_ssl_mode"sv, ""sv),
          {SslMode::kDisabled, SslMode::kPreferred, SslMode::kRequired,
           SslMode::kPassthrough, SslMode::kDefault})},
      source_ssl_cert{get_option_string(
          section, mysql_harness::ConfigOption("client_ssl_cert"sv, ""sv))},
      source_ssl_key{get_option_string(
          section, mysql_harness::ConfigOption("client_ssl_key"sv, ""sv))},
      source_ssl_cipher{get_option_string(
          section, mysql_harness::ConfigOption("client_ssl_cipher"sv, ""sv))},
      source_ssl_curves{get_option_string(
          section, mysql_harness::ConfigOption("client_ssl_curves"sv, ""sv))},
      source_ssl_dh_params{get_option_string(
          section,
          mysql_harness::ConfigOption("client_ssl_dh_params"sv, ""sv))},
      dest_ssl_mode{get_option_ssl_mode(
          section,
          mysql_harness::ConfigOption("server_ssl_mode"sv, "as_client"sv),
          {SslMode::kDisabled, SslMode::kPreferred, SslMode::kRequired,
           SslMode::kAsClient})},
      dest_ssl_verify{get_option_ssl_verify(
          section,
          mysql_harness::ConfigOption("server_ssl_verify"sv, "disabled"sv),
          {SslVerify::kDisabled, SslVerify::kVerifyCa,
           SslVerify::kVerifyIdentity})},
      dest_ssl_cipher{get_option_string(
          section, mysql_harness::ConfigOption("server_ssl_cipher"sv, ""sv))},
      dest_ssl_ca_file{get_option_string(
          section, mysql_harness::ConfigOption("server_ssl_ca"sv, ""sv))},
      dest_ssl_ca_dir{get_option_string(
          section, mysql_harness::ConfigOption("server_ssl_capath"sv, ""sv))},
      dest_ssl_crl_file{get_option_string(
          section, mysql_harness::ConfigOption("server_ssl_crl"sv, ""sv))},
      dest_ssl_crl_dir{get_option_string(
          section, mysql_harness::ConfigOption("server_ssl_crlpath"sv, ""sv))},
      dest_ssl_curves{get_option_string(
          section, mysql_harness::ConfigOption("server_ssl_curves"sv, ""sv))} {
  using namespace std::string_literals;

  // either bind_address or socket needs to be set, or both
  if (!bind_address.port() && !named_socket.is_set()) {
    throw std::invalid_argument(
        "either bind_address or socket option needs to be supplied, or both");
  }

  // if client-ssl-mode isn't set, use either PASSTHROUGH or PREFERRED
  if (source_ssl_mode == SslMode::kDefault) {
    if (source_ssl_cert.empty() && source_ssl_key.empty()) {
      source_ssl_mode = SslMode::kPassthrough;
    } else {
      source_ssl_mode = SslMode::kPreferred;
    }
  }

  if (source_ssl_mode != SslMode::kDisabled &&
      source_ssl_mode != SslMode::kPassthrough) {
    if (source_ssl_cert.empty()) {
      throw std::invalid_argument(
          "client_ssl_cert must be set, if client_ssl_mode is '"s +
          ssl_mode_to_string(source_ssl_mode) + "'.");
    }
    if (source_ssl_key.empty()) {
      throw std::invalid_argument(
          "client_ssl_key must be set, if client_ssl_mode is '"s +
          ssl_mode_to_string(source_ssl_mode) + "'.");
    }
  }

  if (source_ssl_mode == SslMode::kPassthrough &&
      dest_ssl_mode != SslMode::kAsClient) {
    throw std::invalid_argument(
        "If client_ssl_mode is PASSTHROUGH, server_ssl_mode must be "
        "AS_CLIENT.");
  }

  if (dest_ssl_verify != SslVerify::kDisabled) {
    if (dest_ssl_ca_dir.empty() && dest_ssl_ca_file.empty()) {
      throw std::invalid_argument(
          "server_ssl_ca or server_ssl_capath must be set, if "
          "server_ssl_verify is '"s +
          ssl_verify_to_string(dest_ssl_verify) + "'.");
    }
  }
}
