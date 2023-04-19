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

#ifndef OPENSSLPP_ACCESSOR_HPP
#define OPENSSLPP_ACCESSOR_HPP

#include <opensslpp/accessor_fwd.hpp>

namespace opensslpp {

template <typename WrapperType>
class accessor {
 protected:
  static void *get_impl(WrapperType &obj) noexcept { return obj.impl_.get(); }
  static const void *get_impl(const WrapperType &obj) noexcept {
    return obj.impl_.get();
  }
  static void set_impl(WrapperType &obj, void *impl_raw) noexcept {
    obj.impl_.reset(impl_raw);
  }
  static void *release(WrapperType &obj) noexcept {
    return obj.impl_.release();
  }
};

}  // namespace opensslpp

#endif
