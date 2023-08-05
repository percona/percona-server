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

#ifndef MASKING_FUNCTIONS_RANDOM_STRING_GENERATORS_HPP
#define MASKING_FUNCTIONS_RANDOM_STRING_GENERATORS_HPP

#include <cstddef>
#include <string>
#include <string_view>

namespace masking_functions {

// A set of functions that generate random strings of specific length / type /
// format. Used in implementations of Random Data Generation data-masking UDFs.

// An auxiliary enum used to specify desired character type in the
// 'random_character_class_string' function.
enum class character_class {
  lower_alpha,          // [a-z]
  upper_alpha,          // [A-Z]
  numeric,              // [0-9]
  alpha,                // [a-zA-Z]
  lower_alpha_numeric,  // [a-z0-9]
  upper_alpha_numeric,  // [A-Z0-9]
  alpha_numeric         // [a-zA-Z0-9]
};

// Returns a string containing 'length' random characters of the specified.
// 'char_class' type
std::string random_character_class_string(character_class char_class,
                                          std::size_t length);

// Returns a string containing 'length' random 'lower_alpha' characters.
inline std::string random_lower_alpha_string(std::size_t length) {
  return random_character_class_string(character_class::lower_alpha, length);
}

// Returns a string containing 'length' random 'upper_alpha' characters.
inline std::string random_upper_alpha_string(std::size_t length) {
  return random_character_class_string(character_class::upper_alpha, length);
}

// Returns a string containing 'length' random 'numeric' characters.
inline std::string random_numeric_string(std::size_t length) {
  return random_character_class_string(character_class::numeric, length);
}

// Returns a random number from the closed interval ['min', 'max'].
// The behavior is undefined if 'min' > 'max'.
std::size_t random_number(std::size_t min, std::size_t max);

// Returns a random American Express / Visa / Mastercard / Discover
// credit card number that passes basic checksum validation.
// https://stevemorse.org/ssn/cc.html
std::string random_credit_card();

// Generates a random Canada Social Insurance Number (SIN) in AAA-BBB-CCC
// format.
// E.g. 046-454-286
std::string random_canada_sin();

// Generates a random International Bank Account Number (IBAN).
// Returns a string containing the country code 'country'
// (2 latin uppercase characters) followed by 'length' random digits.
// This function does not calculate proper IBAN checksum (3rd and 4th
// digits) - those positions have randomly-generated digits.
std::string random_iban(std::string_view country, std::size_t length);

// Generates a random US Social Security Number (SSN) in AAA-BB-CCCC format.
// E.g. 951-26-0058
std::string random_ssn();

// Generates a random v4 Universal Unique Identifier (UUID).
// E.g. 82d9b7cc-7fad-481b-8eed-a27c11b4a404
std::string random_uuid();

// Generates a random United Kingdom National Insurance Number (UK NIN)
// in nine-character format.
// E.g. AA123456C
std::string random_uk_nin();

// Generates a random US phone number in 1-555-AAA-BBBB format.
// E.g. 1-555-682-5423
std::string random_us_phone();

}  // namespace masking_functions

#endif  // MASKING_FUNCTIONS_RANDOM_STRING_GENERATORS_HPP
