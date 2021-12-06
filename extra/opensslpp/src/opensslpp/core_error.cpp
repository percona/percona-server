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

#include <array>
#include <cassert>

#include <openssl/err.h>

#include <opensslpp/core_error.hpp>

namespace opensslpp {

static constexpr std::size_t error_message_buffer_size = 256;

/* static */
[[noreturn]] void core_error::raise_with_error_string(
    const std::string &prefix /* = std::string() */) {
  std::string message{prefix};

  using buffer_type = std::array<char, error_message_buffer_size>;
  buffer_type buffer;

  unsigned long err = ERR_get_error();
  if (err != 0) {
    if (!prefix.empty()) message += ": ";

    ERR_error_string_n(err, buffer.data(), buffer.size());
    message += buffer.data();
    ERR_clear_error();
  }

  throw core_error{message};
}

}  // namespace opensslpp
