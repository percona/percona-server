/* Copyright (c) 2018, 2021 Percona LLC and/or its affiliates. All rights
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

#include <boost/scope_exit.hpp>
#include <cstring>
#include <optional>
#include "generate_credential_file.h"
#include "i_vault_curl.h"
#include "incorrect_vault_key.h"
#include "mock_logger.h"
#include "vault_base64.h"
#include "vault_curl.h"
#include "vault_environment.h"
#include "vault_io.h"
#include "vault_mount.h"
#include "vault_parser_composer.h"
#include "vault_test_base.h"

extern std::string uuid;

namespace keyring__vault_io_unittest {
using namespace keyring;

using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::SetArgReferee;
using ::testing::StrEq;

class Mock_vault_curl : public IVault_curl {
 public:
  MOCK_METHOD1(init, bool(const Vault_credentials &vault_credentials));
  MOCK_METHOD1(list_keys, bool(Secure_string *response));
  MOCK_METHOD2(write_key, bool(const Vault_key &key, Secure_string *response));
  MOCK_METHOD2(read_key, bool(const Vault_key &key, Secure_string *response));
  MOCK_METHOD2(delete_key, bool(const Vault_key &key, Secure_string *response));
  MOCK_METHOD1(set_timeout, void(uint timeout));
  MOCK_CONST_METHOD0(get_resolved_secret_mount_point_version,
                     Vault_version_type());
};

class Mock_vault_parser_composer : public IVault_parser_composer {
 public:
  MOCK_METHOD2(parse_keys, bool(const Secure_string &payload,
                                VaultKeyFetchedCallback key_fetched_callback));
  MOCK_METHOD3(parse_key_data, bool(const Secure_string &payload, IKey *key,
                                    Vault_version_type vault_version));
  MOCK_METHOD2(parse_key_signature, bool(const Secure_string &key_signature,
                                         KeyParameters *key_parameters));
  MOCK_METHOD2(parse_errors,
               bool(const Secure_string &payload, Secure_string *errors));
  MOCK_METHOD4(parse_mount_point_config,
               bool(const Secure_string &config_payload,
                    std::size_t &max_versions, bool &cas_required,
                    Optional_secure_string &delete_version_after));
  MOCK_METHOD4(compose_write_key_postdata,
               bool(const Vault_key &key, const Secure_string &encoded_key_data,
                    Vault_version_type vault_version, Secure_string &postdata));
};

class Vault_io_test : public Vault_test_base {
 public:
  ~Vault_io_test() override = default;

 protected:
  void SetUp() override {
    if (!check_env_configured()) {
      GTEST_SKIP() << "The vault environment is not configured";
    }

    Vault_test_base::SetUp();
  }

  Vault_curl *create_vault_curl(IVault_parser_composer *parser) const {
    return new Vault_curl(get_logger(), parser, 0);
  }
  Mock_vault_curl *create_mock_vault_curl() const {
    return new Mock_vault_curl;
  }
  Vault_parser_composer *create_vault_parser_composer() const {
    return new Vault_parser_composer(get_logger());
  }
  Mock_vault_parser_composer *create_mock_vault_parser_composer() const {
    return new Mock_vault_parser_composer;
  }
};

TEST_F(Vault_io_test, InitWithNotExisitingCredentialFile) {
  std::string credential_file_name =
      get_vault_env()->get_non_existing_conf_file_name();
  IVault_parser_composer *parser = create_vault_parser_composer();
  Vault_io vault_io(get_logger(), create_vault_curl(parser), parser);
  remove(credential_file_name.c_str());
  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(get_logger())),
              log(MY_ERROR_LEVEL, StrEq("Could not open credentials file '" +
                                        credential_file_name + "'.")));
  EXPECT_TRUE(vault_io.init(&credential_file_name));

  remove(credential_file_name.c_str());
}

TEST_F(Vault_io_test, InitWithInvalidToken) {
  std::string conf_with_invalid_token =
      get_vault_env()->get_invalid_conf_file_name();
  ASSERT_FALSE(generate_credential_file(
      conf_with_invalid_token, "ut_tests_non_existing",
      mount_point_version_type::mount_point_version_v1,
      credentials_validity_type::credentials_validity_invalid_token));

  IVault_parser_composer *parser = create_vault_parser_composer();
  Vault_io vault_io(get_logger(), create_vault_curl(parser), parser);
  EXPECT_FALSE(vault_io.init(&conf_with_invalid_token));

  EXPECT_CALL(
      *(reinterpret_cast<Mock_logger *>(get_logger())),
      log(MY_ERROR_LEVEL, StrEq("Could not retrieve list of keys from Vault. "
                                "Vault has returned the following error(s): "
                                "permission denied")));
  ISerialized_object *serialized_keys = nullptr;
  EXPECT_TRUE(vault_io.get_serialized_object(&serialized_keys));

  remove(conf_with_invalid_token.c_str());
}

TEST_F(Vault_io_test, GetSerializedObjectWithTwoKeys) {
  Vault_parser_composer *parser = create_vault_parser_composer();
  Vault_io vault_io(get_logger(), create_vault_curl(parser), parser);

  EXPECT_FALSE(vault_io.init(&get_vault_env()->get_default_conf_file_name()));

  // First add two keys into Vault
  Vault_key key1(get_vault_env()->get_key1_id_raw(), "AES", "Arczi", "Artur",
                 5);
  EXPECT_TRUE(*key1.get_key_signature() ==
              get_vault_env()->get_key_signature("key1", "Arczi"));
  key1.set_key_operation(STORE_KEY);
  EXPECT_FALSE(vault_io.flush_to_storage(&key1));
  Vault_key key2(get_vault_env()->get_key2_id_raw(), "AES", "Kamil", "Kami", 4);
  EXPECT_TRUE(*key2.get_key_signature() ==
              get_vault_env()->get_key_signature("key2", "Kamil"));
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
                  get_vault_env()->get_key_signature("key1", "Arczi") ||
              *key1_loaded->get_key_signature() ==
                  get_vault_env()->get_key_signature("key2", "Kamil"));
  IKey *key2_loaded = nullptr;
  delete key1_loaded;
  EXPECT_TRUE(serialized_keys->has_next_key());
  serialized_keys->get_next_key(&key2_loaded);
  EXPECT_TRUE(*key2_loaded->get_key_signature() ==
                  get_vault_env()->get_key_signature("key2", "Kamil") ||
              *key2_loaded->get_key_signature() ==
                  get_vault_env()->get_key_signature("key1", "Arczi"));
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
  Vault_parser_composer *parser = create_vault_parser_composer();
  Vault_io vault_io_for_storing(get_logger(), create_vault_curl(parser),
                                parser);

  EXPECT_FALSE(vault_io_for_storing.init(
      &get_vault_env()->get_default_conf_file_name()));

  // First add two keys into Vault
  Vault_key key1(get_vault_env()->get_key1_id_raw(), "AES", "Robert", "Robi",
                 4);
  EXPECT_TRUE(*key1.get_key_signature() ==
              get_vault_env()->get_key_signature("key1", "Robert"));
  key1.set_key_operation(STORE_KEY);
  EXPECT_FALSE(vault_io_for_storing.flush_to_storage(&key1));
  Vault_key key2(get_vault_env()->get_key2_id_raw(), "AES", "Kamil", "Kami", 4);
  EXPECT_TRUE(*key2.get_key_signature() ==
              get_vault_env()->get_key_signature("key2", "Kamil"));
  key2.set_key_operation(STORE_KEY);
  EXPECT_FALSE(vault_io_for_storing.flush_to_storage(&key2));
  // *****

  // Now fetch two keys with separate Vault_io
  Vault_parser_composer *vault_parser2 =
      new Vault_parser_composer(get_logger());
  Vault_curl *vault_curl2 = new Vault_curl(get_logger(), vault_parser2, 0);
  Vault_io vault_io_for_fetching(get_logger(), vault_curl2, vault_parser2);
  EXPECT_FALSE(vault_io_for_fetching.init(
      &get_vault_env()->get_default_conf_file_name()));

  ISerialized_object *serialized_keys = nullptr;
  EXPECT_FALSE(vault_io_for_fetching.get_serialized_object(&serialized_keys));
  IKey *key1_loaded = nullptr;
  ASSERT_TRUE(serialized_keys != nullptr);
  EXPECT_TRUE(serialized_keys->has_next_key());
  serialized_keys->get_next_key(&key1_loaded);
  EXPECT_TRUE(*key1_loaded->get_key_signature() ==
                  get_vault_env()->get_key_signature("key1", "Robert") ||
              *key1_loaded->get_key_signature() ==
                  get_vault_env()->get_key_signature("key2", "Kamil"));
  IKey *key2_loaded = nullptr;
  delete key1_loaded;
  EXPECT_TRUE(serialized_keys->has_next_key());
  serialized_keys->get_next_key(&key2_loaded);
  EXPECT_TRUE(*key2_loaded->get_key_signature() ==
                  get_vault_env()->get_key_signature("key2", "Kamil") ||
              *key2_loaded->get_key_signature() ==
                  get_vault_env()->get_key_signature("key1", "Robert"));
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
  Vault_parser_composer *parser = create_vault_parser_composer();
  Vault_io vault_io_for_storing(get_logger(), create_vault_curl(parser),
                                parser);

  EXPECT_FALSE(vault_io_for_storing.init(
      &get_vault_env()->get_default_conf_file_name()));

  // First add two keys into Vault
  Vault_key key1(get_vault_env()->get_key1_id_raw(), "AES", "Robert", "Robi",
                 4);
  EXPECT_TRUE(*key1.get_key_signature() ==
              get_vault_env()->get_key_signature("key1", "Robert"));
  key1.set_key_operation(STORE_KEY);
  EXPECT_FALSE(vault_io_for_storing.flush_to_storage(&key1));
  Vault_key key2(get_vault_env()->get_key2_id_raw(), "AES", "Kamil", "Kami", 4);
  EXPECT_TRUE(*key2.get_key_signature() ==
              get_vault_env()->get_key_signature("key2", "Kamil"));
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
  Vault_parser_composer *vault_parser2 =
      new Vault_parser_composer(get_logger());
  Vault_curl *vault_curl2 = new Vault_curl(get_logger(), vault_parser2, 0);
  Vault_io vault_io_for_fetching(get_logger(), vault_curl2, vault_parser2);

  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(get_logger())),
              log(MY_WARNING_LEVEL,
                  StrEq("Could not parse key's signature, skipping the key.")));

  EXPECT_FALSE(vault_io_for_fetching.init(
      &get_vault_env()->get_default_conf_file_name()));

  ISerialized_object *serialized_keys = nullptr;
  EXPECT_FALSE(vault_io_for_fetching.get_serialized_object(&serialized_keys));
  IKey *key1_loaded = nullptr;
  ASSERT_TRUE(serialized_keys != nullptr);
  EXPECT_TRUE(serialized_keys->has_next_key());
  serialized_keys->get_next_key(&key1_loaded);
  EXPECT_TRUE(*key1_loaded->get_key_signature() ==
                  get_vault_env()->get_key_signature("key1", "Robert") ||
              *key1_loaded->get_key_signature() ==
                  get_vault_env()->get_key_signature("key2", "Kamil"));
  IKey *key2_loaded = nullptr;
  delete key1_loaded;
  EXPECT_TRUE(serialized_keys->has_next_key());
  serialized_keys->get_next_key(&key2_loaded);
  EXPECT_TRUE(*key2_loaded->get_key_signature() ==
                  get_vault_env()->get_key_signature("key2", "Kamil") ||
              *key2_loaded->get_key_signature() ==
                  get_vault_env()->get_key_signature("key1", "Robert"));
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
  Vault_parser_composer *parser = create_vault_parser_composer();
  Vault_io vault_io(get_logger(), create_vault_curl(parser), parser);
  EXPECT_FALSE(vault_io.init(&get_vault_env()->get_default_conf_file_name()));

  Vault_key key_to_store(get_vault_env()->get_key1_id_raw(), "AES", "rob",
                         "Robi", 4);
  key_to_store.set_key_operation(STORE_KEY);
  EXPECT_FALSE(vault_io.flush_to_storage(&key_to_store));

  Vault_key key(get_vault_env()->get_key1_id_raw(), nullptr, "rob", nullptr, 0);
  EXPECT_FALSE(vault_io.retrieve_key_type_and_data(&key));
  EXPECT_TRUE(*key.get_key_signature() ==
              get_vault_env()->get_key_signature("key1", "rob"));
  ASSERT_TRUE(memcmp(key.get_key_data(), "Robi", key.get_key_data_size()) == 0);
  EXPECT_STREQ("AES", key.get_key_type_as_string()->c_str());

  Vault_key key_to_remove(get_vault_env()->get_key1_id_raw(), nullptr, "rob",
                          nullptr, 0);
  key_to_remove.set_key_operation(REMOVE_KEY);
  EXPECT_FALSE(vault_io.flush_to_storage(&key_to_remove));
}

TEST_F(Vault_io_test, FlushAndRemoveSingleKey) {
  Vault_parser_composer *parser = create_vault_parser_composer();
  Vault_io vault_io(get_logger(), create_vault_curl(parser), parser);
  EXPECT_FALSE(vault_io.init(&get_vault_env()->get_default_conf_file_name()));
  Vault_key key(get_vault_env()->get_key1_id_raw(), "AES", "rob", "Robi", 4);
  key.set_key_operation(STORE_KEY);
  EXPECT_FALSE(vault_io.flush_to_storage(&key));
  Vault_key key_to_remove(key);
  key_to_remove.set_key_operation(REMOVE_KEY);
  EXPECT_FALSE(vault_io.flush_to_storage(&key_to_remove));
}

TEST_F(Vault_io_test, FlushKeyRetrieveDeleteInit) {
  Vault_parser_composer *parser = create_vault_parser_composer();
  Vault_io vault_io(get_logger(), create_vault_curl(parser), parser);
  EXPECT_FALSE(vault_io.init(&get_vault_env()->get_default_conf_file_name()));
  Vault_key key(get_vault_env()->get_key1_id_raw(), "AES", "rob", "Robi", 4);
  key.set_key_operation(STORE_KEY);
  EXPECT_FALSE(vault_io.flush_to_storage(&key));
  Vault_key key1_id(get_vault_env()->get_key1_id_raw(), nullptr, "rob", nullptr,
                    0);
  EXPECT_FALSE(vault_io.retrieve_key_type_and_data(&key1_id));
  EXPECT_TRUE(*key1_id.get_key_signature() ==
              get_vault_env()->get_key_signature("key1", "rob"));
  ASSERT_TRUE(
      memcmp(key1_id.get_key_data(), "Robi", key1_id.get_key_data_size()) == 0);
  EXPECT_STREQ("AES", key1_id.get_key_type_as_string()->c_str());

  Vault_key key_to_remove(key);
  key_to_remove.set_key_operation(REMOVE_KEY);
  EXPECT_FALSE(vault_io.flush_to_storage(&key_to_remove));

  Vault_parser_composer *vault_parser2 =
      new Vault_parser_composer(get_logger());
  Vault_curl *vault_curl2 = new Vault_curl(get_logger(), vault_parser2, 0);
  Vault_io vault_io2(get_logger(), vault_curl2, vault_parser2);
  EXPECT_FALSE(vault_io2.init(&get_vault_env()->get_default_conf_file_name()));
  ISerialized_object *serialized_keys = nullptr;
  EXPECT_FALSE(vault_io2.get_serialized_object(&serialized_keys));
  ASSERT_TRUE(serialized_keys == nullptr);  // no keys
}

TEST_F(Vault_io_test, ErrorFromVaultCurlOnVaultIOInit) {
  Mock_vault_curl *mock_curl = create_mock_vault_curl();
  Vault_io vault_io(get_logger(), mock_curl, create_vault_parser_composer());

  EXPECT_CALL(*mock_curl, init(_)).WillOnce(Return(true));  // init unsuccessful
  EXPECT_TRUE(vault_io.init(&get_vault_env()->get_default_conf_file_name()));
}

TEST_F(Vault_io_test, ErrorFromVaultCurlOnListKeys) {
  Mock_vault_curl *mock_curl = create_mock_vault_curl();
  Vault_io vault_io(get_logger(), mock_curl, create_vault_parser_composer());

  EXPECT_CALL(*mock_curl, init(_)).WillOnce(Return(false));  // init successful
  EXPECT_FALSE(vault_io.init(&get_vault_env()->get_default_conf_file_name()));

  ISerialized_object *serialized_object;

  EXPECT_CALL(*mock_curl, list_keys(_))
      .WillOnce(Return(true));  // failed to list keys
  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(get_logger())),
              log(MY_ERROR_LEVEL,
                  StrEq("Could not retrieve list of keys from Vault.")));

  EXPECT_TRUE(vault_io.get_serialized_object(&serialized_object));
  EXPECT_EQ(serialized_object, nullptr);
}

TEST_F(Vault_io_test, ErrorsFromVaultInVaultsResponseOnListKeys) {
  Mock_vault_curl *mock_curl = create_mock_vault_curl();
  Vault_io vault_io(get_logger(), mock_curl, create_vault_parser_composer());

  EXPECT_CALL(*mock_curl, init(_)).WillOnce(Return(false));  // init successful
  EXPECT_FALSE(vault_io.init(&get_vault_env()->get_default_conf_file_name()));

  ISerialized_object *serialized_object;
  Secure_string vault_response("{ \"errors\": [\"list is broken\"] }");

  EXPECT_CALL(*mock_curl, list_keys(_))
      .WillOnce(DoAll(SetArgPointee<0>(vault_response), Return(true)));

  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(get_logger())),
              log(MY_ERROR_LEVEL,
                  StrEq("Could not retrieve list of keys from Vault. Vault has "
                        "returned the following error(s): list is broken")));

  EXPECT_TRUE(vault_io.get_serialized_object(&serialized_object));
  EXPECT_EQ(serialized_object, nullptr);

  vault_response =
      "{\"errors\": [\"list is broken\", \"and some other error\"]}";

  EXPECT_CALL(*mock_curl, list_keys(_))
      .WillOnce(DoAll(SetArgPointee<0>(vault_response), Return(true)));

  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(get_logger())),
              log(MY_ERROR_LEVEL,
                  StrEq("Could not retrieve list of keys from Vault. Vault "
                        "has returned the following error(s): list is "
                        "broken\nand some other error")));

  EXPECT_TRUE(vault_io.get_serialized_object(&serialized_object));
  EXPECT_EQ(serialized_object, nullptr);

  vault_response =
      "{ \"errors\": [\"list is broken\",\n\"and yet another error\"\n] }";

  EXPECT_CALL(*mock_curl, list_keys(_))
      .WillOnce(DoAll(SetArgPointee<0>(vault_response), Return(true)));

  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(get_logger())),
              log(MY_ERROR_LEVEL,
                  StrEq("Could not retrieve list of keys from Vault. Vault "
                        "has returned the following error(s): list is "
                        "broken\nand yet another error")));

  EXPECT_TRUE(vault_io.get_serialized_object(&serialized_object));
  EXPECT_EQ(serialized_object, nullptr);
}

TEST_F(Vault_io_test, ErrorsFromVaultCurlOnReadKey) {
  Mock_vault_curl *mock_curl = create_mock_vault_curl();
  Vault_io vault_io(get_logger(), mock_curl, create_vault_parser_composer());

  EXPECT_CALL(*mock_curl, init(_)).WillOnce(Return(false));  // init successful
  EXPECT_FALSE(vault_io.init(&get_vault_env()->get_default_conf_file_name()));

  IKey *key = nullptr;

  EXPECT_CALL(*mock_curl, read_key(_, _)).WillOnce(Return(true));
  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(get_logger())),
              log(MY_ERROR_LEVEL, StrEq("Could not read key from Vault.")));
  EXPECT_TRUE(vault_io.retrieve_key_type_and_data(key));
}

TEST_F(Vault_io_test, ErrorsFromVaultInVaultsCurlResponseOnReadKey) {
  Mock_vault_curl *mock_curl = create_mock_vault_curl();
  Vault_io vault_io(get_logger(), mock_curl, create_vault_parser_composer());

  EXPECT_CALL(*mock_curl, init(_)).WillOnce(Return(false));  // init successful
  EXPECT_FALSE(vault_io.init(&get_vault_env()->get_default_conf_file_name()));

  IKey *key = nullptr;
  Secure_string vault_response("{ \"errors\": [\"Cannot read this stuff\"] }");

  EXPECT_CALL(*mock_curl, read_key(_, _))
      .WillOnce(DoAll(SetArgPointee<1>(vault_response), Return(true)));
  EXPECT_CALL(
      *(reinterpret_cast<Mock_logger *>(get_logger())),
      log(MY_ERROR_LEVEL, StrEq("Could not read key from Vault. Vault has "
                                "returned the following error(s):"
                                " Cannot read this stuff")));
  EXPECT_TRUE(vault_io.retrieve_key_type_and_data(key));
}

TEST_F(Vault_io_test, ErrorsFromVaultCurlOnDeleteKey) {
  Mock_vault_curl *mock_curl = create_mock_vault_curl();
  Vault_io vault_io(get_logger(), mock_curl, create_vault_parser_composer());

  EXPECT_CALL(*mock_curl, init(_)).WillOnce(Return(false));  // init successful
  EXPECT_FALSE(vault_io.init(&get_vault_env()->get_default_conf_file_name()));

  Vault_key key(get_vault_env()->get_key1_id_raw(), "AES", "Arczi", "Artur", 5);
  key.set_key_operation(REMOVE_KEY);

  EXPECT_CALL(*mock_curl, delete_key(_, _)).WillOnce(Return(true));
  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(get_logger())),
              log(MY_ERROR_LEVEL, StrEq("Could not delete key from Vault.")));
  EXPECT_TRUE(vault_io.flush_to_storage(&key));
}

TEST_F(Vault_io_test, ErrorsFromVaultInVaultsCurlResponseOnDeleteKey) {
  Mock_vault_curl *mock_curl = create_mock_vault_curl();
  Vault_io vault_io(get_logger(), mock_curl, create_vault_parser_composer());

  EXPECT_CALL(*mock_curl, init(_)).WillOnce(Return(false));  // init successful
  EXPECT_FALSE(vault_io.init(&get_vault_env()->get_default_conf_file_name()));

  Vault_key key(get_vault_env()->get_key1_id_raw(), "AES", "Arczi", "Artur", 5);
  key.set_key_operation(REMOVE_KEY);
  Secure_string vault_response(
      "{ \"errors\": [\"Cannot delete this stuff\"] }");

  EXPECT_CALL(*mock_curl, delete_key(_, _))
      .WillOnce(DoAll(SetArgPointee<1>(vault_response), Return(false)));
  EXPECT_CALL(
      *(reinterpret_cast<Mock_logger *>(get_logger())),
      log(MY_ERROR_LEVEL, StrEq("Could not delete key from Vault. Vault "
                                "has returned the following error(s):"
                                " Cannot delete this stuff")));
  EXPECT_TRUE(vault_io.flush_to_storage(&key));
}

TEST_F(Vault_io_test, ErrorsFromVaultCurlOnWriteKey) {
  Mock_vault_curl *mock_curl = create_mock_vault_curl();
  Vault_io vault_io(get_logger(), mock_curl, create_vault_parser_composer());

  EXPECT_CALL(*mock_curl, init(_)).WillOnce(Return(false));  // init successful
  EXPECT_FALSE(vault_io.init(&get_vault_env()->get_default_conf_file_name()));

  Vault_key key(get_vault_env()->get_key1_id_raw(), "AES", "Arczi", "Artur", 5);
  key.set_key_operation(STORE_KEY);

  EXPECT_CALL(*mock_curl, write_key(_, _)).WillOnce(Return(true));
  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(get_logger())),
              log(MY_ERROR_LEVEL, StrEq("Could not write key to Vault.")));
  EXPECT_TRUE(vault_io.flush_to_storage(&key));
}

TEST_F(Vault_io_test, ErrorsFromVaultInVaultsCurlResponseOnWriteKey) {
  Mock_vault_curl *mock_curl = create_mock_vault_curl();
  Vault_io vault_io(get_logger(), mock_curl, create_vault_parser_composer());

  EXPECT_CALL(*mock_curl, init(_)).WillOnce(Return(false));  // init successful
  EXPECT_FALSE(vault_io.init(&get_vault_env()->get_default_conf_file_name()));

  Vault_key key(get_vault_env()->get_key1_id_raw(), "AES", "Arczi", "Artur", 5);
  key.set_key_operation(STORE_KEY);
  Secure_string vault_response("{ \"errors\": [\"Cannot write this stuff\"] }");

  EXPECT_CALL(*mock_curl, write_key(_, _))
      .WillOnce(DoAll(SetArgPointee<1>(vault_response), Return(false)));
  EXPECT_CALL(
      *(reinterpret_cast<Mock_logger *>(get_logger())),
      log(MY_ERROR_LEVEL, StrEq("Could not write key to Vault. Vault has "
                                "returned the following error(s):"
                                " Cannot write this stuff")));
  EXPECT_TRUE(vault_io.flush_to_storage(&key));
}

TEST_F(Vault_io_test, ErrorFromParseKeysOnGetSerializedObject) {
  Vault_parser_composer real_parser(get_logger());
  Mock_vault_parser_composer *mock_vault_parser =
      create_mock_vault_parser_composer();
  Vault_curl *real_vault_curl = create_vault_curl(mock_vault_parser);
  Secure_string postdata1, postdata2;
  Secure_string encoded_key_data1, encoded_key_data2;
  Vault_io vault_io(get_logger(), real_vault_curl, mock_vault_parser);

  EXPECT_FALSE(vault_io.init(&get_vault_env()->get_default_conf_file_name()));

  // First add two keys into Vault
  Vault_key key1(get_vault_env()->get_key1_id_raw(), "AES", "Arczi", "Artur",
                 5);
  Vault_key key2(get_vault_env()->get_key2_id_raw(), "AES", "Kamil", "Kami", 4);

  EXPECT_FALSE(
      Vault_base64::encode(reinterpret_cast<const char *>(key1.get_key_data()),
                           key1.get_key_data_size(), &encoded_key_data1,
                           Vault_base64::Format::SINGLE_LINE));
  EXPECT_FALSE(real_parser.compose_write_key_postdata(
      key1, encoded_key_data1,
      real_vault_curl->get_resolved_secret_mount_point_version(), postdata1));
  EXPECT_FALSE(
      Vault_base64::encode(reinterpret_cast<const char *>(key2.get_key_data()),
                           key2.get_key_data_size(), &encoded_key_data2,
                           Vault_base64::Format::SINGLE_LINE));
  EXPECT_FALSE(real_parser.compose_write_key_postdata(
      key2, encoded_key_data2,
      real_vault_curl->get_resolved_secret_mount_point_version(), postdata2));

  EXPECT_CALL(*mock_vault_parser, compose_write_key_postdata(_, _, _, _))
      .WillOnce(DoAll(SetArgReferee<3>(postdata1), Return(false)))
      .WillOnce(DoAll(SetArgReferee<3>(postdata2), Return(false)));

  EXPECT_TRUE(*key1.get_key_signature() ==
              get_vault_env()->get_key_signature("key1", "Arczi"));
  key1.set_key_operation(STORE_KEY);
  EXPECT_FALSE(vault_io.flush_to_storage(&key1));

  EXPECT_TRUE(*key2.get_key_signature() ==
              get_vault_env()->get_key_signature("key2", "Kamil"));
  key2.set_key_operation(STORE_KEY);
  EXPECT_FALSE(vault_io.flush_to_storage(&key2));
  // *****

  EXPECT_CALL(*mock_vault_parser, parse_keys(_, _)).WillOnce(Return(true));
  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(get_logger())),
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
  Vault_parser_composer real_parser(get_logger());
  Mock_vault_parser_composer *mock_vault_parser =
      create_mock_vault_parser_composer();
  Vault_curl *real_vault_curl = create_vault_curl(mock_vault_parser);
  Secure_string postdata;
  Secure_string encoded_key_data;

  Vault_io vault_io(get_logger(), real_vault_curl, mock_vault_parser);
  EXPECT_FALSE(vault_io.init(&get_vault_env()->get_default_conf_file_name()));

  Vault_key key_to_store(get_vault_env()->get_key1_id_raw(), "AES", "rob",
                         "Robi", 4);

  EXPECT_FALSE(Vault_base64::encode(
      reinterpret_cast<const char *>(key_to_store.get_key_data()),
      key_to_store.get_key_data_size(), &encoded_key_data,
      Vault_base64::Format::SINGLE_LINE));
  EXPECT_FALSE(real_parser.compose_write_key_postdata(
      key_to_store, encoded_key_data,
      real_vault_curl->get_resolved_secret_mount_point_version(), postdata));

  EXPECT_CALL(*mock_vault_parser, compose_write_key_postdata(_, _, _, _))
      .WillOnce(DoAll(SetArgReferee<3>(postdata), Return(false)));

  key_to_store.set_key_operation(STORE_KEY);
  EXPECT_FALSE(vault_io.flush_to_storage(&key_to_store));

  Vault_key key(get_vault_env()->get_key1_id_raw(), nullptr, "rob", nullptr, 0);
  EXPECT_CALL(*mock_vault_parser, parse_key_data(_, &key, Vault_version_v1))
      .WillOnce(Return(true));
  EXPECT_CALL(*mock_vault_parser, parse_errors(_, _)).WillOnce(Return(false));
  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(get_logger())),
              log(MY_ERROR_LEVEL, StrEq("Could not read key from Vault.")));
  EXPECT_TRUE(vault_io.retrieve_key_type_and_data(&key));

  Vault_key key_to_remove(get_vault_env()->get_key1_id_raw(), nullptr, "rob",
                          nullptr, 0);
  key_to_remove.set_key_operation(REMOVE_KEY);
  EXPECT_FALSE(vault_io.flush_to_storage(&key_to_remove));
}

TEST_F(Vault_io_test,
       ErrorFromParseKeyDataAndParseErrorsOnRetrieveKeyTypeAndValue) {
  Vault_parser_composer real_parser(get_logger());
  Mock_vault_parser_composer *mock_vault_parser =
      create_mock_vault_parser_composer();
  Vault_curl *real_vault_curl = create_vault_curl(mock_vault_parser);
  Secure_string postdata;
  Secure_string encoded_key_data;

  Vault_io vault_io(get_logger(), real_vault_curl, mock_vault_parser);
  EXPECT_FALSE(vault_io.init(&get_vault_env()->get_default_conf_file_name()));

  Vault_key key_to_store(get_vault_env()->get_key1_id_raw(), "AES", "rob",
                         "Robi", 4);

  EXPECT_FALSE(Vault_base64::encode(
      reinterpret_cast<const char *>(key_to_store.get_key_data()),
      key_to_store.get_key_data_size(), &encoded_key_data,
      Vault_base64::Format::SINGLE_LINE));
  EXPECT_FALSE(real_parser.compose_write_key_postdata(
      key_to_store, encoded_key_data,
      real_vault_curl->get_resolved_secret_mount_point_version(), postdata));

  EXPECT_CALL(*mock_vault_parser, compose_write_key_postdata(_, _, _, _))
      .WillOnce(DoAll(SetArgReferee<3>(postdata), Return(false)));

  key_to_store.set_key_operation(STORE_KEY);
  EXPECT_FALSE(vault_io.flush_to_storage(&key_to_store));

  Vault_key key(get_vault_env()->get_key1_id_raw(), nullptr, "rob", nullptr, 0);
  EXPECT_CALL(*mock_vault_parser, parse_key_data(_, &key, Vault_version_v1))
      .WillOnce(Return(true));
  EXPECT_CALL(*mock_vault_parser, parse_errors(_, _)).WillOnce(Return(true));
  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(get_logger())),
              log(MY_ERROR_LEVEL, StrEq("Could not read key from Vault. Error "
                                        "while parsing error messages")));
  EXPECT_TRUE(vault_io.retrieve_key_type_and_data(&key));

  Vault_key key_to_remove(get_vault_env()->get_key1_id_raw(), nullptr, "rob",
                          nullptr, 0);
  key_to_remove.set_key_operation(REMOVE_KEY);
  EXPECT_FALSE(vault_io.flush_to_storage(&key_to_remove));
}
}  // namespace keyring__vault_io_unittest
