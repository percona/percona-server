#ifndef MYSQL_INCORRECT_VAULT_KEY_H
#define MYSQL_INCORRECT_VAULT_KEY_H

#include "vault_key.h"

namespace keyring {

struct Incorrect_vault_key : public Vault_key {
  Incorrect_vault_key(const char *a_key_id, const char *a_key_type,
                      const char *a_user_id, const void *a_key,
                      size_t a_key_len)
      : Vault_key(a_key_id, a_key_type, a_user_id, a_key, a_key_len),
        add_to_key_id_length(0),
        add_to_user_id_length(0) {}

  Incorrect_vault_key(const Incorrect_vault_key &incorrect_vault_key)
      : Vault_key(incorrect_vault_key),
        add_to_key_id_length(incorrect_vault_key.add_to_key_id_length),
        add_to_user_id_length(incorrect_vault_key.add_to_user_id_length) {}

  int add_to_key_id_length;
  int add_to_user_id_length;

  virtual bool get_next_key(IKey **key) {
    if (was_key_retrieved) {
      *key = nullptr;
      return true;
    }
    *key = new Incorrect_vault_key(*this);
    was_key_retrieved = true;
    return false;
  }

  virtual void create_key_signature() const {
    if (key_id.empty()) return;
    std::ostringstream key_signature_ss;
    key_signature_ss << key_id.length() + add_to_key_id_length << '_';
    key_signature_ss << key_id;
    key_signature_ss << user_id.length() + add_to_user_id_length << '_';
    key_signature_ss << user_id;
    key_signature = key_signature_ss.str();
  }
};

}  // namespace keyring

#endif  // MYSQL_INCORRECT_VAULT_KEY_H
