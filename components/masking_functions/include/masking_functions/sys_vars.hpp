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

#ifndef MASKING_FUNCTIONS_SYS_VARS_HPP
#define MASKING_FUNCTIONS_SYS_VARS_HPP

#include <cstdint>
#include <string>
#include <string_view>

namespace masking_functions {

std::string_view get_dict_database_name() noexcept;
std::uint64_t get_flush_interval_seconds() noexcept;

bool register_sys_vars();
bool unregister_sys_vars();
bool check_sys_vars(std::string &error_message);

}  // namespace masking_functions

#endif  // MASKING_FUNCTIONS_SYS_VARS_HPP
