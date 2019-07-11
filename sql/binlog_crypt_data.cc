/* Copyright (c) 2018 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; version 2 of
   the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#include "binlog_crypt_data.h"

#include "my_global.h"
#include "my_sys.h"
#ifdef MYSQL_SERVER
#include <mysql/service_mysql_keyring.h>
#include "system_key.h"
#include "log.h"
#include <sstream>
#endif

Binlog_crypt_data::Binlog_crypt_data()
  : key_length(0)
  , key(NULL)
  , enabled(false)
  , scheme(0)
{}

Binlog_crypt_data::~Binlog_crypt_data()
{
  free_key(key, key_length);
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
  memcpy(nonce, b.nonce, BINLOG_NONCE_LENGTH);
}

void Binlog_crypt_data::free_key(uchar *&key, size_t &key_length)
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
  memcpy(nonce, b.nonce, BINLOG_NONCE_LENGTH);
  return *this;
}

bool Binlog_crypt_data::load_latest_binlog_key()
{
  free_key(key, key_length);
  bool error= false;
#ifdef MYSQL_SERVER
  char *system_key_type = NULL;
  size_t system_key_len = 0;
  uchar *system_key = NULL;

  DBUG_EXECUTE_IF("binlog_encryption_error_on_key_fetch",
                  { return true; } );

  if (my_key_fetch(PERCONA_BINLOG_KEY_NAME, &system_key_type, NULL,
                   reinterpret_cast<void**>(&system_key), &system_key_len) ||
      (system_key == NULL &&
       (my_key_generate(PERCONA_BINLOG_KEY_NAME, "AES", NULL, 16) ||
        my_key_fetch(PERCONA_BINLOG_KEY_NAME, &system_key_type, NULL,
                     reinterpret_cast<void**>(&system_key), &system_key_len) ||
        system_key == NULL)))
         return true;

  DBUG_ASSERT(strncmp(system_key_type, "AES", 3) == 0);
  my_free(system_key_type);

  error= (parse_system_key(system_key, system_key_len, &key_version, &key, &key_length) == reinterpret_cast<uchar*>(NullS));
  my_free(system_key);
#endif
  return error;
}

bool Binlog_crypt_data::init_with_loaded_key(uint sch, const uchar* nonce)
{
  scheme= sch;
#ifdef MYSQL_SERVER
  DBUG_ASSERT(key != NULL);
  DBUG_ASSERT(nonce != NULL);
  memcpy(this->nonce, nonce, BINLOG_NONCE_LENGTH);
#endif
  enabled= true;
  return false;
}

bool Binlog_crypt_data::init(uint sch, uint kv, const uchar* nonce)
{
  free_key(key, key_length);
#ifdef MYSQL_SERVER
  char *key_type = NULL;
  std::ostringstream percona_binlog_with_ver_ss;
  percona_binlog_with_ver_ss << PERCONA_BINLOG_KEY_NAME << ':' << kv;
  if (my_key_fetch(percona_binlog_with_ver_ss.str().c_str(), &key_type, NULL,
                   reinterpret_cast<void**>(&key), &key_length) ||
      key == NULL)
    return true;
  DBUG_ASSERT(strncmp(key_type, "AES", 3) == 0);
  my_free(key_type);

  if(init_with_loaded_key(sch, nonce))
  {
    free_key(key, key_length);
    return true;
  }
#endif
  return false;
}

void Binlog_crypt_data::set_iv(uchar* iv, uint32 offs) const
{
  DBUG_ASSERT(key != NULL);
  DBUG_ASSERT(key_length == 16);

  uchar iv_plain[BINLOG_IV_LENGTH];
  memcpy(iv_plain, nonce, BINLOG_NONCE_LENGTH);
  int4store(iv_plain + BINLOG_NONCE_LENGTH, offs);

  my_aes_encrypt(iv_plain, BINLOG_IV_LENGTH, iv,
                 key, key_length, my_aes_128_ecb, NULL, false);
}
