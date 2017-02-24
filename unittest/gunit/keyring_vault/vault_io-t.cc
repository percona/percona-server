#include <my_global.h>
#include <gtest/gtest.h>
#include <fstream>
#include "mock_logger.h"
#include "vault_io.h"
#include <string.h>
#include <curl/curl.h>

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

  TEST_F(Vault_io_test, InitWithNotExisitingCredentialFile)
  {
    std::string credential_file_name("./some_funny_name");
    Vault_io vault_io(logger, vault_curl, vault_parser);
    remove(credential_file_name.c_str());
    EXPECT_CALL(*((Mock_logger *)logger),
      log(MY_ERROR_LEVEL, StrEq("Could not open file with credentials.")));
    EXPECT_EQ(vault_io.init(&credential_file_name), TRUE);

    remove(credential_file_name.c_str());
  }

  TEST_F(Vault_io_test, InitWithInvalidToken)
  {
    Vault_io vault_io(logger, vault_curl, vault_parser);
    std::string conf_with_invalid_token("invalid_token.conf");

    std::remove(conf_with_invalid_token.c_str());
    std::ofstream my_file;
    my_file.open(conf_with_invalid_token.c_str());
    my_file << "vault_url = https://127.0.0.1:8200" << std::endl;
    my_file << "secret_mount_point = secret" << std::endl;
    my_file << "token = 123-123-123" << std::endl;
    my_file << "vault_ca = ./vault_ca.crt";
    my_file.close();

    EXPECT_EQ(vault_io.init(&conf_with_invalid_token), FALSE);

    EXPECT_CALL(*((Mock_logger *)logger),
      log(MY_ERROR_LEVEL, StrEq("Could not retrieve list of keys from Vault. "
          "Vault has returned the following error(s): [\"permission denied\"]")));
    ISerialized_object *serialized_keys= NULL;
    EXPECT_EQ(vault_io.get_serialized_object(&serialized_keys), TRUE);

    std::remove(conf_with_invalid_token.c_str());
  }

/*
  //Tests InitWithNotExisitingVaultCA and InitWithOutVaultCA need Vault's CA cert to not
  //be trusted by machine.
 
  TEST_F(Vault_io_test, InitWithNotExisitingVaultCA)
  {
    Vault_io vault_io(logger, vault_curl, vault_parser);

    std::remove(credential_file_url.c_str());
    std::ofstream my_file;
    my_file.open(credential_file_url.c_str());
    my_file << "vault_url = https://127.0.0.1:8200" << std::endl;
    my_file << "secret_mount_point = secret" << std::endl;
    my_file << "token = " << correct_token << std::endl;
    my_file << "vault_ca = ./no_ca.crt";
    my_file.close();

    EXPECT_EQ(vault_io.init(&credential_file_url), FALSE);

    EXPECT_CALL(*((Mock_logger *)logger),
      log(MY_ERROR_LEVEL, StrEq("Could not retrieve list of keys from Vault.")));
    EXPECT_CALL(*((Mock_logger *)logger),
      log(MY_ERROR_LEVEL, StrEq("Curl returned this error code: 77 with error message "
                                ": error setting certificate verify locations:\n  CAfile: "
                                "./no_ca.crt\n  CApath: none")));

    ISerialized_object *serialized_keys= NULL;
    EXPECT_EQ(vault_io.get_serialized_object(&serialized_keys), TRUE);

    std::remove(credential_file_url.c_str());
  }

  TEST_F(Vault_io_test, InitWithOutVaultCA)
  {
    Vault_io vault_io(logger, vault_curl, vault_parser);

    std::remove(credential_file_url.c_str());
    std::ofstream my_file;
    my_file.open(credential_file_url.c_str());
    my_file << "vault_url = https://127.0.0.1:8200" << std::endl;
    my_file << "secret_mount_point = secret" << std::endl;
    my_file << "token = " << correct_token << std::endl;
    my_file.close();


    EXPECT_CALL(*((Mock_logger *)logger),
      log(MY_WARNING_LEVEL, StrEq("There is no vault_ca specified in keyring_vault's configuration file. "
                                  "Please make sure that Vault's CA certificate is trusted by the "
                                  "machine from which you intend to connect to Vault.")));
    EXPECT_CALL(*((Mock_logger *)logger),
      log(MY_ERROR_LEVEL, StrEq("Could not retrieve list of keys from Vault.")));
    EXPECT_CALL(*((Mock_logger *)logger),
      log(MY_ERROR_LEVEL, StrEq("Curl returned this error code: 60 with error message : "
                                "SSL certificate problem: unable to get local "
                                "issuer certificate")));


    EXPECT_EQ(vault_io.init(&credential_file_url), FALSE);

    ISerialized_object *serialized_keys= NULL;
    EXPECT_EQ(vault_io.get_serialized_object(&serialized_keys), TRUE);

    std::remove(credential_file_url.c_str());
  }*/

  TEST_F(Vault_io_test, GetSerializedObjectWithTwoKeys)
  {
    Vault_io vault_io(logger, vault_curl, vault_parser);

    EXPECT_EQ(vault_io.init(&credential_file_url), FALSE);

    //First Add Two keys into Vault
    Vault_key key1("key1", "AES", "Arczi", "Artur", 5);
    EXPECT_STREQ(key1.get_key_signature()->c_str(), "4_key15_Arczi");
    key1.set_key_operation(STORE_KEY);
    EXPECT_EQ(vault_io.flush_to_storage(&key1), FALSE);
    Vault_key key2("key2", "AES", "Kamil", "Kami", 4);
    EXPECT_STREQ(key2.get_key_signature()->c_str(), "4_key25_Kamil");
    key2.set_key_operation(STORE_KEY);
    EXPECT_EQ(vault_io.flush_to_storage(&key2), FALSE);
    //*****

    //Now fetch two keys with separate Vault_io
    ISerialized_object *serialized_keys= NULL;
    EXPECT_EQ(vault_io.get_serialized_object(&serialized_keys), FALSE);
    IKey *key1_loaded= NULL;
    ASSERT_TRUE(serialized_keys != NULL);
    EXPECT_EQ(serialized_keys->has_next_key(), TRUE);
    serialized_keys->get_next_key(&key1_loaded);
    EXPECT_STREQ(key1_loaded->get_key_signature()->c_str(), "4_key25_Kamil");
    IKey *key2_loaded= NULL;
    delete key1_loaded;
    EXPECT_EQ(serialized_keys->has_next_key(), TRUE);
    serialized_keys->get_next_key(&key2_loaded);
    EXPECT_STREQ(key2_loaded->get_key_signature()->c_str(), "4_key15_Arczi");
    delete key2_loaded;
    EXPECT_EQ(serialized_keys->has_next_key(), FALSE);
    delete serialized_keys;

    //Now remove the keys
    Vault_key key1_to_remove(key1);
    key1_to_remove.set_key_operation(REMOVE_KEY);
    EXPECT_EQ(vault_io.flush_to_storage(&key1_to_remove), FALSE);
    Vault_key key2_to_remove(key2);
    key2_to_remove.set_key_operation(REMOVE_KEY);
    EXPECT_EQ(vault_io.flush_to_storage(&key2_to_remove), FALSE);
  }

  TEST_F(Vault_io_test, GetSerializedObjectWithTwoKeysWithDifferentVaultIO)
  {
    Vault_io vault_io_for_storing(logger, vault_curl, vault_parser);

    EXPECT_EQ(vault_io_for_storing.init(&credential_file_url), FALSE);

    //First Add Two keys into Vault
    Vault_key key1("key1", "AES", "Robert", "Robi", 4);
    EXPECT_STREQ(key1.get_key_signature()->c_str(), "4_key16_Robert");
    key1.set_key_operation(STORE_KEY);
    EXPECT_EQ(vault_io_for_storing.flush_to_storage(&key1), FALSE);
    Vault_key key2("key2", "AES", "Kamil", "Kami", 4);
    EXPECT_STREQ(key2.get_key_signature()->c_str(), "4_key25_Kamil");
    key2.set_key_operation(STORE_KEY);
    EXPECT_EQ(vault_io_for_storing.flush_to_storage(&key2), FALSE);
    //*****

    //Now fetch two keys with separate Vault_io
    Vault_curl *vault_curl2 = new Vault_curl(logger);
    Vault_parser *vault_parser2 = new Vault_parser(logger);
    Vault_io vault_io_for_fetching(logger, vault_curl2, vault_parser2);
    EXPECT_EQ(vault_io_for_fetching.init(&credential_file_url), FALSE);

    ISerialized_object *serialized_keys= NULL;
    EXPECT_EQ(vault_io_for_fetching.get_serialized_object(&serialized_keys), FALSE);
    IKey *key1_loaded= NULL;
    ASSERT_TRUE(serialized_keys != NULL);
    EXPECT_EQ(serialized_keys->has_next_key(), TRUE);
    serialized_keys->get_next_key(&key1_loaded);
    EXPECT_STREQ(key1_loaded->get_key_signature()->c_str(), "4_key16_Robert");
    IKey *key2_loaded= NULL;
    delete key1_loaded;
    EXPECT_EQ(serialized_keys->has_next_key(), TRUE);
    serialized_keys->get_next_key(&key2_loaded);
    EXPECT_STREQ(key2_loaded->get_key_signature()->c_str(), "4_key25_Kamil");
    delete key2_loaded;
    EXPECT_EQ(serialized_keys->has_next_key(), FALSE);
    delete serialized_keys;

    //Now remove the keys
    Vault_key key1_to_remove(key1);
    key1_to_remove.set_key_operation(REMOVE_KEY);
    EXPECT_EQ(vault_io_for_storing.flush_to_storage(&key1_to_remove), FALSE);
    Vault_key key2_to_remove(key2);
    key2_to_remove.set_key_operation(REMOVE_KEY);
    EXPECT_EQ(vault_io_for_storing.flush_to_storage(&key2_to_remove), FALSE);
  }

  TEST_F(Vault_io_test, RetrieveKeyTypeAndValue)
  {
    Vault_io vault_io(logger, vault_curl, vault_parser);
    EXPECT_EQ(vault_io.init(&credential_file_url), FALSE);

    Vault_key key_to_store("key1", "AES", "rob", "Robi", 4);
    key_to_store.set_key_operation(STORE_KEY);
    EXPECT_EQ(vault_io.flush_to_storage(&key_to_store), FALSE);

    Vault_key key("key1", NULL, "rob", NULL, 0);
    EXPECT_EQ(vault_io.retrieve_key_type_and_data(&key), FALSE);
    EXPECT_STREQ(key.get_key_signature()->c_str(), "4_key13_rob");
    ASSERT_TRUE(memcmp(key.get_key_data(), "Robi", key.get_key_data_size()) == 0);
    EXPECT_STREQ("AES", key.get_key_type()->c_str());

    Vault_key key_to_remove("key1", NULL, "rob", NULL, 0);
    key_to_remove.set_key_operation(REMOVE_KEY);
    EXPECT_EQ(vault_io.flush_to_storage(&key_to_remove), FALSE);
  }

  TEST_F(Vault_io_test, FlushAndRemoveSingleKey)
  {
    Vault_io vault_io(logger, vault_curl, vault_parser);
    EXPECT_EQ(vault_io.init(&credential_file_url), FALSE);
    Vault_key key("key1", "AES", "rob", "Robi", 4);
    key.set_key_operation(STORE_KEY);
    EXPECT_EQ(vault_io.flush_to_storage(&key), FALSE);
    Vault_key key_to_remove(key);
    key_to_remove.set_key_operation(REMOVE_KEY);
    EXPECT_EQ(vault_io.flush_to_storage(&key_to_remove), FALSE);
  }

  TEST_F(Vault_io_test, FlushKeyRetrieveDeleteInit)
  {
    Vault_io vault_io(logger, vault_curl, vault_parser);
    EXPECT_EQ(vault_io.init(&credential_file_url), FALSE);
    Vault_key key("key1", "AES", "rob", "Robi", 4);
    key.set_key_operation(STORE_KEY);
    EXPECT_EQ(vault_io.flush_to_storage(&key), FALSE);
    Vault_key key1_id("key1", NULL, "rob", NULL, 0);
    EXPECT_EQ(vault_io.retrieve_key_type_and_data(&key1_id), FALSE);
    EXPECT_STREQ(key1_id.get_key_signature()->c_str(), "4_key13_rob");
    ASSERT_TRUE(memcmp(key1_id.get_key_data(), "Robi", key1_id.get_key_data_size()) == 0);
    EXPECT_STREQ("AES", key1_id.get_key_type()->c_str());

    Vault_key key_to_remove(key);
    key_to_remove.set_key_operation(REMOVE_KEY);
    EXPECT_EQ(vault_io.flush_to_storage(&key_to_remove), FALSE);

    Vault_curl *vault_curl2 = new Vault_curl(logger);
    Vault_parser *vault_parser2 = new Vault_parser(logger);
    Vault_io vault_io2(logger, vault_curl2, vault_parser2);
    EXPECT_EQ(vault_io2.init(&credential_file_url), FALSE);
    ISerialized_object *serialized_keys= NULL;
    EXPECT_EQ(vault_io2.get_serialized_object(&serialized_keys), FALSE);
    ASSERT_TRUE(serialized_keys == NULL); //no keys
  }

  class Mock_vault_curl : public IVault_curl
  {
  public:
    MOCK_METHOD1(init, my_bool(Vault_credentials *vault_credentials));
    MOCK_METHOD1(list_keys, my_bool(std::string *response));
    MOCK_METHOD2(write_key, my_bool(IKey *key, std::string *response));
    MOCK_METHOD2(read_key, my_bool(IKey *key, std::string *response));
    MOCK_METHOD2(delete_key, my_bool(IKey *key, std::string *response));
  };

  TEST_F(Vault_io_test, ErrorFromVaultCurlOnVaultIOInit)
  {
    delete vault_curl;
    Mock_vault_curl *mock_curl = new Mock_vault_curl();
    Vault_io vault_io(logger, mock_curl, vault_parser);

    EXPECT_CALL(*mock_curl, init(_))
      .WillOnce(Return(TRUE)); // init unsuccessfull
    EXPECT_EQ(vault_io.init(&credential_file_url), TRUE);
  }

  TEST_F(Vault_io_test, ErrorFromVaultCurlOnListKeys)
  {
    delete vault_curl;
    Mock_vault_curl *mock_curl = new Mock_vault_curl();
    Vault_io vault_io(logger, mock_curl, vault_parser);

    EXPECT_CALL(*mock_curl, init(_))
      .WillOnce(Return(FALSE)); // init successfull
    EXPECT_EQ(vault_io.init(&credential_file_url), FALSE);

    ISerialized_object *serialized_object;

    EXPECT_CALL(*mock_curl, list_keys(_))
      .WillOnce(Return(TRUE)); //failed to list keys
    EXPECT_CALL(*((Mock_logger *)logger),
      log(MY_ERROR_LEVEL, StrEq("Could not retrieve list of keys from Vault.")));

    EXPECT_EQ(vault_io.get_serialized_object(&serialized_object), TRUE);
    EXPECT_EQ(serialized_object, reinterpret_cast<ISerialized_object*>(NULL));
  }

  TEST_F(Vault_io_test, ErrorsFromVaultInVaultsResponseOnListKeys)
  {
    delete vault_curl;
    Mock_vault_curl *mock_curl = new Mock_vault_curl();
    Vault_io vault_io(logger, mock_curl, vault_parser);

    EXPECT_CALL(*mock_curl, init(_))
      .WillOnce(Return(FALSE)); // init successfull
    EXPECT_EQ(vault_io.init(&credential_file_url), FALSE);

    ISerialized_object *serialized_object;
    std::string vault_response("{ errors: [\"list is broken\"] }"); 

    EXPECT_CALL(*mock_curl, list_keys(_))
      .WillOnce(DoAll(SetArgPointee<0>(vault_response), Return(TRUE)));
      
    EXPECT_CALL(*((Mock_logger *)logger),
      log(MY_ERROR_LEVEL, StrEq("Could not retrieve list of keys from Vault. Vault has returned the following error(s): [\"list is broken\"]")));

    EXPECT_EQ(vault_io.get_serialized_object(&serialized_object), TRUE);
    EXPECT_EQ(serialized_object, reinterpret_cast<ISerialized_object*>(NULL));

    vault_response = "{errors: [\"list is broken\", \"and some other error\"]}"; 

    EXPECT_CALL(*mock_curl, list_keys(_))
      .WillOnce(DoAll(SetArgPointee<0>(vault_response), Return(TRUE)));
      
    EXPECT_CALL(*((Mock_logger *)logger),
      log(MY_ERROR_LEVEL, StrEq("Could not retrieve list of keys from Vault. Vault has returned the following error(s): [\"list is broken\", \"and some other error\"]")));

    EXPECT_EQ(vault_io.get_serialized_object(&serialized_object), TRUE);
    EXPECT_EQ(serialized_object, reinterpret_cast<ISerialized_object*>(NULL));

    vault_response = "{ errors: [\"list is broken\",\n\"and some other error\"\n] }"; 

    EXPECT_CALL(*mock_curl, list_keys(_))
      .WillOnce(DoAll(SetArgPointee<0>(vault_response), Return(TRUE)));
      
    EXPECT_CALL(*((Mock_logger *)logger),
      log(MY_ERROR_LEVEL, StrEq("Could not retrieve list of keys from Vault. Vault has returned the following error(s): [\"list is broken\",\"and some other error\"]")));

    EXPECT_EQ(vault_io.get_serialized_object(&serialized_object), TRUE);
    EXPECT_EQ(serialized_object, reinterpret_cast<ISerialized_object*>(NULL));
  }

  TEST_F(Vault_io_test, ErrorsFromVaultCurlOnReadKey)
  {
    delete vault_curl;
    Mock_vault_curl *mock_curl = new Mock_vault_curl();
    Vault_io vault_io(logger, mock_curl, vault_parser);

    EXPECT_CALL(*mock_curl, init(_))
      .WillOnce(Return(FALSE)); // init successfull
    EXPECT_EQ(vault_io.init(&credential_file_url), FALSE);

    IKey *key = NULL;

    EXPECT_CALL(*mock_curl, read_key(key, _))
      .WillOnce(Return(TRUE));
    EXPECT_CALL(*((Mock_logger *)logger),
      log(MY_ERROR_LEVEL, StrEq("Could not read key from Vault.")));
    EXPECT_EQ(vault_io.retrieve_key_type_and_data(key), TRUE);
  }

  TEST_F(Vault_io_test, ErrorsFromVaultInVaultsCurlResponseOnReadKey)
  {
    delete vault_curl;
    Mock_vault_curl *mock_curl = new Mock_vault_curl();
    Vault_io vault_io(logger, mock_curl, vault_parser);

    EXPECT_CALL(*mock_curl, init(_))
      .WillOnce(Return(FALSE)); // init successfull
    EXPECT_EQ(vault_io.init(&credential_file_url), FALSE);

    IKey *key = NULL;
    std::string vault_response("{ errors: [\"Cannot read this stuff\"] }"); 

    EXPECT_CALL(*mock_curl, read_key(key, _))
      .WillOnce(DoAll(SetArgPointee<1>(vault_response), Return(TRUE)));
    EXPECT_CALL(*((Mock_logger *)logger),
      log(MY_ERROR_LEVEL, StrEq("Could not read key from Vault. Vault has returned the following error(s):"
                                " [\"Cannot read this stuff\"]")));
    EXPECT_EQ(vault_io.retrieve_key_type_and_data(key), TRUE);
  }

  TEST_F(Vault_io_test, ErrorsFromVaultCurlOnDeleteKey)
  {
    delete vault_curl;
    Mock_vault_curl *mock_curl = new Mock_vault_curl();
    Vault_io vault_io(logger, mock_curl, vault_parser);

    EXPECT_CALL(*mock_curl, init(_))
      .WillOnce(Return(FALSE)); // init successfull
    EXPECT_EQ(vault_io.init(&credential_file_url), FALSE);

    Vault_key key("key1", "AES", "Arczi", "Artur", 5);
    key.set_key_operation(REMOVE_KEY);

    EXPECT_CALL(*mock_curl, delete_key(_,_))
      .WillOnce(Return(TRUE));
    EXPECT_CALL(*((Mock_logger *)logger),
      log(MY_ERROR_LEVEL, StrEq("Could not delete key from Vault.")));
    EXPECT_EQ(vault_io.flush_to_storage(&key), TRUE);

    //delete mock_curl;
  }

  TEST_F(Vault_io_test, ErrorsFromVaultInVaultsCurlResponseOnDeleteKey)
  {
    delete vault_curl;
    Mock_vault_curl *mock_curl = new Mock_vault_curl();
    Vault_io vault_io(logger, mock_curl, vault_parser);

    EXPECT_CALL(*mock_curl, init(_))
      .WillOnce(Return(FALSE)); // init successfull
    EXPECT_EQ(vault_io.init(&credential_file_url), FALSE);

    Vault_key key("key1", "AES", "Arczi", "Artur", 5);
    key.set_key_operation(REMOVE_KEY);
    std::string vault_response("{ errors: [\"Cannot delete this stuff\"] }"); 

    EXPECT_CALL(*mock_curl, delete_key(_, _))
      .WillOnce(DoAll(SetArgPointee<1>(vault_response), Return(FALSE)));
    EXPECT_CALL(*((Mock_logger *)logger),
      log(MY_ERROR_LEVEL, StrEq("Could not delete key from Vault. Vault has returned the following error(s):"
                                " [\"Cannot delete this stuff\"]")));
    EXPECT_EQ(vault_io.flush_to_storage(&key), TRUE);
  }

  TEST_F(Vault_io_test, ErrorsFromVaultCurlOnWriteKey)
  {
    delete vault_curl;
    Mock_vault_curl *mock_curl = new Mock_vault_curl();
    Vault_io vault_io(logger, mock_curl, vault_parser);

    EXPECT_CALL(*mock_curl, init(_))
      .WillOnce(Return(FALSE)); // init successfull
    EXPECT_EQ(vault_io.init(&credential_file_url), FALSE);

    Vault_key key("key1", "AES", "Arczi", "Artur", 5);
    key.set_key_operation(STORE_KEY);

    EXPECT_CALL(*mock_curl, write_key(_,_))
      .WillOnce(Return(TRUE));
    EXPECT_CALL(*((Mock_logger *)logger),
      log(MY_ERROR_LEVEL, StrEq("Could not write key to Vault.")));
    EXPECT_EQ(vault_io.flush_to_storage(&key), TRUE);
  }

  TEST_F(Vault_io_test, ErrorsFromVaultInVaultsCurlResponseOnWriteKey)
  {
    delete vault_curl;
    Mock_vault_curl *mock_curl = new Mock_vault_curl();
    Vault_io vault_io(logger, mock_curl, vault_parser);

    EXPECT_CALL(*mock_curl, init(_))
      .WillOnce(Return(FALSE)); // init successfull
    EXPECT_EQ(vault_io.init(&credential_file_url), FALSE);

    Vault_key key("key1", "AES", "Arczi", "Artur", 5);
    key.set_key_operation(STORE_KEY);
    std::string vault_response("{ errors: [\"Cannot write this stuff\"] }"); 

    EXPECT_CALL(*mock_curl, write_key(_, _))
      .WillOnce(DoAll(SetArgPointee<1>(vault_response), Return(FALSE)));
    EXPECT_CALL(*((Mock_logger *)logger),
      log(MY_ERROR_LEVEL, StrEq("Could not write key to Vault. Vault has returned the following error(s):"
                                " [\"Cannot write this stuff\"]")));
    EXPECT_EQ(vault_io.flush_to_storage(&key), TRUE);
  }

  class Mock_vault_parser : public IVault_parser
  {
  public:
    MOCK_METHOD2(parse_keys, my_bool(std::string *payload, Vault_keys_list *keys));
    MOCK_METHOD2(parse_key_data, my_bool(std::string *payload, IKey *key));
    MOCK_METHOD2(parse_key_signature, my_bool(const std::string *key_signature, std::string key_parameters[2]));
    MOCK_METHOD2(parse_errors, my_bool(std::string *payload, std::string *errors));
  };

  TEST_F(Vault_io_test, ErrorFromParseKeysOnGetSerializedObject)
  {
    delete vault_parser;

    Mock_vault_parser *mock_vault_parser = new Mock_vault_parser;
    Vault_io vault_io(logger, vault_curl, mock_vault_parser);

    EXPECT_EQ(vault_io.init(&credential_file_url), FALSE);

    //First Add Two keys into Vault
    Vault_key key1("key1", "AES", "Arczi", "Artur", 5);
    EXPECT_STREQ(key1.get_key_signature()->c_str(), "4_key15_Arczi");
    key1.set_key_operation(STORE_KEY);
    EXPECT_EQ(vault_io.flush_to_storage(&key1), FALSE);
    Vault_key key2("key2", "AES", "Kamil", "Kami", 4);
    EXPECT_STREQ(key2.get_key_signature()->c_str(), "4_key25_Kamil");
    key2.set_key_operation(STORE_KEY);
    EXPECT_EQ(vault_io.flush_to_storage(&key2), FALSE);
    //*****

    EXPECT_CALL(*mock_vault_parser, parse_keys(_, _))
      .WillOnce(Return(TRUE));
    EXPECT_CALL(*((Mock_logger *)logger),
      log(MY_ERROR_LEVEL, StrEq("Could not retrieve list of keys from Vault.")));

    ISerialized_object *serialized_keys= NULL;
    EXPECT_EQ(vault_io.get_serialized_object(&serialized_keys), TRUE);

    ASSERT_TRUE(serialized_keys == NULL);

    //Now remove the keys
    Vault_key key1_to_remove(key1);
    key1_to_remove.set_key_operation(REMOVE_KEY);
    EXPECT_EQ(vault_io.flush_to_storage(&key1_to_remove), FALSE);
    Vault_key key2_to_remove(key2);
    key2_to_remove.set_key_operation(REMOVE_KEY);
    EXPECT_EQ(vault_io.flush_to_storage(&key2_to_remove), FALSE);
  }

  TEST_F(Vault_io_test, ErrorFromParseKeyDataOnRetrieveKeyTypeAndValue)
  {
    delete vault_parser;
    Mock_vault_parser *mock_vault_parser = new Mock_vault_parser;
    Vault_io vault_io(logger, vault_curl, mock_vault_parser);
    EXPECT_EQ(vault_io.init(&credential_file_url), FALSE);

    Vault_key key_to_store("key1", "AES", "rob", "Robi", 4);
    key_to_store.set_key_operation(STORE_KEY);
    EXPECT_EQ(vault_io.flush_to_storage(&key_to_store), FALSE);

    Vault_key key("key1", NULL, "rob", NULL, 0);
    EXPECT_CALL(*mock_vault_parser, parse_key_data(_, &key))
      .WillOnce(Return(TRUE));
    EXPECT_CALL(*mock_vault_parser, parse_errors(_, _))
      .WillOnce(Return(FALSE));
    EXPECT_CALL(*((Mock_logger *)logger),
      log(MY_ERROR_LEVEL, StrEq("Could not read key from Vault.")));
    EXPECT_EQ(vault_io.retrieve_key_type_and_data(&key), TRUE);

    Vault_key key_to_remove("key1", NULL, "rob", NULL, 0);
    key_to_remove.set_key_operation(REMOVE_KEY);
    EXPECT_EQ(vault_io.flush_to_storage(&key_to_remove), FALSE);
  }

  TEST_F(Vault_io_test, ErrorFromParseKeyDataAndParseErrorsOnRetrieveKeyTypeAndValue)
  {
    delete vault_parser;
    Mock_vault_parser *mock_vault_parser = new Mock_vault_parser;
    Vault_io vault_io(logger, vault_curl, mock_vault_parser);
    EXPECT_EQ(vault_io.init(&credential_file_url), FALSE);

    Vault_key key_to_store("key1", "AES", "rob", "Robi", 4);
    key_to_store.set_key_operation(STORE_KEY);
    EXPECT_EQ(vault_io.flush_to_storage(&key_to_store), FALSE);

    Vault_key key("key1", NULL, "rob", NULL, 0);
    EXPECT_CALL(*mock_vault_parser, parse_key_data(_, &key))
      .WillOnce(Return(TRUE));
    EXPECT_CALL(*mock_vault_parser, parse_errors(_, _))
      .WillOnce(Return(TRUE));
    EXPECT_CALL(*((Mock_logger *)logger),
      log(MY_ERROR_LEVEL, StrEq("Could not read key from Vault. Error while parsing error messages")));
    EXPECT_EQ(vault_io.retrieve_key_type_and_data(&key), TRUE);

    Vault_key key_to_remove("key1", NULL, "rob", NULL, 0);
    key_to_remove.set_key_operation(REMOVE_KEY);
    EXPECT_EQ(vault_io.flush_to_storage(&key_to_remove), FALSE);
  }
} //namespace keyring__file_io_unittest

int main(int argc, char **argv) {
  curl_global_init(CURL_GLOBAL_DEFAULT);
  ::testing::InitGoogleTest(&argc, argv);
  int ret= RUN_ALL_TESTS();
  curl_global_cleanup();
  return ret;
}
