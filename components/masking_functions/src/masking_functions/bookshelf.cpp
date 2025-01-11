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

#include "masking_functions/bookshelf.hpp"

#include <iterator>

#include "masking_functions/charset_string.hpp"
#include "masking_functions/dictionary.hpp"

namespace masking_functions {

bool bookshelf::contains(const charset_string &dictionary_name,
                         const charset_string &term) const noexcept {
  const auto dictionary_it{dictionaries_.find(dictionary_name)};
  if (dictionary_it == std::cend(dictionaries_)) {
    return false;
  }
  return dictionary_it->second.contains(term);
}

const charset_string &bookshelf::get_random(
    const charset_string &dictionary_name) const noexcept {
  const auto dictionary_it{dictionaries_.find(dictionary_name)};
  if (dictionary_it == std::cend(dictionaries_)) {
    return dictionary::shared_empty;
  }
  return dictionary_it->second.get_random();
}

bool bookshelf::remove(const charset_string &dictionary_name) noexcept {
  return dictionaries_.erase(dictionary_name) != 0U;
}

bool bookshelf::remove(const charset_string &dictionary_name,
                       const charset_string &term) noexcept {
  const auto dictionary_it{dictionaries_.find(dictionary_name)};
  if (dictionary_it == std::end(dictionaries_)) {
    return false;
  }
  const auto result{dictionary_it->second.remove(term)};
  if (dictionary_it->second.is_empty()) {
    dictionaries_.erase(dictionary_it);
  }
  return result;
}

bool bookshelf::insert(const charset_string &dictionary_name,
                       const charset_string &term) {
  // here we use try_emplace as a combined version of find and
  // insert
  const auto [dictionary_it,
              inserted]{dictionaries_.try_emplace(dictionary_name)};
  if (!inserted) {
    return dictionary_it->second.insert(term);
  }
  try {
    dictionary_it->second.insert(term);
  } catch (...) {
    dictionaries_.erase(dictionary_it);
    throw;
  }
  return true;
}

}  // namespace masking_functions
