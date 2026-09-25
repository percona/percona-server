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

#include "components/audit_log_filter/charset_conversion.h"

#include "mysql/strings/m_ctype.h"

#include <stdexcept>

namespace audit_log_filter {

Utf8Conversion convert_query_to_utf8mb4(std::string_view input,
                                        const CHARSET_INFO *source) {
  Utf8Conversion result;
  if (input.empty()) return result;
  if (input.data() == nullptr || source == nullptr) {
    throw std::invalid_argument("Invalid audit query charset or buffer");
  }
  // Rewritten SQL has a binary label but contains UTF-8 identifiers. Decoding
  // it as one Unicode code point per byte would double-encode those
  // identifiers.
  if (source == &my_charset_bin) source = &my_charset_utf8mb4_bin;
  if (source->cset == nullptr || source->cset->mb_wc == nullptr) {
    throw std::invalid_argument("Invalid audit query charset decoder");
  }
  result.text.reserve(input.size());
  const auto *pos = reinterpret_cast<const uint8_t *>(input.data());
  const auto *end = pos + input.size();
  const bool ascii_compatible = my_charset_is_ascii_based(source);
  while (pos < end) {
    if (ascii_compatible && *pos < 0x80) {
      const auto *start = pos++;
      while (pos < end && *pos < 0x80) ++pos;
      result.text.append(reinterpret_cast<const char *>(start), pos - start);
      continue;
    }
    my_wc_t wc = 0;
    const int decoded = source->cset->mb_wc(source, &wc, pos, end);
    size_t consumed = 0;
    if (decoded > 0) {
      consumed = static_cast<size_t>(decoded);
      if (consumed > source->mbmaxlen ||
          consumed > static_cast<size_t>(end - pos)) {
        throw std::runtime_error("Invalid audit query decoder length");
      }
      uint8_t encoded[4];
      const int length = my_charset_utf8mb4_bin.cset->wc_mb(
          &my_charset_utf8mb4_bin, wc, encoded, encoded + sizeof(encoded));
      if (length > 0 && length <= static_cast<int>(sizeof(encoded)) &&
          wc <= 0x10ffff && !(wc >= 0xd800 && wc <= 0xdfff)) {
        result.text.append(reinterpret_cast<const char *>(encoded), length);
        pos += consumed;
        continue;
      }
      if (length != MY_CS_ILUNI && wc <= 0x10ffff &&
          !(wc >= 0xd800 && wc <= 0xdfff)) {
        throw std::runtime_error("Invalid audit query encoder result");
      }
    } else if (decoded == MY_CS_ILSEQ) {
      consumed = 1;
    } else if (decoded > MY_CS_TOOSMALL) {
      // Negative character length: complete sequence with no Unicode mapping.
      consumed = static_cast<size_t>(-decoded);
      if (consumed > source->mbmaxlen) {
        throw std::runtime_error("Invalid audit query unmapped length");
      }
    } else {
      const int needed = MY_CS_TOOSMALL - decoded + 1;
      if (needed <= 0 || static_cast<unsigned>(needed) > source->mbmaxlen ||
          static_cast<size_t>(needed) <= static_cast<size_t>(end - pos)) {
        throw std::runtime_error("Invalid audit query incomplete sequence");
      }
      consumed = end - pos;
    }
    if (consumed == 0 || consumed > static_cast<size_t>(end - pos)) {
      throw std::runtime_error("Invalid audit query decoder progress");
    }
    result.text.push_back('?');
    ++result.replacements;
    pos += consumed;
  }
  return result;
}

Utf8Conversion validate_utf8(std::string_view input) {
  return convert_query_to_utf8mb4(input, &my_charset_utf8mb4_bin);
}

}  // namespace audit_log_filter
