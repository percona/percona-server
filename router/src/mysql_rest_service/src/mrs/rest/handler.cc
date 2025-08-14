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

#include "mrs/rest/handler.h"

#include <memory>
#include <string>
#include <utility>

#include "mysqld_error.h"

#include "http/base/request_handler.h"
#include "mysql/harness/logging/logger.h"
#include "mysql/harness/logging/logging.h"
#include "mysql/harness/string_utils.h"
#include "mysqlrouter/component/http_server_component.h"

#include "mrs/authentication/www_authentication_handler.h"
#include "mrs/database/converters/column_datatype_converter.h"
#include "mrs/database/json_mapper/errors.h"
#include "mrs/http/error.h"
#include "mrs/interface/rest_error.h"
#include "mrs/rest/request_context.h"
#include "mrs/router_observation_entities.h"

#include "collector/mysql_cache_manager.h"
#include "helper/container/generic.h"
#include "helper/json/rapid_json_to_map.h"
#include "helper/json/rapid_json_to_struct.h"
#include "helper/json/text_to.h"
#include "helper/string/contains.h"
#include "helper/to_string.h"

IMPORT_LOG_FUNCTIONS()

namespace mrs {
namespace rest {

namespace {

bool _match_glob(const std::string &pat, size_t ppos, const std::string &str,
                 size_t spos) {
  size_t pend = pat.length();
  size_t send = str.length();
  // we allow the string to be matched up to the \0
  while (ppos < pend && spos <= send) {
    int sc = str[spos];
    int pc = pat[ppos];
    switch (pc) {
      case '*':
        // skip multiple consecutive *
        while (ppos < pend && pat[ppos + 1] == '*') ++ppos;

        // match * by trying every substring of str with the rest of the pattern
        for (size_t sp = spos; sp <= send; ++sp) {
          // if something matched, we're fine
          if (_match_glob(pat, ppos + 1, str, sp)) return true;
        }
        // if there were no matches, then give up
        return false;
      case '\\':
        ++ppos;
        if (ppos >= pend)  // can't have an escape at the end of the pattern
          throw std::logic_error("Invalid pattern " + pat);
        pc = pat[ppos];
        if (sc != pc) return false;
        ++ppos;
        ++spos;
        break;
      case '?':
        ++ppos;
        ++spos;
        break;
      default:
        if (sc != pc) return false;
        ++ppos;
        ++spos;
        break;
    }
  }
  return ppos == pend && spos == send;
}

/**
 * Match a string against a glob-like pattern.
 *
 * Allowed wildcard characters: '*', '?'.
 * Supports escaping wildcards via '\\' character.
 *
 * Note: works with ASCII only, no UTF8 support
 */
bool match_glob(const std::string &pattern, const std::string &s) {
  return _match_glob(pattern, 0, s, 0);
}

}  // namespace

using RestError = mrs::interface::RestError;
using ETagMismatch = mrs::interface::ETagMismatch;
using AuthHandler = mrs::interface::AuthorizeManager::AuthorizeHandlerPtr;
using AuthHandlers = mrs::interface::AuthorizeManager::AuthHandlers;
using WwwAuthenticationHandler = mrs::authentication::WwwAuthenticationHandler;
using Parameters = mrs::interface::RestHandler::Parameters;
using HttpHeaders = ::http::base::Headers;
using HttpBuffer = ::http::base::IOBuffer;
using HttpRequest = ::http::base::Request;
using ApplyToV3 = mrs::database::entry::AuthPrivilege::ApplyToV3;
using ApplyToV4 = mrs::database::entry::AuthPrivilege::ApplyToV4;

template <typename T>
std::string to_string(const std::optional<T> &v) {
  using std::to_string;
  if (!v.has_value()) return "null";

  return to_string(v.value());
}

static bool check_privileges_v3(const ApplyToV3 &p,
                                const UniversalId &service_id,
                                const UniversalId &schema_id,
                                const UniversalId &db_object_id) {
  const bool log_level_is_debug = mysql_harness::logging::log_level_is_handled(
      mysql_harness::logging::LogLevel::kDebug);

  if (log_level_is_debug) {
    log_debug("RestRequestHandler: object_id:%s",
              to_string(p.object_id).c_str());
    log_debug("RestRequestHandler: schema_id:%s",
              to_string(p.schema_id).c_str());
    log_debug("RestRequestHandler: service_id:%s",
              to_string(p.service_id).c_str());
  }

  if (!p.object_id && !p.schema_id && !p.service_id) {
    return true;
  }

  if (p.object_id.has_value() && db_object_id == *p.object_id) {
    return true;
  }

  if (p.schema_id.has_value() && schema_id == *p.schema_id) {
    return true;
  }

  return (p.service_id.has_value() && service_id == *p.service_id);
}

static bool check_privileges_v4(const ApplyToV4 &p,
                                const std::string &service_path,
                                const std::string &schema_path,
                                const std::string &db_object_path) {
  if (p.service_name != "*" && !match_glob(p.service_name, service_path))
    return false;

  if (p.schema_name != "*" && !match_glob(p.schema_name, schema_path))
    return false;

  if (p.object_name != "*" && !match_glob(p.object_name, db_object_path))
    return false;

  return true;
}

uint32_t do_privilege_check(
    const std::vector<database::entry::AuthPrivilege> &privileges,
    const UniversalId &service_id, const std::string &service_path,
    const UniversalId &schema_id, const std::string &schema_path,
    const UniversalId &db_object_id, const std::string &db_object_path) {
  uint32_t aggregated_privileges = 0;

  const bool log_level_is_debug = mysql_harness::logging::log_level_is_handled(
      mysql_harness::logging::LogLevel::kDebug);

  if (log_level_is_debug) {
    log_debug("RestRequestHandler: look for service:%s, schema:%s, obj:%s",
              service_path.c_str(), schema_path.c_str(),
              db_object_path.c_str());
  }

  for (const auto &p : privileges) {
    bool matches = false;

    if (std::holds_alternative<ApplyToV3>(p.select_by)) {
      matches = check_privileges_v3(std::get<ApplyToV3>(p.select_by),
                                    service_id, schema_id, db_object_id);
    } else {
      matches = check_privileges_v4(std::get<ApplyToV4>(p.select_by),
                                    service_path, schema_path, db_object_path);
    }

    if (matches) {
      if (log_level_is_debug) {
        log_debug("RestRequestHandler: appending:%i", p.crud);
      }
      aggregated_privileges |= p.crud;
    }
  }

  if (log_level_is_debug) {
    log_debug("RestRequestHandler: aggregated_privileges:%i",
              aggregated_privileges);
  }

  return aggregated_privileges;
}

uint32_t Handler::check_privileges(
    const std::vector<database::entry::AuthPrivilege> &privileges,
    const UniversalId &service_id, const std::string &service_path,
    const UniversalId &schema_id, const std::string &schema_path,
    const UniversalId &db_object_id, const std::string &db_object_path) {
  return do_privilege_check(privileges, service_id, service_path, schema_id,
                            schema_path, db_object_id, db_object_path);
}

uint32_t get_access_right_from_http_method(const uint32_t method) {
  using Op = mrs::database::entry::Operation::Values;

  switch (method) {
    case HttpMethod::Get:
      return Op::valueRead;
    case HttpMethod::Post:
      return Op::valueCreate;
    case HttpMethod::Put:
      return Op::valueUpdate;
    case HttpMethod::Delete:
      return Op::valueDelete;
  }

  return 0;
}

static const char *get_content_type(
    const Handler::HttpResult::Type type,
    const std::optional<std::string> &type_text) {
  if (type_text) return type_text.value().c_str();

  return helper::get_mime_name(type);
}

std::string get_http_method_name(HttpMethod::key_type type) {
  static const std::map<HttpMethod::key_type, std::string> allowed_types{
      {HttpMethod::Connect, "CONNECT"}, {HttpMethod::Delete, "DELETE"},
      {HttpMethod::Get, "GET"},         {HttpMethod::Head, "HEAD"},
      {HttpMethod::Options, "OPTIONS"}, {HttpMethod::Patch, "PATCH"},
      {HttpMethod::Post, "POST"},       {HttpMethod::Put, "PUT"},
      {HttpMethod::Trace, "TRACE"}};

  auto it = allowed_types.find(type);
  if (it != allowed_types.end()) {
    return it->second;
  }

  return std::to_string(type);
}

class RestRequestHandler : public ::http::base::RequestHandler {
 public:
  using Cached = collector::MysqlCacheManager::CachedObject;
  using AuthUser = database::entry::AuthUser;
  using RestHandler = mrs::interface::RestHandler;
  using HandlerPtr = std::weak_ptr<RestHandler>;
  using Options = mrs::interface::Options;

 public:
  RestRequestHandler(HandlerPtr rest_handler,
                     mrs::interface::AuthorizeManager *auth_manager,
                     const bool may_log_requests)
      : rest_handler_{rest_handler},
        auth_manager_{auth_manager},
        may_log_requests_{may_log_requests} {}

  void trace_error(const http::ErrorChangeResponse &e) {
    logger_.debug([&]() {
      return std::string("Catch: ErrorChangeResponse name: ").append(e.name());
    });
    logger_.debug([&]() {
      return std::string("Catch: ErrorChangeResponse retry: ")
          .append((e.retry() ? "true" : "false"));
    });
  }

  void trace_error(const http::Error &e) {
    logger_.debug([&]() {
      return std::string("Catch: http::Error status: ")
          .append(std::to_string(e.status));
    });
    logger_.debug([&]() {
      return std::string("Catch: http::Error message: ").append(e.message);
    });
  }

  void trace_error(const mysqlrouter::MySQLSession::Error &e) {
    logger_.debug([&]() {
      return std::string("Catch: MySQLSession::Error code: ")
          .append(std::to_string(static_cast<int>(e.code())));
    });
    logger_.debug([&]() {
      return std::string("Catch: MySQLSession::Error message: ")
          .append(e.message());
    });
    logger_.debug([&]() {
      return std::string("Catch: MySQLSession::Error message: ")
          .append(e.what());
    });
  }

  void trace_error(const RestError &e) {
    logger_.debug([&]() {
      return std::string("Catch: RestError message: ").append(e.what());
    });
  }

  void trace_error(const std::exception &e) {
    logger_.debug([&]() {
      return std::string("Catch: std::exception message: ").append(e.what());
    });
  }

  void trace_error(const Handler::HttpResult &e) {
    logger_.debug([&]() {
      return std::string("Catch: HttpResult with code: ")
          .append(std::to_string(static_cast<int>(e.status)));
    });
    logger_.debug([&]() {
      return std::string("Catch: HttpResult with message: ").append(e.response);
    });
  }

  void trace_http(const char *type, interface::ReqRes &options,
                  HttpMethod::key_type method, const std::string &path,
                  const HttpHeaders &headers, HttpBuffer &buffer) const {
    if (!may_log_requests_) return;
    if (!options.header_) return;

    logger_.info([&]() {
      return std::string("HTTP ")
          .append(type)
          .append(" method: ")
          .append(get_http_method_name(method));
    });

    logger_.info([&]() {
      return std::string("HTTP ")  //
          .append(type)
          .append(" path: ")
          .append(path);
    });

    for (const auto &[k, v] : headers) {
      logger_.info([&]() {
        const bool hide = (k == "Authorization") || (k == "Location") ||
                          (k == "Set-Cookie") || (k == "Cookie");
        return std::string("HTTP ")
            .append(type)
            .append(" parameters: ")
            .append(k)
            .append("=")
            .append(hide ? "*****" : v);
      });
    }

    if (auto in_len = buffer.length()) {
      const bool has_token =
          buffer.get().find("accessToken") != std::string::npos;
      auto data = buffer.copy(in_len);
      logger_.info([&]() {
        return std::string("HTTP ")  //
            .append(type)
            .append(" body: ")
            .append(has_token ? "*****" : buffer.get());
      });
    }
  }

  Handler::HttpResult handle_request_impl(RestHandler *handler,
                                          RequestContext &ctxt) {
    // Debug handlers do not have auth_manager.
    if (auth_manager_) {
      ctxt.sql_session_cache = auth_manager_->get_cache()->get_empty(
          collector::kMySQLConnectionMetadataRO, false);
    }

    const auto service_id = handler->get_service_id();
    const auto method = ctxt.request->get_method();

    std::string full_service_path =
        handler->get_url_host() + handler->get_service_path();

    logger_.debug([&]() {
      return std::string("handle_request(service_id:")
          .append(service_id.to_string())
          .append("): start(method:")
          .append(get_http_method_name(method))
          .append(" url:'")
          .append(ctxt.request->get_uri().join())
          .append("')");
    });

    auto options = handler->get_options();
    const auto &ih = ctxt.request->get_input_headers();
    auto &oh = ctxt.request->get_output_headers();

    switch (ctxt.request->get_method()) {
      case HttpMethod::Options:
        Counter<kEntityCounterHttpRequestOptions>::increment();
        break;
      case HttpMethod::Get:
        Counter<kEntityCounterHttpRequestGet>::increment();
        break;
      case HttpMethod::Post:
        Counter<kEntityCounterHttpRequestPost>::increment();
        break;
      case HttpMethod::Put:
        Counter<kEntityCounterHttpRequestPut>::increment();
        break;
      case HttpMethod::Delete:
        Counter<kEntityCounterHttpRequestDelete>::increment();
        break;
      default:
        break;
    }

    trace_http("Request", options.debug.http.request, method,
               ctxt.request->get_uri().join(), ih,
               ctxt.request->get_input_buffer());

    for (const auto &kv : handler->get_options().parameters_) {
      if (mysql_harness::make_lower(kv.first) ==
          "access-control-allow-origin") {
        if (handler->get_options().allowed_origins.type !=
            mrs::interface::Options::AllowedOrigins::AllowNone) {
          continue;
        }
      }
      oh.add(kv.first.c_str(), kv.second.c_str());
    }

    auto origin = ih.find_cstr("Origin");

    if (origin) {
      using AO = mrs::interface::Options::AllowedOrigins;

      auto &ao = handler->get_options().allowed_origins;

      switch (ao.type) {
        case AO::AllowAll:
          oh.add("Access-Control-Allow-Origin", origin);
          break;
        case AO::AllowSpecified:
          if (helper::container::has(ao.allowed_origins, origin))
            oh.add("Access-Control-Allow-Origin", origin);
          break;
        case AO::AllowNone:
          break;
      }
    }

    // set the Access-Control-Allow-Methods if not already set on the service
    // level
    if (!oh.find("Access-Control-Allow-Methods")) {
      std::string access_control_allow_methods;

      for (const auto method :
           {HttpMethod::Get, HttpMethod::Post, HttpMethod::Put,
            HttpMethod::Delete, HttpMethod::Options}) {
        if ((get_access_right_from_http_method(method) &
             handler->get_access_rights()) ||
            HttpMethod::Options == method) {
          if (!access_control_allow_methods.empty()) {
            access_control_allow_methods += ", ";
          }
          access_control_allow_methods += get_http_method_name(method);
        }
      }
      oh.add("Access-Control-Allow-Methods",
             access_control_allow_methods.c_str());
    }

    if (method == HttpMethod::Options) {
      throw http::Error{HttpStatusCode::Ok};
    }

    if (!handler->request_begin(&ctxt)) {
      logger_.debug("'request_begin' returned false");
      throw http::Error{HttpStatusCode::Forbidden};
    }

    auto required_access = get_access_right_from_http_method(method);
    if (!(required_access & handler->get_access_rights())) {
      logger_.debug([&]() {
        return std::string("'required_access' denied, required_access:")
            .append(std::to_string(required_access))
            .append(", access:")
            .append(std::to_string(handler->get_access_rights()));
      });
      throw http::Error{HttpStatusCode::Forbidden};
    }

    auto required_auth = handler->requires_authentication();
    if (Handler::Authorization::kNotNeeded != required_auth) {
      logger_.debug([&]() {
        return std::string("RestRequestHandler(service_id:")
            .append(service_id.to_string())
            .append("): authenticate");
      });

      // request_ctxt.user is valid after success of this call
      if (Handler::Authorization::kRequires == required_auth) {
        try {
          if (!auth_manager_->authorize(
                  handler->get_protocol(), handler->get_url_host(), service_id,
                  handler->get_options().query.passthrough_db_user, ctxt,
                  &ctxt.user)) {
            logger_.debug("Authentication handler fails");
            throw http::Error(HttpStatusCode::Unauthorized);
          }
        } catch (const Handler::HttpResult &force_result) {
          if (handler->get_options().debug.log_exceptions)
            trace_error(force_result);
          return force_result;
        }

        logger_.debug("Authentication handler ok.");
      } else {
        // Just check the user
        auth_manager_->is_authorized(service_id, ctxt, &ctxt.user);
      }

      handler->authorization(&ctxt);

      if (handler->may_check_access()) {
        logger_.debug([&]() {
          return std::string("RestRequestHandler(service_id:")
              .append(service_id.to_string())
              .append("): required_access:")
              .append(std::to_string(required_access));
        });
        if (!(required_access &
              do_privilege_check(
                  ctxt.user.privileges, handler->get_service_id(),
                  full_service_path, handler->get_schema_id(),
                  handler->get_schema_path(), handler->get_db_object_id(),
                  handler->get_db_object_path()))) {
          throw http::Error{HttpStatusCode::Forbidden};
        }
      }
    }

    logger_.debug([&]() {
      return std::string("RestRequestHandler(service_id:")
          .append(service_id.to_string())
          .append("dispatch(method:")
          .append(get_http_method_name(ctxt.request->get_method()))
          .append(", path:")
          .append(ctxt.request->get_uri().get_path())
          .append(")");
    });

    switch (method) {
      case HttpMethod::Get:
        return handler->handle_get(&ctxt);
        break;

      case HttpMethod::Post: {
        auto &input_buffer = ctxt.request->get_input_buffer();
        auto size = input_buffer.length();
        return handler->handle_post(&ctxt, input_buffer.pop_front(size));
      } break;

      case HttpMethod::Delete:
        return handler->handle_delete(&ctxt);
        break;

      case HttpMethod::Put: {
        auto response = handler->handle_put(&ctxt);
        return response;
        break;
      }

      default:
        throw http::Error{HttpStatusCode::MethodNotAllowed};
    }
  }

  void handle_request(::http::base::Request &req) override {
    RequestContext request_ctxt{&req, auth_manager_};
    auto handler = rest_handler_.lock();

    if (!handler) {
      send_rfc7807_error(req, HttpStatusCode::GatewayTimeout, {});
      return;
    }

    try {
      auto result = handle_request_impl(handler.get(), request_ctxt);

      auto &b = req.get_output_buffer();
      auto &out_hdrs = req.get_output_headers();

      b.add(result.response.c_str(), result.response.length());

      if (!result.etag.empty()) {
        out_hdrs.add("Cache-Control", "no-cache");
        out_hdrs.add("ETag", result.etag.c_str());
      }

      out_hdrs.add("Content-Type",
                   get_content_type(result.type, result.type_text));

      handler->request_end(&request_ctxt);
      send_reply(req, result.status,
                 HttpStatusCode::get_default_status_text(result.status), b);
    } catch (const http::ErrorChangeResponse &e) {
      if (handler->get_options().debug.log_exceptions) trace_error(e);
      if (e.retry()) {
        logger_.debug("handle_request override");
        auto r = e.change_response(&req);
        send_reply(req, r.status, r.message);
      } else
        handle_error(handler.get(), &request_ctxt, e.change_response(&req));
    } catch (const http::Error &e) {
      if (handler->get_options().debug.log_exceptions) trace_error(e);
      handle_error(handler.get(), &request_ctxt, e);
    } catch (const mrs::database::JSONInputError &e) {
      if (handler->get_options().debug.log_exceptions) trace_error(e);
      handle_error(handler.get(), &request_ctxt, e);
    } catch (const mysqlrouter::MySQLSession::Error &e) {
      if (handler->get_options().debug.log_exceptions) trace_error(e);
      handle_error(handler.get(), &request_ctxt, e);
    } catch (const RestError &e) {
      if (handler->get_options().debug.log_exceptions) trace_error(e);
      handle_error(handler.get(), &request_ctxt, e);
    } catch (const ETagMismatch &e) {
      if (handler->get_options().debug.log_exceptions) trace_error(e);
      handle_error(handler.get(), &request_ctxt, e);
    } catch (const std::invalid_argument &e) {
      if (handler->get_options().debug.log_exceptions) trace_error(e);
      handle_error(handler.get(), &request_ctxt, e);
    } catch (const std::exception &e) {
      if (handler->get_options().debug.log_exceptions) trace_error(e);
      handle_error(handler.get(), &request_ctxt, e);
    }
  }

 private:
  mysql_harness::logging::DomainLogger logger_;

  static http::Error err_to_http_error(
      const mysqlrouter::MySQLSession::Error &err) {
    if (ER_GTID_MODE_OFF == err.code()) {
      return {HttpStatusCode::BadRequest,
              "'Asof' requirement was not fulfilled, GTID_MODE is not "
              "configured properly on the MySQL Server."};
    } else if (ER_WRONG_VALUE == err.code()) {
      // Its safe to forward this message:
      //     ER_WRONG_VALUE_MSG
      //     "Incorrect %-.32s value: \'%-.128s\'"
      return {HttpStatusCode::BadRequest, err.message()};
    }

    return {HttpStatusCode::InternalError};
  }

  static const http::Error &err_to_http_error(const http::Error &err) {
    return err;
  }

  static http::Error err_to_http_error(
      const mrs::database::JSONInputError &err) {
    return {HttpStatusCode::BadRequest, err.what()};
  }

  static http::Error err_to_http_error(const RestError &err) {
    return {HttpStatusCode::BadRequest, err.what()};
  }

  static http::Error err_to_http_error(const ETagMismatch &err) {
    return {HttpStatusCode::PreconditionFailed, err.what()};
  }

  static http::Error err_to_http_error(const std::invalid_argument &err) {
    return {HttpStatusCode::BadRequest, err.what()};
  }

  static http::Error err_to_http_error(const std::exception &) {
    return {HttpStatusCode::InternalError};
  }

  using ObjectKeyValue = std::map<std::string, std::string>;

  ObjectKeyValue responose_encode_error(
      const http::Error &, const mysqlrouter::MySQLSession::Error &e) {
    ObjectKeyValue result{{"message", e.message()},
                          {"what", e.what()},
                          {"sqlcode", std::to_string(e.code())}};
    return result;
  }

  ObjectKeyValue responose_encode_error(const http::Error &converted,
                                        const std::exception &e) {
    ObjectKeyValue result{{"message", converted.message}, {"what", e.what()}};
    return result;
  }

  ObjectKeyValue responose_encode_error(const http::Error &converted,
                                        const http::Error &) {
    ObjectKeyValue result{{"message", converted.message}};
    return result;
  }

  template <typename Err>
  void handle_error(RestHandler *handler, RequestContext *ctxt,
                    const Err &err) {
    const http::Error &e = err_to_http_error(err);
    logger_.debug([&]() {
      std::string msg;
      for (const auto &it : responose_encode_error(e, err)) {
        msg += " " + it.first + "=" + it.second;
      }
      return "handle_error" + msg;
    });
    if (!handler->request_error(ctxt, e)) {
      switch (e.status) {
        case HttpStatusCode::Ok:
          [[fallthrough]];
        case HttpStatusCode::NotModified:
          [[fallthrough]];
        case HttpStatusCode::TemporaryRedirect:
          [[fallthrough]];
        case HttpStatusCode::PermanentRedirect:
          send_reply(*ctxt->request, e.status, e.message);
          break;
        case HttpStatusCode::Unauthorized:
          if (ctxt->selected_handler && ctxt->session) {
            auth_manager_->unauthorize(ctxt->session, &ctxt->cookies);
          }
          [[fallthrough]];
        default:
          if (handler->get_options().debug.http.response.detailed_errors_ &&
              may_log_requests_)
            send_rfc7807_error(*ctxt->request, e.status,
                               responose_encode_error(e, err));
          else
            send_rfc7807_error(*ctxt->request, e.status,
                               responose_encode_error(e, e));
      }
    }
  }

  const Options &get_options() {
    const static Options default_options;
    auto h = rest_handler_.lock();

    if (h) return h->get_options();

    return default_options;
  }

  void send_reply(HttpRequest &req, int status_code) {
    auto options = get_options();
    if (options.debug.http.response.body_) {
      logger_.debug([status_code]() {
        return std::string("HTTP Response status: ")
            .append(std::to_string(status_code));
      });
    }

    trace_http("Response", options.debug.http.response, req.get_method(), "",
               req.get_output_headers(), req.get_output_buffer());
    req.send_reply(status_code);
  }

  void send_reply(HttpRequest &req, int status_code,
                  const std::string &status_text) {
    auto options = get_options();
    if (options.debug.http.response.body_) {
      logger_.debug([&]() {
        return std::string("HTTP Response status: ")
            .append(std::to_string(status_code));
      });

      logger_.debug([&]() {
        return std::string("HTTP Response status text: ")  //
            .append(status_text);
      });
    }
    trace_http("Response", options.debug.http.response, req.get_method(), "",
               req.get_output_headers(), req.get_output_buffer());
    req.send_reply(status_code, status_text);
  }

  void send_reply(HttpRequest &req, int status_code,
                  const std::string &status_text, HttpBuffer &buffer) {
    auto options = get_options();
    if (options.debug.http.response.body_) {
      logger_.debug([&]() {
        return std::string("HTTP Response status: ")
            .append(std::to_string(status_code));
      });
      logger_.debug([&]() {
        return std::string("HTTP Response status text: ").append(status_text);
      });
    }
    trace_http("Response", options.debug.http.response, req.get_method(), "",
               req.get_output_headers(), buffer);
    req.send_reply(status_code, status_text, buffer);
  }

  void send_rfc7807_error(HttpRequest &req,
                          HttpStatusCode::key_type status_code,
                          const std::map<std::string, std::string> &fields) {
    auto &out_hdrs = req.get_output_headers();
    out_hdrs.add("Content-Type", "application/problem+json");

    rapidjson::Document json_doc;

    auto &allocator = json_doc.GetAllocator();

    json_doc.SetObject();
    for (const auto &field : fields) {
      json_doc.AddMember(
          rapidjson::Value(field.first.c_str(), field.first.size(), allocator),
          rapidjson::Value(field.second.c_str(), field.second.size(),
                           allocator),
          allocator);
    }

    json_doc.AddMember("status", status_code, allocator);

    send_json_document(req, status_code, json_doc);
  }

  void send_json_document(HttpRequest &req,
                          HttpStatusCode::key_type status_code,
                          const rapidjson::Document &json_doc) {
    // serialize json-document into a string
    auto &chunk = req.get_output_buffer();

    {
      rapidjson::StringBuffer json_buf;
      {
        rapidjson::Writer<rapidjson::StringBuffer> json_writer(json_buf);

        json_doc.Accept(json_writer);

      }  // free json_doc and json_writer early

      // perhaps we could use evbuffer_add_reference() and a unique-ptr on
      // json_buf here. needs to be benchmarked
      chunk.add(json_buf.GetString(), json_buf.GetSize());
    }  // free json_buf early
    send_reply(req, status_code,
               HttpStatusCode::get_default_status_text(status_code), chunk);
  }

  HandlerPtr rest_handler_;
  mrs::interface::AuthorizeManager *auth_manager_;
  const bool may_log_requests_;
};

namespace cvt {
using std::to_string;
const std::string &to_string(const std::string &str) { return str; }
}  // namespace cvt

static const char *to_cstr(const bool b) { return b ? "true" : "false"; }

template <typename ValueType, bool default_value = false>
bool to_bool(const ValueType &value) {
  using std::to_string;
  const static std::map<std::string, bool> allowed_values{
      {"true", true}, {"false", false}, {"1", true}, {"0", false}};
  auto it = allowed_values.find(cvt::to_string(value));
  if (it != allowed_values.end()) {
    return it->second;
  }

  return default_value;
}

template <typename ValueType>
uint64_t to_uint(const ValueType &value) {
  const auto &v = cvt::to_string(value);
  return std::stoull(v.c_str());
}

template <typename ValueType>
double to_double(const ValueType &value) {
  const auto &v = cvt::to_string(value);
  return std::stod(v.c_str());
}

class ParseOptions
    : public helper::json::RapidReaderHandlerToStruct<mrs::interface::Options> {
 public:
  template <typename ValueType>
  void handle_object_value(const std::string &key, const ValueType &vt) {
    //    log_debug("handle_object_value key:%s, v:%s", key.c_str(),
    //              cvt::to_string(vt).c_str());
    static const std::string kHeaders = "headers.";
    using std::to_string;
    if (helper::starts_with(key, kHeaders)) {
      result_.parameters_[key.substr(kHeaders.length())] = cvt::to_string(vt);
    } else if (key == "logging.exceptions") {
      result_.debug.log_exceptions = to_bool(vt);
    } else if (key == "logging.request.headers") {
      result_.debug.http.request.header_ = to_bool(vt);
    } else if (key == "logging.request.body") {
      result_.debug.http.request.body_ = to_bool(vt);
    } else if (key == "logging.response.headers") {
      result_.debug.http.response.header_ = to_bool(vt);
    } else if (key == "logging.response.body") {
      //      log_debug("handle_object_value hit %s", to_cstr(to_bool(vt)));
      result_.debug.http.response.body_ = to_bool(vt);
    } else if (key == "returnInternalErrorDetails") {
      result_.debug.http.response.detailed_errors_ = to_bool(vt);
    } else if (key == "metadata.gtid") {
      result_.metadata.gtid = to_bool(vt);
    } else if (key == "passthroughDbUser") {
      result_.query.passthrough_db_user = to_bool(vt);
    } else if (key == "sqlQuery.wait") {
      result_.query.wait = to_uint(vt);
    } else if (key == "sqlQuery.embedWait") {
      result_.query.embed_wait = to_bool(vt);
    } else if (key == "sqlQuery.timeout") {
      result_.query.timeout = to_uint(vt);
    } else if (key == "http.allowedOrigin") {
      if (mysql_harness::make_lower(cvt::to_string(vt)) == "auto")
        result_.allowed_origins.type = Result::AllowedOrigins::AllowAll;
      else
        result_.allowed_origins.allowed_origins.push_back(cvt::to_string(vt));
    } else if (key == "result.includeLinks") {
      result_.result.include_links = to_bool(vt);
    } else if (key == "result.cacheTimeToLive") {
      result_.result.cache_ttl_ms = to_double(vt) * 1000;
    } else if (key == "mysqlTask.name") {
      result_.mysql_task.name = cvt::to_string(vt);
    } else if (key == "mysqlTask.eventSchema") {
      result_.mysql_task.event_schema = cvt::to_string(vt);
    } else if (key == "mysqlTask.driver") {
      auto driver = mysql_harness::make_lower(cvt::to_string(vt));
      if (driver == "database") {
        result_.mysql_task.driver =
            mrs::interface::Options::MysqlTask::DriverType::kDatabase;
      } else if (driver == "router") {
        result_.mysql_task.driver =
            mrs::interface::Options::MysqlTask::DriverType::kRouter;
      } else {
        log_warning("Invalid driver type '%s' for option '%s'", driver.c_str(),
                    key.c_str());
        result_.mysql_task.driver =
            mrs::interface::Options::MysqlTask::DriverType::kNone;
      }
    } else if (key == "mysqlTask.monitoringSql") {
      result_.mysql_task.monitoring_sql.push_back(cvt::to_string(vt));
    }
  }

  template <typename ValueType>
  void handle_array_value(const std::string &key, const ValueType &vt) {
    using std::to_string;
    if (key == "http.allowedOrigin") {
      result_.allowed_origins.type = Result::AllowedOrigins::AllowSpecified;
      result_.allowed_origins.allowed_origins.push_back(cvt::to_string(vt));
    } else if (key == "mysqlTask.monitoringSql.monitoringSql") {
      result_.mysql_task.monitoring_sql.push_back(cvt::to_string(vt));
    }
  }

  template <typename ValueType>
  void handle_value(const ValueType &vt) {
    const auto &key = get_current_key();
    if (is_object_path()) {
      if (key == "mysqlTask.statusDataJsonSchema")
        result_.mysql_task.status_data_json_schema = cvt::to_string(vt);
      else
        handle_object_value(key, vt);
    } else if (is_array_value()) {
      handle_array_value(key, vt);
    }
  }

  bool String(const Ch *v, rapidjson::SizeType v_len, bool) override {
    handle_value(std::string{v, v_len});
    return true;
  }

  bool RawNumber(const Ch *v, rapidjson::SizeType v_len, bool) override {
    handle_value(std::string{v, v_len});
    return true;
  }

  bool Bool(bool v) override {
    handle_value(v);
    return true;
  }
};

mrs::interface::Options parse_json_options(
    const std::optional<std::string> &options) {
  if (!options.has_value()) return {};

  return helper::json::text_to_handler<ParseOptions>(options.value());
}

Handler::Handler(const Protocol protocol, const std::string &url_host,
                 const std::vector<UriPathMatcher> &rest_path_matcher,
                 const std::optional<std::string> &options,
                 mrs::interface::AuthorizeManager *auth_manager)
    : options_{parse_json_options(options)},
      url_host_{url_host},
      rest_path_matcher_{rest_path_matcher},
      authorization_manager_{auth_manager},
      protocol_{protocol == endpoint::handler::k_protocolHttp ? "http"
                                                              : "https"},
      log_level_is_debug_(mysql_harness::logging::log_level_is_handled(
          mysql_harness::logging::LogLevel::kDebug)),
      log_level_is_info_(mysql_harness::logging::log_level_is_handled(
          mysql_harness::logging::LogLevel::kInfo)) {
  if (log_level_is_debug_) {
    for (const auto &kv : options_.parameters_) {
      log_debug("headers: '%s':'%s'", kv.first.c_str(), kv.second.c_str());
    }
    log_debug("debug.log_exceptions: %s",
              to_cstr(options_.debug.log_exceptions));
    log_debug("debug.http.request.header: %s",
              to_cstr(options_.debug.http.request.header_));
    log_debug("debug.http.request.body: %s",
              to_cstr(options_.debug.http.request.body_));
    log_debug("debug.http.response.header: %s",
              to_cstr(options_.debug.http.response.header_));
    log_debug("debug.http.response.body: %s",
              to_cstr(options_.debug.http.response.body_));
    log_debug("debug.http.response.detailed_errors_: %s",
              to_cstr(options_.debug.http.response.detailed_errors_));
  }
}

Handler::~Handler() {
  if (log_level_is_debug_ || log_level_is_info_) {
    for (const auto &path : rest_path_matcher_) {
      if (log_level_is_info_) {
        log_info(
            "Removing Url-Handler that processes requests on host: '%s' and "
            "path that matches path: '%s'",
            url_host_.c_str(), path.path.c_str());
      }
      if (log_level_is_debug_) {
        log_debug("route-remove: '%s' on host '%s'", path.path.c_str(),
                  url_host_.c_str());
      }
    }
  }

  assert(!handler_id_.empty() && "initialize() was not called.");

  for (auto id : handler_id_) {
    HttpServerComponent::get_instance().remove_route(id);
  }
}

void Handler::initialize(const Configuration &configuration) {
  const bool may_log_requests = configuration.may_log_request();

  for (auto &path : rest_path_matcher_) {
    auto handler = std::make_unique<RestRequestHandler>(
        weak_from_this(), authorization_manager_, may_log_requests);

    if (log_level_is_debug_) {
      log_debug("route-add: '%s' on host '%s'", path.path.c_str(),
                url_host_.c_str());
    }

    if (log_level_is_info_) {
      log_info(
          "Adding Url-Handler that processes requests on host '%s' and path "
          "that matches: '%s'",
          url_host_.c_str(), path.path.c_str());
    }

    handler_id_.emplace_back(
        HttpServerComponent::get_instance().add_direct_match_route(
            url_host_, path, std::move(handler)));
  }
}

bool Handler::request_begin(RequestContext *) { return true; }

void Handler::request_end(RequestContext *) {}

bool Handler::request_error(RequestContext *, const http::Error &) {
  return false;
}

const interface::Options &Handler::get_options() const { return options_; }

void Handler::throw_unauthorize_when_check_auth_fails(RequestContext *ctxt) {
  if (this->requires_authentication() != Authorization::kNotNeeded) {
    if (!ctxt->user.has_user_id)
      throw http::Error(HttpStatusCode::Unauthorized);
  }
}

void Handler::authorization(RequestContext *) {}

const std::string &Handler::get_url_host() const { return url_host_; }

const std::string &Handler::get_protocol() const { return protocol_; }

bool Handler::may_check_access() const { return true; }

const std::string &Handler::empty_path() const {
  static std::string k_empty;
  return k_empty;
}

}  // namespace rest
}  // namespace mrs
