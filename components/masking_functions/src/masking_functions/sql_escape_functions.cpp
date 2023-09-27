/* Copyright (c) 2023 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   51 Franklin Street, Suite 500, Boston, MA 02110-1335 USA */

#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <string>

#include <my_sys.h>

#include "masking_functions/charset_string.hpp"

namespace masking_functions {

std::string escape_string(const charset_string &cs_str) {
  // by default we always convert strings being escaped to utf8mb4,
  // as this is the default character set used by internal
  // mysql_command_factory / mysql_command_query services
  charset_string conversion_buffer;
  const auto &utf8mb4_cs_str = smart_convert_to_collation(
      cs_str, charset_string::get_utf8mb4_collation(cs_str.get_services()),
      conversion_buffer);
  // resize the buffer to fit the worst case length of utf8mb4_cs_str
  const std::size_t max_size = 4 * utf8mb4_cs_str.get_size_in_characters() + 1;
  std::string res(max_size, '_');

  auto buffer = utf8mb4_cs_str.get_buffer();
  const auto *cs =
      get_charset_by_name(charset_string::utf8mb4_collation_name, MYF(0));
  assert(cs != nullptr);
  std::size_t r = escape_string_for_mysql(cs, res.data(), max_size,
                                          buffer.data(), buffer.size());
  if (r == ~static_cast<std::size_t>(0))
    throw std::runtime_error{"cannot escape string for sql"};

  res.resize(r);
  return res;
}

}  // namespace masking_functions
