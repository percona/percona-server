/* Copyright (c) 2024 Percona LLC and/or its affiliates. All rights reserved.

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

#include "masking_functions/dictionary.hpp"

#include "masking_functions/random_string_generators.hpp"

#include <iterator>

namespace masking_functions {

dictionary::dictionary(const std::string &term)
    :  // here we use std::unordered_set iterator range constructor with
       // single 'term' element converted to a fake range
      terms_{&term, std::next(&term)} {}

bool dictionary::contains(const std::string &term) const noexcept {
  // TODO: in c++20 change to terms_.contains(term)
  return terms_.count(term) > 0U;
}

std::string_view dictionary::get_random() const noexcept {
  if (terms_.empty()) {
    return {};
  }

  const auto random_index{random_number(0, terms_.size() - 1U)};
  return *std::next(std::begin(terms_), random_index);
}

bool dictionary::insert(const std::string &term) {
  return terms_.emplace(term).second;
}

bool dictionary::remove(const std::string &term) noexcept {
  return terms_.erase(term) > 0U;
}

}  // namespace masking_functions
