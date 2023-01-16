#include <cstdio>
#include <fstream>
#include <string>

enum GENERATE_CREDENTIALS {
  CORRECT,
  WITH_INVALID_TOKEN,
};

static bool generate_credential_file(
    const std::string &credential_file_path,
    const GENERATE_CREDENTIALS generate_credetials = CORRECT,
    const std::string &secret_mount_point = "") {
  std::remove(credential_file_path.c_str());
  const char *mysql_test_dir(getenv("MYSQL_TEST_DIR"));
  const char *vault_token(getenv("MTR_VAULT_TOKEN"));
  if (mysql_test_dir == nullptr || vault_token == nullptr) return true;
  std::string credential_file_template_path = mysql_test_dir;
  credential_file_template_path +=
      "/std_data/keyring_vault_confs/keyring_vault_ut.conf";
  std::ifstream credentials_file_template(
      credential_file_template_path.c_str());
  std::ofstream credentials_file(credential_file_path.c_str());
  if (!credentials_file_template || !credentials_file) return true;
  std::string line;
  while (!getline(credentials_file_template, line).fail()) {
    if (line.find("token") != std::string::npos) {
      if (generate_credetials == WITH_INVALID_TOKEN)
        line = "token = 123-123-123";
      else
        (line = "token = ").append(vault_token);
    }
    if (secret_mount_point.empty() == false &&
        line.find("secret_mount_point") != std::string::npos)
      line = "secret_mount_point = " + secret_mount_point;
    size_t mysql_test_dir_var_pos = line.find("MYSQL_TEST_DIR");
    if (mysql_test_dir_var_pos != std::string::npos)
      line.replace(mysql_test_dir_var_pos, strlen("MYSQL_TEST_DIR"),
                   mysql_test_dir);
    credentials_file << line << std::endl;
  }
  return false;
}
