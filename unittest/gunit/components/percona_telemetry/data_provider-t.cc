#include <gmock/gmock.h>
#include <gtest/gtest.h>

#define private public
#include "components/percona_telemetry/data_provider.h"
#include "components/percona_telemetry/logger.h"
#undef private

using ::testing::_;
using ::testing::A;
using ::testing::DoAll;
using ::testing::Eq;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::WithArg;

SERVICE_TYPE_NO_CONST(mysql_command_factory) command_factory;
SERVICE_TYPE_NO_CONST(mysql_command_options) command_options;
SERVICE_TYPE_NO_CONST(mysql_command_query) command_query;
SERVICE_TYPE_NO_CONST(mysql_command_query_result) command_query_result;
SERVICE_TYPE_NO_CONST(mysql_command_field_info) command_field_info;
SERVICE_TYPE_NO_CONST(mysql_command_error_info) command_error_info;
SERVICE_TYPE_NO_CONST(mysql_command_thread) command_thread;

SERVICE_TYPE_NO_CONST(log_builtins) log_builtins_srv;
SERVICE_TYPE_NO_CONST(log_builtins_string) log_builtins_string_srv;
Logger logger(log_builtins_srv, log_builtins_string_srv);

namespace data_provider_unittests {

class MockDataProvider : public DataProvider {
 public:
  MockDataProvider()
      : DataProvider(command_factory, command_options, command_query,
                     command_query_result, command_field_info,
                     command_error_info, command_thread, logger) {}
  MOCK_METHOD(bool, do_query,
              (const std::string &query, QueryResult *result,
               unsigned int *err_no, bool suppress_query_error_log),
              (override));
};

class DataProviderTest : public ::testing::Test {
 protected:
  virtual void SetUp() {}
  virtual void TearDown() {}
};

TEST_F(DataProviderTest, Sanity_test) { EXPECT_EQ(1, 1); }

TEST_F(DataProviderTest, get_database_instance_id_test) {
  MockDataProvider dataProvider;
  const std::string expected_id("expected_id");
  Row row{expected_id};

  EXPECT_CALL(dataProvider, do_query(Eq(std::string("SELECT @@server_uuid")),
                                     A<QueryResult *>(), _, _))
      .Times(1)
      .WillOnce(DoAll(
          WithArg<1>(Invoke([&row](QueryResult *qr) { qr->push_back(row); })),
          Return(false)));

  EXPECT_EQ(dataProvider.get_database_instance_id(), expected_id);
}

TEST_F(DataProviderTest, get_database_instance_id_queries_only_once_test) {
  MockDataProvider dataProvider;
  const std::string expected_id("expected_id");
  Row row{expected_id};

  EXPECT_CALL(dataProvider, do_query(_, A<QueryResult *>(), _, _))
      .Times(1)
      .WillOnce(DoAll(
          WithArg<1>(Invoke([&row](QueryResult *qr) { qr->push_back(row); })),
          Return(false)));

  EXPECT_EQ(dataProvider.get_database_instance_id(), expected_id);
  // If we call it again, it should use cached value.
  // If it calls do_query() again, the above Times(1) expectation will fail
  EXPECT_EQ(dataProvider.get_database_instance_id(), expected_id);
}

TEST_F(DataProviderTest, get_database_instance_id_do_query_fails_test) {
  MockDataProvider dataProvider;
  const std::string expected_id("expected_id");
  Row row{expected_id};

  EXPECT_CALL(dataProvider, do_query(_, A<QueryResult *>(), _, _))
      .Times(2)
      .WillOnce(Return(true))
      .WillOnce(DoAll(
          WithArg<1>(Invoke([&row](QueryResult *qr) { qr->push_back(row); })),
          Return(false)));

  // do_query fails for the 1st time. Empty string is expected as the result
  EXPECT_EQ(dataProvider.get_database_instance_id(), "");
  // If we call it again, it should call do_query() again, as the previous
  // attempt failed.
  EXPECT_EQ(dataProvider.get_database_instance_id(), expected_id);
}

// Helper function for metrics reported as a single value
static void collect_single_value_common(
    const std::string &expected_query, const std::string &expected_json_key,
    std::function<bool(MockDataProvider *, rapidjson::Document *)> fn,
    bool should_cache_result = false) {
  MockDataProvider dataProvider;
  const std::string expected_val("expected_val");
  Row row{expected_val};

  EXPECT_CALL(dataProvider,
              do_query(Eq(expected_query), A<QueryResult *>(), _, _))
      .Times(should_cache_result ? 1 : 2)
      .WillRepeatedly(DoAll(
          WithArg<1>(Invoke([&row](QueryResult *qr) { qr->push_back(row); })),
          Return(false)));

  // KH:
  rapidjson::Document document(rapidjson::Type::kObjectType);
  EXPECT_FALSE(fn(&dataProvider, &document));

  EXPECT_TRUE(document.HasMember(expected_json_key.c_str()));
  auto iter = document.FindMember(expected_json_key.c_str());
  EXPECT_STREQ(iter->value.GetString(), expected_val.c_str());

  // The above expect Times(N) cares about caching check
  rapidjson::Document document2(rapidjson::Type::kObjectType);
  EXPECT_FALSE(fn(&dataProvider, &document2));
}

// Helper function for metrics reported as an array
static void collect_array_info_common(
    const std::string &expected_query, const std::string &expected_json_key,
    std::function<bool(MockDataProvider *, rapidjson::Document *)> fn) {
  MockDataProvider dataProvider;
  std::string Item_A("Item_A");
  std::string Item_B("Item_B");
  Row row1{Item_A};
  Row row2{Item_B};

  EXPECT_CALL(dataProvider,
              do_query(Eq(expected_query), A<QueryResult *>(), _, _))
      .Times(2)
      .WillRepeatedly(DoAll(WithArg<1>(Invoke([&row1, &row2](QueryResult *qr) {
                              qr->push_back(row1);
                              qr->push_back(row2);
                            })),
                            Return(false)));

  rapidjson::Document document(rapidjson::Type::kObjectType);
  EXPECT_FALSE(fn(&dataProvider, &document));

  EXPECT_TRUE(document.HasMember(expected_json_key.c_str()));
  auto iter = document.FindMember(expected_json_key.c_str());
  EXPECT_TRUE(iter->value.IsArray());
  EXPECT_EQ(iter->value.GetArray().Size(), 2);

  bool item_a_found = false;
  bool item_b_found = false;

  if (Item_A.compare(iter->value.GetArray()[0].GetString()) == 0 ||
      Item_A.compare(iter->value.GetArray()[1].GetString()) == 0) {
    item_a_found = true;
  }
  if (Item_B.compare(iter->value.GetArray()[0].GetString()) == 0 ||
      Item_B.compare(iter->value.GetArray()[1].GetString()) == 0) {
    item_b_found = true;
  }

  EXPECT_TRUE(item_a_found && item_b_found);

  // Let's do it once again, to ensure, that values are not cached
  rapidjson::Document document2(rapidjson::Type::kObjectType);
  EXPECT_FALSE(fn(&dataProvider, &document2));
}

TEST_F(DataProviderTest, collect_db_instance_id_info_test) {
  const std::string query("SELECT @@server_uuid");
  const std::string expected_json_key("db_instance_id");
  collect_single_value_common(query, expected_json_key,
                              &MockDataProvider::collect_db_instance_id_info,
                              true);
}

TEST_F(DataProviderTest, collect_product_version_info_test) {
  MockDataProvider dataProvider;
  Row row{"1.2.3.4", "This is version comment"};

  EXPECT_CALL(dataProvider,
              do_query(Eq(std::string("SELECT @@VERSION, @@VERSION_COMMENT")),
                       A<QueryResult *>(), _, _))
      .Times(1)
      .WillOnce(DoAll(
          WithArg<1>(Invoke([&row](QueryResult *qr) { qr->push_back(row); })),
          Return(false)));

  rapidjson::Document document(rapidjson::Type::kObjectType);
  EXPECT_FALSE(dataProvider.collect_product_version_info(&document));

  EXPECT_TRUE(document.HasMember("pillar_version"));
  auto iter = document.FindMember("pillar_version");
  EXPECT_STREQ(iter->value.GetString(), "1.2.3.4");

  // The 2nd call will use cached version, do_query() won't be called
  rapidjson::Document document2(rapidjson::Type::kObjectType);
  EXPECT_FALSE(dataProvider.collect_product_version_info(&document2));
}

TEST_F(DataProviderTest, collect_product_version_info_pro_test) {
  MockDataProvider dataProvider;
  Row row{"1.2.3.4", "This is Pro build version comment"};

  EXPECT_CALL(dataProvider,
              do_query(Eq(std::string("SELECT @@VERSION, @@VERSION_COMMENT")),
                       A<QueryResult *>(), _, _))
      .Times(1)
      .WillOnce(DoAll(
          WithArg<1>(Invoke([&row](QueryResult *qr) { qr->push_back(row); })),
          Return(false)));

  rapidjson::Document document(rapidjson::Type::kObjectType);
  EXPECT_FALSE(dataProvider.collect_product_version_info(&document));

  EXPECT_TRUE(document.HasMember("pillar_version"));
  auto iter = document.FindMember("pillar_version");
  EXPECT_STREQ(iter->value.GetString(), "1.2.3.4-pro");
}

TEST_F(DataProviderTest, collect_plugins_info_test) {
  const std::string query(
      std::string("SELECT PLUGIN_NAME FROM information_schema.plugins WHERE "
                  "PLUGIN_STATUS='ACTIVE'"));
  const std::string expected_json_key("active_plugins");
  collect_array_info_common(query, expected_json_key,
                            &MockDataProvider::collect_plugins_info);
}

TEST_F(DataProviderTest, collect_components_info_test) {
  const std::string query(
      std::string("SELECT component_urn FROM mysql.component"));
  const std::string expected_json_key("active_components");
  collect_array_info_common(query, expected_json_key,
                            &MockDataProvider::collect_components_info);
}

TEST_F(DataProviderTest, collect_uptime_info_test) {
  MockDataProvider dataProvider;
  Row row{"Uptime", "12345"};

  EXPECT_CALL(dataProvider,
              do_query(Eq(std::string("SHOW GLOBAL STATUS LIKE 'Uptime'")),
                       A<QueryResult *>(), _, _))
      .Times(1)
      .WillOnce(DoAll(
          WithArg<1>(Invoke([&row](QueryResult *qr) { qr->push_back(row); })),
          Return(false)));

  rapidjson::Document document(rapidjson::Type::kObjectType);
  EXPECT_FALSE(dataProvider.collect_uptime_info(&document));

  EXPECT_TRUE(document.HasMember("uptime"));
  auto iter = document.FindMember("uptime");
  EXPECT_STREQ(iter->value.GetString(), "12345");
}

TEST_F(DataProviderTest, collect_dbs_number_info_test) {
  const std::string query(std::string(
      "SELECT COUNT(*) FROM information_schema.SCHEMATA WHERE SCHEMA_NAME NOT "
      "IN('mysql', 'information_schema', 'performance_schema', 'sys')"));
  const std::string expected_json_key("databases_count");
  collect_single_value_common(query, expected_json_key,
                              &MockDataProvider::collect_dbs_number_info);
}

TEST_F(DataProviderTest, collect_dbs_size_info_test) {
  const std::string query(std::string(
      "SELECT IFNULL(ROUND(SUM(data_length + index_length), 1), '0') "
      "size_MB FROM information_schema.tables WHERE table_schema NOT "
      "IN('mysql', 'information_schema', 'performance_schema', 'sys')"));
  const std::string expected_json_key("databases_size");
  collect_single_value_common(query, expected_json_key,
                              &MockDataProvider::collect_dbs_size_info);
}

TEST_F(DataProviderTest, collect_se_usage_info_test) {
  const std::string query(
      std::string("SELECT DISTINCT ENGINE FROM information_schema.tables WHERE "
                  "table_schema NOT IN('mysql', 'information_schema', "
                  "'performance_schema', 'sys')"));
  const std::string expected_json_key("se_engines_in_use");
  collect_array_info_common(query, expected_json_key,
                            &MockDataProvider::collect_se_usage_info);
}

TEST_F(DataProviderTest, collect_group_replication_info_test) {
  MockDataProvider dataProvider;

  const std::string query1(std::string(
      "SELECT MEMBER_ROLE, @@global.group_replication_group_name, "
      "@@global.group_replication_single_primary_mode FROM "
      "performance_schema.replication_group_members WHERE MEMBER_STATE != "
      "'OFFLINE' AND MEMBER_ID='expected_id'"));

  const std::string role_val("role_val");
  const std::string db_id_val("db_id_val");
  const std::string single_primary_mode_val("single_primary_mode_val");
  Row row1{role_val, db_id_val, single_primary_mode_val};

  EXPECT_CALL(dataProvider, do_query(Eq(query1), A<QueryResult *>(), _, _))
      .Times(1)
      .WillOnce(DoAll(WithArg<1>(Invoke([&row1](QueryResult *qr) {
                        qr->clear();
                        qr->push_back(row1);
                      })),
                      Return(false)));

  const std::string query2(std::string(
      "SELECT COUNT(*) FROM performance_schema.replication_group_members"));

  const std::string count_val("count_val");
  Row row2{count_val};

  EXPECT_CALL(dataProvider, do_query(Eq(query2), A<QueryResult *>(), _, _))
      .Times(1)
      .WillOnce(DoAll(WithArg<1>(Invoke([&row2](QueryResult *qr) {
                        qr->clear();
                        qr->push_back(row2);
                      })),
                      Return(false)));

  const std::string expected_id("expected_id");
  Row row3{expected_id};

  EXPECT_CALL(dataProvider, do_query(Eq(std::string("SELECT @@server_uuid")),
                                     A<QueryResult *>(), _, _))
      .Times(1)
      .WillOnce(DoAll(WithArg<1>(Invoke([&row3](QueryResult *qr) {
                        qr->clear();
                        qr->push_back(row3);
                      })),
                      Return(false)));

  rapidjson::Document document(rapidjson::Type::kObjectType);
  EXPECT_FALSE(dataProvider.collect_group_replication_info(&document));

  EXPECT_TRUE(document.HasMember("group_replication_info"));
  auto gr_iter = document.FindMember("group_replication_info");

  EXPECT_TRUE(gr_iter->value.HasMember("role"));
  auto iter = gr_iter->value.FindMember("role");
  EXPECT_STREQ(iter->value.GetString(), role_val.c_str());

  EXPECT_TRUE(gr_iter->value.HasMember("single_primary_mode"));
  iter = gr_iter->value.FindMember("single_primary_mode");
  EXPECT_STREQ(iter->value.GetString(), single_primary_mode_val.c_str());

  EXPECT_TRUE(gr_iter->value.HasMember("group_size"));
  iter = gr_iter->value.FindMember("group_size");
  EXPECT_STREQ(iter->value.GetString(), count_val.c_str());
}

TEST_F(DataProviderTest, collect_async_replication_info_test) {
  MockDataProvider dataProvider;

  EXPECT_CALL(dataProvider, do_query(Eq(std::string("SHOW REPLICAS")),
                                     A<QueryResult *>(), _, _))
      .Times(1)
      .WillOnce(DoAll(WithArg<1>(Invoke([](QueryResult *qr) {
                        qr->clear();
                        qr->push_back(Row{"foo"});
                      })),
                      Return(false)));

  EXPECT_CALL(dataProvider, do_query(Eq(std::string("SHOW REPLICA STATUS")),
                                     A<QueryResult *>(), _, _))
      .Times(1)
      .WillOnce(DoAll(WithArg<1>(Invoke([](QueryResult *qr) {
                        qr->clear();
                        qr->push_back(Row{"bar"});
                      })),
                      Return(false)));

  // check that we try new naming first
  EXPECT_CALL(
      dataProvider,
      do_query(Eq(std::string("SELECT @@global.rpl_semi_sync_source_enabled")),
               A<QueryResult *>(), _, _))
      .Times(1)
      .WillOnce(Return(true));

  // then return actual result when old naming used
  EXPECT_CALL(
      dataProvider,
      do_query(Eq(std::string("SELECT @@global.rpl_semi_sync_master_enabled")),
               A<QueryResult *>(), _, _))
      .Times(1)
      .WillOnce(DoAll(WithArg<1>(Invoke([](QueryResult *qr) {
                        qr->clear();
                        qr->push_back(Row{"1"});
                      })),
                      Return(false)));

  // the same, but for replica
  EXPECT_CALL(
      dataProvider,
      do_query(Eq(std::string("SELECT @@global.rpl_semi_sync_replica_enabled")),
               A<QueryResult *>(), _, _))
      .Times(1)
      .WillOnce(Return(true));

  // then return actual result when old naming used
  EXPECT_CALL(
      dataProvider,
      do_query(Eq(std::string("SELECT @@global.rpl_semi_sync_slave_enabled")),
               A<QueryResult *>(), _, _))
      .Times(1)
      .WillOnce(DoAll(WithArg<1>(Invoke([](QueryResult *qr) {
                        qr->clear();
                        qr->push_back(Row{"0"});  // but we are not semi-sync
                      })),
                      Return(false)));

  rapidjson::Document document(rapidjson::Type::kObjectType);
  EXPECT_FALSE(dataProvider.collect_async_replication_info(&document));

  EXPECT_TRUE(document.HasMember("replication_info"));
  auto ri_iter = document.FindMember("replication_info");

  EXPECT_TRUE(ri_iter->value.HasMember("is_semisync_source"));
  auto iter = ri_iter->value.FindMember("is_semisync_source");
  EXPECT_STREQ(iter->value.GetString(), "1");

  EXPECT_TRUE(ri_iter->value.HasMember("is_replica"));
  iter = ri_iter->value.FindMember("is_replica");
  EXPECT_STREQ(iter->value.GetString(), "1");
}

}  // namespace data_provider_unittests