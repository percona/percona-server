#ifndef MYSQL_VAULT_KEYS_H
#define MYSQL_VAULT_KEYS_H

#include "i_serialized_object.h"
#include <list>

namespace keyring
{

class Vault_keys_list : public ISerialized_object
{
public:
  Vault_keys_list()
  {}

  virtual my_bool get_next_key(IKey **key);
  virtual my_bool has_next_key();
  void push_back(IKey* key);
  size_t size();

  ~Vault_keys_list();

protected:
  typedef std::list<IKey*> Keys_list;
  Keys_list keys;
  Keys_list::const_iterator keys_iter;
};

} //namespace keyring

#endif //MYSQL_VAULT_KEYS_H
