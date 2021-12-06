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

#ifndef OPENSSLPP_DH_COMPUTE_OPERATIONS_HPP
#define OPENSSLPP_DH_COMPUTE_OPERATIONS_HPP

#include <string>

#include <opensslpp/dh_key_fwd.hpp>

#include <opensslpp/big_number_fwd.hpp>
#include <opensslpp/dh_padding_fwd.hpp>

namespace opensslpp {

std::string compute_dh_key(const big_number &public_component,
                           const dh_key &private_key, dh_padding padding);

std::string compute_dh_key(const dh_key &public_key, const dh_key &private_key,
                           dh_padding padding);

}  // namespace opensslpp

#endif
