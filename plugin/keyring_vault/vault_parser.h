#ifndef MYSQL_VAULT_PARSER_H
#define MYSQL_VAULT_PARSER_H

#include "my_global.h"
#include "i_vault_parser.h"
#include "logger.h"
#include "vault_memory.h"

namespace keyring
{

class Vault_parser : public IVault_parser
{
public:
  Vault_parser(ILogger *logger)
    : logger(logger)
  {}

  bool parse_keys(const Secure_string &payload, Vault_keys_list *keys);
  bool parse_key_data(const Secure_string &payload, IKey *key);
  bool parse_key_signature(const Secure_string &base64_key_signature, Secure_string key_parameters[2]);
  bool parse_errors(const Secure_string &payload, Secure_string *errors);

private:
  bool retrieve_tag_value(const Secure_string &payload, const Secure_string &tag, const char opening_bracket,
                          const char closing_bracket, Secure_string *value);
  bool retrieve_list(const Secure_string &payload, const Secure_string &list_name, Secure_string *list);
  bool retrieve_map(const Secure_string &payload, const Secure_string &map_name, Secure_string *map);
  bool retrieve_tokens_from_list(const Secure_string &list, std::vector<Secure_string> *tokens);
  bool retrieve_value_from_map(const Secure_string &map, const Secure_string &key,
                               Secure_string *value);


  ILogger *logger;
};

} // namespace keyring

#endif // MYSQL_VAULT_PARSER_H
