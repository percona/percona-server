#include <algorithm>
#include "vault_curl.h"
#include "base64.h"

namespace keyring
{

static size_t write_response_memory(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  std::stringstream *read_data = (std::stringstream *)userp;

  read_data->write((char*)contents, realsize);
  if (!read_data->good())
    return 0;
  return realsize;
}

std::string Vault_curl::get_error_from_curl(CURLcode curl_code)
{
  size_t len = strlen(curl_errbuf);
  std::stringstream ss;
  if (curl_code != CURLE_OK)
  {
    ss << "Curl returned this error code: " << curl_code;
    ss << " with error message : ";
    if(len)
      ss << curl_errbuf;
    else
      ss << curl_easy_strerror(curl_code);
  }
  return ss.str();
}

my_bool Vault_curl::init(Vault_credentials *vault_credentials)
{
  curl = curl_easy_init();
  if (curl == NULL)
  {
    logger->log(MY_ERROR_LEVEL, "Could not create CURL session");
    return TRUE;
  }
  this->token_header = "X-Vault-Token:" + (*vault_credentials)["token"];
  this->vault_url = (*vault_credentials)["vault_url"] + "/v1/" + (*vault_credentials)["secret_mount_point"];
  this->vault_ca = (*vault_credentials)["vault_ca"];
  if(this->vault_ca.empty())
  {
    logger->log(MY_WARNING_LEVEL, "There is no vault_ca specified in keyring_vault's configuration file. "
                                  "Please make sure that Vault's CA certificate is trusted by the machine from "
                                  "which you intend to connect to Vault."); 
  }
  return FALSE;
}

my_bool Vault_curl::reset_curl_session()
{
  CURLcode curl_res = CURLE_OK;
  curl_easy_reset(curl);
  read_data_ss.str("");
  read_data_ss.clear();
  curl_errbuf[0] = '\0';
  if (list != NULL)
  {
    curl_slist_free_all(list);
    list = NULL;
  }

  if ((list = curl_slist_append(list, token_header.c_str())) == NULL ||
      (list = curl_slist_append(list, "Content-Type: application/json")) == NULL ||
      (curl_res = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curl_errbuf)) != CURLE_OK ||
      (curl_res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_response_memory)) != CURLE_OK ||
      (curl_res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, reinterpret_cast<void*>(&read_data_ss))) != CURLE_OK ||
      (curl_res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list)) != CURLE_OK ||
      (curl_res = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1)) != CURLE_OK ||
      (curl_res = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L)) != CURLE_OK ||
      (vault_ca.empty() == false &&
       (curl_res = curl_easy_setopt(curl, CURLOPT_CAINFO, vault_ca.c_str())) != CURLE_OK
      ) ||
      (curl_res = curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL)) != CURLE_OK)
  {
    logger->log(MY_ERROR_LEVEL, get_error_from_curl(curl_res).c_str());
    return TRUE;
  }
  return FALSE;
}

my_bool Vault_curl::list_keys(std::string *response)
{
  CURLcode curl_res = CURLE_OK;
  curl_easy_reset(curl);
  long http_code = 0;

  if (reset_curl_session() ||
      (curl_res = curl_easy_setopt(curl, CURLOPT_URL, (vault_url + "?list=true").c_str())) != CURLE_OK ||
      (curl_res = curl_easy_perform(curl)) != CURLE_OK ||
      (curl_res = curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code)) != CURLE_OK)
  {
    logger->log(MY_ERROR_LEVEL,
                get_error_from_curl(curl_res).c_str());
    return TRUE;
  }
  if (http_code == 404)
  {
    *response=""; //no keys found
    return FALSE; 
  }
  *response = read_data_ss.str();
  return http_code != 200 && http_code != 202; //200 and 202 are success return code
}

my_bool Vault_curl::write_key(IKey *key, std::string *response)
{
  //base64 encoding
  uint64 memory_needed = base64_needed_encoded_length(key->get_key_data_size());
  char *base64_encoded_key_data = new char[memory_needed];
  if (base64_encode((const char*)key->get_key_data(), key->get_key_data_size(), base64_encoded_key_data) != 0)
  {
    delete[] base64_encoded_key_data;
    logger->log(MY_ERROR_LEVEL, "Could not encode a key in base64");
    return TRUE;
  }
  char* new_end = std::remove(base64_encoded_key_data, base64_encoded_key_data + memory_needed, '\n');
  memory_needed = new_end - base64_encoded_key_data;
  //base64 end of encoding
  
  CURLcode curl_res = CURLE_OK;
  std::string postdata="{\"type\":\"" + *key->get_key_type() + "\",\"";
  postdata += "value\":\"";
  postdata.append(base64_encoded_key_data, memory_needed-1); //base64 encode returns data with NULL terminating string - which we do not care about
  postdata += "\"}";
  delete[] base64_encoded_key_data;

  if (reset_curl_session() ||
      (curl_res = curl_easy_setopt(curl, CURLOPT_URL,
                                   (vault_url + '/' + key->get_key_signature()->c_str()).c_str())) != CURLE_OK ||
      (curl_res = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata.c_str())) != CURLE_OK ||
      (curl_res = curl_easy_perform(curl)) != CURLE_OK)
  {
    logger->log(MY_ERROR_LEVEL, get_error_from_curl(curl_res).c_str());
    return TRUE;
  }
  *response = read_data_ss.str();
  return FALSE;
}

my_bool Vault_curl::read_key(IKey *key, std::string *response)
{
  CURLcode curl_res = CURLE_OK;
  if (reset_curl_session() ||
      (curl_res = curl_easy_setopt(curl, CURLOPT_URL, (vault_url + '/' + key->get_key_signature()->c_str()).c_str())) !=
      CURLE_OK ||
      (curl_res = curl_easy_perform(curl)) != CURLE_OK)
  {
    logger->log(MY_ERROR_LEVEL, get_error_from_curl(curl_res).c_str());
    return TRUE;
  }
  *response = read_data_ss.str();
  return FALSE;
}

my_bool Vault_curl::delete_key(IKey *key, std::string *response)
{
  CURLcode curl_res = CURLE_OK;
  if (reset_curl_session() ||
      (curl_res = curl_easy_setopt(curl, CURLOPT_URL, (vault_url + '/' + key->get_key_signature()->c_str()).c_str())) !=
      CURLE_OK ||
      (curl_res = curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE")) != CURLE_OK ||
      (curl_res = curl_easy_perform(curl)) != CURLE_OK)
  {
    logger->log(MY_ERROR_LEVEL, get_error_from_curl(curl_res).c_str());
    return TRUE;
  }
  *response = read_data_ss.str();
  return FALSE;
}

} //namespace keyring
