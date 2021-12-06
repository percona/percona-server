/* Copyright (c) 2022 Percona LLC and/or its affiliates. All rights reserved.

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

#ifndef OPENSSLPP_OPERATION_CANCELLLED_ERROR_HPP
#define OPENSSLPP_OPERATION_CANCELLLED_ERROR_HPP

#include <string>

#include <opensslpp/operation_cancelled_error_fwd.hpp>

#include <opensslpp/core_error.hpp>

namespace opensslpp {

class operation_cancelled_error : public core_error {
 public:
  using core_error::core_error;
};

}  // namespace opensslpp

#endif
