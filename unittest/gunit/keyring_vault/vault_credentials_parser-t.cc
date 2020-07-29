#include <gtest/gtest.h>
#include <fstream>
#include <memory>
#include "i_keys_container.h"
#include "mock_logger.h"
#include "test_utils.h"
#include "vault_credentials_parser.h"

namespace keyring__vault_credentials_parser_unittest {
using namespace keyring;

using ::testing::StrEq;

class Vault_credentials_parser_test : public ::testing::Test {
 protected:
  virtual void SetUp() { logger = new Mock_logger(); }

  virtual void TearDown() {
    delete logger;
    std::remove("./credentials");
  }

  void create_empty_credentials_file(std::ofstream &my_file) {
    std::remove("./credentials");
    my_file.open("./credentials");
  }

 protected:
  ILogger *logger;
};

TEST_F(Vault_credentials_parser_test, ParseNotExistingFile) {
  Vault_credentials_parser vault_credentials_parser(logger);
  std::string token;

  EXPECT_CALL(
      *(reinterpret_cast<Mock_logger *>(logger)),
      log(MY_ERROR_LEVEL, StrEq("Could not open file with credentials.")));
  EXPECT_CALL(
      *(reinterpret_cast<Mock_logger *>(logger)),
      log(MY_WARNING_LEVEL, StrEq("File '/.there_no_such_file' not found (OS "
                                  "errno 2 - No such file or directory)")));

  std::string file_url = "/.there_no_such_file";
  Vault_credentials vault_credentials;
  EXPECT_TRUE(vault_credentials_parser.parse(file_url, &vault_credentials));
  EXPECT_TRUE(vault_credentials["vault_url"].empty());
  EXPECT_TRUE(vault_credentials["token"].empty());
  EXPECT_TRUE(vault_credentials["secret_mount_point"].empty());
  EXPECT_TRUE(vault_credentials["vault_ca"].empty());

  ASSERT_TRUE(token.empty());
}

TEST_F(Vault_credentials_parser_test, ParseEmptyFile) {
  Vault_credentials_parser vault_credentials_parser(logger);
  std::string token;

  // create empty credentials file
  std::ofstream myfile;
  create_empty_credentials_file(myfile);
  myfile.close();

  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(logger)),
              log(MY_ERROR_LEVEL, StrEq("Empty file with credentials.")));
  std::string file_url = "./credentials";

  Vault_credentials vault_credentials;
  EXPECT_TRUE(vault_credentials_parser.parse(file_url, &vault_credentials));
  EXPECT_TRUE(vault_credentials["vault_url"].empty());
  EXPECT_TRUE(vault_credentials["token"].empty());
  EXPECT_TRUE(vault_credentials["secret_mount_point"].empty());
  EXPECT_TRUE(vault_credentials["vault_ca"].empty());
}

TEST_F(Vault_credentials_parser_test, ParseFileWithoutSecretMountPoint) {
  Vault_credentials_parser vault_credentials_parser(logger);
  std::string token;

  // create empty credentials file
  std::ofstream my_file;
  create_empty_credentials_file(my_file);
  my_file << "vault_url = http://127.0.0.1:8200" << std::endl;
  my_file << "token = 123-123-123" << std::endl;
  my_file << "vault_ca = /some/path";
  my_file.close();

  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(logger)),
              log(MY_ERROR_LEVEL, StrEq("Could not read secret_mount_point "
                                        "from the configuration file.")));
  std::string file_url = "./credentials";

  Vault_credentials vault_credentials;
  EXPECT_TRUE(vault_credentials_parser.parse(file_url, &vault_credentials));

  EXPECT_TRUE(vault_credentials["vault_url"].empty());
  EXPECT_TRUE(vault_credentials["token"].empty());
  EXPECT_TRUE(vault_credentials["secret_mount_point"].empty());
  EXPECT_TRUE(vault_credentials["vault_ca"].empty());
}

TEST_F(Vault_credentials_parser_test, ParseFileWithoutVaultURL) {
  Vault_credentials_parser vault_credentials_parser(logger);
  std::string token;

  // create empty credentials file
  std::ofstream my_file;
  create_empty_credentials_file(my_file);
  my_file << "secret_mount_point = secret" << std::endl;
  my_file << "token = 123-123-123" << std::endl;
  my_file << "vault_ca = /some/path";
  my_file.close();

  EXPECT_CALL(
      *(reinterpret_cast<Mock_logger *>(logger)),
      log(MY_ERROR_LEVEL,
          StrEq("Could not read vault_url from the configuration file.")));
  std::string file_url = "./credentials";

  Vault_credentials vault_credentials;
  EXPECT_TRUE(vault_credentials_parser.parse(file_url, &vault_credentials));

  EXPECT_TRUE(vault_credentials["vault_url"].empty());
  EXPECT_TRUE(vault_credentials["token"].empty());
  EXPECT_TRUE(vault_credentials["secret_mount_point"].empty());
  EXPECT_TRUE(vault_credentials["vault_ca"].empty());
}

TEST_F(Vault_credentials_parser_test, ParseFileWithoutToken) {
  Vault_credentials_parser vault_credentials_parser(logger);
  std::string token;

  // create empty credentials file
  std::ofstream my_file;
  create_empty_credentials_file(my_file);
  my_file << "vault_url = http://127.0.0.1:8200" << std::endl;
  my_file << "secret_mount_point = secret" << std::endl;
  my_file << "vault_ca = /some/path";
  my_file.close();

  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(logger)),
              log(MY_ERROR_LEVEL,
                  StrEq("Could not read token from the configuration file.")));
  std::string file_url = "./credentials";

  Vault_credentials vault_credentials;
  EXPECT_TRUE(vault_credentials_parser.parse(file_url, &vault_credentials));

  EXPECT_TRUE(vault_credentials["vault_url"].empty());
  EXPECT_TRUE(vault_credentials["token"].empty());
  EXPECT_TRUE(vault_credentials["secret_mount_point"].empty());
  EXPECT_TRUE(vault_credentials["vault_ca"].empty());
}

TEST_F(Vault_credentials_parser_test, ParseFileWithoutVaultCA) {
  Vault_credentials_parser vault_credentials_parser(logger);
  std::string token;

  // create empty credentials file
  std::ofstream my_file;
  create_empty_credentials_file(my_file);

  my_file << "vault_url = http://127.0.0.1:8200" << std::endl;
  my_file << "secret_mount_point = secret" << std::endl;
  my_file << "token = 123-123-123";
  my_file.close();

  std::string file_url = "./credentials";
  Vault_credentials vault_credentials;
  EXPECT_FALSE(vault_credentials_parser.parse(file_url, &vault_credentials));

  EXPECT_STREQ(vault_credentials["vault_url"].c_str(), "http://127.0.0.1:8200");
  EXPECT_STREQ(vault_credentials["secret_mount_point"].c_str(), "secret");
  EXPECT_STREQ(vault_credentials["token"].c_str(), "123-123-123");
  EXPECT_TRUE(vault_credentials["vault_ca"].empty());
}

TEST_F(Vault_credentials_parser_test, ParseFileWithCorrectCredentials) {
  Vault_credentials_parser vault_credentials_parser(logger);
  std::string token;

  // create empty credentials file
  std::ofstream my_file;
  create_empty_credentials_file(my_file);
  my_file << "vault_url = http://127.0.0.1:8200" << std::endl;
  my_file << "secret_mount_point = secret" << std::endl;
  my_file << "token = 123-123-123" << std::endl;
  my_file << "vault_ca = /some/path";
  my_file.close();

  std::string file_url = "./credentials";
  Vault_credentials vault_credentials;
  EXPECT_FALSE(vault_credentials_parser.parse(file_url, &vault_credentials));

  EXPECT_STREQ(vault_credentials["vault_url"].c_str(), "http://127.0.0.1:8200");
  EXPECT_STREQ(vault_credentials["secret_mount_point"].c_str(), "secret");
  EXPECT_STREQ(vault_credentials["token"].c_str(), "123-123-123");
  EXPECT_STREQ(vault_credentials["vault_ca"].c_str(), "/some/path");
}

TEST_F(Vault_credentials_parser_test, ParseFileWithCorrectCredentialsSpaces) {
  Vault_credentials_parser vault_credentials_parser(logger);
  std::string token;

  // create empty credentials file
  std::ofstream my_file;
  create_empty_credentials_file(my_file);
  my_file << "vault_url =http://127.0.0.1:8200" << std::endl;
  my_file << "secret_mount_point=secret" << std::endl;
  my_file << "token = 123-123-123" << std::endl;
  my_file << "vault_ca = /some/path";
  my_file.close();

  std::string file_url = "./credentials";
  Vault_credentials vault_credentials;
  EXPECT_FALSE(vault_credentials_parser.parse(file_url, &vault_credentials));

  EXPECT_STREQ(vault_credentials["vault_url"].c_str(), "http://127.0.0.1:8200");
  EXPECT_STREQ(vault_credentials["secret_mount_point"].c_str(), "secret");
  EXPECT_STREQ(vault_credentials["token"].c_str(), "123-123-123");
  EXPECT_STREQ(vault_credentials["vault_ca"].c_str(), "/some/path");
}

TEST_F(Vault_credentials_parser_test,
       ParseFileWithCorrectCredentialsTrailingSpaces) {
  Vault_credentials_parser vault_credentials_parser(logger);
  std::string token;

  // create empty credentials file
  std::ofstream my_file;
  create_empty_credentials_file(my_file);
  my_file << "vault_url =http://127.0.0.1:8200 " << std::endl;
  my_file << "secret_mount_point=secret " << std::endl;
  my_file << "token = 123-123-123 " << std::endl;
  my_file << "vault_ca = /some/path ";
  my_file.close();

  std::string file_url = "./credentials";
  Vault_credentials vault_credentials;
  EXPECT_FALSE(vault_credentials_parser.parse(file_url, &vault_credentials));

  EXPECT_STREQ(vault_credentials["vault_url"].c_str(), "http://127.0.0.1:8200");
  EXPECT_STREQ(vault_credentials["secret_mount_point"].c_str(), "secret");
  EXPECT_STREQ(vault_credentials["token"].c_str(), "123-123-123");
  EXPECT_STREQ(vault_credentials["vault_ca"].c_str(), "/some/path");
}

TEST_F(Vault_credentials_parser_test, ParseFileWithValuesWithSpacesInIt) {
  Vault_credentials_parser vault_credentials_parser(logger);
  std::string token;

  // create empty credentials file
  std::ofstream my_file;
  create_empty_credentials_file(my_file);
  my_file << "vault_url =http: //127 .0.0.1: 8200 " << std::endl;
  my_file << "secret_mount_point= s-e c-r -e t " << std::endl;
  my_file << "token = 12000 3-10  23- 123 " << std::endl;
  my_file << "vault_ca = /some/  path";
  my_file.close();

  std::string file_url = "./credentials";
  Vault_credentials vault_credentials;
  EXPECT_FALSE(vault_credentials_parser.parse(file_url, &vault_credentials));

  EXPECT_STREQ(vault_credentials["vault_url"].c_str(),
               "http: //127 .0.0.1: 8200");
  EXPECT_STREQ(vault_credentials["secret_mount_point"].c_str(), "s-e c-r -e t");
  EXPECT_STREQ(vault_credentials["token"].c_str(), "12000 3-10  23- 123");
  EXPECT_STREQ(vault_credentials["vault_ca"].c_str(), "/some/  path");
}
}  // namespace keyring__vault_credentials_parser_unittest

#ifndef MERGE_UNITTESTS
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  MY_INIT(argv[0]);
  my_testing::setup_server_for_unit_tests();
  int ret = RUN_ALL_TESTS();
  my_testing::teardown_server_for_unit_tests();

  return ret;
}
#endif  // MERGE_UNITTESTS
