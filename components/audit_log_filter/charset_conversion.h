/* Copyright (c) 2026 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#ifndef AUDIT_LOG_FILTER_CHARSET_CONVERSION_H_INCLUDED
#define AUDIT_LOG_FILTER_CHARSET_CONVERSION_H_INCLUDED

#include <cstddef>
#include <string>
#include <string_view>

struct CHARSET_INFO;

namespace audit_log_filter {

struct Utf8Conversion {
  std::string text;
  size_t replacements{0};
};

// Length-aware conversion, including embedded NULs. Malformed characters become
// '?'; invalid contracts and internal/resource failures throw, never return raw
// bytes. A binary source is treated as UTF-8 (password rewrites use this
// label).
Utf8Conversion convert_query_to_utf8mb4(std::string_view input,
                                        const CHARSET_INFO *source);
Utf8Conversion validate_utf8(std::string_view input);

}  // namespace audit_log_filter
#endif  // AUDIT_LOG_FILTER_CHARSET_CONVERSION_H_INCLUDED
