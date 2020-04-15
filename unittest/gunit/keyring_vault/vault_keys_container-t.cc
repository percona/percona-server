/* Copyright (c) 2018 Percona LLC and/or its affiliates. All rights reserved.

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

#include <my_global.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <mysql/plugin_keyring.h>
#include <boost/scope_exit.hpp>
#include "vault_keys_container.h"
#include "mock_logger.h"
#include "vault_io.h"
#include <fstream>
#include "i_serialized_object.h"
#include "uuid.h"
#include "generate_credential_file.h"
#include "test_utils.h"
#include "vault_mount.h"

boost::movelib::unique_ptr<keyring::IKeys_container> keys(NULL);

#ifdef HAVE_PSI_INTERFACE
namespace keyring
{
  PSI_memory_key key_memory_KEYRING = PSI_NOT_INSTRUMENTED;
  PSI_memory_key key_LOCK_keyring = PSI_NOT_INSTRUMENTED;
}
#endif
mysql_rwlock_t LOCK_keyring;

namespace keyring__vault_keys_container_unittest
{
  using namespace keyring;
  using ::testing::Return;
  using ::testing::InSequence;
  using ::testing::_;
  using ::testing::StrEq;
  using ::testing::DoAll;
  using ::testing::SetArgPointee;
  using ::testing::WithArgs;
  using ::testing::Invoke;

  static std::string uuid = generate_uuid();
  static std::string credential_file_url = "./keyring_vault.conf";
  ILogger *logger;

  class Vault_keys_container_test : public ::testing::Test
  {
  public:
    Vault_keys_container_test()
    {}
  protected:
    virtual void SetUp()
    {
      sample_key_data = "Robi";
      sample_key = new Vault_key((uuid+"Roberts_key").c_str(), "AES", "Robert", sample_key_data.c_str(), sample_key_data.length());

      vault_keys_container = new Vault_keys_container(logger);
      vault_curl = new Vault_curl(logger, 0);
      vault_parser = new Vault_parser(logger);
    }
    virtual void TearDown()
    {
      delete vault_keys_container;
    }

  protected:
    Vault_keys_container *vault_keys_container;
    IVault_curl *vault_curl;
    IVault_parser *vault_parser;
    std::string correct_token;
    bool credential_file_was_created;
    Vault_key *sample_key;
    std::string sample_key_data;
  };

  TEST_F(Vault_keys_container_test, InitWithCorrectCredential)
  {
    IKeyring_io *vault_io = new Vault_io(logger, vault_curl, vault_parser);
    EXPECT_FALSE(vault_keys_container->init(vault_io, credential_file_url));
    delete sample_key; // unused in this test
  }

  TEST_F(Vault_keys_container_test, InitWithFileWithInvalidToken)
  {
    std::string conf_with_invalid_token("./invalid_token.conf");
    ASSERT_FALSE(generate_credential_file(conf_with_invalid_token, WITH_INVALID_TOKEN));

    IKeyring_io *vault_io = new Vault_io(logger, vault_curl, vault_parser);

    EXPECT_CALL(*(reinterpret_cast<Mock_logger*>(logger)),
      log(MY_ERROR_LEVEL, StrEq("Could not retrieve list of keys from Vault. "
                                "Vault has returned the following error(s): [\"permission denied\"]")));
    EXPECT_CALL(*(reinterpret_cast<Mock_logger*>(logger)),
      log(MY_ERROR_LEVEL, StrEq("Error while loading keyring content. The keyring might be malformed")));
    EXPECT_TRUE(vault_keys_container->init(vault_io, "invalid_token.conf"));
    delete sample_key; // unused in this test

    std::remove("invalid_token.conf");
  }

  TEST_F(Vault_keys_container_test, InitWithEmptyCredentialFile)
  {
    std::remove("empty_credential.conf");
    std::ofstream myfile;
    myfile.open("empty_credential.conf");
    myfile.close();

    IKeyring_io *vault_io = new Vault_io(logger, vault_curl, vault_parser);
    EXPECT_CALL(*(reinterpret_cast<Mock_logger*>(logger)),
      log(MY_ERROR_LEVEL, StrEq("Empty file with credentials.")));
    EXPECT_TRUE(vault_keys_container->init(vault_io, "empty_credential.conf"));
    delete sample_key; // unused in this test

    std::remove("empty_credential.conf");
  }

  TEST_F(Vault_keys_container_test, StoreFetchRemove)
  {
    IKeyring_io *vault_io = new Vault_io(logger, vault_curl, vault_parser);
    EXPECT_FALSE(vault_keys_container->init(vault_io, credential_file_url));
    EXPECT_FALSE(vault_keys_container->store_key(sample_key));
    ASSERT_TRUE(vault_keys_container->get_number_of_keys() == 1);

    Vault_key key_id((uuid+"Roberts_key").c_str(), NULL, "Robert",NULL,0);
    IKey* fetched_key = vault_keys_container->fetch_key(&key_id);

    ASSERT_TRUE(fetched_key != NULL);
    std::string expected_key_signature = get_key_signature(uuid,"Roberts_key","Robert");
    EXPECT_STREQ(fetched_key->get_key_signature()->c_str(), expected_key_signature.c_str());
    EXPECT_EQ(fetched_key->get_key_signature()->length(), expected_key_signature.length());
    uchar* key_data_fetched = fetched_key->get_key_data();
    size_t key_data_fetched_size = fetched_key->get_key_data_size();
    EXPECT_EQ(memcmp(sample_key_data.c_str(), reinterpret_cast<const char*>(key_data_fetched),
                     key_data_fetched_size), 0);
    EXPECT_STREQ("AES", fetched_key->get_key_type()->c_str());
    ASSERT_TRUE(sample_key_data.length() == key_data_fetched_size);

    vault_keys_container->remove_key(&key_id);
    ASSERT_TRUE(vault_keys_container->get_number_of_keys() == 0);
    my_free(fetched_key->release_key_data());
  }

  TEST_F(Vault_keys_container_test, FetchNotExisting)
  {
    IKeyring_io *keyring_io = new Vault_io(logger, vault_curl, vault_parser);
    EXPECT_FALSE(vault_keys_container->init(keyring_io, credential_file_url));
    keyring::Key key_id((uuid+"Roberts_key").c_str(), NULL, "Robert",NULL,0);
    IKey* fetched_key = vault_keys_container->fetch_key(&key_id);
    ASSERT_TRUE(fetched_key == NULL);
    delete sample_key; // unused in this test
  }

  TEST_F(Vault_keys_container_test, RemoveNotExisting)
  {
    IKeyring_io *keyring_io = new Vault_io(logger, vault_curl, vault_parser);
    EXPECT_FALSE(vault_keys_container->init(keyring_io, credential_file_url));
    keyring::Key key_id((uuid+"Roberts_key").c_str(), "AES", "Robert",NULL,0);
    EXPECT_TRUE(vault_keys_container->remove_key(&key_id));
    delete sample_key; // unused in this test
  }

  TEST_F(Vault_keys_container_test, StoreFetchNotExistingDelete)
  {
    IKeyring_io *keyring_io = new Vault_io(logger, vault_curl, vault_parser);
    EXPECT_FALSE(vault_keys_container->init(keyring_io, credential_file_url));
    EXPECT_FALSE(vault_keys_container->store_key(sample_key));
    ASSERT_TRUE(vault_keys_container->get_number_of_keys() == 1);
    keyring::Key key_id((uuid+"NotRoberts_key").c_str(), NULL, "NotRobert",NULL,0);
    IKey* fetched_key = vault_keys_container->fetch_key(&key_id);
    ASSERT_TRUE(fetched_key == NULL);
    ASSERT_TRUE(vault_keys_container->get_number_of_keys() == 1);

    EXPECT_FALSE(vault_keys_container->remove_key(sample_key));
    EXPECT_EQ(vault_keys_container->get_number_of_keys(), (ulong)0);
  }

  TEST_F(Vault_keys_container_test, StoreRemoveNotExisting)
  {
    IKeyring_io *keyring_io = new Vault_io(logger, vault_curl, vault_parser);
    EXPECT_FALSE(vault_keys_container->init(keyring_io, credential_file_url));
    EXPECT_FALSE(vault_keys_container->store_key(sample_key));
    ASSERT_TRUE(vault_keys_container->get_number_of_keys() == 1);
    keyring::Key key_id((uuid+"NotRoberts_key").c_str(), "AES", "NotRobert",NULL,0);
    // Failed to remove key
    ASSERT_TRUE(vault_keys_container->remove_key(&key_id));
    ASSERT_TRUE(vault_keys_container->get_number_of_keys() == 1);

    // Clean up
    EXPECT_FALSE(vault_keys_container->remove_key(sample_key));
    EXPECT_EQ(vault_keys_container->get_number_of_keys(), (ulong)0);
  }

  TEST_F(Vault_keys_container_test, StoreStoreStoreFetchRemove)
  {
    ASSERT_TRUE(vault_keys_container->get_number_of_keys() == 0);
    IKeyring_io *keyring_io = new Vault_io(logger, vault_curl, vault_parser);
    EXPECT_FALSE(vault_keys_container->init(keyring_io, credential_file_url));
    EXPECT_FALSE(vault_keys_container->store_key(sample_key));
    ASSERT_TRUE(vault_keys_container->get_number_of_keys() == 1);

    std::string key_data1("Robi1");
    Vault_key *key1 = new Vault_key((uuid+"Roberts_key1").c_str(), "AES", "Robert", key_data1.c_str(), key_data1.length());

    EXPECT_FALSE(vault_keys_container->store_key(key1));
    ASSERT_TRUE(vault_keys_container->get_number_of_keys() == 2);

    std::string key_data2("Robi2");
    Vault_key *key2 = new Vault_key((uuid+"Roberts_key2").c_str(), "AES", "Robert", key_data2.c_str(), key_data2.length());
    EXPECT_FALSE(vault_keys_container->store_key(key2));
    ASSERT_TRUE(vault_keys_container->get_number_of_keys() == 3);

    std::string key_data3("Robi3");
    Vault_key *key3 = new Vault_key((uuid+"Roberts_key3").c_str(), "AES", "Robert", key_data3.c_str(), key_data3.length());

    EXPECT_FALSE(vault_keys_container->store_key(key3));
    ASSERT_TRUE(vault_keys_container->get_number_of_keys() == 4);

    Vault_key key2_id((uuid+"Roberts_key2").c_str(), NULL, "Robert",NULL,0);
    IKey* fetched_key = vault_keys_container->fetch_key(&key2_id);

    ASSERT_TRUE(fetched_key != NULL);
    std::string expected_key_signature = get_key_signature(uuid,"Roberts_key2","Robert");
    EXPECT_STREQ(fetched_key->get_key_signature()->c_str(), expected_key_signature.c_str());
    EXPECT_EQ(fetched_key->get_key_signature()->length(), expected_key_signature.length());
    uchar *key_data_fetched = fetched_key->get_key_data();
    size_t key_data_fetched_size = fetched_key->get_key_data_size();
    EXPECT_FALSE(memcmp(key_data_fetched, key_data2.c_str(), key_data_fetched_size)); 
    ASSERT_TRUE(key_data2.length() == key_data_fetched_size);

    Vault_key key3_id((uuid+"Roberts_key3").c_str(), NULL, "Robert",NULL,0);
    vault_keys_container->remove_key(&key3_id);
    vault_keys_container->remove_key(key2);
    vault_keys_container->remove_key(key1);
    vault_keys_container->remove_key(sample_key);
    ASSERT_TRUE(vault_keys_container->get_number_of_keys() == 0);

    my_free(fetched_key->release_key_data());
  }

  TEST_F(Vault_keys_container_test, StoreTwiceTheSame)
  {
    IKeyring_io *keyring_io = new Vault_io(logger, vault_curl, vault_parser);
    EXPECT_FALSE(vault_keys_container->init(keyring_io, credential_file_url));
    EXPECT_FALSE(vault_keys_container->store_key(sample_key));
    ASSERT_TRUE(vault_keys_container->get_number_of_keys() == 1);
    EXPECT_TRUE(vault_keys_container->store_key(sample_key));
    ASSERT_TRUE(vault_keys_container->get_number_of_keys() == 1);

    vault_keys_container->remove_key(sample_key);
    ASSERT_TRUE(vault_keys_container->get_number_of_keys() == 0);
  }

  TEST_F(Vault_keys_container_test, StoreStoreStoreFetchRemoveWithSleeps)
  {
    IKeyring_io *keyring_io = new Vault_io(logger, vault_curl, vault_parser);
    EXPECT_FALSE(vault_keys_container->init(keyring_io, credential_file_url));
    EXPECT_FALSE(vault_keys_container->store_key(sample_key));
    ASSERT_TRUE(vault_keys_container->get_number_of_keys() == 1);

    my_sleep(20000000);

    std::string key_data1("Robi1");
    Vault_key *key1 = new Vault_key((uuid+"Roberts_key1").c_str(), "AES", "Robert", key_data1.c_str(), key_data1.length());

    EXPECT_FALSE(vault_keys_container->store_key(key1));
    ASSERT_TRUE(vault_keys_container->get_number_of_keys() == 2);

    my_sleep(10000000);

    std::string key_data2("Robi2");
    Vault_key *key2 = new Vault_key((uuid+"Roberts_key2").c_str(), "AES", "Robert", key_data2.c_str(), key_data2.length());
    EXPECT_FALSE(vault_keys_container->store_key(key2));
    ASSERT_TRUE(vault_keys_container->get_number_of_keys() == 3);

    my_sleep(5000000);

    std::string key_data3("Robi3");
    Vault_key *key3 = new Vault_key((uuid+"Roberts_key3").c_str(), "AES", "Robert", key_data3.c_str(), key_data3.length());

    EXPECT_FALSE(vault_keys_container->store_key(key3));
    ASSERT_TRUE(vault_keys_container->get_number_of_keys() == 4);

    Vault_key key2_id((uuid+"Roberts_key2").c_str(), NULL, "Robert",NULL,0);
    IKey* fetched_key = vault_keys_container->fetch_key(&key2_id);

    my_sleep(5000000);

    ASSERT_TRUE(fetched_key != NULL);
    std::string expected_key_signature = get_key_signature(uuid,"Roberts_key2","Robert");
    EXPECT_STREQ(fetched_key->get_key_signature()->c_str(), expected_key_signature.c_str());
    EXPECT_EQ(fetched_key->get_key_signature()->length(), expected_key_signature.length());
    uchar *key_data_fetched = fetched_key->get_key_data();
    size_t key_data_fetched_size = fetched_key->get_key_data_size();
    EXPECT_FALSE(memcmp(key_data_fetched, key_data2.c_str(), key_data_fetched_size)); 
    ASSERT_TRUE(key_data2.length() == key_data_fetched_size);

    Vault_key key3_id((uuid+"Roberts_key3").c_str(), NULL, "Robert",NULL,0);
    vault_keys_container->remove_key(&key3_id);
    vault_keys_container->remove_key(key2);
    my_sleep(5000000);
    vault_keys_container->remove_key(key1);
    vault_keys_container->remove_key(sample_key);
    ASSERT_TRUE(vault_keys_container->get_number_of_keys() == 0);

    my_free(fetched_key->release_key_data());
  }

  TEST_F(Vault_keys_container_test, StorePBStorePBStorePBStoreIK1StoreIK2FetchPBFetchIK)
  {
    IKeyring_io *keyring_io = new Vault_io(logger, vault_curl, vault_parser);
    EXPECT_FALSE(vault_keys_container->init(keyring_io, credential_file_url));

    std::string key_data1("system_key_data_1");
    Vault_key *key1= new Vault_key("percona_binlog:0", "AES", NULL, key_data1.c_str(), key_data1.length()+1);
    key1->xor_data();
    EXPECT_EQ(vault_keys_container->store_key(key1), 0);

    std::string key_data2("system_key_data_2");
    Vault_key *key2= new Vault_key("percona_binlog:1", "AES", NULL, key_data2.c_str(), key_data2.length()+1);
    key2->xor_data();
    EXPECT_EQ(vault_keys_container->store_key(key2), 0);

    std::string key_data3("system_key_data_3");
    Vault_key *key3= new Vault_key("percona_binlog:2", "AES", NULL, key_data3.c_str(), key_data3.length()+1);
    key3->xor_data();
    EXPECT_EQ(vault_keys_container->store_key(key3), 0);

    std::string ik_data1("data1");

    Vault_key *innodb_key1= new Vault_key("percona_innodb1_2_3:0:0", "AES", NULL, ik_data1.c_str(), ik_data1.length()+1);
    innodb_key1->xor_data();
    EXPECT_EQ(vault_keys_container->store_key(innodb_key1), 0);

    std::string ik_data2("data2");

    Vault_key *innodb_key2= new Vault_key("percona_innodb1_2_3:0:1", "AES", NULL, ik_data2.c_str(), ik_data2.length()+1);
    innodb_key2->xor_data();
    EXPECT_EQ(vault_keys_container->store_key(innodb_key2), 0);

    Vault_key latest_percona_binlog_key("percona_binlog", NULL, NULL, NULL, 0);
    IKey* fetched_key= vault_keys_container->fetch_key(&latest_percona_binlog_key);
    ASSERT_TRUE(fetched_key != NULL);

    Vault_key key(fetched_key->get_key_id()->c_str(), fetched_key->get_key_type()->c_str(), fetched_key->get_user_id()->c_str(),
            fetched_key->get_key_data(), fetched_key->get_key_data_size());
    key.xor_data();

    std::string expected_key_signature= get_key_signature("","percona_binlog","");
    EXPECT_STREQ(key.get_key_signature()->c_str(), expected_key_signature.c_str());
    EXPECT_EQ(key.get_key_signature()->length(), expected_key_signature.length());
    uchar* key_data_fetched= key.get_key_data();
    size_t key_data_fetched_size= key.get_key_data_size();
    std::string key_data_with_version= "2:" + key_data3;
    EXPECT_STREQ(key_data_with_version.c_str(), reinterpret_cast<const char*>(key_data_fetched));
    EXPECT_STREQ("AES", fetched_key->get_key_type()->c_str());
    ASSERT_TRUE(key_data_with_version.length()+1 == key_data_fetched_size);

    Vault_key latest_innodb_key("percona_innodb1_2_3:0", NULL, NULL, NULL, 0);
    IKey* fetched_innodb_key= vault_keys_container->fetch_key(&latest_innodb_key);
    ASSERT_TRUE(fetched_innodb_key != NULL);

    Vault_key innodb_key(fetched_innodb_key->get_key_id()->c_str(), fetched_innodb_key->get_key_type()->c_str(), fetched_innodb_key->get_user_id()->c_str(),
            fetched_innodb_key->get_key_data(), fetched_innodb_key->get_key_data_size());
    innodb_key.xor_data();

    expected_key_signature= get_key_signature("","percona_innodb1_2_3:0","");
    EXPECT_STREQ(innodb_key.get_key_signature()->c_str(), expected_key_signature.c_str());
    EXPECT_EQ(innodb_key.get_key_signature()->length(), expected_key_signature.length());
    key_data_fetched= innodb_key.get_key_data();
    key_data_fetched_size= innodb_key.get_key_data_size();
    key_data_with_version= "1:" + ik_data2;
    EXPECT_STREQ(key_data_with_version.c_str(), reinterpret_cast<const char*>(key_data_fetched));
    EXPECT_STREQ("AES", fetched_key->get_key_type()->c_str());
    ASSERT_TRUE(key_data_with_version.length()+1 == key_data_fetched_size);

    my_free(fetched_key->release_key_data());
    my_free(fetched_innodb_key->release_key_data());

    delete sample_key; // unused in this test
  }

  TEST_F(Vault_keys_container_test, StorePBRotatePBFetchPBStoreSKRotatePBFetchPBRotateSKFetchSK)
  {
    IKeyring_io *keyring_io= new Vault_io(logger, vault_curl, vault_parser);
    EXPECT_FALSE(vault_keys_container->init(keyring_io, credential_file_url));

    std::string key_data1("percona_binlog_key_data_1");
    Vault_key *key1= new Vault_key("percona_binlog:3", "AES", NULL, key_data1.c_str(), key_data1.length()+1);
    key1->xor_data();

    EXPECT_EQ(vault_keys_container->store_key(key1), FALSE);

    std::string key_data2("percona_binlog_key_data_2");
    Vault_key *percona_binlog_rotation= new Vault_key("percona_binlog", "AES", NULL, key_data2.c_str(), key_data2.length()+1);
    percona_binlog_rotation->xor_data();
    EXPECT_EQ(vault_keys_container->store_key(percona_binlog_rotation), FALSE);

    Vault_key latest_percona_binlog_key("percona_binlog", NULL, NULL, NULL, 0);
    IKey* fetched_key= vault_keys_container->fetch_key(&latest_percona_binlog_key);

    ASSERT_TRUE(fetched_key != NULL);

    Vault_key key(fetched_key->get_key_id()->c_str(), fetched_key->get_key_type()->c_str(), fetched_key->get_user_id()->c_str(),
            fetched_key->get_key_data(), fetched_key->get_key_data_size());
    key.xor_data();

    std::string expected_key_signature= get_key_signature("","percona_binlog","");
    EXPECT_STREQ(key.get_key_signature()->c_str(), expected_key_signature.c_str());
    EXPECT_EQ(key.get_key_signature()->length(), expected_key_signature.length());
    uchar* key_data_fetched= key.get_key_data();
    size_t key_data_fetched_size= key.get_key_data_size();
    std::string key_data_with_version= "4:" + key_data2;
    EXPECT_STREQ(key_data_with_version.c_str(), reinterpret_cast<const char*>(key_data_fetched));
    EXPECT_STREQ("AES", fetched_key->get_key_type()->c_str());
    ASSERT_TRUE(key_data_with_version.length()+1 == key_data_fetched_size);

    std::string sk_data1("sk_data_1");
    Vault_key *sys_key1= new Vault_key("percona_sk:0", "AES", NULL, sk_data1.c_str(), sk_data1.length()+1);
    sys_key1->xor_data();

    EXPECT_EQ(vault_keys_container->store_key(sys_key1), FALSE);

    std::string key_data3("system_key_data_3");
    Vault_key *percona_binlog_rotation4_to_5= new Vault_key("percona_binlog", "AES", NULL, key_data3.c_str(), key_data3.length()+1);
    percona_binlog_rotation4_to_5->xor_data();
    EXPECT_EQ(vault_keys_container->store_key(percona_binlog_rotation4_to_5), FALSE);

    Vault_key latest_percona_binlog_key_2("percona_binlog", NULL, NULL, NULL, 0);
    IKey* fetched_key_2= vault_keys_container->fetch_key(&latest_percona_binlog_key_2);

    ASSERT_TRUE(fetched_key_2 != NULL);

    Vault_key key_2(fetched_key_2->get_key_id()->c_str(), fetched_key_2->get_key_type()->c_str(), fetched_key_2->get_user_id()->c_str(),
              fetched_key_2->get_key_data(), fetched_key_2->get_key_data_size());
    key_2.xor_data();

    EXPECT_STREQ(key_2.get_key_signature()->c_str(), expected_key_signature.c_str());
    EXPECT_EQ(key_2.get_key_signature()->length(), expected_key_signature.length());
    key_data_fetched= key_2.get_key_data();
    key_data_fetched_size= key_2.get_key_data_size();
    key_data_with_version= "5:" + key_data3;
    EXPECT_STREQ(key_data_with_version.c_str(), reinterpret_cast<const char*>(key_data_fetched));
    EXPECT_STREQ("AES", fetched_key->get_key_type()->c_str());
    ASSERT_TRUE(key_data_with_version.length()+1 == key_data_fetched_size);

    std::string key_data4("system_key_data_4");
    Vault_key *percona_binlog_rotation5_to_6= new Vault_key("percona_binlog", "AES", NULL, key_data4.c_str(), key_data4.length()+1);
    percona_binlog_rotation5_to_6->xor_data();
    EXPECT_EQ(vault_keys_container->store_key(percona_binlog_rotation5_to_6), FALSE);

    Vault_key latest_percona_binlog_key_3("percona_binlog", NULL, NULL, NULL, 0);
    IKey* fetched_key_3= vault_keys_container->fetch_key(&latest_percona_binlog_key_3);

    ASSERT_TRUE(fetched_key_3 != NULL);

    Vault_key key_3(fetched_key_3->get_key_id()->c_str(), fetched_key_3->get_key_type()->c_str(), fetched_key_3->get_user_id()->c_str(),
              fetched_key_3->get_key_data(), fetched_key_3->get_key_data_size());
    key_3.xor_data();

    EXPECT_STREQ(key_3.get_key_signature()->c_str(), expected_key_signature.c_str());
    EXPECT_EQ(key_3.get_key_signature()->length(), expected_key_signature.length());
    key_data_fetched= key_3.get_key_data();
    key_data_fetched_size= key_3.get_key_data_size();
    key_data_with_version= "6:" + key_data4;
    EXPECT_STREQ(key_data_with_version.c_str(), reinterpret_cast<const char*>(key_data_fetched));
    EXPECT_STREQ("AES", fetched_key->get_key_type()->c_str());
    ASSERT_TRUE(key_data_with_version.length()+1 == key_data_fetched_size);

    std::string sk_data2("sk_data_2");
    Vault_key *percona_sk_rotation0_to_1= new Vault_key("percona_sk", "AES", NULL, sk_data2.c_str(), sk_data2.length()+1);
    percona_sk_rotation0_to_1->xor_data();
    EXPECT_EQ(vault_keys_container->store_key(percona_sk_rotation0_to_1), FALSE);

    Vault_key latest_sk("percona_sk", NULL, NULL, NULL, 0);
    IKey* fetched_sk= vault_keys_container->fetch_key(&latest_sk);

    ASSERT_TRUE(fetched_sk != NULL);

    Vault_key sk(fetched_sk->get_key_id()->c_str(), fetched_sk->get_key_type()->c_str(), fetched_sk->get_user_id()->c_str(),
           fetched_sk->get_key_data(), fetched_sk->get_key_data_size());
    sk.xor_data();

    expected_key_signature = get_key_signature("","percona_sk","");
    EXPECT_STREQ(sk.get_key_signature()->c_str(), expected_key_signature.c_str());
    EXPECT_EQ(sk.get_key_signature()->length(), expected_key_signature.length());
    key_data_fetched= sk.get_key_data();
    key_data_fetched_size= sk.get_key_data_size();
    key_data_with_version = "1:" + sk_data2;
    EXPECT_STREQ(key_data_with_version.c_str(), reinterpret_cast<const char*>(key_data_fetched));
    EXPECT_STREQ("AES", fetched_sk->get_key_type()->c_str());
    ASSERT_TRUE(key_data_with_version.length()+1 == key_data_fetched_size);

    my_free(fetched_key->release_key_data());
    my_free(fetched_key_2->release_key_data());
    my_free(fetched_key_3->release_key_data());
    my_free(fetched_sk->release_key_data());

    delete sample_key; // unused in this test
  }

  TEST_F(Vault_keys_container_test, StoreStoreStoreSystemKeyAndTryRemovingSystemKey)
  {
    IKeyring_io *keyring_io = new Vault_io(logger, vault_curl, vault_parser);
    EXPECT_FALSE(vault_keys_container->init(keyring_io, credential_file_url));

    std::string key_data1("system_key_data_1");
    IKey *key1= new Vault_key("percona_binlog:7", "AES", NULL, key_data1.c_str(), key_data1.length()+1);

    EXPECT_EQ(vault_keys_container->store_key(key1), 0);

    std::string key_data2("system_key_data_2");
    Vault_key *key2= new Vault_key("percona_binlog:8", "AES", NULL, key_data2.c_str(), key_data2.length()+1);
    EXPECT_EQ(vault_keys_container->store_key(key2), 0);

    std::string key_data3("system_key_data_3");
    Vault_key *key3= new Vault_key("percona_binlog:9", "AES", NULL, key_data3.c_str(), key_data3.length()+1);
    EXPECT_EQ(vault_keys_container->store_key(key3), 0);

    Vault_key latest_percona_binlog_key("percona_binlog:9", NULL, NULL, NULL, 0);
    ASSERT_TRUE(vault_keys_container->remove_key(&latest_percona_binlog_key) == TRUE);

    Vault_key percona_binlog_key("percona_binlog", NULL, NULL, NULL, 0);
    ASSERT_TRUE(vault_keys_container->remove_key(&percona_binlog_key) == TRUE);

    delete sample_key; // unused in this test
  }

  TEST_F(Vault_keys_container_test, StoreStoreStoreRemoveFetchSystemKeyFetchRegularKey)
  {
    IKeyring_io *keyring_io = new Vault_io(logger, vault_curl, vault_parser);
    EXPECT_FALSE(vault_keys_container->init(keyring_io, credential_file_url));

    std::string key_data1("system_key_data_1");
    IKey *key1= new Vault_key("percona_binlog:10", "AES", NULL, key_data1.c_str(), key_data1.length()+1);

    EXPECT_EQ(vault_keys_container->store_key(key1), 0);

    std::string key_data2("system_key_data_2");
    Vault_key *key2= new Vault_key("percona_binlog:11", "AES", NULL, key_data2.c_str(), key_data2.length()+1);
    key2->xor_data();
    EXPECT_EQ(vault_keys_container->store_key(key2), 0);

    std::string key_data3("Robi3");
    Vault_key *key3= new Vault_key("Roberts_key3", "AES", "Robert", key_data3.c_str(), key_data3.length()+1);
    EXPECT_EQ(vault_keys_container->store_key(key3), 0);

    std::string key_data4("Robi4");
    Vault_key *key4= new Vault_key("Roberts_key4", "AES", "Robert", key_data4.c_str(), key_data4.length()+1);
    EXPECT_EQ(vault_keys_container->store_key(key4), 0);

    Vault_key key3_id("Roberts_key3", "AES", "Robert",NULL,0);
    vault_keys_container->remove_key(&key3_id);

    Vault_key latest_percona_binlog_key("percona_binlog", NULL, NULL, NULL, 0);
    IKey* fetched_key= vault_keys_container->fetch_key(&latest_percona_binlog_key);

    ASSERT_TRUE(fetched_key != NULL);

    std::string expected_key_signature= get_key_signature("","percona_binlog","");
    EXPECT_STREQ(fetched_key->get_key_signature()->c_str(), expected_key_signature.c_str());
    EXPECT_EQ(fetched_key->get_key_signature()->length(), expected_key_signature.length());
    uchar *key_data_fetched= fetched_key->get_key_data();
    size_t key_data_fetched_size= fetched_key->get_key_data_size();
    std::string key_data_with_version= "11:" + key_data2;
    EXPECT_STREQ(key_data_with_version.c_str(), reinterpret_cast<const char*>(key_data_fetched));
    EXPECT_STREQ("AES", fetched_key->get_key_type()->c_str());
    ASSERT_TRUE(key_data_with_version.length()+1 == key_data_fetched_size);

    my_free(fetched_key->release_key_data());

    Vault_key regular_key("Roberts_key4", NULL, "Robert", NULL, 0);
    IKey *fetched_regular_key= vault_keys_container->fetch_key(&regular_key);

    ASSERT_TRUE(fetched_regular_key != NULL);
    std::string expected_regular_key_signature= get_key_signature("","Roberts_key4","Robert");
    EXPECT_STREQ(fetched_regular_key->get_key_signature()->c_str(), expected_regular_key_signature.c_str());
    EXPECT_EQ(fetched_regular_key->get_key_signature()->length(), expected_regular_key_signature.length());
    uchar *regular_key_data_fetched= fetched_regular_key->get_key_data();
    size_t regular_key_data_fetched_size= fetched_regular_key->get_key_data_size();
    EXPECT_STREQ(key_data4.c_str(), reinterpret_cast<const char*>(regular_key_data_fetched));
    ASSERT_TRUE(key_data4.length()+1 == regular_key_data_fetched_size);

    my_free(fetched_regular_key->release_key_data());
    delete sample_key; // unused in this test
  }

  class Mock_vault_io : public IVault_io
  {
  public:
    MOCK_METHOD1(retrieve_key_type_and_data, my_bool(IKey *key));
    MOCK_METHOD1(init, my_bool(std::string *keyring_filename));
    MOCK_METHOD1(flush_to_backup, my_bool(ISerialized_object *serialized_object));
    MOCK_METHOD1(flush_to_storage, my_bool(ISerialized_object *serialized_object));
    MOCK_METHOD0(get_serializer, ISerializer*());
    MOCK_METHOD1(get_serialized_object, my_bool(ISerialized_object **serialized_object));
    MOCK_METHOD0(has_next_serialized_object, my_bool());
    MOCK_METHOD1(set_curl_timeout, void(uint timeout));
  };

  class Mock_serialized_object : public ISerialized_object
  {
  public:
    MOCK_METHOD1(get_next_key, my_bool(IKey **key));
    MOCK_METHOD0(has_next_key, my_bool());
    MOCK_METHOD0(get_key_operation, Key_operation());
    MOCK_METHOD1(set_key_operation, void(Key_operation));
  };

  class Mock_serializer : public ISerializer
  {
  public:
    MOCK_METHOD3(serialize, ISerialized_object*(HASH*, IKey*, Key_operation));
  };

  class Vault_keys_container_with_mocked_io_test : public ::testing::Test
  {
  protected:
    virtual void SetUp()
    {
      std::string sample_key_data = "Robi";
      sample_key = new Vault_key((uuid+"Roberts_key").c_str(), "AES", "Robert", sample_key_data.c_str(), sample_key_data.length());
      credential_file_url = "./credentials";
    }
    virtual void TearDown()
    {
      delete vault_keys_container;
    }
  protected:
    Vault_keys_container *vault_keys_container;
    Mock_vault_io *vault_io;
    Vault_key *sample_key;
    char* sample_key_data;
    std::string credential_file_url;

    void expect_calls_on_init();
    void expect_calls_on_store_sample_key();
  };

  void Vault_keys_container_with_mocked_io_test::expect_calls_on_init()
  {
    Mock_serialized_object *mock_serialized_object = new Mock_serialized_object;

    EXPECT_CALL(*vault_io, init(Pointee(StrEq(credential_file_url))))
      .WillOnce(Return(FALSE)); // init successfull
    EXPECT_CALL(*vault_io, get_serialized_object(_))
      .WillOnce(DoAll(SetArgPointee<0>(mock_serialized_object), Return(FALSE)));
    EXPECT_CALL(*mock_serialized_object, has_next_key()).WillOnce(Return(FALSE)); // no keys to read
    EXPECT_CALL(*vault_io, has_next_serialized_object()).WillOnce(Return(FALSE));
  }

  TEST_F(Vault_keys_container_with_mocked_io_test, ErrorFromIODuringInitOnGettingSerializedObject)
  {
    vault_io = new Mock_vault_io();
    Mock_logger *logger = new Mock_logger();
    vault_keys_container = new Vault_keys_container(logger);

    EXPECT_CALL(*vault_io, init(Pointee(StrEq(credential_file_url))))
      .WillOnce(Return(FALSE)); // init successfull
    EXPECT_CALL(*vault_io, get_serialized_object(_)).WillOnce(Return(TRUE));
    EXPECT_CALL(*logger, log(MY_ERROR_LEVEL, StrEq("Error while loading keyring content. The keyring might be malformed")));

    EXPECT_TRUE(vault_keys_container->init(vault_io, credential_file_url));
    EXPECT_EQ(vault_keys_container->get_number_of_keys(), (unsigned int)0);
    delete logger;
    delete sample_key; // unused in this test
  }

  TEST_F(Vault_keys_container_with_mocked_io_test, ErrorFromIODuringInitInvalidKeyAndMockedSerializedObject)
  {
    vault_io = new Mock_vault_io();
    Mock_logger *logger = new Mock_logger();
    vault_keys_container = new Vault_keys_container(logger);

    IKey *invalid_key = new Vault_key();
    std::string invalid_key_type("ZZZ");
    invalid_key->set_key_type(&invalid_key_type);

    Mock_serialized_object *mock_serialized_object = new Mock_serialized_object;

    EXPECT_CALL(*vault_io, init(Pointee(StrEq(credential_file_url))))
      .WillOnce(Return(FALSE)); // init successfull
    {
      InSequence dummy;
      EXPECT_CALL(*vault_io, get_serialized_object(_)).WillOnce(DoAll(SetArgPointee<0>(mock_serialized_object), Return(FALSE)));
      EXPECT_CALL(*mock_serialized_object, has_next_key()).WillOnce(Return(TRUE));
      EXPECT_CALL(*mock_serialized_object, get_next_key(_)).WillOnce(DoAll(SetArgPointee<0>(sample_key), Return(FALSE)));
      EXPECT_CALL(*mock_serialized_object, has_next_key()).WillOnce(Return(TRUE));
      EXPECT_CALL(*mock_serialized_object, get_next_key(_)).WillOnce(DoAll(SetArgPointee<0>(invalid_key), Return(FALSE)));

      EXPECT_CALL(*logger, log(MY_ERROR_LEVEL, StrEq("Error while loading keyring content. The keyring might be malformed")));
   }

    EXPECT_TRUE(vault_keys_container->init(vault_io, credential_file_url));
    EXPECT_EQ(vault_keys_container->get_number_of_keys(), static_cast<uint>(0));
    delete logger;
  }

  TEST_F(Vault_keys_container_with_mocked_io_test, ErrorFromIODuringInitInvalidKey)
  {
    vault_io = new Mock_vault_io();
    Mock_logger *logger = new Mock_logger();
    vault_keys_container = new Vault_keys_container(logger);

    Vault_key *invalid_key = new Vault_key();
    std::string invalid_key_type("ZZZ");
    invalid_key->set_key_type(&invalid_key_type);

    Vault_keys_list *keys_list = new Vault_keys_list();
    keys_list->push_back(invalid_key);

    EXPECT_CALL(*vault_io, init(Pointee(StrEq(credential_file_url))))
      .WillOnce(Return(FALSE)); // init successfull
    {
      InSequence dummy;
      EXPECT_CALL(*vault_io, get_serialized_object(_)).WillOnce(DoAll(SetArgPointee<0>(keys_list), Return(FALSE)));
      EXPECT_CALL(*logger, log(MY_ERROR_LEVEL, StrEq("Error while loading keyring content. The keyring might be malformed")));
    }
    EXPECT_TRUE(vault_keys_container->init(vault_io, credential_file_url));
    EXPECT_EQ(vault_keys_container->get_number_of_keys(), static_cast<uint>(0));
    delete logger;
    delete sample_key; // unused in this test
  }

  TEST_F(Vault_keys_container_with_mocked_io_test, ErrorFromSerializerOnFlushToKeyringWhenStoringKey)
  {
    vault_io = new Mock_vault_io();
    Mock_logger *logger = new Mock_logger();
    vault_keys_container = new Vault_keys_container(logger);
    expect_calls_on_init();
    EXPECT_FALSE(vault_keys_container->init(vault_io, credential_file_url));
    EXPECT_EQ(vault_keys_container->get_number_of_keys(), static_cast<uint>(0));
    Mock_serializer *mock_serializer = new Mock_serializer;

    {
      InSequence dummy;
      ISerialized_object *null_serialized_object = NULL;
      // flush to keyring
      EXPECT_CALL(*vault_io, get_serializer())
        .WillOnce(Return(mock_serializer));
      EXPECT_CALL(*mock_serializer, serialize(_,sample_key,STORE_KEY))
        .WillOnce(Return(null_serialized_object));
      EXPECT_CALL(*logger, log(MY_ERROR_LEVEL, StrEq("Could not flush keys to keyring")));
    }
    EXPECT_TRUE(vault_keys_container->store_key(sample_key));
    ASSERT_TRUE(vault_keys_container->get_number_of_keys() == 0);

    delete logger;
    delete sample_key;
    delete mock_serializer;
  }

  TEST_F(Vault_keys_container_with_mocked_io_test, ErrorFromSerializerOnFlushToKeyringWhenRemovingKey)
  {
    vault_io = new Mock_vault_io();
    Mock_logger *logger = new Mock_logger();
    vault_keys_container = new Vault_keys_container(logger);
    expect_calls_on_init();
    EXPECT_FALSE(vault_keys_container->init(vault_io, credential_file_url));
    ASSERT_TRUE(vault_keys_container->get_number_of_keys() == 0);
    Mock_serializer *mock_serializer = new Mock_serializer;

    Vault_key *serialized_sample_key = new Vault_key(*sample_key);

    {
      InSequence dummy;
      // flush to keyring
      EXPECT_CALL(*vault_io, get_serializer())
        .WillOnce(Return(mock_serializer));
      EXPECT_CALL(*mock_serializer, serialize(_,sample_key,STORE_KEY))
        .WillOnce(Return(serialized_sample_key));
      EXPECT_CALL(*vault_io, flush_to_storage(serialized_sample_key));
    }
    EXPECT_FALSE(vault_keys_container->store_key(sample_key));
    ASSERT_TRUE(vault_keys_container->get_number_of_keys() == 1);

    {
      InSequence dummy;
      ISerialized_object *null_serialized_object = NULL;
      // flush to keyring
      EXPECT_CALL(*vault_io, get_serializer())
        .WillOnce(Return(mock_serializer));
      EXPECT_CALL(*mock_serializer, serialize(_,sample_key,REMOVE_KEY))
        .WillOnce(Return(null_serialized_object));
      EXPECT_CALL(*logger, log(MY_ERROR_LEVEL, StrEq("Could not flush keys to keyring")));
    }

    EXPECT_TRUE(vault_keys_container->remove_key(sample_key));
    ASSERT_TRUE(vault_keys_container->get_number_of_keys() == 1);

    delete logger;
    delete mock_serializer;
  }

  TEST_F(Vault_keys_container_with_mocked_io_test, StoreAndRemoveKey)
  {
    vault_io = new Mock_vault_io();
    Mock_logger *logger = new Mock_logger();
    vault_keys_container = new Vault_keys_container(logger);
    expect_calls_on_init();
    EXPECT_FALSE(vault_keys_container->init(vault_io, credential_file_url));
    ASSERT_TRUE(vault_keys_container->get_number_of_keys() == 0);
    Mock_serializer *mock_serializer = new Mock_serializer;

    Vault_key *serialized_sample_key = new Vault_key(*sample_key);
 
    {
      InSequence dummy;
      // flush to keyring
      EXPECT_CALL(*vault_io, get_serializer())
        .WillOnce(Return(mock_serializer));
      EXPECT_CALL(*mock_serializer, serialize(_,sample_key,STORE_KEY))
        .WillOnce(Return(serialized_sample_key));
      EXPECT_CALL(*vault_io, flush_to_storage(serialized_sample_key))
	.WillOnce(Return(FALSE));
    }
    sample_key->set_key_operation(STORE_KEY);
    EXPECT_FALSE(vault_keys_container->store_key(sample_key));
    ASSERT_TRUE(vault_keys_container->get_number_of_keys() == 1);

    Vault_key *serialized_sample_key_to_remove = new Vault_key(*sample_key);

    serialized_sample_key_to_remove->set_key_operation(REMOVE_KEY);
    {
      InSequence dummy;
      // flush to keyring
      EXPECT_CALL(*vault_io, get_serializer())
        .WillOnce(Return(mock_serializer));
      EXPECT_CALL(*mock_serializer, serialize(_,sample_key,REMOVE_KEY))
        .WillOnce(Return(serialized_sample_key_to_remove));
      EXPECT_CALL(*vault_io, flush_to_storage(serialized_sample_key_to_remove));
    }
    sample_key->set_key_operation(REMOVE_KEY);
    EXPECT_FALSE(vault_keys_container->remove_key(sample_key));
    ASSERT_TRUE(vault_keys_container->get_number_of_keys() == 0);

    delete logger;
    delete mock_serializer;
  }

  TEST_F(Vault_keys_container_with_mocked_io_test, ErrorFromIOWhileRemovingKeyAfterAdding2Keys)
  {
    vault_io = new Mock_vault_io();
    Mock_logger *logger = new Mock_logger();
    vault_keys_container = new Vault_keys_container(logger);
    expect_calls_on_init();
    EXPECT_FALSE(vault_keys_container->init(vault_io, credential_file_url));
    ASSERT_TRUE(vault_keys_container->get_number_of_keys() == 0);
    Mock_serializer *mock_serializer = new Mock_serializer;

    Vault_key *serialized_sample_key = new Vault_key(*sample_key);

    {
      InSequence dummy;
      // flush to keyring
      EXPECT_CALL(*vault_io, get_serializer())
        .WillOnce(Return(mock_serializer));
      EXPECT_CALL(*mock_serializer, serialize(_,sample_key,STORE_KEY))
        .WillOnce(Return(serialized_sample_key));
      EXPECT_CALL(*vault_io, flush_to_storage(serialized_sample_key));
    }
    EXPECT_FALSE(vault_keys_container->store_key(sample_key));
    ASSERT_TRUE(vault_keys_container->get_number_of_keys() == 1);

    std::string key_data2("Robi2");
    Vault_key *key2 = new Vault_key((uuid+"Roberts_key2").c_str(), "AES", "Robert", key_data2.c_str(), key_data2.length());

    Vault_key *serialized_key2 = new Vault_key(*key2);

    {
      InSequence dummy;
      // flush to keyring
      EXPECT_CALL(*vault_io, get_serializer())
        .WillOnce(Return(mock_serializer));
      EXPECT_CALL(*mock_serializer, serialize(_,key2,STORE_KEY))
        .WillOnce(Return(serialized_key2));
      EXPECT_CALL(*vault_io, flush_to_storage(serialized_key2));
    }
    EXPECT_FALSE(vault_keys_container->store_key(key2));
    ASSERT_TRUE(vault_keys_container->get_number_of_keys() == 2);

    {
      InSequence dummy;
      ISerialized_object *null_serialized_object = NULL;
      // flush to keyring
      EXPECT_CALL(*vault_io, get_serializer())
        .WillOnce(Return(mock_serializer));
      EXPECT_CALL(*mock_serializer, serialize(_,sample_key,REMOVE_KEY))
        .WillOnce(Return(null_serialized_object));
      EXPECT_CALL(*logger, log(MY_ERROR_LEVEL, StrEq("Could not flush keys to keyring")));
    }

    EXPECT_TRUE(vault_keys_container->remove_key(sample_key));
    ASSERT_TRUE(vault_keys_container->get_number_of_keys() == 2);

    delete logger;
    delete mock_serializer;
  }

  TEST_F(Vault_keys_container_with_mocked_io_test, Store2KeysAndRemoveThem)
  {
    vault_io = new Mock_vault_io();
    Mock_logger *logger = new Mock_logger();
    vault_keys_container = new Vault_keys_container(logger);
    expect_calls_on_init();
    EXPECT_FALSE(vault_keys_container->init(vault_io, credential_file_url));
    ASSERT_TRUE(vault_keys_container->get_number_of_keys() == 0);
    Mock_serializer *mock_serializer = new Mock_serializer;

    sample_key->set_key_operation(STORE_KEY);
    Vault_key *serialized_sample_key = new Vault_key(*sample_key);

    {
      InSequence dummy;
      // flush to keyring
      EXPECT_CALL(*vault_io, get_serializer())
        .WillOnce(Return(mock_serializer));
      EXPECT_CALL(*mock_serializer, serialize(_,sample_key,STORE_KEY))
        .WillOnce(Return(serialized_sample_key));
      EXPECT_CALL(*vault_io, flush_to_storage(serialized_sample_key));
    }
    EXPECT_FALSE(vault_keys_container->store_key(sample_key));
    ASSERT_TRUE(vault_keys_container->get_number_of_keys() == 1);

    std::string key_data2("Robi2");
    Vault_key *key2 = new Vault_key((uuid+"Roberts_key2").c_str(), "AES", "Robert", key_data2.c_str(), key_data2.length());
    key2->set_key_operation(STORE_KEY);

    Vault_key *serialized_key2 = new Vault_key(*key2);

    {
      InSequence dummy;
      // flush to keyring
      EXPECT_CALL(*vault_io, get_serializer())
        .WillOnce(Return(mock_serializer));
      EXPECT_CALL(*mock_serializer, serialize(_,key2,STORE_KEY))
        .WillOnce(Return(serialized_key2));
      EXPECT_CALL(*vault_io, flush_to_storage(serialized_key2));
    }
    EXPECT_FALSE(vault_keys_container->store_key(key2));
    ASSERT_TRUE(vault_keys_container->get_number_of_keys() == 2);

    sample_key->set_key_operation(REMOVE_KEY);

    Vault_key *serialized_sample_key_to_remove = new Vault_key(*sample_key);

    {
      InSequence dummy;
      // flush to keyring
      EXPECT_CALL(*vault_io, get_serializer())
        .WillOnce(Return(mock_serializer));
      EXPECT_CALL(*mock_serializer, serialize(_,sample_key,REMOVE_KEY))
        .WillOnce(Return(serialized_sample_key_to_remove));
      EXPECT_CALL(*vault_io, flush_to_storage(serialized_sample_key_to_remove));
    }

    sample_key->set_key_operation(REMOVE_KEY);
    EXPECT_FALSE(vault_keys_container->remove_key(sample_key));
    ASSERT_TRUE(vault_keys_container->get_number_of_keys() == 1);

    key2->set_key_operation(REMOVE_KEY);
    Vault_key *serialized_key2_to_remove = new Vault_key(*key2);

    {
      InSequence dummy;

      // flush to keyring
      EXPECT_CALL(*vault_io, get_serializer())
        .WillOnce(Return(mock_serializer));
      EXPECT_CALL(*mock_serializer, serialize(_,key2,REMOVE_KEY))
        .WillOnce(Return(serialized_key2_to_remove));
      EXPECT_CALL(*vault_io, flush_to_storage(serialized_key2_to_remove));
    }

    EXPECT_FALSE(vault_keys_container->remove_key(key2));
    ASSERT_TRUE(vault_keys_container->get_number_of_keys() == 0);

    delete logger;
    delete mock_serializer;
  }

  TEST_F(Vault_keys_container_with_mocked_io_test, ErrorFromRetriveKeyTypeDuringFetch)
  {
    vault_io = new Mock_vault_io();
    Mock_logger *logger = new Mock_logger();
    vault_keys_container = new Vault_keys_container(logger);

    Vault_key *key_from_list = new Vault_key((uuid+"key1").c_str(), NULL, "Robert", NULL, 0);

    EXPECT_CALL(*vault_io, init(Pointee(StrEq(credential_file_url))))
      .WillOnce(Return(FALSE)); // init successfull
    EXPECT_CALL(*vault_io, get_serialized_object(_))
      .WillOnce(DoAll(SetArgPointee<0>(key_from_list), Return(FALSE)));
    EXPECT_CALL(*vault_io, has_next_serialized_object()).WillOnce(Return(FALSE)); // just one key

    EXPECT_FALSE(vault_keys_container->init(vault_io, credential_file_url));
    EXPECT_EQ(vault_keys_container->get_number_of_keys(), static_cast<uint>(1));

    Vault_key key_fetched((uuid+"key1").c_str(), NULL, "Robert", NULL, 0);
    ASSERT_TRUE(key_fetched.get_key_data() == NULL);

    EXPECT_CALL(*vault_io, retrieve_key_type_and_data(_))
      .WillOnce(Return(TRUE));
    // retrieving key for the first time - key's data and type is kept only in Vault
    // need to fetch them on container's fetch operation
    EXPECT_EQ(vault_keys_container->fetch_key(&key_fetched), (IKey*)0);

    delete logger;
    delete sample_key;
  }

  void set_data(IKey *key)
  {
    std::string type("AES");
    key->set_key_type(&type);
    uchar *data = new uchar[2];
    data[0] = 'a';
    data[1] = 'b';
    key->set_key_data(data, 2);
  }

  TEST_F(Vault_keys_container_with_mocked_io_test, CheckThatRetriveKeyTypeIsNotCalledForSecondFetch)
  {
    vault_io = new Mock_vault_io();
    Mock_logger *logger = new Mock_logger();
    vault_keys_container = new Vault_keys_container(logger);

    Vault_key *key_from_list = new Vault_key((uuid+"key1").c_str(), NULL, "Robert", NULL, 0);

    EXPECT_CALL(*vault_io, init(Pointee(StrEq(credential_file_url))))
      .WillOnce(Return(FALSE)); // init successfull
    EXPECT_CALL(*vault_io, get_serialized_object(_))
      .WillOnce(DoAll(SetArgPointee<0>(key_from_list), Return(FALSE)));
    EXPECT_CALL(*vault_io, has_next_serialized_object()).WillOnce(Return(FALSE)); // just one key

    EXPECT_FALSE(vault_keys_container->init(vault_io, credential_file_url));
    EXPECT_EQ(vault_keys_container->get_number_of_keys(), static_cast<uint>(1));

    Vault_key key_to_fetch((uuid+"key1").c_str(), NULL, "Robert", NULL, 0);
    ASSERT_TRUE(key_to_fetch.get_key_data() == NULL);
    IKey *key_fetched_from_keyring;

    EXPECT_CALL(*vault_io, retrieve_key_type_and_data(_))
      .WillOnce(DoAll(WithArgs<0>(Invoke(set_data)), Return(FALSE)));
    // retrieving key for the first time - key's data and type is kept only in Vault
    // need to fetch them on container's fetch operation
    key_fetched_from_keyring = vault_keys_container->fetch_key(&key_to_fetch);
      
    // When we call fetch_key for the 2nd time - key's data and type should be already cached
    // thus the second call should not call retrieve_key_type_and_data
    Vault_key key_to_re_fetch((uuid+"key1").c_str(), NULL, "Robert", NULL, 0);
    EXPECT_CALL(*vault_io, retrieve_key_type_and_data(_)).Times(0);
    key_fetched_from_keyring = vault_keys_container->fetch_key(&key_to_re_fetch);

    ASSERT_TRUE(key_fetched_from_keyring != NULL);
    std::string expected_key_signature = get_key_signature(uuid,"key1","Robert");
    EXPECT_STREQ(key_fetched_from_keyring->get_key_signature()->c_str(), expected_key_signature.c_str());
    EXPECT_EQ(memcmp("ab", reinterpret_cast<const char*>(key_fetched_from_keyring->get_key_data()),
                     key_fetched_from_keyring->get_key_data_size()), 0);

    my_free(key_to_fetch.release_key_data());
    my_free(key_to_re_fetch.release_key_data());
    delete logger;
    delete sample_key; // unused in this test
  }
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::InitGoogleMock(&argc, argv);
  MY_INIT(argv[0]);
  my_testing::setup_server_for_unit_tests();

  //create unique secret mount point for this test suite
  curl_global_init(CURL_GLOBAL_DEFAULT);
  CURL *curl = curl_easy_init();
  if (curl == NULL)
  {
    std::cout << "Could not initialize CURL session" << std::endl;
    curl_global_cleanup();
    return 1; 
  }
  BOOST_SCOPE_EXIT(&curl)
  {
    curl_easy_cleanup(curl);
    curl_global_cleanup();
  } BOOST_SCOPE_EXIT_END

  keyring__vault_keys_container_unittest::logger = new keyring::Mock_logger();
  keyring::Vault_mount vault_mount(curl, keyring__vault_keys_container_unittest::logger);
  std::string mount_point_path=
      "cicd/" + keyring__vault_keys_container_unittest::uuid;

  if (generate_credential_file(
          keyring__vault_keys_container_unittest::credential_file_url,
          CORRECT, mount_point_path))
  {
    std::cout << "Could not generate credential file" << std::endl;
    return 2; 
  }
  if (vault_mount.init(
          &keyring__vault_keys_container_unittest::credential_file_url,
          &mount_point_path))
  {
    std::cout << "Could not initialize Vault_mount" << std::endl;
    return 3; 
  }
  if (vault_mount.mount_secret_backend())
  {
    std::cout << "Could not mount secret backend" << std::endl;
    return 4;
  }

  int ret= RUN_ALL_TESTS();

  //remove unique secret mount point
  if (vault_mount.unmount_secret_backend())
  {
    std::cout << "Could not unmount secret backend" << std::endl;
  }
  delete keyring__vault_keys_container_unittest::logger;

  my_testing::teardown_server_for_unit_tests();
  return ret;
}
