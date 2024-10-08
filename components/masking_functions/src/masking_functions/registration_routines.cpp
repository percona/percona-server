/* Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.
   Copyright (c) 2018, 2019 Francisco Miguel Biete Banon. All rights reserved.
   Copyright (c) 2023 Percona LLC and/or its affiliates. All rights reserved.

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

#include "masking_functions/registration_routines.hpp"

#include <algorithm>
#include <array>
#include <bitset>
#include <locale>

#include <mysql/components/component_implementation.h>

#include <mysql/components/services/dynamic_privilege.h>
#include <mysql/components/services/mysql_current_thread_reader.h>
#include <mysql/components/services/security_context.h>

#include <mysqlpp/udf_context.hpp>
#include <mysqlpp/udf_context_charset_extension.hpp>
#include <mysqlpp/udf_registration.hpp>
#include <mysqlpp/udf_wrappers.hpp>

#include "masking_functions/charset_string.hpp"
#include "masking_functions/charset_string_operations.hpp"
#include "masking_functions/command_service_tuple.hpp"
#include "masking_functions/primitive_singleton.hpp"
#include "masking_functions/query_cache.hpp"
#include "masking_functions/random_string_generators.hpp"
#include "masking_functions/sql_escape_functions.hpp"
#include "masking_functions/string_service_tuple.hpp"

extern REQUIRES_SERVICE_PLACEHOLDER(udf_registration);
extern REQUIRES_SERVICE_PLACEHOLDER(dynamic_privilege_register);

extern REQUIRES_SERVICE_PLACEHOLDER(mysql_udf_metadata);

extern REQUIRES_SERVICE_PLACEHOLDER(mysql_current_thread_reader);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_thd_security_context);
extern REQUIRES_SERVICE_PLACEHOLDER(global_grants_check);

namespace {

using global_string_services = masking_functions::primitive_singleton<
    masking_functions::string_service_tuple>;
using global_query_cache =
    masking_functions::primitive_singleton<masking_functions::query_cache_ptr>;

constexpr std::string_view masking_dictionaries_privilege_name =
    "MASKING_DICTIONARIES_ADMIN";

// Returns 'true' if current MySQL user has 'MASKING_DICTIONARIES_ADMIN'
// dynamic privilege
bool have_masking_admin_privilege() {
  THD *thd;
  if (mysql_service_mysql_current_thread_reader->get(&thd)) {
    throw std::runtime_error{"Couldn't query current thd"};
  }

  Security_context_handle sctx;
  if (mysql_service_mysql_thd_security_context->get(thd, &sctx)) {
    throw std::runtime_error{"Couldn't query security context"};
  }

  if (mysql_service_global_grants_check->has_global_grant(
          sctx, masking_dictionaries_privilege_name.data(),
          masking_dictionaries_privilege_name.size()))
    return true;

  return false;
}

// Creates an instance of the 'charcter_string' class from the
// 'argno' UDF argument. The argument must be of the 'STRING_RESULT'
// type and its value must not be NULL. Argument's collation is
// determined via 'mysql_service_mysql_udf_metadata' MySQL service.
masking_functions::charset_string make_charset_string_from_arg(
    mysqlpp::udf_context const &ctx, std::size_t argno) {
  assert(argno < ctx.get_number_of_args());
  assert(ctx.get_arg_type(argno) == STRING_RESULT);
  const auto arg = ctx.get_arg<STRING_RESULT>(argno);
  if (arg.data() == nullptr)
    throw std::invalid_argument{"cannot create charset_string from NULL"};

  mysqlpp::udf_context_charset_extension charset_ext{
      mysql_service_mysql_udf_metadata};
  return {global_string_services::instance(), arg,
          charset_ext.get_arg_collation(ctx, argno)};
}

constexpr std::string_view x_ascii_masking_char = "X";
constexpr std::string_view star_ascii_masking_char = "*";

// This function determines the value of the masking character used by a
// number of 'mask_xxx()' UDFs.
// It tries to obtain the value of the 'argno' UDF argument:
// if no such argument exists, the provided 'default_ascii_masking_char'
// is used instead.
// The result value is returned in a form of 'charset_string'.
masking_functions::charset_string determine_masking_char(
    mysqlpp::udf_context const &ctx, std::size_t argno,
    std::string_view default_ascii_masking_char) {
  masking_functions::charset_string masking_char;

  if (argno >= ctx.get_number_of_args() || ctx.is_arg_null(argno)) {
    masking_char = masking_functions::charset_string(
        global_string_services::instance(), default_ascii_masking_char,
        masking_functions::charset_string::ascii_collation_name);
  } else {
    masking_char = make_charset_string_from_arg(ctx, argno);
  }
  if (masking_char.get_size_in_characters() != 1)
    throw std::invalid_argument{"masking character must be of length 1"};

  return masking_char;
}

void set_return_value_collation_from_arg(
    mysqlpp::udf_context_charset_extension &charset_ext,
    mysqlpp::udf_context &ctx, std::size_t argno) {
  charset_ext.set_return_value_collation(
      ctx, charset_ext.get_arg_collation(ctx, argno));
}

//
// gen_range(int, int)
//
// Generates a random integer in the [<first argument>..<second argument>]
// range inclusive.
// If <first argument> is less then <second argument>, NULL is returned.
class gen_range_impl {
 public:
  gen_range_impl(mysqlpp::udf_context &ctx) {
    if (ctx.get_number_of_args() != 2)
      throw std::invalid_argument{"Wrong argument list: should be (int, int)"};

    ctx.mark_result_nullable(true);
    ctx.mark_result_const(false);

    ctx.mark_arg_nullable(0, false);
    ctx.set_arg_type(0, INT_RESULT);

    ctx.mark_arg_nullable(1, false);
    ctx.set_arg_type(1, INT_RESULT);
  }

  mysqlpp::udf_result_t<INT_RESULT> calculate(const mysqlpp::udf_context &ctx) {
    const auto lower = *ctx.get_arg<INT_RESULT>(0);
    const auto upper = *ctx.get_arg<INT_RESULT>(1);

    if (upper < lower) {
      return std::nullopt;
    } else {
      return masking_functions::random_number(lower, upper);
    }
  }
};

//
// gen_rnd_email([int], [int], [string])
//
// Generates a random email in the following format:
// <name>.<surname>@<domain>.
// <name> - a randomly-generated sequence of lower-case latin letters ([a-z])
// of length <first argument> (5 if not present)
// <surname> - a randomly-generated sequence of lower-case latin letters
// ([a-z]) of length <second argument> (7 if not present)
// <domain> is taken from the <third argument> ('example.com' if not present)
class gen_rnd_email_impl {
 private:
  static constexpr std::string_view default_ascii_email_domain = "example.com";
  static constexpr std::size_t default_name_length = 5;
  static constexpr std::size_t default_surname_length = 7;
  static constexpr std::size_t max_name_length = 1024;
  static constexpr std::size_t max_surname_length = 1024;

 public:
  gen_rnd_email_impl(mysqlpp::udf_context &ctx) {
    if (ctx.get_number_of_args() > 3)
      throw std::invalid_argument{
          "Wrong argument list: should be ([int], [int], [string])"};

    ctx.mark_result_nullable(true);
    ctx.mark_result_const(false);

    if (ctx.get_number_of_args() >= 1) {
      ctx.mark_arg_nullable(0, false);
      ctx.set_arg_type(0, INT_RESULT);
    }

    if (ctx.get_number_of_args() >= 2) {
      ctx.mark_arg_nullable(1, false);
      ctx.set_arg_type(1, INT_RESULT);
    }

    if (ctx.get_number_of_args() >= 3) {
      ctx.mark_arg_nullable(2, false);
      ctx.set_arg_type(2, STRING_RESULT);
    }

    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    if (ctx.get_number_of_args() >= 3) {
      set_return_value_collation_from_arg(charset_ext, ctx, 2);
    } else {
      charset_ext.set_return_value_collation(
          ctx, masking_functions::charset_string::default_collation_name);
    }
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &ctx) {
    masking_functions::charset_string cs_email_domain;
    if (ctx.get_number_of_args() >= 3) {
      cs_email_domain = make_charset_string_from_arg(ctx, 2);
    } else {
      cs_email_domain = masking_functions::charset_string{
          global_string_services::instance(), default_ascii_email_domain,
          masking_functions::charset_string::default_collation_name};
    }

    const long long name_length = ctx.get_number_of_args() >= 1
                                      ? *ctx.get_arg<INT_RESULT>(0)
                                      : default_name_length;
    if (name_length <= 0) {
      throw std::invalid_argument{"Name length must be a positive number"};
    }
    const auto casted_name_length = static_cast<std::size_t>(name_length);
    if (casted_name_length > max_name_length) {
      throw std::invalid_argument{"Name length must not exceed " +
                                  std::to_string(max_name_length)};
    }

    const long long surname_length = ctx.get_number_of_args() >= 2
                                         ? *ctx.get_arg<INT_RESULT>(1)
                                         : default_surname_length;
    if (surname_length <= 0) {
      throw std::invalid_argument{"Surname length must be a positive number"};
    }
    const auto casted_surname_length = static_cast<std::size_t>(surname_length);
    if (casted_surname_length > max_surname_length) {
      throw std::invalid_argument{"Surname length must not exceed " +
                                  std::to_string(max_surname_length)};
    }

    std::string email;
    email.reserve(casted_name_length + 1 + casted_surname_length + 1);
    email += masking_functions::random_lower_alpha_string(casted_name_length);
    email += '.';
    email +=
        masking_functions::random_lower_alpha_string(casted_surname_length);
    email += '@';

    masking_functions::charset_string default_cs_email{
        global_string_services::instance(), email,
        masking_functions::charset_string::default_collation_name};
    auto cs_email = default_cs_email.convert_to_collation_copy(
        cs_email_domain.get_collation());

    cs_email += cs_email_domain;

    return {std::string{cs_email.get_buffer()}};
  }
};

//
// gen_rnd_iban([string], [int])
//
// Generates a random International Bank Account Number (IBAN).
// Returns a string containing the country code <first argument>
// (2 latin uppercase characters) followed by (<second argument> - 2) random
// digits.
// E.g. ZZ0123456789012
// This function does not calculate proper IBAN checksum (3rd and 4th
// digits) - those positions have randomly-generated digits.
class gen_rnd_iban_impl {
 private:
  static constexpr std::string_view default_ascii_country_code{"ZZ"};
  static constexpr std::size_t country_code_length{2U};
  static constexpr std::size_t min_number_of_characters{15U};
  static constexpr std::size_t max_number_of_characters{34U};
  static constexpr std::size_t default_number_of_characters{16U};

  static void validate_ansi_country_code(
      const masking_functions::charset_string &ascii_country_code) {
    if (ascii_country_code.get_size_in_characters() != country_code_length ||
        ascii_country_code.get_size_in_bytes() != country_code_length) {
      throw std::invalid_argument{"IBAN country code must be exactly " +
                                  std::to_string(country_code_length) +
                                  " ASCII characters"};
    }
    const auto buffer = ascii_country_code.get_buffer();
    if (std::find_if_not(std::begin(buffer), std::end(buffer), [](char ch) {
          return std::isupper(ch, std::locale::classic());
        }) != std::end(buffer))
      throw std::invalid_argument{
          "IBAN country code must include only latin upper-case characters"};
  }

 public:
  gen_rnd_iban_impl(mysqlpp::udf_context &ctx) {
    if (ctx.get_number_of_args() > 2)
      throw std::invalid_argument{
          "Wrong argument list: should be ([string], [int])"};

    ctx.mark_result_nullable(true);
    ctx.mark_result_const(false);

    if (ctx.get_number_of_args() >= 1) {
      ctx.mark_arg_nullable(0, false);
      ctx.set_arg_type(0, STRING_RESULT);
    }

    if (ctx.get_number_of_args() >= 2) {
      ctx.mark_arg_nullable(1, false);
      ctx.set_arg_type(1, INT_RESULT);
    }

    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    if (ctx.get_number_of_args() >= 1) {
      set_return_value_collation_from_arg(charset_ext, ctx, 0);
    } else {
      charset_ext.set_return_value_collation(
          ctx, masking_functions::charset_string::default_collation_name);
    }
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &ctx) {
    masking_functions::charset_string cs_country_code;
    if (ctx.get_number_of_args() >= 1) {
      cs_country_code = make_charset_string_from_arg(ctx, 0);
    } else {
      cs_country_code = masking_functions::charset_string{
          global_string_services::instance(), default_ascii_country_code,
          masking_functions::charset_string::default_collation_name};
    }

    masking_functions::charset_string conversion_buffer;
    const auto &ascii_country_code =
        masking_functions::smart_convert_to_collation(
            cs_country_code,
            masking_functions::charset_string::ascii_collation_name,
            conversion_buffer);
    validate_ansi_country_code(ascii_country_code);

    const long long iban_length = ctx.get_number_of_args() >= 2
                                      ? *ctx.get_arg<INT_RESULT>(1)
                                      : default_number_of_characters;

    if (iban_length < 0) {
      throw std::invalid_argument{"IBAN length must not be a negative number"};
    }

    const auto casted_iban_length = static_cast<std::size_t>(iban_length);

    if (casted_iban_length < min_number_of_characters ||
        casted_iban_length > max_number_of_characters) {
      throw std::invalid_argument{"IBAN length must be between " +
                                  std::to_string(min_number_of_characters) +
                                  " and " +
                                  std::to_string(max_number_of_characters)};
    }

    auto generated_iban = masking_functions::random_iban(
        ascii_country_code.get_buffer(),
        casted_iban_length - country_code_length);
    auto ascii_iban = masking_functions::charset_string(
        global_string_services::instance(), generated_iban,
        masking_functions::charset_string::ascii_collation_name);

    const auto &cs_iban = masking_functions::smart_convert_to_collation(
        ascii_iban, cs_country_code.get_collation(), conversion_buffer);
    return {std::string{cs_iban.get_buffer()}};
  }
};

class rnd_impl_base {
 public:
  rnd_impl_base(mysqlpp::udf_context &ctx) {
    if (ctx.get_number_of_args() != 0) {
      throw std::invalid_argument{"Wrong argument list: should be empty"};
    }
    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    charset_ext.set_return_value_collation(
        ctx, masking_functions::charset_string::default_collation_name);
  }

 protected:
  ~rnd_impl_base() = default;
};

//
// gen_rnd_canada_sin()
//
// Generates a Canada Social Insurance Number (SIN) in AAA-BBB-CCC format
// that passes checksum validation.
// E.g. 123-456-789

class gen_rnd_canada_sin_impl final : private rnd_impl_base {
 public:
  using rnd_impl_base::rnd_impl_base;
  mysqlpp::udf_result_t<STRING_RESULT> calculate(const mysqlpp::udf_context &) {
    return masking_functions::random_canada_sin();
  }
};

//
// gen_rnd_pan()
//
// Generates a random American Express / Visa / Mastercard / Discover
// credit card number that passes basic checksum validation.
// E.g. 1234567887654321
class gen_rnd_pan_impl final : private rnd_impl_base {
 public:
  using rnd_impl_base::rnd_impl_base;
  mysqlpp::udf_result_t<STRING_RESULT> calculate(const mysqlpp::udf_context &) {
    return masking_functions::random_credit_card();
  }
};

//
// gen_rnd_ssn()
//
// Generates a random US Social Security Number (SSN) in AAA-BB-CCCC format,
// where AAA is in the 900..999 range (meaning not used / reserved).
// E.g. 951-26-0058
class gen_rnd_ssn_impl final : private rnd_impl_base {
 public:
  using rnd_impl_base::rnd_impl_base;
  mysqlpp::udf_result_t<STRING_RESULT> calculate(const mysqlpp::udf_context &) {
    return masking_functions::random_ssn();
  }
};

//
// gen_rnd_uk_nin()
//
// Generates a random United Kingdom National Insurance Number (UK NIN)
// in nine-character format that always starts with 'AA' and ends with 'C'.
// E.g. AA123456C
class gen_rnd_uk_nin_impl final : private rnd_impl_base {
 public:
  using rnd_impl_base::rnd_impl_base;
  mysqlpp::udf_result_t<STRING_RESULT> calculate(const mysqlpp::udf_context &) {
    return masking_functions::random_uk_nin();
  }
};

//
// gen_rnd_us_phone()
//
// Generates a random US phone number in 1-555-AAA-BBBB format.
// E.g. 1-555-682-5423
class gen_rnd_us_phone_impl final : private rnd_impl_base {
 public:
  using rnd_impl_base::rnd_impl_base;
  mysqlpp::udf_result_t<STRING_RESULT> calculate(const mysqlpp::udf_context &) {
    return masking_functions::random_us_phone();
  }
};

//
// gen_rnd_uuid()
//
// Generates a random v4 Universal Unique Identifier (UUID).
// E.g. 82d9b7cc-7fad-481b-8eed-a27c11b4a404
class gen_rnd_uuid_impl final : private rnd_impl_base {
 public:
  using rnd_impl_base::rnd_impl_base;
  mysqlpp::udf_result_t<STRING_RESULT> calculate(const mysqlpp::udf_context &) {
    return masking_functions::random_uuid();
  }
};

//
// mask_inner(string, int, int, [char])
//
// This function masks every character in the provided string <first
// argument> except for the first <second argument> and the last <third
// argument> characters.
// Optional <forth argument> is used as a masking character.
class mask_inner_impl {
 public:
  mask_inner_impl(mysqlpp::udf_context &ctx) {
    if (ctx.get_number_of_args() < 3 || ctx.get_number_of_args() > 4)
      throw std::invalid_argument{
          "Wrong argument list: should be (string, int, int, [char])"};

    ctx.mark_result_nullable(true);
    ctx.mark_result_const(true);

    ctx.mark_arg_nullable(0, true);
    ctx.set_arg_type(0, STRING_RESULT);

    ctx.mark_arg_nullable(1, false);
    ctx.set_arg_type(1, INT_RESULT);

    ctx.mark_arg_nullable(2, false);
    ctx.set_arg_type(2, INT_RESULT);

    if (ctx.get_number_of_args() >= 4) {
      ctx.mark_arg_nullable(3, false);
      ctx.set_arg_type(3, STRING_RESULT);
    }

    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    set_return_value_collation_from_arg(charset_ext, ctx, 0);
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &ctx) {
    if (ctx.is_arg_null(0)) return std::nullopt;

    const auto cs_str = make_charset_string_from_arg(ctx, 0);

    const auto masking_char =
        determine_masking_char(ctx, 3, x_ascii_masking_char);

    const auto left_margin = *ctx.get_arg<INT_RESULT>(1);
    const auto right_margin = *ctx.get_arg<INT_RESULT>(2);

    if (left_margin < 0 || right_margin < 0) {
      throw std::invalid_argument{"Margins can't be negative!"};
    }

    const auto casted_left_margin = static_cast<std::size_t>(left_margin);
    const auto casted_right_margin = static_cast<std::size_t>(right_margin);

    const auto result = masking_functions::mask_inner(
        cs_str, casted_left_margin, casted_right_margin, masking_char);

    return {std::string{result.get_buffer()}};
  }
};

//
// mask_outer(string, int, int, [char])
//
// This function masks the first <second argument> and the last <third
// argument> characters in the provided string <first argument>.
// Optional <forth argument> is used as a masking character.
class mask_outer_impl {
 public:
  mask_outer_impl(mysqlpp::udf_context &ctx) {
    if (ctx.get_number_of_args() < 3 || ctx.get_number_of_args() > 4)
      throw std::invalid_argument{
          "Wrong argument list: should be (string, int, int [char])"};

    ctx.mark_result_nullable(true);
    ctx.mark_result_const(true);

    ctx.mark_arg_nullable(0, true);
    ctx.set_arg_type(0, STRING_RESULT);

    ctx.mark_arg_nullable(1, false);
    ctx.set_arg_type(1, INT_RESULT);

    ctx.mark_arg_nullable(2, false);
    ctx.set_arg_type(2, INT_RESULT);

    if (ctx.get_number_of_args() >= 4) {
      ctx.mark_arg_nullable(3, false);
      ctx.set_arg_type(3, STRING_RESULT);
    }

    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    set_return_value_collation_from_arg(charset_ext, ctx, 0);
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &ctx) {
    if (ctx.is_arg_null(0)) return std::nullopt;

    const auto cs_str = make_charset_string_from_arg(ctx, 0);

    const auto masking_char =
        determine_masking_char(ctx, 3, x_ascii_masking_char);

    const auto left_margin = *ctx.get_arg<INT_RESULT>(1);
    const auto right_margin = *ctx.get_arg<INT_RESULT>(2);

    if (left_margin < 0 || right_margin < 0) {
      throw std::invalid_argument{"Margins can't be negative!"};
    }

    const auto casted_left_margin = static_cast<std::size_t>(left_margin);
    const auto casted_right_margin = static_cast<std::size_t>(right_margin);

    const auto result = masking_functions::mask_outer(
        cs_str, casted_left_margin, casted_right_margin, masking_char);

    return {std::string{result.get_buffer()}};
  }
};

// A base class used by a number of concrete 'mask_xxx()' UDF
// implementations. It provides generic implementation of the 'calculate()'
// method that relies on 'min_length()', 'max_length()',
// 'default_ascii_masking_char()' and 'process()' that are supposed to be
// overloaded in the derived classes. It sets common aguments / result
// nullness expectations as well as desired argument types
// ("string [, string]"). It also performs basic argument validation,
// the length of the first argument must be within the
// "[min_length(), max_length()]" range, and tries to determine the masking
// character from the second argument if present. After that it simply
// delegates its execution to the 'process()' virtual function.
class mask_impl_base {
 private:
  virtual std::size_t min_length() const = 0;
  virtual std::size_t max_length() const = 0;
  virtual std::string_view default_ascii_masking_char() const = 0;
  virtual masking_functions::charset_string process(
      const masking_functions::charset_string &cs_str,
      const masking_functions::charset_string &masking_char) const = 0;

 public:
  mask_impl_base(mysqlpp::udf_context &ctx) {
    if (ctx.get_number_of_args() < 1 || ctx.get_number_of_args() > 2)
      throw std::invalid_argument{
          "Wrong argument list: should be (string, [char])"};

    ctx.mark_result_nullable(true);
    ctx.mark_result_const(true);

    ctx.mark_arg_nullable(0, true);
    ctx.set_arg_type(0, STRING_RESULT);

    if (ctx.get_number_of_args() >= 2) {
      ctx.mark_arg_nullable(1, false);
      ctx.set_arg_type(1, STRING_RESULT);
    }

    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    set_return_value_collation_from_arg(charset_ext, ctx, 0);
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &ctx) {
    if (ctx.is_arg_null(0)) return std::nullopt;

    const auto cs_str = make_charset_string_from_arg(ctx, 0);
    const auto cs_str_length = cs_str.get_size_in_characters();

    if (cs_str_length < min_length() || cs_str_length > max_length()) {
      if (min_length() == max_length()) {
        throw std::invalid_argument{"Argument must be exactly " +
                                    std::to_string(min_length()) +
                                    " characters"};
      } else {
        throw std::invalid_argument{
            "Argument must be between " + std::to_string(min_length()) +
            " and " + std::to_string(max_length()) + " characters"};
      }
    }

    const auto masking_char =
        determine_masking_char(ctx, 1, default_ascii_masking_char());

    const auto result = process(cs_str, masking_char);

    return {std::string{result.get_buffer()}};
  }

 protected:
  ~mask_impl_base() = default;
};

//
// mask_canada_sin(string, [char])
//
// Canada SIN consists of 3 groups of 3 consequtive digits optionally
// separated with a delimiter
// E.g. 123456789 or 123-456-789
//      XXXXXXXXX or XXX-XXX-XXX
// min_length          : 9
// max_length          : 11
// default_masking_char: X
class mask_canada_sin_impl final : private mask_impl_base {
 public:
  using mask_impl_base::calculate;
  using mask_impl_base::mask_impl_base;

 private:
  virtual std::size_t min_length() const override { return 9; }
  virtual std::size_t max_length() const override { return 11; }
  virtual std::string_view default_ascii_masking_char() const override {
    return x_ascii_masking_char;
  }

  virtual masking_functions::charset_string process(
      const masking_functions::charset_string &cs_str,
      const masking_functions::charset_string &masking_char) const override {
    if (cs_str.get_size_in_characters() == max_length()) {
      // in the case when the length is max_length (11) we expect the
      // delimiters to be at the predefined positions (after the first 3
      // digits and before the last 3 digits), we just mask everything except
      // the delimiters
      auto sresult = masking_functions::mask_inner(cs_str, 4, 4, masking_char);
      sresult = masking_functions::mask_inner(sresult, 0, 8, masking_char);
      return masking_functions::mask_inner(sresult, 8, 0, masking_char);
    } else {
      // otherwise (no delimiters at all, or just one at an unknown position),
      // we use 'mask_inner_alphanum()' for the whole range
      return masking_functions::mask_inner_alphanum(cs_str, 0, 0, masking_char);
    }
  }
};

//
// mask_iban(string, [char])
//
// IBAN consists of 2 upper-case latin letters (country code) followed by
// 13..32 latin alphanumeric characters [A-Z0-9]. It may include up to 8
// delimiters
// E.g. ZZ0123456789012 or IE12 BOFI 9000 0112 3456 78
//      ZZ************* or IE** **** **** **** **** **
// min_length          : 15
// max_length          : 42
// default_masking_char: *
class mask_iban_impl final : private mask_impl_base {
 public:
  using mask_impl_base::calculate;
  using mask_impl_base::mask_impl_base;

 private:
  virtual std::size_t min_length() const override { return 15; }
  virtual std::size_t max_length() const override { return 34 + 8; }
  virtual std::string_view default_ascii_masking_char() const override {
    return star_ascii_masking_char;
  }
  virtual masking_functions::charset_string process(
      const masking_functions::charset_string &cs_str,
      const masking_functions::charset_string &masking_char) const override {
    // as positions of the delimiters may vary, we always use the
    // 'mask_inner_alphanum()' function for everything except for the first 2
    // characters
    return masking_functions::mask_inner_alphanum(cs_str, 2, 0, masking_char);
  }
};

//
// mask_pan(string, [char])
//
// Card number consists of 14..16 digits possibly separated with up to 3
// delimiters.
// E.g. 1234567887654321 or 1234 5678 8765 4321
//      XXXXXXXXXXXX4321 or XXXX XXXX XXXX 4321
// min_length          : 14
// max_length          : 19
// default_masking_char: X
class mask_pan_impl final : private mask_impl_base {
 public:
  using mask_impl_base::calculate;
  using mask_impl_base::mask_impl_base;

 private:
  virtual std::size_t min_length() const override { return 14; }
  virtual std::size_t max_length() const override { return 19; }
  virtual std::string_view default_ascii_masking_char() const override {
    return x_ascii_masking_char;
  }

  virtual masking_functions::charset_string process(
      const masking_functions::charset_string &cs_str,
      const masking_functions::charset_string &masking_char) const override {
    // as positions of the delimiters may vary, we always use the
    // 'mask_inner_alphanum()' function for everything except for the last 4
    // characters
    return masking_functions::mask_inner_alphanum(cs_str, 0, 4, masking_char);
  }
};

//
// mask_pan_relaxed(string, [char])
//
// Card number consists of 14..16 digits possibly separated with up to 3
// delimiters.
// E.g. 1234567887654321 or 1234 5678 8765 4321
//      123456XXXXXX4321 or 1234 56XX XXXX 4321
// min_length          : 14
// max_length          : 19
// default_masking_char: X
class mask_pan_relaxed_impl final : private mask_impl_base {
 public:
  using mask_impl_base::calculate;
  using mask_impl_base::mask_impl_base;

 private:
  virtual std::size_t min_length() const override { return 14; }
  virtual std::size_t max_length() const override { return 19; }
  virtual std::string_view default_ascii_masking_char() const override {
    return x_ascii_masking_char;
  }
  virtual masking_functions::charset_string process(
      const masking_functions::charset_string &cs_str,
      const masking_functions::charset_string &masking_char) const override {
    // as positions of the delimiters may vary, we always use the
    // 'mask_inner_alphanum()' function for everything except the first 6 and
    // the last 4 characters
    return masking_functions::mask_inner_alphanum(cs_str, 6, 4, masking_char);
  }
};

//
// mask_ssn(string, [char])
//
// US SSN consists of 3 groups of 3, 2 and 4 consequtive digits optionally
// separated with a delimiter
// E.g. 909636922 or 909-63-6922
//      *****6922 or ***-**-6922
// min_length          : 9
// max_length          : 11
// default_masking_char: *
class mask_ssn_impl final : private mask_impl_base {
 public:
  using mask_impl_base::calculate;
  using mask_impl_base::mask_impl_base;

 private:
  virtual std::size_t min_length() const override { return 9; }
  virtual std::size_t max_length() const override { return 11; }
  virtual std::string_view default_ascii_masking_char() const override {
    return star_ascii_masking_char;
  }
  virtual masking_functions::charset_string process(
      const masking_functions::charset_string &cs_str,
      const masking_functions::charset_string &masking_char) const override {
    if (cs_str.get_size_in_characters() == max_length()) {
      // in the case when the length is max_length (11) we expect the
      // delimiters to be at the predefined positions (after the first 3
      // digits and before the last 4 digits), we just mask everything except
      // the delimiters
      auto sresult = masking_functions::mask_inner(cs_str, 4, 5, masking_char);
      return masking_functions::mask_inner(sresult, 0, 8, masking_char);
    } else {
      // otherwise (no delimiters at all, or just one at an unknown position),
      // we use 'mask_inner_alphanum()' for everything except the last 4 digits
      return masking_functions::mask_inner_alphanum(cs_str, 0, 4, masking_char);
    }
  }
};

//
// mask_uk_nin(string, [char])
//
// UK NIN consists of 3 groups of 3, 2 and 4 or 3 groups of 2, 6 and 1
// consequtive digits optionaly separated with a delimiter
// E.g. QQ123456C or QQ 123456 C or QQ1 23 456C
//      QQ******* or QQ ****** * or QQ* ** ****
// min_length          : 9
// max_length          : 11
// default_masking_char: *
class mask_uk_nin_impl final : private mask_impl_base {
 public:
  using mask_impl_base::calculate;
  using mask_impl_base::mask_impl_base;

 private:
  virtual std::size_t min_length() const override { return 9; }
  virtual std::size_t max_length() const override { return 11; }
  virtual std::string_view default_ascii_masking_char() const override {
    return star_ascii_masking_char;
  }
  virtual masking_functions::charset_string process(
      const masking_functions::charset_string &cs_str,
      const masking_functions::charset_string &masking_char) const override {
    // as positions of the delimiters may vary, we always use the
    // 'mask_inner_alphanum()' function for everything except for the first 2
    // characters
    return masking_functions::mask_inner_alphanum(cs_str, 2, 0, masking_char);
  }
};

//
// mask_uuid(string, [char])
//
// UUID consists of 5 groups of 8, 4, 4, 4 and 12 consequtive hexadecimal
// characters separated with a non-optional dash.
// E.g. 82d9b7cc-7fad-481b-8eed-a27c11b4a404
//      ********-****-****-****-************
// min_length          : 36
// max_length          : 36
// default_masking_char: *
class mask_uuid_impl final : private mask_impl_base {
 public:
  using mask_impl_base::calculate;
  using mask_impl_base::mask_impl_base;

 private:
  virtual std::size_t min_length() const override { return 36; }
  virtual std::size_t max_length() const override { return 36; }
  virtual std::string_view default_ascii_masking_char() const override {
    return star_ascii_masking_char;
  }
  virtual masking_functions::charset_string process(
      const masking_functions::charset_string &cs_str,
      const masking_functions::charset_string &masking_char) const override {
    return masking_functions::mask_inner_alphanum(cs_str, 0, 0, masking_char);
  }
};

//
// gen_blocklist(string, string, string)
//
// This function replaces a term present in one dictionary with a term from a
// second dictionary and returns the replacement term.
// It tries to find the term (the first argument) in the dictionary
// whose name is specified via the second argument.
// If found, we return a random term from the dictionary whose name is specified
// via the third argument, otherwise the original term is returned.
class gen_blocklist_impl {
 public:
  gen_blocklist_impl(mysqlpp::udf_context &ctx) {
    if (ctx.get_number_of_args() != 3)
      throw std::invalid_argument{
          "Wrong argument list: gen_blocklist(string, string, string)"};

    ctx.mark_result_nullable(true);
    ctx.mark_result_const(false);

    // arg0 - term
    ctx.mark_arg_nullable(0, true);
    ctx.set_arg_type(0, STRING_RESULT);

    // arg1 - dictionary 1 (from)
    ctx.mark_arg_nullable(1, false);
    ctx.set_arg_type(1, STRING_RESULT);

    // arg2 - dictionary 2 (to)
    ctx.mark_arg_nullable(2, false);
    ctx.set_arg_type(2, STRING_RESULT);

    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    set_return_value_collation_from_arg(charset_ext, ctx, 0);
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &ctx) {
    if (ctx.is_arg_null(0)) return std::nullopt;

    const auto cs_term = make_charset_string_from_arg(ctx, 0);
    const auto cs_term_escaped = escape_string(cs_term);
    const auto cs_dict_a_escaped =
        escape_string(make_charset_string_from_arg(ctx, 1));
    const auto cs_dict_b_escaped =
        escape_string(make_charset_string_from_arg(ctx, 2));

    {
      auto sresult = global_query_cache::instance()->contains(cs_dict_a_escaped,
                                                              cs_term_escaped);

      if (!sresult) {
        return cs_term_escaped;
      }
    }

    auto sresult =
        global_query_cache::instance()->get_random(cs_dict_b_escaped);

    if (!sresult.empty()) {
      masking_functions::charset_string utf8_result{
          global_string_services::instance(), sresult,
          masking_functions::charset_string::utf8mb4_collation_name};
      masking_functions::charset_string conversion_buffer;
      const auto &cs_result = masking_functions::smart_convert_to_collation(
          utf8_result, cs_term.get_collation(), conversion_buffer);
      return {std::string{cs_result.get_buffer()}};
    }

    return std::nullopt;
  }
};

//
// gen_dictionary(string)
//
// This function returns a random term from the dictionary whose name
// specified via the first argument.
class gen_dictionary_impl {
 public:
  gen_dictionary_impl(mysqlpp::udf_context &ctx) {
    if (ctx.get_number_of_args() != 1)
      throw std::invalid_argument{
          "Wrong argument list: gen_dictionary(string)"};

    ctx.mark_result_nullable(true);
    ctx.mark_result_const(false);

    // arg0 - dictionary
    ctx.mark_arg_nullable(0, false);
    ctx.set_arg_type(0, STRING_RESULT);

    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    charset_ext.set_return_value_collation(
        ctx, masking_functions::charset_string::default_collation_name);
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &ctx) {
    const auto cs_dictionary_escaped =
        escape_string(make_charset_string_from_arg(ctx, 0));
    auto sresult =
        global_query_cache::instance()->get_random(cs_dictionary_escaped);

    if (!sresult.empty()) {
      return sresult;
    }

    return std::nullopt;
  }
};

//
// masking_dictionaries_flush()
//
// Flush the data from the masking dictionaries table to the memory cache.
class masking_dictionaries_flush_impl {
 public:
  explicit masking_dictionaries_flush_impl(mysqlpp::udf_context &ctx) {
    if (!have_masking_admin_privilege()) {
      throw std::invalid_argument{
          "Function requires " +
          std::string(masking_dictionaries_privilege_name) + " privilege"};
    }

    if (ctx.get_number_of_args() > 0)
      throw std::invalid_argument{
          "Wrong argument list: masking_dictionaries_flush()"};

    ctx.mark_result_nullable(true);
    // Calling this UDF two or more times has exactly the same effect as just
    // calling it once. So, we mark the result as 'const' here so that the
    // optimizer could use this info to eliminate unnecessary calls.
    ctx.mark_result_const(true);

    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    charset_ext.set_return_value_collation(
        ctx, masking_functions::charset_string::default_collation_name);
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(const mysqlpp::udf_context &ctx
                                                 [[maybe_unused]]) {
    global_query_cache::instance()->reload_cache();

    return "1";
  }
};

//
// masking_dictionary_remove(string)
//
// This function removes the dictionary, whose name is specified via the first
// argument, and all of its terms from the dictionary registry.
class masking_dictionary_remove_impl {
 public:
  masking_dictionary_remove_impl(mysqlpp::udf_context &ctx) {
    if (!have_masking_admin_privilege()) {
      throw std::invalid_argument{
          "Function requires " +
          std::string(masking_dictionaries_privilege_name) + " privilege"};
    }

    if (ctx.get_number_of_args() != 1)
      throw std::invalid_argument{
          "Wrong argument list: masking_dictionary_remove(string)"};

    ctx.mark_result_nullable(true);
    // Calling this UDF two or more times has exactly the same effect as just
    // calling it once. So, we mark the result as 'const' here so that the
    // optimizer could use this info to eliminate unnecessary calls.
    ctx.mark_result_const(true);

    // arg0 - dictionary
    ctx.mark_arg_nullable(0, false);
    ctx.set_arg_type(0, STRING_RESULT);

    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    charset_ext.set_return_value_collation(
        ctx, masking_functions::charset_string::default_collation_name);
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &ctx) {
    const auto cs_dictionary_escaped =
        escape_string(make_charset_string_from_arg(ctx, 0));

    if (!global_query_cache::instance()->remove(cs_dictionary_escaped)) {
      return std::nullopt;
    }

    return "1";
  }
};

//
// masking_dictionary_term_add(string, string)
//
// This function adds one term specified via the second argument to the
// dictionary whose name is specified via the first argument.
class masking_dictionary_term_add_impl {
 public:
  masking_dictionary_term_add_impl(mysqlpp::udf_context &ctx) {
    if (!have_masking_admin_privilege()) {
      throw std::invalid_argument{
          "Function requires " +
          std::string(masking_dictionaries_privilege_name) + " privilege"};
    }

    if (ctx.get_number_of_args() != 2)
      throw std::invalid_argument{
          "Wrong argument list: masking_dictionary_term_add(string, "
          "string)"};

    ctx.mark_result_nullable(true);
    // Calling this UDF two or more times has exactly the same effect as just
    // calling it once. So, we mark the result as 'const' here so that the
    // optimizer could use this info to eliminate unnecessary calls.
    ctx.mark_result_const(true);

    // arg0 - dictionary
    ctx.mark_arg_nullable(0, false);
    ctx.set_arg_type(0, STRING_RESULT);

    // arg1 - term
    ctx.mark_arg_nullable(1, false);
    ctx.set_arg_type(1, STRING_RESULT);

    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    charset_ext.set_return_value_collation(
        ctx, masking_functions::charset_string::default_collation_name);
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &ctx) {
    const auto cs_dictionary_escaped =
        escape_string(make_charset_string_from_arg(ctx, 0));
    const auto cs_term_escaped =
        escape_string(make_charset_string_from_arg(ctx, 1));

    if (!global_query_cache::instance()->insert(cs_dictionary_escaped,
                                                cs_term_escaped)) {
      return std::nullopt;
    }

    return "1";
  }
};

//
// masking_dictionary_term_remove(string, string)
//
// This function removes one term specified via the second argument from the
// dictionary whose name is specified via the first argument.
class masking_dictionary_term_remove_impl {
 public:
  masking_dictionary_term_remove_impl(mysqlpp::udf_context &ctx) {
    if (!have_masking_admin_privilege()) {
      throw std::invalid_argument{
          "Function requires " +
          std::string(masking_dictionaries_privilege_name) + " privilege"};
    }

    if (ctx.get_number_of_args() != 2)
      throw std::invalid_argument{
          "Wrong argument list: masking_dictionary_term_remove(string, "
          "string)"};

    ctx.mark_result_nullable(true);
    // Calling this UDF two or more times has exactly the same effect as just
    // calling it once. So, we mark the result as 'const' here so that the
    // optimizer could use this info to eliminate unnecessary calls.
    ctx.mark_result_const(true);

    // arg0 - dictionary
    ctx.mark_arg_nullable(0, false);
    ctx.set_arg_type(0, STRING_RESULT);

    // arg1 - term
    ctx.mark_arg_nullable(1, false);
    ctx.set_arg_type(1, STRING_RESULT);

    mysqlpp::udf_context_charset_extension charset_ext{
        mysql_service_mysql_udf_metadata};
    charset_ext.set_return_value_collation(
        ctx, masking_functions::charset_string::default_collation_name);
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &ctx) {
    const auto cs_dictionary_escaped =
        escape_string(make_charset_string_from_arg(ctx, 0));
    const auto cs_term_escaped =
        escape_string(make_charset_string_from_arg(ctx, 1));

    if (!global_query_cache::instance()->remove(cs_dictionary_escaped,
                                                cs_term_escaped)) {
      return std::nullopt;
    }

    return "1";
  }
};

}  // anonymous namespace

DECLARE_INT_UDF_AUTO(gen_range)
DECLARE_STRING_UDF_AUTO(gen_rnd_email)
DECLARE_STRING_UDF_AUTO(gen_rnd_iban)
DECLARE_STRING_UDF_AUTO(gen_rnd_canada_sin)
DECLARE_STRING_UDF_AUTO(gen_rnd_pan)
DECLARE_STRING_UDF_AUTO(gen_rnd_ssn)
DECLARE_STRING_UDF_AUTO(gen_rnd_uk_nin)
DECLARE_STRING_UDF_AUTO(gen_rnd_us_phone)
DECLARE_STRING_UDF_AUTO(gen_rnd_uuid)

DECLARE_STRING_UDF_AUTO(mask_inner)
DECLARE_STRING_UDF_AUTO(mask_outer)
DECLARE_STRING_UDF_AUTO(mask_canada_sin)
DECLARE_STRING_UDF_AUTO(mask_iban)
DECLARE_STRING_UDF_AUTO(mask_pan)
DECLARE_STRING_UDF_AUTO(mask_pan_relaxed)
DECLARE_STRING_UDF_AUTO(mask_ssn)
DECLARE_STRING_UDF_AUTO(mask_uk_nin)
DECLARE_STRING_UDF_AUTO(mask_uuid)
DECLARE_STRING_UDF_AUTO(gen_blocklist)
DECLARE_STRING_UDF_AUTO(gen_dictionary)
DECLARE_STRING_UDF_AUTO(masking_dictionaries_flush)
DECLARE_STRING_UDF_AUTO(masking_dictionary_remove)
DECLARE_STRING_UDF_AUTO(masking_dictionary_term_add)
DECLARE_STRING_UDF_AUTO(masking_dictionary_term_remove)

// TODO: in c++20 (where CTAD works for alias templates) this shoud be changed
// to 'static const udf_info_container known_udfs'
std::array known_udfs{DECLARE_UDF_INFO_AUTO(gen_range),
                      DECLARE_UDF_INFO_AUTO(gen_rnd_email),
                      DECLARE_UDF_INFO_AUTO(gen_rnd_iban),
                      DECLARE_UDF_INFO_AUTO(gen_rnd_canada_sin),
                      DECLARE_UDF_INFO_AUTO(gen_rnd_pan),
                      DECLARE_UDF_INFO_AUTO(gen_rnd_ssn),
                      DECLARE_UDF_INFO_AUTO(gen_rnd_uk_nin),
                      DECLARE_UDF_INFO_AUTO(gen_rnd_us_phone),
                      DECLARE_UDF_INFO_AUTO(gen_rnd_uuid),

                      DECLARE_UDF_INFO_AUTO(mask_inner),
                      DECLARE_UDF_INFO_AUTO(mask_outer),
                      DECLARE_UDF_INFO_AUTO(mask_canada_sin),
                      DECLARE_UDF_INFO_AUTO(mask_iban),
                      DECLARE_UDF_INFO_AUTO(mask_pan),
                      DECLARE_UDF_INFO_AUTO(mask_pan_relaxed),
                      DECLARE_UDF_INFO_AUTO(mask_ssn),
                      DECLARE_UDF_INFO_AUTO(mask_uk_nin),
                      DECLARE_UDF_INFO_AUTO(mask_uuid),
                      DECLARE_UDF_INFO_AUTO(gen_blocklist),
                      DECLARE_UDF_INFO_AUTO(gen_dictionary),
                      DECLARE_UDF_INFO_AUTO(masking_dictionaries_flush),
                      DECLARE_UDF_INFO_AUTO(masking_dictionary_remove),
                      DECLARE_UDF_INFO_AUTO(masking_dictionary_term_add),
                      DECLARE_UDF_INFO_AUTO(masking_dictionary_term_remove)};

using udf_bitset_type =
    mysqlpp::udf_bitset<std::tuple_size_v<decltype(known_udfs)>>;
static udf_bitset_type registered_udfs;

static bool privileges_registered = false;

namespace masking_functions {

bool register_dynamic_privileges() {
  if (privileges_registered) {
    return true;
  }
  if (mysql_service_dynamic_privilege_register->register_privilege(
          masking_dictionaries_privilege_name.data(),
          masking_dictionaries_privilege_name.size()) == 0) {
    privileges_registered = true;
  }
  return privileges_registered;
}

bool unregister_dynamic_privileges() {
  if (!privileges_registered) {
    return true;
  }
  if (mysql_service_dynamic_privilege_register->unregister_privilege(
          masking_dictionaries_privilege_name.data(),
          masking_dictionaries_privilege_name.size()) == 0) {
    privileges_registered = false;
  }
  return !privileges_registered;
}

bool register_udfs() {
  mysqlpp::register_udfs(mysql_service_udf_registration, known_udfs,
                         registered_udfs);

  return registered_udfs.all();
}

bool unregister_udfs() {
  mysqlpp::unregister_udfs(mysql_service_udf_registration, known_udfs,
                           registered_udfs);

  return registered_udfs.none();
}

}  // namespace masking_functions
