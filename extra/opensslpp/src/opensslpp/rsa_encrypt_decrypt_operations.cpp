/* Copyright (c) 2022 Percona LLC and/or its affiliates. All rights reserved.

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

#include <cassert>
#include <vector>

#include <openssl/rsa.h>

#include <opensslpp/rsa_encrypt_decrypt_operations.hpp>

#include <opensslpp/core_error.hpp>
#include <opensslpp/rsa_encryption_padding.hpp>
#include <opensslpp/rsa_key.hpp>

#include "opensslpp/rsa_encryption_padding_conversions.hpp"
#include "opensslpp/rsa_key_accessor.hpp"

namespace opensslpp {

std::string encrypt_with_rsa_public_key(std::string_view input,
                                        const rsa_key &key,
                                        rsa_encryption_padding padding) {
  assert(!key.is_empty());

  if (input.size() > key.get_max_block_size_in_bytes(padding))
    throw core_error{
        "encryption block size is too long for the specified padding and RSA "
        "key"};

  std::string res(key.get_size_in_bytes(), '\0');

  auto enc_status = RSA_public_encrypt(
      input.size(), reinterpret_cast<const unsigned char *>(input.data()),
      reinterpret_cast<unsigned char *>(res.data()),
      rsa_key_accessor::get_impl_const_casted(key),
      rsa_encryption_padding_to_native_padding(padding));
  if (enc_status == -1)
    core_error::raise_with_error_string(
        "cannot encrypt data block with the specified public RSA key");

  return res;
}

std::string encrypt_with_rsa_private_key(std::string_view input,
                                         const rsa_key &key,
                                         rsa_encryption_padding padding) {
  assert(!key.is_empty());

  if (!key.is_private())
    throw core_error{"RSA key does not have private components"};

  if (padding == rsa_encryption_padding::pkcs1_oaep)
    throw core_error{
        "encrypting with RSA private key is not supported for PKCS1 OAEP "
        "padding"};

  if (input.size() > key.get_max_block_size_in_bytes(padding))
    throw core_error{
        "encryption block size is too long for the specified padding and RSA "
        "key"};

  std::string res(key.get_size_in_bytes(), '\0');

  auto enc_status = RSA_private_encrypt(
      input.size(), reinterpret_cast<const unsigned char *>(input.data()),
      reinterpret_cast<unsigned char *>(res.data()),
      rsa_key_accessor::get_impl_const_casted(key),
      rsa_encryption_padding_to_native_padding(padding));
  if (enc_status == -1)
    core_error::raise_with_error_string(
        "cannot encrypt data block with the specified private RSA key");

  return res;
}

std::string decrypt_with_rsa_public_key(std::string_view input,
                                        const rsa_key &key,
                                        rsa_encryption_padding padding) {
  assert(!key.is_empty());

  if (padding == rsa_encryption_padding::pkcs1_oaep)
    throw core_error{
        "decrypting with RSA public key is not supported for PKCS1 OAEP "
        "padding"};

  if (input.size() != key.get_size_in_bytes())
    throw core_error{
        "decryption block size is not the same as RSA key length in bytes"};

  std::string res(key.get_size_in_bytes(), '\0');

  auto enc_status = RSA_public_decrypt(
      input.size(), reinterpret_cast<const unsigned char *>(input.data()),
      reinterpret_cast<unsigned char *>(res.data()),
      rsa_key_accessor::get_impl_const_casted(key),
      rsa_encryption_padding_to_native_padding(padding));
  if (enc_status == -1)
    core_error::raise_with_error_string(
        "cannot decrypt data block with the specified public RSA key");

  assert(enc_status >= 0);
  res.resize(static_cast<std::size_t>(enc_status));
  return res;
}

std::string decrypt_with_rsa_private_key(std::string_view input,
                                         const rsa_key &key,
                                         rsa_encryption_padding padding) {
  assert(!key.is_empty());

  if (!key.is_private())
    throw core_error{"RSA key does not have private components"};

  if (input.size() != key.get_size_in_bytes())
    throw core_error{
        "decryption block size is not the same as RSA key length in bytes"};

  std::string res(key.get_size_in_bytes(), '\0');

  auto enc_status = RSA_private_decrypt(
      input.size(), reinterpret_cast<const unsigned char *>(input.data()),
      reinterpret_cast<unsigned char *>(res.data()),
      rsa_key_accessor::get_impl_const_casted(key),
      rsa_encryption_padding_to_native_padding(padding));
  if (enc_status == -1)
    core_error::raise_with_error_string(
        "cannot decrypt data block with the specified private RSA key");

  assert(enc_status >= 0);
  res.resize(static_cast<std::size_t>(enc_status));

  return res;
}

}  // namespace opensslpp
