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

#include <cstddef>
#include <iterator>

#include "masking_functions/charset_string.hpp"
#include "masking_functions/random_string_generators.hpp"

namespace masking_functions {

const charset_string dictionary::shared_empty{};

bool dictionary::contains(const charset_string &term) const noexcept {
  // TODO: in c++20 change to terms_.contains(term)
  return terms_.count(term) > 0U;
}

const charset_string &dictionary::get_random() const noexcept {
  if (terms_.empty()) {
    return shared_empty;
  }

  const auto random_index{random_number(0, terms_.size() - 1U)};
  return *std::next(std::begin(terms_),
                    static_cast<std::ptrdiff_t>(random_index));
}

bool dictionary::insert(const charset_string &term) {
  return terms_.emplace(term).second;
}

bool dictionary::remove(const charset_string &term) noexcept {
  return terms_.erase(term) > 0U;
}

}  // namespace masking_functions
