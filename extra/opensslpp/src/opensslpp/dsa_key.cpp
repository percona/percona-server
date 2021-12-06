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

#include <openssl/dsa.h>
#include <openssl/pem.h>

#include <opensslpp/dsa_key.hpp>

#include <opensslpp/big_number.hpp>
#include <opensslpp/core_error.hpp>
#include <opensslpp/operation_cancelled_error.hpp>

#include "opensslpp/big_number_accessor.hpp"
#include "opensslpp/bio.hpp"
#include "opensslpp/bio_accessor.hpp"
#include "opensslpp/dsa_key_accessor.hpp"
#include "opensslpp/key_generation_cancellation_context.hpp"
#include "opensslpp/key_generation_cancellation_context_accessor.hpp"

namespace opensslpp {

void dsa_key::dsa_deleter::operator()(void *dsa) const noexcept {
  if (dsa != nullptr) DSA_free(static_cast<DSA *>(dsa));
}

dsa_key::dsa_key(const dsa_key &obj)
    : impl_{obj.is_empty()
                ? nullptr
                : DSAparams_dup(dsa_key_accessor::get_impl_const_casted(obj))} {
  if (!obj.is_empty()) {
    if (is_empty()) throw core_error{"cannot duplicate DSA key"};

    auto public_component = obj.get_public_component();
    auto private_component = obj.get_private_component();

    auto *dsa_raw = dsa_key_accessor::get_impl(*this);
    int set_result;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    dsa_raw->pub_key = big_number_accessor::get_impl(public_component);
    dsa_raw->priv_key = big_number_accessor::get_impl(private_component);
    set_result = 1;
#else
    set_result =
        DSA_set0_key(dsa_raw, big_number_accessor::get_impl(public_component),
                     big_number_accessor::get_impl(private_component));
#endif
    if (set_result == 0)
      throw core_error{
          "cannot set private/public components when duplicating DSA key"};
    big_number_accessor::release(public_component);
    big_number_accessor::release(private_component);
  }
}

dsa_key &dsa_key::operator=(const dsa_key &obj) {
  auto tmp = dsa_key{obj};
  swap(tmp);
  return *this;
}

void dsa_key::swap(dsa_key &obj) noexcept { impl_.swap(obj.impl_); }

bool dsa_key::has_public_component() const noexcept {
  assert(!is_empty());
  const auto *dsa_raw = dsa_key_accessor::get_impl(*this);
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  return dsa_raw->pub_key != nullptr;
#else
  return DSA_get0_pub_key(dsa_raw) != nullptr;
#endif
}

bool dsa_key::has_private_component() const noexcept {
  assert(!is_empty());
  const auto *dsa_raw = dsa_key_accessor::get_impl(*this);
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  return dsa_raw->priv_key != nullptr;
#else
  return DSA_get0_priv_key(dsa_raw) != nullptr;
#endif
}

std::size_t dsa_key::get_size_in_bits() const noexcept {
  assert(!is_empty());
  const auto *dsa_raw = dsa_key_accessor::get_impl(*this);
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  return BN_num_bits(dsa_raw->p);
#else
  return DSA_bits(dsa_raw);
#endif
}

std::size_t dsa_key::get_size_in_bytes() const noexcept {
  assert(!is_empty());
  return DSA_size(dsa_key_accessor::get_impl(*this));
}

std::size_t dsa_key::get_security_size_in_bits() const noexcept {
  assert(!is_empty());
  const auto *dsa_raw = dsa_key_accessor::get_impl(*this);
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  int l = BN_num_bits(dsa_raw->p);
  int n = BN_num_bits(dsa_raw->q);

  int secbits, bits;
  if (l >= 15360)
    secbits = 256;
  else if (l >= 7680)
    secbits = 192;
  else if (l >= 3072)
    secbits = 128;
  else if (l >= 2048)
    secbits = 112;
  else if (l >= 1024)
    secbits = 80;
  else
    return 0;
  if (n == -1) return secbits;
  bits = n / 2;
  if (bits < 80) return 0;
  return bits >= secbits ? secbits : bits;
#else
  return DSA_security_bits(dsa_raw);
#endif
}

big_number dsa_key::get_public_component() const {
  assert(!is_empty());
  const auto *dsa_raw = dsa_key_accessor::get_impl(*this);
  auto public_component_raw =
#if OPENSSL_VERSION_NUMBER < 0x10100000L
      dsa_raw->pub_key;
#else
      DSA_get0_pub_key(dsa_raw);
#endif
  if (public_component_raw == nullptr) return {};
  big_number res;
  auto public_component_raw_copy = BN_dup(public_component_raw);
  if (public_component_raw_copy == nullptr)
    throw core_error{"cannot extract public component from DSA key"};
  big_number_accessor::set_impl(res, public_component_raw_copy);
  return res;
}

big_number dsa_key::get_private_component() const {
  assert(!is_empty());
  const auto *dsa_raw = dsa_key_accessor::get_impl(*this);
  auto private_component_raw =
#if OPENSSL_VERSION_NUMBER < 0x10100000L
      dsa_raw->priv_key;
#else
      DSA_get0_priv_key(dsa_raw);
#endif
  if (private_component_raw == nullptr) return {};
  big_number res;
  auto private_component_raw_copy = BN_dup(private_component_raw);
  if (private_component_raw_copy == nullptr)
    throw core_error{"cannot extract private component from DSA key"};
  big_number_accessor::set_impl(res, private_component_raw_copy);
  return res;
}

void dsa_key::promote_to_key() {
  assert(!is_empty());
  if (has_public_component() || has_private_component())
    throw core_error{
        "DSA key has already ben generated for these DSA parameters"};
  if (DSA_generate_key(dsa_key_accessor::get_impl(*this)) == 0)
    core_error::raise_with_error_string("cannot generate DSA key");
}

dsa_key dsa_key::derive_public_key() const {
  assert(!is_empty());
  auto public_component = get_public_component();
  if (public_component.is_empty())
    throw core_error{
        "cannot derive public key from DSA without public component"};

  dsa_key res{};
  dsa_key_accessor::set_impl(
      res, DSAparams_dup(dsa_key_accessor::get_impl_const_casted(*this)));
  if (res.is_empty()) throw core_error{"cannot derive public key from DSA key"};

  auto *dsa_raw = dsa_key_accessor::get_impl(res);
  int set_result;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  dsa_raw->pub_key = big_number_accessor::get_impl(public_component);
  set_result = 1;
#else
  set_result = DSA_set0_key(
      dsa_raw, big_number_accessor::get_impl(public_component), nullptr);
#endif
  if (set_result == 0)
    throw core_error{"cannot set public component when deriving from DSA key"};

  big_number_accessor::release(public_component);

  return res;
}

/*static*/
dsa_key dsa_key::generate_parameters(
    std::uint32_t bits,
    const key_generation_cancellation_callback
        &cancellation_callback /* = key_generation_cancellation_callback{} */) {
  auto res = dsa_key{};
  dsa_key_accessor::set_impl(res, DSA_new());
  if (res.is_empty()) throw core_error{"cannot create DSA key"};

  key_generation_cancellation_context cancellation_ctx{cancellation_callback};
  auto generation_status = DSA_generate_parameters_ex(
      dsa_key_accessor::get_impl(res), static_cast<int>(bits), nullptr, 0,
      nullptr, nullptr,
      key_generation_cancellation_context_accessor::get_impl(cancellation_ctx));
  if (cancellation_ctx.is_cancelled())
    throw operation_cancelled_error{"DSA parameters generation cancelled"};

  if (generation_status == 0)
    core_error::raise_with_error_string("cannot generate DSA parameters");

  return res;
}

/*static*/
std::string dsa_key::export_parameters_pem(const dsa_key &key) {
  assert(!key.is_empty());

  auto sink = bio{};
  const int r = PEM_write_bio_DSAparams(bio_accessor::get_impl(sink),
                                        dsa_key_accessor::get_impl(key));
  if (r == 0)
    core_error::raise_with_error_string(
        "cannot export DSA key to PEM PARAMETERS");

  return sink.str();
}

/*static*/
std::string dsa_key::export_private_pem(const dsa_key &key) {
  assert(!key.is_empty());

  if (!key.has_private_component())
    throw core_error{"DSA key does not have private component"};

  auto sink = bio{};
  const int r =
      PEM_write_bio_DSAPrivateKey(bio_accessor::get_impl(sink),
                                  dsa_key_accessor::get_impl_const_casted(key),
                                  nullptr, nullptr, 0, nullptr, nullptr);
  if (r == 0)
    core_error::raise_with_error_string(
        "cannot export DSA key to PEM PRIVATE KEY");

  return sink.str();
}

/*static*/
std::string dsa_key::export_public_pem(const dsa_key &key) {
  assert(!key.is_empty());

  if (!key.has_public_component())
    throw core_error{"DSA key does not have public component"};

  auto sink = bio{};
  const int r =
      PEM_write_bio_DSA_PUBKEY(bio_accessor::get_impl(sink),
                               dsa_key_accessor::get_impl_const_casted(key));
  if (r == 0)
    core_error::raise_with_error_string(
        "cannot export DSA key to PEM PUBLIC KEY");

  return sink.str();
}

/*static*/
dsa_key dsa_key::import_parameters_pem(const std::string &pem) {
  auto source = bio{pem};
  dsa_key res{};
  dsa_key_accessor::set_impl(
      res, PEM_read_bio_DSAparams(bio_accessor::get_impl(source), nullptr,
                                  nullptr, nullptr));
  if (res.is_empty())
    core_error::raise_with_error_string(
        "cannot import DSA key from PEM PARAMETERS");

  return res;
}

/*static*/
dsa_key dsa_key::import_private_pem(const std::string &pem) {
  auto source = bio{pem};
  dsa_key res{};
  dsa_key_accessor::set_impl(
      res, PEM_read_bio_DSAPrivateKey(bio_accessor::get_impl(source), nullptr,
                                      nullptr, nullptr));
  if (res.is_empty())
    core_error::raise_with_error_string(
        "cannot import DSA key from PEM PRIVATE KEY");

  return res;
}

/*static*/
dsa_key dsa_key::import_public_pem(const std::string &pem) {
  auto source = bio{pem};
  dsa_key res{};
  dsa_key_accessor::set_impl(
      res, PEM_read_bio_DSA_PUBKEY(bio_accessor::get_impl(source), nullptr,
                                   nullptr, nullptr));
  if (res.is_empty())
    core_error::raise_with_error_string(
        "cannot import DSA key from PEM PUBLIC KEY");

  return res;
}

std::ostream &operator<<(std::ostream &os, const dsa_key &obj) {
  assert(!obj.is_empty());
  if (obj.has_private_component())
    return os << dsa_key::export_private_pem(obj);
  else if (obj.has_public_component())
    return os << dsa_key::export_public_pem(obj);
  else
    return os << dsa_key::export_parameters_pem(obj);
}

}  // namespace opensslpp
