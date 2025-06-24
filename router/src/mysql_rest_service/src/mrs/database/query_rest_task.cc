/*
 * Copyright (c) 2025, Oracle and/or its affiliates.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2.0,
 * as published by the Free Software Foundation.
 *
 * This program is designed to work with certain software (including
 * but not limited to OpenSSL) that is licensed under separate terms,
 * as designated in a particular file or component or in included license
 * documentation.  The authors of MySQL hereby grant you an additional
 * permission to link the program and your derivative works with the
 * separately licensed software that they have either included with
 * the program or referenced in the documentation.
 *
 * This program is distributed in the hope that it will be useful,  but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License, version 2.0, for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "mrs/database/query_rest_task.h"
#include <map>
#include "helper/container/generic.h"
#include "helper/json/rapid_json_iterator.h"
#include "helper/json/to_sqlstring.h"
#include "helper/json/to_string.h"
#include "mrs/database/helper/sp_function_query.h"
#include "mrs/http/error.h"
#include "mysql/harness/utility/string.h"
#include "mysqlrouter/utils_sqlstring.h"

namespace mrs {
namespace database {

namespace {
inline std::string join_script(const std::vector<std::string> &script) {
  std::string r;
  for (const auto &s : script) {
    r.append(s);
    if (s.back() != ';')
      r.append(";\n");
    else
      r.append("\n");
  }
  return r;
}
}  // namespace

QueryRestMysqlTask::QueryRestMysqlTask(
    mrs::database::MysqlTaskMonitor *task_monitor)
    : task_monitor_(task_monitor) {}

const char *QueryRestMysqlTask::get_sql_state() {
  if (!sqlstate_.has_value()) return nullptr;
  return sqlstate_.value().c_str();
}

mysqlrouter::sqlstring QueryRestMysqlTask::build_procedure_call(
    const std::string &schema, const std::string &object,
    const mysqlrouter::sqlstring &user_id,
    std::optional<std::string> user_ownership_column, const ResultSets &rs,
    const rapidjson::Document &doc, std::list<std::string> *out_preamble,
    std::list<std::string> *out_postamble) {
  using namespace std::string_literals;
  using namespace helper::json::sql;

  auto &param_fields = rs.parameters.fields;

  mysqlrouter::sqlstring query{"CALL !.!("};
  query << schema << object;

  auto result = mysqlrouter::sqlstring("");
  bool first = true;
  for (auto &el : param_fields) {
    if (!first) {
      query.append_preformatted(",");
    }
    first = false;
    if (user_ownership_column.has_value() &&
        (*user_ownership_column == el.bind_name)) {
      query.append_preformatted(user_id);
    } else if (el.mode == mrs::database::entry::Field::Mode::modeIn) {
      auto it = doc.FindMember(el.name.c_str());
      if (it != doc.MemberEnd()) {
        mysqlrouter::sqlstring sql = get_sql_format(el.data_type);
        sql << it->value;
        query.append_preformatted(sql);
      } else {
        query.append_preformatted("NULL");
      }
    } else {
      std::string var = "@__" + el.bind_name;
      query.append_preformatted(var.c_str());
      mysqlrouter::sqlstring item(("?, " + var).c_str());
      item << el.name;
      result.append_preformatted_sep(", ", item);

      if (el.mode == mrs::database::entry::Field::Mode::modeInOut) {
        mysqlrouter::sqlstring set_var{("SET " + var + " = ?").c_str()};
        auto it = doc.FindMember(el.name.c_str());
        if (it != doc.MemberEnd()) {
          mysqlrouter::sqlstring sql = get_sql_format(el.data_type);
          sql << it->value;
          set_var << sql;
        } else {
          set_var << nullptr;
        }
        out_preamble->emplace_back(set_var.str());
      }
    }
  }
  query.append_preformatted(")");

  out_postamble->emplace_back(
      "SET @task_result = JSON_OBJECT(\"taskResult\", @task_result, " +
      result.str() + ")");

  return query;
}

mysqlrouter::sqlstring QueryRestMysqlTask::build_function_call(
    const std::string &schema, const std::string &object,
    const mysqlrouter::sqlstring &user_id,
    std::optional<std::string> user_ownership_column, const ResultSets &rs,
    const rapidjson::Document &doc, std::list<std::string> *out_postamble) {
  using namespace std::string_literals;
  using namespace helper::json::sql;

  auto &param_fields = rs.parameters.fields;

  mysqlrouter::sqlstring query{"SELECT !.!("};
  query << schema << object;

  auto result = mysqlrouter::sqlstring("");
  bool first = true;
  for (auto &el : param_fields) {
    if (!first) {
      query.append_preformatted(",");
    }
    first = false;
    if (user_ownership_column.has_value() &&
        (*user_ownership_column == el.bind_name)) {
      query.append_preformatted(user_id);
    } else if (el.mode == mrs::database::entry::Field::Mode::modeIn) {
      auto it = doc.FindMember(el.name.c_str());
      if (it != doc.MemberEnd()) {
        mysqlrouter::sqlstring sql = get_sql_format(el.data_type);
        sql << it->value;
        query.append_preformatted(sql);
      } else {
        query.append_preformatted("NULL");
      }
    }
  }
  query.append_preformatted(") INTO @__result");

  out_postamble->emplace_back(
      "SET @task_result = JSON_OBJECT(\"taskResult\", @task_result, "
      "\"result\", @__result)");

  return query;
}

mysqlrouter::sqlstring QueryRestMysqlTask::wrap_async_server_call(
    const std::string &schema, const mysqlrouter::sqlstring &user_id,
    const MysqlTaskOptions &task_options, mysqlrouter::sqlstring query,
    std::list<std::string> preamble, std::list<std::string> postamble) {
  std::string task_sql;
  {
    for (const auto &s : preamble) {
      task_sql.append(s).append(";");
    }

    task_sql.append(query.str()).append(";");

    for (const auto &s : postamble) {
      task_sql.append(s).append(";");
    }
  }

  mysqlrouter::sqlstring sql(
      "CALL mysql_tasks.execute_prepared_stmt_from_app_async(?, ?, ?, ?, ?, ?, "
      "?, ?, ?, NULL, @task_id)",
      0);

  sql << task_sql << user_id
      << (task_options.event_schema.empty() ? schema
                                            : task_options.event_schema)
      << nullptr  // task_type
      << (task_options.name.empty() ? "REST:" + url_ : task_options.name)
      << nullptr   // task_data
      << nullptr;  // data_json_schema
  if (task_options.status_data_json_schema.empty())
    sql << nullptr;
  else
    sql << task_options.status_data_json_schema;
  sql << join_script(task_options.monitoring_sql);

  return sql;
}

void QueryRestMysqlTask::execute_procedure_at_server(
    collector::CountedMySQLSession *session,
    const mysqlrouter::sqlstring &user_id,
    std::optional<std::string> user_ownership_column, const std::string &schema,
    const std::string &object, const std::string &url,
    const MysqlTaskOptions &task_options, const rapidjson::Document &doc,
    const ResultSets &rs) {
  url_ = url;
  execute_at_server(session, user_id, user_ownership_column, true, schema,
                    object, url, task_options, doc, rs);
}

void QueryRestMysqlTask::execute_procedure_at_router(
    CachedSession session, const mysqlrouter::sqlstring &user_id,
    std::optional<std::string> user_ownership_column, const std::string &schema,
    const std::string &object, const std::string &url,
    const MysqlTaskOptions &task_options, const rapidjson::Document &doc,
    const ResultSets &rs) {
  url_ = url;
  execute_at_router(std::move(session), user_id, user_ownership_column, true,
                    schema, object, task_options, doc, rs);
}

void QueryRestMysqlTask::execute_function_at_server(
    collector::CountedMySQLSession *session,
    const mysqlrouter::sqlstring &user_id,
    std::optional<std::string> user_ownership_column, const std::string &schema,
    const std::string &object, const std::string &url,
    const MysqlTaskOptions &task_options, const rapidjson::Document &doc,
    const ResultSets &rs) {
  url_ = url;

  execute_at_server(session, user_id, user_ownership_column, false, schema,
                    object, url, task_options, doc, rs);
}

void QueryRestMysqlTask::execute_function_at_router(
    CachedSession session, const mysqlrouter::sqlstring &user_id,
    std::optional<std::string> user_ownership_column, const std::string &schema,
    const std::string &object, const std::string &url,
    const MysqlTaskOptions &task_options, const rapidjson::Document &doc,
    const ResultSets &rs) {
  url_ = url;
  execute_at_router(std::move(session), user_id, user_ownership_column, false,
                    schema, object, task_options, doc, rs);
}

void QueryRestMysqlTask::execute_at_server(
    collector::CountedMySQLSession *session,
    const mysqlrouter::sqlstring &user_id,
    std::optional<std::string> user_ownership_column, bool is_procedure,
    const std::string &schema, const std::string &object,
    const std::string &url, const MysqlTaskOptions &task_options,
    const rapidjson::Document &doc, const ResultSets &rs) {
  url_ = url;

  std::list<std::string> preamble;
  std::list<std::string> postamble;
  mysqlrouter::sqlstring call_sql;

  if (is_procedure)
    call_sql =
        build_procedure_call(schema, object, user_id, user_ownership_column, rs,
                             doc, &preamble, &postamble);
  else
    call_sql = build_function_call(schema, object, user_id,
                                   user_ownership_column, rs, doc, &postamble);

  query_ =
      wrap_async_server_call(schema, user_id, task_options, std::move(call_sql),
                             std::move(preamble), std::move(postamble));
  execute(session);

  std::string task_id;
  auto row = session->query_one("select @task_id as taskId");
  if (row && (*row)[0]) {
    task_id = (*row)[0];
  } else {
    log_warning("Could not start async task for %s", url.c_str());
    throw std::runtime_error("Error starting asynchronous task");
  }

  response = helper::json::to_string(
      {{"message", "Request accepted. Starting to process task in background."},
       {"taskId", task_id},
       {"statusUrl", url + "/" + task_id}});
}

void QueryRestMysqlTask::execute_at_router(
    CachedSession session, const mysqlrouter::sqlstring &user_id,
    std::optional<std::string> user_ownership_column, bool is_procedure,
    const std::string &schema, const std::string &object,
    const MysqlTaskOptions &task_options, const rapidjson::Document &doc,
    const ResultSets &rs) {
  using namespace std::string_literals;
  using namespace helper::json::sql;

  std::string task_id;
  std::string progress_event_name;
  auto row = session->query_one("select uuid(), replace(uuid(), '-', '')");
  if (row && (*row)[0]) {
    task_id = (*row)[0];
    mysqlrouter::sqlstring tmp("!.!");
    tmp << (task_options.event_schema.empty() ? schema
                                              : task_options.event_schema);
    tmp << (*row)[1];
    progress_event_name = tmp.str();
  } else {
    throw std::runtime_error("Error in UUID() call");
  }

  mysqlrouter::sqlstring internal_data(
      "JSON_OBJECT('mysqlMetadata', JSON_OBJECT('events', ?, 'autoGc', true))");

  if (task_options.monitoring_sql.empty()) {
    auto event = mysqlrouter::sqlstring("JSON_ARRAY(NULL)");
    internal_data << event;
  } else {
    auto event = mysqlrouter::sqlstring("JSON_ARRAY(NULL, ?)");
    event << progress_event_name;
    internal_data << event;
  }

  query_ = {
      "CALL `mysql_tasks`.`create_app_task_with_id`(?, ?, ?,"
      " 'Router_Async_SQL', json_merge_patch(?, ?), ?, ?)"};
  query_ << user_id << task_id
         << (task_options.name.empty() ? "REST:" + url_ : task_options.name)
         << internal_data  // internal_data
         << "{}"           // data
         << "{}";          // data_json_schema
  if (task_options.status_data_json_schema.empty())
    query_ << nullptr;
  else
    query_ << task_options.status_data_json_schema;
  execute(session.get());

  query_ = {"CALL `mysql_tasks`.`start_task_monitor`(?, ?, ?, NULL)"};
  query_ << progress_event_name << task_id;
  if (task_options.monitoring_sql.empty())
    query_ << nullptr;
  else
    query_ << join_script(task_options.monitoring_sql);
  execute(session.get());

  query_ = {
      "CALL `mysql_tasks`.`add_task_log`(?, 'Executing...', NULL, 0,"
      " 'RUNNING')"};
  query_ << task_id;
  execute(session.get());

  // the rest is executed asynchronously by the task thread

  std::list<std::string> preamble;
  std::string script;
  std::list<std::string> postamble;
  if (is_procedure)
    query_ =
        build_procedure_call(schema, object, user_id, user_ownership_column, rs,
                             doc, &preamble, &postamble);
  else
    query_ = build_function_call(schema, object, user_id, user_ownership_column,
                                 rs, doc, &postamble);

  script = query_.str();

  mysqlrouter::sqlstring query{"CALL `mysql_tasks`.`stop_task_monitor`(?, ?)"};
  query << progress_event_name << task_id;
  postamble.emplace_back(query.str());

  query = {
      "CALL `mysql_tasks`.`add_task_log`(?, 'Execution finished.',"
      " CAST(@task_result AS JSON), 100, 'COMPLETED')"};
  query << task_id;
  postamble.emplace_back(query.str());

  task_monitor_->call_async(
      std::move(session), std::move(preamble), std::move(script),
      std::move(postamble),
      [task_id, progress_event_name](const std::exception &e) {
        return on_task_error(e, task_id, progress_event_name);
      },
      task_id);

  response = helper::json::to_string(
      {{"message", "Request accepted. Starting to process task."},
       {"taskId", task_id},
       {"statusUrl", url_ + "/" + task_id}});
}

std::list<std::string> QueryRestMysqlTask::on_task_error(
    const std::exception &e, const std::string &task_id,
    const std::string &progress_event_name) {
  std::list<std::string> sql;

  mysqlrouter::sqlstring query{"CALL `mysql_tasks`.`stop_task_monitor`(?, ?)"};
  query << progress_event_name << task_id;
  sql.emplace_back(query.str());

  query = {"CALL `mysql_tasks`.`add_task_log`(?, ?, NULL, 100, 'ERROR')"};
  query << task_id;
  query << e.what();
  sql.emplace_back(query.str());

  return sql;
}

void QueryRestMysqlTask::kill_task(collector::CountedMySQLSession *session,
                                   const mysqlrouter::sqlstring &user_id,
                                   const std::string &task_id) {
  mysqlrouter::sqlstring query{"CALL mysql_tasks.kill_app_task(?, ?)"};
  query << user_id << task_id;
  try {
    session->execute(query.str());
  } catch (const mysqlrouter::MySQLSession::Error &e) {
    if (e.code() == 1644 && e.message() == "Task inactive.") {
      // task already finished
      return;
    } else if (e.code() == 1095) {
      // task belongs to a different mysql user (sql security definer?)
      throw http::Error(HttpStatusCode::Forbidden);
    }
    throw;
  }
}

}  // namespace database
}  // namespace mrs
