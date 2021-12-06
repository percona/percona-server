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

#include <openssl/dsa.h>
#include <openssl/evp.h>

#include <opensslpp/dsa_sign_verify_operations.hpp>

#include <opensslpp/core_error.hpp>
#include <opensslpp/dsa_key.hpp>

#include "opensslpp/dsa_key_accessor.hpp"

namespace opensslpp {

std::string sign_with_dsa_private_key(const std::string &digest_type,
                                      const std::string &digest_data,
                                      const dsa_key &key) {
  assert(!key.is_empty());

  if (!key.has_private_component())
    throw core_error{"DSA key does not have private component"};

  auto md = EVP_get_digestbyname(digest_type.c_str());
  if (md == nullptr) throw core_error{"unknown digest name"};

  auto md_nid = EVP_MD_type(md);

  // TODO: use c++17 non-const std::string::data() member here
  using buffer_type = std::vector<unsigned char>;
  buffer_type res(key.get_size_in_bytes());

  unsigned int signature_length = 0;
  auto sign_status = DSA_sign(
      md_nid, reinterpret_cast<const unsigned char *>(digest_data.c_str()),
      digest_data.size(), res.data(), &signature_length,
      dsa_key_accessor::get_impl_const_casted(key));

  if (sign_status != 1)
    core_error::raise_with_error_string(
        "cannot sign message digest with the specified private DSA key");

  return {reinterpret_cast<char *>(res.data()),
          static_cast<std::size_t>(signature_length)};
}

bool verify_with_dsa_public_key(const std::string &digest_type,
                                const std::string &digest_data,
                                const std::string &signature_data,
                                const dsa_key &key) {
  assert(!key.is_empty());

  auto md = EVP_get_digestbyname(digest_type.c_str());
  if (md == nullptr) throw core_error{"unknown digest name"};

  auto md_nid = EVP_MD_type(md);

  auto verify_status = DSA_verify(
      md_nid, reinterpret_cast<const unsigned char *>(digest_data.c_str()),
      digest_data.size(),
      reinterpret_cast<const unsigned char *>(signature_data.c_str()),
      signature_data.size(), dsa_key_accessor::get_impl_const_casted(key));

  assert(verify_status == -1 || verify_status == 0 || verify_status == 1);
  if (verify_status == -1)
    core_error::raise_with_error_string(
        "cannot verify message signature with the specified public DSA key");

  return verify_status == 1;
}

}  // namespace opensslpp
