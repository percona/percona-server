#ifndef MYSQL_VAULT_CURL_H
#define MYSQL_VAULT_CURL_H

#include <my_global.h>
#include <curl/curl.h>
#include "i_vault_curl.h"
#include "logger.h"
#include "vault_credentials.h"
#include "i_keyring_key.h"

namespace keyring
{

class Vault_curl : public IVault_curl
{
public:
  Vault_curl(ILogger *logger)
    : logger(logger)
    , curl(NULL)	
    , list(NULL)
  {}

  ~Vault_curl()
  {
    if (list != NULL)
      curl_slist_free_all(list);
    curl_easy_cleanup(curl);
  }

  my_bool init(Vault_credentials *vault_credentials);
  my_bool list_keys(std::string *response);
  my_bool write_key(IKey *key, std::string *response);
  my_bool read_key(IKey *key, std::string *response);
  my_bool delete_key(IKey *key, std::string *response);

protected:
  my_bool reset_curl_session();
  std::string get_error_from_curl(CURLcode curl_code);

  ILogger *logger;
  Secure_string token_header;
  Secure_string vault_url;
  CURL *curl;
  char curl_errbuf[CURL_ERROR_SIZE]; //error from CURL
  std::stringstream read_data_ss;
  struct curl_slist *list;
  Secure_string vault_ca;
};

} //namespace keyring

#endif //MYSQL_VAULT_CURL_H
