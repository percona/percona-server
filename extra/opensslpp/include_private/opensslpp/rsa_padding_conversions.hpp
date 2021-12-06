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

#ifndef OPENSSLPP_RSA_PADDING_CONVERSIONS_HPP
#define OPENSSLPP_RSA_PADDING_CONVERSIONS_HPP

#include <cassert>

#include <openssl/rsa.h>

#include <opensslpp/rsa_padding.hpp>

namespace opensslpp {

inline int rsa_padding_to_native_padding(rsa_padding padding) noexcept {
  int res = RSA_NO_PADDING;
  switch (padding) {
    case rsa_padding::no:
      res = RSA_NO_PADDING;
      break;
    case rsa_padding::pkcs1:
      res = RSA_PKCS1_PADDING;
      break;
    default:
      assert(false);
  }
  return res;
}

}  // namespace opensslpp

#endif
