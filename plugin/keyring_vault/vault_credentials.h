#ifndef MYSQL_VAULT_CREDENTIALS
#define MYSQL_VAULT_CREDENTIALS

#include <my_global.h>
#include <map>
#include <string>
#include "vault_memory.h"

namespace keyring
{
  typedef std::basic_string<char, std::char_traits<char>, Secure_allocator<char> > Secure_string;
  typedef std::map<Secure_string, Secure_string> Vault_credentials;
} //namespace keyring

#endif //MYSQL_VAULT_CREDENTIALS
