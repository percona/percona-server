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

#ifndef MASKING_FUNCTIONS_BOOKSHELF_HPP
#define MASKING_FUNCTIONS_BOOKSHELF_HPP

#include "masking_functions/bookshelf_fwd.hpp"

#include <string>
#include <string_view>
#include <unordered_map>

#include "masking_functions/dictionary_fwd.hpp"

namespace masking_functions {

class bookshelf {
 public:
  bookshelf();
  bookshelf(const dictionary &) = delete;
  bookshelf(bookshelf &&) = delete;
  bookshelf &operator=(const bookshelf &) = delete;
  bookshelf &operator=(bookshelf &&) = delete;
  ~bookshelf();

  bool contains(const std::string &dictionary_name,
                const std::string &term) const noexcept;
  // returns empty std::string_view if no such dictionary exist
  std::string_view get_random(
      const std::string &dictionary_name) const noexcept;
  bool remove(const std::string &dictionary_name) noexcept;
  bool remove(const std::string &dictionary_name,
              const std::string &term) noexcept;
  bool insert(const std::string &dictionary_name, const std::string &term);

 private:
  // TODO: in c++20 change to method signatures to accept std::string_view
  //       and container to std::unordered_map<std::string, dictionary_ptr,
  //       transparent_string_like_hash, std::equal_to<>>.
  using dictionary_container = std::unordered_map<std::string, dictionary_ptr>;
  dictionary_container dictionaries_;
};

}  // namespace masking_functions

#endif  // MASKING_FUNCTIONS_BOOKSHELF_HPP
