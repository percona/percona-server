#ifndef MYSQL_VAULT_MOUNT_H
#define MYSQL_VAULT_MOUNT_H

#include <curl/curl.h>
#include "vault_credentials_parser.h"

namespace keyring {

class Vault_mount {
 public:
  Vault_mount(CURL *curl, ILogger *logger) : curl(curl), logger(logger) {}

  bool init(std::string *keyring_storage_url, std::string *secret_mount_point);
  bool mount_secret_backend();
  bool unmount_secret_backend();

 private:
  CURL *curl;
  ILogger *logger;
  Secure_string secret_mount_point;

  Secure_string token_header;
  Secure_string vault_mount_point_url;
  Secure_string vault_ca;

  Vault_credentials vault_credentials;
  char curl_errbuf[CURL_ERROR_SIZE];  // error from CURL

  std::string get_error_from_curl(CURLcode curl_code);
};

}  // namespace keyring

#endif  // MYSQL_VAULT_MOUNT_H
