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

#include <algorithm>
#include <cassert>
#include <stdexcept>

namespace audit_log_filter {

std::string convert_query_to_utf8mb4(std::string_view input,
                                     const CHARSET_INFO *source) {
  std::string result;
  if (input.empty()) return result;
  if (input.data() == nullptr || source == nullptr) {
    throw std::invalid_argument("Invalid audit query charset or buffer");
  }
  // Rewritten SQL has a binary label but contains UTF-8 identifiers. Decoding
  // it as one Unicode code point per byte would double-encode those
  // identifiers.
  if (source == &my_charset_bin) source = &my_charset_utf8mb4_bin;
  result.reserve(input.size());
  const auto *pos = reinterpret_cast<const uint8_t *>(input.data());
  const auto *end = pos + input.size();
  const bool ascii_compatible = my_charset_is_ascii_based(source);
  while (pos < end) {
    if (ascii_compatible && *pos < 0x80) {
      const auto *start = pos++;
      while (pos < end && *pos < 0x80) ++pos;
      result.append(reinterpret_cast<const char *>(start), pos - start);
      continue;
    }
    my_wc_t wc = 0;
    const int decoded = source->cset->mb_wc(source, &wc, pos, end);
    // A malformed byte (MY_CS_ILSEQ) or an incomplete final sequence
    // (MY_CS_TOOSMALL*) skips one byte, so text after a bad lead byte is kept.
    // A character without a Unicode mapping (-length) is skipped whole.
    const size_t consumed = decoded > 0                ? decoded
                            : decoded > MY_CS_TOOSMALL ? std::max(-decoded, 1)
                                                       : 1;
    if (consumed > static_cast<size_t>(end - pos)) {
      throw std::runtime_error("Invalid audit query decoder length");
    }
    pos += consumed;
    // ucs2 and utf32 decode surrogates and values above U+10FFFF, which have
    // no valid UTF-8 encoding.
    if (decoded <= 0 || wc > 0x10ffff || (wc >= 0xd800 && wc <= 0xdfff)) {
      result.push_back('?');
      continue;
    }
    uint8_t encoded[4];
    const int length = my_charset_utf8mb4_bin.cset->wc_mb(
        &my_charset_utf8mb4_bin, wc, encoded, encoded + sizeof(encoded));
    assert(length > 0);
    result.append(reinterpret_cast<const char *>(encoded), length);
  }
  return result;
}

std::string sanitize_utf8(std::string_view input) {
  return convert_query_to_utf8mb4(input, &my_charset_utf8mb4_bin);
}

}  // namespace audit_log_filter
