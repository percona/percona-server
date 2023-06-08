/* Copyright (c) 2018, 2019 Francisco Miguel Biete Banon. All rights reserved.
   Copyright (c) 2023 Percona LLC and/or its affiliates. All rights reserved.

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

#include "components/masking_functions/include/component.h"
#include "components/masking_functions/include/udf/udf_utils.h"

#include "components/masking_functions/include/udf/udf_utils_string.h"

#include <algorithm>
#include <cctype>
#include <functional>
#include <locale>

#include "m_ctype.h"
#include "my_sys.h"

#include <mysql/components/services/mysql_string.h>
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_string_converter);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_string_character_access);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_string_factory);

namespace mysql {
namespace plugins {
std::string &ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char c) {
            return !std::isspace(c);
          }));
  return s;
}

std::string convert(std::string_view const &src, std::string_view const &src_cs,
                    std::string_view const &dst_cs) {
  // In practice we only use this to convert single character strings
  // For that, this buffer is more than enough
  constexpr std::size_t buffer_size = 256;
  char buffer[buffer_size];
  uint64 used_buffer = 0;

  // We can't use the mysql_string_converter service, because it assumes that
  // strings are null terminated Which is not true for charsets such as UCS2 or
  // UTF32
  CHARSET_INFO *cs_o =
      get_charset_by_csname(dst_cs.data(), MY_CS_PRIMARY, MYF(0));
  CHARSET_INFO *cs_r =
      get_charset_by_csname(src_cs.data(), MY_CS_PRIMARY, MYF(0));

  uint error;
  used_buffer = my_convert(buffer, buffer_size - 1, cs_o, src.data(),
                           src.size(), cs_r, &error);

  if (cs_o == nullptr || cs_r == nullptr) {
    return "";
  }
  if (error != 0) {
    return "";
  }

  return std::string(buffer, used_buffer);
}

/**
 * Masks the interior part of a string, leaving the ends untouched, and returns
 * the result. An optional masking character can be specified.
 * @param str: The string to mask.
 * @param str_length: The length of the string to mask
 * @param margin1: A nonnegative integer that specifies the number of characters
 * on the left end of the string to remain unmasked. If the value is 0, no left
 * end characters remain unmasked.
 * @param margin2: A nonnegative integer that specifies the number of characters
 * on the right end of the string to remain unmasked. If the value is 0, no
 * right end characters remain unmasked.
 * @param mask_char: The single character used for masking.
 *
 * @returns The masked string, or empty if either margin is negative. If the sum
 * of the margin values is larger than the argument length, no masking occurs
 * and the argument is returned unchanged.
 */
std::string mask_inner(const char *str, std::size_t str_length,
                       std::size_t margin1, std::size_t margin2,
                       std::string_view const &original_charset,
                       std::string_view const &mask_char) {
  // Calculate margins from offsets
  // NOTE: the string service doesn't work with some character sets, such as
  // utf16/32
  ulong c2, c3;
  uint mlen = 0;
  {
    my_h_string mstr;
    mysql_service_mysql_string_converter->convert_from_buffer(
        &mstr, str, str_length, original_charset.data());
    mysql_service_mysql_string_character_access->get_char_length(mstr, &mlen);
    mysql_service_mysql_string_character_access->get_char_offset(mstr, margin1,
                                                                 &c2);
    mysql_service_mysql_string_character_access->get_char_offset(
        mstr, mlen - margin2, &c3);
    mysql_service_mysql_string_factory->destroy(mstr);
  }

  if (margin1 + margin2 >= mlen) {
    // too long => return unchanged
    return std::string(str, str_length);
  }

  const int pads_to_insert = mlen - margin1 - margin2;

  std::string sresult;
  if (margin1 > 0) sresult.append(str, c2);

  for (int i = 0; i < pads_to_insert; ++i) sresult.append(mask_char);

  if (margin2 > 0) sresult.append(str + c2 + pads_to_insert, str_length - c3);

  return sresult;
}

std::string &rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](unsigned char c) { return !std::isspace(c); })
              .base(),
          s.end());
  return s;
}

std::string &tolower(std::string &s) {
  std::transform(s.begin(), s.end(), s.begin(), ::tolower);
  return s;
}

std::string &trim(std::string &s) { return ltrim(rtrim(s)); }

std::string decide_masking_char(mysqlpp::udf_context const &ctx,
                                std::size_t argno,
                                std::string_view const &original_charset,
                                std::string_view const &def) {
  std::string masking_char;
  if (ctx.get_number_of_args() >= argno + 1) {
    auto repl_charset = ctx.get_arg_charset(argno);

    if (repl_charset != original_charset) {
      masking_char = mysql::plugins::convert(ctx.get_arg<STRING_RESULT>(argno),
                                             repl_charset, original_charset);
    } else {
      masking_char = ctx.get_arg<STRING_RESULT>(argno);
    }
  } else {
    masking_char = mysql::plugins::convert(def, "utf8mb4", original_charset);
  }

  return masking_char;
}

std::string decide_masking_char(UDF_ARGS *args, std::size_t argno,
                                std::string_view const &original_charset,
                                std::string_view const &def) {
  std::string masking_char;
  if (args->arg_count >= argno + 1) {
    std::string repl_charset;
    mysql::plugins::get_arg_character_set(args, argno, repl_charset);

    if (repl_charset != original_charset) {
      masking_char = mysql::plugins::convert(
          std::string_view(args->args[argno], args->lengths[argno]),
          repl_charset, original_charset);
    } else {
      masking_char = std::string(args->args[argno], args->lengths[argno]);
    }
  } else {
    masking_char = mysql::plugins::convert(def, "utf8mb4", original_charset);
  }

  return masking_char;
}

}  // namespace plugins
}  // namespace mysql
