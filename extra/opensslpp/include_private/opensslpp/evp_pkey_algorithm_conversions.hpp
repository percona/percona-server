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

#ifndef OPENSSLPP_EVP_PKEY_ALGORITHM_CONVERSIONS_HPP
#define OPENSSLPP_EVP_PKEY_ALGORITHM_CONVERSIONS_HPP

#include <cassert>

#include <openssl/evp.h>

#include <opensslpp/evp_pkey_algorithm.hpp>

namespace opensslpp {

inline int evp_pkey_algorithm_to_native_algorithm(
    evp_pkey_algorithm algorithm) noexcept {
  int res = EVP_PKEY_NONE;
  switch (algorithm) {
    case evp_pkey_algorithm::rsa:
      res = EVP_PKEY_RSA;
      break;
    case evp_pkey_algorithm::dsa:
      res = EVP_PKEY_DSA;
      break;
    case evp_pkey_algorithm::dh:
      res = EVP_PKEY_DH;
      break;
    default:
      res = EVP_PKEY_NONE;
  }
  return res;
}

inline evp_pkey_algorithm native_algorithm_to_evp_pkey_algorithm(
    int native_algorithm) noexcept {
  evp_pkey_algorithm res = evp_pkey_algorithm::unspecified;
  switch (native_algorithm) {
    case EVP_PKEY_RSA:
      res = evp_pkey_algorithm::rsa;
      break;
    case EVP_PKEY_DSA:
      res = evp_pkey_algorithm::dsa;
      break;
    case EVP_PKEY_DH:
      res = evp_pkey_algorithm::dh;
      break;
  }
  return res;
}

}  // namespace opensslpp

#endif
