#include "vault_keys_list.h"

namespace keyring
{

//The caller takes ownership of the key, thus it is
//his resposibility to free the key	
my_bool Vault_keys_list::get_next_key(IKey **key)
{
  *key= NULL;
  if (size() == 0 || keys_iter == keys.end())
    return TRUE;
  *key = *(keys_iter++);
  return FALSE;
}

my_bool Vault_keys_list::has_next_key()
{
  return size() != 0 && keys_iter != keys.end();
}

size_t Vault_keys_list::size()
{
  return keys.size();
}

Vault_keys_list::~Vault_keys_list()
{
  //remove not fetched keys
  if (size() > 0)
  {
    while(keys_iter != keys.end())
      delete *keys_iter;
  }
}

void Vault_keys_list::push_back(IKey* key)
{
  keys.push_back(key);
  if(keys.size() == 1)
    keys_iter = keys.begin();
}

} //namespace keyring
