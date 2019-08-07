#ifndef keyring_encryption_key_info_h
#define keyring_encryption_key_info_h

using EncryptionKeyId = uint32_t;

static const uint ENCRYPTION_KEY_VERSION_INVALID = (~(uint)0);
static const uint FIL_DEFAULT_ENCRYPTION_KEY = 0;
static const uint ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED = 0;

struct KeyringEncryptionKeyIdInfo {
  KeyringEncryptionKeyIdInfo(bool was_encryption_key_id_set,
                             EncryptionKeyId encryption_key_id)
      : was_encryption_key_id_set(was_encryption_key_id_set),
        id(encryption_key_id) {}

  KeyringEncryptionKeyIdInfo() {}

  bool was_encryption_key_id_set{false};
  EncryptionKeyId id{FIL_DEFAULT_ENCRYPTION_KEY};
};

#endif
