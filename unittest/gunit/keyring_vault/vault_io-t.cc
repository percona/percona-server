#include <my_global.h>
#include <gtest/gtest.h>
#include <fstream>
#include "mock_logger.h"
#include "vault_io.h"
#include "incorrect_vault_key.h"
#include <string.h>
#include <curl/curl.h>
#include "uuid.h"

#if defined(HAVE_PSI_INTERFACE)
namespace keyring
{
  PSI_memory_key key_memory_KEYRING = PSI_NOT_INSTRUMENTED;
}
#endif

namespace keyring__vault_io_unittest
{
  using namespace keyring;

  using ::testing::Return;
  using ::testing::StrEq;
  using ::testing::_;
  using ::testing::SetArgPointee;

  static std::string uuid;

  class Vault_io_test : public ::testing::Test
  {
  protected:
    virtual void SetUp()
    {
      credential_file_url = "./keyring_vault.conf";
      logger= new Mock_logger();
      vault_curl = new Vault_curl(logger);
      vault_parser = new Vault_parser(logger);
    }

    virtual void TearDown()
    {
      delete logger;
    }

  protected:
    ILogger *logger;
    IVault_curl *vault_curl;
    IVault_parser *vault_parser;
    std::string credential_file_url;
  };
/*
  TEST_F(Vault_io_test, InitWithNotExisitingCredentialFile)
  {
    std::string credential_file_name("./some_funny_name");
    Vault_io vault_io(logger, vault_curl, vault_parser);
    remove(credential_file_name.c_str());
    EXPECT_CALL(*(reinterpret_cast<Mock_logger*>(logger)),
      log(MY_ERROR_LEVEL, StrEq("Could not open file with credentials.")));
    EXPECT_TRUE(vault_io.init(&credential_file_name));

    remove(credential_file_name.c_str());
  }

  TEST_F(Vault_io_test, InitWithInvalidToken)
  {
    Vault_io vault_io(logger, vault_curl, vault_parser);
    std::string conf_with_invalid_token("./invalid_token.conf");

    std::remove(conf_with_invalid_token.c_str());
    std::ofstream my_file;
    my_file.open(conf_with_invalid_token.c_str());
    my_file << "vault_url = https://127.0.0.1:8600" << std::endl;
    my_file << "secret_mount_point = secret" << std::endl;
    my_file << "token = 123-123-123" << std::endl;
    my_file << "vault_ca = /home/rob/vault_certs/vault_ca.crt";
    my_file.close();

    EXPECT_FALSE(vault_io.init(&conf_with_invalid_token));

    EXPECT_CALL(*(reinterpret_cast<Mock_logger*>(logger)),
      log(MY_ERROR_LEVEL, StrEq("Could not retrieve list of keys from Vault. "
          "Vault has returned the following error(s): [\"permission denied\"]")));
    ISerialized_object *serialized_keys= NULL;
    EXPECT_TRUE(vault_io.get_serialized_object(&serialized_keys));
  }
*/

  TEST_F(Vault_io_test, GetSerializedObjectWithTwoKeys)
  {
    Vault_io vault_io(logger, vault_curl, vault_parser);

    EXPECT_FALSE(vault_io.init(&credential_file_url));

    // First add two keys into Vault
    Vault_key key1((uuid+"key1").c_str(), "AES", "Arczi", "Artur", 5);
    EXPECT_TRUE(*key1.get_key_signature() == get_key_signature(uuid,"key1","Arczi"));
    key1.set_key_operation(STORE_KEY);
    EXPECT_FALSE(vault_io.flush_to_storage(&key1));
    Vault_key key2((uuid+"key2").c_str(), "AES", "Kamil", "Kami", 4);
    EXPECT_TRUE(*key2.get_key_signature() == get_key_signature(uuid,"key2","Kamil"));
    key2.set_key_operation(STORE_KEY);
    EXPECT_FALSE(vault_io.flush_to_storage(&key2));
    // *****

    // Now fetch two keys with separate Vault_io
    ISerialized_object *serialized_keys= NULL;
    EXPECT_FALSE(vault_io.get_serialized_object(&serialized_keys));
    IKey *key1_loaded= NULL;
    ASSERT_TRUE(serialized_keys != NULL);
    EXPECT_TRUE(serialized_keys->has_next_key());
    serialized_keys->get_next_key(&key1_loaded);
    EXPECT_TRUE(*key1_loaded->get_key_signature() == get_key_signature(uuid,"key1","Arczi") ||
                *key1_loaded->get_key_signature() == get_key_signature(uuid,"key2","Kamil"));
    IKey *key2_loaded= NULL;
    delete key1_loaded;
    EXPECT_TRUE(serialized_keys->has_next_key());
    serialized_keys->get_next_key(&key2_loaded);
    EXPECT_TRUE(*key2_loaded->get_key_signature() == get_key_signature(uuid,"key2","Kamil") ||
                *key2_loaded->get_key_signature() == get_key_signature(uuid,"key1","Arczi"));
    delete key2_loaded;
    EXPECT_FALSE(serialized_keys->has_next_key());
    delete serialized_keys;

    // Now remove the keys
    Vault_key key1_to_remove(key1);
    key1_to_remove.set_key_operation(REMOVE_KEY);
    EXPECT_FALSE(vault_io.flush_to_storage(&key1_to_remove));
    Vault_key key2_to_remove(key2);
    key2_to_remove.set_key_operation(REMOVE_KEY);
    EXPECT_FALSE(vault_io.flush_to_storage(&key2_to_remove));
  }

  TEST_F(Vault_io_test, GetSerializedObjectWithTwoKeysWithDifferentVaultIO)
  {
    Vault_io vault_io_for_storing(logger, vault_curl, vault_parser);

    EXPECT_FALSE(vault_io_for_storing.init(&credential_file_url));

    // First add two keys into Vault
    Vault_key key1((uuid+"key1").c_str(), "AES", "Robert", "Robi", 4);
    EXPECT_TRUE(*key1.get_key_signature() == get_key_signature(uuid,"key1","Robert"));
    key1.set_key_operation(STORE_KEY);
    EXPECT_FALSE(vault_io_for_storing.flush_to_storage(&key1));
    Vault_key key2((uuid+"key2").c_str(), "AES", "Kamil", "Kami", 4);
    EXPECT_TRUE(*key2.get_key_signature() == get_key_signature(uuid,"key2","Kamil"));
    key2.set_key_operation(STORE_KEY);
    EXPECT_FALSE(vault_io_for_storing.flush_to_storage(&key2));
    // *****

    // Now fetch two keys with separate Vault_io
    Vault_curl *vault_curl2 = new Vault_curl(logger);
    Vault_parser *vault_parser2 = new Vault_parser(logger);
    Vault_io vault_io_for_fetching(logger, vault_curl2, vault_parser2);
    EXPECT_FALSE(vault_io_for_fetching.init(&credential_file_url));

    ISerialized_object *serialized_keys= NULL;
    EXPECT_FALSE(vault_io_for_fetching.get_serialized_object(&serialized_keys));
    IKey *key1_loaded= NULL;
    ASSERT_TRUE(serialized_keys != NULL);
    EXPECT_TRUE(serialized_keys->has_next_key());
    serialized_keys->get_next_key(&key1_loaded);
    EXPECT_TRUE(*key1_loaded->get_key_signature() == get_key_signature(uuid,"key1","Robert") ||
                *key1_loaded->get_key_signature() == get_key_signature(uuid,"key2","Kamil"));
    IKey *key2_loaded= NULL;
    delete key1_loaded;
    EXPECT_TRUE(serialized_keys->has_next_key());
    serialized_keys->get_next_key(&key2_loaded);
    EXPECT_TRUE(*key2_loaded->get_key_signature() == get_key_signature(uuid,"key2","Kamil") ||
                *key2_loaded->get_key_signature() == get_key_signature(uuid,"key1","Robert"));
    delete key2_loaded;
    EXPECT_FALSE(serialized_keys->has_next_key());
    delete serialized_keys;

    // Now remove the keys
    Vault_key key1_to_remove(key1);
    key1_to_remove.set_key_operation(REMOVE_KEY);
    EXPECT_FALSE(vault_io_for_storing.flush_to_storage(&key1_to_remove));
    Vault_key key2_to_remove(key2);
    key2_to_remove.set_key_operation(REMOVE_KEY);
    EXPECT_FALSE(vault_io_for_storing.flush_to_storage(&key2_to_remove));
  } 
  TEST_F(Vault_io_test, InitWithIncorrectKeyInVault)
  {
    Vault_io vault_io_for_storing(logger, vault_curl, vault_parser);

    EXPECT_FALSE(vault_io_for_storing.init(&credential_file_url));

    // First add two keys into Vault
    Vault_key key1((uuid+"key1").c_str(), "AES", "Robert", "Robi", 4);
    EXPECT_TRUE(*key1.get_key_signature() == get_key_signature(uuid,"key1","Robert"));
    key1.set_key_operation(STORE_KEY);
    EXPECT_FALSE(vault_io_for_storing.flush_to_storage(&key1));
    Vault_key key2((uuid+"key2").c_str(), "AES", "Kamil", "Kami", 4);
    EXPECT_TRUE(*key2.get_key_signature() == get_key_signature(uuid,"key2","Kamil"));
    key2.set_key_operation(STORE_KEY);
    EXPECT_FALSE(vault_io_for_storing.flush_to_storage(&key2));
    Incorrect_vault_key incorrect_key("key3","RSA","Artur", "Arczi", 5);
    incorrect_key.set_key_operation(STORE_KEY);
    incorrect_key.add_to_key_id_length = 14;
    // make sure signature is incorrect
    EXPECT_STREQ(incorrect_key.get_key_signature()->c_str(), "18_key35_Artur");
    EXPECT_FALSE(vault_io_for_storing.flush_to_storage(&incorrect_key));
    // *****

    // Now fetch two keys with separate Vault_io - incorrect key should have been ignored
    Vault_curl *vault_curl2 = new Vault_curl(logger);
    Vault_parser *vault_parser2 = new Vault_parser(logger);
    Vault_io vault_io_for_fetching(logger, vault_curl2, vault_parser2);

   EXPECT_CALL(*(reinterpret_cast<Mock_logger*>(logger)),
      log(MY_WARNING_LEVEL, StrEq("Could not parse key's signature, skipping the key.")));

    EXPECT_FALSE(vault_io_for_fetching.init(&credential_file_url));

    ISerialized_object *serialized_keys= NULL;
    EXPECT_FALSE(vault_io_for_fetching.get_serialized_object(&serialized_keys));
    IKey *key1_loaded= NULL;
    ASSERT_TRUE(serialized_keys != NULL);
    EXPECT_TRUE(serialized_keys->has_next_key());
    serialized_keys->get_next_key(&key1_loaded);
    EXPECT_TRUE(*key1_loaded->get_key_signature() == get_key_signature(uuid,"key1","Robert") ||
                *key1_loaded->get_key_signature() == get_key_signature(uuid,"key2","Kamil"));
    IKey *key2_loaded= NULL;
    delete key1_loaded;
    EXPECT_TRUE(serialized_keys->has_next_key());
    serialized_keys->get_next_key(&key2_loaded);
    EXPECT_TRUE(*key2_loaded->get_key_signature() == get_key_signature(uuid,"key2","Kamil") ||
                *key2_loaded->get_key_signature() == get_key_signature(uuid,"key1","Robert"));
    delete key2_loaded;
    EXPECT_FALSE(serialized_keys->has_next_key());
    delete serialized_keys;

    // Now remove the keys
    Vault_key key1_to_remove(key1);
    key1_to_remove.set_key_operation(REMOVE_KEY);
    EXPECT_FALSE(vault_io_for_storing.flush_to_storage(&key1_to_remove));
    Vault_key key2_to_remove(key2);
    key2_to_remove.set_key_operation(REMOVE_KEY);
    EXPECT_FALSE(vault_io_for_storing.flush_to_storage(&key2_to_remove));
    Incorrect_vault_key incorrect_key_to_remove(incorrect_key);
    incorrect_key_to_remove.set_key_operation(REMOVE_KEY);
    EXPECT_FALSE(vault_io_for_storing.flush_to_storage(&incorrect_key_to_remove));
  }

  TEST_F(Vault_io_test, RetrieveKeyTypeAndValue)
  {
    Vault_io vault_io(logger, vault_curl, vault_parser);
    EXPECT_FALSE(vault_io.init(&credential_file_url));

    Vault_key key_to_store((uuid+"key1").c_str(), "AES", "rob", "Robi", 4);
    key_to_store.set_key_operation(STORE_KEY);
    EXPECT_FALSE(vault_io.flush_to_storage(&key_to_store));

    Vault_key key((uuid+"key1").c_str(), NULL, "rob", NULL, 0);
    EXPECT_FALSE(vault_io.retrieve_key_type_and_data(&key));
    EXPECT_TRUE(*key.get_key_signature() == get_key_signature(uuid,"key1","rob"));
    ASSERT_TRUE(memcmp(key.get_key_data(), "Robi", key.get_key_data_size()) == 0);
    EXPECT_STREQ("AES", key.get_key_type()->c_str());

    Vault_key key_to_remove((uuid+"key1").c_str(), NULL, "rob", NULL, 0);
    key_to_remove.set_key_operation(REMOVE_KEY);
    EXPECT_FALSE(vault_io.flush_to_storage(&key_to_remove));
  }

  TEST_F(Vault_io_test, FlushAndRemoveSingleKey)
  {
    Vault_io vault_io(logger, vault_curl, vault_parser);
    EXPECT_FALSE(vault_io.init(&credential_file_url));
    Vault_key key((uuid+"key1").c_str(), "AES", "rob", "Robi", 4);
    key.set_key_operation(STORE_KEY);
    EXPECT_FALSE(vault_io.flush_to_storage(&key));
    Vault_key key_to_remove(key);
    key_to_remove.set_key_operation(REMOVE_KEY);
    EXPECT_FALSE(vault_io.flush_to_storage(&key_to_remove));
  }

  TEST_F(Vault_io_test, FlushKeyRetrieveDeleteInit)
  {
    Vault_io vault_io(logger, vault_curl, vault_parser);
    EXPECT_FALSE(vault_io.init(&credential_file_url));
    Vault_key key((uuid+"key1").c_str(), "AES", "rob", "Robi", 4);
    key.set_key_operation(STORE_KEY);
    EXPECT_FALSE(vault_io.flush_to_storage(&key));
    Vault_key key1_id((uuid+"key1").c_str(), NULL, "rob", NULL, 0);
    EXPECT_FALSE(vault_io.retrieve_key_type_and_data(&key1_id));
    EXPECT_TRUE(*key1_id.get_key_signature() == get_key_signature(uuid,"key1","rob"));
    ASSERT_TRUE(memcmp(key1_id.get_key_data(), "Robi", key1_id.get_key_data_size()) == 0);
    EXPECT_STREQ("AES", key1_id.get_key_type()->c_str());

    Vault_key key_to_remove(key);
    key_to_remove.set_key_operation(REMOVE_KEY);
    EXPECT_FALSE(vault_io.flush_to_storage(&key_to_remove));

    Vault_curl *vault_curl2 = new Vault_curl(logger);
    Vault_parser *vault_parser2 = new Vault_parser(logger);
    Vault_io vault_io2(logger, vault_curl2, vault_parser2);
    EXPECT_FALSE(vault_io2.init(&credential_file_url));
    ISerialized_object *serialized_keys= NULL;
    EXPECT_FALSE(vault_io2.get_serialized_object(&serialized_keys));
    ASSERT_TRUE(serialized_keys == NULL); // no keys
  }

  class Mock_vault_curl : public IVault_curl
  {
  public:
    MOCK_METHOD1(init, bool(const Vault_credentials &vault_credentials));
    MOCK_METHOD1(list_keys, bool(Secure_string *response));
    MOCK_METHOD2(write_key, bool(const Vault_key &key, Secure_string *response));
    MOCK_METHOD2(read_key, bool(const Vault_key &key, Secure_string *response));
    MOCK_METHOD2(delete_key, bool(const Vault_key &key, Secure_string *response));
  };

  TEST_F(Vault_io_test, ErrorFromVaultCurlOnVaultIOInit)
  {
    delete vault_curl;
    Mock_vault_curl *mock_curl = new Mock_vault_curl();
    Vault_io vault_io(logger, mock_curl, vault_parser);

    EXPECT_CALL(*mock_curl, init(_))
      .WillOnce(Return(true)); // init unsuccessful
    EXPECT_TRUE(vault_io.init(&credential_file_url));
  }

  TEST_F(Vault_io_test, ErrorFromVaultCurlOnListKeys)
  {
    delete vault_curl;
    Mock_vault_curl *mock_curl = new Mock_vault_curl();
    Vault_io vault_io(logger, mock_curl, vault_parser);

    EXPECT_CALL(*mock_curl, init(_))
      .WillOnce(Return(false)); // init successful
    EXPECT_FALSE(vault_io.init(&credential_file_url));

    ISerialized_object *serialized_object;

    EXPECT_CALL(*mock_curl, list_keys(_))
      .WillOnce(Return(true)); // failed to list keys
    EXPECT_CALL(*(reinterpret_cast<Mock_logger*>(logger)),
      log(MY_ERROR_LEVEL, StrEq("Could not retrieve list of keys from Vault.")));

    EXPECT_TRUE(vault_io.get_serialized_object(&serialized_object));
    EXPECT_EQ(serialized_object, reinterpret_cast<ISerialized_object*>(NULL));
  }

  TEST_F(Vault_io_test, ErrorsFromVaultInVaultsResponseOnListKeys)
  {
    delete vault_curl;
    Mock_vault_curl *mock_curl = new Mock_vault_curl();
    Vault_io vault_io(logger, mock_curl, vault_parser);

    EXPECT_CALL(*mock_curl, init(_))
      .WillOnce(Return(false)); // init successful
    EXPECT_FALSE(vault_io.init(&credential_file_url));

    ISerialized_object *serialized_object;
    Secure_string vault_response("{ errors: [\"list is broken\"] }"); 

    EXPECT_CALL(*mock_curl, list_keys(_))
      .WillOnce(DoAll(SetArgPointee<0>(vault_response), Return(true)));
      
    EXPECT_CALL(*(reinterpret_cast<Mock_logger*>(logger)),
      log(MY_ERROR_LEVEL, StrEq("Could not retrieve list of keys from Vault. Vault has returned the following error(s): [\"list is broken\"]")));

    EXPECT_TRUE(vault_io.get_serialized_object(&serialized_object));
    EXPECT_EQ(serialized_object, reinterpret_cast<ISerialized_object*>(NULL));

    vault_response = "{errors: [\"list is broken\", \"and some other error\"]}"; 

    EXPECT_CALL(*mock_curl, list_keys(_))
      .WillOnce(DoAll(SetArgPointee<0>(vault_response), Return(true)));
      
    EXPECT_CALL(*(reinterpret_cast<Mock_logger*>(logger)),
      log(MY_ERROR_LEVEL, StrEq("Could not retrieve list of keys from Vault. Vault has returned the following error(s): [\"list is broken\", \"and some other error\"]")));

    EXPECT_TRUE(vault_io.get_serialized_object(&serialized_object));
    EXPECT_EQ(serialized_object, reinterpret_cast<ISerialized_object*>(NULL));

    vault_response = "{ errors: [\"list is broken\",\n\"and some other error\"\n] }"; 

    EXPECT_CALL(*mock_curl, list_keys(_))
      .WillOnce(DoAll(SetArgPointee<0>(vault_response), Return(true)));
      
    EXPECT_CALL(*(reinterpret_cast<Mock_logger*>(logger)),
      log(MY_ERROR_LEVEL, StrEq("Could not retrieve list of keys from Vault. Vault has returned the following error(s): [\"list is broken\",\"and some other error\"]")));

    EXPECT_TRUE(vault_io.get_serialized_object(&serialized_object));
    EXPECT_EQ(serialized_object, reinterpret_cast<ISerialized_object*>(NULL));
  }

  TEST_F(Vault_io_test, ErrorsFromVaultCurlOnReadKey)
  {
    delete vault_curl;
    Mock_vault_curl *mock_curl = new Mock_vault_curl();
    Vault_io vault_io(logger, mock_curl, vault_parser);

    EXPECT_CALL(*mock_curl, init(_))
      .WillOnce(Return(false)); // init successful
    EXPECT_FALSE(vault_io.init(&credential_file_url));

    IKey *key = NULL;

    EXPECT_CALL(*mock_curl, read_key(_, _))
      .WillOnce(Return(true));
    EXPECT_CALL(*(reinterpret_cast<Mock_logger*>(logger)),
      log(MY_ERROR_LEVEL, StrEq("Could not read key from Vault.")));
    EXPECT_TRUE(vault_io.retrieve_key_type_and_data(key));
  }

  TEST_F(Vault_io_test, ErrorsFromVaultInVaultsCurlResponseOnReadKey)
  {
    delete vault_curl;
    Mock_vault_curl *mock_curl = new Mock_vault_curl();
    Vault_io vault_io(logger, mock_curl, vault_parser);

    EXPECT_CALL(*mock_curl, init(_))
      .WillOnce(Return(false)); // init successful
    EXPECT_FALSE(vault_io.init(&credential_file_url));

    IKey *key = NULL;
    Secure_string vault_response("{ errors: [\"Cannot read this stuff\"] }"); 

    EXPECT_CALL(*mock_curl, read_key(_, _))
      .WillOnce(DoAll(SetArgPointee<1>(vault_response), Return(true)));
    EXPECT_CALL(*(reinterpret_cast<Mock_logger*>(logger)),
      log(MY_ERROR_LEVEL, StrEq("Could not read key from Vault. Vault has returned the following error(s):"
                                " [\"Cannot read this stuff\"]")));
    EXPECT_TRUE(vault_io.retrieve_key_type_and_data(key));
  }

  TEST_F(Vault_io_test, ErrorsFromVaultCurlOnDeleteKey)
  {
    delete vault_curl;
    Mock_vault_curl *mock_curl = new Mock_vault_curl();
    Vault_io vault_io(logger, mock_curl, vault_parser);

    EXPECT_CALL(*mock_curl, init(_))
      .WillOnce(Return(false)); // init successful
    EXPECT_FALSE(vault_io.init(&credential_file_url));

    Vault_key key((uuid+"key1").c_str(), "AES", "Arczi", "Artur", 5);
    key.set_key_operation(REMOVE_KEY);

    EXPECT_CALL(*mock_curl, delete_key(_,_))
      .WillOnce(Return(true));
    EXPECT_CALL(*(reinterpret_cast<Mock_logger*>(logger)),
      log(MY_ERROR_LEVEL, StrEq("Could not delete key from Vault.")));
    EXPECT_TRUE(vault_io.flush_to_storage(&key));
  }

  TEST_F(Vault_io_test, ErrorsFromVaultInVaultsCurlResponseOnDeleteKey)
  {
    delete vault_curl;
    Mock_vault_curl *mock_curl = new Mock_vault_curl();
    Vault_io vault_io(logger, mock_curl, vault_parser);

    EXPECT_CALL(*mock_curl, init(_))
      .WillOnce(Return(false)); // init successful
    EXPECT_FALSE(vault_io.init(&credential_file_url));

    Vault_key key((uuid+"key1").c_str(), "AES", "Arczi", "Artur", 5);
    key.set_key_operation(REMOVE_KEY);
    Secure_string vault_response("{ errors: [\"Cannot delete this stuff\"] }"); 

    EXPECT_CALL(*mock_curl, delete_key(_, _))
      .WillOnce(DoAll(SetArgPointee<1>(vault_response), Return(false)));
    EXPECT_CALL(*(reinterpret_cast<Mock_logger*>(logger)),
      log(MY_ERROR_LEVEL, StrEq("Could not delete key from Vault. Vault has returned the following error(s):"
                                " [\"Cannot delete this stuff\"]")));
    EXPECT_TRUE(vault_io.flush_to_storage(&key));
  }

  TEST_F(Vault_io_test, ErrorsFromVaultCurlOnWriteKey)
  {
    delete vault_curl;
    Mock_vault_curl *mock_curl = new Mock_vault_curl();
    Vault_io vault_io(logger, mock_curl, vault_parser);

    EXPECT_CALL(*mock_curl, init(_))
      .WillOnce(Return(false)); // init successful
    EXPECT_FALSE(vault_io.init(&credential_file_url));

    Vault_key key((uuid+"key1").c_str(), "AES", "Arczi", "Artur", 5);
    key.set_key_operation(STORE_KEY);

    EXPECT_CALL(*mock_curl, write_key(_,_))
      .WillOnce(Return(true));
    EXPECT_CALL(*(reinterpret_cast<Mock_logger*>(logger)),
      log(MY_ERROR_LEVEL, StrEq("Could not write key to Vault.")));
    EXPECT_TRUE(vault_io.flush_to_storage(&key));
  }

  TEST_F(Vault_io_test, ErrorsFromVaultInVaultsCurlResponseOnWriteKey)
  {
    delete vault_curl;
    Mock_vault_curl *mock_curl = new Mock_vault_curl();
    Vault_io vault_io(logger, mock_curl, vault_parser);

    EXPECT_CALL(*mock_curl, init(_))
      .WillOnce(Return(false)); // init successful
    EXPECT_FALSE(vault_io.init(&credential_file_url));

    Vault_key key((uuid+"key1").c_str(), "AES", "Arczi", "Artur", 5);
    key.set_key_operation(STORE_KEY);
    Secure_string vault_response("{ errors: [\"Cannot write this stuff\"] }"); 

    EXPECT_CALL(*mock_curl, write_key(_, _))
      .WillOnce(DoAll(SetArgPointee<1>(vault_response), Return(false)));
    EXPECT_CALL(*(reinterpret_cast<Mock_logger*>(logger)),
      log(MY_ERROR_LEVEL, StrEq("Could not write key to Vault. Vault has returned the following error(s):"
                                " [\"Cannot write this stuff\"]")));
    EXPECT_TRUE(vault_io.flush_to_storage(&key));
  }

  class Mock_vault_parser : public IVault_parser
  {
  public:
    MOCK_METHOD2(parse_keys, bool(const Secure_string &payload, Vault_keys_list *keys));
    MOCK_METHOD2(parse_key_data, bool(const Secure_string &payload, IKey *key));
    MOCK_METHOD2(parse_key_signature, bool(const Secure_string &key_signature, Secure_string key_parameters[2]));
    MOCK_METHOD2(parse_errors, bool(const Secure_string &payload, Secure_string *errors));
  };

  TEST_F(Vault_io_test, ErrorFromParseKeysOnGetSerializedObject)
  {
    delete vault_parser;

    Mock_vault_parser *mock_vault_parser = new Mock_vault_parser;
    Vault_io vault_io(logger, vault_curl, mock_vault_parser);

    EXPECT_FALSE(vault_io.init(&credential_file_url));

    // First add two keys into Vault
    Vault_key key1((uuid+"key1").c_str(), "AES", "Arczi", "Artur", 5);
    EXPECT_TRUE(*key1.get_key_signature() == get_key_signature(uuid,"key1","Arczi"));
    key1.set_key_operation(STORE_KEY);
    EXPECT_FALSE(vault_io.flush_to_storage(&key1));
    Vault_key key2((uuid+"key2").c_str(), "AES", "Kamil", "Kami", 4);
    EXPECT_TRUE(*key2.get_key_signature() == get_key_signature(uuid,"key2","Kamil"));
    key2.set_key_operation(STORE_KEY);
    EXPECT_FALSE(vault_io.flush_to_storage(&key2));
    // *****

    EXPECT_CALL(*mock_vault_parser, parse_keys(_, _))
      .WillOnce(Return(true));
    EXPECT_CALL(*(reinterpret_cast<Mock_logger*>(logger)),
      log(MY_ERROR_LEVEL, StrEq("Could not retrieve list of keys from Vault.")));

    ISerialized_object *serialized_keys= NULL;
    EXPECT_TRUE(vault_io.get_serialized_object(&serialized_keys));

    ASSERT_TRUE(serialized_keys == NULL);

    // Now remove the keys
    Vault_key key1_to_remove(key1);
    key1_to_remove.set_key_operation(REMOVE_KEY);
    EXPECT_FALSE(vault_io.flush_to_storage(&key1_to_remove));
    Vault_key key2_to_remove(key2);
    key2_to_remove.set_key_operation(REMOVE_KEY);
    EXPECT_FALSE(vault_io.flush_to_storage(&key2_to_remove));
  }

  TEST_F(Vault_io_test, ErrorFromParseKeyDataOnRetrieveKeyTypeAndValue)
  {
    delete vault_parser;
    Mock_vault_parser *mock_vault_parser = new Mock_vault_parser;
    Vault_io vault_io(logger, vault_curl, mock_vault_parser);
    EXPECT_FALSE(vault_io.init(&credential_file_url));

    Vault_key key_to_store((uuid+"key1").c_str(), "AES", "rob", "Robi", 4);
    key_to_store.set_key_operation(STORE_KEY);
    EXPECT_FALSE(vault_io.flush_to_storage(&key_to_store));

    Vault_key key((uuid+"key1").c_str(), NULL, "rob", NULL, 0);
    EXPECT_CALL(*mock_vault_parser, parse_key_data(_, &key))
      .WillOnce(Return(true));
    EXPECT_CALL(*mock_vault_parser, parse_errors(_, _))
      .WillOnce(Return(false));
    EXPECT_CALL(*(reinterpret_cast<Mock_logger*>(logger)),
      log(MY_ERROR_LEVEL, StrEq("Could not read key from Vault.")));
    EXPECT_TRUE(vault_io.retrieve_key_type_and_data(&key));

    Vault_key key_to_remove((uuid+"key1").c_str(), NULL, "rob", NULL, 0);
    key_to_remove.set_key_operation(REMOVE_KEY);
    EXPECT_FALSE(vault_io.flush_to_storage(&key_to_remove));
  }

  TEST_F(Vault_io_test, ErrorFromParseKeyDataAndParseErrorsOnRetrieveKeyTypeAndValue)
  {
    delete vault_parser;
    Mock_vault_parser *mock_vault_parser = new Mock_vault_parser;
    Vault_io vault_io(logger, vault_curl, mock_vault_parser);
    EXPECT_FALSE(vault_io.init(&credential_file_url));

    Vault_key key_to_store((uuid+"key1").c_str(), "AES", "rob", "Robi", 4);
    key_to_store.set_key_operation(STORE_KEY);
    EXPECT_FALSE(vault_io.flush_to_storage(&key_to_store));

    Vault_key key((uuid+"key1").c_str(), NULL, "rob", NULL, 0);
    EXPECT_CALL(*mock_vault_parser, parse_key_data(_, &key))
      .WillOnce(Return(true));
    EXPECT_CALL(*mock_vault_parser, parse_errors(_, _))
      .WillOnce(Return(true));
    EXPECT_CALL(*(reinterpret_cast<Mock_logger*>(logger)),
      log(MY_ERROR_LEVEL, StrEq("Could not read key from Vault. Error while parsing error messages")));
    EXPECT_TRUE(vault_io.retrieve_key_type_and_data(&key));

    Vault_key key_to_remove((uuid+"key1").c_str(), NULL, "rob", NULL, 0);
    key_to_remove.set_key_operation(REMOVE_KEY);
    EXPECT_FALSE(vault_io.flush_to_storage(&key_to_remove));
  }
} // namespace keyring__file_io_unittest

int main(int argc, char **argv) {
  curl_global_init(CURL_GLOBAL_DEFAULT);
  ::testing::InitGoogleTest(&argc, argv);
  keyring__vault_io_unittest::uuid = generate_uuid();
  int ret= RUN_ALL_TESTS();
  curl_global_cleanup();
  return ret;
}
