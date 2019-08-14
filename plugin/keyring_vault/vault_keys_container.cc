#include "vault_keys_container.h"

namespace keyring {
bool Vault_keys_container::init(IKeyring_io *keyring_io_value,
                                std::string keyring_storage_url_value) {
  vault_io = dynamic_cast<IVault_io *>(keyring_io_value);
  DBUG_ASSERT(vault_io != nullptr);
  return Keys_container::init(keyring_io_value, keyring_storage_url_value);
}

IKey *Vault_keys_container::fetch_key(IKey *key) {
  DBUG_ASSERT(key->get_key_data() == nullptr);
  DBUG_ASSERT(key->get_key_type()->empty());

  IKey *fetched_key = get_key_from_hash(key);

  if (fetched_key == nullptr) return nullptr;

  if (fetched_key->get_key_type()->empty() &&
      vault_io->retrieve_key_type_and_data(
          fetched_key))  // key is fetched for the first time
    return nullptr;

  return Keys_container::fetch_key(key);
}

bool Vault_keys_container::flush_to_backup() { return false; }
}  // namespace keyring
