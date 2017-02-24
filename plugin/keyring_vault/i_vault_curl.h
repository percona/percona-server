#ifndef MYSQL_I_VAULT_CURL
#define MYSQL_I_VAULT_CURL

#include "i_keyring_key.h"
#include "vault_credentials.h"

namespace keyring {

class IVault_curl : public Keyring_alloc
{
public:
  virtual my_bool init(Vault_credentials *vault_credentials) = 0;
  virtual my_bool list_keys(std::string *response) = 0;
  virtual my_bool write_key(IKey *key, std::string *response) = 0;
  virtual my_bool read_key(IKey *key, std::string *response) = 0;
  virtual my_bool delete_key(IKey *key, std::string *response) = 0;

  virtual ~IVault_curl() {};
};

}//namespace keyring

#endif //MYSQL_I_KEYS_CONTAINER_H
