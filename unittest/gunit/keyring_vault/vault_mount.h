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

#ifndef MYSQL_GUNIT_VAULT_MOUNT_H
#define MYSQL_GUNIT_VAULT_MOUNT_H

#include <curl/curl.h>

#include "logger.h"
#include "vault_credentials.h"

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

#endif  // MYSQL_GUNIT_VAULT_MOUNT_H
