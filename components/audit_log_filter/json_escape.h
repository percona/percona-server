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

#ifndef AUDIT_LOG_FILTER_JSON_ESCAPE_H_INCLUDED
#define AUDIT_LOG_FILTER_JSON_ESCAPE_H_INCLUDED

#include <string>
#include <string_view>

namespace audit_log_filter {

/**
 * @brief Append a string to @p out escaped as JSON string contents, without
 *        the surrounding quotes. Embedded NULs are escaped too.
 *
 * The output is the same as rapidjson::Writer<StringBuffer>::String(): quote
 * and backslash are escaped, backspace, form feed, newline, carriage return
 * and tab use their short escapes, any other byte below 0x20 becomes a
 * six-character Unicode escape, and every other byte (UTF-8 included) is
 * copied. The Writer is not used because GCC 16 reports a false
 * -Wstringop-overflow inside rapidjson's StringBuffer in optimized builds.
 *
 * @param out Output string
 * @param in String to be escaped
 */
inline void append_json_escaped(std::string &out, std::string_view in) {
  static constexpr char kHexDigits[] = "0123456789ABCDEF";

  for (const char ch : in) {
    const auto c = static_cast<unsigned char>(ch);
    switch (c) {
      case '"':
        out.append("\\\"");
        break;
      case '\\':
        out.append("\\\\");
        break;
      case '\b':
        out.append("\\b");
        break;
      case '\f':
        out.append("\\f");
        break;
      case '\n':
        out.append("\\n");
        break;
      case '\r':
        out.append("\\r");
        break;
      case '\t':
        out.append("\\t");
        break;
      default:
        if (c < 0x20) {
          const char escaped[] = {
              '\\', 'u', '0', '0', kHexDigits[c >> 4], kHexDigits[c & 0x0F]};
          out.append(escaped, sizeof(escaped));
        } else {
          out.push_back(ch);
        }
    }
  }
}

}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_JSON_ESCAPE_H_INCLUDED
