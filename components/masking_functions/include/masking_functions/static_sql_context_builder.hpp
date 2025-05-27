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

#ifndef MASKING_FUNCTIONS_STATIC_SQL_CONTEXT_BUILDER_HPP
#define MASKING_FUNCTIONS_STATIC_SQL_CONTEXT_BUILDER_HPP

#include "masking_functions/abstract_sql_context_builder.hpp"  // IWYU pragma: export

#include "masking_functions/command_service_tuple_fwd.hpp"

namespace masking_functions {

class static_sql_context_builder : public abstract_sql_context_builder {
 public:
  explicit static_sql_context_builder(const command_service_tuple &services);
  static_sql_context_builder(const static_sql_context_builder &) = delete;
  static_sql_context_builder &operator=(const static_sql_context_builder &) =
      delete;
  static_sql_context_builder(static_sql_context_builder &&) = delete;
  static_sql_context_builder &operator=(static_sql_context_builder &&) = delete;
  ~static_sql_context_builder() override;

 private:
  sql_context_ptr static_instance_;

  sql_context_ptr do_build() const override;
};

}  // namespace masking_functions

#endif  // MASKING_FUNCTIONS_STATIC_SQL_CONTEXT_BUILDER_HPP
