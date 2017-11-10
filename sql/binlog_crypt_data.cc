#include "binlog_crypt_data.h"

#include "my_global.h"
#include "my_sys.h"
#ifdef MYSQL_SERVER
#include <mysql/service_mysql_keyring.h>
#endif
#include <algorithm>
#include "boost/move/unique_ptr.hpp"

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
  this->swap(b);
  return *this;
}

void Binlog_crypt_data::swap(Binlog_crypt_data &b)
{
  enabled= b.enabled;
  key_version= b.key_version;
  key_length= b.key_length;
  std::swap(this->key, b.key);
  key_length= b.key_length;
  memcpy(iv, b.iv, BINLOG_IV_LENGTH);
  dst_len= b.dst_len;
  memcpy(nonce, b.nonce, BINLOG_NONCE_LENGTH);
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

  boost::movelib::unique_ptr<char, void (*)(void*)> key_type(NULL, my_free);
  char *key_type_raw = NULL;
  size_t key_len;

  DBUG_EXECUTE_IF("binlog_encryption_error_on_key_fetch",
                  { return 1; } );

  int fetch_result = my_key_fetch("percona_binlog", &key_type_raw, NULL,
                                  reinterpret_cast<void**>(&key), &key_len);
  key_type.reset(key_type_raw);
  if (key != NULL && key_len != 16)
  {
    free_key();
    return true;
  }
  key_type.reset();

  if (key == NULL)
  {
    my_key_generate("percona_binlog", "AES", NULL, 16);
    fetch_result = my_key_fetch("percona_binlog", &key_type_raw, NULL,
                                reinterpret_cast<void**>(&key), &key_len);
    key_type.reset(key_type_raw);
    if (fetch_result || key_len != 16)
    {
      free_key();
      return true;
    }
    DBUG_ASSERT(strncmp(key_type.get(), "AES", 3) == 0);
  }
#endif
  enabled= true;
  return false;
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
