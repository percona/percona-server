/* Copyright (c) 2018, 2019 Francisco Miguel Biete Banon. All rights reserved.
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

#include "components/masking_functions/include/udf/udf_utils.h"

#include <algorithm>
#include <cassert>

namespace mysql {
namespace plugins {

std::string random_string(std::size_t length, bool letter_start) {
  std::random_device r;
  std::default_random_engine el(r());

  const std::string charset_char(
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz");
  std::uniform_int_distribution<int> dist_char(0, charset_char.length() - 1);

  auto randchar_a = [&]() -> char { return charset_char[dist_char(el)]; };

  const std::string charset_charal(
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz");
  std::uniform_int_distribution<int> dist_charal(0, charset_charal.length() - 1);

  auto randchar_an = [&]() -> char { return charset_charal[dist_charal(el)]; };

  std::string str(length, '0');
  if (letter_start) {
    std::string::iterator it = str.begin();
    std::generate_n(it++, 1, randchar_a);
    std::generate_n(it, length - 1, randchar_an);
  } else {
    std::generate_n(str.begin(), length, randchar_an);
  }

  return str;
}

std::string random_number(std::size_t length) {
    const std::string charset("1234567890");
    std::random_device r;
    std::default_random_engine el(r());
    std::uniform_int_distribution<int> dist(0, charset.length() - 1);

  auto randchar = [&]() -> char {

    return charset[dist(el)];
  };

  std::string str(length, '0');
  std::generate_n(str.begin(), length, randchar);

  return str;
}

std::size_t random_number(std::size_t min, std::size_t max) {
  std::random_device r;
  std::default_random_engine el(r());
  std::uniform_int_distribution<long> dist(min, max);

  return dist(el);
}

// Validate: https://stevemorse.org/ssn/cc.html
std::string random_credit_card() {
  std::string str;
  switch (random_number(3, 6)) {
    case 3:
      // American Express: 1st N 3, 2nd N [4,7], len 15
      str.assign("3")
          .append(std::to_string((random_number(0, 9) % 2) == 0 ? 4 : 7))
          .append(random_number(12));
      break;
    case 4:
      // Visa: 1st N 4, len 16
      str.assign("4").append(random_number(14));
      break;
    case 5:
      // Master Card: 1st N 5, 2nd N [1,5], len 16
      str.assign("5")
          .append(std::to_string(random_number(1, 5)))
          .append(random_number(13));
      break;
    case 6:
      // Discover Card: 1st N 6, 2nd N 0, 3rd N 1, 4th N 1, len 16
      str.assign("6011").append(random_number(11));
      break;
  }

  std::size_t check_sum = 0, n;
  std::size_t check_offset = (str.size() + 1) % 2;
  for (std::size_t i = 0; i < str.size(); i++) {
    n = str[i] - '0';  // We can convert to int substracting the ASCII for 0
    if ((i + check_offset) % 2 == 0) {
      n *= 2;
      check_sum += n > 9 ? (n - 9) : n;
    } else {
      check_sum += n;
    }
  }

  if (check_sum % 10 == 0) {
    str.append(std::to_string(0));
  } else {
    str.append(std::to_string(10 - (check_sum % 10)));
  }

  assert(str.size() == 16 || str.size() == 15);
  return str;
}

std::string random_uuid() {
  // AAA-GG-SSSS
  // Area, Group number, Serial number
  // Not valid number: Area: 000, 666, 900-999
  return std::string("")
      .append(random_number(8))
      .append("-")
      .append(random_number(4))
      .append("-")
      .append(random_number(4))
      .append("-")
      .append(random_number(4))
      .append("-")
      .append(random_number(12));
}

std::string random_ssn() {
  // AAA-GG-SSSS
  // Area, Group number, Serial number
  // Not valid number: Area: 000, 666, 900-999
  return std::to_string(random_number(900, 999))
      .append("-")
      .append(random_number(2))
      .append("-")
      .append(random_number(4));
}

std::string random_iban(std::string_view const& country, std::size_t length) {
  return std::string(country).append(random_number(length));
}

std::string random_uk_nin() {
  // 1-555-AAA-BBBB

  return std::string("AA").append(random_number(6)).append("C");
}

std::string random_us_phone() {
  // 1-555-AAA-BBBB

  return std::string("1")
      .append("-")
      .append("555")
      .append("-")
      .append(random_number(3))
      .append("-")
      .append(random_number(4));
}

bool set_return_value_charset(UDF_INIT *initid, std::string_view const& charset) {
  void *cs = const_cast<char *>(charset.data());
  if (mysql_service_mysql_udf_metadata->result_set(initid, "charset", cs))
    return true;

  return false;
}

bool get_arg_character_set(UDF_ARGS *args, std::size_t index, std::string & charset) {
  void *output = nullptr;
  if (args->arg_type[index] != STRING_RESULT) return true;

  if (mysql_service_mysql_udf_metadata->argument_get(args, "charset", index,
                                                     &output))
    return true;

  charset = static_cast<char *>(output);
  return false;
}

bool set_return_value_charset_to_match_arg(UDF_INIT *initid, UDF_ARGS *args,
                                           std::size_t index) {
  void *output = nullptr;
  if (args->arg_type[index] != STRING_RESULT) return true;
  if (mysql_service_mysql_udf_metadata->argument_get(args, "charset", index,
                                                     &output))
    return true;

  if (mysql_service_mysql_udf_metadata->result_set(initid, "charset", output))
    return true;

  return false;
}

}  // namespace plugins
}  // namespace mysql
