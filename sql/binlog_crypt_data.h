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
  uint get_key_version() const
  {
    return key_version;
  }

  void free_key(uchar *&key, size_t &key_length);
  bool init(uint sch, uint kv, const uchar* nonce);
  bool init_with_loaded_key(uint sch, const uchar* nonce);
  bool load_latest_binlog_key();
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
