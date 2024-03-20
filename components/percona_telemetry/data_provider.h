#ifndef PERCONA_TELEMETRY_DATA_PROVIDER_H
#define PERCONA_TELEMETRY_DATA_PROVIDER_H

#include <memory>
#include <string>
#include <vector>

#include <mysql/components/component_implementation.h>
#include <mysql/components/services/log_builtins.h>
#include <mysql/components/services/mysql_command_services.h>
#include <mysql/components/services/security_context.h>

#ifdef RAPIDJSON_NO_SIZETYPEDEFINE
#include "my_rapidjson_size_t.h"
#endif

#include <rapidjson/document.h>

#define COMPONENT_PERCONA_TELEMETRY_DEBUG_JSON_PRINTER
#ifndef COMPONENT_PERCONA_TELEMETRY_DEBUG_JSON_PRINTER
#include <rapidjson/writer.h>
using RapidJsonWritterType = rapidjson::Writer<rapidjson::StringBuffer>;
#else
#include <rapidjson/prettywriter.h>
using RapidJsonWritterType = rapidjson::PrettyWriter<rapidjson::StringBuffer>;
#endif

#include <rapidjson/stringbuffer.h>

using Row = std::vector<std::string>;
using QueryResult = std::vector<Row>;

class Logger;
class DbReplicationIdSolver;

class DataProvider {
 public:
  DataProvider(
      SERVICE_TYPE(mysql_command_factory) & command_factory_service,
      SERVICE_TYPE(mysql_command_options) & command_options_service,
      SERVICE_TYPE(mysql_command_query) & command_query_service,
      SERVICE_TYPE(mysql_command_query_result) & command_query_result_service,
      SERVICE_TYPE(mysql_command_field_info) & command_field_info_service,
      SERVICE_TYPE(mysql_command_error_info) & command_error_info_service,
      SERVICE_TYPE(mysql_command_thread) & command_thread_service,
      Logger &logger);

  ~DataProvider() = default;

  DataProvider(const DataProvider &rhs) = delete;
  DataProvider(DataProvider &&rhs) = delete;
  DataProvider &operator=(const DataProvider &rhs) = delete;
  DataProvider &operator=(DataProvider &&rhs) = delete;

  void thread_access_begin();
  void thread_access_end();
  std::string get_report();

 private:
  bool do_query(const std::string &query, QueryResult *result,
                unsigned int *err_no = nullptr,
                bool suppress_query_error_log = false);
  bool collect_db_instance_id_info(rapidjson::Document *document);
  bool collect_product_version_info(rapidjson::Document *document);
  bool collect_plugins_info(rapidjson::Document *document);
  bool collect_components_info(rapidjson::Document *document);
  bool collect_uptime_info(rapidjson::Document *document);
  bool collect_dbs_number_info(rapidjson::Document *document);
  bool collect_dbs_size_info(rapidjson::Document *document);
  bool collect_se_usage_info(rapidjson::Document *document);
  bool collect_group_replication_info(rapidjson::Document *document);
  bool collect_async_replication_info(rapidjson::Document *document);
  bool collect_db_replication_id(rapidjson::Document *document);
  bool collect_metrics(rapidjson::Document *document);

  const std::string &get_database_instance_id();

  SERVICE_TYPE(mysql_command_factory) & command_factory_service_;
  SERVICE_TYPE(mysql_command_options) & command_options_service_;
  SERVICE_TYPE(mysql_command_query) & command_query_service_;
  SERVICE_TYPE(mysql_command_query_result) & command_query_result_service_;
  SERVICE_TYPE(mysql_command_field_info) & command_field_info_service_;
  SERVICE_TYPE(mysql_command_error_info) & command_error_info_service_;
  SERVICE_TYPE(mysql_command_thread) & command_thread_service_;

  Logger &logger_;
  std::shared_ptr<DbReplicationIdSolver> db_replication_id_solver_;

  std::string database_instance_id_cache_;
  std::string version_cache_;
};

#endif /* PERCONA_TELEMETRY_DATA_PROVIDER_H */
