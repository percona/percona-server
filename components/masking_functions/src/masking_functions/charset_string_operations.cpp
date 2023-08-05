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

#include "masking_functions/charset_string_operations.hpp"

#include <cassert>
#include <cstddef>
#include <locale>
#include <stdexcept>

#include "masking_functions/charset_string.hpp"

namespace {

masking_functions::charset_string &append_repeat(
    masking_functions::charset_string &initial,
    const masking_functions::charset_string &str, std::size_t n) {
  assert(initial.get_collation() == str.get_collation());
  // TODO: rework with logarithmic concatenation
  // while(...) { if (...) result += str; result += result; }
  for (std::size_t i = 0; i < n; ++i) initial += str;

  return initial;
}

}  // namespace

namespace masking_functions {

charset_string repeat(const charset_string &str, std::size_t n) {
  charset_string result{str.get_services(), "", str.get_collation()};
  append_repeat(result, str, n);

  assert(result.get_size_in_characters() == n * str.get_size_in_characters());

  return result;
}

charset_string mask_inner(const charset_string &str, std::size_t left_margin,
                          std::size_t right_margin,
                          const charset_string &mask_char) {
  if (mask_char.get_size_in_characters() != 1)
    throw std::invalid_argument{"invalid masking character specified"};
  charset_string conversion_buffer;
  const auto &converted_mask_char = smart_convert_to_collation(
      mask_char, str.get_collation(), conversion_buffer);

  const auto number_of_characters = str.get_size_in_characters();

  if (left_margin + right_margin >= number_of_characters) return str;

  const std::size_t pads_to_insert =
      number_of_characters - left_margin - right_margin;

  auto result = str.substr(0, left_margin);
  append_repeat(result, converted_mask_char, pads_to_insert);
  if (right_margin != 0) {
    result += str.substr(left_margin + pads_to_insert, right_margin);
  }

  assert(result.get_size_in_characters() == number_of_characters);

  return result;
}

charset_string mask_outer(const charset_string &str, std::size_t left_margin,
                          std::size_t right_margin,
                          const charset_string &mask_char) {
  if (mask_char.get_size_in_characters() != 1)
    throw std::invalid_argument{"invalid masking character specified"};
  charset_string conversion_buffer;
  const auto &converted_mask_char = smart_convert_to_collation(
      mask_char, str.get_collation(), conversion_buffer);

  const auto number_of_characters = str.get_size_in_characters();

  if (left_margin + right_margin >= number_of_characters)
    return repeat(converted_mask_char, number_of_characters);

  const std::size_t inner_part_length =
      number_of_characters - left_margin - right_margin;

  auto result = repeat(converted_mask_char, left_margin);
  result += str.substr(left_margin, inner_part_length);
  if (right_margin != 0) {
    append_repeat(result, converted_mask_char, right_margin);
  }

  assert(result.get_size_in_characters() == number_of_characters);

  return result;
}

charset_string mask_inner_alphanum(const charset_string &str,
                                   std::size_t left_margin,
                                   std::size_t right_margin,
                                   const charset_string &mask_char) {
  if (mask_char.get_size_in_characters() != 1)
    throw std::invalid_argument{"invalid masking character specified"};
  charset_string conversion_buffer;
  const auto &converted_mask_char = smart_convert_to_collation(
      mask_char, str.get_collation(), conversion_buffer);

  const auto number_of_characters = str.get_size_in_characters();

  if (left_margin + right_margin >= number_of_characters) return str;

  const std::size_t chars_to_process =
      number_of_characters - left_margin - right_margin;

  auto result = str.substr(0, left_margin);
  std::size_t index = left_margin;
  const std::size_t right_margin_index = left_margin + chars_to_process;

  bool previous_alnum_flag{false};
  bool alnum_flag{false};
  std::size_t group_length{0U};

  while (index <= right_margin_index) {
    // for the end of range we create an artificial alphanum / other
    // boundary by setting alnum_flag / previous_alnum_flag to different
    // values
    if (index == right_margin_index) {
      previous_alnum_flag = alnum_flag;
      alnum_flag = !previous_alnum_flag;
    } else {
      const auto ch = str[index];
      alnum_flag = (ch < 127U && std::isalnum(static_cast<char>(ch),
                                              std::locale::classic()));
    }

    // for the very first character we need to initialize previous_alnum_flag
    // so that it would not be treated as a alphanum / other transition
    if (index == left_margin) {
      previous_alnum_flag = alnum_flag;
    }

    if (alnum_flag == previous_alnum_flag) {
      ++group_length;
    } else {
      if (previous_alnum_flag) {
        append_repeat(result, converted_mask_char, group_length);
      } else {
        result += str.substr(index - group_length, group_length);
      }
      previous_alnum_flag = alnum_flag;
      group_length = 1;
    }
    ++index;
  }

  if (right_margin != 0) {
    result += str.substr(right_margin_index, right_margin);
  }
  assert(result.get_size_in_characters() == number_of_characters);

  return result;
}

}  // namespace masking_functions
