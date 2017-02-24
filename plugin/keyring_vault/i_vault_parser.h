#ifndef MYSQL_I_VAULT_PARSER_H
#define MYSQL_I_VAULT_PARSER_H

#include "my_global.h"
#include "i_keyring_key.h"
#include "vault_keys_list.h"
#include "logger.h"

namespace keyring
{
  class IVault_parser
  {
  public:
    virtual my_bool parse_keys(std::string *payload, Vault_keys_list *keys) = 0;
    virtual my_bool parse_key_data(std::string *payload, IKey *key) = 0;
    virtual my_bool parse_key_signature(const std::string *key_signature, std::string key_parameters[2]) = 0;
    virtual my_bool parse_errors(std::string *payload, std::string *errors) = 0;
    virtual ~IVault_parser() {}
  };
} //namespace keyring

#endif // MYSQL_I_VAULT_PARSER_H

