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
#include <mutex>
#include <optional>

namespace masking_functions {

dictionary::dictionary(const std::string &term) : terms_{}, terms_mutex_{} {
  terms_.emplace(term);
}

bool dictionary::contains(const std::string &term) const noexcept {
  std::shared_lock terms_read_lock{terms_mutex_};
  return terms_.count(term) > 0U;
}

optional_string dictionary::get_random() const {
  std::shared_lock terms_read_lock{terms_mutex_};

  if (terms_.empty()) {
    return std::nullopt;
  }

  const auto random_index{random_number(0, terms_.size() - 1U)};
  return *std::next(terms_.begin(), random_index);
}

bool dictionary::insert(const std::string &term) {
  std::unique_lock terms_write_lock{terms_mutex_};
  return terms_.emplace(term).second;
}

bool dictionary::remove(const std::string &term) noexcept {
  std::unique_lock terms_write_lock{terms_mutex_};
  return terms_.erase(term) > 0U;
}

}  // namespace masking_functions
