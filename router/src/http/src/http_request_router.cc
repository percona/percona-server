/*
  Copyright (c) 2021, 2025, Oracle and/or its affiliates.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2.0,
  as published by the Free Software Foundation.

  This program is designed to work with certain software (including
  but not limited to OpenSSL) that is licensed under separate terms,
  as designated in a particular file or component or in included license
  documentation.  The authors of MySQL hereby grant you an additional
  permission to link the program and your derivative works with the
  separately licensed software that they have either included with
  the program or referenced in the documentation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "http_request_router.h"

#include <unicode/regex.h>
#include <unicode/unistr.h>
#include <unicode/utypes.h>

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "mysql/harness/stdx/expected.h"
#include "mysqlrouter/component/http_auth_realm_component.h"
#include "mysqlrouter/component/http_server_auth.h"

#include "mysql/harness/logging/logging.h"

IMPORT_LOG_FUNCTIONS()

using BaseRequestHandlerPtr = HttpRequestRouter::BaseRequestHandlerPtr;

stdx::expected<void, UErrorCode> HttpRequestRouter::RouteMatcher::compile() {
  UErrorCode out_status = U_ZERO_ERROR;

  std::unique_ptr<icu::RegexPattern> pattern(
      icu::RegexPattern::compile(url_pattern_.c_str(), 0, out_status));
  if (out_status != U_ZERO_ERROR) return stdx::unexpected(out_status);

  regex_pattern_ = std::move(pattern);

  return {};
}

stdx::expected<void, UErrorCode> HttpRequestRouter::RouteMatcher::matches(
    std::string_view input) const {
  return matches(icu::UnicodeString(input.data(), input.size()));
}

stdx::expected<void, UErrorCode> HttpRequestRouter::RouteMatcher::matches(
    const icu::UnicodeString &input) const {
  UErrorCode out_status = U_ZERO_ERROR;

  std::unique_ptr<icu::RegexMatcher> regex_matcher(
      regex_pattern_->matcher(input, out_status));

  const auto find_res = regex_matcher->find(out_status);
  if (find_res == 0) return stdx::unexpected(out_status);

  return {};
}

/**
 * Request router
 *
 * send requests for a path of the URI to a handler callback
 *
 * if no handler is found, reply with 404 not found
 */
void HttpRequestRouter::append(const std::string &url_host,
                               const std::string &url_regex_str,
                               std::unique_ptr<http::base::RequestHandler> cb) {
  log_debug("adding route for regex: %s, url_host: '%s'", url_regex_str.c_str(),
            url_host.c_str());

  RouteMatcher matcher(url_regex_str, std::move(cb));

  auto compile_res = matcher.compile();
  if (!compile_res) {
    throw std::runtime_error("compile of " + url_regex_str +
                             "failed with status " +
                             std::to_string(compile_res.error()));
  }

  std::unique_lock lock(route_mtx_);
  auto req_it = request_handlers_.find(url_host);
  if (req_it == request_handlers_.end()) {
    std::vector<RouteMatcher> router_matchers;
    router_matchers.emplace_back(std::move(matcher));

    request_handlers_.emplace(url_host, std::move(router_matchers));
  } else {
    req_it->second.emplace_back(std::move(matcher));
  }
}

void HttpRequestRouter::remove(const void *handler_id) {
  std::unique_lock lock(route_mtx_);

  for (auto map_it = request_handlers_.begin();
       map_it != request_handlers_.end();) {
    auto &[url_host, request_handlers] = *map_it;
    for (auto it = request_handlers.begin(); it != request_handlers.end();) {
      if (it->handler().get() == handler_id) {
        log_debug("removing route for regex: %s, url_host: '%s'",
                  it->url_pattern().c_str(), url_host.c_str());
        it = request_handlers.erase(it);
      } else {
        ++it;
      }
    }

    if (request_handlers.empty()) {
      // if there are no more request-handlers for a hostname, remove the entry
      // in the hostname map.
      map_it = request_handlers_.erase(map_it);
    } else {
      ++map_it;
    }
  }
}

void HttpRequestRouter::remove(const std::string &url_host,
                               const std::string &url_regex_str) {
  log_debug("removing route for regex: %s, url_host: '%s'",
            url_regex_str.c_str(), url_host.c_str());

  std::unique_lock lock(route_mtx_);

  const auto req_it = request_handlers_.find(url_host);
  if (req_it == request_handlers_.end()) return;

  auto &request_handlers = req_it->second;

  for (auto it = request_handlers.begin(); it != request_handlers.end();) {
    if (it->url_pattern() == url_host && it->url_pattern() == url_regex_str) {
      it = request_handlers.erase(it);
    } else {
      it++;
    }
  }

  if (request_handlers.empty()) {
    // if there are no more request-handlers for a hostname, remove the entry in
    // the hostname map.
    request_handlers_.erase(req_it);
  }
}

// if no routes are specified, return 404
void HttpRequestRouter::handler_not_found(http::base::Request &req) {
  if (!require_realm_.empty()) {
    if (auto realm =
            HttpAuthRealmComponent::get_instance().get(require_realm_)) {
      if (HttpAuth::require_auth(req, realm)) {
        // request is already handled, nothing to do
        return;
      }

      // access granted, fall through
    }
  }
  req.send_error(HttpStatusCode::NotFound);
}

void HttpRequestRouter::set_default_route(
    std::unique_ptr<http::base::RequestHandler> cb) {
  log_debug("adding default route");
  std::unique_lock lock(route_mtx_);

  default_route_ = std::move(cb);
}

void HttpRequestRouter::clear_default_route() {
  log_debug("removing default route");
  std::unique_lock lock(route_mtx_);

  default_route_ = nullptr;
}

void HttpRequestRouter::route(http::base::Request &req) {
  const auto &uri = req.get_uri();

  // CONNECT can't be routed to the request handlers as it doesn't have a "path"
  // part.
  //
  // If the client Accepts "application/problem+json", send it a RFC7807 error
  // otherwise a classic text/html one.
  if (req.get_method() == HttpMethod::Connect) {
    auto hdr_accept = req.get_input_headers().find("Accept");
    if (hdr_accept &&
        hdr_accept->find("application/problem+json") != std::string::npos) {
      req.get_output_headers().add("Content-Type", "application/problem+json");
      std::string json_problem(R"({
  "title": "Method Not Allowed",
  "status": 405
})");
      int status_code = HttpStatusCode::MethodNotAllowed;
      req.send_reply(status_code,
                     HttpStatusCode::get_default_status_text(status_code),
                     json_problem);
    } else {
      req.send_error(HttpStatusCode::MethodNotAllowed);
    }
    return;
  }

  std::string url_host;
  auto hdr_host = req.get_input_headers().find(":authority");
  if (hdr_host) {
    url_host = *hdr_host;
  }

  auto handler = find_route_handler(url_host, uri.get_path());

  if (handler) {
    handler->handle_request(req);
    return;
  }

  handler_not_found(req);
}

namespace {

// If the argument sent as a parameter is in "<hostname>:<port>" format
// (matches "^(.*):[0-9]+$" regex) returns <hostname> part, otherwise returns
// std::nullopt
std::optional<std::string_view> get_host_if_host_and_port(
    const std::string_view &url_host) {
  auto last_colon = url_host.find_last_of(':');

  // no colon or colon at the end
  if (last_colon == std::string_view::npos ||
      last_colon == url_host.size() - 1) {
    return std::nullopt;
  }

  // some non-digit after the colon
  if (std::find_if(url_host.begin() + last_colon + 1, url_host.end(),
                   [](const auto &c) { return !std::isdigit(c); }) !=
      url_host.end()) {
    return std::nullopt;
  }

  return url_host.substr(0, last_colon);
}

}  // namespace

BaseRequestHandlerPtr HttpRequestRouter::find_route_handler(
    std::string_view url_host, std::string_view path) {
  // as .matches() is called in a loop on the same string,
  // convert it to UnicodeString upfront.
  //
  // That saves doing the same conversion for each path under a lock.
  icu::UnicodeString uni_path(path.data(), path.size());

  std::shared_lock lock(route_mtx_);

  if (!url_host.empty()) {
    // Check if we have a handler with a hostname that exactly matches the one
    // from the request
    auto req_it = request_handlers_.find(url_host);
    // No exact match. Check if the url_host in the request is in the
    // <hostname>:<port> format. If that is the case we still accept it if the
    // <hostname> part matches the handler.
    if (req_it == request_handlers_.end()) {
      auto hostname = get_host_if_host_and_port(url_host);
      if (hostname) {
        // Currently the SDK's "CREATE SERVICE" command does not support ipv6 so
        // it is good enough to do the exact match of the host part here. If
        // that ever changes we need something smarter to also match the names
        // with and without enclosing []'s
        req_it = request_handlers_.find(*hostname);
      }
    }

    if (req_it != request_handlers_.end()) {
      auto &request_handlers = req_it->second;

      for (auto &request_handler : request_handlers) {
        if (request_handler.matches(uni_path)) {
          return request_handler.handler();
        }
      }
    }
  }

  // no handler with matching host found, try the one with empty host
  auto req_it = request_handlers_.find("");
  if (req_it != request_handlers_.end()) {
    auto &request_handlers = req_it->second;

    for (auto &request_handler : request_handlers) {
      if (request_handler.matches(uni_path)) {
        return request_handler.handler();
      }
    }
  }

  return default_route_;
}
