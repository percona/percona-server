/* Copyright (c) 2020 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; version 2 of
   the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#ifndef MYSQLPP_COMMON_TYPES_HPP
#define MYSQLPP_COMMON_TYPES_HPP

#include <cassert>
#include <string_view>

#include <mysql/udf_registration_types.h>

namespace mysqlpp {

using item_result_type = Item_result;
inline std::string_view get_item_result_label(
    item_result_type item_result) noexcept {
  switch (item_result) {
    case INVALID_RESULT:
      return {"invalid"};
    case STRING_RESULT:
      return {"string"};
    case REAL_RESULT:
      return {"real"};
    case INT_RESULT:
      return {"int"};
    case ROW_RESULT:
      return {"row"};
    case DECIMAL_RESULT:
      return {"decimal"};
    default:
      assert(0);
  }
  return {};
}

}  // namespace mysqlpp

#endif
