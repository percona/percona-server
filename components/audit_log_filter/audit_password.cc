/* Copyright (c) 2026 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#include "components/audit_log_filter/audit_password.h"

#include <openssl/rand.h>

#include <array>
#include <bitset>
#include <cstddef>
#include <string>
#include <string_view>

namespace audit_log_filter::encryption {

bool generate_password(std::string &password) {
  return generate_password(password, RAND_bytes);
}

bool generate_password(std::string &password, RandomBytesFunc random_bytes) {
  static const char alphanumerics[] =
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  static constexpr size_t alphanumerics_size = sizeof(alphanumerics) - 1;
  // Random bytes below this limit map evenly onto the alphanumerics
  static constexpr size_t rand_byte_limit = 256 - 256 % alphanumerics_size;

  password.clear();
  password.reserve(kGeneratedPasswordLength);

  unsigned char rand_bytes[kGeneratedPasswordLength];

  while (password.size() < kGeneratedPasswordLength) {
    if (random_bytes(rand_bytes, sizeof(rand_bytes)) != 1) {
      password.clear();
      return false;
    }

    for (const auto byte : rand_bytes) {
      if (byte < rand_byte_limit &&
          password.size() < kGeneratedPasswordLength) {
        password.push_back(alphanumerics[byte % alphanumerics_size]);
      }
    }
  }

  return true;
}

bool is_legacy_generated_password(const std::string &password) {
  static constexpr std::string_view legacy_alphanumerics{
      "0123456789ABCDEFGHIJKLMNOPQRSTWXYZabcdefghijklmnopqrstuvwxyz"};
  static constexpr size_t legacy_alphanumerics_size =
      legacy_alphanumerics.size();
  // random() returns the upper 31 bits of r[i] = r[i - 3] + r[i - 31], so
  // result[i] = result[i - 3] + result[i - 31] + carry - wrap * 2^31, where
  // carry and wrap are 0 or 1. The password holds the results modulo
  // legacy_alphanumerics_size.
  static constexpr size_t short_lag = 3;
  static constexpr size_t long_lag = 31;
  static constexpr size_t wrap_remainder =
      (size_t{1} << 31) % legacy_alphanumerics_size;
  // rand() and random() share their state within the process. Calls made by
  // other threads at one point of the password generation break the relation
  // for the characters that follow it by less than long_lag. Two such points
  // are accepted. Three of them can leave as few as two relations, which is
  // not enough to tell the password from any other string.
  static constexpr size_t max_interruptions = 2;
  static constexpr size_t max_broken_relations = max_interruptions * long_lag;
  // A string that was not generated keeps a relation with the probability
  // of 4 / legacy_alphanumerics_size. It keeps 33 of the 95 relations with
  // the probability below 1e-15.
  static constexpr size_t min_kept_relations = 33;
  static_assert(kGeneratedPasswordLength - long_lag >=
                max_broken_relations + min_kept_relations);
  // A generated password holds about 53 of the alphanumerics. Strings of a
  // few characters may keep the relation without being generated, like a
  // single '0' repeated.
  static constexpr size_t min_distinct_characters =
      legacy_alphanumerics_size / 2;

  if (password.size() != kGeneratedPasswordLength) {
    return false;
  }

  std::array<size_t, kGeneratedPasswordLength> indexes;
  std::bitset<legacy_alphanumerics_size> used_characters;

  for (size_t i = 0; i < kGeneratedPasswordLength; ++i) {
    indexes[i] = legacy_alphanumerics.find(password[i]);

    if (indexes[i] == std::string_view::npos) {
      return false;
    }

    used_characters.set(indexes[i]);
  }

  if (used_characters.count() < min_distinct_characters) {
    return false;
  }

  size_t broken_relations = 0;

  for (size_t i = long_lag; i < kGeneratedPasswordLength; ++i) {
    const size_t diff = (indexes[i] + 2 * legacy_alphanumerics_size -
                         indexes[i - short_lag] - indexes[i - long_lag]) %
                        legacy_alphanumerics_size;

    if (diff != 0 && diff != 1 &&
        diff != legacy_alphanumerics_size - wrap_remainder &&
        diff != legacy_alphanumerics_size - wrap_remainder + 1 &&
        ++broken_relations > max_broken_relations) {
      return false;
    }
  }

  return true;
}

}  // namespace audit_log_filter::encryption
