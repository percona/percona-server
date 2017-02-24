#include <my_global.h>
#include <gtest/gtest.h>
#include "mock_logger.h"
#include "vault_parser.h"
#include "vault_key.h"

#if defined(HAVE_PSI_INTERFACE)
namespace keyring
{
  PSI_memory_key key_memory_KEYRING = PSI_NOT_INSTRUMENTED;
//  PSI_memory_key key_LOCK_keyring = PSI_NOT_INSTRUMENTED;
}
#endif

namespace keyring__vault_parser_unittest
{
  using namespace keyring;

  using ::testing::StrEq;

  class Vault_parser_test : public ::testing::Test
  {
  protected:
    virtual void SetUp()
    {
      logger= new Mock_logger();
    }

    virtual void TearDown()
    {
      delete logger;
    }

  protected:
    ILogger *logger;
  };

  TEST_F(Vault_parser_test, ParseKeySignature)
  {
    std::string key_signature("4_key13_rob");
    Vault_parser vault_parser(logger);
    std::string key_parameters[2];
    EXPECT_EQ(vault_parser.parse_key_signature(&key_signature, key_parameters), FALSE);
    EXPECT_STREQ(key_parameters[0].c_str(), "key1");
    EXPECT_STREQ(key_parameters[1].c_str(), "rob");
  }

  TEST_F(Vault_parser_test, ParseKeySignature2)
  {
    std::string key_signature("4_key16_Robert");
    Vault_parser vault_parser(logger);
    std::string key_parameters[2];
    EXPECT_EQ(vault_parser.parse_key_signature(&key_signature, key_parameters), FALSE);
    EXPECT_STREQ(key_parameters[0].c_str(), "key1");
    EXPECT_STREQ(key_parameters[1].c_str(), "Robert");
  }

  TEST_F(Vault_parser_test, ParseKeySignature3)
  {
    std::string key_signature("7__key1238_Robert33");
    Vault_parser vault_parser(logger);
    std::string key_parameters[2];
    EXPECT_EQ(vault_parser.parse_key_signature(&key_signature, key_parameters), FALSE);
    EXPECT_STREQ(key_parameters[0].c_str(), "_key123");
    EXPECT_STREQ(key_parameters[1].c_str(), "Robert33");
  }

  TEST_F(Vault_parser_test, ParseKeySignature4)
  {
    std::string key_signature("9_123key12310_12Robert33");
    Vault_parser vault_parser(logger);
    std::string key_parameters[2];
    EXPECT_EQ(vault_parser.parse_key_signature(&key_signature, key_parameters), FALSE);
    EXPECT_STREQ(key_parameters[0].c_str(), "123key123");
    EXPECT_STREQ(key_parameters[1].c_str(), "12Robert33");
  }

  TEST_F(Vault_parser_test, ParseKeySignature5)
  {
    std::string key_signature("48_INNODBKey-3c40d1ab-1475-11e7-ae1c-9cb6d0d5dc99-10_");
    Vault_parser vault_parser(logger);
    std::string key_parameters[2];
    EXPECT_EQ(vault_parser.parse_key_signature(&key_signature, key_parameters), FALSE);
    EXPECT_STREQ(key_parameters[0].c_str(), "INNODBKey-3c40d1ab-1475-11e7-ae1c-9cb6d0d5dc99-1");
    EXPECT_STREQ(key_parameters[1].c_str(), "");
  }

  TEST_F(Vault_parser_test, ParseVaultPayload)
  {
    std::string payload("{\"request_id\":\"724a5ad6-7ee3-7950-879a-488a261a03ec\","
                        "\"lease_id\":\"\",\"renewable\":false,\"lease_duration\":0,"
                        "\"data\":{\"keys\":[\"4_key13_rob\",\"4_key23_rob\"]},\"wrap_info"
                        "\":null,\"warnings\":null,\"auth\":null}");
    Vault_keys_list keys;
    Vault_parser vault_parser(logger);
    EXPECT_EQ(vault_parser.parse_keys(&payload, &keys), FALSE);
    EXPECT_EQ(keys.size(), static_cast<uint>(2));
    EXPECT_EQ(keys.has_next_key(), TRUE);
    IKey *key_loaded= NULL;
    EXPECT_EQ(keys.get_next_key(&key_loaded), FALSE);
    EXPECT_STREQ(key_loaded->get_key_signature()->c_str(), "4_key13_rob");
    EXPECT_EQ(keys.has_next_key(), TRUE);
    delete key_loaded;
    key_loaded = NULL;
    EXPECT_EQ(keys.get_next_key(&key_loaded), FALSE);
    EXPECT_STREQ(key_loaded->get_key_signature()->c_str(), "4_key23_rob");
    EXPECT_EQ(keys.has_next_key(), FALSE);
    delete key_loaded;
    key_loaded = NULL;
  }

  TEST_F(Vault_parser_test, ParseVaultPayloadEmptyKeyList)
  {
    std::string payload("{\"request_id\":\"724a5ad6-7ee3-7950-879a-488a261a03ec\","
                        "\"lease_id\":\"\",\"renewable\":false,\"lease_duration\":0,"
                        "\"data\":{\"keys\":[]},\"wrap_info"
                        "\":null,\"warnings\":null,\"auth\":null}");
    Vault_keys_list keys;
    Vault_parser vault_parser(logger);
    EXPECT_EQ(vault_parser.parse_keys(&payload, &keys), FALSE);
    EXPECT_EQ(keys.size(), static_cast<uint>(0));
  }

  TEST_F(Vault_parser_test, ParseVaultPayloadNoKeyList)
  {
    std::string payload("{\"request_id\":\"724a5ad6-7ee3-7950-879a-488a261a03ec\","
                        "\"lease_id\":\"\",\"renewable\":false,\"lease_duration\":0,"
                        "\"data\":{},\"wrap_info"
                        "\":null,\"warnings\":null,\"auth\":null}");
    Vault_keys_list keys;
    Vault_parser vault_parser(logger);
    EXPECT_CALL(*((Mock_logger *)logger),
      log(MY_ERROR_LEVEL, StrEq("Could not parse keys tag with keys list from Vault.")));
    EXPECT_EQ(vault_parser.parse_keys(&payload, &keys), TRUE);
    EXPECT_EQ(keys.size(), static_cast<uint>(0));
  }

  TEST_F(Vault_parser_test, ParseKeyData)
  {
    //Robi - encoded base64 = Um9iaQ==  

    std::string payload("{\"request_id\":\"77626d44-edbd-c82f-8220-c3c6b13ef2e1\","
                        "\"lease_id\":\"\",\"renewable\":false,\"lease_duration\""
                        ":2764800,\"data\":{\"type\":\"AES\",\"value\":\"Um9iaQ==\"},"
                        "\"wrap_info\":null,\"warnings\":null,\"auth\":null}");

    Vault_parser vault_parser(logger);
    Vault_key key("key1", NULL, "rob", NULL, 0);
    EXPECT_EQ(vault_parser.parse_key_data(&payload, &key), FALSE);
    EXPECT_STREQ(key.get_key_signature()->c_str(), "4_key13_rob");
    ASSERT_TRUE(memcmp(key.get_key_data(), "Robi", key.get_key_data_size()) == 0);
    EXPECT_STREQ("AES", key.get_key_type()->c_str());
  }

  TEST_F(Vault_parser_test, ParseKeyDataMissingTypeTag)
  {
    std::string payload("{\"request_id\":\"77626d44-edbd-c82f-8220-c3c6b13ef2e1\","
                        "\"lease_id\":\"\",\"renewable\":false,\"lease_duration\""
                        ":2764800,\"data\":{\"value\":\"Um9iaQ==\"},"
                        "\"wrap_info\":null,\"warnings\":null,\"auth\":null}");

    Vault_parser vault_parser(logger);
    Vault_key key("key1", NULL, "rob", NULL, 0);
    EXPECT_CALL(*((Mock_logger *)logger),
      log(MY_ERROR_LEVEL, StrEq("Could not parse type tag for a key.")));
    EXPECT_EQ(vault_parser.parse_key_data(&payload, &key), TRUE);
  }

  TEST_F(Vault_parser_test, ParseKeyDataMissingValueTag)
  {
    std::string payload("{\"request_id\":\"77626d44-edbd-c82f-8220-c3c6b13ef2e1\","
                        "\"lease_id\":\"\",\"renewable\":false,\"lease_duration\""
                        ":2764800,\"data\":{\"type\":\"AES\"},"
                        "\"wrap_info\":null,\"warnings\":null,\"auth\":null}");

    Vault_parser vault_parser(logger);
    Vault_key key("key1", NULL, "rob", NULL, 0);
    EXPECT_CALL(*((Mock_logger *)logger),
      log(MY_ERROR_LEVEL, StrEq("Could not parse value tag for a key.")));
    EXPECT_EQ(vault_parser.parse_key_data(&payload, &key), TRUE);
  }


} //namespace keyring__file_io_unittest

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int ret= RUN_ALL_TESTS();
  return ret;
}
