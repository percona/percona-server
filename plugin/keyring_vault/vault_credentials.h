#ifndef MYSQL_VAULT_CREDENTIALS
#define MYSQL_VAULT_CREDENTIALS

#include <my_global.h>
#include <map>
#include "vault_memory.h"
#include "vault_secure_string.h"

namespace keyring
{
  typedef std::map<Secure_string, Secure_string> Vault_credentials;
  const Secure_string& get_credential(const Vault_credentials &credentials, const Secure_string &key);
} // namespace keyring

#endif // MYSQL_VAULT_CREDENTIALS
