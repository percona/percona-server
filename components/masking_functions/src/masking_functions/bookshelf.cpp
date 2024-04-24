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

#include <mutex>
#include <optional>
#include <shared_mutex>

#include "masking_functions/dictionary.hpp"

namespace masking_functions {

bool bookshelf::contains(const std::string &dictionary_name,
                         const std::string &term) const noexcept {
  const auto dict{find_dictionary_internal(dictionary_name)};
  if (!dict) {
    return false;
  }
  return dict->contains(term);
}

// returning a copy deliberately for thread safety
optional_string bookshelf::get_random(
    const std::string &dictionary_name) const noexcept {
  const auto dict{find_dictionary_internal(dictionary_name)};
  if (!dict) {
    return std::nullopt;
  }
  return dict->get_random();
}

bool bookshelf::remove(const std::string &dictionary_name) noexcept {
  std::unique_lock dictionaries_write_lock{dictionaries_mutex_};
  return dictionaries_.erase(dictionary_name) != 0U;
}

bool bookshelf::remove(const std::string &dictionary_name,
                       const std::string &term) noexcept {
  const auto dict{find_dictionary_internal(dictionary_name)};
  if (!dict) {
    return false;
  }
  return dict->remove(term);
  // after this operation we may have a dictionary with no terms in it -
  // it is fine and much safer than trying to re-acquire a write lock and
  // removing the dictionary from the bookshelf when it has 0 terms.
}

bool bookshelf::insert(const std::string &dictionary_name,
                       const std::string &term) {
  auto dict{find_dictionary_internal(dictionary_name)};
  if (dict) {
    return dict->insert(term);
  }

  // if no dictionary with such name alteady exist, we need to
  // create it under a write lock
  {
    std::unique_lock dictionaries_write_lock{dictionaries_mutex_};
    // it may happen that between the read and write locks another thread
    // already created the dictionary with the same name - checking again
    dict = std::make_shared<dictionary>(term);
    const auto [dictionary_it,
                inserted]{dictionaries_.emplace(dictionary_name, dict)};
    if (inserted) {
      return true;
    }
    dict = dictionary_it->second;
  }
  return dict->insert(term);
}

dictionary_ptr bookshelf::find_dictionary_internal(
    const std::string &dictionary_name) const noexcept {
  std::shared_lock dictionaries_read_lock{dictionaries_mutex_};
  const auto dictionary_it{dictionaries_.find(dictionary_name)};
  if (dictionary_it == std::cend(dictionaries_)) {
    return {};
  }
  return dictionary_it->second;
}

}  // namespace masking_functions
