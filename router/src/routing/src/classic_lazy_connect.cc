/*
  Copyright (c) 2022, 2023, Oracle and/or its affiliates.

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

#include "classic_lazy_connect.h"

#include <chrono>
#include <deque>
#include <memory>
#include <ratio>
#include <sstream>

#include "classic_change_user_sender.h"
#include "classic_connect.h"
#include "classic_connection_base.h"
#include "classic_greeting_forwarder.h"  // ServerGreetor
#include "classic_init_schema_sender.h"
#include "classic_query_sender.h"
#include "classic_reset_connection_sender.h"
#include "classic_set_option_sender.h"
#include "mysql/harness/logging/logging.h"
#include "mysql_com.h"
#include "mysqlrouter/classic_protocol_message.h"

IMPORT_LOG_FUNCTIONS()

using namespace std::chrono_literals;

/**
 * capture the system-variables.
 *
 * Expects a resultset similar to that of:
 *
 * @code
 * SELECT <key>, <value>
 *   FROM performance_schema.session_variables
 *  WHERE VARIABLE_NAME IN ('collation_connection')
 * @endcode
 *
 * - 2 columns (column-names are ignored)
 * - multiple rows
 */
class SelectSessionVariablesHandler : public QuerySender::Handler {
 public:
  SelectSessionVariablesHandler(MysqlRoutingClassicConnectionBase *connection)
      : connection_(connection) {}

  void on_column_count(uint64_t count) override {
    col_count_ = count;

    if (col_count_ != 2) {
      something_failed_ = true;
    }
  }

  void on_column(const classic_protocol::message::server::ColumnMeta
                     & /* col */) override {
    if (something_failed_) return;
  }

  void on_row(const classic_protocol::message::server::Row &row) override {
    if (something_failed_) return;

    auto it = row.begin();  // row[0]

    if (!(*it).has_value()) {
      something_failed_ = true;
      return;
    }

    std::string key = it->value();

    ++it;  // row[1]

    session_variables_.emplace_back(key, *it);
  }

  void on_row_end(
      const classic_protocol::message::server::Eof & /* eof */) override {
    if (something_failed_) {
      // something failed when parsing the resultset. Disable sharing for now.
      connection_->some_state_changed(true);
    } else {
      // move all captured session-vars to the system-variable storage.
      for (; !session_variables_.empty(); session_variables_.pop_front()) {
        auto &node = session_variables_.front();

        connection_->execution_context().system_variables().set(
            std::move(node.first), std::move(node.second));
      }
    }
  }

  void on_ok(const classic_protocol::message::server::Ok & /* ok */) override {
    // ok, shouldn't happen. Disable sharing for now.
    connection_->some_state_changed(true);
  }

  void on_error(const classic_protocol::message::server::Error &err) override {
    // error, shouldn't happen. Disable sharing for now.
    log_debug("Fetching system-vars failed: %s", err.message().c_str());

    connection_->some_state_changed(true);
  }

 private:
  uint64_t col_count_{};
  uint64_t col_cur_{};
  MysqlRoutingClassicConnectionBase *connection_;

  bool something_failed_{false};

  std::deque<std::pair<std::string, Value>> session_variables_;
};

stdx::expected<Processor::Result, std::error_code> LazyConnector::process() {
  switch (stage()) {
    case Stage::Connect:
      return connect();
    case Stage::Connected:
      return connected();
    case Stage::Authenticated:
      return authenticated();
    case Stage::SetSchema:
      return set_schema();
    case Stage::SetSchemaDone:
      return set_schema_done();
    case Stage::SetServerOption:
      return set_server_option();
    case Stage::SetServerOptionDone:
      return set_server_option_done();
    case Stage::SetVars:
      return set_vars();
    case Stage::SetVarsDone:
      return set_vars_done();
    case Stage::FetchSysVars:
      return fetch_sys_vars();
    case Stage::FetchSysVarsDone:
      return fetch_sys_vars_done();
    case Stage::Done:
      trace_span_end(trace_event_connect_);

      return Result::Done;
  }

  harness_assert_this_should_not_execute();
}

stdx::expected<Processor::Result, std::error_code> LazyConnector::connect() {
  if (auto &tr = tracer()) {
    tr.trace(Tracer::Event().stage("connect::connect"));
  }

  trace_event_connect_ =
      trace_span(parent_event_, "mysql/prepare_server_connection");

  auto *socket_splicer = connection()->socket_splicer();
  auto &server_conn = socket_splicer->server_conn();

  if (!server_conn.is_open()) {
    stage(Stage::Connected);

    // creates a fresh connection or takes one from the pool.
    connection()->push_processor(std::make_unique<ConnectProcessor>(
        connection(),
        [this](const classic_protocol::message::server::Error &err) {
          on_error_(err);
        },
        trace_event_connect_));
  } else {
    stage(Stage::Done);  // there still is a connection open, nothing to do.
  }

  return Result::Again;
}

/**
 * the handshake part.
 */
stdx::expected<Processor::Result, std::error_code> LazyConnector::connected() {
  auto *socket_splicer = connection()->socket_splicer();
  auto &server_conn = socket_splicer->server_conn();
  auto *client_protocol = connection()->client_protocol();
  auto *server_protocol = connection()->server_protocol();

  if (!server_conn.is_open()) {
    if (auto &tr = tracer()) {
      tr.trace(Tracer::Event().stage("connect::not_connected"));
    }

    // looks like connection failed, leave.
    stage(Stage::Done);
    return Result::Again;
  }

  trace_event_authenticate_ =
      trace_span(trace_event_connect_, "mysql/authenticate");

  /*
   * if the connection is from the pool, we need a change user.
   */
  if (server_protocol->server_greeting()) {
    connection()->client_greeting_sent(true);

    if (!in_handshake_ &&
        ((client_protocol->username() == server_protocol->username()) &&
         (client_protocol->sent_attributes() ==
          server_protocol->sent_attributes()))) {
      // it is ok if the schema differs, it will be handled later set_schema()

      if (auto *ev = trace_event_authenticate_) {
        ev->attrs.emplace_back("mysql.remote.needs_full_handshake", false);
      }

      connection()->push_processor(std::make_unique<ResetConnectionSender>(
          connection(), trace_event_authenticate_));
    } else {
      if (auto *ev = trace_event_authenticate_) {
        ev->attrs.emplace_back("mysql.remote.needs_full_handshake", true);
        ev->attrs.emplace_back(
            "mysql.remote.username_differs",
            client_protocol->username() == server_protocol->username());
        ev->attrs.emplace_back("mysql.remote.connection_attributes_differ",
                               client_protocol->sent_attributes() ==
                                   server_protocol->sent_attributes());
      }

      connection()->push_processor(std::make_unique<ChangeUserSender>(
          connection(), in_handshake_,
          [this](const classic_protocol::message::server::Error &err) {
            on_error_(err);
          },
          trace_event_authenticate_));
    }
  } else {
    if (auto *ev = trace_event_authenticate_) {
      ev->attrs.emplace_back("mysql.remote.needs_full_handshake", true);
    }

    connection()->push_processor(std::make_unique<ServerGreetor>(
        connection(), in_handshake_,
        [this](const classic_protocol::message::server::Error &err) {
          if (connect_error_is_transient(err) &&
              (connection()->client_protocol()->password().has_value() ||
               !connection()
                    ->server_protocol()
                    ->server_greeting()
                    .has_value()) &&
              std::chrono::steady_clock::now() <
                  started_ + connection()->context().connect_retry_timeout()) {
            // the error is transient.
            //
            // try to reconnect as long as the connect-timeout hasn't been
            // reached yet.

            // only try to reconnect, if
            //
            // 1. the connect failed in the server-greeting
            // 2. the client's password is known as otherwise client would
            // receive the auth-switch several times as part of the auth
            // handshake.

            retry_connect_ = true;
          } else {
            // propagate the error up to the caller.
            on_error_(err);
          }
        },
        trace_event_authenticate_));
  }

  stage(Stage::Authenticated);
  return Result::Again;
}

stdx::expected<Processor::Result, std::error_code>
LazyConnector::authenticated() {
  if (!connection()->authenticated() ||
      !connection()->socket_splicer()->server_conn().is_open()) {
    if (auto &tr = tracer()) {
      tr.trace(Tracer::Event().stage("connect::authenticate::error"));
    }

    if (auto *ev = trace_event_authenticate_) {
      trace_span_end(ev, TraceEvent::StatusCode::kError);
    }

    if (retry_connect_) {
      retry_connect_ = false;

      stage(Stage::Connect);
      connection()->connect_timer().expires_after(kConnectRetryInterval);
      connection()->connect_timer().async_wait([this](std::error_code ec) {
        if (ec) return;

        connection()->resume();
      });

      return Result::Suspend;
    }

    stage(Stage::Done);
    return Result::Again;
  } else {
    if (auto &tr = tracer()) {
      tr.trace(Tracer::Event().stage("connect::authenticate::ok"));
    }

    if (auto *ev = trace_event_authenticate_) {
      trace_span_end(ev);
    }

    stage(Stage::SetVars);
  }
  return Result::Again;
}

namespace {
void set_session_var(std::string &q, const std::string &key, const Value &val) {
  if (q.empty()) {
    q = "SET ";
  } else {
    q += ",\n    ";
  }

  q += "@@SESSION." + key + " = " + val.to_string();
}

void set_session_var_if_not_set(
    std::string &q, const ExecutionContext::SystemVariables &sysvars,
    const std::string &key, const Value &value) {
  if (sysvars.get(key) == Value(std::nullopt)) {
    set_session_var(q, key, value);
  }
}

void set_session_var_or_value(std::string &q,
                              const ExecutionContext::SystemVariables &sysvars,
                              const std::string &key,
                              const Value &default_value) {
  auto value = sysvars.get(key);
  if (value == Value(std::nullopt)) {
    set_session_var(q, key, default_value);
  } else {
    set_session_var(q, key, value);
  }
}
}  // namespace

stdx::expected<Processor::Result, std::error_code> LazyConnector::set_vars() {
  auto &sysvars = connection()->execution_context().system_variables();

  std::string stmt;

  const auto need_session_trackers =
      connection()->context().connection_sharing() &&
      connection()->greeting_from_router();

  // must be first, to track all variables that are set.
  if (need_session_trackers) {
    set_session_var_or_value(stmt, sysvars, "session_track_system_variables",
                             Value("*"));
  } else {
    auto var = sysvars.get("session_track_system_variables");
    if (var != Value(std::nullopt)) {
      set_session_var(stmt, "session_track_system_variables", var);
    }
  }

  for (const auto &var : sysvars) {
    // already set earlier.
    if (var.first == "session_track_system_variables") continue;

    // is read-only
    if (var.first == "statement_id") continue;

    set_session_var(stmt, var.first, var.second);
  }

  if (need_session_trackers) {
    set_session_var_if_not_set(stmt, sysvars, "session_track_gtids",
                               Value("OWN_GTID"));
    set_session_var_if_not_set(stmt, sysvars, "session_track_transaction_info",
                               Value("CHARACTERISTICS"));
    set_session_var_if_not_set(stmt, sysvars, "session_track_state_change",
                               Value("ON"));
  }

  if (!stmt.empty()) {
    stage(Stage::SetVarsDone);

    if (auto &tr = tracer()) {
      tr.trace(Tracer::Event().stage("connect::set_var"));
    }

    trace_event_set_vars_ = trace_span(trace_event_connect_, "mysql/set_var");
    if (auto *ev = trace_event_set_vars_) {
      for (const auto &var : sysvars) {
        if (var.first == "statement_id") continue;

        if (var.second.value()) {
          ev->attrs.emplace_back(
              "mysql.session.@@SESSION." + var.first,
              TraceEvent::element_type::second_type{*var.second.value()});
        } else {
          // NULL
          ev->attrs.emplace_back(var.first,
                                 TraceEvent::element_type::second_type{});
        }
      }
    }

    connection()->push_processor(
        std::make_unique<QuerySender>(connection(), stmt));
  } else {
    stage(Stage::SetServerOption);
  }
  return Result::Again;
}

stdx::expected<Processor::Result, std::error_code>
LazyConnector::set_vars_done() {
  if (auto *ev = trace_event_set_vars_) {
    trace_span_end(ev);
  }

  stage(Stage::SetServerOption);
  return Result::Again;
}

stdx::expected<Processor::Result, std::error_code>
LazyConnector::set_server_option() {
  auto *src_protocol = connection()->client_protocol();
  auto *dst_protocol = connection()->server_protocol();

  bool client_has_multi_statements = src_protocol->client_capabilities().test(
      classic_protocol::capabilities::pos::multi_statements);
  bool server_has_multi_statements = dst_protocol->client_capabilities().test(
      classic_protocol::capabilities::pos::multi_statements);

  stage(Stage::SetServerOptionDone);

  if (client_has_multi_statements != server_has_multi_statements) {
    connection()->push_processor(std::make_unique<SetOptionSender>(
        connection(), client_has_multi_statements
                          ? MYSQL_OPTION_MULTI_STATEMENTS_ON
                          : MYSQL_OPTION_MULTI_STATEMENTS_OFF));
  }

  return Result::Again;
}

stdx::expected<Processor::Result, std::error_code>
LazyConnector::set_server_option_done() {
  stage(Stage::FetchSysVars);
  return Result::Again;
}

stdx::expected<Processor::Result, std::error_code>
LazyConnector::fetch_sys_vars() {
  std::ostringstream oss;

  if (connection()->connection_sharing_possible()) {
    // fetch the sys-vars that aren't known yet.
    for (const auto &expected_var :
         {"collation_connection", "character_set_client", "sql_mode"}) {
      const auto &sys_vars =
          connection()->execution_context().system_variables();
      auto find_res = sys_vars.find(expected_var);
      if (!find_res) {
        if (oss.tellp() != 0) {
          oss << " UNION ";
        }

        // use ' to quote to make it ANSI_QUOTES safe.
        oss << "SELECT " << std::quoted(expected_var, '\'') << ", @@SESSION."
            << std::quoted(expected_var, '`');
      }
    }
  }

  if (oss.tellp() != 0) {
    trace_event_fetch_sys_vars_ =
        trace_span(trace_event_connect_, "mysql/fetch_sys_vars");

    stage(Stage::FetchSysVarsDone);

    connection()->push_processor(std::make_unique<QuerySender>(
        connection(), oss.str(),
        std::make_unique<SelectSessionVariablesHandler>(connection())));
  } else {
    stage(Stage::SetSchema);
  }

  return Result::Again;
}

stdx::expected<Processor::Result, std::error_code>
LazyConnector::fetch_sys_vars_done() {
  trace_span_end(trace_event_fetch_sys_vars_);

  stage(Stage::SetSchema);
  return Result::Again;
}

stdx::expected<Processor::Result, std::error_code> LazyConnector::set_schema() {
  auto client_schema = connection()->client_protocol()->schema();
  auto server_schema = connection()->server_protocol()->schema();

  if (!client_schema.empty() && (client_schema != server_schema)) {
    trace_event_set_schema_ =
        trace_span(trace_event_connect_, "mysql/set_schema");
    stage(Stage::SetSchemaDone);

    connection()->push_processor(
        std::make_unique<InitSchemaSender>(connection(), client_schema));
  } else {
    stage(Stage::Done);
  }

  return Result::Again;
}

stdx::expected<Processor::Result, std::error_code>
LazyConnector::set_schema_done() {
  if (auto *ev = trace_event_set_schema_) {
    trace_span_end(ev);
  }

  stage(Stage::Done);
  return Result::Again;
}
