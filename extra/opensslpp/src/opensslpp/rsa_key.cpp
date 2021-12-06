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
#include <memory>
#include <vector>

#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>

#include "opensslpp/rsa_key.hpp"

#include "opensslpp/big_number.hpp"
#include "opensslpp/big_number_accessor.hpp"
#include "opensslpp/bio.hpp"
#include "opensslpp/bio_accessor.hpp"
#include "opensslpp/core_error.hpp"
#include "opensslpp/rsa_key_accessor.hpp"
#include "opensslpp/rsa_padding.hpp"

namespace opensslpp {
void rsa_key::rsa_deleter::operator()(void *rsa) const noexcept {
  if (rsa != nullptr) RSA_free(static_cast<RSA *>(rsa));
}

/*static*/ const big_number rsa_key::exponent_3{RSA_3};
/*static*/ const big_number rsa_key::exponent_f4{RSA_F4};
/*static*/ const big_number &rsa_key::default_exponent{rsa_key::exponent_f4};

rsa_key::rsa_key(const rsa_key &obj)
    :  // due to a bug in openssl interface, RSAPrivateKey_dup() expects
       // non-const parameter while it does not do any modifications with the
       // object - it just performs duplication via ASN1_item_i2d/ASN1_item_d2i
       // conversions
      impl_{
          obj.is_empty() ? nullptr
          : obj.is_private()
              ? RSAPrivateKey_dup(rsa_key_accessor::get_impl_const_casted(obj))
              : RSAPublicKey_dup(
                    rsa_key_accessor::get_impl_const_casted(obj))} {
  if (!obj.is_empty() && is_empty())
    core_error::raise_with_error_string("cannot duplicate RSA key");
}

rsa_key &rsa_key::operator=(const rsa_key &obj) {
  auto tmp = rsa_key{obj};
  swap(tmp);
  return *this;
}

void rsa_key::swap(rsa_key &obj) noexcept { impl_.swap(obj.impl_); }

bool rsa_key::is_private() const noexcept {
  assert(!is_empty());
  const BIGNUM *p = nullptr;
  const BIGNUM *q = nullptr;
  RSA_get0_factors(rsa_key_accessor::get_impl(*this), &p, &q);
  return p != nullptr && q != nullptr;
}

std::size_t rsa_key::get_size_in_bits() const noexcept {
  return RSA_bits(rsa_key_accessor::get_impl(*this));
}

std::size_t rsa_key::get_size_in_bytes() const noexcept {
  return RSA_size(rsa_key_accessor::get_impl(*this));
}

std::size_t rsa_key::get_max_block_size_in_bytes(
    rsa_padding padding) const noexcept {
  std::size_t padding_bytes = 0;
  switch (padding) {
    case rsa_padding::no:
      padding_bytes = 0;
      break;
    case rsa_padding::pkcs1:
      padding_bytes = RSA_PKCS1_PADDING_SIZE;
      break;
  }
  std::size_t block_size = get_size_in_bytes();
  return block_size > padding_bytes ? block_size - padding_bytes : 0;
}

rsa_key rsa_key::derive_public_key() const {
  assert(!is_empty());
  rsa_key res{};
  res.impl_.reset(
      RSAPublicKey_dup(rsa_key_accessor::get_impl_const_casted(*this)));
  if (res.is_empty())
    core_error::raise_with_error_string("cannot derive public RSA key");

  return res;
}

/*static*/
rsa_key rsa_key::generate(std::uint32_t bits,
                          const big_number &exponent /* = default_exponent */) {
  auto res = rsa_key{};
  res.impl_.reset(RSA_new());
  if (res.is_empty())
    core_error::raise_with_error_string("cannot create RSA key");

  if (RSA_generate_key_ex(
          rsa_key_accessor::get_impl(res), static_cast<int>(bits),
          big_number_accessor::get_impl_const_casted(exponent), nullptr) == 0)
    core_error::raise_with_error_string("cannot generate RSA key");
  return res;
}

/*static*/
std::string rsa_key::export_private_pem(const rsa_key &key) {
  assert(!key.is_empty());

  if (!key.is_private())
    throw core_error("RSA key does not have private components");

  auto sink = bio{};
  const int r =
      PEM_write_bio_RSAPrivateKey(bio_accessor::get_impl(sink),
                                  rsa_key_accessor::get_impl_const_casted(key),
                                  nullptr, nullptr, 0, nullptr, nullptr);
  if (r == 0)
    core_error::raise_with_error_string(
        "cannot export RSA key to PEM PRIVATE KEY");

  return sink.str();
}

/*static*/
std::string rsa_key::export_public_pem(const rsa_key &key) {
  assert(!key.is_empty());

  auto sink = bio{};
  const int r = PEM_write_bio_RSAPublicKey(bio_accessor::get_impl(sink),
                                           rsa_key_accessor::get_impl(key));
  if (r == 0)
    core_error::raise_with_error_string(
        "cannot export RSA key to PEM PUBLIC KEY");

  return sink.str();
}

/*static*/
rsa_key rsa_key::import_private_pem(const std::string &pem) {
  auto source = bio{pem};
  rsa_key res{};
  res.impl_.reset(PEM_read_bio_RSAPrivateKey(bio_accessor::get_impl(source),
                                             nullptr, nullptr, nullptr));
  if (res.is_empty())
    core_error::raise_with_error_string(
        "cannot import RSA key from PEM PRIVATE KEY");

  return res;
}

/*static*/
rsa_key rsa_key::import_public_pem(const std::string &pem) {
  auto source = bio{pem};
  rsa_key res{};
  res.impl_.reset(PEM_read_bio_RSAPublicKey(bio_accessor::get_impl(source),
                                            nullptr, nullptr, nullptr));
  if (res.is_empty())
    core_error::raise_with_error_string(
        "cannot import RSA key from PEM PUBLIC KEY");

  return res;
}

inline int rsa_padding_to_native_padding(rsa_padding padding) noexcept {
  switch (padding) {
    case rsa_padding::no:
      return RSA_NO_PADDING;
    case rsa_padding::pkcs1:
      return RSA_PKCS1_PADDING;
    default:
      assert(false);
  }
}

std::string encrypt_with_rsa_public_key(const std::string &input,
                                        const rsa_key &key,
                                        rsa_padding padding) {
  assert(!key.is_empty());

  if (input.size() > key.get_max_block_size_in_bytes(padding))
    throw core_error(
        "encryption block size is too long for the specified padding and RSA "
        "key");

  // TODO: use c++17 non-const std::string::data() member here
  using buffer_type = std::vector<unsigned char>;
  buffer_type res(key.get_size_in_bytes());

  auto enc_status = RSA_public_encrypt(
      input.size(), reinterpret_cast<const unsigned char *>(input.c_str()),
      res.data(), rsa_key_accessor::get_impl_const_casted(key),
      rsa_padding_to_native_padding(padding));
  if (enc_status == -1)
    core_error::raise_with_error_string(
        "cannot encrypt data block with the specified public RSA key");

  return {reinterpret_cast<char *>(res.data()), res.size()};
}

std::string encrypt_with_rsa_private_key(const std::string &input,
                                         const rsa_key &key,
                                         rsa_padding padding) {
  assert(!key.is_empty());

  if (!key.is_private())
    throw core_error("RSA key does not have private components");

  if (input.size() > key.get_max_block_size_in_bytes(padding))
    throw core_error(
        "encryption block size is too long for the specified padding and RSA "
        "key");

  // TODO: use c++17 non-const std::string::data() member here
  using buffer_type = std::vector<unsigned char>;
  buffer_type res(key.get_size_in_bytes());

  auto enc_status = RSA_private_encrypt(
      input.size(), reinterpret_cast<const unsigned char *>(input.c_str()),
      res.data(), rsa_key_accessor::get_impl_const_casted(key),
      rsa_padding_to_native_padding(padding));
  if (enc_status == -1)
    core_error::raise_with_error_string(
        "cannot encrypt data block with the specified private RSA key");

  return {reinterpret_cast<char *>(res.data()), res.size()};
}

std::string decrypt_with_rsa_public_key(const std::string &input,
                                        const rsa_key &key,
                                        rsa_padding padding) {
  assert(!key.is_empty());

  if (input.size() != key.get_size_in_bytes())
    core_error::raise_with_error_string(
        "decryption block size is not the same as RSA key length in bytes");

  // TODO: use c++17 non-const std::string::data() member here
  using buffer_type = std::vector<unsigned char>;
  buffer_type res(key.get_size_in_bytes());

  auto enc_status = RSA_public_decrypt(
      input.size(), reinterpret_cast<const unsigned char *>(input.c_str()),
      res.data(), rsa_key_accessor::get_impl_const_casted(key),
      rsa_padding_to_native_padding(padding));
  if (enc_status == -1)
    core_error::raise_with_error_string(
        "cannot encrypt data block with the specified public RSA key");

  return {reinterpret_cast<char *>(res.data()),
          static_cast<std::size_t>(enc_status)};
}

std::string decrypt_with_rsa_private_key(const std::string &input,
                                         const rsa_key &key,
                                         rsa_padding padding) {
  assert(!key.is_empty());

  if (!key.is_private())
    throw core_error("RSA key does not have private components");

  if (input.size() != key.get_size_in_bytes())
    throw core_error(
        "decryption block size is not the same as RSA key length in bytes");

  // TODO: use c++17 non-const std::string::data() member here
  using buffer_type = std::vector<unsigned char>;
  buffer_type res(key.get_size_in_bytes());

  auto enc_status = RSA_private_decrypt(
      input.size(), reinterpret_cast<const unsigned char *>(input.c_str()),
      res.data(), rsa_key_accessor::get_impl_const_casted(key),
      rsa_padding_to_native_padding(padding));
  if (enc_status == -1)
    core_error::raise_with_error_string(
        "cannot encrypt data block with the specified private RSA key");

  return {reinterpret_cast<char *>(res.data()),
          static_cast<std::size_t>(enc_status)};
}

std::ostream &operator<<(std::ostream &os, const rsa_key &obj) {
  return os << (obj.is_private() ? rsa_key::export_private_pem(obj)
                                 : rsa_key::export_public_pem(obj));
}

}  // namespace opensslpp
