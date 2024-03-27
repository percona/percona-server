/* Copyright (c) 2023 Percona LLC and/or its affiliates. All rights reserved.

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

#ifndef MASKING_FUNCTIONS_SQL_CONTEXT_HPP
#define MASKING_FUNCTIONS_SQL_CONTEXT_HPP

#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "masking_functions/command_service_tuple_fwd.hpp"
#include "masking_functions/dictionary_container.hpp"

namespace masking_functions {

// A wrapper class that uses MySQL connection services under the hood and
// simplifies query execution operations.
// It requires an instance of the 'command_service_tuple' class for
// construction.
class sql_context {
 public:
  explicit sql_context(const command_service_tuple &services);

  sql_context(sql_context const &) = delete;
  sql_context(sql_context &&) noexcept = default;

  sql_context &operator=(sql_context const &) = delete;
  sql_context &operator=(sql_context &&) noexcept = default;

  ~sql_context() = default;

  const command_service_tuple &get_services() const noexcept {
    return *impl_.get_deleter().services;
  }

  optional_dictionary_container query_list(std::string_view query);

  bool execute(std::string_view query);

 private:
  struct deleter {
    void operator()(void *ptr) const noexcept;
    const command_service_tuple *services;
  };
  using impl_type = std::unique_ptr<void, deleter>;
  impl_type impl_;
};

}  // namespace masking_functions

#endif  // MASKING_FUNCTIONS_SQL_CONTEXT_HPP
