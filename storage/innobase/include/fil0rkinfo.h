#ifndef fil0rkinfo_h
#define fil0rkinfo_h

//#ifndef UNIV_INNOCHECKSUM

#define CRYPT_SCHEME_UNENCRYPTED 0
#define CRYPT_SCHEME_1 1

struct Keyring_encryption_info
{
   Keyring_encryption_info()
     : keyring_encryption_key_is_missing(false)
     , page0_has_crypt_data(false)
     , keyring_encryption_min_key_version(0)
     , type(CRYPT_SCHEME_UNENCRYPTED)
   {}
   bool keyring_encryption_key_is_missing; // initlialized in dict_mem_table_create
   bool page0_has_crypt_data;
   uint keyring_encryption_min_key_version;
   uint type;

   bool is_encryption_in_progress()
   {
     return keyring_encryption_min_key_version == 0 && type != CRYPT_SCHEME_UNENCRYPTED;
   }
};

//#endif // UNIV_INNOCHECKSUM

#endif
