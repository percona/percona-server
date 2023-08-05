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

#ifndef MASKING_FUNCTIONS_SQL_ESCAPE_FUNCTIONS_HPP
#define MASKING_FUNCTIONS_SQL_ESCAPE_FUNCTIONS_HPP

#include <string>

#include "masking_functions/charset_string_fwd.hpp"

namespace masking_functions {

// This function converts the specified 'charset_string' 'cs_str', which can
// have any character set, to 'utf8mb4' and then escape special characters so
// that this value can be safely used inside MySQL string literals (to prevent
// SQL injections).
//
// This function is only used inside `sql_builder` class implementation and
// should have been made a private helper function of this class. However,
// as MySQL still does provide this functionality via 'mysql_query_xxx'
// services, we had to use one of the `mysys` functions directly, which
// violates MySQL component building recommendations.
// In order to isolate this function from other "conformant" code, it is
// put in a separate translation unit.
std::string escape_string(const charset_string &cs_str);

}  // namespace masking_functions

#endif  // MASKING_FUNCTIONS_SQL_ESCAPE_FUNCTIONS_HPP
