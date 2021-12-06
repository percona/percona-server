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

#ifndef OPENSSLPP_RSA_SIGN_VERIFY_OPERATIONS_HPP
#define OPENSSLPP_RSA_SIGN_VERIFY_OPERATIONS_HPP

#include <string>

#include <opensslpp/rsa_key_fwd.hpp>

namespace opensslpp {

std::string sign_with_rsa_private_key(const std::string &digest_type,
                                      const std::string &digest_data,
                                      const rsa_key &key);

bool verify_with_rsa_public_key(const std::string &digest_type,
                                const std::string &digest_data,
                                const std::string &signature_data,
                                const rsa_key &key);

}  // namespace opensslpp

#endif
