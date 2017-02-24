#include "vault_parser.h"
#include "vault_key.h"
#include <vector>
#include <algorithm>
#include "base64.h"

namespace keyring
{

my_bool Vault_parser::retrieve_tag_value(std::string *payload, std::string *tag, char opening_bracket,
                                         char closing_bracket, std::string *value)
{
  value->clear();
  size_t opening_bracket_pos, closing_bracket_pos, tag_pos = payload->find(*tag);
  if (tag_pos == std::string::npos)
    return FALSE;

  if ((opening_bracket_pos = (payload->find(opening_bracket, tag_pos))) == std::string::npos ||
      (closing_bracket_pos = (payload->find(closing_bracket, opening_bracket_pos))) == std::string::npos)
  {
    std::stringstream err_ss("Could not parse tag ");
    err_ss << *tag << " from Vault's response."; 
    logger->log(MY_ERROR_LEVEL, err_ss.str().c_str());
    return TRUE;
  }

 *value = payload->substr(opening_bracket_pos, closing_bracket_pos - opening_bracket_pos +1);
  value->erase(std::remove(value->begin(), value->end(), '\n'), value->end());
  return FALSE;
}

my_bool Vault_parser::retrieve_list(std::string *payload, std::string list_name, std::string *list)
{
  return retrieve_tag_value(payload, &list_name, '[', ']', list);
}

my_bool Vault_parser::retrieve_map(std::string *payload, std::string map_name, std::string *map)
{
  return retrieve_tag_value(payload, &map_name, '{', '}', map);
}

my_bool Vault_parser::retrieve_tokens_from_list(std::string *list, std::vector<std::string> *tokens)
{
  std::size_t token_start = 0, token_end = 0;
  while ((token_start = list->find('\"', token_end))
           != std::string::npos &&
         token_start < list->size())
  {
    if ((token_end = list->find('\"', token_start+1)) == std::string::npos)
    {
      tokens->clear();
      return TRUE;
    }
    tokens->push_back(list->substr(token_start+1,
                                   token_end-token_start-1));
    ++token_end;
  }
  return FALSE;
}

my_bool Vault_parser::retrieve_value_from_map(std::string *map, std::string key, std::string *value)
{
  size_t key_tag_pos = std::string::npos, value_start_pos = std::string::npos, 
         value_end_pos = std::string::npos;
  my_bool was_error = FALSE;

  if ((key_tag_pos = map->find(key)) != std::string::npos &&
      (value_start_pos = map->find(":\"", key_tag_pos)) != std::string::npos &&
      (value_end_pos = map->find("\"", value_start_pos + strlen(":\""))) != std::string::npos)
  {
    value_start_pos += strlen(":\"");
    DBUG_ASSERT(value_end_pos > 0);
    value_end_pos--; //due to closing "
    *value = map->substr(value_start_pos, (value_end_pos-value_start_pos+1));
  }
  else
    was_error = TRUE;

  if (was_error || value->empty())
  {
    std::stringstream err_ss;
    err_ss << "Could not parse " << key << " tag for a key.";
    logger->log(MY_ERROR_LEVEL, err_ss.str().c_str());
    return TRUE;
  }
  return FALSE;
}

my_bool Vault_parser::parse_errors(std::string *payload, std::string *errors)
{
  return retrieve_list(payload, "errors", errors);
}

my_bool Vault_parser::parse_keys(std::string *payload, Vault_keys_list *keys)
{
  /* payload is build as follows:
   * (...)"data":{"keys":["keysignature","keysignature"]}(...)
   * We need to retrieve keys signatures from it
   */
  std::vector<std::string> key_tokens;
  std::string keys_list;
  if (retrieve_list(payload, "keys", &keys_list) ||
      keys_list.empty() ||
      retrieve_tokens_from_list(&keys_list, &key_tokens))
  {
    logger->log(MY_ERROR_LEVEL, "Could not parse keys tag with keys list from Vault.");
    return TRUE;
  }
  std::string key_parameters[2];
  for(std::vector<std::string>::const_iterator iter = key_tokens.begin();
      iter != key_tokens.end(); ++iter)
  {
    if (parse_key_signature(&*iter, key_parameters))
    {
      logger->log(MY_ERROR_LEVEL, "Could not parse key's signature");
      return TRUE;
    }
    keys->push_back(new Vault_key(key_parameters[0].c_str(), NULL,
                                  key_parameters[1].c_str(), NULL, 0));
  }
  return FALSE;
}

my_bool Vault_parser::parse_key_signature(const std::string *key_signature, std::string key_parameters[2])
{
  //key_signature= lengthof(key_id)||key_id||lengthof(user_id)||user_id
  std::string digits("0123456789");
  size_t next_pos_to_start_from = 0;
  for (int i= 0; i < 2; ++i)
  {
    std::size_t key_id_pos = key_signature->find_first_not_of(digits, next_pos_to_start_from);
    if (key_id_pos == std::string::npos || (*key_signature)[key_id_pos] != '_')
      return TRUE;
    ++key_id_pos;
    std::string key_id_length = key_signature->substr(next_pos_to_start_from, key_id_pos);
    int key_l = atoi(key_id_length.c_str());
    if (key_l < 0)
      return TRUE;
    key_parameters[i] = key_signature->substr(key_id_pos, key_l);
    next_pos_to_start_from= key_id_pos+key_l;
  }
  return FALSE;
}

my_bool Vault_parser::parse_key_data(std::string *payload, IKey *key)
{
  std::string map, type, value;
  if (retrieve_map(payload, "data", &map) ||
      retrieve_value_from_map(&map, "type", &type) ||
      retrieve_value_from_map(&map, "value", &value))
    return TRUE;

  uint64 base64_length_of_memory_needed_for_decod = base64_needed_decoded_length(value.length());
  uchar *data = new uchar[base64_length_of_memory_needed_for_decod]; //should this memory be copied into decoded_lenght (actual memory needed)

  int64 decoded_length = base64_decode(value.c_str(), value.length(), data, NULL, 0);
  if (decoded_length < 0)
  {
    delete[] data;
    logger->log(MY_ERROR_LEVEL, "Could not decode base64 key's value");
    return TRUE;
  }
  //I am not sure do I need this
  else if (static_cast<uint64>(decoded_length) < base64_length_of_memory_needed_for_decod)//decoded key data can be fit into smaller amount of memory
  {
    uchar *tmp_data = data;
    data = new uchar[decoded_length];
    memcpy(data, tmp_data, decoded_length);
    memset(tmp_data, 0, base64_length_of_memory_needed_for_decod);
    delete[] tmp_data; 
  }
  key->set_key_data(data, decoded_length);
  key->set_key_type(&type);

  return FALSE;
}

} //namespace keyring
