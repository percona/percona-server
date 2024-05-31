/* Copyright (c) 2024 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; version 2 of
   the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#include <mysqld_error.h>
#include <sstream>

#include "data_provider.h"
#include "logger.h"

namespace {
inline const char *b2s(bool val) { return val ? "1" : "0"; }

namespace JSONKey {
const char *pillar_version = "pillar_version";
const char *db_instance_id = "db_instance_id";
const char *active_plugins = "active_plugins";
const char *active_components = "active_components";
const char *uptime = "uptime";
const char *databases_count = "databases_count";
const char *databases_size = "databases_size";
const char *se_engines_in_use = "se_engines_in_use";
const char *role = "role";
const char *db_replication_id = "db_replication_id";
const char *single_primary_mode = "single_primary_mode";
const char *group_size = "group_size";
const char *group_replication_info = "group_replication_info";
const char *is_semisync_source = "is_semisync_source";
const char *is_source = "is_source";
const char *is_semisync_replica = "is_semisync_replica";
const char *is_replica = "is_replica";
const char *replication_info = "replication_info";
}  // namespace JSONKey
}  // namespace

/* We need to provide db_replication_id key at the top level of the JSON
structure. Its value can potentially originate from different sources, so we
have to decide which one to use. This class solves the problem if there is
more than one source of the ID. */
class DbReplicationIdSolver {
 public:
  DbReplicationIdSolver() = default;
  ~DbReplicationIdSolver() = default;
  DbReplicationIdSolver(const DbReplicationIdSolver &) = delete;
  DbReplicationIdSolver(const DbReplicationIdSolver &&) = delete;
  DbReplicationIdSolver &operator=(const DbReplicationIdSolver &) = delete;
  DbReplicationIdSolver &operator=(const DbReplicationIdSolver &&) = delete;

  /* Voters in the order of their priorities. Lower number, lower priority. */
  enum Voter { NONE, GROUP_REPLICATION };

  void vote(const std::string &id, Voter voter) {
    if (voter > id_voter_) {
      db_replication_id_ = id;
      id_voter_ = voter;
    }
  }

  const std::string &get_db_replication_id() const {
    return db_replication_id_;
  }

  void reset() {
    db_replication_id_.clear();
    id_voter_ = Voter::NONE;
  }

 private:
  std::string db_replication_id_;
  Voter id_voter_{Voter::NONE};
};

DataProvider::DataProvider(
    SERVICE_TYPE(mysql_command_factory) & command_factory_service,
    SERVICE_TYPE(mysql_command_options) & command_options_service,
    SERVICE_TYPE(mysql_command_query) & command_query_service,
    SERVICE_TYPE(mysql_command_query_result) & command_query_result_service,
    SERVICE_TYPE(mysql_command_field_info) & command_field_info_service,
    SERVICE_TYPE(mysql_command_error_info) & command_error_info_service,
    SERVICE_TYPE(mysql_command_thread) & command_thread_service, Logger &logger)
    : command_factory_service_(command_factory_service),
      command_options_service_(command_options_service),
      command_query_service_(command_query_service),
      command_query_result_service_(command_query_result_service),
      command_field_info_service_(command_field_info_service),
      command_error_info_service_(command_error_info_service),
      command_thread_service_(command_thread_service),
      logger_(logger),
      db_replication_id_solver_(std::make_shared<DbReplicationIdSolver>()) {}

void DataProvider::thread_access_begin() { command_thread_service_.init(); }

void DataProvider::thread_access_end() { command_thread_service_.end(); }

bool DataProvider::do_query(const std::string &query, QueryResult *result,
                            unsigned int *err_no,
                            bool suppress_query_error_log) {
  MYSQL_RES_H mysql_res = nullptr;
  MYSQL_ROW_H row = nullptr;
  uint64_t row_count = 0;
  unsigned int num_column = 0;
  std::string result_set;
  MYSQL_H mysql_h = nullptr;
  bool res = true;

  if (!result) {
    return true;
  }
  result->clear();

  mysql_service_status_t sstatus = command_factory_service_.init(&mysql_h);
  if (!sstatus)
    sstatus |=
        command_options_service_.set(mysql_h, MYSQL_COMMAND_PROTOCOL, nullptr);
  if (!sstatus)
    sstatus |=
        command_options_service_.set(mysql_h, MYSQL_COMMAND_USER_NAME, "root");
  if (!sstatus)
    sstatus |=
        command_options_service_.set(mysql_h, MYSQL_COMMAND_HOST_NAME, nullptr);
  if (!sstatus) sstatus |= command_factory_service_.connect(mysql_h);

  // starting from this point, if the above succeeded we need to close mysql_h.
  std::shared_ptr<void> mysql_h_close_guard(
      mysql_h,
      [&srv = command_factory_service_, do_close = !sstatus](void *ptr) {
        if (do_close && ptr) srv.close(static_cast<MYSQL_H>(ptr));
      });

  // if any of the above failed, just exit
  if (sstatus) {
    goto err;
  }

  if (command_query_service_.query(mysql_h, query.data(), query.length())) {
    if (err_no) {
      command_error_info_service_.sql_errno(mysql_h, err_no);
      if (command_error_info_service_.sql_errno(mysql_h, err_no)) {
        logger_.warning("Failed to get the last query error");
      }
    }
    goto err;
  }

  command_query_result_service_.store_result(mysql_h, &mysql_res);
  if (mysql_res) {
    std::shared_ptr<void> query_result_free_guard(
        mysql_res, [&srv = command_query_result_service_](void *ptr) {
          if (ptr) srv.free_result(static_cast<MYSQL_RES_H>(ptr));
        });

    if (command_query_service_.affected_rows(mysql_h, &row_count)) {
      goto err;
    }
    if (command_field_info_service_.num_fields(mysql_res, &num_column)) {
      goto err;
    }

    for (uint64_t i = 0; i < row_count; i++) {
      if (command_query_result_service_.fetch_row(mysql_res, &row)) {
        goto err;
      }
      ulong *length = nullptr;
      command_query_result_service_.fetch_lengths(mysql_res, &length);

      Row new_row;

      for (unsigned int j = 0; j < num_column; j++) {
        new_row.emplace_back(std::string(row[j]));
      }
      result->push_back(std::move(new_row));
    }
  }

  res = false;
err:
  if (res && !suppress_query_error_log) {
    logger_.info("do_query() failed. query: %s", query.c_str());
  }
  return res;
}

const std::string &DataProvider::get_database_instance_id() {
  if (!database_instance_id_cache_.length()) {
    QueryResult result;
    if (do_query("SELECT @@server_uuid", &result)) {
      static std::string empty;
      return empty;
    }
    database_instance_id_cache_ = result[0][0];
  }
  return database_instance_id_cache_;
}

bool DataProvider::collect_db_instance_id_info(rapidjson::Document *document) {
  const std::string &id = get_database_instance_id();

  /* Id can be empty if:
     1. There is too low grace interval and the server didn't start yet,
        so the SQL query failed. It will recover next time.
     2. Some other reason that caused selecting server_id to fail. */
  if (id.length() == 0) {
    logger_.warning(
        "Collecting db_instance_id failed. It may be caused by server still "
        "initializing.");
    return true;
  }
  rapidjson::Document::AllocatorType &allocator = document->GetAllocator();
  rapidjson::Value instance_id;
  instance_id.SetString(id.c_str(), allocator);
  document->AddMember(rapidjson::StringRef(JSONKey::db_instance_id),
                      instance_id, allocator);
  return false;
}
bool DataProvider::collect_product_version_info(rapidjson::Document *document) {
  // Version doesn't change during the lifetime, so query and cache it
  if (version_cache_.empty()) {
    QueryResult result;
    if (do_query("SELECT @@VERSION, @@VERSION_COMMENT", &result)) {
      return true;
    }

    version_cache_ = result[0][0];

    // Is it 'pro' build?
    if (result[0][1].find("Pro") != std::string::npos) {
      version_cache_ += "-pro";
    }
  }

  rapidjson::Document::AllocatorType &allocator = document->GetAllocator();
  rapidjson::Value version;
  version.SetString(version_cache_.c_str(), allocator);
  // we use "pillar_version" as a key for backward compatibility with ph0
  document->AddMember(rapidjson::StringRef(JSONKey::pillar_version), version,
                      allocator);
  return false;
}

bool DataProvider::collect_plugins_info(rapidjson::Document *document) {
  QueryResult result;
  if (do_query("SELECT PLUGIN_NAME FROM information_schema.plugins WHERE "
               "PLUGIN_STATUS='ACTIVE'",
               &result)) {
    return true;
  }

  rapidjson::Document::AllocatorType &allocator = document->GetAllocator();

  rapidjson::Value plugins(rapidjson::Type::kArrayType);

  for (auto &plugin_iter : result) {
    rapidjson::Value plugin_name;
    plugin_name.SetString(plugin_iter[0].c_str(), allocator);
    plugins.PushBack(plugin_name, allocator);
  }
  document->AddMember(rapidjson::StringRef(JSONKey::active_plugins), plugins,
                      allocator);

  return false;
}

bool DataProvider::collect_components_info(rapidjson::Document *document) {
  QueryResult result;
  if (do_query("SELECT component_urn FROM mysql.component", &result)) {
    return true;
  }

  rapidjson::Document::AllocatorType &allocator = document->GetAllocator();

  rapidjson::Value components(rapidjson::Type::kArrayType);

  for (auto &component_iter : result) {
    rapidjson::Value component_name;
    component_name.SetString(component_iter[0].c_str(), allocator);
    components.PushBack(component_name, allocator);
  }
  document->AddMember(rapidjson::StringRef(JSONKey::active_components),
                      components, allocator);
  return false;
}

bool DataProvider::collect_uptime_info(rapidjson::Document *document) {
  QueryResult result;
  if (do_query("SHOW GLOBAL STATUS LIKE 'Uptime'", &result)) {
    return true;
  }

  rapidjson::Document::AllocatorType &allocator = document->GetAllocator();
  rapidjson::Value uptime;
  uptime.SetString(result[0][1].c_str(), allocator);
  document->AddMember(rapidjson::StringRef(JSONKey::uptime), uptime, allocator);
  return false;
}

bool DataProvider::collect_dbs_number_info(rapidjson::Document *document) {
  QueryResult result;
  if (do_query(
          "SELECT COUNT(*) FROM information_schema.SCHEMATA WHERE SCHEMA_NAME "
          "NOT IN('mysql', 'information_schema', 'performance_schema', 'sys')",
          &result)) {
    return true;
  }

  rapidjson::Document::AllocatorType &allocator = document->GetAllocator();
  rapidjson::Value db_cnt;
  db_cnt.SetString(result[0][0].c_str(), allocator);
  document->AddMember(rapidjson::StringRef(JSONKey::databases_count), db_cnt,
                      allocator);
  return false;
}

/* Note that this metric is update very X, so it may be inacurate.
   We could make it accurate but that would need ANALYZE TABLE for every table
   which would be overkill.*/
bool DataProvider::collect_dbs_size_info(rapidjson::Document *document) {
  QueryResult result;
  if (do_query("SELECT IFNULL(ROUND(SUM(data_length + index_length), 1), '0') "
               "size_MB FROM information_schema.tables WHERE table_schema NOT "
               "IN('mysql', 'information_schema', 'performance_schema', 'sys')",
               &result)) {
    return true;
  }

  rapidjson::Document::AllocatorType &allocator = document->GetAllocator();
  rapidjson::Value db_size;
  db_size.SetString(result[0][0].c_str(), allocator);
  document->AddMember(rapidjson::StringRef(JSONKey::databases_size), db_size,
                      allocator);
  return false;
}

bool DataProvider::collect_se_usage_info(rapidjson::Document *document) {
  QueryResult result;
  if (do_query("SELECT DISTINCT ENGINE FROM information_schema.tables WHERE "
               "table_schema NOT IN('mysql', 'information_schema', "
               "'performance_schema', 'sys');",
               &result)) {
    return true;
  }

  rapidjson::Document::AllocatorType &allocator = document->GetAllocator();
  rapidjson::Value se_engines(rapidjson::Type::kArrayType);

  for (auto &engine_iter : result) {
    rapidjson::Value engine_name;
    engine_name.SetString(engine_iter[0].c_str(), allocator);
    se_engines.PushBack(engine_name, allocator);
  }
  document->AddMember(rapidjson::StringRef(JSONKey::se_engines_in_use),
                      se_engines, allocator);
  return false;
}

bool DataProvider::collect_group_replication_info(
    rapidjson::Document *document) {
  // Do fast check if there is anything to learn about GR
  static const std::string query_base(
      "SELECT MEMBER_ROLE, @@global.group_replication_group_name, "
      "@@global.group_replication_single_primary_mode FROM "
      "performance_schema.replication_group_members WHERE MEMBER_STATE != "
      "'OFFLINE'");
  std::ostringstream ss;
  ss << query_base << " AND MEMBER_ID='" << get_database_instance_id() << "'";
  QueryResult result;
  if (do_query(ss.str(), &result, nullptr, true)) {
    /* Ideally we should get ER_UNKNOWN_SYSTEM_VARIABLE (1193) if GR plugin
       is not installed. Unfortunately
       mysql_command_error_info_service.sql_errno() returns 0 in case of such
       failure. It returns proper string message, but we cannot rely on it as it
       can be localized. Do the best possible: if the query failed, it means "no
       GR replication" */
    /*
    if (err_no == ER_UNKNOWN_SYSTEM_VARIABLE) {
      return false;
    }
    */
    return false;
  }

  if (result.size() > 0) {
    // We've got some rows. Try to collect more details.
    rapidjson::Document::AllocatorType &allocator = document->GetAllocator();
    rapidjson::Document gr_json(rapidjson::Type::kObjectType);

    rapidjson::Value role;
    role.SetString(result[0][0].c_str(), allocator);
    gr_json.AddMember(rapidjson::StringRef(JSONKey::role), role, allocator);

    db_replication_id_solver_->vote(
        result[0][1], DbReplicationIdSolver::Voter::GROUP_REPLICATION);

    rapidjson::Value single_primary_mode;
    single_primary_mode.SetString(result[0][2].c_str(), allocator);
    gr_json.AddMember(rapidjson::StringRef(JSONKey::single_primary_mode),
                      single_primary_mode, allocator);

    /* replication group size */
    if (!do_query(
            "SELECT COUNT(*) FROM performance_schema.replication_group_members",
            &result)) {
      rapidjson::Value group_size;
      group_size.SetString(result[0][0].c_str(), allocator);
      gr_json.AddMember(rapidjson::StringRef(JSONKey::group_size), group_size,
                        allocator);
    }

    document->AddMember(rapidjson::StringRef(JSONKey::group_replication_info),
                        gr_json, allocator);
  }
  return false;
}

bool DataProvider::collect_async_replication_info(
    rapidjson::Document *document) {
  bool is_source = false;
  bool is_replica = false;
  bool is_semisync_source = false;
  bool is_semisync_replica = false;

  // If we are source
  QueryResult result;
  if (do_query("SHOW REPLICAS", &result)) {
    return true;
  }
  is_source = result.size() > 0;

  // If we are replica
  if (do_query("SHOW REPLICA STATUS", &result)) {
    return true;
  }
  is_replica = result.size() > 0;

  // Name of the variable depends on what plugin was installed
  // If we are semisync source
  if (!do_query("SELECT @@global.rpl_semi_sync_source_enabled", &result,
                nullptr, true) ||
      !do_query("SELECT @@global.rpl_semi_sync_master_enabled", &result,
                nullptr, true)) {
    is_semisync_source = !result[0][0].compare("1");
    is_semisync_source &= is_source;
  }
  // If we are semisync replica
  if (!do_query("SELECT @@global.rpl_semi_sync_replica_enabled", &result,
                nullptr, true) ||
      !do_query("SELECT @@global.rpl_semi_sync_slave_enabled", &result, nullptr,
                true)) {
    is_semisync_replica = !result[0][0].compare("1");
    is_semisync_replica &= is_replica;
  }

  if (is_source || is_replica || is_semisync_source || is_semisync_replica) {
    rapidjson::Document::AllocatorType &allocator = document->GetAllocator();
    rapidjson::Document r_json(rapidjson::Type::kObjectType);

    if (is_semisync_source) {
      rapidjson::Value is_semisync_source_json;
      is_semisync_source_json.SetString(b2s(is_semisync_source), allocator);
      r_json.AddMember(rapidjson::StringRef(JSONKey::is_semisync_source),
                       is_semisync_source_json, allocator);
    } else if (is_source) {
      rapidjson::Value is_source_json;
      is_source_json.SetString(b2s(is_source), allocator);
      r_json.AddMember(rapidjson::StringRef(JSONKey::is_source), is_source_json,
                       allocator);
    }

    if (is_semisync_replica) {
      rapidjson::Value is_semisync_replica_json;
      is_semisync_replica_json.SetString(b2s(is_semisync_replica), allocator);
      r_json.AddMember(rapidjson::StringRef(JSONKey::is_semisync_replica),
                       is_semisync_replica_json, allocator);
    } else if (is_replica) {
      rapidjson::Value is_replica_json;
      is_replica_json.SetString(b2s(is_replica), allocator);
      r_json.AddMember(rapidjson::StringRef(JSONKey::is_replica),
                       is_replica_json, allocator);
    }

    // we had to log something
    assert(!r_json.ObjectEmpty());
    if (r_json.ObjectEmpty()) {
      return true;
    }

    document->AddMember(rapidjson::StringRef(JSONKey::replication_info), r_json,
                        allocator);
  }

  return false;
}

bool DataProvider::collect_db_replication_id(rapidjson::Document *document) {
  const std::string &id = db_replication_id_solver_->get_db_replication_id();
  if (id.length() > 0) {
    rapidjson::Document::AllocatorType &allocator = document->GetAllocator();
    rapidjson::Value replication_group_id;
    replication_group_id.SetString(id.c_str(), allocator);
    document->AddMember(rapidjson::StringRef(JSONKey::db_replication_id),
                        replication_group_id, allocator);
  }

  return false;
}

bool DataProvider::collect_metrics(rapidjson::Document *document) {
  /* The configuration of this instance might have changed, so we need to colect
     it every time. */
  db_replication_id_solver_->reset();

  bool res = collect_db_instance_id_info(document);

  // If db_instance_id cannot be collected all other metrics are meaningless
  if (res) {
    logger_.info(
        "Collecting db_instance_id failed. Skipping metrics scaping this time");
    return true;
  }

  // Do the best we can, collect what is possible, even if something fails.
  res |= collect_product_version_info(document);
  res |= collect_plugins_info(document);
  res |= collect_components_info(document);
  res |= collect_uptime_info(document);
  res |= collect_dbs_number_info(document);
  res |= collect_dbs_size_info(document);
  res |= collect_se_usage_info(document);
  res |= collect_group_replication_info(document);
  res |= collect_async_replication_info(document);

  /* The requirement is to have db_replication_id key at the top of JSON
  structure. But it may originate from the different places. The above
  collect_* methods may set their proposals to db_replication_id_solver_
  and it is up to db_replication_id_solver_ to decide which one to use. */
  res |= collect_db_replication_id(document);
  return res;
}

std::string DataProvider::get_report() {
  rapidjson::Document JSON(rapidjson::Type::kObjectType);

  if (collect_metrics(&JSON) && !JSON.ObjectEmpty()) {
    logger_.info("Collecting of some metrics failed.");
  }

  if (JSON.ObjectEmpty()) {
    logger_.info("Collecting of all metrics failed.");
    return std::string();
  }

  rapidjson::StringBuffer string_buffer;
  string_buffer.Clear();
  RapidJsonWritterType string_writer(string_buffer);
  JSON.Accept(string_writer);
  std::string json(string_buffer.GetString(), string_buffer.GetSize());

  return json;
}
