#include "binlog_crypt_data.h"

#include "my_global.h"
#include "my_sys.h"
#ifdef MYSQL_SERVER
#include <mysql/service_mysql_keyring.h>
#endif
#include <algorithm>

Binlog_crypt_data::Binlog_crypt_data()
  : key(NULL)
  , enabled(false)
  , scheme(0)
{}

Binlog_crypt_data::~Binlog_crypt_data()
{
  free_key();
}

Binlog_crypt_data::Binlog_crypt_data(const Binlog_crypt_data &b)
{
  enabled = b.enabled;
  key_version = b.key_version;
  if (b.key_length && b.key != NULL)
  {
    key= reinterpret_cast<uchar*>(my_malloc(PSI_NOT_INSTRUMENTED, b.key_length, MYF(MY_WME)));
    memcpy(key, b.key, b.key_length);
  }
  else
    key= NULL;

  key_length= b.key_length;
  memcpy(iv, b.iv, BINLOG_IV_LENGTH);
  dst_len = b.dst_len;
  memcpy(nonce, b.nonce, BINLOG_NONCE_LENGTH);
}

void Binlog_crypt_data::free_key()
{
  if (key != NULL)
  {
    DBUG_ASSERT(key_length == 16);
    memset_s(key, 512, 0, key_length);
    my_free(key);
    key= NULL;
    key_length= 0;
  }
}

Binlog_crypt_data& Binlog_crypt_data::operator=(Binlog_crypt_data b)
{
  enabled= b.enabled;
  key_version= b.key_version;
  key_length= b.key_length;
  std::swap(this->key, b.key);
  key_length= b.key_length;
  memcpy(iv, b.iv, BINLOG_IV_LENGTH);
  dst_len= b.dst_len;
  memcpy(nonce, b.nonce, BINLOG_NONCE_LENGTH);

  return *this;
}

bool Binlog_crypt_data::init(uint sch, uint kv, const uchar* nonce)
{
  scheme= sch;
  key_version= kv;
  free_key();
  key_length= 16;

#ifdef MYSQL_SERVER
  DBUG_ASSERT(nonce != NULL);
  memcpy(this->nonce, nonce, BINLOG_NONCE_LENGTH);

  char *key_type= NULL;
  size_t key_len;

  DBUG_EXECUTE_IF("binlog_encryption_error_on_key_fetch",
                  { return 1; } );

  if (my_key_fetch("percona_binlog", &key_type, NULL,
                   reinterpret_cast<void**>(&key), &key_len) ||
      (key != NULL && key_len != 16))
  {
    free_key();
    if (key_type != NULL)
      my_free(key_type);
    return true;
  }
  my_free(key_type);
  key_type= NULL;

  if (key == NULL)
  {
    my_key_generate("percona_binlog", "AES", NULL, 16);
    if (my_key_fetch("percona_binlog", &key_type, NULL,
                     reinterpret_cast<void**>(&key), &key_len) ||
        key_len != 16)
    {
      free_key();
      if (key_type != NULL)
        my_free(key_type);
      return true;
    }
    DBUG_ASSERT(strncmp(key_type, "AES", 3) == 0);
  }
  my_free(key_type);
#endif
  enabled= true;
  return false;
}

bool Binlog_crypt_data::is_enabled() const
{
  return enabled;
}

void Binlog_crypt_data::disable()
{
  enabled= false;
}

uchar* Binlog_crypt_data::get_key() const
{
  return key;
}

size_t Binlog_crypt_data::get_keys_length() const
{
  return key_length;
}

void Binlog_crypt_data::set_iv(uchar* iv, uint32 offs) const
{
  DBUG_ASSERT(key != NULL && key_length == 16);

  uchar iv_plain[BINLOG_IV_LENGTH];
  memcpy(iv_plain, nonce, BINLOG_NONCE_LENGTH);
  int4store(iv_plain + BINLOG_NONCE_LENGTH, offs);

  my_aes_encrypt(iv_plain, BINLOG_IV_LENGTH, iv,
                 key, key_length, my_aes_128_ecb, NULL, false);
}
