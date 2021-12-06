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

#include <openssl/dh.h>

#include <opensslpp/dh_compute_operations.hpp>

#include <opensslpp/big_number.hpp>
#include <opensslpp/core_error.hpp>
#include <opensslpp/dh_key.hpp>
#include <opensslpp/dh_padding.hpp>

#include "opensslpp/big_number_accessor.hpp"
#include "opensslpp/dh_key_accessor.hpp"

namespace opensslpp {

using compute_key_function = int (*)(unsigned char *, const BIGNUM *, DH *);

static compute_key_function get_compute_key_function(
    dh_padding padding) noexcept {
  compute_key_function res = nullptr;
  switch (padding) {
    case dh_padding::rfc5246:
      res = &DH_compute_key;
      break;
    case dh_padding::nist_sp800_56a:
      res = &DH_compute_key_padded;
      break;
  }
  assert(res != nullptr);
  return res;
}

static std::string compute_dh_key_internal(const BIGNUM *public_component,
                                           const dh_key &private_key,
                                           dh_padding padding) {
  if (!private_key.has_private_component())
    throw core_error{
        "cannot compute shared key as DH key does not have private component"};

  auto function = get_compute_key_function(padding);

  // TODO: use c++17 non-const std::string::data() member here
  using buffer_type = std::vector<unsigned char>;
  buffer_type res(private_key.get_size_in_bytes());
  auto compute_status =
      (*function)(res.data(), public_component,
                  dh_key_accessor::get_impl_const_casted(private_key));

  if (compute_status == -1)
    core_error::raise_with_error_string(
        "cannot compute shared key from DH private / public components");

  return {reinterpret_cast<char *>(res.data()), res.size()};
}

std::string compute_dh_key(const big_number &public_component,
                           const dh_key &private_key, dh_padding padding) {
  assert(!public_component.is_empty());
  assert(!private_key.is_empty());
  return compute_dh_key_internal(
      big_number_accessor::get_impl(public_component), private_key, padding);
}

std::string compute_dh_key(const dh_key &public_key, const dh_key &private_key,
                           dh_padding padding) {
  assert(!public_key.is_empty());
  assert(!private_key.is_empty());
  if (!public_key.has_public_component())
    throw core_error{
        "cannot compute shared key as DH key does not have public component"};

  const auto *dh_raw = dh_key_accessor::get_impl(public_key);
  const auto *public_component_raw =
#if OPENSSL_VERSION_NUMBER < 0x10100000L
      dh_raw->pub_key;
#else
      DH_get0_pub_key(dh_raw);
#endif

  return compute_dh_key_internal(public_component_raw, private_key, padding);
}

}  // namespace opensslpp
