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

#ifndef MASKING_FUNCTIONS_SQL_CONTEXT_FWD_HPP
#define MASKING_FUNCTIONS_SQL_CONTEXT_FWD_HPP

#include <cstdint>
#include <memory>

namespace masking_functions {

class sql_context;

using sql_context_ptr = std::shared_ptr<sql_context>;

enum class sql_context_registry_access : std::uint8_t { non_locking, locking };

}  // namespace masking_functions

#endif  // MASKING_FUNCTIONS_SQL_CONTEXT_FWD_HPP
