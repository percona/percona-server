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

#ifndef MASKING_FUNCTIONS_DICT_CONTAINER_HPP
#define MASKING_FUNCTIONS_DICT_CONTAINER_HPP

#include <map>
#include <mutex>
#include <optional>
#include <set>
#include <shared_mutex>
#include <string>
#include <vector>

namespace masking_functions {

using optional_string = std::optional<std::string>;

class dictionary_container {
  struct term_container {
    explicit term_container(std::string term) : term_list{std::move(term)} {}
    mutable std::shared_mutex term_mutex;
    std::set<std::string> term_list;
  };

 public:
  bool contains(const std::string &dictionary_name,
                const std::string &term) const noexcept;
  optional_string get(const std::string &dictionary_name) const noexcept;
  bool remove(const std::string &dictionary_name) noexcept;
  bool remove(const std::string &dictionary_name,
              const std::string &term) noexcept;
  bool insert(const std::string &dictionary_name,
              const std::string &term) noexcept;

 private:
  std::map<std::string, term_container> m_container;
};

using optional_dictionary_container = std::optional<dictionary_container>;

}  // namespace masking_functions

#endif  // MASKING_FUNCTIONS_DICT_CONTAINER_HPP
