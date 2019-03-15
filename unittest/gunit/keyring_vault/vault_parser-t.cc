#include <gtest/gtest.h>
#include <memory>
#include "i_keys_container.h"
#include "mock_logger.h"
#include "my_rnd.h"
#include "vault_base64.h"
#include "vault_key.h"
#include "vault_parser.h"

namespace keyring__vault_parser_unittest {
using namespace keyring;

using ::testing::StrEq;
typedef keyring::IVault_parser::KeyParameters KeyParameters;

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
  Vault_parser vault_parser(logger);
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
  Vault_parser vault_parser(logger);
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
  Vault_parser vault_parser(logger);
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
  Vault_parser vault_parser(logger);
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

  Vault_parser vault_parser(logger);
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
  Vault_parser vault_parser(logger);
  EXPECT_FALSE(vault_parser.parse_keys(payload, &keys));
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
  Vault_parser vault_parser(logger);
  EXPECT_FALSE(vault_parser.parse_keys(payload, &keys));
  EXPECT_EQ(keys.size(), static_cast<uint>(0));
}

TEST_F(Vault_parser_test, ParseVaultPayloadNoKeyList) {
  Secure_string payload(
      "{\"request_id\":\"724a5ad6-7ee3-7950-879a-488a261a03ec\","
      "\"lease_id\":\"\",\"renewable\":false,\"lease_duration\":0,"
      "\"data\":{},\"wrap_info"
      "\":null,\"warnings\":null,\"auth\":null}");
  Vault_keys_list keys;
  Vault_parser vault_parser(logger);
  EXPECT_CALL(
      *(reinterpret_cast<Mock_logger *>(logger)),
      log(MY_ERROR_LEVEL,
          StrEq("Could not parse keys tag with keys list from Vault.")));
  EXPECT_TRUE(vault_parser.parse_keys(payload, &keys));
  EXPECT_EQ(keys.size(), static_cast<uint>(0));
}

TEST_F(Vault_parser_test, ParseKeyData) {
  // Robi - encoded base64 = Um9iaQ==

  Secure_string payload(
      "{\"request_id\":\"77626d44-edbd-c82f-8220-c3c6b13ef2e1\","
      "\"lease_id\":\"\",\"renewable\":false,\"lease_duration\""
      ":2764800,\"data\":{\"type\":\"AES\",\"value\":\"Um9iaQ==\"},"
      "\"wrap_info\":null,\"warnings\":null,\"auth\":null}");

  Vault_parser vault_parser(logger);
  Vault_key key("key1", nullptr, "rob", nullptr, 0);
  EXPECT_FALSE(vault_parser.parse_key_data(payload, &key));
  EXPECT_STREQ(key.get_key_signature()->c_str(), "4_key13_rob");
  ASSERT_TRUE(memcmp(key.get_key_data(), "Robi", key.get_key_data_size()) == 0);
  EXPECT_STREQ("AES", key.get_key_type()->c_str());
}

TEST_F(Vault_parser_test, ParseKeyDataMissingTypeTag) {
  Secure_string payload(
      "{\"request_id\":\"77626d44-edbd-c82f-8220-c3c6b13ef2e1\","
      "\"lease_id\":\"\",\"renewable\":false,\"lease_duration\""
      ":2764800,\"data\":{\"value\":\"Um9iaQ==\"},"
      "\"wrap_info\":null,\"warnings\":null,\"auth\":null}");

  Vault_parser vault_parser(logger);
  Vault_key key("key1", nullptr, "rob", nullptr, 0);
  EXPECT_CALL(
      *(reinterpret_cast<Mock_logger *>(logger)),
      log(MY_ERROR_LEVEL, StrEq("Could not parse type tag for a key.")));
  EXPECT_TRUE(vault_parser.parse_key_data(payload, &key));
}

TEST_F(Vault_parser_test, ParseKeyDataMissingValueTag) {
  Secure_string payload(
      "{\"request_id\":\"77626d44-edbd-c82f-8220-c3c6b13ef2e1\","
      "\"lease_id\":\"\",\"renewable\":false,\"lease_duration\""
      ":2764800,\"data\":{\"type\":\"AES\"},"
      "\"wrap_info\":null,\"warnings\":null,\"auth\":null}");

  Vault_parser vault_parser(logger);
  Vault_key key("key1", nullptr, "rob", nullptr, 0);
  EXPECT_CALL(
      *(reinterpret_cast<Mock_logger *>(logger)),
      log(MY_ERROR_LEVEL, StrEq("Could not parse value tag for a key.")));
  EXPECT_TRUE(vault_parser.parse_key_data(payload, &key));
}

TEST_F(Vault_parser_test, ParsePayloadThatsGarbage) {
  uchar garbage[101000];
  my_rand_buffer(garbage, sizeof(garbage));
  Secure_string payload(reinterpret_cast<char *>(garbage), sizeof(garbage));

  Vault_parser vault_parser(logger);
  Vault_key key("key1", nullptr, "rob", nullptr, 0);
  EXPECT_CALL(
      *(reinterpret_cast<Mock_logger *>(logger)),
      log(MY_ERROR_LEVEL, StrEq("Could not parse type tag for a key.")));
  EXPECT_TRUE(vault_parser.parse_key_data(payload, &key));
}

}  // namespace keyring__vault_parser_unittest

#ifndef MERGE_UNITTESTS
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
#endif  // MERGE_UNITTESTS
