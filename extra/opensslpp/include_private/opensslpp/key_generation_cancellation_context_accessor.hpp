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

#ifndef OPENSSLPP_KEY_GENERATION_CANCELLATION_CONTEXT_ACCESSOR_HPP
#define OPENSSLPP_KEY_GENERATION_CANCELLATION_CONTEXT_ACCESSOR_HPP

#include <openssl/ossl_typ.h>

#include "opensslpp/key_generation_cancellation_context_fwd.hpp"
#include "opensslpp/typed_accessor.hpp"

namespace opensslpp {

using key_generation_cancellation_context_accessor =
    typed_accessor<key_generation_cancellation_context, BN_GENCB>;

}  // namespace opensslpp

#endif
