#include "vault_mount.h"
#include <iostream>
#include <sstream>

namespace keyring {

bool Vault_mount::init(std::string *keyring_storage_url,
                       std::string *secret_mount_point) {
  Vault_credentials_parser vault_credentials_parser(logger);
  if (vault_credentials_parser.parse(*keyring_storage_url, &vault_credentials))
    return true;

  this->token_header =
      "X-Vault-Token:" + get_credential(vault_credentials, "token");
  this->vault_mount_point_url =
      get_credential(vault_credentials, "vault_url") + "/v1/sys/mounts/";
  this->vault_mount_point_url += secret_mount_point->c_str();
  this->vault_ca = get_credential(vault_credentials, "vault_ca");

  return false;
}

bool Vault_mount::mount_secret_backend() {
  curl_easy_reset(curl);
  struct curl_slist *list = NULL;
  long http_code = 0;
  Secure_string postdata("{\"type\":\"generic\"}");
  CURLcode curl_res = CURLE_OK;

  if ((list = curl_slist_append(list, token_header.c_str())) == NULL ||
      (list = curl_slist_append(list, "Content-Type: application/json")) ==
          NULL ||
      (curl_res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list)) !=
          CURLE_OK ||
      (curl_res = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curl_errbuf)) !=
          CURLE_OK ||
      (curl_res = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1)) !=
          CURLE_OK ||
      (curl_res = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L)) !=
          CURLE_OK ||
      (curl_res = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L)) !=
          CURLE_OK ||
      (!vault_ca.empty() &&
       (curl_res = curl_easy_setopt(curl, CURLOPT_CAINFO, vault_ca.c_str())) !=
           CURLE_OK) ||
      (curl_res = curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL)) !=
          CURLE_OK ||
      (curl_res = curl_easy_setopt(
           curl, CURLOPT_URL, vault_mount_point_url.c_str())) != CURLE_OK ||
      (curl_res = curl_easy_setopt(curl, CURLOPT_POSTFIELDS,
                                   postdata.c_str())) != CURLE_OK ||
      (curl_res = curl_easy_perform(curl)) != CURLE_OK ||
      (curl_res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE,
                                    &http_code)) != CURLE_OK ||
      http_code / 100 != 2 /* 2** are success return codes*/
  ) {
    std::cout << "Could not create secret mount point";
    std::cout << get_error_from_curl(curl_res).c_str();
    curl_slist_free_all(list);
    return true;
  }
  curl_slist_free_all(list);
  return false;
}

bool Vault_mount::unmount_secret_backend() {
  curl_easy_reset(curl);
  struct curl_slist *list = NULL;
  long http_code = 0;
  CURLcode curl_res = CURLE_OK;

  if ((list = curl_slist_append(list, token_header.c_str())) == NULL ||
      (list = curl_slist_append(list, "Content-Type: application/json")) ==
          NULL ||
      (curl_res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list)) !=
          CURLE_OK ||
      (curl_res = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curl_errbuf)) !=
          CURLE_OK ||
      (curl_res = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1)) !=
          CURLE_OK ||
      (curl_res = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L)) !=
          CURLE_OK ||
      (curl_res = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L)) !=
          CURLE_OK ||
      (!vault_ca.empty() &&
       (curl_res = curl_easy_setopt(curl, CURLOPT_CAINFO, vault_ca.c_str())) !=
           CURLE_OK) ||
      (curl_res = curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL)) !=
          CURLE_OK ||
      (curl_res = curl_easy_setopt(
           curl, CURLOPT_URL, vault_mount_point_url.c_str())) != CURLE_OK ||
      (curl_res = curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE")) !=
          CURLE_OK ||
      (curl_res = curl_easy_perform(curl)) != CURLE_OK ||
      (curl_res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE,
                                    &http_code)) != CURLE_OK ||
      http_code / 100 != 2 /* 2** are success return codes*/
  ) {
    std::cout << "Could not delete secret mount point";
    std::cout << get_error_from_curl(curl_res).c_str();
    curl_slist_free_all(list);
    return true;
  }
  curl_slist_free_all(list);
  return false;
}

std::string Vault_mount::get_error_from_curl(CURLcode curl_code) {
  size_t len = strlen(curl_errbuf);
  std::ostringstream ss;
  if (curl_code != CURLE_OK) {
    ss << "CURL returned this error code: " << curl_code;
    ss << " with error message : ";
    if (len)
      ss << curl_errbuf;
    else
      ss << curl_easy_strerror(curl_code);
  }
  return ss.str();
}

}  // namespace keyring
