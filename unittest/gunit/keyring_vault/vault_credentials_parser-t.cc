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

#include <boost/move/unique_ptr.hpp>

#include <gtest/gtest.h>

#include <my_global.h>

#include "i_keys_container.h"
#include "mock_logger.h"
#include "vault_credentials_parser.h"
#include "vault_credentials.h"

boost::movelib::unique_ptr<keyring::IKeys_container> keys(NULL);

extern "C" int my_plugin_log_message(MYSQL_PLUGIN *, enum plugin_log_level,
                                     const char *, ...)
{
  return 0;
}

#if defined(HAVE_PSI_INTERFACE)
namespace keyring {
PSI_memory_key key_memory_KEYRING= PSI_NOT_INSTRUMENTED;
}
#endif

namespace keyring__vault_credentials_parser_unittest {
using namespace keyring;

using ::testing::StrEq;

class Vault_credentials_parser_test : public ::testing::Test {
 protected:
  virtual void SetUp() { logger= new Mock_logger(); }

  virtual void TearDown()
  {
    delete logger;
    std::remove("./credentials");
  }

  void create_empty_credentials_file(std::ofstream &my_file)
  {
    std::remove("./credentials");
    my_file.open("./credentials");
  }

 protected:
  ILogger *logger;
};

TEST_F(Vault_credentials_parser_test, ParseNotExistingFile)
{
  Vault_credentials_parser vault_credentials_parser(logger);
  std::string              token;

  std::string file_url= "/.there_no_such_file";
  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(logger)),
              log(MY_ERROR_LEVEL, StrEq("Could not open credentials file '" +
                                        file_url + "'.")));

  Vault_credentials vault_credentials;
  EXPECT_TRUE(vault_credentials_parser.parse(file_url, vault_credentials));
  EXPECT_TRUE(vault_credentials.get_vault_url().empty());
  EXPECT_TRUE(vault_credentials.get_token().empty());
  EXPECT_TRUE(vault_credentials.get_secret_mount_point().empty());
  EXPECT_TRUE(vault_credentials.get_vault_ca().empty());

  ASSERT_TRUE(token.empty());
}

TEST_F(Vault_credentials_parser_test, ParseEmptyFile)
{
  Vault_credentials_parser vault_credentials_parser(logger);
  std::string              token;

  // create empty credentials file
  std::ofstream myfile;
  create_empty_credentials_file(myfile);
  myfile.close();

  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(logger)),
              log(MY_ERROR_LEVEL, StrEq("Credentials file is empty.")));
  std::string file_url= "./credentials";

  Vault_credentials vault_credentials;
  EXPECT_TRUE(vault_credentials_parser.parse(file_url, vault_credentials));
  EXPECT_TRUE(vault_credentials.get_vault_url().empty());
  EXPECT_TRUE(vault_credentials.get_token().empty());
  EXPECT_TRUE(vault_credentials.get_secret_mount_point().empty());
  EXPECT_TRUE(vault_credentials.get_vault_ca().empty());
}

TEST_F(Vault_credentials_parser_test, ParseFileWithoutSecretMountPoint)
{
  Vault_credentials_parser vault_credentials_parser(logger);
  std::string              token;

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
  std::string file_url= "./credentials";

  Vault_credentials vault_credentials;
  EXPECT_TRUE(vault_credentials_parser.parse(file_url, vault_credentials));

  EXPECT_TRUE(vault_credentials.get_vault_url().empty());
  EXPECT_TRUE(vault_credentials.get_token().empty());
  EXPECT_TRUE(vault_credentials.get_secret_mount_point().empty());
  EXPECT_TRUE(vault_credentials.get_vault_ca().empty());
}

TEST_F(Vault_credentials_parser_test, ParseFileWithoutVaultURL)
{
  Vault_credentials_parser vault_credentials_parser(logger);
  std::string              token;

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
  std::string file_url= "./credentials";

  Vault_credentials vault_credentials;
  EXPECT_TRUE(vault_credentials_parser.parse(file_url, vault_credentials));

  EXPECT_TRUE(vault_credentials.get_vault_url().empty());
  EXPECT_TRUE(vault_credentials.get_token().empty());
  EXPECT_TRUE(vault_credentials.get_secret_mount_point().empty());
  EXPECT_TRUE(vault_credentials.get_vault_ca().empty());
}

TEST_F(Vault_credentials_parser_test, ParseFileWithoutToken)
{
  Vault_credentials_parser vault_credentials_parser(logger);
  std::string              token;

  // create empty credentials file
  std::ofstream my_file;
  create_empty_credentials_file(my_file);
  my_file << "vault_url = http://127.0.0.1:8200" << std::endl;
  my_file << "secret_mount_point = secret" << std::endl;
  my_file << "vault_ca = /some/path";
  my_file.close();

  EXPECT_CALL(
      *(reinterpret_cast<Mock_logger *>(logger)),
      log(MY_ERROR_LEVEL,
          StrEq("Could not read token from the configuration file.")));
  std::string file_url= "./credentials";

  Vault_credentials vault_credentials;
  EXPECT_TRUE(vault_credentials_parser.parse(file_url, vault_credentials));

  EXPECT_TRUE(vault_credentials.get_vault_url().empty());
  EXPECT_TRUE(vault_credentials.get_token().empty());
  EXPECT_TRUE(vault_credentials.get_secret_mount_point().empty());
  EXPECT_TRUE(vault_credentials.get_vault_ca().empty());
}

TEST_F(Vault_credentials_parser_test, ParseFileWithoutVaultCA)
{
  Vault_credentials_parser vault_credentials_parser(logger);
  std::string              token;

  // create empty credentials file
  std::ofstream my_file;
  create_empty_credentials_file(my_file);

  my_file << "vault_url = http://127.0.0.1:8200" << std::endl;
  my_file << "secret_mount_point = secret" << std::endl;
  my_file << "token = 123-123-123";
  my_file.close();

  std::string       file_url= "./credentials";
  Vault_credentials vault_credentials;
  EXPECT_FALSE(vault_credentials_parser.parse(file_url, vault_credentials));

  EXPECT_STREQ(vault_credentials.get_vault_url().c_str(),
               "http://127.0.0.1:8200");
  EXPECT_STREQ(vault_credentials.get_secret_mount_point().c_str(), "secret");
  EXPECT_STREQ(vault_credentials.get_token().c_str(), "123-123-123");
  EXPECT_TRUE(vault_credentials.get_vault_ca().empty());
}

TEST_F(Vault_credentials_parser_test, ParseFileWithCorrectCredentials)
{
  Vault_credentials_parser vault_credentials_parser(logger);
  std::string              token;

  // create empty credentials file
  std::ofstream my_file;
  create_empty_credentials_file(my_file);
  my_file << "vault_url = https://127.0.0.1:8200" << std::endl;
  my_file << "secret_mount_point = secret" << std::endl;
  my_file << "token = 123-123-123" << std::endl;
  my_file << "vault_ca = /some/path" << std::endl;
  my_file << "secret_mount_point_version = 1";
  my_file.close();

  std::string       file_url= "./credentials";
  Vault_credentials vault_credentials;
  EXPECT_FALSE(vault_credentials_parser.parse(file_url, vault_credentials));

  EXPECT_STREQ(vault_credentials.get_vault_url().c_str(),
               "https://127.0.0.1:8200");
  EXPECT_STREQ(vault_credentials.get_secret_mount_point().c_str(), "secret");
  EXPECT_STREQ(vault_credentials.get_token().c_str(), "123-123-123");
  EXPECT_STREQ(vault_credentials.get_vault_ca().c_str(), "/some/path");
  EXPECT_EQ(vault_credentials.get_secret_mount_point_version(),
            Vault_version_v1);
}

TEST_F(Vault_credentials_parser_test,
       ParseFileWithCorrectCredentialsWithSecretMountPointVersion2)
{
  Vault_credentials_parser vault_credentials_parser(logger);
  std::string              token;

  // create empty credentials file
  std::ofstream my_file;
  create_empty_credentials_file(my_file);
  my_file << "vault_url = https://127.0.0.1:8200" << std::endl;
  my_file << "secret_mount_point = secret" << std::endl;
  my_file << "token = 123-123-123" << std::endl;
  my_file << "vault_ca = /some/path" << std::endl;
  my_file << "secret_mount_point_version = 2";
  my_file.close();

  std::string       file_url= "./credentials";
  Vault_credentials vault_credentials;
  EXPECT_FALSE(vault_credentials_parser.parse(file_url, vault_credentials));

  EXPECT_STREQ(vault_credentials.get_vault_url().c_str(),
               "https://127.0.0.1:8200");
  EXPECT_STREQ(vault_credentials.get_secret_mount_point().c_str(), "secret");
  EXPECT_STREQ(vault_credentials.get_token().c_str(), "123-123-123");
  EXPECT_STREQ(vault_credentials.get_vault_ca().c_str(), "/some/path");
  EXPECT_EQ(vault_credentials.get_secret_mount_point_version(),
            Vault_version_v2);
}

TEST_F(Vault_credentials_parser_test,
       ParseFileWithCorrectCredentialsWithSecretMountPointVersionAUTO)
{
  Vault_credentials_parser vault_credentials_parser(logger);
  std::string              token;

  // create empty credentials file
  std::ofstream my_file;
  create_empty_credentials_file(my_file);
  my_file << "vault_url = https://127.0.0.1:8200" << std::endl;
  my_file << "secret_mount_point = secret" << std::endl;
  my_file << "token = 123-123-123" << std::endl;
  my_file << "vault_ca = /some/path" << std::endl;
  my_file << "secret_mount_point_version = AUTO";
  my_file.close();

  std::string       file_url= "./credentials";
  Vault_credentials vault_credentials;
  EXPECT_FALSE(vault_credentials_parser.parse(file_url, vault_credentials));

  EXPECT_STREQ(vault_credentials.get_vault_url().c_str(),
               "https://127.0.0.1:8200");
  EXPECT_STREQ(vault_credentials.get_secret_mount_point().c_str(), "secret");
  EXPECT_STREQ(vault_credentials.get_token().c_str(), "123-123-123");
  EXPECT_STREQ(vault_credentials.get_vault_ca().c_str(), "/some/path");
  EXPECT_EQ(vault_credentials.get_secret_mount_point_version(),
            Vault_version_auto);
}

TEST_F(Vault_credentials_parser_test,
       ParseFileWithCorrectCredentialsWithSecretMountPointVersionAUTO2)
{
  Vault_credentials_parser vault_credentials_parser(logger);
  std::string              token;

  // create empty credentials file
  std::ofstream my_file;
  create_empty_credentials_file(my_file);
  my_file << "vault_url = https://127.0.0.1:8200" << std::endl;
  my_file << "secret_mount_point = secret" << std::endl;
  my_file << "token = 123-123-123" << std::endl;
  my_file << "vault_ca = /some/path" << std::endl;
  my_file << "secret_mount_point_version =        AUTO   ";
  my_file.close();

  std::string       file_url= "./credentials";
  Vault_credentials vault_credentials;
  EXPECT_FALSE(vault_credentials_parser.parse(file_url, vault_credentials));

  EXPECT_STREQ(vault_credentials.get_vault_url().c_str(),
               "https://127.0.0.1:8200");
  EXPECT_STREQ(vault_credentials.get_secret_mount_point().c_str(), "secret");
  EXPECT_STREQ(vault_credentials.get_token().c_str(), "123-123-123");
  EXPECT_STREQ(vault_credentials.get_vault_ca().c_str(), "/some/path");
  EXPECT_EQ(vault_credentials.get_secret_mount_point_version(),
            Vault_version_auto);
}

TEST_F(Vault_credentials_parser_test,
       ParseFileWithWithSecretMountVersionTooBig)
{
  Vault_credentials_parser vault_credentials_parser(logger);
  std::string              token;

  // create empty credentials file
  std::ofstream my_file;
  create_empty_credentials_file(my_file);
  my_file << "vault_url = http://127.0.0.1:8200" << std::endl;
  my_file << "secret_mount_point = secret" << std::endl;
  my_file << "token = 123-123-123" << std::endl;
  my_file << "vault_ca = /some/path" << std::endl;
  my_file << "secret_mount_point_version = 3";
  my_file.close();

  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(logger)),
              log(MY_ERROR_LEVEL,
                  StrEq("secret_mount_point_version in the configuration "
                        "file must be either 1 or 2.")));
  std::string file_url= "./credentials";

  Vault_credentials vault_credentials;
  EXPECT_TRUE(vault_credentials_parser.parse(file_url, vault_credentials));

  EXPECT_TRUE(vault_credentials.get_vault_url().empty());
  EXPECT_TRUE(vault_credentials.get_token().empty());
  EXPECT_TRUE(vault_credentials.get_secret_mount_point().empty());
  EXPECT_TRUE(vault_credentials.get_vault_ca().empty());
  EXPECT_EQ(vault_credentials.get_secret_mount_point_version(),
            Vault_version_unknown);
}

TEST_F(Vault_credentials_parser_test,
       ParseFileWithWithSecretMountVersionTooBig2)
{
  Vault_credentials_parser vault_credentials_parser(logger);
  std::string              token;

  // create empty credentials file
  std::ofstream my_file;
  create_empty_credentials_file(my_file);
  my_file << "vault_url = http://127.0.0.1:8200" << std::endl;
  my_file << "secret_mount_point = secret" << std::endl;
  my_file << "token = 123-123-123" << std::endl;
  my_file << "vault_ca = /some/path" << std::endl;
  my_file << "secret_mount_point_version = 1000000000000000000";
  my_file.close();

  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(logger)),
              log(MY_ERROR_LEVEL,
                  StrEq("secret_mount_point_version in the configuration "
                        "file is neither AUTO nor a numeric value.")));
  std::string file_url= "./credentials";

  Vault_credentials vault_credentials;
  EXPECT_TRUE(vault_credentials_parser.parse(file_url, vault_credentials));

  EXPECT_TRUE(vault_credentials.get_vault_url().empty());
  EXPECT_TRUE(vault_credentials.get_token().empty());
  EXPECT_TRUE(vault_credentials.get_secret_mount_point().empty());
  EXPECT_TRUE(vault_credentials.get_vault_ca().empty());
  EXPECT_EQ(vault_credentials.get_secret_mount_point_version(),
            Vault_version_unknown);
}

TEST_F(Vault_credentials_parser_test,
       ParseFileWithWithSecretMountVersionTooSmall2)
{
  Vault_credentials_parser vault_credentials_parser(logger);
  std::string              token;

  // create empty credentials file
  std::ofstream my_file;
  create_empty_credentials_file(my_file);
  my_file << "vault_url = http://127.0.0.1:8200" << std::endl;
  my_file << "secret_mount_point = secret" << std::endl;
  my_file << "token = 123-123-123" << std::endl;
  my_file << "vault_ca = /some/path" << std::endl;
  my_file << "secret_mount_point_version = -1";
  my_file.close();

  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(logger)),
              log(MY_ERROR_LEVEL,
                  StrEq("secret_mount_point_version in the configuration "
                        "file must be either 1 or 2.")));
  std::string file_url= "./credentials";

  Vault_credentials vault_credentials;
  EXPECT_TRUE(vault_credentials_parser.parse(file_url, vault_credentials));

  EXPECT_TRUE(vault_credentials.get_vault_url().empty());
  EXPECT_TRUE(vault_credentials.get_token().empty());
  EXPECT_TRUE(vault_credentials.get_secret_mount_point().empty());
  EXPECT_TRUE(vault_credentials.get_vault_ca().empty());
  EXPECT_EQ(vault_credentials.get_secret_mount_point_version(),
            Vault_version_unknown);
}

TEST_F(Vault_credentials_parser_test,
       ParseFileWithWithSecretMountVersionBeingString)
{
  Vault_credentials_parser vault_credentials_parser(logger);
  std::string              token;

  // create empty credentials file
  std::ofstream my_file;
  create_empty_credentials_file(my_file);
  my_file << "vault_url = http://127.0.0.1:8200" << std::endl;
  my_file << "secret_mount_point = secret" << std::endl;
  my_file << "token = 123-123-123" << std::endl;
  my_file << "vault_ca = /some/path" << std::endl;
  my_file << "secret_mount_point_version = THIS_SHOULD_NOT_BE_STRING";
  my_file.close();

  EXPECT_CALL(*(reinterpret_cast<Mock_logger *>(logger)),
              log(MY_ERROR_LEVEL,
                  StrEq("secret_mount_point_version in the configuration "
                        "file is neither AUTO nor a numeric value.")));
  std::string file_url= "./credentials";

  Vault_credentials vault_credentials;
  EXPECT_TRUE(vault_credentials_parser.parse(file_url, vault_credentials));

  EXPECT_TRUE(vault_credentials.get_vault_url().empty());
  EXPECT_TRUE(vault_credentials.get_token().empty());
  EXPECT_TRUE(vault_credentials.get_secret_mount_point().empty());
  EXPECT_TRUE(vault_credentials.get_vault_ca().empty());
  EXPECT_EQ(vault_credentials.get_secret_mount_point_version(),
            Vault_version_unknown);
}


TEST_F(Vault_credentials_parser_test, ParseFileWithCorrectCredentialsSpaces)
{
  Vault_credentials_parser vault_credentials_parser(logger);
  std::string              token;

  // create empty credentials file
  std::ofstream my_file;
  create_empty_credentials_file(my_file);
  my_file << "vault_url =https://127.0.0.1:8200" << std::endl;
  my_file << "secret_mount_point=secret" << std::endl;
  my_file << "token = 123-123-123" << std::endl;
  my_file << "vault_ca = /some/path";
  my_file.close();

  std::string       file_url= "./credentials";
  Vault_credentials vault_credentials;
  EXPECT_FALSE(vault_credentials_parser.parse(file_url, vault_credentials));

  EXPECT_STREQ(vault_credentials.get_vault_url().c_str(),
               "https://127.0.0.1:8200");
  EXPECT_STREQ(vault_credentials.get_secret_mount_point().c_str(), "secret");
  EXPECT_STREQ(vault_credentials.get_token().c_str(), "123-123-123");
  EXPECT_STREQ(vault_credentials.get_vault_ca().c_str(), "/some/path");
}

TEST_F(Vault_credentials_parser_test,
       ParseFileWithCorrectCredentialsTrailingSpaces)
{
  Vault_credentials_parser vault_credentials_parser(logger);
  std::string              token;

  // create empty credentials file
  std::ofstream my_file;
  create_empty_credentials_file(my_file);
  my_file << "vault_url =https://127.0.0.1:8200 " << std::endl;
  my_file << "secret_mount_point=secret " << std::endl;
  my_file << "token = 123-123-123 " << std::endl;
  my_file << "vault_ca = /some/path ";
  my_file.close();

  std::string       file_url= "./credentials";
  Vault_credentials vault_credentials;
  EXPECT_FALSE(vault_credentials_parser.parse(file_url, vault_credentials));

  EXPECT_STREQ(vault_credentials.get_vault_url().c_str(),
               "https://127.0.0.1:8200");
  EXPECT_STREQ(vault_credentials.get_secret_mount_point().c_str(), "secret");
  EXPECT_STREQ(vault_credentials.get_token().c_str(), "123-123-123");
  EXPECT_STREQ(vault_credentials.get_vault_ca().c_str(), "/some/path");
}

TEST_F(Vault_credentials_parser_test, ParseFileWithValuesWithSpacesInIt)
{
  Vault_credentials_parser vault_credentials_parser(logger);
  std::string              token;

  // create empty credentials file
  std::ofstream my_file;
  create_empty_credentials_file(my_file);
  my_file << "vault_url =https://127 .0.0.1: 8200 " << std::endl;
  my_file << "secret_mount_point= s-e c-r -e t " << std::endl;
  my_file << "token = 12000 3-10  23- 123 " << std::endl;
  my_file << "vault_ca = /some/  path";
  my_file.close();

  std::string       file_url= "./credentials";
  Vault_credentials vault_credentials;
  EXPECT_FALSE(vault_credentials_parser.parse(file_url, vault_credentials));

  EXPECT_STREQ(vault_credentials.get_vault_url().c_str(),
               "https://127 .0.0.1: 8200");
  EXPECT_STREQ(vault_credentials.get_secret_mount_point().c_str(),
               "s-e c-r -e t");
  EXPECT_STREQ(vault_credentials.get_token().c_str(), "12000 3-10  23- 123");
  EXPECT_STREQ(vault_credentials.get_vault_ca().c_str(), "/some/  path");
}
}  // namespace keyring__vault_credentials_parser_unittest

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
