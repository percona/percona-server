/* Copyright (c) 2018, 2021 Percona LLC and/or its affiliates. All rights reserved.

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

#include <fstream>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <my_global.h>
#include "sql_plugin_ref.h"

#include <boost/move/unique_ptr.hpp>
#include <boost/preprocessor/stringize.hpp>

#include "generate_credential_file.h"
#include "mock_logger.h"
#include "vault_environment.h"
#include "vault_mount.h"
#include "vault_test_base.h"

#include "keyring_impl.cc"
#include "vault_keyring.cc"

namespace keyring__api_unittest {
using ::testing::StrEq;
using namespace keyring;

class Keyring_vault_api_test : public Vault_test_base {
 public:
  virtual ~Keyring_vault_api_test() {}

 protected:
  virtual void SetUp()
  {
    Vault_test_base::SetUp();

    const std::string &conf_name=
        Vault_environment::get_instance()->get_default_conf_file_name();
    keyring_vault_config_file_storage_.assign(
        conf_name.c_str(), conf_name.c_str() + conf_name.size() + 1);

    keyring_vault_config_file= keyring_vault_config_file_storage_.data();

    st_plugin_int plugin_info;  // for Logger initialization
    plugin_info.name.str= plugin_name;
    plugin_info.name.length= strlen(plugin_name);
    ASSERT_FALSE(keyring_vault_init(&plugin_info));
    // use MockLogger instead of Logger
    logger.reset(new Mock_logger);

    key_memory_KEYRING= PSI_NOT_INSTRUMENTED;
    key_LOCK_keyring= PSI_NOT_INSTRUMENTED;
  }
  virtual void TearDown()
  {
    keyring_vault_deinit(NULL);

    Vault_test_base::TearDown();
  }

 protected:
  static char        plugin_name[];
  static std::string sample_key_data;

  typedef std::vector<char> char_container;
  char_container            keyring_vault_config_file_storage_;
};
/*static*/ char        Keyring_vault_api_test::plugin_name[]= "FakeKeyring";
/*static*/ std::string Keyring_vault_api_test::sample_key_data= "Robi";


TEST_F(Keyring_vault_api_test, StoreFetchRemove)
{
  EXPECT_FALSE(mysql_key_store(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key").c_str(),
      "AES", "Robert", sample_key_data.c_str(), sample_key_data.length()));
  char * key_type;
  size_t key_len;
  void * key;
  EXPECT_FALSE(mysql_key_fetch(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key").c_str(),
      &key_type, "Robert", &key, &key_len));
  EXPECT_STREQ("AES", key_type);
  EXPECT_EQ(key_len, sample_key_data.length());
  ASSERT_TRUE(memcmp((char *)key, sample_key_data.c_str(), key_len) == 0);
  my_free(key_type);
  key_type= NULL;
  my_free(key);
  key= NULL;
  EXPECT_FALSE(mysql_key_remove(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key").c_str(),
      "Robert"));
  // make sure the key was removed - fetch it
  EXPECT_FALSE(mysql_key_fetch(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key").c_str(),
      &key_type, "Robert", &key, &key_len));
  ASSERT_TRUE(key == NULL);
}

TEST_F(Keyring_vault_api_test, CheckIfInmemoryKeyIsNOTXORed)
{
  EXPECT_FALSE(mysql_key_store(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key").c_str(),
      "AES", "Robert", sample_key_data.c_str(), sample_key_data.length()));

  Vault_key key_id(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key").c_str(),
      NULL, "Robert", NULL, 0);
  IKey *fetched_key= keys->fetch_key(&key_id);
  ASSERT_TRUE(fetched_key != NULL);
  std::string expected_key_signature=
      Vault_environment::get_instance()->get_key_signature("Robert_key",
                                                           "Robert");
  EXPECT_STREQ(fetched_key->get_key_signature()->c_str(),
               expected_key_signature.c_str());
  EXPECT_EQ(fetched_key->get_key_signature()->length(),
            expected_key_signature.length());
  uchar *key_data_fetched= fetched_key->get_key_data();
  size_t key_data_fetched_size= fetched_key->get_key_data_size();
  EXPECT_STREQ("AES", fetched_key->get_key_type()->c_str());
  ASSERT_TRUE(memcmp(sample_key_data.c_str(), key_data_fetched,
                     key_data_fetched_size) == 0);
  ASSERT_TRUE(sample_key_data.length() == key_data_fetched_size);
  my_free(fetched_key->release_key_data());

  // clean up
  EXPECT_FALSE(mysql_key_remove(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key").c_str(),
      "Robert"));
}

TEST_F(Keyring_vault_api_test, FetchNotExisting)
{
  char * key_type= NULL;
  void * key= NULL;
  size_t key_len= 0;
  EXPECT_FALSE(mysql_key_fetch(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key").c_str(),
      &key_type, "Robert", &key, &key_len));
  ASSERT_TRUE(key == NULL);
}

TEST_F(Keyring_vault_api_test, RemoveNotExisting)
{
  EXPECT_TRUE(mysql_key_remove(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key").c_str(),
      "Robert"));
}

TEST_F(Keyring_vault_api_test, StoreFetchNotExisting)
{
  EXPECT_FALSE(mysql_key_store(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key").c_str(),
      "AES", "Robert", sample_key_data.c_str(), sample_key_data.length()));
  char * key_type;
  size_t key_len;
  void * key;
  EXPECT_FALSE(
      mysql_key_fetch("NotExisting", &key_type, "Robert", &key, &key_len));
  ASSERT_TRUE(key == NULL);

  EXPECT_FALSE(mysql_key_remove(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key").c_str(),
      "Robert"));
}

TEST_F(Keyring_vault_api_test, StoreStoreStoreFetchRemove)
{
  std::string key_data1("Robi1");
  std::string key_data2("Robi2");

  EXPECT_FALSE(mysql_key_store(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key").c_str(),
      "AES", "Robert", sample_key_data.c_str(), sample_key_data.length()));
  EXPECT_FALSE(mysql_key_store(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key1").c_str(),
      "AES", "Robert", key_data1.c_str(), key_data1.length()));
  EXPECT_FALSE(mysql_key_store(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key2").c_str(),
      "AES", "Robert", key_data2.c_str(), key_data2.length()));
  char * key_type;
  size_t key_len;
  void * key;
  EXPECT_FALSE(mysql_key_fetch(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key1").c_str(),
      &key_type, "Robert", &key, &key_len));
  EXPECT_STREQ("AES", key_type);
  EXPECT_EQ(key_len, key_data1.length());
  ASSERT_TRUE(memcmp((char *)key, key_data1.c_str(), key_len) == 0);
  my_free(key_type);
  key_type= NULL;
  my_free(key);
  key= NULL;
  EXPECT_FALSE(mysql_key_remove(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key2").c_str(),
      "Robert"));
  // make sure the key was removed - fetch it
  EXPECT_FALSE(mysql_key_fetch(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key2").c_str(),
      &key_type, "Robert", &key, &key_len));
  ASSERT_TRUE(key == NULL);

  EXPECT_FALSE(mysql_key_remove(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key").c_str(),
      "Robert"));
  EXPECT_FALSE(mysql_key_remove(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key1").c_str(),
      "Robert"));
}

TEST_F(Keyring_vault_api_test, StoreValidTypes)
{
  EXPECT_FALSE(mysql_key_store(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key").c_str(),
      "AES", "Robert", sample_key_data.c_str(), sample_key_data.length()));
  EXPECT_FALSE(mysql_key_store(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key3").c_str(),
      "RSA", "Robert", sample_key_data.c_str(), sample_key_data.length()));
  EXPECT_FALSE(mysql_key_store(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key4").c_str(),
      "DSA", "Robert", sample_key_data.c_str(), sample_key_data.length()));
  // clean up
  EXPECT_FALSE(mysql_key_remove(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key").c_str(),
      "Robert"));
  EXPECT_FALSE(mysql_key_remove(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key3").c_str(),
      "Robert"));
  EXPECT_FALSE(mysql_key_remove(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key4").c_str(),
      "Robert"));
}

TEST_F(Keyring_vault_api_test, StoreInvalidType)
{
  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(logger.get())),
              log(MY_ERROR_LEVEL,
                  StrEq("Error while storing key: invalid key_type")));
  EXPECT_TRUE(mysql_key_store(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key").c_str(),
      "YYY", "Robert", sample_key_data.c_str(), sample_key_data.length()));
  char * key_type;
  size_t key_len;
  void * key;
  EXPECT_FALSE(mysql_key_fetch(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key").c_str(),
      &key_type, "Robert", &key, &key_len));
  ASSERT_TRUE(key == NULL);
}

TEST_F(Keyring_vault_api_test, StoreTwiceTheSameDifferentTypes)
{
  EXPECT_FALSE(mysql_key_store(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key").c_str(),
      "AES", "Robert", sample_key_data.c_str(), sample_key_data.length()));
  EXPECT_TRUE(mysql_key_store(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key").c_str(),
      "RSA", "Robert", sample_key_data.c_str(), sample_key_data.length()));
  EXPECT_FALSE(mysql_key_remove(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key").c_str(),
      "Robert"));
}

TEST_F(Keyring_vault_api_test, KeyGenerate)
{
  EXPECT_FALSE(mysql_key_generate(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key").c_str(),
      "AES", "Robert", 128));
  char * key_type;
  size_t key_len;
  void * key;
  EXPECT_FALSE(mysql_key_fetch(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key").c_str(),
      &key_type, "Robert", &key, &key_len));
  EXPECT_STREQ("AES", key_type);
  EXPECT_EQ(key_len, (size_t)128);
  // Try accessing the last byte of key
  volatile char ch= ((char *)key)[key_len - 1];
  // Just to get rid of unused variable compiler error
  (void)ch;
  my_free(key);
  my_free(key_type);
  EXPECT_FALSE(mysql_key_remove(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key").c_str(),
      "Robert"));
}

TEST_F(Keyring_vault_api_test, NullUser)
{
  EXPECT_FALSE(mysql_key_store(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key").c_str(),
      "AES", NULL, sample_key_data.c_str(), sample_key_data.length() + 1));
  char * key_type;
  size_t key_len;
  void * key;
  EXPECT_FALSE(mysql_key_fetch(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key").c_str(),
      &key_type, NULL, &key, &key_len));
  EXPECT_STREQ("AES", key_type);
  EXPECT_EQ(key_len, sample_key_data.length() + 1);
  ASSERT_TRUE(memcmp((char *)key, sample_key_data.c_str(), key_len) == 0);
  my_free(key_type);
  key_type= NULL;
  my_free(key);
  key= NULL;
  EXPECT_TRUE(mysql_key_store(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key").c_str(),
      "RSA", NULL, sample_key_data.c_str(), sample_key_data.length() + 1));
  EXPECT_FALSE(mysql_key_store(
      (Vault_environment::get_instance()->get_uuid() + "Kamil_key").c_str(),
      "AES", NULL, sample_key_data.c_str(), sample_key_data.length() + 1));
  EXPECT_FALSE(mysql_key_fetch(
      (Vault_environment::get_instance()->get_uuid() + "Kamil_key").c_str(),
      &key_type, NULL, &key, &key_len));
  EXPECT_STREQ("AES", key_type);
  EXPECT_EQ(key_len, sample_key_data.length() + 1);
  ASSERT_TRUE(memcmp((char *)key, sample_key_data.c_str(), key_len) == 0);
  my_free(key_type);
  key_type= NULL;
  my_free(key);
  key= NULL;
  EXPECT_FALSE(mysql_key_store(
      (Vault_environment::get_instance()->get_uuid() + "Artur_key").c_str(),
      "AES", "Artur", sample_key_data.c_str(), sample_key_data.length() + 1));
  EXPECT_FALSE(mysql_key_fetch(
      (Vault_environment::get_instance()->get_uuid() + "Artur_key").c_str(),
      &key_type, "Artur", &key, &key_len));
  EXPECT_STREQ("AES", key_type);
  EXPECT_EQ(key_len, sample_key_data.length() + 1);
  ASSERT_TRUE(memcmp((char *)key, sample_key_data.c_str(), key_len) == 0);
  my_free(key_type);
  key_type= NULL;
  my_free(key);
  key= NULL;
  EXPECT_FALSE(mysql_key_remove(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key").c_str(),
      NULL));
  EXPECT_FALSE(mysql_key_fetch(
      (Vault_environment::get_instance()->get_uuid() + "Robert_key").c_str(),
      &key_type, "Robert", &key, &key_len));
  ASSERT_TRUE(key == NULL);
  EXPECT_FALSE(mysql_key_fetch(
      (Vault_environment::get_instance()->get_uuid() + "Artur_key").c_str(),
      &key_type, "Artur", &key, &key_len));
  EXPECT_STREQ("AES", key_type);
  EXPECT_EQ(key_len, sample_key_data.length() + 1);
  ASSERT_TRUE(memcmp((char *)key, sample_key_data.c_str(), key_len) == 0);
  my_free(key_type);
  key_type= NULL;
  my_free(key);
  key= NULL;

  EXPECT_FALSE(mysql_key_remove(
      (Vault_environment::get_instance()->get_uuid() + "Kamil_key").c_str(),
      NULL));
  EXPECT_FALSE(mysql_key_remove(
      (Vault_environment::get_instance()->get_uuid() + "Artur_key").c_str(),
      "Artur"));
}

TEST_F(Keyring_vault_api_test, NullKeyId)
{
  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(logger.get())),
              log(MY_ERROR_LEVEL,
                  StrEq("Error while storing key: key_id cannot be empty")));
  EXPECT_TRUE(mysql_key_store(NULL, "AES", "Robert", sample_key_data.c_str(),
                              sample_key_data.length() + 1));
  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(logger.get())),
              log(MY_ERROR_LEVEL,
                  StrEq("Error while storing key: key_id cannot be empty")));
  EXPECT_TRUE(mysql_key_store(NULL, "AES", NULL, sample_key_data.c_str(),
                              sample_key_data.length() + 1));
  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(logger.get())),
              log(MY_ERROR_LEVEL,
                  StrEq("Error while storing key: key_id cannot be empty")));
  EXPECT_TRUE(mysql_key_store("", "AES", "Robert", sample_key_data.c_str(),
                              sample_key_data.length() + 1));
  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(logger.get())),
              log(MY_ERROR_LEVEL,
                  StrEq("Error while storing key: key_id cannot be empty")));
  EXPECT_TRUE(mysql_key_store("", "AES", NULL, sample_key_data.c_str(),
                              sample_key_data.length() + 1));
  char * key_type;
  size_t key_len;
  void * key;
  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(logger.get())),
              log(MY_ERROR_LEVEL,
                  StrEq("Error while fetching key: key_id cannot be empty")));
  EXPECT_TRUE(mysql_key_fetch(NULL, &key_type, "Robert", &key, &key_len));
  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(logger.get())),
              log(MY_ERROR_LEVEL,
                  StrEq("Error while fetching key: key_id cannot be empty")));
  EXPECT_TRUE(mysql_key_fetch(NULL, &key_type, NULL, &key, &key_len));
  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(logger.get())),
              log(MY_ERROR_LEVEL,
                  StrEq("Error while fetching key: key_id cannot be empty")));
  EXPECT_TRUE(mysql_key_fetch("", &key_type, "Robert", &key, &key_len));
  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(logger.get())),
              log(MY_ERROR_LEVEL,
                  StrEq("Error while fetching key: key_id cannot be empty")));
  EXPECT_TRUE(mysql_key_fetch("", &key_type, NULL, &key, &key_len));

  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(logger.get())),
              log(MY_ERROR_LEVEL,
                  StrEq("Error while removing key: key_id cannot be empty")));
  EXPECT_TRUE(mysql_key_remove(NULL, "Robert"));
  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(logger.get())),
              log(MY_ERROR_LEVEL,
                  StrEq("Error while removing key: key_id cannot be empty")));
  EXPECT_TRUE(mysql_key_remove(NULL, NULL));
  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(logger.get())),
              log(MY_ERROR_LEVEL,
                  StrEq("Error while removing key: key_id cannot be empty")));
  EXPECT_TRUE(mysql_key_remove("", "Robert"));
  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(logger.get())),
              log(MY_ERROR_LEVEL,
                  StrEq("Error while removing key: key_id cannot be empty")));
  EXPECT_TRUE(mysql_key_remove("", NULL));

  EXPECT_CALL(
      *(reinterpret_cast<Mock_logger *>(logger.get())),
      log(MY_ERROR_LEVEL,
          StrEq("Error while generating key: key_id cannot be empty")));
  EXPECT_TRUE(mysql_key_generate(NULL, "AES", "Robert", 128));
  EXPECT_CALL(
      *(reinterpret_cast<Mock_logger *>(logger.get())),
      log(MY_ERROR_LEVEL,
          StrEq("Error while generating key: key_id cannot be empty")));
  EXPECT_TRUE(mysql_key_generate(NULL, "AES", NULL, 128));
  EXPECT_CALL(
      *(reinterpret_cast<Mock_logger *>(logger.get())),
      log(MY_ERROR_LEVEL,
          StrEq("Error while generating key: key_id cannot be empty")));
  EXPECT_TRUE(mysql_key_generate("", "AES", "Robert", 128));
  EXPECT_CALL(
      *(reinterpret_cast<Mock_logger *>(logger.get())),
      log(MY_ERROR_LEVEL,
          StrEq("Error while generating key: key_id cannot be empty")));
  EXPECT_TRUE(mysql_key_generate("", "AES", NULL, 128));
}

TEST_F(Keyring_vault_api_test, StorePBStoreSKFetchPBRemovePB)
{
  EXPECT_EQ(
      mysql_key_store("percona_binlog", "AES", NULL, sample_key_data.c_str(),
                      sample_key_data.length() + 1),
      0);
  EXPECT_EQ(mysql_key_store("percona_RGRGRG_1", "AES", NULL, "1234_",
                            strlen("1234_") + 1),
            0);
  char * key_type;
  size_t key_len;
  void * key;
  EXPECT_EQ(
      mysql_key_fetch("percona_binlog", &key_type, NULL, &key, &key_len), 0);
  EXPECT_STREQ("AES", key_type);
  std::string key_data_with_version= "1:" + sample_key_data;
  EXPECT_EQ(key_len, key_data_with_version.length() + 1);
  ASSERT_TRUE(memcmp((char *)key, key_data_with_version.c_str(), key_len) ==
              0);
  my_free(key_type);
  key_type= NULL;
  my_free(key);
  key= NULL;
  EXPECT_EQ(mysql_key_remove("percona_binlog", NULL), 1);
  //make sure the key was not removed - fetch it
  EXPECT_EQ(
      mysql_key_fetch("percona_binlog", &key_type, NULL, &key, &key_len), 0);
  EXPECT_STREQ("AES", key_type);
  EXPECT_EQ(key_len, key_data_with_version.length() + 1);
  ASSERT_TRUE(memcmp((char *)key, key_data_with_version.c_str(), key_len) ==
              0);
  my_free(key_type);
  my_free(key);
}

TEST_F(Keyring_vault_api_test,
       RotatePBStoreSKFetchPBRotatePBFetchPBRotatePBRotateSKFetchPBFetchSK)
{
  std::string percona_binlog_key_data_1("key1");
  EXPECT_EQ(mysql_key_store("percona_binlog", "AES", NULL,
                            percona_binlog_key_data_1.c_str(),
                            percona_binlog_key_data_1.length() + 1),
            0);

  std::string percona_sk_data_1("system_key1");
  EXPECT_EQ(
      mysql_key_store("percona_sk", "AES", NULL, percona_sk_data_1.c_str(),
                      percona_sk_data_1.length() + 1),
      0);

  char * key_type;
  size_t key_len;
  void * key;
  EXPECT_EQ(
      mysql_key_fetch("percona_binlog", &key_type, NULL, &key, &key_len), 0);
  EXPECT_STREQ("AES", key_type);
  std::string key_data_with_version= "2:" + percona_binlog_key_data_1;
  EXPECT_EQ(key_len, key_data_with_version.length() + 1);
  ASSERT_TRUE(memcmp((char *)key, key_data_with_version.c_str(), key_len) ==
              0);
  my_free(key_type);
  key_type= NULL;
  my_free(key);
  key= NULL;

  std::string percona_binlog_key_data_2("key2");
  EXPECT_EQ(mysql_key_store("percona_binlog", "AES", NULL,
                            percona_binlog_key_data_2.c_str(),
                            percona_binlog_key_data_2.length() + 1),
            0);

  EXPECT_EQ(
      mysql_key_fetch("percona_binlog", &key_type, NULL, &key, &key_len), 0);
  EXPECT_STREQ("AES", key_type);
  key_data_with_version= "3:" + percona_binlog_key_data_2;
  EXPECT_EQ(key_len, key_data_with_version.length() + 1);
  ASSERT_TRUE(memcmp((char *)key, key_data_with_version.c_str(), key_len) ==
              0);
  my_free(key_type);
  key_type= NULL;
  my_free(key);
  key= NULL;

  std::string percona_binlog_key_data_3("key3___");
  EXPECT_EQ(mysql_key_store("percona_binlog", "AES", NULL,
                            percona_binlog_key_data_3.c_str(),
                            percona_binlog_key_data_3.length() + 1),
            0);

  std::string percona_sk_data_2("percona_sk_data2");
  EXPECT_EQ(
      mysql_key_store("percona_sk", "AES", NULL, percona_sk_data_2.c_str(),
                      percona_sk_data_2.length() + 1),
      0);
  EXPECT_EQ(
      mysql_key_fetch("percona_binlog", &key_type, NULL, &key, &key_len), 0);
  EXPECT_STREQ("AES", key_type);
  key_data_with_version= "4:" + percona_binlog_key_data_3;
  EXPECT_EQ(key_len, key_data_with_version.length() + 1);
  ASSERT_TRUE(memcmp((char *)key, key_data_with_version.c_str(), key_len) ==
              0);
  my_free(key_type);
  key_type= NULL;
  my_free(key);
  key= NULL;

  EXPECT_EQ(mysql_key_fetch("percona_sk", &key_type, NULL, &key, &key_len),
            0);
  EXPECT_STREQ("AES", key_type);
  key_data_with_version= "2:" + percona_sk_data_2;
  EXPECT_EQ(key_len, key_data_with_version.length() + 1);
  ASSERT_TRUE(memcmp((char *)key, key_data_with_version.c_str(), key_len) ==
              0);
  my_free(key_type);
  key_type= NULL;
  my_free(key);
  key= NULL;
}
}  // namespace keyring__api_unittest

#define TRAP_TEST_CASE_NAME WARNINGKeyring_vault_api_test_trap
#define TRAP_TEST_NAME OnlyThisTestIsRunWhenVaultEnvironmentVariablesAreNotSet
#define TRAP_TEST_FULL_NAME               \
  BOOST_PP_STRINGIZE(TRAP_TEST_CASE_NAME) \
  "." BOOST_PP_STRINGIZE(TRAP_TEST_NAME)

namespace keyring__api_unittest {
GTEST_TEST(TRAP_TEST_CASE_NAME, TRAP_TEST_NAME) { GTEST_SUCCEED(); }
}  // namespace keyring__api_unittest

int main(int argc, char **argv)
{
  system_charset_info= &my_charset_utf8_general_ci;

  ::testing::InitGoogleTest(&argc, argv);
  ::testing::InitGoogleMock(&argc, argv);

  if (is_vault_environment_configured())
  {
    if (!::testing::GTEST_FLAG(filter).empty())
      ::testing::GTEST_FLAG(filter)+= ':';
    ::testing::GTEST_FLAG(filter)+= '-';
    ::testing::GTEST_FLAG(filter)+= TRAP_TEST_FULL_NAME;
    ::testing::AddGlobalTestEnvironment(Vault_environment::create_instance());
  }
  else
    ::testing::GTEST_FLAG(filter)= TRAP_TEST_FULL_NAME;

  return RUN_ALL_TESTS();
}
