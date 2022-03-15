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

#include <openssl/dh.h>
#include <openssl/pem.h>

#include <opensslpp/dh_key.hpp>

#include <opensslpp/big_number.hpp>
#include <opensslpp/core_error.hpp>

#include "opensslpp/big_number_accessor.hpp"
#include "opensslpp/bio.hpp"
#include "opensslpp/bio_accessor.hpp"
#include "opensslpp/dh_key_accessor.hpp"

namespace opensslpp {

/*static*/ const std::uintmax_t dh_key::generator_2{DH_GENERATOR_2};
/*static*/ const std::uintmax_t dh_key::generator_5{DH_GENERATOR_5};
/*static*/ const std::uintmax_t dh_key::default_generator{dh_key::generator_5};

void dh_key::dh_deleter::operator()(void *dhp) const noexcept {
  if (dhp != nullptr) DH_free(static_cast<DH *>(dhp));
}

dh_key::dh_key(const dh_key &obj)
    : impl_{obj.is_empty()
                ? nullptr
                : DHparams_dup(dh_key_accessor::get_impl_const_casted(obj))} {
  if (!obj.is_empty()) {
    if (is_empty())
      core_error::raise_with_error_string("cannot duplicate DH parameters");
    auto public_component = obj.get_public_component();
    auto private_component = obj.get_private_component();
    if (DH_set0_key(dh_key_accessor::get_impl(*this),
                    big_number_accessor::get_impl(public_component),
                    big_number_accessor::get_impl(private_component)) == 0)
      throw core_error{
          "cannot set private/public components when duplicating DH key"};
    big_number_accessor::release(public_component);
    big_number_accessor::release(private_component);
  }
}

dh_key &dh_key::operator=(const dh_key &obj) {
  auto tmp = dh_key{obj};
  swap(tmp);
  return *this;
}

void dh_key::swap(dh_key &obj) noexcept { impl_.swap(obj.impl_); }

bool dh_key::has_private_component() const noexcept {
  return DH_get0_priv_key(dh_key_accessor::get_impl(*this)) != nullptr;
}

bool dh_key::has_public_component() const noexcept {
  return DH_get0_pub_key(dh_key_accessor::get_impl(*this)) != nullptr;
}

std::size_t dh_key::get_size_in_bits() const noexcept {
  assert(!is_empty());
  return DH_bits(dh_key_accessor::get_impl(*this));
}

std::size_t dh_key::get_size_in_bytes() const noexcept {
  assert(!is_empty());
  return DH_size(dh_key_accessor::get_impl(*this));
}

std::size_t dh_key::get_security_size_in_bits() const noexcept {
  assert(!is_empty());
  return DH_security_bits(dh_key_accessor::get_impl(*this));
}

big_number dh_key::get_public_component() const {
  assert(!is_empty());
  auto public_component_raw = DH_get0_pub_key(dh_key_accessor::get_impl(*this));
  if (public_component_raw == nullptr) return {};
  big_number res;
  auto public_component_raw_copy = BN_dup(public_component_raw);
  if (public_component_raw_copy == nullptr)
    throw core_error{"cannot extract public component from DH key"};
  big_number_accessor::set_impl(res, public_component_raw_copy);
  return res;
}

big_number dh_key::get_private_component() const {
  assert(!is_empty());
  auto private_component_raw =
      DH_get0_priv_key(dh_key_accessor::get_impl(*this));
  if (private_component_raw == nullptr) return {};
  big_number res;
  auto private_component_raw_copy = BN_dup(private_component_raw);
  if (private_component_raw_copy == nullptr)
    throw core_error{"cannot extract private component from DH key"};
  big_number_accessor::set_impl(res, private_component_raw_copy);
  return res;
}

void dh_key::promote_to_key() {
  assert(!is_empty());
  if (has_public_component() || has_private_component())
    throw core_error{
        "DH key has already ben generated for these DH parameters"};
  if (DH_generate_key(dh_key_accessor::get_impl(*this)) == 0)
    core_error::raise_with_error_string("cannot generate DH key");
}

dh_key dh_key::derive_public_key() const {
  assert(!is_empty());
  auto public_component = get_public_component();
  if (public_component.is_empty())
    throw core_error{
        "cannot derive public key from DH without public component"};

  dh_key res{};
  dh_key_accessor::set_impl(
      res, DHparams_dup(dh_key_accessor::get_impl_const_casted(*this)));
  if (res.is_empty())
    core_error::raise_with_error_string("cannot derive public key from DH key");

  if (DH_set0_key(dh_key_accessor::get_impl(res),
                  big_number_accessor::get_impl(public_component),
                  nullptr) == 0)
    throw core_error{"cannot set public component when deriving from DH key"};

  big_number_accessor::release(public_component);

  return res;
}

/*static*/
dh_key dh_key::generate_parameters(
    std::uint32_t bits, std::uintmax_t generator /* = default_generator */) {
  auto res = dh_key{};
  dh_key_accessor::set_impl(res, DH_new());
  if (res.is_empty())
    core_error::raise_with_error_string("cannot create DH parameters");

  if (DH_generate_parameters_ex(dh_key_accessor::get_impl(res),
                                static_cast<int>(bits),
                                static_cast<int>(generator), nullptr) == 0)
    core_error::raise_with_error_string("cannot generate DH parameters");

  return res;
}

/*static*/
std::string dh_key::export_parameters_pem(const dh_key &key) {
  assert(!key.is_empty());

  auto sink = bio{};
  const int r = PEM_write_bio_DHparams(bio_accessor::get_impl(sink),
                                       dh_key_accessor::get_impl(key));
  if (r == 0)
    core_error::raise_with_error_string(
        "cannot export DH key to PEM PARAMETERS");

  return sink.str();
}

struct evp_pkey_deleter {
  void operator()(EVP_PKEY *key) const noexcept {
    if (key != nullptr) EVP_PKEY_free(key);
  }
};
using evp_pkey_capsule = std::unique_ptr<EVP_PKEY, evp_pkey_deleter>;

/*static*/
std::string dh_key::export_private_pem(const dh_key &key) {
  evp_pkey_capsule pkey{EVP_PKEY_new()};
  if (EVP_PKEY_set1_DH(pkey.get(),
                       dh_key_accessor::get_impl_const_casted(key)) != 1)
    core_error::raise_with_error_string(
        "cannot assign PRIVATE DH key to EVP PKEY");

  auto sink = bio{};
  const int r =
      PEM_write_bio_PrivateKey(bio_accessor::get_impl(sink), pkey.get(),
                               nullptr, nullptr, 0, nullptr, nullptr);
  if (r == 0)
    core_error::raise_with_error_string(
        "cannot export DH key to PEM PRIVATE KEY");

  return sink.str();
}

/*static*/
std::string dh_key::export_public_pem(const dh_key &key) {
  evp_pkey_capsule pkey{EVP_PKEY_new()};
  if (EVP_PKEY_set1_DH(pkey.get(),
                       dh_key_accessor::get_impl_const_casted(key)) != 1)
    core_error::raise_with_error_string(
        "cannot assign PUBLIC DH key to EVP PKEY");

  auto sink = bio{};
  const int r = PEM_write_bio_PUBKEY(bio_accessor::get_impl(sink), pkey.get());
  if (r == 0)
    core_error::raise_with_error_string(
        "cannot export DH key to PEM PUBLIC KEY");

  return sink.str();
}

/*static*/
dh_key dh_key::import_parameters_pem(const std::string &pem) {
  auto source = bio{pem};
  dh_key res{};
  dh_key_accessor::set_impl(
      res, PEM_read_bio_DHparams(bio_accessor::get_impl(source), nullptr,
                                 nullptr, nullptr));
  if (res.is_empty())
    core_error::raise_with_error_string(
        "cannot import DH key from PEM PARAMETERS");

  return res;
}

/*static*/
dh_key dh_key::import_private_pem(const std::string &pem) {
  auto source = bio{pem};
  evp_pkey_capsule pkey{PEM_read_bio_PrivateKey(bio_accessor::get_impl(source),
                                                nullptr, nullptr, nullptr)};
  if (!pkey)
    core_error::raise_with_error_string(
        "cannot import DH key from PEM PRIVATE KEY");

  dh_key res{};
  dh_key_accessor::set_impl(res, EVP_PKEY_get1_DH(pkey.get()));
  if (res.is_empty())
    core_error::raise_with_error_string(
        "cannot extract PRIVATE DH key from EVP KEY");

  return res;
}

/*static*/
dh_key dh_key::import_public_pem(const std::string &pem) {
  auto source = bio{pem};
  evp_pkey_capsule pkey{PEM_read_bio_PUBKEY(bio_accessor::get_impl(source),
                                            nullptr, nullptr, nullptr)};
  if (!pkey)
    core_error::raise_with_error_string(
        "cannot import DH key from PEM PUBLIC KEY");

  dh_key res{};
  dh_key_accessor::set_impl(res, EVP_PKEY_get1_DH(pkey.get()));
  if (res.is_empty())
    core_error::raise_with_error_string(
        "cannot extract PUBLIC DH key from EVP KEY");

  return res;
}

std::ostream &operator<<(std::ostream &os, const dh_key &obj) {
  assert(!obj.is_empty());
  if (obj.has_private_component())
    return os << dh_key::export_private_pem(obj);
  else if (obj.has_public_component())
    return os << dh_key::export_public_pem(obj);
  else
    return os << dh_key::export_parameters_pem(obj);
}

}  // namespace opensslpp
