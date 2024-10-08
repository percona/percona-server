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

#ifndef MASKING_FUNCTIONS_QUERY_CACHE_FWD_HPP
#define MASKING_FUNCTIONS_QUERY_CACHE_FWD_HPP

#include <memory>

namespace masking_functions {

class query_cache;

using query_cache_ptr = std::unique_ptr<query_cache>;

}  // namespace masking_functions

#endif  // MASKING_FUNCTIONS_QUERY_CACHE_FWD_HPP
