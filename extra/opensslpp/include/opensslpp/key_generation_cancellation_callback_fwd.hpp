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

#ifndef OPENSSLPP_KEY_GENERATION_CANCELLATION_CALLBACK_FWD_HPP
#define OPENSSLPP_KEY_GENERATION_CANCELLATION_CALLBACK_FWD_HPP

#include <functional>

namespace opensslpp {

// This callback can be passed to key generation functions
// such as rsa_key::generate(), dsa_key::generate_parameters(),
// dh_key::generate_parameters(), etc.
// OpenSSL internally periodically invokes it and checks for the return value.
// If the result is 'false', normal execution continues.
// If the result is 'true', execution aborts and key generation
// function throws operation_cancelled_error{} exception.
using key_generation_cancellation_callback = std::function<bool()>;

}  // namespace opensslpp

#endif
