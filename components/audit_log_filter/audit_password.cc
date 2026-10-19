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

#include <cstddef>
#include <string>

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

}  // namespace audit_log_filter::encryption
