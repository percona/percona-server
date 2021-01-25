#ifndef MYSQL_GUNIT_VAULT_MOUNT_H
#define MYSQL_GUNIT_VAULT_MOUNT_H

#include <my_global.h>
#include <curl/curl.h>

#include "vault_credentials.h"
#include "logger.h"

namespace keyring {

class Vault_mount {
 public:
  Vault_mount(CURL *curl, ILogger *logger) : curl(curl), logger(logger) {}

  bool init(const std::string &keyring_storage_url,
            const std::string &secret_mount_point,
            const std::string &admin_token);
  bool mount_secret_backend();
  bool unmount_secret_backend();

 private:
  CURL *        curl;
  ILogger *     logger;
  Secure_string secret_mount_point;

  Secure_string token_header;
  Secure_string vault_mount_point_url;
  Secure_string vault_ca;

  Vault_credentials vault_credentials;
  char              curl_errbuf[CURL_ERROR_SIZE];  //error from CURL

  std::string get_error_from_curl(CURLcode curl_code);
};

}  //namespace keyring

#endif  // MYSQL_GUNIT_VAULT_MOUNT_H
