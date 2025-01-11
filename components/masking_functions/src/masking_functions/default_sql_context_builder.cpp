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

#include "masking_functions/default_sql_context_builder.hpp"

#include <memory>

#include "masking_functions/sql_context.hpp"

namespace masking_functions {

sql_context_ptr default_sql_context_builder::do_build() const {
  return std::make_shared<sql_context>(
      get_services(), sql_context_registry_access::locking,
      sql_context_registry_access::locking, false);
}

}  // namespace masking_functions
