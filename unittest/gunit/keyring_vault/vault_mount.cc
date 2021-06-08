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

#include "vault_mount.h"

#include <iostream>
#include <sstream>

#include "vault_credentials_parser.h"
#include "vault_credentials.h"

namespace keyring {

bool Vault_mount::init(const std::string &keyring_storage_url,
                       const std::string &secret_mount_point,
                       const std::string &admin_token)
{
  Vault_credentials_parser vault_credentials_parser(logger);
  if (vault_credentials_parser.parse(keyring_storage_url, vault_credentials))
    return true;

  this->token_header= "X-Vault-Token:";
  this->token_header.append(admin_token.c_str(), admin_token.size());
  this->vault_mount_point_url=
      vault_credentials.get_vault_url() + "/v1/sys/mounts/";
  this->vault_mount_point_url+= secret_mount_point.c_str();
  this->vault_ca= vault_credentials.get_vault_ca();

  return false;
}

bool Vault_mount::mount_secret_backend()
{
  curl_easy_reset(curl);
  struct curl_slist *list= NULL;
  long               http_code= 0;
  Secure_string      postdata("{\"type\":\"generic\"}");
  CURLcode           curl_res= CURLE_OK;

  if ((list= curl_slist_append(list, token_header.c_str())) == NULL ||
      (list= curl_slist_append(list, "Content-Type: application/json")) ==
          NULL ||
      (curl_res= curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list)) !=
          CURLE_OK ||
      (curl_res= curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curl_errbuf)) !=
          CURLE_OK ||
      (curl_res= curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1)) !=
          CURLE_OK ||
      (curl_res= curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L)) !=
          CURLE_OK ||
      (curl_res= curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L)) !=
          CURLE_OK ||
      (!vault_ca.empty() &&
       (curl_res= curl_easy_setopt(curl, CURLOPT_CAINFO, vault_ca.c_str())) !=
           CURLE_OK) ||
      (curl_res= curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL)) !=
          CURLE_OK ||
      (curl_res= curl_easy_setopt(
           curl, CURLOPT_URL, vault_mount_point_url.c_str())) != CURLE_OK ||
      (curl_res= curl_easy_setopt(curl, CURLOPT_POSTFIELDS,
                                  postdata.c_str())) != CURLE_OK ||
      (curl_res= curl_easy_perform(curl)) != CURLE_OK ||
      (curl_res= curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE,
                                   &http_code)) != CURLE_OK ||
      http_code / 100 != 2 /* 2** are success return codes*/
  )
  {
    std::cout << "Could not create secret mount point";
    std::cout << get_error_from_curl(curl_res).c_str();
    curl_slist_free_all(list);
    return true;
  }
  curl_slist_free_all(list);
  return false;
}

bool Vault_mount::unmount_secret_backend()
{
  curl_easy_reset(curl);
  struct curl_slist *list= NULL;
  long               http_code= 0;
  CURLcode           curl_res= CURLE_OK;

  if ((list= curl_slist_append(list, token_header.c_str())) == NULL ||
      (list= curl_slist_append(list, "Content-Type: application/json")) ==
          NULL ||
      (curl_res= curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list)) !=
          CURLE_OK ||
      (curl_res= curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curl_errbuf)) !=
          CURLE_OK ||
      (curl_res= curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1)) !=
          CURLE_OK ||
      (curl_res= curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L)) !=
          CURLE_OK ||
      (curl_res= curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L)) !=
          CURLE_OK ||
      (!vault_ca.empty() &&
       (curl_res= curl_easy_setopt(curl, CURLOPT_CAINFO, vault_ca.c_str())) !=
           CURLE_OK) ||
      (curl_res= curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL)) !=
          CURLE_OK ||
      (curl_res= curl_easy_setopt(
           curl, CURLOPT_URL, vault_mount_point_url.c_str())) != CURLE_OK ||
      (curl_res= curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE")) !=
          CURLE_OK ||
      (curl_res= curl_easy_perform(curl)) != CURLE_OK ||
      (curl_res= curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE,
                                   &http_code)) != CURLE_OK ||
      http_code / 100 != 2 /* 2** are success return codes*/
  )
  {
    std::cout << "Could not delete secret mount point";
    std::cout << get_error_from_curl(curl_res).c_str();
    curl_slist_free_all(list);
    return true;
  }
  curl_slist_free_all(list);
  return false;
}

std::string Vault_mount::get_error_from_curl(CURLcode curl_code)
{
  size_t             len= strlen(curl_errbuf);
  std::ostringstream ss;
  if (curl_code != CURLE_OK)
  {
    ss << "CURL returned this error code: " << curl_code;
    ss << " with error message : ";
    if (len)
      ss << curl_errbuf;
    else
      ss << curl_easy_strerror(curl_code);
  }
  return ss.str();
}

}  //namespace keyring
