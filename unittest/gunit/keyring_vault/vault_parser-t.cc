/* Copyright (c) 2018, 2021 Percona LLC and/or its affiliates. All rights
   reserved.

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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <optional>

#include "i_keys_container.h"
#include "mock_logger.h"
#include "my_rnd.h"
#include "vault_base64.h"
#include "vault_credentials.h"
#include "vault_key.h"
#include "vault_parser_composer.h"

namespace keyring__vault_parser_unittest {
using namespace keyring;

using ::testing::StrEq;
typedef keyring::IVault_parser_composer::KeyParameters KeyParameters;

class Vault_parser_test : public ::testing::Test {
 protected:
  virtual void SetUp() { logger = new Mock_logger(); }

  virtual void TearDown() { delete logger; }

 protected:
  ILogger *logger;
};

TEST_F(Vault_parser_test, ParseKeySignature) {
  std::string key_signature("4_key13_rob");
  Secure_string encoded_key_signature;
  EXPECT_FALSE(Vault_base64::encode(
      key_signature.c_str(), key_signature.length(), &encoded_key_signature,
      Vault_base64::Format::SINGLE_LINE));
  Vault_parser_composer vault_parser(logger);
  KeyParameters key_parameters;
  EXPECT_FALSE(
      vault_parser.parse_key_signature(encoded_key_signature, &key_parameters));
  EXPECT_STREQ(key_parameters[0].c_str(), "key1");
  EXPECT_STREQ(key_parameters[1].c_str(), "rob");
}

TEST_F(Vault_parser_test, ParseKeySignature2) {
  std::string key_signature("4_key16_Robert");
  Secure_string encoded_key_signature;
  EXPECT_FALSE(Vault_base64::encode(
      key_signature.c_str(), key_signature.length(), &encoded_key_signature,
      Vault_base64::Format::SINGLE_LINE));
  Vault_parser_composer vault_parser(logger);
  KeyParameters key_parameters;
  EXPECT_FALSE(
      vault_parser.parse_key_signature(encoded_key_signature, &key_parameters));
  EXPECT_STREQ(key_parameters[0].c_str(), "key1");
  EXPECT_STREQ(key_parameters[1].c_str(), "Robert");
}

TEST_F(Vault_parser_test, ParseKeySignature3) {
  std::string key_signature("7__key1238_Robert33");
  Secure_string encoded_key_signature;
  EXPECT_FALSE(Vault_base64::encode(
      key_signature.c_str(), key_signature.length(), &encoded_key_signature,
      Vault_base64::Format::SINGLE_LINE));
  Vault_parser_composer vault_parser(logger);
  KeyParameters key_parameters;
  EXPECT_FALSE(
      vault_parser.parse_key_signature(encoded_key_signature, &key_parameters));
  EXPECT_STREQ(key_parameters[0].c_str(), "_key123");
  EXPECT_STREQ(key_parameters[1].c_str(), "Robert33");
}

TEST_F(Vault_parser_test, ParseKeySignature4) {
  std::string key_signature("9_123key12310_12Robert33");
  Secure_string encoded_key_signature;
  EXPECT_FALSE(Vault_base64::encode(
      key_signature.c_str(), key_signature.length(), &encoded_key_signature,
      Vault_base64::Format::SINGLE_LINE));
  Vault_parser_composer vault_parser(logger);
  KeyParameters key_parameters;
  EXPECT_FALSE(
      vault_parser.parse_key_signature(encoded_key_signature, &key_parameters));
  EXPECT_STREQ(key_parameters[0].c_str(), "123key123");
  EXPECT_STREQ(key_parameters[1].c_str(), "12Robert33");
}

TEST_F(Vault_parser_test, ParseKeySignature5) {
  std::string key_signature(
      "48_INNODBKey-3c40d1ab-1475-11e7-ae1c-9cb6d0d5dc99-10_");
  Secure_string encoded_key_signature;
  EXPECT_FALSE(Vault_base64::encode(
      key_signature.c_str(), key_signature.length(), &encoded_key_signature,
      Vault_base64::Format::SINGLE_LINE));

  Vault_parser_composer vault_parser(logger);
  KeyParameters key_parameters;
  EXPECT_FALSE(
      vault_parser.parse_key_signature(encoded_key_signature, &key_parameters));
  EXPECT_STREQ(key_parameters[0].c_str(),
               "INNODBKey-3c40d1ab-1475-11e7-ae1c-9cb6d0d5dc99-1");
  EXPECT_STREQ(key_parameters[1].c_str(), "");
}

TEST_F(Vault_parser_test, ParseVaultPayload) {
  std::string key1_signature("4_key13_rob");
  std::string key2_signature("4_key23_rob");
  Secure_string encoded_key1_signature;
  EXPECT_FALSE(Vault_base64::encode(
      key1_signature.c_str(), key1_signature.length(), &encoded_key1_signature,
      Vault_base64::Format::SINGLE_LINE));
  Secure_string encoded_key2_signature;
  EXPECT_FALSE(Vault_base64::encode(
      key2_signature.c_str(), key2_signature.length(), &encoded_key2_signature,
      Vault_base64::Format::SINGLE_LINE));

  Secure_string payload(
      "{\"request_id\":\"724a5ad6-7ee3-7950-879a-488a261a03ec\","
      "\"lease_id\":\"\",\"renewable\":false,\"lease_duration\":0,"
      "\"data\":{\"keys\":[\"");
  payload += encoded_key1_signature.c_str();
  payload += "\",\"";
  payload += encoded_key2_signature.c_str();
  payload += "\"]},\"wrap_info\":null,\"warnings\":null,\"auth\":null}";
  Vault_keys_list keys;
  Vault_parser_composer vault_parser(logger);
  EXPECT_FALSE(
      vault_parser.parse_keys(payload, [&keys](std::unique_ptr<Vault_key> key) {
        keys.push_back(key.release());
      }));
  EXPECT_EQ(keys.size(), static_cast<uint>(2));
  EXPECT_TRUE(keys.has_next_key());
  IKey *key_loaded = nullptr;
  EXPECT_FALSE(keys.get_next_key(&key_loaded));
  EXPECT_STREQ(key_loaded->get_key_signature()->c_str(), "4_key13_rob");
  EXPECT_TRUE(keys.has_next_key());
  delete key_loaded;
  key_loaded = nullptr;
  EXPECT_FALSE(keys.get_next_key(&key_loaded));
  EXPECT_STREQ(key_loaded->get_key_signature()->c_str(), "4_key23_rob");
  EXPECT_FALSE(keys.has_next_key());
  delete key_loaded;
  key_loaded = nullptr;
}

TEST_F(Vault_parser_test, ParseVaultPayloadEmptyKeyList) {
  Secure_string payload(
      "{\"request_id\":\"724a5ad6-7ee3-7950-879a-488a261a03ec\","
      "\"lease_id\":\"\",\"renewable\":false,\"lease_duration\":0,"
      "\"data\":{\"keys\":[]},\"wrap_info"
      "\":null,\"warnings\":null,\"auth\":null}");
  Vault_keys_list keys;
  Vault_parser_composer vault_parser(logger);
  EXPECT_FALSE(
      vault_parser.parse_keys(payload, [&keys](std::unique_ptr<Vault_key> key) {
        keys.push_back(key.release());
      }));
  EXPECT_EQ(keys.size(), static_cast<uint>(0));
}

TEST_F(Vault_parser_test, ParseVaultPayloadNoKeyList) {
  Secure_string payload(
      "{\"request_id\":\"724a5ad6-7ee3-7950-879a-488a261a03ec\","
      "\"lease_id\":\"\",\"renewable\":false,\"lease_duration\":0,"
      "\"data\":{},\"wrap_info"
      "\":null,\"warnings\":null,\"auth\":null}");
  Vault_keys_list keys;
  Vault_parser_composer vault_parser(logger);
  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(logger)),
              log(MY_ERROR_LEVEL, StrEq("Vault Server response[\"data\"] "
                                        "does not have \"keys\" member")));
  EXPECT_TRUE(
      vault_parser.parse_keys(payload, [&keys](std::unique_ptr<Vault_key> key) {
        keys.push_back(key.release());
      }));
  EXPECT_EQ(keys.size(), static_cast<uint>(0));
}

TEST_F(Vault_parser_test, ParseKeyData) {
  // Robi - encoded base64 = Um9iaQ==

  Secure_string payload(
      "{\"request_id\":\"77626d44-edbd-c82f-8220-c3c6b13ef2e1\","
      "\"lease_id\":\"\",\"renewable\":false,\"lease_duration\""
      ":2764800,\"data\":{\"type\":\"AES\",\"value\":\"Um9iaQ==\"},"
      "\"wrap_info\":null,\"warnings\":null,\"auth\":null}");

  Vault_parser_composer vault_parser(logger);
  Vault_key key("key1", nullptr, "rob", nullptr, 0);
  EXPECT_FALSE(vault_parser.parse_key_data(payload, &key, Vault_version_v1));
  EXPECT_STREQ(key.get_key_signature()->c_str(), "4_key13_rob");
  ASSERT_TRUE(memcmp(key.get_key_data(), "Robi", key.get_key_data_size()) == 0);
  EXPECT_STREQ("AES", key.get_key_type_as_string()->c_str());
}

TEST_F(Vault_parser_test, ParseKeyDataMissingTypeTag) {
  Secure_string payload(
      "{\"request_id\":\"77626d44-edbd-c82f-8220-c3c6b13ef2e1\","
      "\"lease_id\":\"\",\"renewable\":false,\"lease_duration\""
      ":2764800,\"data\":{\"value\":\"Um9iaQ==\"},"
      "\"wrap_info\":null,\"warnings\":null,\"auth\":null}");

  Vault_parser_composer vault_parser(logger);
  Vault_key key("key1", nullptr, "rob", nullptr, 0);
  EXPECT_CALL(
      *(reinterpret_cast<Mock_logger *>(logger)),
      log(MY_ERROR_LEVEL,
          StrEq("Vault Server response data does not have \"type\" member")));
  EXPECT_TRUE(vault_parser.parse_key_data(payload, &key, Vault_version_v1));
}

TEST_F(Vault_parser_test, ParseKeyDataMissingValueTag) {
  Secure_string payload(
      "{\"request_id\":\"77626d44-edbd-c82f-8220-c3c6b13ef2e1\","
      "\"lease_id\":\"\",\"renewable\":false,\"lease_duration\""
      ":2764800,\"data\":{\"type\":\"AES\"},"
      "\"wrap_info\":null,\"warnings\":null,\"auth\":null}");

  Vault_parser_composer vault_parser(logger);
  Vault_key key("key1", nullptr, "rob", nullptr, 0);
  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(logger)),
              log(MY_ERROR_LEVEL, StrEq("Vault Server response data does "
                                        "not have \"value\" member")));
  EXPECT_TRUE(vault_parser.parse_key_data(payload, &key, Vault_version_v1));
}

TEST_F(Vault_parser_test, ParseKeyDataDeleted) {
  Secure_string payload(
      "{\"request_id\":\"4559df83-fc5f-9959-314a-443c85eef27c\","
      "\"lease_id\":\"\",\"renewable\":false,\"lease_duration\":0,"
      "\"data\":{\"data\":null,\"metadata\":{"
      "\"created_time\":\"2023-09-27T14:44:07.703848947Z\","
      "\"custom_metadata\":null,"
      "\"deletion_time\":\"2023-09-27T14:44:07.705030251Z\","
      "\"destroyed\":false,\"version\":1}},"
      "\"wrap_info\":null,\"warnings\":null,\"auth\":null}");

  Vault_parser_composer vault_parser(logger);
  Vault_key key{};
  EXPECT_CALL(
      *(reinterpret_cast<Mock_logger *>(logger)),
      log(MY_INFORMATION_LEVEL, StrEq("Vault Server outdated key skipped")));
  EXPECT_TRUE(vault_parser.parse_key_data(payload, &key, Vault_version_v2));
}

TEST_F(Vault_parser_test, GetMountConfig) {
  Vault_parser_composer vault_parser(logger);
  std::size_t max_versions = 0;
  bool cas_required = false;
  Optional_secure_string delete_version_after;

  Secure_string payload(
      "{"
      "  \"request_id\": \"a2c9306a-7f82-6a59-ebfa-bc6142d66c39\","
      "  \"lease_id\": \"\","
      "  \"renewable\": false,"
      "  \"lease_duration\": 0,"
      "  \"data\": {"
      "    \"max_versions\": 42,"
      "    \"cas_required\": true,"
      "    \"delete_version_after\": \"0s\""
      "  },"
      "  \"wrap_info\": null,"
      "  \"warnings\": null,"
      "  \"auth\": null"
      "}");

  EXPECT_FALSE(vault_parser.parse_mount_point_config(
      payload, max_versions, cas_required, delete_version_after));
  EXPECT_EQ(max_versions, 42U);
  EXPECT_TRUE(cas_required);
  EXPECT_TRUE(delete_version_after != std::nullopt);
  EXPECT_STREQ(delete_version_after.value().c_str(), "0s");

  payload =
      "{"
      "  \"request_id\": \"a2c9306a-7f82-6a59-ebfa-bc6142d66c39\","
      "  \"lease_id\": \"\","
      "  \"renewable\": false,"
      "  \"lease_duration\": 0,"
      "  \"data\": {"
      "    \"max_versions\": 43,"
      "    \"cas_required\": false"
      "  },"
      "  \"wrap_info\": null,"
      "  \"warnings\": null,"
      "  \"auth\": null"
      "}";
  EXPECT_FALSE(vault_parser.parse_mount_point_config(
      payload, max_versions, cas_required, delete_version_after));
  EXPECT_EQ(max_versions, 43U);
  EXPECT_FALSE(cas_required);
  EXPECT_TRUE(delete_version_after == std::nullopt);
}

TEST_F(Vault_parser_test, GetMountConfigNull) {
  Secure_string payload(
      "{"
      "  \"request_id\": \"a2c9306a-7f82-6a59-ebfa-bc6142d66c39\","
      "  \"lease_id\": \"\","
      "  \"renewable\": false,"
      "  \"lease_duration\": 0,"
      "  \"data\": null,"
      "  \"wrap_info\": null,"
      "  \"warnings\": null,"
      "  \"auth\": null"
      "}");

  Vault_parser_composer vault_parser(logger);
  std::size_t max_versions = 0;
  bool cas_required = false;
  Optional_secure_string delete_version_after;
  EXPECT_CALL(
      static_cast<Mock_logger &>(*logger),
      log(MY_ERROR_LEVEL, StrEq("Vault Server mount config "
                                "response[\"data\"] is not an Object")));
  EXPECT_TRUE(vault_parser.parse_mount_point_config(
      payload, max_versions, cas_required, delete_version_after));
}

TEST_F(Vault_parser_test, GetMountConfigIncomplete) {
  Secure_string payload(
      "{"
      "  \"request_id\": \"a2c9306a-7f82-6a59-ebfa-bc6142d66c39\","
      "  \"lease_id\": \"\","
      "  \"renewable\": false,"
      "  \"lease_duration\": 0,"
      "  \"data\": {"
      "    \"cas_required\": true"
      "  },"
      "  \"wrap_info\": null,"
      "  \"warnings\": null,"
      "  \"auth\": null"
      "}");

  Vault_parser_composer vault_parser(logger);
  std::size_t max_versions = 0;
  bool cas_required = false;
  Optional_secure_string delete_version_after;
  EXPECT_CALL(static_cast<Mock_logger &>(*logger),
              log(MY_ERROR_LEVEL,
                  StrEq("Vault Server mount config response[\"data\"] does "
                        "not have \"max_versions\" member")));
  EXPECT_TRUE(vault_parser.parse_mount_point_config(
      payload, max_versions, cas_required, delete_version_after));
}

TEST_F(Vault_parser_test, ParsePayloadThatsGarbage) {
  uchar garbage[101000];
  my_rand_buffer(garbage, sizeof(garbage));
  Secure_string payload(reinterpret_cast<char *>(garbage), sizeof(garbage));

  Vault_parser_composer vault_parser(logger);
  Vault_key key("key1", nullptr, "rob", nullptr, 0);
  EXPECT_CALL(
      *(reinterpret_cast<Mock_logger *>(logger)),
      log(MY_ERROR_LEVEL, StrEq("Could not parse Vault Server response.")));
  EXPECT_TRUE(vault_parser.parse_key_data(payload, &key, Vault_version_v1));
}

}  // namespace keyring__vault_parser_unittest
