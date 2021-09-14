/* Copyright (c) 2018, 2021, Oracle and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "./enc_stream_cipher.h"
#include <algorithm>
#include <cstring>
#include <memory>

#ifndef assert
#define assert(x)
#endif
namespace myrocks {

#define HAVE_BYTESTOKEY_SHA512_HANDLING
#define HAVE_DECRYPTION_INTO_SAME_SOURCE_BUFFER

typedef unsigned char uchar;
typedef unsigned int uint32;
typedef unsigned long long ulonglong;
// from big_endian.h
static inline void int4store(uchar *T, uint32 A) {
  *(T) = (uchar)(A);
  *(T + 1) = (uchar)(A >> 8);
  *(T + 2) = (uchar)(A >> 16);
  *(T + 3) = (uchar)(A >> 24);
}

static inline void int8store(uchar *T, ulonglong A) {
  uint def_temp = (uint)A, def_temp2 = (uint)(A >> 32);
  int4store(T, def_temp);
  int4store(T + 4, def_temp2);
}

int Stream_cipher::get_header_size() { return m_header_size; }

std::unique_ptr<Stream_cipher> Aes_ctr::get_encryptor() {
  std::unique_ptr<Stream_cipher> cipher(new Aes_ctr_encryptor);
  return cipher;
}

std::unique_ptr<Stream_cipher> Aes_ctr::get_decryptor() {
  std::unique_ptr<Stream_cipher> cipher(new Aes_ctr_decryptor);
  return cipher;
}

template <Cipher_type TYPE>
bool Aes_ctr_cipher<TYPE>::open(const unsigned char *fileKey,
                                const unsigned char *iv) {
  m_header_size = 0;

  memcpy(m_file_key, fileKey, FILE_KEY_LENGTH);
  memcpy(m_iv, iv, AES_BLOCK_SIZE);

  /*
    AES-CTR counter is set to 0. Data stream is always encrypted beginning with
    counter 0.
  */
  return init_cipher(0);
}

template <Cipher_type TYPE>
Aes_ctr_cipher<TYPE>::~Aes_ctr_cipher<TYPE>() {
  close();
}

template <Cipher_type TYPE>
void Aes_ctr_cipher<TYPE>::close() {
  deinit_cipher();
}

template <Cipher_type TYPE>
bool Aes_ctr_cipher<TYPE>::set_stream_offset(uint64_t offset) {
  unsigned char buffer[AES_BLOCK_SIZE];
  /* A seek in the down stream would overflow the offset */
  if (offset > UINT64_MAX - m_header_size) return true;

  // check if we are already on this offset to avoid cipher reinitialization
  if (offset == m_streamOffset) {
    return false;
  }

  deinit_cipher();
  if (init_cipher(offset)) return true;
  /*
    The cipher works with blocks. While init_cipher() above is called it will
    initialize the cipher assuming it is pointing to the beginning of a block,
    the following encrypt/decrypt operations will adjust the cipher to point to
    the requested offset in the block, so next encrypt/decrypt operations will
    work fine without the need to take care of reading from/writing to the
    middle of a block.
  */
  if (TYPE == Cipher_type::ENCRYPT)
    return encrypt(buffer, buffer, offset % AES_BLOCK_SIZE);
  else
    return decrypt(buffer, buffer, offset % AES_BLOCK_SIZE);
}

template <Cipher_type TYPE>
bool Aes_ctr_cipher<TYPE>::init_cipher(uint64_t offset) {
  uint64_t counter = offset / AES_BLOCK_SIZE;

  assert(m_ctx == nullptr);
  m_ctx = EVP_CIPHER_CTX_new();
  if (m_ctx == nullptr) return true;

  /*
    AES's IV is 16 bytes.
    In CTR mode, we will use the last 8 bytes as the counter.
    Counter is stored in big-endian.
  */
  int8store(m_iv + 8, counter);
  /* int8store stores it in little-endian, so swap it to big-endian */
  std::swap(m_iv[8], m_iv[15]);
  std::swap(m_iv[9], m_iv[14]);
  std::swap(m_iv[10], m_iv[13]);
  std::swap(m_iv[11], m_iv[12]);

  int res;
  const EVP_CIPHER *cipher_type = EVP_aes_256_ctr();

  /* EVP_CipherInit() returns 1 for success and 0 for failure */
  res = EVP_CipherInit(m_ctx, cipher_type, m_file_key, m_iv,
                       static_cast<int>(TYPE));

  m_streamOffset = offset;
  return res == 0;
}

template <Cipher_type TYPE>
void Aes_ctr_cipher<TYPE>::deinit_cipher() {
  if (m_ctx) EVP_CIPHER_CTX_free(m_ctx);
  m_ctx = nullptr;
}

template <Cipher_type TYPE>
bool Aes_ctr_cipher<TYPE>::encrypt(unsigned char *dest,
                                   const unsigned char *src, int length) {
  if (TYPE == Cipher_type::DECRYPT) {
    /* It should never be called by a decrypt cipher */
    assert(0);
    return true;
  }

  /* length == 0 : nothing to encrypt */
  if (length == 0) return false;

  m_streamOffset += length;

  if (EVP_Cipher(m_ctx, dest, const_cast<unsigned char *>(src), length) == 0)
    return true;

  return false;
}

template <Cipher_type TYPE>
bool Aes_ctr_cipher<TYPE>::decrypt(unsigned char *dest,
                                   const unsigned char *src, int length) {
  if (TYPE == Cipher_type::ENCRYPT) {
    /* It should never be called by an encrypt cipher */
    assert(0);
    return true;
  }

  /* length == 0 : nothing to decrypt */
  if (length == 0) return false;

  m_streamOffset += length;

#ifdef HAVE_DECRYPTION_INTO_SAME_SOURCE_BUFFER
  if (EVP_Cipher(m_ctx, dest, const_cast<unsigned char *>(src), length) == 0)
    return true;
#else
  const int DECRYPTED_BUFFER_SIZE = AES_BLOCK_SIZE * 2048;
  unsigned char buffer[DECRYPTED_BUFFER_SIZE];

  /* Decrypt in up to DECRYPTED_BUFFER_SIZE chunks */
  while (length != 0) {
    int chunk_len = std::min(length, DECRYPTED_BUFFER_SIZE);
    if (EVP_Cipher(m_ctx, buffer, const_cast<unsigned char *>(src),
                   chunk_len) == 0)
      return true;
    memcpy(dest, buffer, chunk_len);
    src += chunk_len;
    dest += chunk_len;
    length -= chunk_len;
  }
#endif

  return false;
}

template class Aes_ctr_cipher<Cipher_type::ENCRYPT>;
template class Aes_ctr_cipher<Cipher_type::DECRYPT>;

}  // namespace myrocks
