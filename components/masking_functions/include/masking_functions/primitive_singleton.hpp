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

#ifndef MASKING_FUNCTIONS_PRIMITIVE_SINGLETON_HPP
#define MASKING_FUNCTIONS_PRIMITIVE_SINGLETON_HPP

#include <type_traits>

namespace masking_functions {

// An auxiliary class template which allows to make a primitive singleton
// from an arbitrary type.
template <typename T>
struct primitive_singleton {
  using instance_type = T;
  static instance_type &instance() noexcept(
      std::is_nothrow_default_constructible_v<instance_type>) {
    static instance_type object;
    return object;
  }
};

}  // namespace masking_functions

#endif  // MASKING_FUNCTIONS_PRIMITIVE_SINGLETON_HPP
