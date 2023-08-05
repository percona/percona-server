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

#ifndef MASKING_FUNCTIONS_CHARSET_STRING_OPERATIONS_HPP
#define MASKING_FUNCTIONS_CHARSET_STRING_OPERATIONS_HPP

#include <cstddef>

#include "masking_functions/charset_string_fwd.hpp"

namespace masking_functions {

// A set of helper functions that operate on 'charset_string' and
// that are too complex to be a part of the class interface.

// Returns a new 'charset_string' that has 'srt' repeated 'n' times.
charset_string repeat(const charset_string &str, std::size_t n);

// Returns a copy of 'str' in which every character that is not within the
// first 'left_margin' characters and not within the last 'right_margin'
// characters is masked with 'maksd_char'.
charset_string mask_inner(const charset_string &str, std::size_t left_margin,
                          std::size_t right_margin,
                          const charset_string &mask_char);

// Returns a copy of 'str' in which every character within the first
// 'left_margin' characters or within the last 'right_margin'
// characters is masked with 'maksd_char'.
charset_string mask_outer(const charset_string &str, std::size_t left_margin,
                          std::size_t right_margin,
                          const charset_string &mask_char);

// Returns a copy of 'str' in which every alpha-numeric latin character
// ('[a-z-A-Z0-9]') that is not within the first 'left_margin' characters
// and not within the last 'right_margin' characters is masked with
// 'maksd_char'.
charset_string mask_inner_alphanum(const charset_string &str,
                                   std::size_t left_margin,
                                   std::size_t right_margin,
                                   const charset_string &mask_char);

}  // namespace masking_functions

#endif  // MASKING_FUNCTIONS_CHARSET_STRING_OPERATIONS_HPP
