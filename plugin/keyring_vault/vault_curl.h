#ifndef MYSQL_VAULT_CURL_H
#define MYSQL_VAULT_CURL_H

#include <my_global.h>
#include <boost/core/noncopyable.hpp>
#include <curl/curl.h>
#include <sstream>
#include "vault_key.h"
#include "i_vault_curl.h"
#include "logger.h"
#include "vault_credentials.h"
#include "vault_secure_string.h"
#include "i_keyring_key.h"

namespace keyring
{

class Vault_curl : public IVault_curl, private boost::noncopyable
{
public:
  Vault_curl(ILogger *logger, CURL *curl)
    : logger(logger)
    , curl(curl)
    , list(NULL)
  {}

  ~Vault_curl()
  {
    if (list != NULL)
      curl_slist_free_all(list);
  }

  virtual bool init(const Vault_credentials &vault_credentials);
  virtual bool list_keys(Secure_string *response);
  virtual bool write_key(const Vault_key &key, Secure_string *response);
  virtual bool read_key(const Vault_key &key, Secure_string *response);
  virtual bool delete_key(const Vault_key &key, Secure_string *response);

private:

  bool reset_curl_session();
  std::string get_error_from_curl(CURLcode curl_code);
  bool encode_key_signature(const Vault_key &key, Secure_string *encoded_key_signature);
  bool get_key_url(const Vault_key &key, Secure_string *key_url);

  ILogger *logger;
  Secure_string token_header;
  Secure_string vault_url;
  CURL *curl;
  char curl_errbuf[CURL_ERROR_SIZE]; //error from CURL
  Secure_ostringstream read_data_ss;
  struct curl_slist *list;
  Secure_string vault_ca;
};

} //namespace keyring

#endif //MYSQL_VAULT_CURL_H
