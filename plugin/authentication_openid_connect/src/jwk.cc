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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
*/

#include "jwk.h"

#include <algorithm>
#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/core_names.h>
#include <openssl/evp.h>
#include <openssl/param_build.h>
#include <openssl/params.h>
#include <openssl/pem.h>
#include <memory>
#include <stddef.h>
#include <stdexcept>
#include <string>
#include <vector>

std::vector<unsigned char> Jwk::base64url_decode(const std::string &input) {
  std::string prepared_input = input;
  const size_t padding = prepared_input.size() % 4;
  if (padding != 0) prepared_input.append(4 - padding, '=');
  std::ranges::replace(prepared_input, '-', '+');
  std::ranges::replace(prepared_input, '_', '/');

  std::vector<unsigned char> output(prepared_input.size());
  const int len = EVP_DecodeBlock(
      output.data(),
      reinterpret_cast<const unsigned char *>(prepared_input.data()),
      output.size());
  if (len < 0) throw std::runtime_error("Base64 decode failed");
  output.resize(len - (padding == 0 ? 0 : 4 - padding));
  return output;
}

OSSL_PARAM *Rsa_jwk::construct_param() {
  if (n.empty() || e.empty()) throw std::runtime_error("RSA requires n and e");

  const auto n_bytes = base64url_decode(n);
  const auto e_bytes = base64url_decode(e);
  const std::unique_ptr<BIGNUM, decltype(&BN_free)> bn_n(
      BN_bin2bn(n_bytes.data(), n_bytes.size(), nullptr), BN_free);
  const std::unique_ptr<BIGNUM, decltype(&BN_free)> bn_e(
      BN_bin2bn(e_bytes.data(), e_bytes.size(), nullptr), BN_free);

  // BN for RSA
  const std::unique_ptr<OSSL_PARAM_BLD, decltype(&OSSL_PARAM_BLD_free)>
      param_bld(OSSL_PARAM_BLD_new(), OSSL_PARAM_BLD_free);
  if (OSSL_PARAM_BLD_push_BN(param_bld.get(), OSSL_PKEY_PARAM_RSA_N,
                             bn_n.get()) == 0 ||
      OSSL_PARAM_BLD_push_BN(param_bld.get(), OSSL_PKEY_PARAM_RSA_E,
                             bn_e.get()) == 0)
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

  const std::unique_ptr<OSSL_PARAM_BLD, decltype(&OSSL_PARAM_BLD_free)>
      param_bld(OSSL_PARAM_BLD_new(), OSSL_PARAM_BLD_free);
  if (OSSL_PARAM_BLD_push_utf8_string(
          param_bld.get(), OSSL_PKEY_PARAM_GROUP_NAME, crv.c_str(), 0) == 0 ||
      OSSL_PARAM_BLD_push_octet_string(param_bld.get(), OSSL_PKEY_PARAM_PUB_KEY,
                                       pub_key_octet.data(),
                                       pub_key_octet.size()) == 0)
    throw std::runtime_error("Failed to build EC params");

  return OSSL_PARAM_BLD_to_param(param_bld.get());
}

EVP_PKEY *Jwk::pkey_from_ctx(EVP_PKEY_CTX *ctx, OSSL_PARAM *params) {
  EVP_PKEY *pkey(nullptr);
  if (ctx == nullptr || EVP_PKEY_fromdata_init(ctx) == 0 ||
      EVP_PKEY_fromdata(ctx, &pkey, EVP_PKEY_PUBLIC_KEY, params) == 0) {
    throw std::runtime_error("RSA EVP_PKEY_fromdata failed");
  }
  return pkey;
}

std::string Jwk::to_pem() {
  const std::unique_ptr<OSSL_PARAM, decltype(&OSSL_PARAM_free)> param(
      construct_param(), OSSL_PARAM_free);

  const std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)> ctx(
      EVP_PKEY_CTX_new_from_name(nullptr, kty.c_str(), nullptr),
      EVP_PKEY_CTX_free);
  const std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)> pkey(
      pkey_from_ctx(ctx.get(), param.get()), EVP_PKEY_free);
  const std::unique_ptr<BIO, decltype(&BIO_free)> bio(BIO_new(BIO_s_mem()),
                                                      BIO_free);
  std::string pem;

  if (PEM_write_bio_PUBKEY(bio.get(), pkey.get()) == 0)
    throw std::runtime_error("PEM_write_bio_PUBKEY failed");

  BUF_MEM *mem(nullptr);
  BIO_get_mem_ptr(bio.get(), &mem);
  pem.assign(mem->data, mem->length);
  return pem;
}
