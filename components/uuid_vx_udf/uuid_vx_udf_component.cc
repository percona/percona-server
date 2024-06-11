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

#include <cassert>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>

#include <boost/preprocessor/stringize.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/date_time/c_time.hpp>


#include <mysql/components/component_implementation.h>

#include <mysql/components/services/component_sys_var_service.h>
#include <mysql/components/services/mysql_current_thread_reader.h>
#include <mysql/components/services/mysql_runtime_error.h>
#include <mysql/components/services/udf_metadata.h>
#include <mysql/components/services/udf_registration.h>

#include <mysqlpp/udf_context_charset_extension.hpp>
#include <mysqlpp/udf_registration.hpp>
#include <mysqlpp/udf_wrappers.hpp>


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
constexpr char binary_charset[]{"binary"};

constexpr std::string_view nil_uuid_sv{"00000000-0000-0000-0000-000000000000"};
constexpr std::string_view max_uuid_sv{"ffffffff-ffff-ffff-ffff-ffffffffffff"};

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
  explicit uuid_vx_version_impl(mysqlpp::udf_context &ctx) {
    if (ctx.get_number_of_args() != 1) {
      raise<std::invalid_argument>(err_msg_one_argument);
    }

    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);

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
      const auto uxs{ctx.get_arg<STRING_RESULT>(0)};
      const auto uid{gen(std::begin(uxs), std::end(uxs))};
      version = uid.version();
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
  explicit uuid_vx_variant_impl(mysqlpp::udf_context &ctx) {
    if (ctx.get_number_of_args() != 1) {
      raise<std::invalid_argument>(err_msg_one_argument);
    }

    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);

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
      const auto uxs{ctx.get_arg<STRING_RESULT>(0)};
      const auto uid{gen(std::begin(uxs), std::end(uxs))};
      variant = uid.variant();
    } catch (const std::exception &) {
      raise<std::invalid_argument>(err_msg_valid_uuid);
    }
    return static_cast<std::underlying_type_t<decltype(variant)>>(variant);
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
  explicit is_uuid_vx_impl(mysqlpp::udf_context &ctx) {
    if (ctx.get_number_of_args() != 1) {
      raise<std::invalid_argument>(err_msg_one_argument);
    }

    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);

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

    bool verification_result{false};
    try {
      boost::uuids::string_generator gen;
      const auto uxs{ctx.get_arg<STRING_RESULT>(0)};
      gen(std::begin(uxs), std::end(uxs));
      verification_result = true;
    } catch (const std::exception &) {
      verification_result = false;
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
  explicit is_nil_uuid_vx_impl(mysqlpp::udf_context &ctx) {
    if (ctx.get_number_of_args() != 1) {
      raise<std::invalid_argument>(err_msg_one_argument);
    }

    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);

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

    bool verification_result{false};
    try {
      boost::uuids::string_generator gen;
      const auto uxs{ctx.get_arg<STRING_RESULT>(0)};
      const auto uid{gen(std::begin(uxs), std::end(uxs))};
      verification_result = uid.is_nil();
    } catch (const std::exception &) {
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

  explicit is_max_uuid_vx_impl(mysqlpp::udf_context &ctx) {
    if (ctx.get_number_of_args() != 1) {
      raise<std::invalid_argument>(err_msg_one_argument);
    }

    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);

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

    bool verification_result{false};
    try {
      boost::uuids::string_generator gen;
      const auto uxs{ctx.get_arg<STRING_RESULT>(0)};
      const auto uid{gen(std::begin(uxs), std::end(uxs))};
      verification_result = (uid == max_uuid);
    } catch (const std::exception &) {
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
  explicit uuid_v1_impl(mysqlpp::udf_context &ctx) {
    if (ctx.get_number_of_args() != 0) {
      raise<std::invalid_argument>(err_msg_no_arguments);
    }

    ctx.mark_result_const(false);
    ctx.mark_result_nullable(false);
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
 * Helper enum class defining different hash-based UUID namespaces
 */
enum class hash_uuid_namespace_type {
  dns = 0,
  url = 1,
  oid = 2,
  x500 = 3,
  enum_min = dns,
  enum_max = x500
};

/**
 * Helper class for string-based uuids
 */
class string_based_uuid {
 public:
  static boost::uuids::uuid get_uuid_namespace(
      hash_uuid_namespace_type hash_uuid_namespace) noexcept {
    switch (hash_uuid_namespace) {
      case hash_uuid_namespace_type::dns:
        return boost::uuids::ns::dns();
      case hash_uuid_namespace_type::url:
        return boost::uuids::ns::url();
      case hash_uuid_namespace_type::oid:
        return boost::uuids::ns::oid();
      case hash_uuid_namespace_type::x500:
        return boost::uuids::ns::x500dn();
      default:
        // we have 4 NS in the standard by now.
        // In any other case we will trigger an assertion
        assert(false);
    }
    return {};
  }
  static hash_uuid_namespace_type get_namespace_from_arg(
      const mysqlpp::udf_context &ctx, std::size_t index) {
    static constexpr auto default_namespace{hash_uuid_namespace_type::url};
    if (index >= ctx.get_number_of_args()) {
      return default_namespace;
    }

    const auto ns_arg{ctx.get_arg<INT_RESULT>(index)};
    if (!ns_arg.has_value()) {
      return default_namespace;
    }

    const auto ns_raw{ns_arg.value()};
    if (ns_raw < static_cast<std::underlying_type_t<hash_uuid_namespace_type>>(
                     hash_uuid_namespace_type::enum_min) ||
        ns_raw > static_cast<std::underlying_type_t<hash_uuid_namespace_type>>(
                     hash_uuid_namespace_type::enum_max)) {
      raise<std::invalid_argument>(err_msg_uuid_namespace_idx);
    }
    return static_cast<hash_uuid_namespace_type>(ns_raw);
  }
};

/**
 * Implementation of UUID_V3() function.
 * The function takes 1 or 2 arguments. It generates string
 * based UUID of version 3. The first argument is string to hash with
 * MD5 algorithm. The second optional argument is name space for UUID.
 * DNS: 0, URL: 1, OID: 2, X.500: 3, default is 1, or URL
 */
class uuid_v3_impl : private string_based_uuid {
 public:
  explicit uuid_v3_impl(mysqlpp::udf_context &ctx) {
    const auto narg{ctx.get_number_of_args()};
    if (narg < 1 || narg > 2) {
      raise<std::invalid_argument>(err_msg_one_or_two_arguments);
    }

    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);

    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    charset_ext.set_return_value_charset(ctx, uuid_charset);

    // arg0 - @uuid string
    ctx.mark_arg_nullable(0, true);
    ctx.set_arg_type(0, STRING_RESULT);
    charset_ext.set_arg_value_charset(ctx, 0, string_charset);
    if (narg == 2) {
      // the second argumnent (namespace) can be NULL, meaning default (URL)
      ctx.mark_arg_nullable(1, true);
      ctx.set_arg_type(1, INT_RESULT);
    }
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &ctx) {
    if (ctx.is_arg_null(0)) {
      return {};
    }

    const auto hash_uuid_namespace{get_namespace_from_arg(ctx, 1)};
    boost::uuids::name_generator_md5 gen_v3{
        get_uuid_namespace(hash_uuid_namespace)};
    const auto the_string{ctx.get_arg<STRING_RESULT>(0)};
    return boost::uuids::to_string(
        gen_v3(std::data(the_string), std::size(the_string)));
  }
};

/**
 * Implementation of UUID_V4() function.
 * The function no arguments. It generates random number
 * based UUID of version 4.
 */
class uuid_v4_impl {
 public:
  explicit uuid_v4_impl(mysqlpp::udf_context &ctx) {
    if (ctx.get_number_of_args() > 0) {
      raise<std::invalid_argument>(err_msg_no_arguments);
    }

    ctx.mark_result_const(false);
    ctx.mark_result_nullable(false);

    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    charset_ext.set_return_value_charset(ctx, uuid_charset);
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      [[maybe_unused]] const mysqlpp::udf_context &ctx) {
    static thread_local boost::uuids::random_generator gen_v4;
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
class uuid_v5_impl : private string_based_uuid {
 public:
  explicit uuid_v5_impl(mysqlpp::udf_context &ctx) {
    const auto narg{ctx.get_number_of_args()};
    if (narg < 1 || narg > 2) {
      raise<std::invalid_argument>(err_msg_one_or_two_arguments);
    }

    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);

    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    charset_ext.set_return_value_charset(ctx, uuid_charset);

    // arg0 - @uuid string
    ctx.mark_arg_nullable(0, true);
    ctx.set_arg_type(0, STRING_RESULT);
    charset_ext.set_arg_value_charset(ctx, 0, string_charset);
    if (narg == 2) {
      // the second argumnent (namespace) can be NULL, meaning default (URL)
      ctx.mark_arg_nullable(1, true);
      ctx.set_arg_type(1, INT_RESULT);
    }
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &ctx) {
    if (ctx.is_arg_null(0)) {
      return {};
    }

    const auto hash_uuid_namespace{get_namespace_from_arg(ctx, 1)};
    boost::uuids::name_generator_sha1 gen_v5{
        get_uuid_namespace(hash_uuid_namespace)};
    const auto the_string{ctx.get_arg<STRING_RESULT>(0)};
    return boost::uuids::to_string(
        gen_v5(std::data(the_string), std::size(the_string)));
  }
};

/**
 * Implementation of UUID_V6() function.
 * The function takes no arguments. It generates time stamp
 * based UUID of version 6.
 */
class uuid_v6_impl {
 public:
  explicit uuid_v6_impl(mysqlpp::udf_context &ctx) {
    if (ctx.get_number_of_args() != 0) {
      raise<std::invalid_argument>(err_msg_no_arguments);
    }

    ctx.mark_result_const(false);
    ctx.mark_result_nullable(false);
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
  explicit uuid_v7_impl(mysqlpp::udf_context &ctx) {
    const auto narg{ctx.get_number_of_args()};
    if (narg > 1) {
      raise<std::invalid_argument>(err_msg_zero_or_one_argument);
    }

    ctx.mark_result_const(false);
    ctx.mark_result_nullable(false);
    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    charset_ext.set_return_value_charset(ctx, uuid_charset);

    // arg0 - @time_offset_ms, optional
    if (narg == 1) {
      // NULL value for the first argument (time shift) means 0, e.g. no time
      // shift
      ctx.mark_arg_nullable(0, true);
      ctx.set_arg_type(0, INT_RESULT);
    }
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &ctx) {
    static thread_local boost::uuids::time_generator_v7 gen_v7;
    auto generated{gen_v7()};
    std::int64_t offset{0};
    if (ctx.get_number_of_args() == 1) {
      const auto offset_arg{ctx.get_arg<INT_RESULT>(0)};
      offset = offset_arg.value_or(0);
    }
    if (offset != 0) {
      generated = add_ts_offset(generated, offset);
    }
    return boost::uuids::to_string(generated);
  }

 private:
  /**
   * This function just shifts timestamp (ms part only) for uuid version 7.
   * ofs_ms argument is of type "std::int64_t" so it will not cause integer
   * overflow.
   * @return time-shifted UUID v7
   */
  static boost::uuids::uuid add_ts_offset(boost::uuids::uuid uid,
                                          std::int64_t ofs_ms) {
    // TODO: overflow can happen here
    const std::uint64_t time_ms{static_cast<std::uint64_t>(
        uid.time_point_v7().time_since_epoch().count() + ofs_ms)};
    const std::uint16_t d6tmp{uid.data[6]};  // ver and rand part
    const std::uint8_t d7tmp{uid.data[7]};   // rand part
    const std::uint64_t timestamp_and_others{(time_ms << 16) | (d6tmp << 8) |
                                             d7tmp};
    boost::uuids::detail::store_big_u64(uid.data + 0, timestamp_and_others);
    return uid;
  }
};

/**
 * Implementation of NIL_UUID_VX() function.
 * The function takes no arguments.
 * It generates time NIL UUID.
 */
class nil_uuid_vx_impl {
 public:
  explicit nil_uuid_vx_impl(mysqlpp::udf_context &ctx) {
    if (ctx.get_number_of_args() != 0) {
      raise<std::invalid_argument>(err_msg_no_arguments);
    }

    ctx.mark_result_const(true);
    ctx.mark_result_nullable(false);
    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    charset_ext.set_return_value_charset(ctx, uuid_charset);
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      [[maybe_unused]] const mysqlpp::udf_context &ctx) {
    return std::string{nil_uuid_sv};
  }
};

/**
 * Implementation of MAX_UUID_VX() function.
 * The function takes no arguments.
 * It generates time MAX UUID.
 */
class max_uuid_vx_impl {
 public:
  explicit max_uuid_vx_impl(mysqlpp::udf_context &ctx) {
    if (ctx.get_number_of_args() != 0) {
      raise<std::invalid_argument>(err_msg_no_arguments);
    }

    ctx.mark_result_const(true);
    ctx.mark_result_nullable(false);
    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    charset_ext.set_return_value_charset(ctx, uuid_charset);
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      [[maybe_unused]] const mysqlpp::udf_context &ctx) {
    return std::string{max_uuid_sv};
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
  explicit uuid_vx_to_bin_impl(mysqlpp::udf_context &ctx) {
    if (ctx.get_number_of_args() != 1) {
      raise<std::invalid_argument>(err_msg_one_argument);
    }

    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);

    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    charset_ext.set_return_value_charset(ctx, binary_charset);

    // arg0 - @uuid_string
    ctx.mark_arg_nullable(0, true);
    ctx.set_arg_type(0, STRING_RESULT);
    charset_ext.set_arg_value_charset(ctx, 0, uuid_charset);
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &ctx) {
    if (ctx.is_arg_null(0)) {
      return {};
    }

    boost::uuids::uuid uid;
    try {
      boost::uuids::string_generator gen;
      const auto uxs{ctx.get_arg<STRING_RESULT>(0)};
      uid = gen(std::begin(uxs), std::end(uxs));
    } catch (const std::exception &ex) {
      raise<std::invalid_argument>(err_msg_valid_uuid);
    }

    return std::string{std::begin(uid), std::end(uid)};
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
  explicit bin_to_uuid_vx_impl(mysqlpp::udf_context &ctx) {
    if (ctx.get_number_of_args() != 1) {
      raise<std::invalid_argument>(err_msg_one_argument);
    }

    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);
    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    charset_ext.set_return_value_charset(ctx, uuid_charset);

    // arg0 - @uuid_version
    ctx.mark_arg_nullable(0, true);
    ctx.set_arg_type(0, STRING_RESULT);
    charset_ext.set_arg_value_charset(ctx, 0, binary_charset);
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &ctx) {
    if (ctx.is_arg_null(0)) {
      return {};
    }
    const auto ubs{ctx.get_arg<STRING_RESULT>(0)};
    if (ubs.size() != boost::uuids::uuid::static_size()) {
      raise<std::invalid_argument>(err_msg_16bytes);
    }

    boost::uuids::uuid uid;
    std::copy(std::begin(ubs), std::end(ubs), std::begin(uid));

    return boost::uuids::to_string(uid);
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
  static std::uint64_t get_ts(std::string_view uxs) {
    boost::uuids::string_generator gen;
    boost::uuids::uuid uid;
    std::int64_t ms{0};
    try {
      uid = gen(std::begin(uxs), std::end(uxs));
    } catch (const std::exception &ex) {
      raise<std::invalid_argument>(err_msg_valid_v167_uuid);
    }
    switch (uid.version()) {
      case 1:
        ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                 boost::uuids::uuid_clock::to_sys(uid.time_point_v1())
                     .time_since_epoch())
                 .count();
        break;
      case 6:
        ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                 boost::uuids::uuid_clock::to_sys(uid.time_point_v6())
                     .time_since_epoch())
                 .count();
        break;
      case 7:
        ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                 uid.time_point_v7().time_since_epoch())
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
  static std::string get_timestamp(std::uint64_t milliseconds) {
    std::chrono::system_clock::time_point tm{
        std::chrono::milliseconds{milliseconds}};
    const auto in_time_t{std::chrono::system_clock::to_time_t(tm)};

    std::tm gm_time = {}; // temp for gmtime_r inside of c_time::gmtime
    std::ostringstream oss;
    oss << std::put_time(boost::date_time::c_time::gmtime(&in_time_t, &gm_time), "%Y-%m-%d %H:%M:%S") << '.'
        << std::setfill('0') << std::setw(3) << milliseconds % 1000;
      
    return oss.str();
  }

  /**
   * Returns formatted timestamp with TZ string from the unix time in
   * milliseconds. TZ is always GMT
   * @return String timestamp representation tin the form like Wed May 29
   * 18:05:07 2024 GMT
   */
  static std::string get_timestamp_with_tz(std::uint64_t milliseconds) {
    std::chrono::system_clock::time_point tm{
        std::chrono::milliseconds{milliseconds}};
    const auto in_time_t{std::chrono::system_clock::to_time_t(tm)};
    
    std::ostringstream oss;
    std::tm gm_time = {}; // temp for gmtime_r inside of c_time::gmtime
    oss << std::put_time(boost::date_time::c_time::gmtime(&in_time_t, &gm_time), "%c %Z");
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
class uuid_vx_to_timestamp_impl : private timestamp_based_uuid {
 public:
  explicit uuid_vx_to_timestamp_impl(mysqlpp::udf_context &ctx) {
    if (ctx.get_number_of_args() != 1) {
      raise<std::invalid_argument>(err_msg_one_argument);
    }

    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);
    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    charset_ext.set_return_value_charset(ctx, string_charset);

    // arg0 - @uuid_string
    ctx.mark_arg_nullable(0, true);
    ctx.set_arg_type(0, STRING_RESULT);
    charset_ext.set_arg_value_charset(ctx, 0, uuid_charset);
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &ctx) {
    if (ctx.is_arg_null(0)) {
      return {};
    }

    const auto uxs{ctx.get_arg<STRING_RESULT>(0)};
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
class uuid_vx_to_timestamp_tz_impl : private timestamp_based_uuid {
 public:
  explicit uuid_vx_to_timestamp_tz_impl(mysqlpp::udf_context &ctx) {
    if (ctx.get_number_of_args() != 1) {
      raise<std::invalid_argument>(err_msg_no_arguments);
    }

    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);
    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    charset_ext.set_return_value_charset(ctx, string_charset);

    // arg0 - @uuid_string
    ctx.mark_arg_nullable(0, true);
    ctx.set_arg_type(0, STRING_RESULT);
    charset_ext.set_arg_value_charset(ctx, 0, uuid_charset);
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &ctx) {
    if (ctx.is_arg_null(0)) {
      return {};
    }

    const auto uxs{ctx.get_arg<STRING_RESULT>(0)};
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
class uuid_vx_to_unixtime_impl : private timestamp_based_uuid {
 public:
  explicit uuid_vx_to_unixtime_impl(mysqlpp::udf_context &ctx) {
    if (ctx.get_number_of_args() != 1) {
      raise<std::invalid_argument>(err_msg_one_argument);
    }

    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);

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

    const auto uxs{ctx.get_arg<STRING_RESULT>(0)};
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

namespace { //anon namespace instead of using static keyword
using udf_bitset_type =
    mysqlpp::udf_bitset<std::tuple_size_v<decltype(known_udfs)>>;
udf_bitset_type registered_udfs;

void uuidx_udf_my_error(int error_id, myf flags, ...) {
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
mysql_service_status_t component_uuidx_udf_init() {
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

mysql_service_status_t component_uuidx_udf_deinit() {
  mysqlpp::unregister_udfs(mysql_service_udf_registration, known_udfs,
                           registered_udfs);
  return registered_udfs.none() ? 0 : 1;
}
} // namespace

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
