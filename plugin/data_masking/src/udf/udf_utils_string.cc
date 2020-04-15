/* Copyright (c) 2018, 2019 Francisco Miguel Biete Banon. All rights reserved.

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

#include "plugin/data_masking/include/udf/udf_utils_string.h"

#include <algorithm>
#include <cctype>
#include <functional>
#include <locale>

namespace mysql {
namespace plugins {
std::string &ltrim(std::string &s) {
  s.erase(s.begin(),
          std::find_if(s.begin(), s.end(),
                       std::not1(std::ptr_fun<int, int>(std::isspace))));
  return s;
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
std::string mask_inner(const char *str, const long str_length,
                       const int margin1, const int margin2,
                       const char mask_char) {
  if (margin1 < 0 || margin2 < 0) {
    return std::string();
  }

  std::string str_masked(str);
  // inim: start position
  int inim = (margin1 < str_length) ? margin1 : -1;
  // countm: num characters to mask
  int countm = str_length - (margin2 + margin1);
  if (inim >= 0 && countm >= 0) {
    auto maskchar = [mask_char]() -> char { return mask_char; };
    std::generate_n(str_masked.begin() + inim, countm, maskchar);
  }

  return str_masked;
}

/**
 * Masks the left and right ends of a string, leaving the interior unmasked, and
 * returns the result. An optional masking character can be specified.
 *
 * @param str: The string to mask.
 * @param str_length: The length of the string to mask.
 * @param margin1: A nonnegative integer that specifies the number of characters
 * on the left end of the string to mask. If the value is 0, no left end
 * characters are masked.
 * @param margin2: A nonnegative integer that specifies the number of characters
 * on the right end of the string to mask. If the value is 0, no right end
 * characters are masked.
 * @param mask_char: The single character to use for masking.
 *
 * @return The masked string, or empty if either margin is negative.
 *         If the sum of the margin values is larger than the argument length,
 * the entire argument is masked.
 */
std::string mask_outer(const char *str, const unsigned long str_length,
                       const long margin1, const long margin2,
                       const char mask_char) {
  if (margin1 < 0 || margin2 < 0) {
    return std::string();
  }

  std::string str_masked(str);
  auto maskchar = [mask_char]() -> char { return mask_char; };

  // Mask left
  std::generate_n(str_masked.begin(),
                  std::min(static_cast<unsigned long>(margin1), str_length),
                  maskchar);

  // Mask right
  if (static_cast<unsigned long>(margin2) < (str_length - 1))
    std::generate_n(str_masked.end() - margin2, margin2, maskchar);

  return str_masked;
}

std::string &rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       std::not1(std::ptr_fun<int, int>(std::isspace)))
              .base(),
          s.end());
  return s;
}

std::string &tolower(std::string &s) {
  std::transform(s.begin(), s.end(), s.begin(), ::tolower);
  return s;
}

std::string &trim(std::string &s) { return ltrim(rtrim(s)); }

}  // namespace plugins
}  // namespace mysql
