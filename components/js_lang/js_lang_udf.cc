/* Copyright (c) 2023, 2024 Percona LLC and/or its affiliates. All rights
   reserved.

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

/*
  This file contains implementation of auxiliary UDF functions which give
  users access to various information about connection and user JS context,
  and JS component as a whole.
*/

#include <mysqlpp/udf_context_charset_extension.hpp>
#include <mysqlpp/udf_registration.hpp>
#include <mysqlpp/udf_wrappers.hpp>

#include "js_lang_common.h"
#include "js_lang_core.h"

/** Collation to be used for return value for UDFs which return strings. */
static constexpr char UTF8_DEFAULT_COLLATION_NAME[] = "utf8mb4_0900_ai_ci";

/**
  Max result length for UDFs which return string values and for which we
  decided to return results of TEXT type (as opposed to TINYTEXT or
  MEDIUMTEXT). It's 2^16-1 (max TEXT length in bytes) divided by 4
  (max number of bytes per utf8mb4_* character).

  In practice, the returned value can exceed this length. Doing so should
  only cause problems when a table is created with column types based on
  UDF return type and then too long return value is stored in this table
  (including implicit temporary table case).
*/
static constexpr std::size_t MAX_TEXT_RESULT_LENGTH = ((1 << 16) - 1) / 4;

/**
  Implementation of JS_GET_LAST_ERROR() UDF for getting error message for the
  last JS error in the connection for the current user.
*/
class js_get_last_error_impl {
 public:
  js_get_last_error_impl(mysqlpp::udf_context &ctx) {
    /*
      TODO: Consider supporting retrieval of error info for different users/
            contexts. It might be a separate UDF. It also must do privilege
            checking (might require a new privilege for this).
    */
    if (ctx.get_number_of_args() != 0)
      throw std::invalid_argument{"Wrong argument list: should be ()"};

    ctx.mark_result_nullable(true);
    ctx.mark_result_const(false);

    /*
      Any reasonable error message should fit into TEXT field/64K bytes.
      While TINYTEXT with its 255-byte limit is probably too small for
      this purpose (e.g. MySQL error messages are limited to 512 bytes).

      In practice, UDF can return values exceeding this limit. Doing so,
      should only cause problems in scenarios when one table is created
      with column tyoes based on the type of UDF return value (including
      implicit temporary tables case).
    */
    ctx.set_result_max_length(MAX_TEXT_RESULT_LENGTH);

    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    charset_ext.set_return_value_collation(ctx, UTF8_DEFAULT_COLLATION_NAME);
  }
  mysqlpp::udf_result_t<STRING_RESULT> calculate(const mysqlpp::udf_context &) {
    auto auth_id_ctx = Js_thd::get_current_auth_id_context();

    // Handle the case when JS was never run for in this connection or
    // for the current user in this connection.
    if (auth_id_ctx == nullptr) return std::nullopt;

    return auth_id_ctx->get_last_error();
  }
};

/**
  Implementation of JS_GET_LAST_ERROR_INFO() UDF for getting extended
  information about the last JS error in the connection for the current user.
*/
class js_get_last_error_info_impl {
 public:
  js_get_last_error_info_impl(mysqlpp::udf_context &ctx) {
    // TODO: Consider supporting retrieval of error info for different users/
    //       contexts for this function as well.
    if (ctx.get_number_of_args() != 0)
      throw std::invalid_argument{"Wrong argument list: should be ()"};

    ctx.mark_result_nullable(true);
    ctx.mark_result_const(false);

    /*
      Extended information about error consists of error message, information
      about problematic code line and possibly stacktrace (up to 10 frames
      by default). So it is very unlikely that it won't fit into TEXT field/
      64K bytes.

      In practice, returning value exceeding this limit from the UDF should
      work fine in most cases. The only problematic scenarios involve creation
      of table with column types based on UDF return type and trying to store
      the result in it.
    */
    ctx.set_result_max_length(MAX_TEXT_RESULT_LENGTH);

    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    charset_ext.set_return_value_collation(ctx, UTF8_DEFAULT_COLLATION_NAME);
  }
  mysqlpp::udf_result_t<STRING_RESULT> calculate(const mysqlpp::udf_context &) {
    auto auth_id_ctx = Js_thd::get_current_auth_id_context();

    // Handle the case when JS was never run for in this connection or
    // for the current user in this connection.
    if (auth_id_ctx == nullptr) return std::nullopt;

    return auth_id_ctx->get_last_error_info();
  }
};

/**
  Implementation of JS_CLEAR_LAST_ERROR() UDF for clearing information about
  last JS error for the connection and the current user pair (resets it to a
  state as if no error has happened).
*/
class js_clear_last_error_impl {
 public:
  js_clear_last_error_impl(mysqlpp::udf_context &ctx) {
    // TODO: Consider supporting clearing of error info for different users/
    //       contexts for this function as well.
    if (ctx.get_number_of_args() != 0)
      throw std::invalid_argument{"Wrong argument list: should be ()"};

    ctx.mark_result_nullable(false);
    ctx.mark_result_const(false);
  }
  mysqlpp::udf_result_t<INT_RESULT> calculate(const mysqlpp::udf_context &) {
    auto auth_id_ctx = Js_thd::get_current_auth_id_context();

    // Handle the case when JS was never run for in this connection or
    // for the current user in this connection.
    if (auth_id_ctx == nullptr) return 0;

    return auth_id_ctx->clear_last_error();
  }
};

DECLARE_STRING_UDF_AUTO(js_get_last_error)
DECLARE_STRING_UDF_AUTO(js_get_last_error_info)
DECLARE_INT_UDF_AUTO(js_clear_last_error)

static std::array udfs{DECLARE_UDF_INFO_AUTO(js_get_last_error),
                       DECLARE_UDF_INFO_AUTO(js_get_last_error_info),
                       DECLARE_UDF_INFO_AUTO(js_clear_last_error)};
using udf_bitset_type = mysqlpp::udf_bitset<std::tuple_size_v<decltype(udfs)>>;
static udf_bitset_type registered_udfs;

bool register_udfs() {
  mysqlpp::register_udfs(mysql_service_udf_registration, udfs, registered_udfs);

  if (!registered_udfs.all()) {
    my_error(ER_LANGUAGE_COMPONENT, MYF(0),
             "Can't register auxiliary UDFs for " CURRENT_COMPONENT_NAME_STR
             "component.");
    return true;
  }

  return false;
}

bool unregister_udfs() {
  mysqlpp::unregister_udfs(mysql_service_udf_registration, udfs,
                           registered_udfs);

  if (!registered_udfs.none()) {
    my_error(ER_LANGUAGE_COMPONENT, MYF(0),
             "Can't unregister auxiliary UDFs for " CURRENT_COMPONENT_NAME_STR
             "component.");
    return true;
  }

  return false;
}
