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

#ifndef MASKING_FUNCTIONS_ABSTRACT_SQL_CONTEXT_BUILDER_HPP
#define MASKING_FUNCTIONS_ABSTRACT_SQL_CONTEXT_BUILDER_HPP

#include "masking_functions/abstract_sql_context_builder_fwd.hpp"  // IWYU pragma: export

#include "masking_functions/command_service_tuple_fwd.hpp"
#include "masking_functions/sql_context_fwd.hpp"

namespace masking_functions {

// This class is an abstract interface for an entity that is supposed to
// return ready-to-use instances of 'sql_context' class.
// This interface has two concrete implementations:
// 1. 'default_sql_context_builder' - creates an instance of 'sql_context'
//    class (establishes an internal connection) every time its 'build()'
//    method is called.
// 2. 'static_sql_context_builder' - creates an instance of 'sql_context'
//    class (establishes an internal connection) only once in the constructor
//    and returns a reference of this shared instance every time 'build()'
//    method is called.
//
//                       abstract_sql_context_builder
//                           ^               ^
//                           |               |
// default_sql_context_builder            static_sql_context_builder
class abstract_sql_context_builder {
 public:
  abstract_sql_context_builder(const abstract_sql_context_builder &) = delete;
  abstract_sql_context_builder &operator=(
      const abstract_sql_context_builder &) = delete;
  abstract_sql_context_builder(abstract_sql_context_builder &&) = delete;
  abstract_sql_context_builder &operator=(abstract_sql_context_builder &&) =
      delete;

  virtual ~abstract_sql_context_builder() = default;

  sql_context_ptr build() const { return do_build(); }

 protected:
  explicit abstract_sql_context_builder(const command_service_tuple &services)
      : services_{&services} {}

  const command_service_tuple &get_services() const noexcept {
    return *services_;
  }

 private:
  const command_service_tuple *services_;

  virtual sql_context_ptr do_build() const = 0;
};

}  // namespace masking_functions

#endif  // MASKING_FUNCTIONS_ABSTRACT_SQL_CONTEXT_BUILDER_HPP
