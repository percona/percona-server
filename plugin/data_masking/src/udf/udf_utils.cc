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

unsigned int random_number(const unsigned int min, const unsigned int max) {
  std::random_device r;
  std::default_random_engine el(r());
  std::uniform_int_distribution<unsigned int> dist(min, max);

  return dist(el);
}

// Validate: https://stevemorse.org/ssn/cc.html
std::string random_credit_card() {
  std::string str;
  switch (random_number(3, 6)) {
    case 3:
      // American Express: 1st N 3, 2nd N [4,7], len 15
      str.assign("3")
          .append(std::to_string(random_number(4, 7)))
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
  str.append(std::to_string(10 - (check_sum % 10)));

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

}  // namespace plugins
}  // namespace mysql
