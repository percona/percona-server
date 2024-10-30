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

#include <openssl/evp.h>
#include <openssl/pem.h>

#include <opensslpp/evp_pkey.hpp>

#include <opensslpp/core_error.hpp>
#include <opensslpp/operation_cancelled_error.hpp>
#include <opensslpp/rsa_key.hpp>

#include "opensslpp/bio.hpp"
#include "opensslpp/bio_accessor.hpp"
#include "opensslpp/evp_pkey_accessor.hpp"
#include "opensslpp/evp_pkey_algorithm_conversions.hpp"
#include "opensslpp/rsa_key_accessor.hpp"

namespace {

void duplicate_evp_pkey(opensslpp::evp_pkey &dest,
                        const opensslpp::evp_pkey &source, bool public_only) {
  assert(!source.is_empty());

  auto *casted_impl{
      opensslpp::evp_pkey_accessor::get_impl_const_casted(source)};
  unsigned char *der_raw{nullptr};
  const auto serializer{public_only ? &i2d_PublicKey : &i2d_PrivateKey};
  auto der_length{(*serializer)(casted_impl, &der_raw)};
  if (der_length < 0) {
    opensslpp::core_error::raise_with_error_string(
        "cannot serialize EVP_PKEY to DER format");
  }

  struct ossl_deleter {
    void operator()(void *ptr) const noexcept {
      if (ptr != nullptr) {
        OPENSSL_free(ptr);
      }
    }
  };
  using buffer_ptr = std::unique_ptr<unsigned char, ossl_deleter>;
  buffer_ptr der{der_raw};

  const unsigned char *der_ptr{der_raw};
  const auto deserializer{public_only ? &d2i_PublicKey : &d2i_PrivateKey};
  opensslpp::evp_pkey_accessor::set_impl(
      dest, (*deserializer)(EVP_PKEY_base_id(casted_impl), nullptr, &der_ptr,
                            der_length));
  if (dest.is_empty()) {
    opensslpp::core_error::raise_with_error_string(
        "cannot deserialize EVP_PKEY from DER format");
  }
}

}  // anonymous namespace

namespace opensslpp {

void evp_pkey::evp_pkey_deleter::operator()(void *evp_pkey) const noexcept {
  if (evp_pkey != nullptr) EVP_PKEY_free(static_cast<EVP_PKEY *>(evp_pkey));
}

evp_pkey::evp_pkey(const evp_pkey &obj) : impl_{} {
  if (!obj.is_empty()) {
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    // due to a bug in openssl interface, EVP_PKEY_dup() expects
    // non-const parameter while it does not do any modifications with the
    // object - it just performs duplication via ASN1_item_i2d/ASN1_item_d2i
    // conversions
    impl_.reset(EVP_PKEY_dup(evp_pkey_accessor::get_impl_const_casted(obj)));
    if (!impl_) {
      throw core_error{"cannot duplicate EVP_PKEY key"};
    }
#else
    duplicate_evp_pkey(*this, obj, !obj.is_private());
#endif
  }
}

evp_pkey &evp_pkey::operator=(const evp_pkey &obj) {
  auto tmp = evp_pkey{obj};
  swap(tmp);
  return *this;
}

void evp_pkey::swap(evp_pkey &obj) noexcept { impl_.swap(obj.impl_); }

evp_pkey_algorithm evp_pkey::get_algorithm() const noexcept {
  assert(!is_empty());
  auto native_algorithm{EVP_PKEY_base_id(evp_pkey_accessor::get_impl(*this))};
  return native_algorithm_to_evp_pkey_algorithm(native_algorithm);
}

bool evp_pkey::is_private() const noexcept {
  assert(!is_empty());

  // TODO: implement checks for other algorithms
  const auto *native_rsa{
      EVP_PKEY_get0_RSA(evp_pkey_accessor::get_impl_const_casted(*this))};
  assert(native_rsa != nullptr);
  rsa_key underlying_key;
  rsa_key_accessor::set_impl(underlying_key, const_cast<RSA *>(native_rsa));
  const auto res{underlying_key.is_private()};
  rsa_key_accessor::release(underlying_key);
  return res;
}

std::size_t evp_pkey::get_size_in_bits() const noexcept {
  assert(!is_empty());
  return EVP_PKEY_bits(evp_pkey_accessor::get_impl(*this));
}

std::size_t evp_pkey::get_size_in_bytes() const noexcept {
  assert(!is_empty());
  return EVP_PKEY_size(evp_pkey_accessor::get_impl(*this));
}

evp_pkey evp_pkey::derive_public_key() const {
  assert(!is_empty());

  evp_pkey res{};
  duplicate_evp_pkey(res, *this, true);

  return res;
}

class evp_pkey_keygen_ctx {
 public:
  evp_pkey_keygen_ctx(
      int id, const key_generation_cancellation_callback &cancellation_callback)
      : impl_{EVP_PKEY_CTX_new_id(id, nullptr)},
        cancellation_callback_{&cancellation_callback},
        cancelled_{false} {
    assert(id == EVP_PKEY_RSA);
    if (!impl_) {
      throw core_error{"cannot create EVP_PKEY context for key generation"};
    }

    if (EVP_PKEY_keygen_init(impl_.get()) <= 0) {
      throw core_error{"cannot initialize EVP_PKEY context for key generation"};
    }
  }

  evp_pkey generate(std::size_t bits) {
    // TODO: implement setting bit length for other algorithms
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(impl_.get(), bits) <= 0) {
      throw core_error{"cannot set EVP_PKEY context key generation parameters"};
    }

    EVP_PKEY_CTX_set_cb(impl_.get(),
                        &evp_pkey_keygen_ctx::generate_pkey_callback);
    EVP_PKEY_CTX_set_app_data(impl_.get(), this);
    cancelled_ = false;

    EVP_PKEY *pkey{nullptr};
    auto generation_status{EVP_PKEY_keygen(impl_.get(), &pkey)};

    if (cancelled_) {
      throw operation_cancelled_error{"EVP_PKEY key generation cancelled"};
    }

    if (generation_status <= 0) {
      core_error::raise_with_error_string("cannot generate EVP_PKEY");
    }

    evp_pkey res{};
    evp_pkey_accessor::set_impl(res, pkey);

    return res;
  }

 private:
  struct evp_pkey_ctx_deleter {
    void operator()(EVP_PKEY_CTX *ctx) const noexcept {
      if (ctx != nullptr) {
        EVP_PKEY_CTX_free(ctx);
      }
    }
  };
  using impl_ptr = std::unique_ptr<EVP_PKEY_CTX, evp_pkey_ctx_deleter>;
  impl_ptr impl_;
  const key_generation_cancellation_callback *cancellation_callback_;
  bool cancelled_;

  static int generate_pkey_callback(EVP_PKEY_CTX *ctx) {
    auto &parent{
        *static_cast<evp_pkey_keygen_ctx *>(EVP_PKEY_CTX_get_app_data(ctx))};
    try {
      parent.cancelled_ = (*parent.cancellation_callback_)();
    } catch (...) {
      parent.cancelled_ = true;
    }

    return parent.cancelled_ ? 0 : 1;
  }
};

/*static*/
evp_pkey evp_pkey::generate(
    evp_pkey_algorithm algorithm, std::uint32_t bits,
    const key_generation_cancellation_callback
        &cancellation_callback /* = key_generation_cancellation_callback{} */) {
  validate_if_algorithm_supported(algorithm);

  evp_pkey_keygen_ctx ctx{evp_pkey_algorithm_to_native_algorithm(algorithm),
                          cancellation_callback};
  return ctx.generate(bits);
}

/*static*/
std::string evp_pkey::export_private_pem(const evp_pkey &key) {
  assert(!key.is_empty());

  if (!key.is_private())
    throw core_error{"EVP_PKEY does not have private components"};

  auto sink = bio{};
  const int r =
      PEM_write_bio_PrivateKey(bio_accessor::get_impl(sink),
                               evp_pkey_accessor::get_impl_const_casted(key),
                               nullptr, nullptr, 0, nullptr, nullptr);

  if (r <= 0)
    core_error::raise_with_error_string(
        "cannot export EVP_PKEY key to PEM PRIVATE KEY");

  return std::string{sink.sv()};
}

/*static*/
std::string evp_pkey::export_public_pem(const evp_pkey &key) {
  assert(!key.is_empty());

  auto sink = bio{};
  const int r =
      PEM_write_bio_PUBKEY(bio_accessor::get_impl(sink),
                           evp_pkey_accessor::get_impl_const_casted(key));
  if (r == 0)
    core_error::raise_with_error_string(
        "cannot export EVP_PKEY key to PEM PUBLIC KEY");

  return std::string{sink.sv()};
}

/*static*/
evp_pkey evp_pkey::import_private_pem(std::string_view pem) {
  auto source = bio{pem};
  evp_pkey res{};
  evp_pkey_accessor::set_impl(
      res, PEM_read_bio_PrivateKey(bio_accessor::get_impl(source), nullptr,
                                   nullptr, nullptr));
  if (res.is_empty())
    core_error::raise_with_error_string(
        "cannot import EVP_PKEY from PEM PRIVATE KEY");

  validate_if_algorithm_supported(res.get_algorithm());

  return res;
}

/*static*/
evp_pkey evp_pkey::import_public_pem(std::string_view pem) {
  auto source = bio{pem};
  evp_pkey res{};
  evp_pkey_accessor::set_impl(
      res, PEM_read_bio_PUBKEY(bio_accessor::get_impl(source), nullptr, nullptr,
                               nullptr));
  if (res.is_empty())
    core_error::raise_with_error_string(
        "cannot import EVP_PKEY from PEM PUBLIC KEY");

  validate_if_algorithm_supported(res.get_algorithm());

  return res;
}

void evp_pkey::validate_if_algorithm_supported(evp_pkey_algorithm algorithm) {
  if (algorithm != evp_pkey_algorithm::rsa) {
    throw std::logic_error{
        "current implementation of EVP_PKEY wrapper does not support the "
        "specified algorithm"};
  }
}

std::ostream &operator<<(std::ostream &os, const evp_pkey &obj) {
  assert(!obj.is_empty());
  return os << (obj.is_private() ? evp_pkey::export_private_pem(obj)
                                 : evp_pkey::export_public_pem(obj));
}

}  // namespace opensslpp
