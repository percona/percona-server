#include "vault_key.h"
#include <sstream>

namespace keyring {

my_bool Vault_key::get_next_key(IKey **key)
{
  if (was_key_retrieved)
  {
    *key = NULL;
    return TRUE;
  }
  *key = new Vault_key(*this);
  was_key_retrieved = TRUE;
  return FALSE;
}

my_bool Vault_key::has_next_key()
{
  return !was_key_retrieved;	  
}

void Vault_key::xor_data()
{
  /*We do not xor data in keyring_vault */
}

void Vault_key::create_key_signature() const
{
  if (key_id.empty())
    return;
  std::stringstream key_signature_ss;
  key_signature_ss << key_id.length() << '_';
  key_signature_ss << key_id;
  key_signature_ss << user_id.length() << '_';
  key_signature_ss << user_id;
  key_signature = key_signature_ss.str();
}

} //namespace keyring
