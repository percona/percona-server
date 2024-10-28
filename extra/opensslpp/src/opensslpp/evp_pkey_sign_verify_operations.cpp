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

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>

#include <opensslpp/evp_pkey_sign_verify_operations.hpp>

#include <opensslpp/core_error.hpp>
#include <opensslpp/evp_pkey.hpp>
#include <opensslpp/evp_pkey_signature_padding.hpp>

#include "opensslpp/evp_pkey_accessor.hpp"
#include "opensslpp/evp_pkey_signature_padding_conversions.hpp"

namespace opensslpp {

enum class sign_verify_operation_type { sign, verify };

class evp_pkey_sign_verify_ctx {
 public:
  evp_pkey_sign_verify_ctx(sign_verify_operation_type operation,
                           const evp_pkey &key, const std::string &digest_type)
      : impl_{EVP_PKEY_CTX_new(evp_pkey_accessor::get_impl_const_casted(key),
                               nullptr)} {
    if (!impl_) {
      throw core_error{"cannot create EVP_PKEY context for sign/verify"};
    }

    if (operation == sign_verify_operation_type::sign) {
      if (EVP_PKEY_sign_init(impl_.get()) <= 0) {
        throw core_error{
            "cannot initialize EVP_PKEY context for sign operation"};
      }
    } else if (operation == sign_verify_operation_type::verify) {
      if (EVP_PKEY_verify_init(impl_.get()) <= 0) {
        throw core_error{
            "cannot initialize EVP_PKEY context for verify operation"};
      }
    } else {
      assert(false);
    }

    auto md = EVP_get_digestbyname(digest_type.c_str());
    if (md == nullptr) {
      throw core_error{"unknown digest name"};
    }

    if (EVP_PKEY_CTX_set_signature_md(impl_.get(), md) <= 0) {
      throw core_error{"cannot configure digest in EVP_PKEY context"};
    }
  }

  void set_rsa_signature_padding_mode(int padding_mode) {
    if (EVP_PKEY_CTX_set_rsa_padding(impl_.get(), padding_mode) <= 0) {
      throw core_error{"cannot set RSA padding mode in EVP_PKEY context"};
    }
  }

  std::string sign(std::string_view digest_data) {
    std::string res;
    std::size_t signature_length{0};

    if (EVP_PKEY_sign(
            impl_.get(), nullptr, &signature_length,
            reinterpret_cast<const unsigned char *>(std::data(digest_data)),
            std::size(digest_data)) <= 0) {
      throw core_error{"cannot determine EVP_PKEY signature length"};
    }
    res.resize(signature_length, '\0');

    if (EVP_PKEY_sign(
            impl_.get(), reinterpret_cast<unsigned char *>(std::data(res)),
            &signature_length,
            reinterpret_cast<const unsigned char *>(std::data(digest_data)),
            std::size(digest_data)) <= 0) {
      core_error::raise_with_error_string(
          "cannot sign message digest with the specified private EVP_PKEY");
    }
    res.resize(signature_length);
    return res;
  }

  bool verify(std::string_view digest_data, std::string_view signature_data) {
    auto verification_status{EVP_PKEY_verify(
        impl_.get(),
        reinterpret_cast<const unsigned char *>(std::data(signature_data)),
        std::size(signature_data),
        reinterpret_cast<const unsigned char *>(std::data(digest_data)),
        std::size(digest_data))};
    if (verification_status < 0) {
      core_error::raise_with_error_string(
          "cannot verify message digest with the specified public EVP_PKEY");
    }
    return (verification_status != 0);
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
};

std::string sign_with_private_evp_pkey(const std::string &digest_type,
                                       std::string_view digest_data,
                                       const evp_pkey &key,
                                       evp_pkey_signature_padding padding) {
  assert(!key.is_empty());

  if (!key.is_private())
    throw core_error{"EVP_PKEY key does not have private components"};

  evp_pkey_sign_verify_ctx ctx{sign_verify_operation_type::sign, key,
                               digest_type};
  ctx.set_rsa_signature_padding_mode(
      evp_pkey_signature_padding_to_native_padding(padding));

  return ctx.sign(digest_data);
}

bool verify_with_public_evp_pkey(const std::string &digest_type,
                                 std::string_view digest_data,
                                 std::string_view signature_data,
                                 const evp_pkey &key,
                                 evp_pkey_signature_padding padding) {
  assert(!key.is_empty());

  evp_pkey_sign_verify_ctx ctx{sign_verify_operation_type::verify, key,
                               digest_type};
  ctx.set_rsa_signature_padding_mode(
      evp_pkey_signature_padding_to_native_padding(padding));

  return ctx.verify(digest_data, signature_data);
}

}  // namespace opensslpp
