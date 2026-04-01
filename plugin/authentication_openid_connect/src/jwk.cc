/*
(C) 2026 Percona LLC and/or its affiliates

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/

#include "jwk.h"

#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/core_names.h>
#include <openssl/evp.h>
#include <openssl/param_build.h>
#include <openssl/params.h>
#include <openssl/pem.h>

template <typename T, T *(*alloc)(), void (*dealloc)(T *)>
class Raii {
 private:
  T *ptr;

 public:
  Raii() : ptr(alloc()) {
    if (ptr == nullptr) throw std::bad_alloc();
  }
  explicit Raii(T *ptr) : ptr(ptr) {}

  ~Raii() {
    if (ptr) dealloc(ptr);
  }

  T *get() const noexcept { return ptr; }
  T &operator*() const noexcept { return *ptr; }
  T *operator->() const noexcept { return ptr; }
};

std::vector<unsigned char> Jwk::base64url_decode(const std::string &input) {
  std::string s = input;
  const size_t pad = s.size() % 4;
  if (pad != 0) s.append(4 - pad, '=');
  std::ranges::replace(s, '-', '+');
  std::ranges::replace(s, '_', '/');

  std::vector<unsigned char> output(s.size());
  const int len = EVP_DecodeBlock(
      output.data(), reinterpret_cast<const unsigned char *>(s.data()),
      output.size());
  if (len < 0) throw std::runtime_error("Base64 decode failed");
  output.resize(len - (pad == 0 ? 0 : 4 - pad));
  return output;
}

OSSL_PARAM *Rsa_jwk::construct_param() {
  if (n.empty() || e.empty()) throw std::runtime_error("RSA requires n and e");

  const auto n_bytes = base64url_decode(n);
  const auto e_bytes = base64url_decode(e);
  const Raii<BIGNUM, nullptr, BN_free> bn_n(
      BN_bin2bn(n_bytes.data(), n_bytes.size(), nullptr));
  const Raii<BIGNUM, nullptr, BN_free> bn_e(
      BN_bin2bn(e_bytes.data(), e_bytes.size(), nullptr));

  // BN for RSA
  const Raii<OSSL_PARAM_BLD, OSSL_PARAM_BLD_new, OSSL_PARAM_BLD_free> param_bld;
  if ((OSSL_PARAM_BLD_push_BN(param_bld.get(), OSSL_PKEY_PARAM_RSA_N,
                              bn_n.get()) == 0) ||
      (OSSL_PARAM_BLD_push_BN(param_bld.get(), OSSL_PKEY_PARAM_RSA_E,
                              bn_e.get()) == 0))
    throw std::runtime_error("Failed to push BN params for RSA");

  return OSSL_PARAM_BLD_to_param(param_bld.get());
}

OSSL_PARAM *Ec_jwk::construct_param() {
  if (crv.empty() || x.empty() || y.empty())
    throw std::runtime_error("EC requires crv, x, y");

  if (crv != "P-256" && crv != "P-384" && crv != "P-521")
    throw std::runtime_error("Unsupported EC curve: " + crv);

  auto x_bytes = base64url_decode(x);
  auto y_bytes = base64url_decode(y);

  // Uncompressed public key point: 0x04 + X + Y
  std::vector<unsigned char> pub_key_octet = {0x04};
  pub_key_octet.insert(pub_key_octet.end(), x_bytes.begin(), x_bytes.end());
  pub_key_octet.insert(pub_key_octet.end(), y_bytes.begin(), y_bytes.end());

  const Raii<OSSL_PARAM_BLD, OSSL_PARAM_BLD_new, OSSL_PARAM_BLD_free> param_bld;
  if ((OSSL_PARAM_BLD_push_utf8_string(param_bld.get(),
                                       OSSL_PKEY_PARAM_GROUP_NAME,
                                       crv.c_str(), 0) == 0) ||
      (OSSL_PARAM_BLD_push_octet_string(
           param_bld.get(), OSSL_PKEY_PARAM_PUB_KEY, pub_key_octet.data(),
           pub_key_octet.size()) == 0))
    throw std::runtime_error("Failed to build EC params");

  return OSSL_PARAM_BLD_to_param(param_bld.get());
}

inline EVP_PKEY *pkey_from_ctx(EVP_PKEY_CTX *ctx, OSSL_PARAM *params) {
  EVP_PKEY *pkey(nullptr);
  if (ctx == nullptr || (EVP_PKEY_fromdata_init(ctx) == 0) ||
      (EVP_PKEY_fromdata(ctx, &pkey, EVP_PKEY_PUBLIC_KEY, params) == 0)) {
    throw std::runtime_error("RSA EVP_PKEY_fromdata failed");
  }
  return pkey;
}

std::string Jwk::to_pem() {
  const Raii<OSSL_PARAM, nullptr, OSSL_PARAM_free> param(construct_param());

  const Raii<EVP_PKEY_CTX, nullptr,
             [](EVP_PKEY_CTX *ctx) { EVP_PKEY_CTX_free(ctx); }>
      ctx(EVP_PKEY_CTX_new_from_name(nullptr, kty.c_str(), nullptr));

  const Raii<EVP_PKEY, nullptr, EVP_PKEY_free> pkey(
      pkey_from_ctx(ctx.get(), param.get()));
  const Raii<BIO, []() { return BIO_new(BIO_s_mem()); },
             [](BIO *bio) { BIO_free(bio); }>
      bio;
  std::string pem;

  if (PEM_write_bio_PUBKEY(bio.get(), pkey.get()) == 0)
    throw std::runtime_error("PEM_write_bio_PUBKEY failed");

  BUF_MEM *mem(nullptr);
  BIO_get_mem_ptr(bio.get(), &mem);
  pem.assign(mem->data, mem->length);
  return pem;
}
