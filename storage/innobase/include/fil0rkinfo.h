#ifndef fil0rkinfo_h
#define fil0rkinfo_h

#define CRYPT_SCHEME_UNENCRYPTED 0
#define CRYPT_SCHEME_1 1

struct Keyring_encryption_info {
  bool keyring_encryption_key_is_missing{
      false};  // initlialized in dict_mem_table_create
  bool page0_has_crypt_data{false};
  uint keyring_encryption_min_key_version{0};
  uint type{CRYPT_SCHEME_UNENCRYPTED};
  bool is_mk_to_keyring_rotation{false};
  size_t private_version{3};

  bool is_encryption_in_progress() {
    return keyring_encryption_min_key_version == 0 &&
           type != CRYPT_SCHEME_UNENCRYPTED;
  }
};

#endif
