/* Copyright (c) 2018, 2019 Francisco Miguel Biete Banon. All rights reserved.

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

#include "plugin/data_masking/include/udf/udf_utils.h"

#include <algorithm>
#include <cassert>

namespace mysql {
namespace plugins {
std::string random_string(unsigned long length, bool letter_start) {
  auto randchar_a = []() -> char {
    const std::string charset(
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz");
    std::random_device r;
    std::default_random_engine el(r());
    std::uniform_int_distribution<int> dist(0, charset.length() - 1);

    return charset[dist(el)];
  };
  auto randchar_an = []() -> char {
    const std::string charset(
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz");
    std::random_device r;
    std::default_random_engine el(r());
    std::uniform_int_distribution<int> dist(0, charset.length() - 1);

    return charset[dist(el)];
  };

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

std::string random_number(const unsigned int length) {
  auto randchar = []() -> char {
    const std::string charset("1234567890");
    std::random_device r;
    std::default_random_engine el(r());
    std::uniform_int_distribution<int> dist(0, charset.length() - 1);

    return charset[dist(el)];
  };

  std::string str(length, '0');
  std::generate_n(str.begin(), length, randchar);

  return str;
}

long random_number(const long min, const long max) {
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

  int check_sum = 0, n;
  int check_offset = (str.size() + 1) % 2;
  for (unsigned long i = 0; i < str.size(); i++) {
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

std::string random_us_phone() {
  // 1-555-AAA-BBBB

  return std::string("1")
      .append("-")
      .append("555")
      .append(random_number(3))
      .append("-")
      .append(random_number(4));
}

const char *Charset_service::arg_type("charset");
const char *Charset_service::service_name("mysql_udf_metadata");
SERVICE_TYPE(mysql_udf_metadata) *Charset_service::udf_metadata_service =
    nullptr;

bool Charset_service::init(SERVICE_TYPE(registry) * reg_srv) {
  my_h_service h_udf_metadata_service;
  if (!reg_srv || reg_srv->acquire(service_name, &h_udf_metadata_service))
    return true;
  udf_metadata_service = reinterpret_cast<SERVICE_TYPE(mysql_udf_metadata) *>(
      h_udf_metadata_service);
  return false;
}

bool Charset_service::deinit(SERVICE_TYPE(registry) * reg_srv) {
  if (!reg_srv) return true;
  using udf_metadata_t = SERVICE_TYPE_NO_CONST(mysql_udf_metadata);
  if (udf_metadata_service)
    reg_srv->release(reinterpret_cast<my_h_service>(
        const_cast<udf_metadata_t *>(udf_metadata_service)));
  return false;
}

/* Set the return value character set as latin1 */
bool Charset_service::set_return_value_charset(
    UDF_INIT *initid, const std::string &charset_name) {
  char *charset = const_cast<char *>(charset_name.c_str());
  if (udf_metadata_service->result_set(initid, Charset_service::arg_type,
                                       static_cast<void *>(charset))) {
    return true;
  }
  return false;
}

bool Charset_service::set_args_charset(UDF_ARGS *args,
                                       const std::string &charset_name) {
  char *charset = const_cast<char *>(charset_name.c_str());
  for (uint index = 0; index < args->arg_count; ++index) {
    if (args->arg_type[index] == STRING_RESULT &&
        udf_metadata_service->argument_set(args, Charset_service::arg_type,
                                           index,
                                           static_cast<void *>(charset))) {
      return true;
    }
  }
  return false;
}
}  // namespace plugins
}  // namespace mysql
