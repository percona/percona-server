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

#ifndef OPENSSLPP_TYPED_ACCESSOR_HPP
#define OPENSSLPP_TYPED_ACCESSOR_HPP

#include "opensslpp/accessor.hpp"

namespace opensslpp {

template <typename WrapperType, typename UnderlyingType>
class typed_accessor : private accessor<WrapperType> {
 private:
  using this_type = typed_accessor<WrapperType, UnderlyingType>;
  using base_type = accessor<WrapperType>;

 public:
  static UnderlyingType *get_impl(WrapperType &obj) noexcept {
    return static_cast<UnderlyingType *>(base_type::get_impl(obj));
  }
  static const UnderlyingType *get_impl(const WrapperType &obj) noexcept {
    return static_cast<const UnderlyingType *>(base_type::get_impl(obj));
  }
  static UnderlyingType *get_impl_const_casted(
      const WrapperType &obj) noexcept {
    return const_cast<UnderlyingType *>(this_type::get_impl(obj));
  }
  static void set_impl(WrapperType &obj, UnderlyingType *impl_raw) noexcept {
    base_type::set_impl(obj, impl_raw);
  }
  static void *release(WrapperType &obj) noexcept {
    return static_cast<UnderlyingType *>(base_type::release(obj));
  }
};

}  // namespace opensslpp

#endif
