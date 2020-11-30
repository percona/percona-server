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

#ifndef MYSQLPP_UDF_TRAITS_HPP
#define MYSQLPP_UDF_TRAITS_HPP

#include <optional>
#include <string>
#include <string_view>

namespace mysqlpp {

class udf_context;

struct empty_mixin {};

template <typename T>
struct wrapped_t {
  T mixin;
};

template <typename MixinType, typename ImplType>
struct impl_with_mixin : public MixinType {
  impl_with_mixin(udf_context &ctx) : MixinType{}, impl{ctx} {}
  ImplType impl;
};

template <item_result_type ItemResult>
struct udf_traits;

template <>
struct udf_traits<STRING_RESULT> {
  using arg_type = std::string_view;
  using result_type = std::optional<std::string>;
  using mixin_type = wrapped_t<std::string>;
};
template <>
struct udf_traits<REAL_RESULT> {
  using arg_type = std::optional<double>;
  using result_type = std::optional<double>;
  using mixin_type = empty_mixin;
};
template <>
struct udf_traits<INT_RESULT> {
  using arg_type = std::optional<long long>;
  using result_type = std::optional<long long>;
  using mixin_type = empty_mixin;
};
template <>
struct udf_traits<DECIMAL_RESULT> {
  using arg_type = std::string_view;
  using result_type = std::optional<std::string>;
  using mixin_type = wrapped_t<std::string>;
};

template <item_result_type ItemResult>
using udf_arg_t = typename udf_traits<ItemResult>::arg_type;
template <item_result_type ItemResult>
using udf_result_t = typename udf_traits<ItemResult>::result_type;
template <item_result_type ItemResult>
using udf_mixin_t = typename udf_traits<ItemResult>::mixin_type;

}  // namespace mysqlpp

#endif
