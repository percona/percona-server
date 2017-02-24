#ifndef MYSQL_VAULT_PARSER_H
#define MYSQL_VAULT_PARSER_H

#include "my_global.h"
#include "i_vault_parser.h"
#include "logger.h"

namespace keyring
{

class Vault_parser : public IVault_parser
{
public:
  Vault_parser(ILogger *logger)
    : logger(logger)
  {}

  my_bool parse_keys(std::string *payload, Vault_keys_list *keys);
  my_bool parse_key_data(std::string *payload, IKey *key);
  my_bool parse_key_signature(const std::string *key_signature, std::string key_parameters[2]);
  my_bool parse_errors(std::string *payload, std::string *errors);

protected:
  my_bool retrieve_tag_value(std::string *payload, std::string *tag, char opening_bracket,
                             char closing_bracket, std::string *value);
  my_bool retrieve_list(std::string *payload, std::string list_name, std::string *list);
  my_bool retrieve_map(std::string *payload, std::string map_name, std::string *map);
  my_bool retrieve_tokens_from_list(std::string *list, std::vector<std::string> *tokens);
  my_bool retrieve_value_from_map(std::string *map, std::string key, std::string *value);

  ILogger *logger;
};

} //namespace keyring

#endif //MYSQL_VAULT_PARSER_H

