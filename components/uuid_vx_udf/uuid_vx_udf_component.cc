/* Copyright (c) 2024, Percona LLC and/or its affiliates. All rights reserved.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2.0,
as published by the Free Software Foundation.

This program is also distributed with certain software (including
but not limited to OpenSSL) that is licensed under separate terms,
as designated in a particular file or component or in included license
documentation.  The authors of MySQL hereby grant you an additional
permission to link the program and your derivative works with the
separately licensed software that they have included with MySQL.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License, version 2.0, for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/******************************************************************************/
/* Implementation of UUID by RFC 9562 https://www.rfc-editor.org/rfc/rfc9562  */
/* using Boost uuid library (header-only) version > 1.86                      */
/******************************************************************************/

#include <mysql/components/component_implementation.h>
#include <mysql/components/services/component_sys_var_service.h>
#include <mysql/components/services/mysql_current_thread_reader.h>
#include <mysql/components/services/mysql_runtime_error.h>
#include <mysql/components/services/udf_metadata.h>
#include <mysql/components/services/udf_registration.h>

#include <algorithm>
#include <array>
#include <boost/preprocessor/stringize.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <ctime>
#include <iomanip>
#include <mysqlpp/udf_context_charset_extension.hpp>
#include <mysqlpp/udf_registration.hpp>
#include <mysqlpp/udf_wrappers.hpp>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>

// defined as a macro because needed both raw and stringized
#define CURRENT_COMPONENT_NAME uuid_vx_udf
#define CURRENT_COMPONENT_NAME_STR BOOST_PP_STRINGIZE(CURRENT_COMPONENT_NAME)

REQUIRES_SERVICE_PLACEHOLDER(udf_registration);
REQUIRES_SERVICE_PLACEHOLDER(mysql_udf_metadata);
REQUIRES_SERVICE_PLACEHOLDER(mysql_runtime_error);

namespace {

// declaring the following two constants as char arrays (rather than
// std::string_view) because they are used in c-style interfaces and need
// to be null terminated
constexpr char string_charset[]{"utf8mb4"};
constexpr char uuid_charset[]{"ascii"};

constexpr std::string_view nil_uuid{"00000000-0000-0000-0000-000000000000"};
// TODO: should we convert this to lower-case to match with generated values
constexpr std::string_view max_uuid{"FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF"};

constexpr std::string_view err_msg_one_argument{
    "Function requires exactly one argument."};
constexpr std::string_view err_msg_valid_uuid{
    "Invalid argument. Should be a valid UUID string."};
constexpr std::string_view err_msg_valid_v167_uuid{
    "Invalid argument. Should be a valid UUID of version 1,6,7 in a string "
    "representation."};
constexpr std::string_view err_msg_no_arguments{
    "Function requires no arguments."};
constexpr std::string_view err_msg_one_or_two_arguments{
    "Function requires one or two arguments."};
constexpr std::string_view err_msg_zero_or_one_argument{
    "Function requires zero or one arguments."};
constexpr std::string_view err_msg_uuid_namespace_idx{
    "UUID namespace index must be in the 0..3 range."};
constexpr std::string_view err_msg_16bytes{
    "The string should be exactly 32 hex characters (16 bytes)."};

template <typename Exception>
[[noreturn]] void raise(std::string_view err_msg) {
  throw Exception{std::string{err_msg}};
}

constexpr std::size_t uuid_size_in_bytes = boost::uuids::uuid::static_size();

/**
 * Implementation of UUID_VX_VERSION() function.
 * The function takes exactly one string argument. The argument must be a valid
 * UUID in formatted or hexadecimal form. In other cases the function throws an
 * error.
 * - If the argument is not a valid UUID string, function throws an error.
 * - If the argument is NULL, the function returns NULL.
 * - If the argument is a valid UUID string but has an unknown (out of 1..8
 * range) value in the 'version' field, the function returns -1.
 */
class uuid_vx_version_impl {
 public:
  uuid_vx_version_impl(mysqlpp::udf_context &ctx) {
    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);
    if (ctx.get_number_of_args() != 1) {
      raise<std::invalid_argument>(err_msg_one_argument);
    }
    // arg0 - @uuid_string
    ctx.mark_arg_nullable(0, true);
    ctx.set_arg_type(0, STRING_RESULT);
    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    charset_ext.set_arg_value_charset(ctx, 0, uuid_charset);
  }

  mysqlpp::udf_result_t<INT_RESULT> calculate(const mysqlpp::udf_context &ctx) {
    if (ctx.is_arg_null(0)) {
      return {};
    }

    boost::uuids::uuid::version_type version{
        boost::uuids::uuid::version_unknown};

    try {
      boost::uuids::string_generator gen;
      auto uxs = ctx.get_arg<STRING_RESULT>(0);
      boost::uuids::uuid u = gen(uxs.data());
      version = u.version();
    } catch (const std::exception &) {
      raise<std::invalid_argument>(err_msg_valid_uuid);
    }
    return static_cast<std::underlying_type_t<decltype(version)>>(version);
  }
};

/**
 * Implementation of UUID_VX_VARIANT() function.
 * The function takes exactly one string argument. The argument must be a valid
 * UUID in formatted or hexadecimal form. In other cases the function throws an
 * error.
 * - If the argument is not a valid UUID string, the function throws an error.
 * - If the argument is NULL, the function returns NULL.
 * - Returns 0 for "NCS backward compatibility"
 * - Returns 1 for "RFC 4122"
 * - Returns 2 for "Microsoft Corporation backward compatibility"
 * - Returns 3 for future definitions
 */
class uuid_vx_variant_impl {
 public:
  uuid_vx_variant_impl(mysqlpp::udf_context &ctx) {
    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);
    if (ctx.get_number_of_args() != 1) {
      raise<std::invalid_argument>(err_msg_one_argument);
    }
    // arg0 - @uuid_string
    ctx.mark_arg_nullable(0, true);
    ctx.set_arg_type(0, STRING_RESULT);
    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    charset_ext.set_arg_value_charset(ctx, 0, uuid_charset);
  }

  mysqlpp::udf_result_t<INT_RESULT> calculate(const mysqlpp::udf_context &ctx) {
    if (ctx.is_arg_null(0)) {
      return {};
    }

    boost::uuids::uuid::variant_type variant{
        boost::uuids::uuid::variant_future};
    try {
      boost::uuids::string_generator gen;
      auto uxs = ctx.get_arg<STRING_RESULT>(0);
      boost::uuids::uuid u = gen(uxs.data());
      variant = u.variant();
    } catch (const std::exception &) {
      raise<std::invalid_argument>(err_msg_valid_uuid);
    }
    return variant;
  }
};

/**
 * Implementation of IS_UUID_VX() function.
 * The function takes exactly one string argument. The argument must be a valid
 * UUID in formatted or hexadecimal form. In other cases the function throws an
 * error.
 * - If the argument is NULL, the function returns NULL.
 * - If the argument can be parsed as a UUID of any version, the function
 * returns 1.
 * - If the argument can not be parsed as a UUID of any version, the function
 * returns 0.
 */
class is_uuid_vx_impl {
 public:
  is_uuid_vx_impl(mysqlpp::udf_context &ctx) {
    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);
    if (ctx.get_number_of_args() != 1) {
      raise<std::invalid_argument>(err_msg_one_argument);
    }
    // arg0 - @uuid_string
    ctx.mark_arg_nullable(0, true);
    ctx.set_arg_type(0, STRING_RESULT);
    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    charset_ext.set_arg_value_charset(ctx, 0, uuid_charset);
  }

  mysqlpp::udf_result_t<INT_RESULT> calculate(const mysqlpp::udf_context &ctx) {
    if (ctx.is_arg_null(0)) {
      return {};
    }

    bool verification_result = false;
    try {
      boost::uuids::string_generator gen;
      auto uxs = ctx.get_arg<STRING_RESULT>(0);
      gen(uxs.data());
      verification_result = true;
    } catch (std::exception &) {
    }
    return {verification_result ? 1LL : 0LL};
  }
};

/**
 * Implementation of IS_NIL_UUID_VX() function.
 * The function takes exactly one string argument. The argument must be a valid
 * UUID in formatted or hexadecimal form. In other cases the function throws an
 * error.
 * - If the argument is NULL, the function returns NULL.
 * - If the argument can be parsed as a UUID of any version, and the argument is
 * NIL UUID, the function returns 1.
 * - If the argument is a valid UUID but is not NIL UUID, the function returns
 * 0.
 */
class is_nil_uuid_vx_impl {
 public:
  is_nil_uuid_vx_impl(mysqlpp::udf_context &ctx) {
    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);
    if (ctx.get_number_of_args() != 1) {
      raise<std::invalid_argument>(err_msg_one_argument);
    }
    // arg0 - @uuid_string
    ctx.mark_arg_nullable(0, true);
    ctx.set_arg_type(0, STRING_RESULT);
    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    charset_ext.set_arg_value_charset(ctx, 0, uuid_charset);
  }

  mysqlpp::udf_result_t<INT_RESULT> calculate(const mysqlpp::udf_context &ctx) {
    if (ctx.is_arg_null(0)) {
      return {};
    }

    bool verification_result = false;
    try {
      boost::uuids::string_generator gen;
      auto uxs = ctx.get_arg<STRING_RESULT>(0);
      boost::uuids::uuid u = gen(uxs.data());
      verification_result = u.is_nil();
    } catch (std::exception &) {
      raise<std::invalid_argument>(err_msg_valid_uuid);
    }
    return {verification_result ? 1LL : 0LL};
  }
};

/**
 * Implementation of IS_MAX_UUID_VX() function.
 * The function takes exactly one string argument. The argument must be a valid
 * UUID in formatted or hexadecimal form. In other cases the function throws an
 * error.
 * - If the argument is NULL, the function returns NULL.
 * - If the argument can be parsed as a UUID of any version, and the argument is
 * MAX UUID, the function returns 1.
 * - If the argument is a valid UUID but is not MAX UUID, the function returns
 * 0.
 */
class is_max_uuid_vx_impl {
 public:
  static const boost::uuids::uuid max_uuid;

  is_max_uuid_vx_impl(mysqlpp::udf_context &ctx) {
    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);
    if (ctx.get_number_of_args() != 1) {
      raise<std::invalid_argument>(err_msg_one_argument);
    }
    // arg0 - @uuid_string
    ctx.mark_arg_nullable(0, true);
    ctx.set_arg_type(0, STRING_RESULT);
    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    charset_ext.set_arg_value_charset(ctx, 0, uuid_charset);
  }

  mysqlpp::udf_result_t<INT_RESULT> calculate(const mysqlpp::udf_context &ctx) {
    if (ctx.is_arg_null(0)) {
      return {};
    }

    bool verification_result = true;
    try {
      boost::uuids::string_generator gen;
      auto uxs = ctx.get_arg<STRING_RESULT>(0);
      boost::uuids::uuid u = gen(uxs.data());
      verification_result = (u == max_uuid);
    } catch (std::exception &) {
      raise<std::invalid_argument>(err_msg_valid_uuid);
    }

    return {verification_result ? 1LL : 0LL};
  }
};

const boost::uuids::uuid is_max_uuid_vx_impl::max_uuid{
    {0xffU, 0xffU, 0xffU, 0xffU, 0xffU, 0xffU, 0xffU, 0xffU, 0xffU, 0xffU,
     0xffU, 0xffU, 0xffU, 0xffU, 0xffU, 0xffU}};

/**
 * Implementation of UUID_V1() function.
 * The function takes no arguments. It generates a timestamp-based UUID
 * of version 1.
 */
class uuid_v1_impl {
 public:
  uuid_v1_impl(mysqlpp::udf_context &ctx) {
    ctx.mark_result_const(false);
    ctx.mark_result_nullable(false);
    if (ctx.get_number_of_args() != 0) {
      raise<std::invalid_argument>(err_msg_no_arguments);
    }
    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    charset_ext.set_return_value_charset(ctx, uuid_charset);
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      [[maybe_unused]] const mysqlpp::udf_context &ctx) {
    static thread_local boost::uuids::time_generator_v1 gen_v1;
    return boost::uuids::to_string(gen_v1());
  }
};

/**
 * Helper class for string-based uuids
 */
class string_based_uuid {
 public:
  inline boost::uuids::uuid get_uuid_namespace(int name_index) {
    boost::uuids::uuid ns;
    switch (name_index) {
      case 0:
        ns = boost::uuids::ns::dns();
        break;
      case 1:
        ns = boost::uuids::ns::url();
        break;
      case 2:
        ns = boost::uuids::ns::oid();
        break;
      case 3:
        ns = boost::uuids::ns::x500dn();
        break;  // we have 4 NS in the standard by now. In any other case we
                // just use url()
      default:
        ns = boost::uuids::ns::url();
    }
    return ns;
  }
};

/**
 * Implementation of UUID_V3() function.
 * The function takes 1 or 2 arguments. It generates string
 * based UUID of version 3. The first argument is string to hash with
 * MD5 algorithm. The second optional argument is name space for UUID.
 * DNS: 0, URL: 1, OID: 2, X.500: 3, default is 1, or URL
 */
class uuid_v3_impl : string_based_uuid {
 public:
  uuid_v3_impl(mysqlpp::udf_context &ctx) {
    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);

    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    charset_ext.set_return_value_charset(ctx, uuid_charset);

    size_t narg = ctx.get_number_of_args();
    if (narg > 0 && narg < 3) {
      // arg0 - @uuid string
      ctx.mark_arg_nullable(0, true);
      ctx.set_arg_type(0, STRING_RESULT);
      charset_ext.set_arg_value_charset(ctx, 0, string_charset);
      if (narg == 2) {
        ctx.mark_arg_nullable(1, false);
        ctx.set_arg_type(1, INT_RESULT);
      }
    } else {
      raise<std::invalid_argument>(err_msg_one_or_two_arguments);
    }
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &ctx) {
    int ns_index = 1;
    if (ctx.is_arg_null(0)) {
      return {};
    }
    auto the_string = ctx.get_arg<STRING_RESULT>(0);
    if (ctx.get_number_of_args() > 1) {
      auto ns = ctx.get_arg<INT_RESULT>(1);
      ns_index = ns.value_or(1);
      if (ns_index < 0 || ns_index > 3) {
        raise<std::invalid_argument>(err_msg_uuid_namespace_idx);
      }
    }
    boost::uuids::name_generator_md5 gen_v3(get_uuid_namespace(ns_index));
    std::string s(the_string);
    return boost::uuids::to_string(gen_v3(s));
  }
};

/**
 * Implementation of UUID_V4() function.
 * The function no arguments. It generates random number
 * based UUID of version 4.
 */
class uuid_v4_impl {
 public:
  uuid_v4_impl(mysqlpp::udf_context &ctx) {
    ctx.mark_result_const(false);
    ctx.mark_result_nullable(false);
    if (ctx.get_number_of_args() > 0) {
      raise<std::invalid_argument>(err_msg_no_arguments);
    }

    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    charset_ext.set_return_value_charset(ctx, uuid_charset);
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      [[maybe_unused]] const mysqlpp::udf_context &ctx) {
    boost::uuids::random_generator gen_v4;
    return boost::uuids::to_string(gen_v4());
  }
};

/**
 * Implementation of UUID_V5() function.
 * The function takes 1 or 2 arguments. It generates string
 * based UUID of version 5. The first argument is string to hash with
 * SHA1 algorithm. The second optional argument is name space for UUID.
 * DNS: 0, URL: 1, OID: 2, X.500: 3, default is 1, or URL
 */
class uuid_v5_impl : string_based_uuid {
 public:
  uuid_v5_impl(mysqlpp::udf_context &ctx) {
    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);

    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    charset_ext.set_return_value_charset(ctx, uuid_charset);

    size_t narg = ctx.get_number_of_args();
    if (narg > 0 && narg < 3) {
      // arg0 - @uuid string
      ctx.mark_arg_nullable(0, true);
      ctx.set_arg_type(0, STRING_RESULT);
      charset_ext.set_arg_value_charset(ctx, 0, string_charset);
      if (narg == 2) {
        ctx.mark_arg_nullable(1, false);
        ctx.set_arg_type(1, INT_RESULT);
      }
    } else {
      raise<std::invalid_argument>(err_msg_one_or_two_arguments);
    }
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &ctx) {
    int ns_index = 1;
    if (ctx.is_arg_null(0)) {
      return {};
    }
    auto the_string = ctx.get_arg<STRING_RESULT>(0);
    // static thread_local
    if (ctx.get_number_of_args() == 2) {
      auto ns = ctx.get_arg<INT_RESULT>(1);
      ns_index = ns.value_or(1);
      if (ns_index < 0 || ns_index > 3) {
        raise<std::invalid_argument>(err_msg_uuid_namespace_idx);
      }
    }
    boost::uuids::name_generator_sha1 gen_v5(get_uuid_namespace(ns_index));
    std::string s(the_string);
    return boost::uuids::to_string(gen_v5(s));
  }
};

/**
 * Implementation of UUID_V6() function.
 * The function takes no arguments. It generates time stamp
 * based UUID of version 6.
 */
class uuid_v6_impl {
 public:
  uuid_v6_impl(mysqlpp::udf_context &ctx) {
    ctx.mark_result_const(false);
    ctx.mark_result_nullable(false);
    if (ctx.get_number_of_args() != 0) {
      raise<std::invalid_argument>(err_msg_no_arguments);
    }
    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    charset_ext.set_return_value_charset(ctx, uuid_charset);
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      [[maybe_unused]] const mysqlpp::udf_context &ctx) {
    static thread_local boost::uuids::time_generator_v6 gen_v6;
    return boost::uuids::to_string(gen_v6());
  }
};

/**
 * Implementation of UUID_V7() function.
 * The function takes no arguments or one optional integer argument.
 * It generates time stamp based UUID of version 7.
 * If the argument is present, it is interpreted as time shift. Function
 * generated UUID and then shifts it's timestamp for specified number of
 * miliseconds, forth (positive) or back (negative) in time.
 */
class uuid_v7_impl {
 public:
  uuid_v7_impl(mysqlpp::udf_context &ctx) {
    ctx.mark_result_const(false);
    ctx.mark_result_nullable(false);
    size_t narg = ctx.get_number_of_args();
    // arg0 - @time_offset_ms, optional
    if (narg > 1) {
      raise<std::invalid_argument>(err_msg_zero_or_one_argument);
    }
    if (narg == 1) {
      ctx.mark_arg_nullable(0, false);
      ctx.set_arg_type(0, INT_RESULT);
    }

    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    charset_ext.set_return_value_charset(ctx, uuid_charset);
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &ctx) {
    static thread_local boost::uuids::time_generator_v7 gen_v7;
    auto generated = gen_v7();
    long offset = 0;
    if (ctx.get_number_of_args() == 1) {
      auto offs = ctx.get_arg<INT_RESULT>(0);
      offset = offs.value_or(0);
    }
    if (offset != 0) {
      generated = add_ts_offset(generated, offset);
    }
    return boost::uuids::to_string(generated);
  }

 private:
  /**
   * This function just shifts timestamp (ms part only) for uuid version 7.
   * ofs_ms argument is of type "long" so it will not cause integer overflow.
   * @return time-shifted UUID v7
   */
  boost::uuids::uuid add_ts_offset(boost::uuids::uuid u, long ofs_ms) {
    std::uint64_t time_ms = u.time_point_v7().time_since_epoch().count();
    time_ms += ofs_ms;
    uint16_t d6tmp = u.data[6];  // ver and rand part
    uint8_t d7tmp = u.data[7];   // rand part
    std::uint64_t timestamp_and_others = (time_ms << 16) | (d6tmp << 8) | d7tmp;
    boost::uuids::detail::store_big_u64(u.data + 0, timestamp_and_others);
    return u;
  }
};

/**
 * Implementation of NIL_UUID_VX() function.
 * The function takes no arguments.
 * It generates time NIL UUID.
 */
class nil_uuid_vx_impl {
 public:
  nil_uuid_vx_impl(mysqlpp::udf_context &ctx) {
    ctx.mark_result_const(true);
    ctx.mark_result_nullable(false);
    if (ctx.get_number_of_args() != 0) {
      raise<std::invalid_argument>(err_msg_no_arguments);
    }
    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    charset_ext.set_return_value_charset(ctx, uuid_charset);
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      [[maybe_unused]] const mysqlpp::udf_context &ctx) {
    return std::string{nil_uuid};
  }
};

/**
 * Implementation of MAX_UUID_VX() function.
 * The function takes no arguments.
 * It generates time MAX UUID.
 */
class max_uuid_vx_impl {
 public:
  max_uuid_vx_impl(mysqlpp::udf_context &ctx) {
    ctx.mark_result_const(true);
    ctx.mark_result_nullable(false);
    if (ctx.get_number_of_args() != 0) {
      raise<std::invalid_argument>(err_msg_no_arguments);
    }
    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    charset_ext.set_return_value_charset(ctx, uuid_charset);
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      [[maybe_unused]] const mysqlpp::udf_context &ctx) {
    return std::string{max_uuid};
  }
};

/**
 * Implementation of UUID_VX_TO_BIN() function.
 * The function takes exactly one string argument. The argument must be a valid
 * UUID in formatted or hexadecimal form. In other case function throws error.
 * If the argument can be parsed as UUID of any version, the function returns
 * it's binary representation.
 */
class uuid_vx_to_bin_impl {
 public:
  uuid_vx_to_bin_impl(mysqlpp::udf_context &ctx) {
    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);
    if (ctx.get_number_of_args() != 1) {
      raise<std::invalid_argument>(err_msg_one_argument);
    }
    // arg0 - @uuid_string
    ctx.mark_arg_nullable(0, true);
    ctx.set_arg_type(0, STRING_RESULT);
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &ctx) {
    boost::uuids::string_generator gen;
    auto uxs = ctx.get_arg<STRING_RESULT>(0);
    if (ctx.is_arg_null(0)) {
      return {};
    }
    char result[uuid_size_in_bytes + 1];
    boost::uuids::uuid u;
    try {
      u = gen(uxs.data());
      for (size_t i = 0; i < uuid_size_in_bytes; i++) {
        result[i] = (uint8_t)u.data[i];
      }
      result[uuid_size_in_bytes] = 0;
    } catch (std::exception &ex) {
      raise<std::invalid_argument>(err_msg_valid_uuid);
    }

    return result;
  }
};

/**
 * Implementation of BIN_TO_UUID_VX() function.
 * The function takes exactly one string argument. The argument must be a
 * hexadecimal of exactly 32 chars (16 bytes). In other case function throws
 * error. The function returns UUID with binary data from the argument.
 */
class bin_to_uuid_vx_impl {
 public:
  bin_to_uuid_vx_impl(mysqlpp::udf_context &ctx) {
    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);
    if (ctx.get_number_of_args() != 1) {
      raise<std::invalid_argument>(err_msg_one_argument);
    }
    // arg0 - @uuid_version
    ctx.mark_arg_nullable(0, true);
    ctx.set_arg_type(0, STRING_RESULT);

    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    charset_ext.set_return_value_charset(ctx, uuid_charset);
    charset_ext.set_arg_value_charset(ctx, 0, uuid_charset);
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &ctx) {
    if (ctx.is_arg_null(0)) {
      return {};
    }
    auto ubs = ctx.get_arg<STRING_RESULT>(0);
    if (ubs.size() != uuid_size_in_bytes) {
      raise<std::invalid_argument>(err_msg_16bytes);
    }

    boost::uuids::uuid u;
    for (size_t i = 0; i < uuid_size_in_bytes; i++) {
      u.data[i] = ubs.at(i);
    }

    return boost::uuids::to_string(u);
  }
};

/**
 *  Helper class for timestamp extracting functions
 */
class timestamp_based_uuid {
 public:
  /**
   * Returns time in ms since epoch
   */
  inline u_int64_t get_ts(std::string_view uxs) {
    boost::uuids::string_generator gen;
    boost::uuids::uuid u;
    long ms = 0;
    try {
      u = gen(uxs.data());
    } catch (std::exception &ex) {
      raise<std::invalid_argument>(err_msg_valid_v167_uuid);
    }
    switch (u.version()) {
      case 1:
        ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                 boost::uuids::uuid_clock::to_sys(u.time_point_v1())
                     .time_since_epoch())
                 .count();
        break;
      case 6:
        ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                 boost::uuids::uuid_clock::to_sys(u.time_point_v6())
                     .time_since_epoch())
                 .count();
        break;
        break;
      case 7:
        ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                 u.time_point_v7().time_since_epoch())
                 .count();
        break;
      default: {
        raise<std::invalid_argument>(err_msg_valid_v167_uuid);
      }
    }
    return ms;
  }

  /**
   * Returns formatted timestamp string from the unix time in milliseconds.
   * TZ is always GMT
   * @return String timestamp representation tin the form like 2024-05-29
   * 18:04:14.201
   */
  inline std::string get_timestamp(uint64_t milliseconds) {
    std::chrono::system_clock::time_point tm{
        std::chrono::milliseconds{milliseconds}};
    auto in_time_t = std::chrono::system_clock::to_time_t(tm);

    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&in_time_t), "%Y-%m-%d %H:%M:%S") << '.'
        << std::setfill('0') << std::setw(3) << milliseconds % 1000;
    return oss.str();
  }

  /**
   * Returns formatted timestamp with TZ string from the unix time in
   * milliseconds. TZ is always GMT
   * @return String timestamp representation tin the form like Wed May 29
   * 18:05:07 2024 GMT
   */
  inline std::string get_timestamp_with_tz(uint64_t milliseconds) {
    std::chrono::system_clock::time_point tm{
        std::chrono::milliseconds{milliseconds}};
    auto in_time_t = std::chrono::system_clock::to_time_t(tm);

    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&in_time_t), "%c %Z");

    return oss.str();
  }
};

/**
 * Implementation of UUID_VX_TO_TIMESTAMP() function.
 * The function takes exactly one string argument. The argument must be a valid
 * UUID of version 1,6 or 7 in formatted or hexadecimal form. In other case
 * function throws error. If the argument can be parsed as UUID, the function
 * returns it's timestamp in the form like 2024-05-29 18:04:14.201.
 */
class uuid_vx_to_timestamp_impl : timestamp_based_uuid {
 public:
  uuid_vx_to_timestamp_impl(mysqlpp::udf_context &ctx) {
    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);
    if (ctx.get_number_of_args() != 1) {
      raise<std::invalid_argument>(err_msg_one_argument);
    }
    // arg0 - @uuid_string
    ctx.mark_arg_nullable(0, true);
    ctx.set_arg_type(0, STRING_RESULT);

    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    charset_ext.set_return_value_charset(ctx, string_charset);
    charset_ext.set_arg_value_charset(ctx, 0, uuid_charset);
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &ctx) {
    if (ctx.is_arg_null(0)) {
      return {};
    }
    auto uxs = ctx.get_arg<STRING_RESULT>(0);
    return get_timestamp(get_ts(uxs));
  }
};

/**
 * Implementation of UUID_VX_TO_TIMESTAMP_TZ() function.
 * The function takes exactly one string argument. The argument must be a valid
 * UUID of version 1,6 or 7 in formatted or hexadecimal form. In other case
 * function throws error. If the argument can be parsed as UUID, the function
 * returns it's timestamp in the form like Wed May 29 18:05:07 2024 GMT.
 */
class uuid_vx_to_timestamp_tz_impl : timestamp_based_uuid {
 public:
  uuid_vx_to_timestamp_tz_impl(mysqlpp::udf_context &ctx) {
    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);
    if (ctx.get_number_of_args() != 1) {
      raise<std::invalid_argument>(err_msg_no_arguments);
    }
    // arg0 - @uuid_string
    ctx.mark_arg_nullable(0, true);
    ctx.set_arg_type(0, STRING_RESULT);

    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    charset_ext.set_return_value_charset(ctx, string_charset);
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &ctx) {
    if (ctx.is_arg_null(0)) {
      return {};
    }

    auto uxs = ctx.get_arg<STRING_RESULT>(0);

    return get_timestamp_with_tz(get_ts(uxs));
  }
};

/**
 * Implementation of UUID_VX_TO_TIMESTAMP_TZ() function.
 * The function takes exactly one string argument. The argument must be a valid
 * UUID of version 1,6 or 7 in formatted or hexadecimal form. In other case
 * function throws error. If the argument can be parsed as UUID, the function
 * returns it's timestamp in ms since epoch.
 */
class uuid_vx_to_unixtime_impl : timestamp_based_uuid {
 public:
  uuid_vx_to_unixtime_impl(mysqlpp::udf_context &ctx) {
    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);
    if (ctx.get_number_of_args() != 1) {
      raise<std::invalid_argument>(err_msg_one_argument);
    }
    // arg0 - @uuid_string
    ctx.mark_arg_nullable(0, true);
    ctx.set_arg_type(0, STRING_RESULT);
  }

  mysqlpp::udf_result_t<INT_RESULT> calculate(const mysqlpp::udf_context &ctx) {
    if (ctx.is_arg_null(0)) {
      return {};
    }
    auto uxs = ctx.get_arg<STRING_RESULT>(0);
    return get_ts(uxs);
  }
};

}  // namespace

DECLARE_INT_UDF_AUTO(uuid_vx_version)
DECLARE_INT_UDF_AUTO(uuid_vx_variant)
DECLARE_INT_UDF_AUTO(is_uuid_vx)
DECLARE_INT_UDF_AUTO(is_nil_uuid_vx)
DECLARE_INT_UDF_AUTO(is_max_uuid_vx)
// requires invoke of g++ with -latomic
DECLARE_STRING_UDF_AUTO(uuid_v1)
DECLARE_STRING_UDF_AUTO(uuid_v3)
DECLARE_STRING_UDF_AUTO(uuid_v4)
DECLARE_STRING_UDF_AUTO(uuid_v5)
// requires invoke of g++ with -latomic
DECLARE_STRING_UDF_AUTO(uuid_v6)
DECLARE_STRING_UDF_AUTO(uuid_v7)
DECLARE_STRING_UDF_AUTO(nil_uuid_vx)
DECLARE_STRING_UDF_AUTO(max_uuid_vx)
DECLARE_STRING_UDF_AUTO(uuid_vx_to_bin)
DECLARE_STRING_UDF_AUTO(bin_to_uuid_vx)
DECLARE_STRING_UDF_AUTO(uuid_vx_to_timestamp)
DECLARE_STRING_UDF_AUTO(uuid_vx_to_timestamp_tz)
DECLARE_INT_UDF_AUTO(uuid_vx_to_unixtime)

// array of defined UFDs
static const std::array known_udfs{
    DECLARE_UDF_INFO_AUTO(uuid_vx_version),
    DECLARE_UDF_INFO_AUTO(uuid_vx_variant),
    DECLARE_UDF_INFO_AUTO(is_uuid_vx),
    DECLARE_UDF_INFO_AUTO(is_nil_uuid_vx),
    DECLARE_UDF_INFO_AUTO(is_max_uuid_vx),
    DECLARE_UDF_INFO_AUTO(uuid_v1),
    DECLARE_UDF_INFO_AUTO(uuid_v3),
    DECLARE_UDF_INFO_AUTO(uuid_v4),
    DECLARE_UDF_INFO_AUTO(uuid_v5),
    DECLARE_UDF_INFO_AUTO(uuid_v6),
    DECLARE_UDF_INFO_AUTO(uuid_v7),
    DECLARE_UDF_INFO_AUTO(nil_uuid_vx),
    DECLARE_UDF_INFO_AUTO(max_uuid_vx),
    DECLARE_UDF_INFO_AUTO(uuid_vx_to_bin),
    DECLARE_UDF_INFO_AUTO(bin_to_uuid_vx),
    DECLARE_UDF_INFO_AUTO(uuid_vx_to_timestamp),
    DECLARE_UDF_INFO_AUTO(uuid_vx_to_timestamp_tz),
    DECLARE_UDF_INFO_AUTO(uuid_vx_to_unixtime)};

using udf_bitset_type =
    mysqlpp::udf_bitset<std::tuple_size_v<decltype(known_udfs)>>;
static udf_bitset_type registered_udfs;

static void uuidx_udf_my_error(int error_id, myf flags, ...) {
  va_list args;
  va_start(args, flags);
  mysql_service_mysql_runtime_error->emit(error_id, flags, args);
  va_end(args);
}

/**
  Initialization entry method for Component used when loading the Component.

  @return Status of performed operation
  @retval 0 success
  @retval non-zero failure
*/
static mysql_service_status_t component_uuidx_udf_init() {
  mysqlpp::udf_error_reporter::instance() = &uuidx_udf_my_error;
  mysqlpp::register_udfs(mysql_service_udf_registration, known_udfs,
                         registered_udfs);
  return registered_udfs.all() ? 0 : 1;
}

/**
  De-initialization method for Component
  @return Status of performed operation
  @retval 0 success
  @retval non-zero failure
*/

static mysql_service_status_t component_uuidx_udf_deinit() {
  mysqlpp::unregister_udfs(mysql_service_udf_registration, known_udfs,
                           registered_udfs);
  return registered_udfs.none() ? 0 : 1;
}

// clang-format off
BEGIN_COMPONENT_PROVIDES(CURRENT_COMPONENT_NAME)
END_COMPONENT_PROVIDES();

BEGIN_COMPONENT_REQUIRES(CURRENT_COMPONENT_NAME)
  REQUIRES_SERVICE(mysql_runtime_error),
  REQUIRES_SERVICE(udf_registration),
  REQUIRES_SERVICE(mysql_udf_metadata),
END_COMPONENT_REQUIRES();

BEGIN_COMPONENT_METADATA(CURRENT_COMPONENT_NAME)
  METADATA("mysql.author", "Percona Corporation"),
  METADATA("mysql.license", "GPL"),
END_COMPONENT_METADATA();

DECLARE_COMPONENT(CURRENT_COMPONENT_NAME, CURRENT_COMPONENT_NAME_STR)
  component_uuidx_udf_init,
  component_uuidx_udf_deinit,
END_DECLARE_COMPONENT();
// clang-format on

DECLARE_LIBRARY_COMPONENTS &COMPONENT_REF(CURRENT_COMPONENT_NAME)
    END_DECLARE_LIBRARY_COMPONENTS
