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

#ifndef MYSQLPP_UDF_EXCEPTION_HPP
#define MYSQLPP_UDF_EXCEPTION_HPP

#include <stdexcept>
#include <string>

namespace mysqlpp {

class udf_exception : public std::runtime_error {
 private:
  using error_code_t = int;
  static constexpr error_code_t error_code_sentinel = ~error_code_t{};

 public:
  explicit udf_exception(const std::string &what,
                         error_code_t error_code = error_code_sentinel)
      : std::runtime_error{what}, error_code_{error_code} {}

  bool has_error_code() const noexcept {
    return error_code_ != error_code_sentinel;
  }
  error_code_t get_error_code() const noexcept { return error_code_; }

 private:
  error_code_t error_code_;
};

}  // namespace mysqlpp

#endif
