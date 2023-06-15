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

#include <array>
#include <bitset>

#include "mysql/components/component_implementation.h"
#include "mysql/components/my_service.h"
#include "mysql/components/services/registry.h"
#include "mysql/components/services/udf_registration.h"

#include "components/masking_functions/include/component.h"
#include "components/masking_functions/include/udf/udf_registration.h"
#include "components/masking_functions/include/udf/udf_utils.h"
#include "components/masking_functions/include/udf/udf_utils_sql.h"
#include "components/masking_functions/include/udf/udf_utils_string.h"

#include <mysql/components/services/udf_metadata.h>
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_udf_metadata);
extern REQUIRES_SERVICE_PLACEHOLDER(udf_registration);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_string_character_access);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_string_converter);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_string_factory);

#include <mysqlpp/udf_registration.hpp>
#include <mysqlpp/udf_wrappers.hpp>

class gen_range_impl {
 public:
  gen_range_impl(mysqlpp::udf_context &ctx) {
    if (ctx.get_number_of_args() != 2)
      throw std::invalid_argument("Wrong argument list: should be (int, int)");

    ctx.mark_result_nullable(true);
    ctx.mark_result_const(false);

    ctx.mark_arg_nullable(0, false);
    ctx.set_arg_type(0, INT_RESULT);

    ctx.mark_arg_nullable(1, false);
    ctx.set_arg_type(1, INT_RESULT);
  }

  mysqlpp::udf_result_t<INT_RESULT> calculate(const mysqlpp::udf_context &ctx) {
    const long long lower = *ctx.get_arg<INT_RESULT>(0);
    const long long upper = *ctx.get_arg<INT_RESULT>(1);

    if (upper < lower) {
      return std::nullopt;
    } else {
      return mysql::plugins::random_number(lower, upper);
    }
  }
};

class gen_rnd_email_impl {
 public:
  gen_rnd_email_impl(mysqlpp::udf_context &ctx) {
    if (ctx.get_number_of_args() > 2)
      throw std::invalid_argument(
          "Wrong argument list: should be ([int], [string])");

    ctx.mark_result_nullable(true);
    ctx.mark_result_const(false);

    if (ctx.get_number_of_args() >= 1) {
      ctx.mark_arg_nullable(0, false);
      ctx.set_arg_type(0, INT_RESULT);
    }

    if (ctx.get_number_of_args() >= 2) {
      ctx.mark_arg_nullable(1, false);
      ctx.set_arg_type(1, STRING_RESULT);
    }

    if (ctx.get_number_of_args() >= 2) {
      ctx.set_return_value_charset_to_match_arg(1);
    } else {
      ctx.set_return_value_charset(mysql::plugins::default_charset);
    }
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &ctx) {
    auto original_charset = ctx.get_number_of_args() >= 2
                                ? ctx.get_arg_charset(1)
                                : mysql::plugins::default_charset;
    const std::string_view email_domain = ctx.get_number_of_args() >= 2
                                              ? ctx.get_arg<STRING_RESULT>(1)
                                              : "example.com";

    const long long email_length =
        ctx.get_number_of_args() >= 1 ? *ctx.get_arg<INT_RESULT>(0) : 20;

    if (email_length < 0) {
      throw std::invalid_argument(
          "Wrong argument list: length should be positive");
    }

    unsigned int domain_length = 0;
    {
      my_h_string mstr;
      mysql_service_mysql_string_converter->convert_from_buffer(
          &mstr, email_domain.data(), email_domain.size(), original_charset);
      mysql_service_mysql_string_character_access->get_char_length(
          mstr, &domain_length);
    }
    unsigned int user_length = email_length - (domain_length + 1);

    std::string email =
        mysql::plugins::random_string(user_length, true).append("@");

    std::string sresult = mysql::plugins::convert(
        email, mysql::plugins::default_charset, original_charset);
    sresult += email_domain;

    return mysql::plugins::convert(sresult, mysql::plugins::default_charset,
                                   original_charset);
  }
};

class gen_rnd_iban_impl {
 public:
  gen_rnd_iban_impl(mysqlpp::udf_context &ctx) {
    if (ctx.get_number_of_args() > 2)
      throw std::invalid_argument(
          "Wrong argument list: should be ([string], [int])");

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

    ctx.set_return_value_charset(mysql::plugins::default_charset);
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &ctx) {
    const std::string_view country =
        ctx.get_number_of_args() >= 1 ? ctx.get_arg<STRING_RESULT>(0) : "ZZ";

    if (country.size() != 2) {
      throw std::invalid_argument(
          "Wrong argument list: country should be two characters");
    }

    const long long len =
        ctx.get_number_of_args() >= 2 ? *ctx.get_arg<INT_RESULT>(1) : 16;

    if (len < 15 || len > 34) {
      throw std::invalid_argument(
          "Wrong argument list: length should be between 14 and 34");
    }

    return mysql::plugins::random_iban(country, len);
  }
};

class rnd_impl_base {
 public:
  rnd_impl_base(mysqlpp::udf_context &ctx) {
    if (ctx.get_number_of_args() != 0) {
      throw std::invalid_argument("Wrong argument list: should be empty");

      ctx.set_return_value_charset(mysql::plugins::default_charset);
    }
  }
};

class gen_rnd_pan_impl : public rnd_impl_base {
 public:
  using rnd_impl_base::rnd_impl_base;
  mysqlpp::udf_result_t<STRING_RESULT> calculate(const mysqlpp::udf_context &) {
    return mysql::plugins::random_credit_card();
  }
};

class gen_rnd_ssn_impl : public rnd_impl_base {
 public:
  using rnd_impl_base::rnd_impl_base;
  mysqlpp::udf_result_t<STRING_RESULT> calculate(const mysqlpp::udf_context &) {
    return mysql::plugins::random_ssn();
  }
};

class gen_rnd_uk_nin_impl : public rnd_impl_base {
 public:
  using rnd_impl_base::rnd_impl_base;
  mysqlpp::udf_result_t<STRING_RESULT> calculate(const mysqlpp::udf_context &) {
    return mysql::plugins::random_uk_nin();
  }
};

class gen_rnd_us_phone_impl : public rnd_impl_base {
 public:
  using rnd_impl_base::rnd_impl_base;
  mysqlpp::udf_result_t<STRING_RESULT> calculate(const mysqlpp::udf_context &) {
    return mysql::plugins::random_us_phone();
  }
};

class gen_rnd_uuid_impl : public rnd_impl_base {
 public:
  using rnd_impl_base::rnd_impl_base;
  mysqlpp::udf_result_t<STRING_RESULT> calculate(const mysqlpp::udf_context &) {
    return mysql::plugins::random_uuid();
  }
};

class mask_inner_impl {
 public:
  mask_inner_impl(mysqlpp::udf_context &ctx) {
    if (ctx.get_number_of_args() < 3 || ctx.get_number_of_args() > 4)
      throw std::invalid_argument(
          "Wrong argument list: should be (string, int, int [char])");

    ctx.mark_result_nullable(true);
    ctx.mark_result_const(false);

    ctx.mark_arg_nullable(0, false);
    ctx.set_arg_type(0, STRING_RESULT);

    ctx.mark_arg_nullable(1, false);
    ctx.set_arg_type(1, INT_RESULT);

    ctx.mark_arg_nullable(2, false);
    ctx.set_arg_type(2, INT_RESULT);

    if (ctx.get_number_of_args() >= 4) {
      ctx.mark_arg_nullable(3, false);
      ctx.set_arg_type(3, STRING_RESULT);
    }

    ctx.set_return_value_charset_to_match_arg(0);
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &ctx) {
    auto original_charset = ctx.get_arg_charset(0);
    const auto str = ctx.get_arg<STRING_RESULT>(0);

    if (str.empty()) {
      return std::nullopt;
    }

    const std::string masking_char =
        mysql::plugins::decide_masking_char(ctx, 3, original_charset);

    const long long a2 = *ctx.get_arg<INT_RESULT>(1);
    const long long a3 = *ctx.get_arg<INT_RESULT>(2);

    if (a2 < 0 || a3 < 0) {
      throw std::invalid_argument("Margins can't be negative!");
    }

    std::string sresult = mysql::plugins::mask_inner(
        str.data(), str.size(), a2, a3, original_charset, masking_char);

    if (sresult.size() > 0) {
      return sresult;
    } else {
      return std::nullopt;
    }
  }
};

class mask_outer_impl {
 public:
  mask_outer_impl(mysqlpp::udf_context &ctx) {
    if (ctx.get_number_of_args() < 3 || ctx.get_number_of_args() > 4)
      throw std::invalid_argument(
          "Wrong argument list: should be (string, int, int [char])");

    ctx.mark_result_nullable(true);
    ctx.mark_result_const(false);

    ctx.mark_arg_nullable(
        0, false);  // TODO: what's the point? It can still be null
    ctx.set_arg_type(0, STRING_RESULT);

    ctx.mark_arg_nullable(1, false);
    ctx.set_arg_type(1, INT_RESULT);

    ctx.mark_arg_nullable(2, false);
    ctx.set_arg_type(2, INT_RESULT);

    if (ctx.get_number_of_args() >= 4) {
      ctx.mark_arg_nullable(3, false);
      ctx.set_arg_type(3, STRING_RESULT);
    }

    ctx.set_return_value_charset_to_match_arg(0);
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &ctx) {
    auto original_charset = ctx.get_arg_charset(0);
    const auto str = ctx.get_arg<STRING_RESULT>(0);

    if (str.empty()) {
      return std::nullopt;
    }

    const std::string masking_char =
        mysql::plugins::decide_masking_char(ctx, 3, original_charset);

    const long long a2 = *ctx.get_arg<INT_RESULT>(1);
    const long long a3 = *ctx.get_arg<INT_RESULT>(2);

    if (a2 < 0 || a3 < 0) {
      throw std::invalid_argument("Margins can't be negative!");
    }

    ulong c2, c3;
    uint mlen = 0;
    {
      my_h_string mstr;
      mysql_service_mysql_string_converter->convert_from_buffer(
          &mstr, str.data(), str.size(), original_charset);
      mysql_service_mysql_string_character_access->get_char_length(mstr, &mlen);
      mysql_service_mysql_string_character_access->get_char_offset(mstr, a2,
                                                                   &c2);
      mysql_service_mysql_string_character_access->get_char_offset(
          mstr, mlen - a3, &c3);
      mysql_service_mysql_string_factory->destroy(mstr);
    }

    if (a2 + a3 >= mlen) {
      // too long => all masked
      std::string sresult;
      for (uint i = 0; i < mlen; ++i) sresult.append(masking_char);
      if (sresult.size() > 0) {
        return sresult;
      } else {
        return std::nullopt;
      }
    }
    const int chars_to_keep = mlen - a2 - a3;

    std::string sresult;

    for (uint i = 0; i < a2; ++i) sresult.append(masking_char);

    sresult.append(str.data() + a2, chars_to_keep);

    for (uint i = 0; i < a3; ++i) sresult.append(masking_char);

    if (sresult.size() > 0) {
      return sresult;
    } else {
      return std::nullopt;
    }
  }
};

class mask_impl_base {
 protected:
  virtual std::size_t min_length() = 0;
  virtual std::size_t max_length() = 0;
  virtual std::string process(std::string_view str,
                              std::string const &masking_char,
                              const char* original_charset) = 0;

 public:
  mask_impl_base(mysqlpp::udf_context &ctx) {
    if (ctx.get_number_of_args() < 1 || ctx.get_number_of_args() > 2)
      throw std::invalid_argument(
          "Wrong argument list: should be (string, [char])");

    ctx.mark_result_nullable(true);
    ctx.mark_result_const(false);

    ctx.mark_arg_nullable(0, false);
    ctx.set_arg_type(0, STRING_RESULT);

    if (ctx.get_number_of_args() >= 2) {
      ctx.mark_arg_nullable(1, false);
      ctx.set_arg_type(1, STRING_RESULT);
    }

    ctx.set_return_value_charset_to_match_arg(0);
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &ctx) {
    auto original_charset = ctx.get_arg_charset(0);
    const auto str = ctx.get_arg<STRING_RESULT>(0);

    if (str.size() < min_length() || str.size() > max_length()) {
      if (min_length() == max_length()) {
        std::string err = "Wrong argument: must be exactly ";
        err += std::to_string(min_length());
        err += " characters";
        throw std::invalid_argument(err);
      } else {
        std::string err = "Wrong argument: must be between ";
        err += std::to_string(min_length());
        err += " and ";
        err += std::to_string(max_length());
        err += " characters";
        throw std::invalid_argument(err);
      }
    }

    const std::string masking_char =
        mysql::plugins::decide_masking_char(ctx, 1, original_charset);

    const std::string sresult = process(str, masking_char, original_charset);

    if (sresult.size() > 0) {
      return sresult;
    } else {
      return std::nullopt;
    }
  }
};

class mask_canada_sin_impl : public mask_impl_base {
 public:
  using mask_impl_base::mask_impl_base;

 protected:
  virtual std::size_t min_length() override { return 9; }
  virtual std::size_t max_length() override { return 11; }
  virtual std::string process(std::string_view str,
                              std::string const &masking_char,
                              const char* original_charset) override {
    if (str.size() == 11) {
      std::string sresult = mysql::plugins::mask_inner(
          str.data(), str.size(), 4, 4, original_charset, masking_char);
      sresult = mysql::plugins::mask_inner(sresult.c_str(), sresult.size(), 0,
                                           8, original_charset, masking_char);
      return mysql::plugins::mask_inner(sresult.c_str(), sresult.size(), 8, 0,
                                        original_charset, masking_char);
    } else {
      return mysql::plugins::mask_inner(str.data(), str.size(), 0, 0,
                                        original_charset, masking_char);
    }
  }
};

class mask_iban_impl : public mask_impl_base {
 public:
  using mask_impl_base::mask_impl_base;

 protected:
  virtual std::size_t min_length() override { return 13; }
  virtual std::size_t max_length() override { return 34; }
  virtual std::string process(std::string_view str,
                              std::string const &masking_char,
                              const char* original_charset) override {
    return mysql::plugins::mask_inner(str.data(), str.size(), 2, 0,
                                      original_charset, masking_char);
  }
};

class mask_pan_impl : public mask_impl_base {
 public:
  using mask_impl_base::mask_impl_base;

 protected:
  virtual std::size_t min_length() override { return 14; }
  virtual std::size_t max_length() override { return 19; }
  virtual std::string process(std::string_view str,
                              std::string const &masking_char,
                              const char* original_charset) override {
    return mysql::plugins::mask_inner(str.data(), str.size(), 0, 4,
                                      original_charset, masking_char);
  }
};

class mask_pan_relaxed_impl : public mask_impl_base {
 public:
  using mask_impl_base::mask_impl_base;

 protected:
  virtual std::size_t min_length() override { return 14; }
  virtual std::size_t max_length() override { return 19; }
  virtual std::string process(std::string_view str,
                              std::string const &masking_char,
                              const char* original_charset) override {
    return mysql::plugins::mask_inner(str.data(), str.size(), 6, 4,
                                      original_charset, masking_char);
  }
};

class mask_ssn_impl : public mask_impl_base {
 public:
  using mask_impl_base::mask_impl_base;

 protected:
  virtual std::size_t min_length() override { return 9; }
  virtual std::size_t max_length() override { return 11; }
  virtual std::string process(std::string_view str,
                              std::string const &masking_char,
                              const char* original_charset) override {
    if (str.size() == 11) {
      std::string sresult = mysql::plugins::mask_inner(
          str.data(), str.size(), 4, 5, original_charset, masking_char);
      return mysql::plugins::mask_inner(sresult.c_str(), sresult.size(), 0, 8,
                                        original_charset, masking_char);
    } else {
      return mysql::plugins::mask_inner(str.data(), str.size(), 0, 4,
                                        original_charset, masking_char);
    }
  }
};

class mask_uk_nin_impl : public mask_impl_base {
 public:
  using mask_impl_base::mask_impl_base;

 protected:
  virtual std::size_t min_length() override { return 9; }
  virtual std::size_t max_length() override { return 11; }
  virtual std::string process(std::string_view str,
                              std::string const &masking_char,
                              const char* original_charset) override {
    return mysql::plugins::mask_inner(str.data(), str.size(), 2, 0,
                                      original_charset, masking_char);
  }
};

class mask_uuid_impl : public mask_impl_base {
 public:
  using mask_impl_base::mask_impl_base;

 protected:
  virtual std::size_t min_length() override { return 36; }
  virtual std::size_t max_length() override { return 36; }
  virtual std::string process(std::string_view str,
                              std::string const &masking_char,
                              const char* original_charset) override {
    std::string sresult;

    sresult = mysql::plugins::mask_inner(str.data(), str.size(), 0, 36 - 8,
                                         original_charset, masking_char);
    sresult =
        mysql::plugins::mask_inner(sresult.c_str(), sresult.size(), 9,
                                   36 - 9 - 4, original_charset, masking_char);
    sresult = mysql::plugins::mask_inner(sresult.c_str(), sresult.size(), 9 + 5,
                                         36 - 9 - 5 - 4, original_charset,
                                         masking_char);
    sresult = mysql::plugins::mask_inner(sresult.c_str(), sresult.size(),
                                         9 + 5 + 5, 36 - 9 - 5 - 5 - 4,
                                         original_charset, masking_char);
    sresult = mysql::plugins::mask_inner(sresult.c_str(), sresult.size(),
                                         9 + 5 + 5 + 5, 0, original_charset,
                                         masking_char);

    return sresult;
  }
};

class gen_blocklist_impl {
 public:
  gen_blocklist_impl(mysqlpp::udf_context &ctx) {
    if (ctx.get_number_of_args() != 3)
      throw std::invalid_argument(
          "Wrong argument list: gen_blocklist(string, string, string)");

    ctx.mark_result_nullable(true);
    ctx.mark_result_const(false);

    // arg1 - dictionary
    ctx.mark_arg_nullable(0, false);
    ctx.set_arg_type(0, STRING_RESULT);

    ctx.mark_arg_nullable(1, false);
    ctx.set_arg_type(1, STRING_RESULT);

    ctx.mark_arg_nullable(2, false);
    ctx.set_arg_type(2, STRING_RESULT);

    ctx.set_return_value_charset(mysql::plugins::default_charset);
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &ctx) {
    const std::string term = mysql::plugins::convert(
        ctx.get_arg<STRING_RESULT>(0), ctx.get_arg_charset(0),
        mysql::plugins::default_charset);
    const std::string dict_a = mysql::plugins::convert(
        ctx.get_arg<STRING_RESULT>(1), ctx.get_arg_charset(1),
        mysql::plugins::default_charset);
    const std::string dict_b = mysql::plugins::convert(
        ctx.get_arg<STRING_RESULT>(2), ctx.get_arg_charset(2),
        mysql::plugins::default_charset);

    {
      mysql::components::sql_context sql_ctx;

      std::string query(
          "SELECT Term FROM mysql.masking_dictionaries WHERE Dictionary =\"");
      mysql::components::escape_string_into(query, dict_a);
      query += "\" AND Term = \"";
      mysql::components::escape_string_into(query, term);
      query += "\" ORDER BY RAND() LIMIT 1";

      auto sresult = sql_ctx.query_single_value(query);

      if (!sresult) {
        return term;
      }
    }

    mysql::components::sql_context sql_ctx;

    std::string query =
        "SELECT Term FROM mysql.masking_dictionaries WHERE Dictionary =\"";
    mysql::components::escape_string_into(query, dict_b);
    query += "\" ORDER BY RAND() LIMIT 1";

    auto sresult = sql_ctx.query_single_value(query);

    if (sresult && sresult->size() > 0) {
      return *sresult;
    } else {
      return std::nullopt;
    }
  }
};

class gen_dictionary_impl {
 public:
  gen_dictionary_impl(mysqlpp::udf_context &ctx) {
    if (ctx.get_number_of_args() != 1)
      throw std::invalid_argument(
          "Wrong argument list: gen_dictionary(string)");

    ctx.mark_result_nullable(true);
    ctx.mark_result_const(false);

    // arg1 - dictionary
    ctx.mark_arg_nullable(0, false);
    ctx.set_arg_type(0, STRING_RESULT);

    ctx.set_return_value_charset(mysql::plugins::default_charset);
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &ctx) {
    const std::string dictionary = mysql::plugins::convert(
        ctx.get_arg<STRING_RESULT>(0), ctx.get_arg_charset(0),
        mysql::plugins::default_charset);

    mysql::components::sql_context sql_ctx;

    std::string query(
        "SELECT Term FROM mysql.masking_dictionaries WHERE Dictionary =\"");
    mysql::components::escape_string_into(query, dictionary);
    query += "\" ORDER BY RAND() LIMIT 1";

    auto sresult = sql_ctx.query_single_value(query);

    if (sresult && sresult->size() > 0) {
      return *sresult;
    } else {
      return std::nullopt;
    }
  }
};

class masking_dictionary_remove_impl {
 public:
  masking_dictionary_remove_impl(mysqlpp::udf_context &ctx) {
    if (!mysql::components::have_masking_admin_privilege()) {
      throw std::invalid_argument(
          "Function requires MASKING_DICTIONARIES_ADMIN privilege");
    }

    if (ctx.get_number_of_args() != 1)
      throw std::invalid_argument(
          "Wrong argument list: masking_dictionary_remove(string)");

    ctx.mark_result_nullable(true);
    ctx.mark_result_const(false);

    // arg1 - dictionary
    ctx.mark_arg_nullable(0, false);
    ctx.set_arg_type(0, STRING_RESULT);
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &ctx) {
    const std::string dictionary = mysql::plugins::convert(
        ctx.get_arg<STRING_RESULT>(0), ctx.get_arg_charset(0),
        mysql::plugins::default_charset);

    mysql::components::sql_context sql_ctx;

    std::string query(
        "DELETE FROM mysql.masking_dictionaries WHERE Dictionary=\"");
    mysql::components::escape_string_into(query, dictionary);
    query += "\"";

    if (sql_ctx.execute(query) == 0) {
      return std::nullopt;
    } else {
      return "1";
    }
  }
};

class masking_dictionary_term_add_impl {
 public:
  masking_dictionary_term_add_impl(mysqlpp::udf_context &ctx) {
    if (!mysql::components::have_masking_admin_privilege()) {
      throw std::invalid_argument(
          "Function requires MASKING_DICTIONARIES_ADMIN privilege");
    }

    if (ctx.get_number_of_args() != 2)
      throw std::invalid_argument(
          "Wrong argument list: masking_dictionary_term_add(string, "
          "string)");

    ctx.mark_result_nullable(true);
    ctx.mark_result_const(false);

    // arg1 - dictionary
    ctx.mark_arg_nullable(0, false);
    ctx.set_arg_type(0, STRING_RESULT);

    // arg2 - term
    ctx.mark_arg_nullable(1, false);
    ctx.set_arg_type(1, STRING_RESULT);
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &ctx) {
    const std::string dictionary = mysql::plugins::convert(
        ctx.get_arg<STRING_RESULT>(0), ctx.get_arg_charset(0),
        mysql::plugins::default_charset);

    const std::string term = mysql::plugins::convert(
        ctx.get_arg<STRING_RESULT>(1), ctx.get_arg_charset(1),
        mysql::plugins::default_charset);

    mysql::components::sql_context sql_ctx;

    std::string query(
        "INSERT IGNORE INTO mysql.masking_dictionaries (Dictionary, Term) "
        "VALUES(\"");
    mysql::components::escape_string_into(query, dictionary);
    query += "\", \"";
    mysql::components::escape_string_into(query, term);
    query += "\");";

    if (sql_ctx.execute(query) == 0) {
      return std::nullopt;
    } else {
      return "1";
    }
  }
};

class masking_dictionary_term_remove_impl {
 public:
  masking_dictionary_term_remove_impl(mysqlpp::udf_context &ctx) {
    if (!mysql::components::have_masking_admin_privilege()) {
      throw std::invalid_argument(
          "Function requires MASKING_DICTIONARIES_ADMIN privilege");
    }

    if (ctx.get_number_of_args() != 2)
      throw std::invalid_argument(
          "Wrong argument list: masking_dictionary_term_remove(string, "
          "string)");

    ctx.mark_result_nullable(true);
    ctx.mark_result_const(false);

    // arg1 - dictionary
    ctx.mark_arg_nullable(0, false);
    ctx.set_arg_type(0, STRING_RESULT);

    // arg2 - term
    ctx.mark_arg_nullable(1, false);
    ctx.set_arg_type(1, STRING_RESULT);
  }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &ctx) {
    const std::string dictionary = mysql::plugins::convert(
        ctx.get_arg<STRING_RESULT>(0), ctx.get_arg_charset(0),
        mysql::plugins::default_charset);

    const std::string term = mysql::plugins::convert(
        ctx.get_arg<STRING_RESULT>(1), ctx.get_arg_charset(1),
        mysql::plugins::default_charset);

    mysql::components::sql_context sql_ctx;

    std::string query(
        "DELETE FROM mysql.masking_dictionaries WHERE Dictionary=\"");
    mysql::components::escape_string_into(query, dictionary);
    query += "\" AND Term=\"";
    mysql::components::escape_string_into(query, term);
    query += "\"";

    if (sql_ctx.execute(query) == 0) {
      return std::nullopt;
    } else {
      return "1";
    }
  }
};

DECLARE_INT_UDF(gen_range_impl, gen_range);
DECLARE_STRING_UDF(gen_rnd_email_impl, gen_rnd_email);
DECLARE_STRING_UDF(gen_rnd_iban_impl, gen_rnd_iban);
DECLARE_STRING_UDF(gen_rnd_pan_impl, gen_rnd_pan);
DECLARE_STRING_UDF(gen_rnd_ssn_impl, gen_rnd_ssn);
DECLARE_STRING_UDF(gen_rnd_uk_nin_impl, gen_rnd_uk_nin);
DECLARE_STRING_UDF(gen_rnd_us_phone_impl, gen_rnd_us_phone);
DECLARE_STRING_UDF(gen_rnd_uuid_impl, gen_rnd_uuid);

DECLARE_STRING_UDF(mask_inner_impl, mask_inner);
DECLARE_STRING_UDF(mask_outer_impl, mask_outer);
DECLARE_STRING_UDF(mask_canada_sin_impl, mask_canada_sin);
DECLARE_STRING_UDF(mask_iban_impl, mask_iban);
DECLARE_STRING_UDF(mask_pan_impl, mask_pan);
DECLARE_STRING_UDF(mask_pan_relaxed_impl, mask_pan_relaxed);
DECLARE_STRING_UDF(mask_ssn_impl, mask_ssn);
DECLARE_STRING_UDF(mask_uk_nin_impl, mask_uk_nin);
DECLARE_STRING_UDF(mask_uuid_impl, mask_uuid);
DECLARE_STRING_UDF(gen_blocklist_impl, gen_blocklist);
DECLARE_STRING_UDF(gen_dictionary_impl, gen_dictionary);
DECLARE_STRING_UDF(masking_dictionary_remove_impl, masking_dictionary_remove);
DECLARE_STRING_UDF(masking_dictionary_term_add_impl,
                   masking_dictionary_term_add);
DECLARE_STRING_UDF(masking_dictionary_term_remove_impl,
                   masking_dictionary_term_remove);

std::array known_udfs{
    DECLARE_UDF_INFO(gen_range, INT_RESULT),
    DECLARE_UDF_INFO(gen_rnd_email, STRING_RESULT),
    DECLARE_UDF_INFO(gen_rnd_iban, STRING_RESULT),
    DECLARE_UDF_INFO(gen_rnd_pan, STRING_RESULT),
    DECLARE_UDF_INFO(gen_rnd_ssn, STRING_RESULT),
    DECLARE_UDF_INFO(gen_rnd_uk_nin, STRING_RESULT),
    DECLARE_UDF_INFO(gen_rnd_us_phone, STRING_RESULT),
    DECLARE_UDF_INFO(gen_rnd_uuid, STRING_RESULT),

    DECLARE_UDF_INFO(mask_inner, STRING_RESULT),
    DECLARE_UDF_INFO(mask_outer, STRING_RESULT),
    DECLARE_UDF_INFO(mask_canada_sin, STRING_RESULT),
    DECLARE_UDF_INFO(mask_iban, STRING_RESULT),
    DECLARE_UDF_INFO(mask_pan, STRING_RESULT),
    DECLARE_UDF_INFO(mask_pan_relaxed, STRING_RESULT),
    DECLARE_UDF_INFO(mask_ssn, STRING_RESULT),
    DECLARE_UDF_INFO(mask_uk_nin, STRING_RESULT),
    DECLARE_UDF_INFO(mask_uuid, STRING_RESULT),
    DECLARE_UDF_INFO(gen_blocklist, STRING_RESULT),
    DECLARE_UDF_INFO(gen_dictionary, STRING_RESULT),
    DECLARE_UDF_INFO(masking_dictionary_remove, STRING_RESULT),
    DECLARE_UDF_INFO(masking_dictionary_term_add, STRING_RESULT),
    DECLARE_UDF_INFO(masking_dictionary_term_remove, STRING_RESULT),
};

using udf_bitset_type =
    std::bitset<std::tuple_size<decltype(known_udfs)>::value>;
static udf_bitset_type registered_udfs;

/* The UDFs we will register. */
bool register_udfs() {
  mysqlpp::register_udfs(known_udfs, registered_udfs);

  return !registered_udfs.all();
}

bool unregister_udfs() {
  mysqlpp::unregister_udfs(known_udfs, registered_udfs);

  return !registered_udfs.none();
}
