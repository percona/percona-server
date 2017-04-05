#ifndef MYSQL_I_VAULT_CURL
#define MYSQL_I_VAULT_CURL

#include "i_keyring_key.h"
#include "vault_credentials.h"
#include "vault_secure_string.h"

namespace keyring {

class IVault_curl : public Keyring_alloc
{
public:
  virtual bool init(const Vault_credentials &vault_credentials) = 0;

  virtual bool list_keys(Secure_string *response) = 0;
  virtual bool write_key(const Vault_key &key, Secure_string *response) = 0;
  virtual bool read_key(const Vault_key &key, Secure_string *response) = 0;
  virtual bool delete_key(const Vault_key &key, Secure_string *response) = 0;

  virtual ~IVault_curl() {};
};

} // namespace keyring

#endif // MYSQL_I_VAULT_CURL_H
