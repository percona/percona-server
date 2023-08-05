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

#include "masking_functions/random_string_generators.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <random>
#include <string>
#include <string_view>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace {

std::default_random_engine &get_thread_local_prng() {
  // As the construction of the 'std::random_device' is a pretty heavy operation
  // (it includes opening `/dev/urandom` as a file, reading data from it and
  // closing the file handler) doing this for each individual "random char" call
  // is a huge overhead. Ideally, one global 'std::default_random_engine' object
  // per component would be enough but in this case we would have to protect it
  // with some synchronization primitives. 'thread_local' seems to be a
  // reasonable compromise here.

  // In the majority of implementations 'std::default_random_engine' is
  // 'std::minstd_rand'
  // ('std::linear_congruential_engine<uint_fast32_t, 48271, 0, 2147483647>') -
  // a Linear Congruential Engine (x[n+1] = (a * x[n] + c) mod m') with the
  // period equal to 2147483647 (about 2^31). It is not very hard to "hack"
  // provided that somebody has several consecutive generated numbers.
  // Potentially it can be substituted with `std::mersenne_twister_engine` which
  // has a period of 2^19937 but looks like an overkill for such a simple task.
  // Moreover, we are not using this random number generator for cryptography
  // where it is vitally important to have a high quality RNG, therefore it
  // seems OK to stick to a much simpler version.
  static thread_local std::default_random_engine instance{
      std::random_device{}()};
  return instance;
}

char calculate_luhn_checksum(const std::string &str) noexcept {
  std::size_t checksum = 0, n;
  std::size_t check_offset = (str.size() + 1U) % 2U;
  for (std::size_t i = 0; i < str.size(); i++) {
    // We can convert to int substracting the ASCII for 0
    n = static_cast<unsigned char>(str[i] - '0');
    if ((i + check_offset) % 2U == 0) {
      n *= 2U;
      checksum += n > 9U ? (n - 9U) : n;
    } else {
      checksum += n;
    }
  }

  return checksum % 10 == 0
             ? '0'
             : '0' + static_cast<unsigned char>(10U - (checksum % 10));
}

}  // anonymous namespace

namespace masking_functions {

std::string random_character_class_string(character_class char_class,
                                          std::size_t length) {
  if (length == 0U) return {};

  const std::string_view charset_full{
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz"
      "0123456789"};

  std::string_view selected_charset;

  switch (char_class) {
    case character_class::lower_alpha:
      selected_charset = charset_full.substr(10U + 26U, 26U);
      break;
    case character_class::upper_alpha:
      selected_charset = charset_full.substr(10U, 26U);
      break;
    case character_class::numeric:
      selected_charset = charset_full.substr(0U, 10U);
      break;
    case character_class::alpha:
      selected_charset = charset_full.substr(10U, 26U + 26U);
      break;
    case character_class::lower_alpha_numeric:
      selected_charset = charset_full.substr(10U + 26U, 26U + 10U);
      break;
    case character_class::upper_alpha_numeric:
      selected_charset = charset_full.substr(0U, 10U + 26U);
      break;
    case character_class::alpha_numeric:
      selected_charset = charset_full.substr(0U, 10U + 26U + 26U);
      break;
    default:
      assert(false);
  }

  auto &prng = get_thread_local_prng();

  std::uniform_int_distribution<std::size_t> dist(
      0U, selected_charset.length() - 1U);

  auto random_char = [&]() noexcept { return selected_charset[dist(prng)]; };

  std::string str(length, '-');
  std::generate_n(str.data(), length, random_char);

  return str;
}

std::size_t random_number(std::size_t min, std::size_t max) {
  auto &prng = get_thread_local_prng();
  std::uniform_int_distribution<std::size_t> dist(min, max);

  return dist(prng);
}

std::string random_canada_sin() {
  std::string str;
  str.append(random_numeric_string(3));
  str.append(random_numeric_string(3));
  str.append(random_numeric_string(2));

  str += calculate_luhn_checksum(str);

  str.insert(6, "-");
  str.insert(3, "-");
  return str;
}

// Validate: https://stevemorse.org/ssn/cc.html
std::string random_credit_card() {
  std::string str;
  switch (random_number(3, 6)) {
    case 3:
      // American Express: 1st N 3, 2nd N [4,7], len 15
      str.assign("3")
          .append(random_number(0, 1) == 0 ? "4" : "7")
          .append(random_numeric_string(12));
      break;
    case 4:
      // Visa: 1st N 4, len 16
      str.assign("4").append(random_numeric_string(14));
      break;
    case 5:
      // Master Card: 1st N 5, 2nd N [1,5], len 16
      str.assign("5")
          .append(std::to_string(random_number(1, 5)))
          .append(random_numeric_string(13));
      break;
    case 6:
      // Discover Card: 1st N 6, 2nd N 0, 3rd N 1, 4th N 1, len 16
      str.assign("6011").append(random_numeric_string(11));
      break;
  }

  str += calculate_luhn_checksum(str);

  assert(str.size() == 16 || str.size() == 15);
  return str;
}

std::string random_uuid() {
  static thread_local boost::uuids::random_generator gen;
  auto generated = gen();
  return boost::uuids::to_string(generated);
}

std::string random_ssn() {
  // AAA-GG-SSSS
  // Area, Group number, Serial number
  // Not valid number: Area: 000, 666, 900-999
  return std::to_string(random_number(900, 999))
      .append("-")
      .append(random_numeric_string(2))
      .append("-")
      .append(random_numeric_string(4));
}

std::string random_iban(std::string_view country, std::size_t length) {
  // TODO: consider adding IBAN checksum
  return std::string(country).append(random_numeric_string(length));
}

std::string random_uk_nin() {
  return std::string("AA").append(random_numeric_string(6)).append("C");
}

std::string random_us_phone() {
  // 1-555-AAA-BBBB

  return std::string("1")
      .append("-")
      .append("555")
      .append("-")
      .append(random_numeric_string(3))
      .append("-")
      .append(random_numeric_string(4));
}

}  // namespace masking_functions
