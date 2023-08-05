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

#ifndef MASKING_FUNCTIONS_STRING_SERVICE_TUPLE_HPP
#define MASKING_FUNCTIONS_STRING_SERVICE_TUPLE_HPP

#include <mysql/components/service.h>
#include <mysql/components/services/mysql_string.h>

#include "masking_functions/string_service_tuple_fwd.hpp"

namespace masking_functions {

// A set of MySQL string manipulation services required to perform character
// set-aware string operations.
// This class is intended to be used for constructing instances of the
// 'charset_string' class.
// It is recommended to be used in a combination with the
// 'primitive_singleton' class template.
//
// primitive_singleton<string_service_tuple>::instance() =
//   string_service_tuple{
//     mysql_service_mysql_charset,
//     mysql_service_mysql_string_factory,
//     mysql_service_mysql_string_charset_converter,
//     mysql_service_mysql_string_get_data_in_charset,
//     mysql_service_mysql_string_append,
//     mysql_service_mysql_string_character_access,
//     mysql_service_mysql_string_byte_access,
//     mysql_service_mysql_string_reset,
//     mysql_service_mysql_string_substr,
//     mysql_service_mysql_string_compare};
//   };
// ...
// charset_string cs_str{
//   primitive_singleton<string_service_tuple>::instance()
// };

struct string_service_tuple {
  SERVICE_TYPE(mysql_charset) * charset;
  SERVICE_TYPE(mysql_string_factory) * factory;
  SERVICE_TYPE(mysql_string_charset_converter) * converter;
  SERVICE_TYPE(mysql_string_get_data_in_charset) * get_data_in_charset;
  SERVICE_TYPE(mysql_string_append) * append;
  SERVICE_TYPE(mysql_string_character_access) * character_access;
  SERVICE_TYPE(mysql_string_byte_access) * byte_access;
  SERVICE_TYPE(mysql_string_reset) * reset;
  SERVICE_TYPE(mysql_string_substr) * substr;
  SERVICE_TYPE(mysql_string_compare) * compare;
};

}  // namespace masking_functions

#endif  // MASKING_FUNCTIONS_STRING_SERVICE_TUPLE_HPP
