/*
 Copyright (c) 2014 Google Inc.
 Copyright (c) 2014, 2015 MariaDB Corporation

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; version 2 of the License.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef MY_CRYPT_INCLUDED
#define MY_CRYPT_INCLUDED

#include <my_config.h> /* HAVE_EncryptAes128{Ctr,Gcm} */
#include "my_aes.h"

#define MY_AES_OK                     0
#define MY_AES_OPENSSL_ERROR          -101
#define MY_AES_BAD_KEYSIZE            -102

/* The max key length of all supported algorithms */
#define MY_AES_MAX_KEY_LENGTH 32

#define MY_AES_CTX_SIZE 512

#define ENCRYPTION_FLAG_DECRYPT     0
#define ENCRYPTION_FLAG_ENCRYPT     1
#define ENCRYPTION_FLAG_NOPAD       2

enum my_aes_mode {
    MY_AES_ECB, MY_AES_CBC
#ifdef HAVE_EncryptAes128Ctr
  , MY_AES_CTR
#endif
#ifdef HAVE_EncryptAes128Gcm
  , MY_AES_GCM
#endif
};

class MyEncryptionCTX
{
public:
  MyEncryptionCTX();
  virtual ~MyEncryptionCTX();

  virtual int init(const my_aes_mode mode, int encrypt, const uchar *key, size_t klen,
                   const uchar *iv, size_t ivlen);
  virtual int update(const uchar *src, size_t slen, uchar *dst, size_t *dlen);
  virtual int finish(uchar *dst, size_t *dlen);

protected:
  struct Impl;
  Impl* pimpl;

private:
  MyEncryptionCTX(const MyEncryptionCTX &enc);
};

int my_aes_crypt_init(MyEncryptionCTX* &ctx, enum my_aes_mode mode, int flags,
                      const unsigned char* key, size_t klen,
                      const unsigned char* iv, size_t ivlen);
int my_aes_crypt_update(MyEncryptionCTX *ctx, const unsigned char *src, size_t slen,
                        unsigned char *dst, size_t *dlen);
int my_aes_crypt_finish(MyEncryptionCTX* &ctx, uchar *dst, size_t *dlen);
int my_aes_crypt(enum my_aes_mode mode, int flags,
                 const unsigned char *src, size_t slen, unsigned char *dst, size_t *dlen,
                 const unsigned char *key, size_t klen, const unsigned char *iv, size_t ivlen);

int my_random_bytes(unsigned char* buf, int num);
size_t my_aes_crypt_get_size(enum my_aes_mode mode, size_t source_length);

#endif /* MY_CRYPT_INCLUDED */
