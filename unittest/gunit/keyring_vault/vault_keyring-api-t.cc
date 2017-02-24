#include <my_global.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sql_plugin_ref.h>
#include <fstream>
#include "vault_keyring.cc"
#include "keyring_impl.cc"
#include "mock_logger.h"

namespace keyring__api_unittest
{
  using ::testing::StrEq;
  using namespace keyring;

  class Keyring_vault_api_test : public ::testing::Test
  {
  public:
    Keyring_vault_api_test()
    {}
    ~Keyring_vault_api_test()
    {
      delete[] plugin_name;
      delete[] keyring_filename;
    }

    static std::string correct_token;

  protected:
    virtual void SetUp()
    {
      plugin_name= new char[strlen("FakeKeyring")+1];
      strcpy(plugin_name, "FakeKeyring");
      keyring_filename= new char[strlen("./keyring_vault.conf")+1];
      strcpy(keyring_filename, "./keyring_vault.conf");

      plugin_info.name.str= plugin_name;
      plugin_info.name.length= strlen(plugin_name);
      keyring_vault_config_file= keyring_filename;

      keyring_init_with_mock_logger();

      key_memory_KEYRING= PSI_NOT_INSTRUMENTED;
      key_LOCK_keyring= PSI_NOT_INSTRUMENTED;
      sample_key_data= "Robi";
    }
    virtual void TearDown()
    {
      keyring_deinit_with_mock_logger();
    }
  protected:
    void keyring_init_with_mock_logger();
    void keyring_deinit_with_mock_logger();

    std::string sample_key_data;
    char *plugin_name;
    char *keyring_filename;
    st_plugin_int plugin_info; //for Logger initialization
  };

  std::string Keyring_vault_api_test::correct_token;

  void Keyring_vault_api_test::keyring_init_with_mock_logger()
  {
    ASSERT_TRUE(keyring_vault_init(&plugin_info) == FALSE);
    //use MockLogger instead of Logger
    logger.reset(new Mock_logger());
  }

  void Keyring_vault_api_test::keyring_deinit_with_mock_logger()
  {
    keyring_vault_deinit(NULL);
  }

  TEST_F(Keyring_vault_api_test, StoreFetchRemove)
  {
    EXPECT_EQ(mysql_key_store("Robert_key", "AES", "Robert", sample_key_data.c_str(),
                              sample_key_data.length()), FALSE);
    char *key_type;
    size_t key_len;
    void *key;
    EXPECT_EQ(mysql_key_fetch("Robert_key", &key_type, "Robert", &key,
                              &key_len), FALSE);
    EXPECT_STREQ("AES", key_type);
    EXPECT_EQ(key_len, sample_key_data.length());
    ASSERT_TRUE(memcmp((char *)key, sample_key_data.c_str(), key_len) == 0);
    my_free(key_type);
    key_type= NULL;
    my_free(key);
    key= NULL;
    EXPECT_EQ(mysql_key_remove("Robert_key", "Robert"), FALSE);
    //make sure the key was removed - fetch it
    EXPECT_EQ(mysql_key_fetch("Robert_key", &key_type, "Robert", &key,
                              &key_len), FALSE);
    ASSERT_TRUE(key == NULL);
  }

  TEST_F(Keyring_vault_api_test, CheckIfInmemoryKeyIsNOTXORed)
  {
    EXPECT_EQ(mysql_key_store("Robert_key", "AES", "Robert", sample_key_data.c_str(),
                              sample_key_data.length()), 0);

    Vault_key key_id("Robert_key", NULL, "Robert",NULL,0);
    IKey* fetched_key= keys->fetch_key(&key_id);
    ASSERT_TRUE(fetched_key != NULL);
    std::string expected_key_signature= "10_Robert_key6_Robert";
    EXPECT_STREQ(fetched_key->get_key_signature()->c_str(), expected_key_signature.c_str());
    EXPECT_EQ(fetched_key->get_key_signature()->length(), expected_key_signature.length());
    uchar* key_data_fetched= fetched_key->get_key_data();
    size_t key_data_fetched_size= fetched_key->get_key_data_size();
    EXPECT_STREQ("AES", fetched_key->get_key_type()->c_str());

    ASSERT_TRUE(memcmp(sample_key_data.c_str(), key_data_fetched, key_data_fetched_size) == 0);
    ASSERT_TRUE(sample_key_data.length() == key_data_fetched_size);
    my_free(fetched_key->release_key_data());

    //clean up
    EXPECT_EQ(mysql_key_remove("Robert_key", "Robert"), FALSE);
  }

  TEST_F(Keyring_vault_api_test, FetchNotExisting)
  {
    char *key_type= NULL;
    void *key= NULL;
    size_t key_len= 0;
    EXPECT_EQ(mysql_key_fetch("Robert_key", &key_type, "Robert", &key,
                              &key_len), FALSE);
    ASSERT_TRUE(key == NULL);
  }

  TEST_F(Keyring_vault_api_test, RemoveNotExisting)
  {
    EXPECT_EQ(mysql_key_remove("Robert_key", "Robert"), TRUE);
  }

  TEST_F(Keyring_vault_api_test, StoreFetchNotExisting)
  {
    EXPECT_EQ(mysql_key_store("Robert_key", "AES", "Robert", sample_key_data.c_str(),
                              sample_key_data.length()), FALSE);
    char *key_type;
    size_t key_len;
    void *key;
    EXPECT_EQ(mysql_key_fetch("NotExisting", &key_type, "Robert", &key, &key_len), 0);
    ASSERT_TRUE(key == NULL);

    EXPECT_EQ(mysql_key_remove("Robert_key", "Robert"), FALSE);
  }

  TEST_F(Keyring_vault_api_test, StoreStoreStoreFetchRemove)
  {
    std::string key_data1("Robi1");
    std::string key_data2("Robi2");

    EXPECT_EQ(mysql_key_store("Robert_key", "AES", "Robert", sample_key_data.c_str(),
                              sample_key_data.length()), FALSE);
    EXPECT_EQ(mysql_key_store("Robert_key1", "AES", "Robert", key_data1.c_str(),
                              key_data1.length()), FALSE);
    EXPECT_EQ(mysql_key_store("Robert_key2", "AES", "Robert", key_data2.c_str(),
                              key_data2.length()), FALSE);
    char *key_type;
    size_t key_len;
    void *key;
    EXPECT_EQ(mysql_key_fetch("Robert_key1", &key_type, "Robert", &key,
                              &key_len), FALSE);
    EXPECT_STREQ("AES", key_type);
    EXPECT_EQ(key_len, key_data1.length());
    ASSERT_TRUE(memcmp((char *)key, key_data1.c_str(), key_len) == 0);
    my_free(key_type);
    key_type= NULL;
    my_free(key);
    key= NULL;
    EXPECT_EQ(mysql_key_remove("Robert_key2", "Robert"), FALSE);
    //make sure the key was removed - fetch it
    EXPECT_EQ(mysql_key_fetch("Robert_key2", &key_type, "Robert", &key,
                              &key_len), FALSE);
    ASSERT_TRUE(key == NULL);

    EXPECT_EQ(mysql_key_remove("Robert_key", "Robert"), FALSE);
    EXPECT_EQ(mysql_key_remove("Robert_key1", "Robert"), FALSE);
  }

  TEST_F(Keyring_vault_api_test, StoreValidTypes)
  {
    EXPECT_EQ(mysql_key_store("Robert_key", "AES", "Robert", sample_key_data.c_str(),
                              sample_key_data.length()), FALSE);
    EXPECT_EQ(mysql_key_store("Robert_key3", "RSA", "Robert", sample_key_data.c_str(),
                              sample_key_data.length()), FALSE);
    EXPECT_EQ(mysql_key_store("Robert_key4", "DSA", "Robert", sample_key_data.c_str(),
                              sample_key_data.length()), FALSE);
    //clean up
    EXPECT_EQ(mysql_key_remove("Robert_key", "Robert"), FALSE);
    EXPECT_EQ(mysql_key_remove("Robert_key3", "Robert"), FALSE);
    EXPECT_EQ(mysql_key_remove("Robert_key4", "Robert"), FALSE);

  }

  TEST_F(Keyring_vault_api_test, StoreInvalidType)
  {
    EXPECT_CALL(*((Mock_logger *)logger.get()), log(MY_ERROR_LEVEL, StrEq("Error while storing key: invalid key_type")));
    EXPECT_EQ(mysql_key_store("Robert_key", "YYY", "Robert", sample_key_data.c_str(),
                              sample_key_data.length()), TRUE);
    char *key_type;
    size_t key_len;
    void *key;
    EXPECT_EQ(mysql_key_fetch("Robert_key", &key_type, "Robert", &key,
                              &key_len), FALSE);
    ASSERT_TRUE(key == NULL);
  }

  TEST_F(Keyring_vault_api_test, StoreTwiceTheSameDifferentTypes)
  {
    EXPECT_EQ(mysql_key_store("Robert_key", "AES", "Robert", sample_key_data.c_str(),
                              sample_key_data.length()), FALSE);
    EXPECT_EQ(mysql_key_store("Robert_key", "RSA", "Robert", sample_key_data.c_str(),
                              sample_key_data.length()), TRUE);
    EXPECT_EQ(mysql_key_remove("Robert_key", "Robert"), FALSE);
  }

  TEST_F(Keyring_vault_api_test, KeyGenerate)
  {
    EXPECT_EQ(mysql_key_generate("Robert_key", "AES", "Robert", 128), FALSE);
    char *key_type;
    size_t key_len;
    void *key;
    EXPECT_EQ(mysql_key_fetch("Robert_key", &key_type, "Robert", &key,
                              &key_len), FALSE);
    EXPECT_STREQ("AES", key_type);
    EXPECT_EQ(key_len, (size_t)128);
    //Try accessing the last byte of key
    char ch= ((char*)key)[key_len-1];
    //Just to get rid of unused variable compiler error
    (void)ch;
    my_free(key);
    my_free(key_type);
    EXPECT_EQ(mysql_key_remove("Robert_key", "Robert"), FALSE);
  }

  TEST_F(Keyring_vault_api_test, NullUser)
  {
    EXPECT_EQ(mysql_key_store("Robert_key", "AES", NULL, sample_key_data.c_str(),
                              sample_key_data.length() + 1), 0);
    char *key_type;
    size_t key_len;
    void *key;
    EXPECT_EQ(mysql_key_fetch("Robert_key", &key_type, NULL, &key,
                              &key_len), 0);
    EXPECT_STREQ("AES", key_type);
    EXPECT_EQ(key_len, sample_key_data.length()+1);
    ASSERT_TRUE(memcmp((char *)key, sample_key_data.c_str(), key_len) == 0);
    my_free(key_type);
    key_type= NULL;
    my_free(key);
    key= NULL;
    EXPECT_EQ(mysql_key_store("Robert_key", "RSA", NULL, sample_key_data.c_str(),
                              sample_key_data.length() + 1), 1);
    EXPECT_EQ(mysql_key_store("Kamil_key", "AES", NULL, sample_key_data.c_str(),
                              sample_key_data.length() + 1), 0);
    EXPECT_EQ(mysql_key_fetch("Kamil_key", &key_type, NULL, &key,
                              &key_len), 0);
    EXPECT_STREQ("AES", key_type);
    EXPECT_EQ(key_len, sample_key_data.length()+1);
    ASSERT_TRUE(memcmp((char *)key, sample_key_data.c_str(), key_len) == 0);
    my_free(key_type);
    key_type= NULL;
    my_free(key);
    key= NULL;
    EXPECT_EQ(mysql_key_store("Artur_key", "AES", "Artur", sample_key_data.c_str(),
                              sample_key_data.length() + 1), 0);
    EXPECT_EQ(mysql_key_fetch("Artur_key", &key_type, "Artur", &key,
                              &key_len), 0);
    EXPECT_STREQ("AES", key_type);
    EXPECT_EQ(key_len, sample_key_data.length()+1);
    ASSERT_TRUE(memcmp((char *)key, sample_key_data.c_str(), key_len) == 0);
    my_free(key_type);
    key_type= NULL;
    my_free(key);
    key= NULL;
    EXPECT_EQ(mysql_key_remove("Robert_key", NULL) , 0);
    EXPECT_EQ(mysql_key_fetch("Robert_key", &key_type, "Robert", &key,
                              &key_len), 0);
    ASSERT_TRUE(key == NULL);
    EXPECT_EQ(mysql_key_fetch("Artur_key", &key_type, "Artur", &key,
                              &key_len), 0);
    EXPECT_STREQ("AES", key_type);
    EXPECT_EQ(key_len, sample_key_data.length()+1);
    ASSERT_TRUE(memcmp((char *)key, sample_key_data.c_str(), key_len) == 0);
    my_free(key_type);
    key_type= NULL;
    my_free(key);
    key= NULL;
    
    EXPECT_EQ(mysql_key_remove("Kamil_key", NULL) , 0);
    EXPECT_EQ(mysql_key_remove("Artur_key", "Artur") , 0);
  }

  TEST_F(Keyring_vault_api_test, NullKeyId)
  {
    EXPECT_CALL(*((Mock_logger *)logger.get()), log(MY_ERROR_LEVEL, StrEq("Error while storing key: key_id cannot be empty")));
    EXPECT_EQ(mysql_key_store(NULL, "AES", "Robert", sample_key_data.c_str(),
                              sample_key_data.length() + 1), 1);
    EXPECT_CALL(*((Mock_logger *)logger.get()), log(MY_ERROR_LEVEL, StrEq("Error while storing key: key_id cannot be empty")));
    EXPECT_EQ(mysql_key_store(NULL, "AES", NULL, sample_key_data.c_str(),
                              sample_key_data.length() + 1), 1);
    EXPECT_CALL(*((Mock_logger *)logger.get()), log(MY_ERROR_LEVEL, StrEq("Error while storing key: key_id cannot be empty")));
    EXPECT_EQ(mysql_key_store("", "AES", "Robert", sample_key_data.c_str(),
                              sample_key_data.length() + 1), 1);
    EXPECT_CALL(*((Mock_logger *)logger.get()), log(MY_ERROR_LEVEL, StrEq("Error while storing key: key_id cannot be empty")));
    EXPECT_EQ(mysql_key_store("", "AES", NULL, sample_key_data.c_str(),
                              sample_key_data.length() + 1), 1);
    char *key_type;
    size_t key_len;
    void *key;
    EXPECT_CALL(*((Mock_logger *)logger.get()), log(MY_ERROR_LEVEL, StrEq("Error while fetching key: key_id cannot be empty")));
    EXPECT_EQ(mysql_key_fetch(NULL, &key_type, "Robert", &key, &key_len), 1);
    EXPECT_CALL(*((Mock_logger *)logger.get()), log(MY_ERROR_LEVEL, StrEq("Error while fetching key: key_id cannot be empty")));
    EXPECT_EQ(mysql_key_fetch(NULL, &key_type, NULL, &key, &key_len), 1);
    EXPECT_CALL(*((Mock_logger *)logger.get()), log(MY_ERROR_LEVEL, StrEq("Error while fetching key: key_id cannot be empty")));
    EXPECT_EQ(mysql_key_fetch("", &key_type, "Robert", &key, &key_len), 1);
    EXPECT_CALL(*((Mock_logger *)logger.get()), log(MY_ERROR_LEVEL, StrEq("Error while fetching key: key_id cannot be empty")));
    EXPECT_EQ(mysql_key_fetch("", &key_type, NULL, &key, &key_len), 1);

    EXPECT_CALL(*((Mock_logger *)logger.get()), log(MY_ERROR_LEVEL, StrEq("Error while removing key: key_id cannot be empty")));
    EXPECT_EQ(mysql_key_remove(NULL, "Robert") , 1);
    EXPECT_CALL(*((Mock_logger *)logger.get()), log(MY_ERROR_LEVEL, StrEq("Error while removing key: key_id cannot be empty")));
    EXPECT_EQ(mysql_key_remove(NULL, NULL) , 1);
    EXPECT_CALL(*((Mock_logger *)logger.get()), log(MY_ERROR_LEVEL, StrEq("Error while removing key: key_id cannot be empty")));
    EXPECT_EQ(mysql_key_remove("", "Robert") , 1);
    EXPECT_CALL(*((Mock_logger *)logger.get()), log(MY_ERROR_LEVEL, StrEq("Error while removing key: key_id cannot be empty")));
    EXPECT_EQ(mysql_key_remove("", NULL) , 1);

    EXPECT_CALL(*((Mock_logger *)logger.get()), log(MY_ERROR_LEVEL, StrEq("Error while generating key: key_id cannot be empty")));
    EXPECT_EQ(mysql_key_generate(NULL, "AES", "Robert", 128), 1);
    EXPECT_CALL(*((Mock_logger *)logger.get()), log(MY_ERROR_LEVEL, StrEq("Error while generating key: key_id cannot be empty")));
    EXPECT_EQ(mysql_key_generate(NULL, "AES", NULL, 128), 1);
    EXPECT_CALL(*((Mock_logger *)logger.get()), log(MY_ERROR_LEVEL, StrEq("Error while generating key: key_id cannot be empty")));
    EXPECT_EQ(mysql_key_generate("", "AES", "Robert", 128), 1);
    EXPECT_CALL(*((Mock_logger *)logger.get()), log(MY_ERROR_LEVEL, StrEq("Error while generating key: key_id cannot be empty")));
    EXPECT_EQ(mysql_key_generate("", "AES", NULL, 128), 1);
  }
}
