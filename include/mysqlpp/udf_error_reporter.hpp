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

#ifndef MYSQLPP_UDF_ERROR_REPORTER_HPP
#define MYSQLPP_UDF_ERROR_REPORTER_HPP

#include <my_inttypes.h>

namespace mysqlpp {

// A singleton class that is used to customize error reporting behavior
// of the UDF wrappers framework.
// User code must ensure that 'udf_error_reporter::instance()' is initialized
// before any UDF reporting an error is called.
//
// It can be set to the default implementation from 'mysys'
// mysqlpp::udf_error_reporter::instance() = &my_error;
//
// or (in case of components) to a custom fuction that uses
// 'mysql_service_mysql_runtime_error' service
// static void custom_my_error(int error_id, myf flags, ...) {
//   va_list args;
//   va_start(args, flags);
//   mysql_service_mysql_runtime_error->emit(error_id, flags, args);
//   va_end(args);
// }
// mysqlpp::udf_error_reporter::instance() = &custom_my_error;
class udf_error_reporter {
 public:
  using function_type = void (*)(int, myf, ...);

  static function_type &instance() noexcept {
    static function_type function{nullptr};
    return function;
  }
};

}  // namespace mysqlpp

#endif
