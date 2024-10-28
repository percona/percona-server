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

#ifndef OPENSSLPP_EVP_PKEY_SIGN_VERIFY_OPERATIONS_HPP
#define OPENSSLPP_EVP_PKEY_SIGN_VERIFY_OPERATIONS_HPP

#include <string>
#include <string_view>

#include <opensslpp/evp_pkey_fwd.hpp>
#include <opensslpp/evp_pkey_signature_padding_fwd.hpp>

namespace opensslpp {

// no std::string_view for 'digest_type' as we need it to be nul-terminated
std::string sign_with_private_evp_pkey(const std::string &digest_type,
                                       std::string_view digest_data,
                                       const evp_pkey &key,
                                       evp_pkey_signature_padding padding);

// no std::string_view for 'digest_type' as we need it to be nul-terminated
bool verify_with_public_evp_pkey(const std::string &digest_type,
                                 std::string_view digest_data,
                                 std::string_view signature_data,
                                 const evp_pkey &key,
                                 evp_pkey_signature_padding padding);

}  // namespace opensslpp

#endif
