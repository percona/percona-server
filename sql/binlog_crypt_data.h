#ifndef BINLOG_CRYPT_DATA_H
#define BINLOG_CRYPT_DATA_H

#include "my_global.h"
#include "my_crypt.h"

class Binlog_crypt_data
{
public:
  enum Binlog_crypt_consts
  {
    BINLOG_CRYPTO_SCHEME_LENGTH= 1,
    BINLOG_KEY_VERSION_LENGTH= 4,
    BINLOG_IV_LENGTH= MY_AES_BLOCK_SIZE,
    BINLOG_IV_OFFS_LENGTH= 4,
    BINLOG_NONCE_LENGTH= BINLOG_IV_LENGTH - BINLOG_IV_OFFS_LENGTH
  };

  Binlog_crypt_data();
  ~Binlog_crypt_data();
  Binlog_crypt_data(const Binlog_crypt_data &b);
  Binlog_crypt_data& operator=(Binlog_crypt_data b);

  bool is_enabled() const
  {
    return enabled;
  }
  void disable()
  {
    enabled= false;
  }
  uchar* get_key() const
  {
    return key;
  }
  size_t get_keys_length() const
  {
    return key_length;
  }

  void free_key();
  bool init(uint sch, uint kv, const uchar* nonce);
  void set_iv(uchar* iv, uint32 offs) const;

private:
  uint  key_version;
  size_t key_length;
  uchar *key;
  uchar nonce[BINLOG_NONCE_LENGTH];
  uint dst_len;
  uchar iv[BINLOG_IV_LENGTH];
  bool enabled;
  uint scheme;
};

#endif //BINLOG_CRYPT_DATA_H
