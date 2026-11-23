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

#ifndef AUDIT_LOG_FILTER_PASSWORD_H_INCLUDED
#define AUDIT_LOG_FILTER_PASSWORD_H_INCLUDED

#include <cstddef>
#include <string>

namespace audit_log_filter::encryption {

inline constexpr std::size_t kGeneratedPasswordLength = 126;

/// Source of random bytes with the RAND_bytes() interface
using RandomBytesFunc = int (*)(unsigned char *buf, int num);

/**
 * @brief Generate a random log encryption password
 *
 * The password consists of kGeneratedPasswordLength characters from
 * [0-9A-Za-z] taken from RAND_bytes().
 *
 * @param [out] password Generated password, empty in case of failure
 * @return true in case of success, false otherwise
 */
bool generate_password(std::string &password);

/**
 * @brief Generate a log encryption password from the given random bytes
 *
 * @param [out] password Generated password, empty in case of failure
 * @param [in] random_bytes Source of random bytes, returns 1 on success
 * @return true in case of success, false otherwise
 */
bool generate_password(std::string &password, RandomBytesFunc random_bytes);

/**
 * @brief Check if a password has the structure of the passwords from the
 *        generator used before generate_password()
 *
 * That generator mapped kGeneratedPasswordLength consecutive libc random()
 * results onto 60 alphanumerics (no 'U' and 'V'). random() is an additive
 * feedback generator: each result is the sum of the results 3 and 31 calls
 * back, up to a carry and modulo 2^31, so the characters of such a password
 * are related in the same way for any seed.
 *
 * The relation may be broken at up to two places, where other threads called
 * rand() or random() while the password was generated. A password with more
 * such places is reported only when enough of the relation is left. Strings
 * that hold less than half of the 60 alphanumerics are never reported.
 *
 * Only the structure is checked: a password set by the user is reported as
 * well when it has this structure, like a copy of a generated password.
 *
 * @param [in] password Password to check
 * @return true if the password has the structure of the passwords from the
 *         earlier generator, false otherwise
 */
bool is_legacy_generated_password(const std::string &password);

}  // namespace audit_log_filter::encryption

#endif  // AUDIT_LOG_FILTER_PASSWORD_H_INCLUDED
