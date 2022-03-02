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

#include <stdexcept>
#include <string>

#include <mysqlpp/udf_wrappers.hpp>

namespace {

class wrapped_udf_string_impl {
 public:
  wrapped_udf_string_impl(mysqlpp::udf_context &ctx) {
    if (ctx.get_number_of_args() == 2)
      throw mysqlpp::udf_exception("test udf_exception with sentinel");
    if (ctx.get_number_of_args() == 3)
      throw mysqlpp::udf_exception("test udf_exception without sentinel",
                                   ER_WRAPPED_UDF_EXCEPTION);
    if (ctx.get_number_of_args() == 4) throw 42;

    if (ctx.get_number_of_args() != 1)
      throw std::invalid_argument("function requires exactly one argument");
    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);
    ctx.mark_arg_nullable(0, true);
    ctx.set_arg_type(0, STRING_RESULT);
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &ctx) {
    auto arg_sv = ctx.get_arg<STRING_RESULT>(0);
    if (arg_sv.data() == nullptr) return {};
    if (arg_sv == "100") {
      my_error(ER_DA_OOM, MYF(0));
      throw mysqlpp::udf_exception("test udf_exception with sentinel");
    }
    if (arg_sv == "101")
      throw mysqlpp::udf_exception("test udf_exception without sentinel",
                                   ER_WRAPPED_UDF_EXCEPTION);
    if (arg_sv == "102") throw std::runtime_error("test runtime_error");
    if (arg_sv == "103") throw 42;

    std::string result;
    result += '{';
    result.append(arg_sv.data(), arg_sv.size());
    result += '}';

    return {result};
  }
};

class wrapped_udf_real_impl {
 public:
  wrapped_udf_real_impl(mysqlpp::udf_context &ctx) {
    if (ctx.get_number_of_args() != 1)
      throw std::invalid_argument("function requires exactly one argument");
    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);
    ctx.set_result_decimals_not_fixed();
    ctx.mark_arg_nullable(0, true);
    ctx.set_arg_type(0, REAL_RESULT);
  }

  mysqlpp::udf_result_t<REAL_RESULT> calculate(
      const mysqlpp::udf_context &ctx) {
    auto arg_opt = ctx.get_arg<REAL_RESULT>(0);
    if (!arg_opt) return {};

    if (arg_opt.get() == 100.0) {
      my_error(ER_DA_OOM, MYF(0));
      throw mysqlpp::udf_exception("test udf_exception with sentinel");
    }
    if (arg_opt.get() == 101.0)
      throw mysqlpp::udf_exception("test udf_exception without sentinel",
                                   ER_WRAPPED_UDF_EXCEPTION);
    if (arg_opt.get() == 102.0) throw std::runtime_error("test runtime_error");
    if (arg_opt.get() == 103.0) throw 42;

    return arg_opt.get() + 0.25;
  }
};

class wrapped_udf_int_impl {
 public:
  wrapped_udf_int_impl(mysqlpp::udf_context &ctx) {
    if (ctx.get_number_of_args() != 1)
      throw std::invalid_argument("function requires exactly one argument");
    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);
    ctx.mark_arg_nullable(0, true);
    ctx.set_arg_type(0, INT_RESULT);
  }

  mysqlpp::udf_result_t<INT_RESULT> calculate(const mysqlpp::udf_context &ctx) {
    auto arg_opt = ctx.get_arg<INT_RESULT>(0);
    if (!arg_opt) return {};

    if (arg_opt.get() == 100) {
      my_error(ER_DA_OOM, MYF(0));
      throw mysqlpp::udf_exception("test udf_exception with sentinel");
    }
    if (arg_opt.get() == 101)
      throw mysqlpp::udf_exception("test udf_exception without sentinel",
                                   ER_WRAPPED_UDF_EXCEPTION);
    if (arg_opt.get() == 102) throw std::runtime_error("test runtime_error");
    if (arg_opt.get() == 103) throw 42;

    return arg_opt.get() + 100;
  }
};

}  // end of anonymous namespace

DECLARE_STRING_UDF(wrapped_udf_string_impl, wrapped_udf_string)
DECLARE_REAL_UDF(wrapped_udf_real_impl, wrapped_udf_real)
DECLARE_INT_UDF(wrapped_udf_int_impl, wrapped_udf_int)
