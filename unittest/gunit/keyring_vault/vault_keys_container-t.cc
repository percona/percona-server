#include <my_global.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <mysql/plugin_keyring.h>
#include "vault_keys_container.h"
#include "mock_logger.h"
#include "vault_io.h"
#include <fstream>
#include "i_serialized_object.h"
#include "uuid.h"
#include "generate_credential_file.h"
#include "test_utils.h"
#include "vault_mount.h"

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

  CURL *curl = NULL;
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
      vault_curl = new Vault_curl(logger, curl);
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
      log(MY_ERROR_LEVEL, StrEq("Could not read secret_mount_point from the configuration file.")));
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
  keyring__vault_keys_container_unittest::curl = curl_easy_init();
  if (keyring__vault_keys_container_unittest::curl == NULL)
  {
    std::cout << "Could not initialize CURL session" << std::endl;
    return 1; 
  }
  keyring__vault_keys_container_unittest::logger = new keyring::Mock_logger();
  keyring::Vault_mount vault_mount(keyring__vault_keys_container_unittest::curl, keyring__vault_keys_container_unittest::logger);

  if (generate_credential_file(keyring__vault_keys_container_unittest::credential_file_url, CORRECT, keyring__vault_keys_container_unittest::uuid))
  {
    std::cout << "Could not generate credential file" << std::endl;
    return 2; 
  }
  if (vault_mount.init(&keyring__vault_keys_container_unittest::credential_file_url, &keyring__vault_keys_container_unittest::uuid))
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
  curl_easy_cleanup(keyring__vault_keys_container_unittest::curl);
  curl_global_cleanup();
  delete keyring__vault_keys_container_unittest::logger;

  my_testing::teardown_server_for_unit_tests();
  return ret;
}
