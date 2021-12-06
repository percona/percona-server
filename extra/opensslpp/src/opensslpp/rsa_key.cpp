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

#include <openssl/pem.h>
#include <openssl/rsa.h>

#include <opensslpp/rsa_key.hpp>

#include <opensslpp/big_number.hpp>
#include <opensslpp/core_error.hpp>
#include <opensslpp/operation_cancelled_error.hpp>
#include <opensslpp/rsa_padding.hpp>

#include "opensslpp/big_number_accessor.hpp"
#include "opensslpp/bio.hpp"
#include "opensslpp/bio_accessor.hpp"
#include "opensslpp/key_generation_cancellation_context.hpp"
#include "opensslpp/key_generation_cancellation_context_accessor.hpp"
#include "opensslpp/rsa_key_accessor.hpp"

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
    throw core_error{"cannot duplicate RSA key"};
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
  const auto *rsa_raw = rsa_key_accessor::get_impl(*this);
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  p = rsa_raw->p;
  q = rsa_raw->q;
#else
  RSA_get0_factors(rsa_raw, &p, &q);
#endif
  return p != nullptr && q != nullptr;
}

std::size_t rsa_key::get_size_in_bits() const noexcept {
  assert(!is_empty());
  const auto *rsa_raw = rsa_key_accessor::get_impl(*this);
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  return BN_num_bits(rsa_raw->n);
#else
  return RSA_bits(rsa_raw);
#endif
}

std::size_t rsa_key::get_size_in_bytes() const noexcept {
  assert(!is_empty());
  return RSA_size(rsa_key_accessor::get_impl(*this));
}

std::size_t rsa_key::get_max_block_size_in_bytes(
    rsa_padding padding) const noexcept {
  assert(!is_empty());
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
  rsa_key_accessor::set_impl(
      res, RSAPublicKey_dup(rsa_key_accessor::get_impl_const_casted(*this)));
  if (res.is_empty()) throw core_error{"cannot derive public RSA key"};

  return res;
}

/*static*/
rsa_key rsa_key::generate(
    std::uint32_t bits, const big_number &exponent /* = default_exponent */,
    const key_generation_cancellation_callback
        &cancellation_callback /* = key_generation_cancellation_callback{} */) {
  auto res = rsa_key{};
  rsa_key_accessor::set_impl(res, RSA_new());
  if (res.is_empty()) throw core_error{"cannot create RSA key"};

  key_generation_cancellation_context cancellation_ctx{cancellation_callback};
  auto generation_status = RSA_generate_key_ex(
      rsa_key_accessor::get_impl(res), static_cast<int>(bits),
      big_number_accessor::get_impl_const_casted(exponent),
      key_generation_cancellation_context_accessor::get_impl(cancellation_ctx));
  if (cancellation_ctx.is_cancelled())
    throw operation_cancelled_error{"RSA key generation cancelled"};

  if (generation_status == 0)
    core_error::raise_with_error_string("cannot generate RSA key");
  return res;
}

/*static*/
std::string rsa_key::export_private_pem(const rsa_key &key) {
  assert(!key.is_empty());

  if (!key.is_private())
    throw core_error{"RSA key does not have private components"};

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
  const int r =
      PEM_write_bio_RSA_PUBKEY(bio_accessor::get_impl(sink),
                               rsa_key_accessor::get_impl_const_casted(key));
  if (r == 0)
    core_error::raise_with_error_string(
        "cannot export RSA key to PEM PUBLIC KEY");

  return sink.str();
}

/*static*/
rsa_key rsa_key::import_private_pem(const std::string &pem) {
  auto source = bio{pem};
  rsa_key res{};
  rsa_key_accessor::set_impl(
      res, PEM_read_bio_RSAPrivateKey(bio_accessor::get_impl(source), nullptr,
                                      nullptr, nullptr));
  if (res.is_empty())
    core_error::raise_with_error_string(
        "cannot import RSA key from PEM PRIVATE KEY");

  return res;
}

/*static*/
rsa_key rsa_key::import_public_pem(const std::string &pem) {
  auto source = bio{pem};
  rsa_key res{};
  rsa_key_accessor::set_impl(
      res, PEM_read_bio_RSA_PUBKEY(bio_accessor::get_impl(source), nullptr,
                                   nullptr, nullptr));
  if (res.is_empty())
    core_error::raise_with_error_string(
        "cannot import RSA key from PEM PUBLIC KEY");

  return res;
}

std::ostream &operator<<(std::ostream &os, const rsa_key &obj) {
  assert(!obj.is_empty());
  return os << (obj.is_private() ? rsa_key::export_private_pem(obj)
                                 : rsa_key::export_public_pem(obj));
}

}  // namespace opensslpp
