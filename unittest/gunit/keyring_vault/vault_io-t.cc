#include <curl/curl.h>
#include <gtest/gtest.h>
#include <string.h>
#include <boost/scope_exit.hpp>
#include <fstream>
#include <memory>
#include "generate_credential_file.h"
#include "i_keys_container.h"
#include "incorrect_vault_key.h"
#include "mock_logger.h"
#include "test_utils.h"
#include "uuid.h"
#include "vault_io.h"
#include "vault_mount.h"

extern std::string uuid;
#ifndef MERGE_UNITTESTS
std::string uuid = generate_uuid();
#endif

namespace keyring__vault_io_unittest {
using namespace keyring;

using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::StrEq;
using ::testing::_;

std::string credential_file_url = "./keyring_vault.conf";

class Vault_io_test : public ::testing::Test {
 protected:
  virtual void SetUp() override {
    logger = new Mock_logger();
    vault_curl = new Vault_curl(logger, 0);
    vault_parser = new Vault_parser(logger);
  }

  virtual void TearDown() override { delete logger; }

 protected:
  std::string key_1 = (uuid + "key1");
  std::string key_2 = (uuid + "key2");
  const char *key_1_id = key_1.c_str();
  const char *key_2_id = key_2.c_str();

  IVault_curl *vault_curl;
  IVault_parser *vault_parser;
  ILogger *logger;
};

TEST_F(Vault_io_test, InitWithNotExisitingCredentialFile) {
  std::string credential_file_name("./some_funny_name");
  Vault_io vault_io(logger, vault_curl, vault_parser);
  remove(credential_file_name.c_str());
  EXPECT_CALL(
      *(reinterpret_cast<Mock_logger *>(logger)),
      log(MY_WARNING_LEVEL, StrEq("File './some_funny_name' not found (OS "
                                  "errno 2 - No such file or directory)")));

  EXPECT_CALL(
      *(reinterpret_cast<Mock_logger *>(logger)),
      log(MY_ERROR_LEVEL, StrEq("Could not open file with credentials.")));
  EXPECT_TRUE(vault_io.init(&credential_file_name));

  remove(credential_file_name.c_str());
}

TEST_F(Vault_io_test, InitWithInvalidToken) {
  std::string conf_with_invalid_token("./invalid_token.conf");
  ASSERT_FALSE(
      generate_credential_file(conf_with_invalid_token, WITH_INVALID_TOKEN));

  Vault_io vault_io(logger, vault_curl, vault_parser);
  EXPECT_FALSE(vault_io.init(&conf_with_invalid_token));

  EXPECT_CALL(
      *(reinterpret_cast<Mock_logger *>(logger)),
      log(MY_ERROR_LEVEL, StrEq("Could not retrieve list of keys from Vault. "
                                "Vault has returned the following error(s): "
                                "[\"permission denied\"]")));
  ISerialized_object *serialized_keys = nullptr;
  EXPECT_TRUE(vault_io.get_serialized_object(&serialized_keys));
}

TEST_F(Vault_io_test, GetSerializedObjectWithTwoKeys) {
  Vault_io vault_io(logger, vault_curl, vault_parser);

  EXPECT_FALSE(vault_io.init(&credential_file_url));

  // First add two keys into Vault
  Vault_key key1(key_1_id, "AES", "Arczi", "Artur", 5);
  EXPECT_TRUE(*key1.get_key_signature() ==
              get_key_signature(uuid, "key1", "Arczi"));
  key1.set_key_operation(STORE_KEY);
  EXPECT_FALSE(vault_io.flush_to_storage(&key1));
  Vault_key key2(key_2_id, "AES", "Kamil", "Kami", 4);
  EXPECT_TRUE(*key2.get_key_signature() ==
              get_key_signature(uuid, "key2", "Kamil"));
  key2.set_key_operation(STORE_KEY);
  EXPECT_FALSE(vault_io.flush_to_storage(&key2));
  // *****

  // Now fetch two keys with separate Vault_io
  ISerialized_object *serialized_keys = nullptr;
  EXPECT_FALSE(vault_io.get_serialized_object(&serialized_keys));
  IKey *key1_loaded = nullptr;
  ASSERT_TRUE(serialized_keys != nullptr);
  EXPECT_TRUE(serialized_keys->has_next_key());
  serialized_keys->get_next_key(&key1_loaded);
  EXPECT_TRUE(*key1_loaded->get_key_signature() ==
                  get_key_signature(uuid, "key1", "Arczi") ||
              *key1_loaded->get_key_signature() ==
                  get_key_signature(uuid, "key2", "Kamil"));
  IKey *key2_loaded = nullptr;
  delete key1_loaded;
  EXPECT_TRUE(serialized_keys->has_next_key());
  serialized_keys->get_next_key(&key2_loaded);
  EXPECT_TRUE(*key2_loaded->get_key_signature() ==
                  get_key_signature(uuid, "key2", "Kamil") ||
              *key2_loaded->get_key_signature() ==
                  get_key_signature(uuid, "key1", "Arczi"));
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

TEST_F(Vault_io_test, GetSerializedObjectWithTwoKeysWithDifferentVaultIO) {
  Vault_io vault_io_for_storing(logger, vault_curl, vault_parser);

  EXPECT_FALSE(vault_io_for_storing.init(&credential_file_url));

  // First add two keys into Vault
  Vault_key key1(key_1_id, "AES", "Robert", "Robi", 4);
  EXPECT_TRUE(*key1.get_key_signature() ==
              get_key_signature(uuid, "key1", "Robert"));
  key1.set_key_operation(STORE_KEY);
  EXPECT_FALSE(vault_io_for_storing.flush_to_storage(&key1));
  Vault_key key2(key_2_id, "AES", "Kamil", "Kami", 4);
  EXPECT_TRUE(*key2.get_key_signature() ==
              get_key_signature(uuid, "key2", "Kamil"));
  key2.set_key_operation(STORE_KEY);
  EXPECT_FALSE(vault_io_for_storing.flush_to_storage(&key2));
  // *****

  // Now fetch two keys with separate Vault_io
  Vault_curl *vault_curl2 = new Vault_curl(logger, 0);
  Vault_parser *vault_parser2 = new Vault_parser(logger);
  Vault_io vault_io_for_fetching(logger, vault_curl2, vault_parser2);
  EXPECT_FALSE(vault_io_for_fetching.init(&credential_file_url));

  ISerialized_object *serialized_keys = nullptr;
  EXPECT_FALSE(vault_io_for_fetching.get_serialized_object(&serialized_keys));
  IKey *key1_loaded = nullptr;
  ASSERT_TRUE(serialized_keys != nullptr);
  EXPECT_TRUE(serialized_keys->has_next_key());
  serialized_keys->get_next_key(&key1_loaded);
  EXPECT_TRUE(*key1_loaded->get_key_signature() ==
                  get_key_signature(uuid, "key1", "Robert") ||
              *key1_loaded->get_key_signature() ==
                  get_key_signature(uuid, "key2", "Kamil"));
  IKey *key2_loaded = nullptr;
  delete key1_loaded;
  EXPECT_TRUE(serialized_keys->has_next_key());
  serialized_keys->get_next_key(&key2_loaded);
  EXPECT_TRUE(*key2_loaded->get_key_signature() ==
                  get_key_signature(uuid, "key2", "Kamil") ||
              *key2_loaded->get_key_signature() ==
                  get_key_signature(uuid, "key1", "Robert"));
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
TEST_F(Vault_io_test, InitWithIncorrectKeyInVault) {
  Vault_io vault_io_for_storing(logger, vault_curl, vault_parser);

  EXPECT_FALSE(vault_io_for_storing.init(&credential_file_url));

  // First add two keys into Vault
  Vault_key key1(key_1_id, "AES", "Robert", "Robi", 4);
  EXPECT_TRUE(*key1.get_key_signature() ==
              get_key_signature(uuid, "key1", "Robert"));
  key1.set_key_operation(STORE_KEY);
  EXPECT_FALSE(vault_io_for_storing.flush_to_storage(&key1));
  Vault_key key2(key_2_id, "AES", "Kamil", "Kami", 4);
  EXPECT_TRUE(*key2.get_key_signature() ==
              get_key_signature(uuid, "key2", "Kamil"));
  key2.set_key_operation(STORE_KEY);
  EXPECT_FALSE(vault_io_for_storing.flush_to_storage(&key2));
  Incorrect_vault_key incorrect_key("key3", "RSA", "Artur", "Arczi", 5);
  incorrect_key.set_key_operation(STORE_KEY);
  incorrect_key.add_to_key_id_length = 14;
  // make sure signature is incorrect
  EXPECT_STREQ(incorrect_key.get_key_signature()->c_str(), "18_key35_Artur");
  EXPECT_FALSE(vault_io_for_storing.flush_to_storage(&incorrect_key));
  // *****

  // Now fetch two keys with separate Vault_io - incorrect key should have been
  // ignored
  Vault_curl *vault_curl2 = new Vault_curl(logger, 0);
  Vault_parser *vault_parser2 = new Vault_parser(logger);
  Vault_io vault_io_for_fetching(logger, vault_curl2, vault_parser2);

  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(logger)),
              log(MY_WARNING_LEVEL,
                  StrEq("Could not parse key's signature, skipping the key.")));

  EXPECT_FALSE(vault_io_for_fetching.init(&credential_file_url));

  ISerialized_object *serialized_keys = nullptr;
  EXPECT_FALSE(vault_io_for_fetching.get_serialized_object(&serialized_keys));
  IKey *key1_loaded = nullptr;
  ASSERT_TRUE(serialized_keys != nullptr);
  EXPECT_TRUE(serialized_keys->has_next_key());
  serialized_keys->get_next_key(&key1_loaded);
  EXPECT_TRUE(*key1_loaded->get_key_signature() ==
                  get_key_signature(uuid, "key1", "Robert") ||
              *key1_loaded->get_key_signature() ==
                  get_key_signature(uuid, "key2", "Kamil"));
  IKey *key2_loaded = nullptr;
  delete key1_loaded;
  EXPECT_TRUE(serialized_keys->has_next_key());
  serialized_keys->get_next_key(&key2_loaded);
  EXPECT_TRUE(*key2_loaded->get_key_signature() ==
                  get_key_signature(uuid, "key2", "Kamil") ||
              *key2_loaded->get_key_signature() ==
                  get_key_signature(uuid, "key1", "Robert"));
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

TEST_F(Vault_io_test, RetrieveKeyTypeAndValue) {
  Vault_io vault_io(logger, vault_curl, vault_parser);
  EXPECT_FALSE(vault_io.init(&credential_file_url));

  Vault_key key_to_store(key_1_id, "AES", "rob", "Robi", 4);
  key_to_store.set_key_operation(STORE_KEY);
  EXPECT_FALSE(vault_io.flush_to_storage(&key_to_store));

  Vault_key key(key_1_id, nullptr, "rob", nullptr, 0);
  EXPECT_FALSE(vault_io.retrieve_key_type_and_data(&key));
  EXPECT_TRUE(*key.get_key_signature() ==
              get_key_signature(uuid, "key1", "rob"));
  ASSERT_TRUE(memcmp(key.get_key_data(), "Robi", key.get_key_data_size()) == 0);
  EXPECT_STREQ("AES", key.get_key_type()->c_str());

  Vault_key key_to_remove(key_1_id, nullptr, "rob", nullptr, 0);
  key_to_remove.set_key_operation(REMOVE_KEY);
  EXPECT_FALSE(vault_io.flush_to_storage(&key_to_remove));
}

TEST_F(Vault_io_test, FlushAndRemoveSingleKey) {
  Vault_io vault_io(logger, vault_curl, vault_parser);
  EXPECT_FALSE(vault_io.init(&credential_file_url));
  Vault_key key(key_1_id, "AES", "rob", "Robi", 4);
  key.set_key_operation(STORE_KEY);
  EXPECT_FALSE(vault_io.flush_to_storage(&key));
  Vault_key key_to_remove(key);
  key_to_remove.set_key_operation(REMOVE_KEY);
  EXPECT_FALSE(vault_io.flush_to_storage(&key_to_remove));
}

TEST_F(Vault_io_test, FlushKeyRetrieveDeleteInit) {
  Vault_io vault_io(logger, vault_curl, vault_parser);
  EXPECT_FALSE(vault_io.init(&credential_file_url));
  Vault_key key(key_1_id, "AES", "rob", "Robi", 4);
  key.set_key_operation(STORE_KEY);
  EXPECT_FALSE(vault_io.flush_to_storage(&key));
  Vault_key key1_id(key_1_id, nullptr, "rob", nullptr, 0);
  EXPECT_FALSE(vault_io.retrieve_key_type_and_data(&key1_id));
  EXPECT_TRUE(*key1_id.get_key_signature() ==
              get_key_signature(uuid, "key1", "rob"));
  ASSERT_TRUE(
      memcmp(key1_id.get_key_data(), "Robi", key1_id.get_key_data_size()) == 0);
  EXPECT_STREQ("AES", key1_id.get_key_type()->c_str());

  Vault_key key_to_remove(key);
  key_to_remove.set_key_operation(REMOVE_KEY);
  EXPECT_FALSE(vault_io.flush_to_storage(&key_to_remove));

  Vault_curl *vault_curl2 = new Vault_curl(logger, 0);
  Vault_parser *vault_parser2 = new Vault_parser(logger);
  Vault_io vault_io2(logger, vault_curl2, vault_parser2);
  EXPECT_FALSE(vault_io2.init(&credential_file_url));
  ISerialized_object *serialized_keys = nullptr;
  EXPECT_FALSE(vault_io2.get_serialized_object(&serialized_keys));
  ASSERT_TRUE(serialized_keys == nullptr);  // no keys
}

#define MOCK_NOEXCEPT_METHOD1(m, ...) \
  GMOCK_METHOD1_(, noexcept, , m, __VA_ARGS__)

class Mock_vault_curl : public IVault_curl {
 public:
  MOCK_METHOD1(init, bool(const Vault_credentials &vault_credentials));
  MOCK_METHOD1(list_keys, bool(Secure_string *response));
  MOCK_METHOD2(write_key, bool(const Vault_key &key, Secure_string *response));
  MOCK_METHOD2(read_key, bool(const Vault_key &key, Secure_string *response));
  MOCK_METHOD2(delete_key, bool(const Vault_key &key, Secure_string *response));
  MOCK_NOEXCEPT_METHOD1(set_timeout, void(uint timeout));
};

TEST_F(Vault_io_test, ErrorFromVaultCurlOnVaultIOInit) {
  delete vault_curl;
  Mock_vault_curl *mock_curl = new Mock_vault_curl();
  Vault_io vault_io(logger, mock_curl, vault_parser);

  EXPECT_CALL(*mock_curl, init(_)).WillOnce(Return(true));  // init unsuccessful
  EXPECT_TRUE(vault_io.init(&credential_file_url));
}

TEST_F(Vault_io_test, ErrorFromVaultCurlOnListKeys) {
  delete vault_curl;
  Mock_vault_curl *mock_curl = new Mock_vault_curl();
  Vault_io vault_io(logger, mock_curl, vault_parser);

  EXPECT_CALL(*mock_curl, init(_)).WillOnce(Return(false));  // init successful
  EXPECT_FALSE(vault_io.init(&credential_file_url));

  ISerialized_object *serialized_object;

  EXPECT_CALL(*mock_curl, list_keys(_))
      .WillOnce(Return(true));  // failed to list keys
  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(logger)),
              log(MY_ERROR_LEVEL,
                  StrEq("Could not retrieve list of keys from Vault.")));

  EXPECT_TRUE(vault_io.get_serialized_object(&serialized_object));
  EXPECT_EQ(serialized_object, nullptr);
}

TEST_F(Vault_io_test, ErrorsFromVaultInVaultsResponseOnListKeys) {
  delete vault_curl;
  Mock_vault_curl *mock_curl = new Mock_vault_curl();
  Vault_io vault_io(logger, mock_curl, vault_parser);

  EXPECT_CALL(*mock_curl, init(_)).WillOnce(Return(false));  // init successful
  EXPECT_FALSE(vault_io.init(&credential_file_url));

  ISerialized_object *serialized_object;
  Secure_string vault_response("{ errors: [\"list is broken\"] }");

  EXPECT_CALL(*mock_curl, list_keys(_))
      .WillOnce(DoAll(SetArgPointee<0>(vault_response), Return(true)));

  EXPECT_CALL(
      *(reinterpret_cast<Mock_logger *>(logger)),
      log(MY_ERROR_LEVEL,
          StrEq("Could not retrieve list of keys from Vault. Vault has "
                "returned the following error(s): [\"list is broken\"]")));

  EXPECT_TRUE(vault_io.get_serialized_object(&serialized_object));
  EXPECT_EQ(serialized_object, nullptr);

  vault_response = "{errors: [\"list is broken\", \"and some other error\"]}";

  EXPECT_CALL(*mock_curl, list_keys(_))
      .WillOnce(DoAll(SetArgPointee<0>(vault_response), Return(true)));

  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(logger)),
              log(MY_ERROR_LEVEL,
                  StrEq("Could not retrieve list of keys from Vault. Vault has "
                        "returned the following error(s): [\"list is broken\", "
                        "\"and some other error\"]")));

  EXPECT_TRUE(vault_io.get_serialized_object(&serialized_object));
  EXPECT_EQ(serialized_object, nullptr);

  vault_response =
      "{ errors: [\"list is broken\",\n\"and some other error\"\n] }";

  EXPECT_CALL(*mock_curl, list_keys(_))
      .WillOnce(DoAll(SetArgPointee<0>(vault_response), Return(true)));

  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(logger)),
              log(MY_ERROR_LEVEL,
                  StrEq("Could not retrieve list of keys from Vault. Vault has "
                        "returned the following error(s): [\"list is "
                        "broken\",\"and some other error\"]")));

  EXPECT_TRUE(vault_io.get_serialized_object(&serialized_object));
  EXPECT_EQ(serialized_object, nullptr);
}

TEST_F(Vault_io_test, ErrorsFromVaultCurlOnReadKey) {
  delete vault_curl;
  Mock_vault_curl *mock_curl = new Mock_vault_curl();
  Vault_io vault_io(logger, mock_curl, vault_parser);

  EXPECT_CALL(*mock_curl, init(_)).WillOnce(Return(false));  // init successful
  EXPECT_FALSE(vault_io.init(&credential_file_url));

  IKey *key = nullptr;

  EXPECT_CALL(*mock_curl, read_key(_, _)).WillOnce(Return(true));
  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(logger)),
              log(MY_ERROR_LEVEL, StrEq("Could not read key from Vault.")));
  EXPECT_TRUE(vault_io.retrieve_key_type_and_data(key));
}

TEST_F(Vault_io_test, ErrorsFromVaultInVaultsCurlResponseOnReadKey) {
  delete vault_curl;
  Mock_vault_curl *mock_curl = new Mock_vault_curl();
  Vault_io vault_io(logger, mock_curl, vault_parser);

  EXPECT_CALL(*mock_curl, init(_)).WillOnce(Return(false));  // init successful
  EXPECT_FALSE(vault_io.init(&credential_file_url));

  IKey *key = nullptr;
  Secure_string vault_response("{ errors: [\"Cannot read this stuff\"] }");

  EXPECT_CALL(*mock_curl, read_key(_, _))
      .WillOnce(DoAll(SetArgPointee<1>(vault_response), Return(true)));
  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(logger)),
              log(MY_ERROR_LEVEL, StrEq("Could not read key from Vault. Vault "
                                        "has returned the following error(s):"
                                        " [\"Cannot read this stuff\"]")));
  EXPECT_TRUE(vault_io.retrieve_key_type_and_data(key));
}

TEST_F(Vault_io_test, ErrorsFromVaultCurlOnDeleteKey) {
  delete vault_curl;
  Mock_vault_curl *mock_curl = new Mock_vault_curl();
  Vault_io vault_io(logger, mock_curl, vault_parser);

  EXPECT_CALL(*mock_curl, init(_)).WillOnce(Return(false));  // init successful
  EXPECT_FALSE(vault_io.init(&credential_file_url));

  Vault_key key(key_1_id, "AES", "Arczi", "Artur", 5);
  key.set_key_operation(REMOVE_KEY);

  EXPECT_CALL(*mock_curl, delete_key(_, _)).WillOnce(Return(true));
  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(logger)),
              log(MY_ERROR_LEVEL, StrEq("Could not delete key from Vault.")));
  EXPECT_TRUE(vault_io.flush_to_storage(&key));
}

TEST_F(Vault_io_test, ErrorsFromVaultInVaultsCurlResponseOnDeleteKey) {
  delete vault_curl;
  Mock_vault_curl *mock_curl = new Mock_vault_curl();
  Vault_io vault_io(logger, mock_curl, vault_parser);

  EXPECT_CALL(*mock_curl, init(_)).WillOnce(Return(false));  // init successful
  EXPECT_FALSE(vault_io.init(&credential_file_url));

  Vault_key key(key_1_id, "AES", "Arczi", "Artur", 5);
  key.set_key_operation(REMOVE_KEY);
  Secure_string vault_response("{ errors: [\"Cannot delete this stuff\"] }");

  EXPECT_CALL(*mock_curl, delete_key(_, _))
      .WillOnce(DoAll(SetArgPointee<1>(vault_response), Return(false)));
  EXPECT_CALL(
      *(reinterpret_cast<Mock_logger *>(logger)),
      log(MY_ERROR_LEVEL, StrEq("Could not delete key from Vault. Vault has "
                                "returned the following error(s):"
                                " [\"Cannot delete this stuff\"]")));
  EXPECT_TRUE(vault_io.flush_to_storage(&key));
}

TEST_F(Vault_io_test, ErrorsFromVaultCurlOnWriteKey) {
  delete vault_curl;
  Mock_vault_curl *mock_curl = new Mock_vault_curl();
  Vault_io vault_io(logger, mock_curl, vault_parser);

  EXPECT_CALL(*mock_curl, init(_)).WillOnce(Return(false));  // init successful
  EXPECT_FALSE(vault_io.init(&credential_file_url));

  Vault_key key(key_1_id, "AES", "Arczi", "Artur", 5);
  key.set_key_operation(STORE_KEY);

  EXPECT_CALL(*mock_curl, write_key(_, _)).WillOnce(Return(true));
  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(logger)),
              log(MY_ERROR_LEVEL, StrEq("Could not write key to Vault.")));
  EXPECT_TRUE(vault_io.flush_to_storage(&key));
}

TEST_F(Vault_io_test, ErrorsFromVaultInVaultsCurlResponseOnWriteKey) {
  delete vault_curl;
  Mock_vault_curl *mock_curl = new Mock_vault_curl();
  Vault_io vault_io(logger, mock_curl, vault_parser);

  EXPECT_CALL(*mock_curl, init(_)).WillOnce(Return(false));  // init successful
  EXPECT_FALSE(vault_io.init(&credential_file_url));

  Vault_key key(key_1_id, "AES", "Arczi", "Artur", 5);
  key.set_key_operation(STORE_KEY);
  Secure_string vault_response("{ errors: [\"Cannot write this stuff\"] }");

  EXPECT_CALL(*mock_curl, write_key(_, _))
      .WillOnce(DoAll(SetArgPointee<1>(vault_response), Return(false)));
  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(logger)),
              log(MY_ERROR_LEVEL, StrEq("Could not write key to Vault. Vault "
                                        "has returned the following error(s):"
                                        " [\"Cannot write this stuff\"]")));
  EXPECT_TRUE(vault_io.flush_to_storage(&key));
}

class Mock_vault_parser : public IVault_parser {
 public:
  MOCK_METHOD2(parse_keys,
               bool(const Secure_string &payload, Vault_keys_list *keys));
  MOCK_METHOD2(parse_key_data, bool(const Secure_string &payload, IKey *key));
  MOCK_METHOD2(parse_key_signature, bool(const Secure_string &key_signature,
                                         KeyParameters *key_parameters));
  MOCK_METHOD2(parse_errors,
               bool(const Secure_string &payload, Secure_string *errors));
};

TEST_F(Vault_io_test, ErrorFromParseKeysOnGetSerializedObject) {
  delete vault_parser;

  Mock_vault_parser *mock_vault_parser = new Mock_vault_parser;
  Vault_io vault_io(logger, vault_curl, mock_vault_parser);

  EXPECT_FALSE(vault_io.init(&credential_file_url));

  // First add two keys into Vault
  Vault_key key1(key_1_id, "AES", "Arczi", "Artur", 5);
  EXPECT_TRUE(*key1.get_key_signature() ==
              get_key_signature(uuid, "key1", "Arczi"));
  key1.set_key_operation(STORE_KEY);
  EXPECT_FALSE(vault_io.flush_to_storage(&key1));
  Vault_key key2(key_2_id, "AES", "Kamil", "Kami", 4);
  EXPECT_TRUE(*key2.get_key_signature() ==
              get_key_signature(uuid, "key2", "Kamil"));
  key2.set_key_operation(STORE_KEY);
  EXPECT_FALSE(vault_io.flush_to_storage(&key2));
  // *****

  EXPECT_CALL(*mock_vault_parser, parse_keys(_, _)).WillOnce(Return(true));
  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(logger)),
              log(MY_ERROR_LEVEL,
                  StrEq("Could not retrieve list of keys from Vault.")));

  ISerialized_object *serialized_keys = nullptr;
  EXPECT_TRUE(vault_io.get_serialized_object(&serialized_keys));

  ASSERT_TRUE(serialized_keys == nullptr);

  // Now remove the keys
  Vault_key key1_to_remove(key1);
  key1_to_remove.set_key_operation(REMOVE_KEY);
  EXPECT_FALSE(vault_io.flush_to_storage(&key1_to_remove));
  Vault_key key2_to_remove(key2);
  key2_to_remove.set_key_operation(REMOVE_KEY);
  EXPECT_FALSE(vault_io.flush_to_storage(&key2_to_remove));
}

TEST_F(Vault_io_test, ErrorFromParseKeyDataOnRetrieveKeyTypeAndValue) {
  delete vault_parser;
  Mock_vault_parser *mock_vault_parser = new Mock_vault_parser;
  Vault_io vault_io(logger, vault_curl, mock_vault_parser);
  EXPECT_FALSE(vault_io.init(&credential_file_url));

  Vault_key key_to_store(key_1_id, "AES", "rob", "Robi", 4);
  key_to_store.set_key_operation(STORE_KEY);
  EXPECT_FALSE(vault_io.flush_to_storage(&key_to_store));

  Vault_key key(key_1_id, nullptr, "rob", nullptr, 0);
  EXPECT_CALL(*mock_vault_parser, parse_key_data(_, &key))
      .WillOnce(Return(true));
  EXPECT_CALL(*mock_vault_parser, parse_errors(_, _)).WillOnce(Return(false));
  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(logger)),
              log(MY_ERROR_LEVEL, StrEq("Could not read key from Vault.")));
  EXPECT_TRUE(vault_io.retrieve_key_type_and_data(&key));

  Vault_key key_to_remove(key_1_id, nullptr, "rob", nullptr, 0);
  key_to_remove.set_key_operation(REMOVE_KEY);
  EXPECT_FALSE(vault_io.flush_to_storage(&key_to_remove));
}

TEST_F(Vault_io_test,
       ErrorFromParseKeyDataAndParseErrorsOnRetrieveKeyTypeAndValue) {
  delete vault_parser;
  Mock_vault_parser *mock_vault_parser = new Mock_vault_parser;
  Vault_io vault_io(logger, vault_curl, mock_vault_parser);
  EXPECT_FALSE(vault_io.init(&credential_file_url));

  Vault_key key_to_store(key_1_id, "AES", "rob", "Robi", 4);
  key_to_store.set_key_operation(STORE_KEY);
  EXPECT_FALSE(vault_io.flush_to_storage(&key_to_store));

  Vault_key key(key_1_id, nullptr, "rob", nullptr, 0);
  EXPECT_CALL(*mock_vault_parser, parse_key_data(_, &key))
      .WillOnce(Return(true));
  EXPECT_CALL(*mock_vault_parser, parse_errors(_, _)).WillOnce(Return(true));
  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(logger)),
              log(MY_ERROR_LEVEL, StrEq("Could not read key from Vault. Error "
                                        "while parsing error messages")));
  EXPECT_TRUE(vault_io.retrieve_key_type_and_data(&key));

  Vault_key key_to_remove(key_1_id, nullptr, "rob", nullptr, 0);
  key_to_remove.set_key_operation(REMOVE_KEY);
  EXPECT_FALSE(vault_io.flush_to_storage(&key_to_remove));
}
}  // namespace keyring__vault_io_unittest

#ifndef MERGE_UNITTESTS
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  MY_INIT(argv[0]);
  my_testing::setup_server_for_unit_tests();
  curl_global_init(CURL_GLOBAL_DEFAULT);

  // create unique secret mount point for this test suite
  CURL *curl = curl_easy_init();
  if (curl == nullptr) {
    std::cout << "Could not initialize CURL session" << std::endl;
    curl_global_cleanup();
    return 1;
  }
  BOOST_SCOPE_EXIT(&curl) {
    curl_easy_cleanup(curl);
    curl_global_cleanup();
  }
  BOOST_SCOPE_EXIT_END

  keyring::ILogger *logger = new keyring::Mock_logger();
  keyring::Vault_mount vault_mount(curl, logger);
  std::string mount_point_path = "cicd/" + uuid;
  if (generate_credential_file(keyring__vault_io_unittest::credential_file_url,
                               CORRECT, mount_point_path)) {
    std::cout << "Could not generate credential file" << std::endl;
    return 2;
  }
  if (vault_mount.init(&keyring__vault_io_unittest::credential_file_url,
                       &mount_point_path)) {
    std::cout << "Could not initialized Vault_mount" << std::endl;
    return 3;
  }
  if (vault_mount.mount_secret_backend()) {
    std::cout << "Could not mount secret backend" << std::endl;
    return 4;
  }
  int ret = RUN_ALL_TESTS();

  // remove unique secret mount point
  if (vault_mount.unmount_secret_backend()) {
    std::cout << "Could not unmount secret backend" << std::endl;
  }
  delete logger;
  my_testing::teardown_server_for_unit_tests();

  return ret;
}
#endif  // MERGE_UNITTESTS
