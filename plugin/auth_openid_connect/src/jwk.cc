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

#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#ifdef JWK_OPENSSL_3_0
#include <openssl/core_names.h>
#include <openssl/param_build.h>
#include <openssl/params.h>
#else
#include <openssl/ec.h>
#include <openssl/rsa.h>
#endif
#include <stddef.h>
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

std::vector<unsigned char> Jwk::base64url_decode(const std::string_view input) {
  std::string prepared_input{input.data(), input.size()};
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

#ifdef JWK_OPENSSL_3_0
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
#else
EVP_PKEY *Rsa_jwk::construct_pkey() {
  if (n.empty() || e.empty()) throw std::runtime_error("RSA requires n and e");

  const auto n_bytes = base64url_decode(n);
  const auto e_bytes = base64url_decode(e);
  std::unique_ptr<BIGNUM, decltype(&BN_free)> bn_n(
      BN_bin2bn(n_bytes.data(), n_bytes.size(), nullptr), BN_free);
  std::unique_ptr<BIGNUM, decltype(&BN_free)> bn_e(
      BN_bin2bn(e_bytes.data(), e_bytes.size(), nullptr), BN_free);
  if (!bn_n || !bn_e) throw std::runtime_error("BN_bin2bn failed for RSA");

  std::unique_ptr<RSA, decltype(&RSA_free)> rsa(RSA_new(), RSA_free);
  if (!rsa) throw std::runtime_error("RSA_new failed");

  // RSA_set0_key takes ownership of the BIGNUMs on success
  BIGNUM *raw_n = bn_n.release();
  BIGNUM *raw_e = bn_e.release();
  if (RSA_set0_key(rsa.get(), raw_n, raw_e, nullptr) == 0) {
    BN_free(raw_n);
    BN_free(raw_e);
    throw std::runtime_error("RSA_set0_key failed");
  }

  std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)> pkey(EVP_PKEY_new(),
                                                           EVP_PKEY_free);
  if (!pkey) throw std::runtime_error("EVP_PKEY_new failed");
  if (EVP_PKEY_assign_RSA(pkey.get(), rsa.release()) == 0)
    throw std::runtime_error("EVP_PKEY_assign_RSA failed");
  return pkey.release();
}
#endif

#ifdef JWK_OPENSSL_3_0
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
#else
EVP_PKEY *Ec_jwk::construct_pkey() {
  if (crv.empty() || x.empty() || y.empty())
    throw std::runtime_error("EC requires crv, x, y");

  int nid;
  if (crv == "P-256")
    nid = NID_X9_62_prime256v1;
  else if (crv == "P-384")
    nid = NID_secp384r1;
  else if (crv == "P-521")
    nid = NID_secp521r1;
  else
    throw std::runtime_error("Unsupported EC curve: " + crv);

  std::unique_ptr<EC_KEY, decltype(&EC_KEY_free)> ec_key(
      EC_KEY_new_by_curve_name(nid), EC_KEY_free);
  if (!ec_key) throw std::runtime_error("EC_KEY_new_by_curve_name failed");

  const auto x_bytes = base64url_decode(x);
  const auto y_bytes = base64url_decode(y);
  const std::unique_ptr<BIGNUM, decltype(&BN_free)> bn_x(
      BN_bin2bn(x_bytes.data(), x_bytes.size(), nullptr), BN_free);
  const std::unique_ptr<BIGNUM, decltype(&BN_free)> bn_y(
      BN_bin2bn(y_bytes.data(), y_bytes.size(), nullptr), BN_free);
  if (!bn_x || !bn_y) throw std::runtime_error("BN_bin2bn failed for EC");

  if (EC_KEY_set_public_key_affine_coordinates(ec_key.get(), bn_x.get(),
                                               bn_y.get()) == 0)
    throw std::runtime_error("EC_KEY_set_public_key_affine_coordinates failed");

  std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)> pkey(EVP_PKEY_new(),
                                                           EVP_PKEY_free);
  if (!pkey) throw std::runtime_error("EVP_PKEY_new failed");
  if (EVP_PKEY_assign_EC_KEY(pkey.get(), ec_key.release()) == 0)
    throw std::runtime_error("EVP_PKEY_assign_EC_KEY failed");
  return pkey.release();
}
#endif

#ifdef JWK_OPENSSL_3_0
EVP_PKEY *Jwk::construct_pkey() {
  EVP_PKEY *pkey(nullptr);
  const std::unique_ptr<OSSL_PARAM, decltype(&OSSL_PARAM_free)> param(
      construct_param(), OSSL_PARAM_free);

  const std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)> ctx_ptr(
      EVP_PKEY_CTX_new_from_name(nullptr, kty.c_str(), nullptr),
      EVP_PKEY_CTX_free);
  auto ctx = ctx_ptr.get();

  if (ctx == nullptr || EVP_PKEY_fromdata_init(ctx) == 0 ||
      EVP_PKEY_fromdata(ctx, &pkey, EVP_PKEY_PUBLIC_KEY, param.get()) == 0) {
    throw std::runtime_error("RSA EVP_PKEY_fromdata failed");
  }
  return pkey;
}
#endif

std::string Jwk::to_pem() {
  const std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)> pkey(
      construct_pkey(), EVP_PKEY_free);
  const std::unique_ptr<BIO, decltype(&BIO_free)> bio(BIO_new(BIO_s_mem()),
                                                      BIO_free);
  if (PEM_write_bio_PUBKEY(bio.get(), pkey.get()) == 0)
    throw std::runtime_error("PEM_write_bio_PUBKEY failed");

  BUF_MEM *mem(nullptr);
  BIO_get_mem_ptr(bio.get(), &mem);
  std::string pem;
  pem.assign(mem->data, mem->length);
  return pem;
}
