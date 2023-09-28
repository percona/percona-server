/* Copyright (c) 2023 Percona LLC and/or its affiliates. All rights
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

#include "components/keyrings/common/data/data.h"
#include "components/keyrings/common/data/meta.h"
#include "components/keyrings/common/data/pfs_string.h"

#include "backend/vault_base64.h"
#include "backend/vault_keys_container.h"
#include "backend/vault_parser_composer.h"
#include "config/config.h"
#include "my_rnd.h"

namespace keyring_vault_parser_unittest {

using ::testing::StrEq;

using Data = keyring_common::data::Data;
using Metadata = keyring_common::meta::Metadata;
using Vault_base64 = keyring_vault::backend::Vault_base64;
using Keyring_vault_parser_composer =
    keyring_vault::backend::Keyring_vault_parser_composer;
using Vault_keys_container = keyring_vault::backend::Vault_keys_container;

class Vault_parser_test : public ::testing::Test {};

TEST_F(Vault_parser_test, ParseKeySignature) {
  std::string key_signature("4_key13_rob");
  pfs_string encoded_key_signature;
  EXPECT_FALSE(Vault_base64::encode(
      key_signature.c_str(), key_signature.length(), &encoded_key_signature,
      Vault_base64::Format::SINGLE_LINE));
  std::unique_ptr<Metadata> key_parameters;
  EXPECT_FALSE(Keyring_vault_parser_composer::parse_key_signature(
      encoded_key_signature, key_parameters));
  EXPECT_STREQ(key_parameters->key_id().c_str(), "key1");
  EXPECT_STREQ(key_parameters->owner_id().c_str(), "rob");
}

TEST_F(Vault_parser_test, ParseKeySignature2) {
  std::string key_signature("4_key16_Robert");
  pfs_string encoded_key_signature;
  EXPECT_FALSE(Vault_base64::encode(
      key_signature.c_str(), key_signature.length(), &encoded_key_signature,
      Vault_base64::Format::SINGLE_LINE));
  std::unique_ptr<Metadata> key_parameters;
  EXPECT_FALSE(Keyring_vault_parser_composer::parse_key_signature(
      encoded_key_signature, key_parameters));
  EXPECT_STREQ(key_parameters->key_id().c_str(), "key1");
  EXPECT_STREQ(key_parameters->owner_id().c_str(), "Robert");
}

TEST_F(Vault_parser_test, ParseKeySignature3) {
  std::string key_signature("7__key1238_Robert33");
  pfs_string encoded_key_signature;
  EXPECT_FALSE(Vault_base64::encode(
      key_signature.c_str(), key_signature.length(), &encoded_key_signature,
      Vault_base64::Format::SINGLE_LINE));
  std::unique_ptr<Metadata> key_parameters;
  EXPECT_FALSE(Keyring_vault_parser_composer::parse_key_signature(
      encoded_key_signature, key_parameters));
  EXPECT_STREQ(key_parameters->key_id().c_str(), "_key123");
  EXPECT_STREQ(key_parameters->owner_id().c_str(), "Robert33");
}

TEST_F(Vault_parser_test, ParseKeySignature4) {
  std::string key_signature("9_123key12310_12Robert33");
  pfs_string encoded_key_signature;
  EXPECT_FALSE(Vault_base64::encode(
      key_signature.c_str(), key_signature.length(), &encoded_key_signature,
      Vault_base64::Format::SINGLE_LINE));
  std::unique_ptr<Metadata> key_parameters;
  EXPECT_FALSE(Keyring_vault_parser_composer::parse_key_signature(
      encoded_key_signature, key_parameters));
  EXPECT_STREQ(key_parameters->key_id().c_str(), "123key123");
  EXPECT_STREQ(key_parameters->owner_id().c_str(), "12Robert33");
}

TEST_F(Vault_parser_test, ParseKeySignature5) {
  std::string key_signature(
      "48_INNODBKey-3c40d1ab-1475-11e7-ae1c-9cb6d0d5dc99-10_");
  pfs_string encoded_key_signature;
  EXPECT_FALSE(Vault_base64::encode(
      key_signature.c_str(), key_signature.length(), &encoded_key_signature,
      Vault_base64::Format::SINGLE_LINE));

  std::unique_ptr<Metadata> key_parameters;
  EXPECT_FALSE(Keyring_vault_parser_composer::parse_key_signature(
      encoded_key_signature, key_parameters));
  EXPECT_STREQ(key_parameters->key_id().c_str(),
               "INNODBKey-3c40d1ab-1475-11e7-ae1c-9cb6d0d5dc99-1");
  EXPECT_STREQ(key_parameters->owner_id().c_str(), "");
}

TEST_F(Vault_parser_test, ParseVaultPayload) {
  std::string key1_signature("4_key13_rob");
  std::string key2_signature("4_key23_rob");
  pfs_string encoded_key1_signature;
  EXPECT_FALSE(Vault_base64::encode(
      key1_signature.c_str(), key1_signature.length(), &encoded_key1_signature,
      Vault_base64::Format::SINGLE_LINE));
  pfs_string encoded_key2_signature;
  EXPECT_FALSE(Vault_base64::encode(
      key2_signature.c_str(), key2_signature.length(), &encoded_key2_signature,
      Vault_base64::Format::SINGLE_LINE));

  pfs_string payload(
      "{\"request_id\":\"724a5ad6-7ee3-7950-879a-488a261a03ec\","
      "\"lease_id\":\"\",\"renewable\":false,\"lease_duration\":0,"
      "\"data\":{\"keys\":[\"");
  payload += encoded_key1_signature;
  payload += "\",\"";
  payload += encoded_key2_signature;
  payload += "\"]},\"wrap_info\":null,\"warnings\":null,\"auth\":null}";
  Vault_keys_container keys;
  EXPECT_FALSE(Keyring_vault_parser_composer::parse_keys(payload, &keys));
  EXPECT_EQ(keys.size(), static_cast<uint>(2));
  auto iter = keys.begin();
  EXPECT_STREQ("key1", iter->get()->key_id().c_str());
  EXPECT_STREQ("rob", iter->get()->owner_id().c_str());
  iter++;
  EXPECT_STREQ("key2", iter->get()->key_id().c_str());
  EXPECT_STREQ("rob", iter->get()->owner_id().c_str());
}

TEST_F(Vault_parser_test, ParseVaultPayloadEmptyKeyList) {
  pfs_string payload(
      "{\"request_id\":\"724a5ad6-7ee3-7950-879a-488a261a03ec\","
      "\"lease_id\":\"\",\"renewable\":false,\"lease_duration\":0,"
      "\"data\":{\"keys\":[]},\"wrap_info"
      "\":null,\"warnings\":null,\"auth\":null}");
  Vault_keys_container keys;
  EXPECT_FALSE(Keyring_vault_parser_composer::parse_keys(payload, &keys));
  EXPECT_EQ(keys.size(), static_cast<uint>(0));
}

TEST_F(Vault_parser_test, ParseVaultPayloadNoKeyList) {
  pfs_string payload(
      "{\"request_id\":\"724a5ad6-7ee3-7950-879a-488a261a03ec\","
      "\"lease_id\":\"\",\"renewable\":false,\"lease_duration\":0,"
      "\"data\":{},\"wrap_info"
      "\":null,\"warnings\":null,\"auth\":null}");
  Vault_keys_container keys;
  EXPECT_TRUE(Keyring_vault_parser_composer::parse_keys(payload, &keys));
  EXPECT_EQ(keys.size(), static_cast<uint>(0));
}

TEST_F(Vault_parser_test, ParseKeyData) {
  // Robi - encoded base64 = Um9iaQ==

  pfs_string payload(
      "{\"request_id\":\"77626d44-edbd-c82f-8220-c3c6b13ef2e1\","
      "\"lease_id\":\"\",\"renewable\":false,\"lease_duration\""
      ":2764800,\"data\":{\"type\":\"AES\",\"value\":\"Um9iaQ==\"},"
      "\"wrap_info\":null,\"warnings\":null,\"auth\":null}");

  auto key = Data("key1", "rob");
  EXPECT_EQ(keyring_vault::backend::ParseStatus::Ok,
            Keyring_vault_parser_composer::parse_key_data(
                payload, &key, keyring_vault::config::Vault_version_v1));
  ASSERT_TRUE(memcmp(key.data().decode().c_str(), "Robi",
                     key.data().decode().size()) == 0);
  EXPECT_STREQ("AES", key.type().c_str());
}

TEST_F(Vault_parser_test, ParseKeyDataMissingTypeTag) {
  pfs_string payload(
      "{\"request_id\":\"77626d44-edbd-c82f-8220-c3c6b13ef2e1\","
      "\"lease_id\":\"\",\"renewable\":false,\"lease_duration\""
      ":2764800,\"data\":{\"value\":\"Um9iaQ==\"},"
      "\"wrap_info\":null,\"warnings\":null,\"auth\":null}");

  auto key_data = Data{"key1", "rob"};
  EXPECT_EQ(keyring_vault::backend::ParseStatus::Fail,
            Keyring_vault_parser_composer::parse_key_data(
                payload, &key_data, keyring_vault::config::Vault_version_v1));
}

TEST_F(Vault_parser_test, ParseKeyDataMissingValueTag) {
  pfs_string payload(
      "{\"request_id\":\"77626d44-edbd-c82f-8220-c3c6b13ef2e1\","
      "\"lease_id\":\"\",\"renewable\":false,\"lease_duration\""
      ":2764800,\"data\":{\"type\":\"AES\"},"
      "\"wrap_info\":null,\"warnings\":null,\"auth\":null}");

  auto key_data = Data{"key1", "rob"};
  EXPECT_EQ(keyring_vault::backend::ParseStatus::Fail,
            Keyring_vault_parser_composer::parse_key_data(
                payload, &key_data, keyring_vault::config::Vault_version_v1));
}

TEST_F(Vault_parser_test, ParseKeyDataDeleted) {
  pfs_string payload(
      "{\"request_id\":\"4559df83-fc5f-9959-314a-443c85eef27c\","
      "\"lease_id\":\"\",\"renewable\":false,\"lease_duration\":0,"
      "\"data\":{\"data\":null,\"metadata\":{"
      "\"created_time\":\"2023-09-27T14:44:07.703848947Z\","
      "\"custom_metadata\":null,"
      "\"deletion_time\":\"2023-09-27T14:44:07.705030251Z\","
      "\"destroyed\":false,\"version\":1}},"
      "\"wrap_info\":null,\"warnings\":null,\"auth\":null}");

  Data key{};
  EXPECT_EQ(keyring_vault::backend::ParseStatus::DataDeleted,
            Keyring_vault_parser_composer::parse_key_data(
                payload, &key, keyring_vault::config::Vault_version_v2));
}

TEST_F(Vault_parser_test, GetMountConfig) {
  std::size_t max_versions = 0;
  bool cas_required = false;
  pfs_optional_string delete_version_after;

  pfs_string payload(
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

  EXPECT_FALSE(Keyring_vault_parser_composer::parse_mount_point_config(
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
  EXPECT_FALSE(Keyring_vault_parser_composer::parse_mount_point_config(
      payload, max_versions, cas_required, delete_version_after));
  EXPECT_EQ(max_versions, 43U);
  EXPECT_FALSE(cas_required);
  EXPECT_TRUE(delete_version_after == std::nullopt);
}

TEST_F(Vault_parser_test, GetMountConfigNull) {
  pfs_string payload(
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

  std::size_t max_versions = 0;
  bool cas_required = false;
  pfs_optional_string delete_version_after;
  EXPECT_TRUE(Keyring_vault_parser_composer::parse_mount_point_config(
      payload, max_versions, cas_required, delete_version_after));
}

TEST_F(Vault_parser_test, GetMountConfigIncomplete) {
  pfs_string payload(
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

  std::size_t max_versions = 0;
  bool cas_required = false;
  pfs_optional_string delete_version_after;
  EXPECT_TRUE(Keyring_vault_parser_composer::parse_mount_point_config(
      payload, max_versions, cas_required, delete_version_after));
}

TEST_F(Vault_parser_test, ParsePayloadThatsGarbage) {
  uchar garbage[101000];
  my_rand_buffer(garbage, sizeof(garbage));
  pfs_string payload(reinterpret_cast<char *>(garbage), sizeof(garbage));

  auto key_data = Data{"key1", "rob"};
  EXPECT_EQ(keyring_vault::backend::ParseStatus::Fail,
            Keyring_vault_parser_composer::parse_key_data(
                payload, &key_data, keyring_vault::config::Vault_version_v1));
}

}  // namespace keyring_vault_parser_unittest
