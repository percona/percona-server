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

#include "masking_functions/dictionary_container.hpp"

#include "masking_functions/random_string_generators.hpp"

#include <algorithm>

namespace masking_functions {

bool dictionary_container::contains(const std::string &dictionary_name,
                                    const std::string &term) const noexcept {
  const auto it = m_container.find(dictionary_name);

  if (it == m_container.cend()) {
    return false;
  }

  if (term.length() == 0) {
    return true;
  }

  std::shared_lock term_read_lock{it->second.term_mutex};
  return it->second.term_list.count(term) > 0;
}

optional_string dictionary_container::get(
    const std::string &dictionary_name) const noexcept {
  const auto dict_it = m_container.find(dictionary_name);

  if (dict_it == m_container.cend()) {
    return std::nullopt;
  }

  std::shared_lock term_read_lock{dict_it->second.term_mutex};

  if (dict_it->second.term_list.empty()) {
    return std::nullopt;
  }

  auto random_step = random_number(0, dict_it->second.term_list.size() - 1);
  auto term_it = dict_it->second.term_list.begin();
  std::advance(term_it, random_step);

  return optional_string{std::in_place, *term_it};
}

bool dictionary_container::remove(const std::string &dictionary_name) noexcept {
  return m_container.erase(dictionary_name) > 0;
}

bool dictionary_container::remove(const std::string &dictionary_name,
                                  const std::string &term) noexcept {
  const auto dict_it = m_container.find(dictionary_name);

  if (dict_it == m_container.cend()) {
    return false;
  }

  std::unique_lock term_write_lock{dict_it->second.term_mutex};
  return dict_it->second.term_list.erase(term) > 0;
}

bool dictionary_container::insert(const std::string &dictionary_name,
                                  const std::string &term) noexcept {
  auto it = m_container.find(dictionary_name);

  if (it != m_container.end()) {
    std::unique_lock term_write_lock{it->second.term_mutex};
    it->second.term_list.emplace(term);
  } else {
    m_container.emplace(dictionary_name, term);
  }

  return true;
}

}  // namespace masking_functions
