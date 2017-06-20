#include <my_global.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sql_plugin_ref.h>
#include <fstream>
#include "vault_keyring.cc"
#include "keyring_impl.cc"
#include "mock_logger.h"
#include "uuid.h"
#include "generate_credential_file.h"
#include "test_utils.h"
#include "vault_mount.h"

namespace keyring__api_unittest
{
  using ::testing::StrEq;
  using namespace keyring;

  static std::string uuid = generate_uuid();
  static char *keyring_filename;

  class Keyring_vault_api_test : public ::testing::Test
  {
  public:
    Keyring_vault_api_test()
    {}
    ~Keyring_vault_api_test()
    {
      delete[] plugin_name;
    }

    static std::string correct_token;

  protected:
    virtual void SetUp()
    {
      plugin_name= new char[strlen("FakeKeyring")+1];
      strcpy(plugin_name, "FakeKeyring");

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
    st_plugin_int plugin_info; // for Logger initialization
  };

  std::string Keyring_vault_api_test::correct_token;

  void Keyring_vault_api_test::keyring_init_with_mock_logger()
  {
    ASSERT_FALSE(keyring_vault_init(&plugin_info));
    // use MockLogger instead of Logger
    logger.reset(new Mock_logger());
  }

  void Keyring_vault_api_test::keyring_deinit_with_mock_logger()
  {
    keyring_vault_deinit(NULL);
  }

  TEST_F(Keyring_vault_api_test, StoreFetchRemove)
  {
    EXPECT_FALSE(mysql_key_store((uuid+"Robert_key").c_str(), "AES", "Robert", sample_key_data.c_str(),
                              sample_key_data.length()));
    char *key_type;
    size_t key_len;
    void *key;
    EXPECT_FALSE(mysql_key_fetch((uuid+"Robert_key").c_str(), &key_type, "Robert", &key,
                              &key_len));
    EXPECT_STREQ("AES", key_type);
    EXPECT_EQ(key_len, sample_key_data.length());
    ASSERT_TRUE(memcmp((char *)key, sample_key_data.c_str(), key_len) == 0);
    my_free(key_type);
    key_type= NULL;
    my_free(key);
    key= NULL;
    EXPECT_FALSE(mysql_key_remove((uuid+"Robert_key").c_str(), "Robert"));
    // make sure the key was removed - fetch it
    EXPECT_FALSE(mysql_key_fetch((uuid+"Robert_key").c_str(), &key_type, "Robert", &key,
                                 &key_len));
    ASSERT_TRUE(key == NULL);
  }

  TEST_F(Keyring_vault_api_test, CheckIfInmemoryKeyIsNOTXORed)
  {
    EXPECT_FALSE(mysql_key_store((uuid+"Robert_key").c_str(), "AES", "Robert", sample_key_data.c_str(),
                              sample_key_data.length()));

    Vault_key key_id((uuid+"Robert_key").c_str(), NULL, "Robert",NULL,0);
    IKey* fetched_key = keys->fetch_key(&key_id);
    ASSERT_TRUE(fetched_key != NULL);
    std::string expected_key_signature = get_key_signature(uuid, "Robert_key", "Robert");
    EXPECT_STREQ(fetched_key->get_key_signature()->c_str(), expected_key_signature.c_str());
    EXPECT_EQ(fetched_key->get_key_signature()->length(), expected_key_signature.length());
    uchar* key_data_fetched = fetched_key->get_key_data();
    size_t key_data_fetched_size = fetched_key->get_key_data_size();
    EXPECT_STREQ("AES", fetched_key->get_key_type()->c_str());
    ASSERT_TRUE(memcmp(sample_key_data.c_str(), key_data_fetched, key_data_fetched_size) == 0);
    ASSERT_TRUE(sample_key_data.length() == key_data_fetched_size);
    my_free(fetched_key->release_key_data());

    // clean up
    EXPECT_FALSE(mysql_key_remove((uuid+"Robert_key").c_str(), "Robert"));
  }

  TEST_F(Keyring_vault_api_test, FetchNotExisting)
  {
    char *key_type= NULL;
    void *key= NULL;
    size_t key_len= 0;
    EXPECT_FALSE(mysql_key_fetch((uuid+"Robert_key").c_str(), &key_type, "Robert", &key,
                                 &key_len));
    ASSERT_TRUE(key == NULL);
  }

  TEST_F(Keyring_vault_api_test, RemoveNotExisting)
  {
    EXPECT_TRUE(mysql_key_remove((uuid+"Robert_key").c_str(), "Robert"));
  }

  TEST_F(Keyring_vault_api_test, StoreFetchNotExisting)
  {
    EXPECT_FALSE(mysql_key_store((uuid+"Robert_key").c_str(), "AES", "Robert", sample_key_data.c_str(),
                                 sample_key_data.length()));
    char *key_type;
    size_t key_len;
    void *key;
    EXPECT_FALSE(mysql_key_fetch("NotExisting", &key_type, "Robert", &key, &key_len));
    ASSERT_TRUE(key == NULL);

    EXPECT_FALSE(mysql_key_remove((uuid+"Robert_key").c_str(), "Robert"));
  }

  TEST_F(Keyring_vault_api_test, StoreStoreStoreFetchRemove)
  {
    std::string key_data1("Robi1");
    std::string key_data2("Robi2");

    EXPECT_FALSE(mysql_key_store((uuid+"Robert_key").c_str(), "AES", "Robert", sample_key_data.c_str(),
                                 sample_key_data.length()));
    EXPECT_FALSE(mysql_key_store((uuid+"Robert_key1").c_str(), "AES", "Robert", key_data1.c_str(),
                                 key_data1.length()));
    EXPECT_FALSE(mysql_key_store((uuid+"Robert_key2").c_str(), "AES", "Robert", key_data2.c_str(),
                                 key_data2.length()));
    char *key_type;
    size_t key_len;
    void *key;
    EXPECT_FALSE(mysql_key_fetch((uuid+"Robert_key1").c_str(), &key_type, "Robert", &key,
                                 &key_len));
    EXPECT_STREQ("AES", key_type);
    EXPECT_EQ(key_len, key_data1.length());
    ASSERT_TRUE(memcmp((char *)key, key_data1.c_str(), key_len) == 0);
    my_free(key_type);
    key_type= NULL;
    my_free(key);
    key= NULL;
    EXPECT_FALSE(mysql_key_remove((uuid+"Robert_key2").c_str(), "Robert"));
    // make sure the key was removed - fetch it
    EXPECT_FALSE(mysql_key_fetch((uuid+"Robert_key2").c_str(), &key_type, "Robert", &key,
                                 &key_len));
    ASSERT_TRUE(key == NULL);

    EXPECT_FALSE(mysql_key_remove((uuid+"Robert_key").c_str(), "Robert"));
    EXPECT_FALSE(mysql_key_remove((uuid+"Robert_key1").c_str(), "Robert"));
  }

  TEST_F(Keyring_vault_api_test, StoreValidTypes)
  {
    EXPECT_FALSE(mysql_key_store((uuid+"Robert_key").c_str(), "AES", "Robert", sample_key_data.c_str(),
                                 sample_key_data.length()));
    EXPECT_FALSE(mysql_key_store((uuid+"Robert_key3").c_str(), "RSA", "Robert", sample_key_data.c_str(),
                                 sample_key_data.length()));
    EXPECT_FALSE(mysql_key_store((uuid+"Robert_key4").c_str(), "DSA", "Robert", sample_key_data.c_str(),
                                 sample_key_data.length()));
    // clean up
    EXPECT_FALSE(mysql_key_remove((uuid+"Robert_key").c_str(), "Robert"));
    EXPECT_FALSE(mysql_key_remove((uuid+"Robert_key3").c_str(), "Robert"));
    EXPECT_FALSE(mysql_key_remove((uuid+"Robert_key4").c_str(), "Robert"));

  }

  TEST_F(Keyring_vault_api_test, StoreInvalidType)
  {
    EXPECT_CALL(*(reinterpret_cast<Mock_logger*>(logger.get())),
                log(MY_ERROR_LEVEL, StrEq("Error while storing key: invalid key_type")));
    EXPECT_TRUE(mysql_key_store((uuid+"Robert_key").c_str(), "YYY", "Robert", sample_key_data.c_str(),
                                sample_key_data.length()));
    char *key_type;
    size_t key_len;
    void *key;
    EXPECT_FALSE(mysql_key_fetch((uuid+"Robert_key").c_str(), &key_type, "Robert", &key,
                                 &key_len));
    ASSERT_TRUE(key == NULL);
  }

  TEST_F(Keyring_vault_api_test, StoreTwiceTheSameDifferentTypes)
  {
    EXPECT_FALSE(mysql_key_store((uuid+"Robert_key").c_str(), "AES", "Robert", sample_key_data.c_str(),
                                 sample_key_data.length()));
    EXPECT_TRUE(mysql_key_store((uuid+"Robert_key").c_str(), "RSA", "Robert", sample_key_data.c_str(),
                                sample_key_data.length()));
    EXPECT_FALSE(mysql_key_remove((uuid+"Robert_key").c_str(), "Robert"));
  }

  TEST_F(Keyring_vault_api_test, KeyGenerate)
  {
    EXPECT_FALSE(mysql_key_generate((uuid+"Robert_key").c_str(), "AES", "Robert", 128));
    char *key_type;
    size_t key_len;
    void *key;
    EXPECT_FALSE(mysql_key_fetch((uuid+"Robert_key").c_str(), &key_type, "Robert", &key,
                                 &key_len));
    EXPECT_STREQ("AES", key_type);
    EXPECT_EQ(key_len, (size_t)128);
    // Try accessing the last byte of key
    volatile char ch= ((char*)key)[key_len-1];
    // Just to get rid of unused variable compiler error
    (void)ch;
    my_free(key);
    my_free(key_type);
    EXPECT_FALSE(mysql_key_remove((uuid+"Robert_key").c_str(), "Robert"));
  }

  TEST_F(Keyring_vault_api_test, NullUser)
  {
    EXPECT_FALSE(mysql_key_store((uuid+"Robert_key").c_str(), "AES", NULL, sample_key_data.c_str(),
                                 sample_key_data.length() + 1));
    char *key_type;
    size_t key_len;
    void *key;
    EXPECT_FALSE(mysql_key_fetch((uuid+"Robert_key").c_str(), &key_type, NULL, &key,
                              &key_len));
    EXPECT_STREQ("AES", key_type);
    EXPECT_EQ(key_len, sample_key_data.length()+1);
    ASSERT_TRUE(memcmp((char *)key, sample_key_data.c_str(), key_len) == 0);
    my_free(key_type);
    key_type= NULL;
    my_free(key);
    key= NULL;
    EXPECT_TRUE(mysql_key_store((uuid+"Robert_key").c_str(), "RSA", NULL, sample_key_data.c_str(),
                                sample_key_data.length() + 1));
    EXPECT_FALSE(mysql_key_store((uuid+"Kamil_key").c_str(), "AES", NULL, sample_key_data.c_str(),
                                 sample_key_data.length() + 1));
    EXPECT_FALSE(mysql_key_fetch((uuid+"Kamil_key").c_str(), &key_type, NULL, &key,
                                 &key_len));
    EXPECT_STREQ("AES", key_type);
    EXPECT_EQ(key_len, sample_key_data.length()+1);
    ASSERT_TRUE(memcmp((char *)key, sample_key_data.c_str(), key_len) == 0);
    my_free(key_type);
    key_type= NULL;
    my_free(key);
    key= NULL;
    EXPECT_FALSE(mysql_key_store((uuid+"Artur_key").c_str(), "AES", "Artur", sample_key_data.c_str(),
                                 sample_key_data.length() + 1));
    EXPECT_FALSE(mysql_key_fetch((uuid+"Artur_key").c_str(), &key_type, "Artur", &key,
                                 &key_len));
    EXPECT_STREQ("AES", key_type);
    EXPECT_EQ(key_len, sample_key_data.length()+1);
    ASSERT_TRUE(memcmp((char *)key, sample_key_data.c_str(), key_len) == 0);
    my_free(key_type);
    key_type= NULL;
    my_free(key);
    key= NULL;
    EXPECT_FALSE(mysql_key_remove((uuid+"Robert_key").c_str(), NULL));
    EXPECT_FALSE(mysql_key_fetch((uuid+"Robert_key").c_str(), &key_type, "Robert", &key,
                                 &key_len));
    ASSERT_TRUE(key == NULL);
    EXPECT_FALSE(mysql_key_fetch((uuid+"Artur_key").c_str(), &key_type, "Artur", &key,
                                 &key_len));
    EXPECT_STREQ("AES", key_type);
    EXPECT_EQ(key_len, sample_key_data.length()+1);
    ASSERT_TRUE(memcmp((char *)key, sample_key_data.c_str(), key_len) == 0);
    my_free(key_type);
    key_type= NULL;
    my_free(key);
    key= NULL;
    
    EXPECT_FALSE(mysql_key_remove((uuid+"Kamil_key").c_str(), NULL));
    EXPECT_FALSE(mysql_key_remove((uuid+"Artur_key").c_str(), "Artur"));
  }

  TEST_F(Keyring_vault_api_test, NullKeyId)
  {
    EXPECT_CALL(*(reinterpret_cast<Mock_logger*>(logger.get())), log(MY_ERROR_LEVEL, StrEq("Error while storing key: key_id cannot be empty")));
    EXPECT_TRUE(mysql_key_store(NULL, "AES", "Robert", sample_key_data.c_str(),
                                sample_key_data.length() + 1));
    EXPECT_CALL(*(reinterpret_cast<Mock_logger*>(logger.get())), log(MY_ERROR_LEVEL, StrEq("Error while storing key: key_id cannot be empty")));
    EXPECT_TRUE(mysql_key_store(NULL, "AES", NULL, sample_key_data.c_str(),
                                sample_key_data.length() + 1));
    EXPECT_CALL(*(reinterpret_cast<Mock_logger*>(logger.get())), log(MY_ERROR_LEVEL, StrEq("Error while storing key: key_id cannot be empty")));
    EXPECT_TRUE(mysql_key_store("", "AES", "Robert", sample_key_data.c_str(),
                                sample_key_data.length() + 1));
    EXPECT_CALL(*(reinterpret_cast<Mock_logger*>(logger.get())), log(MY_ERROR_LEVEL, StrEq("Error while storing key: key_id cannot be empty")));
    EXPECT_TRUE(mysql_key_store("", "AES", NULL, sample_key_data.c_str(),
                                sample_key_data.length() + 1));
    char *key_type;
    size_t key_len;
    void *key;
    EXPECT_CALL(*(reinterpret_cast<Mock_logger*>(logger.get())), log(MY_ERROR_LEVEL, StrEq("Error while fetching key: key_id cannot be empty")));
    EXPECT_TRUE(mysql_key_fetch(NULL, &key_type, "Robert", &key, &key_len));
    EXPECT_CALL(*(reinterpret_cast<Mock_logger*>(logger.get())), log(MY_ERROR_LEVEL, StrEq("Error while fetching key: key_id cannot be empty")));
    EXPECT_TRUE(mysql_key_fetch(NULL, &key_type, NULL, &key, &key_len));
    EXPECT_CALL(*(reinterpret_cast<Mock_logger*>(logger.get())), log(MY_ERROR_LEVEL, StrEq("Error while fetching key: key_id cannot be empty")));
    EXPECT_TRUE(mysql_key_fetch("", &key_type, "Robert", &key, &key_len));
    EXPECT_CALL(*(reinterpret_cast<Mock_logger*>(logger.get())), log(MY_ERROR_LEVEL, StrEq("Error while fetching key: key_id cannot be empty")));
    EXPECT_TRUE(mysql_key_fetch("", &key_type, NULL, &key, &key_len));

    EXPECT_CALL(*(reinterpret_cast<Mock_logger*>(logger.get())), log(MY_ERROR_LEVEL, StrEq("Error while removing key: key_id cannot be empty")));
    EXPECT_TRUE(mysql_key_remove(NULL, "Robert") );
    EXPECT_CALL(*(reinterpret_cast<Mock_logger*>(logger.get())), log(MY_ERROR_LEVEL, StrEq("Error while removing key: key_id cannot be empty")));
    EXPECT_TRUE(mysql_key_remove(NULL, NULL) );
    EXPECT_CALL(*(reinterpret_cast<Mock_logger*>(logger.get())), log(MY_ERROR_LEVEL, StrEq("Error while removing key: key_id cannot be empty")));
    EXPECT_TRUE(mysql_key_remove("", "Robert") );
    EXPECT_CALL(*(reinterpret_cast<Mock_logger*>(logger.get())), log(MY_ERROR_LEVEL, StrEq("Error while removing key: key_id cannot be empty")));
    EXPECT_TRUE(mysql_key_remove("", NULL) );

    EXPECT_CALL(*(reinterpret_cast<Mock_logger*>(logger.get())), log(MY_ERROR_LEVEL, StrEq("Error while generating key: key_id cannot be empty")));
    EXPECT_TRUE(mysql_key_generate(NULL, "AES", "Robert", 128));
    EXPECT_CALL(*(reinterpret_cast<Mock_logger*>(logger.get())), log(MY_ERROR_LEVEL, StrEq("Error while generating key: key_id cannot be empty")));
    EXPECT_TRUE(mysql_key_generate(NULL, "AES", NULL, 128));
    EXPECT_CALL(*(reinterpret_cast<Mock_logger*>(logger.get())), log(MY_ERROR_LEVEL, StrEq("Error while generating key: key_id cannot be empty")));
    EXPECT_TRUE(mysql_key_generate("", "AES", "Robert", 128));
    EXPECT_CALL(*(reinterpret_cast<Mock_logger*>(logger.get())), log(MY_ERROR_LEVEL, StrEq("Error while generating key: key_id cannot be empty")));
    EXPECT_TRUE(mysql_key_generate("", "AES", NULL, 128));
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
    return 1; 
  }
  ILogger *logger = new keyring::Mock_logger();
  keyring::Vault_mount vault_mount(curl, logger);

  keyring__api_unittest::keyring_filename= new char[strlen("./keyring_vault.conf")+1];
  strcpy(keyring__api_unittest::keyring_filename, "./keyring_vault.conf");
  std::string keyring_conf(keyring__api_unittest::keyring_filename);
  if (generate_credential_file(keyring_conf, CORRECT, keyring__api_unittest::uuid))
  {
    std::cout << "Could not generate credential file" << std::endl;
    return 2; 
  }
  if (vault_mount.init(&keyring_conf, &keyring__api_unittest::uuid))
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
  curl_easy_cleanup(curl);
  curl_global_cleanup();
  delete logger;
  delete[] keyring__api_unittest::keyring_filename;

  my_testing::teardown_server_for_unit_tests();
  return ret;
}
