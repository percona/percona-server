#ifndef _UTILS_H
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

#define _UTILS_H

#include <my_inttypes.h>
#include <mysql/components/component_implementation.h>
#include <mysql/components/services/registry.h>
#include <mysql/components/services/udf_metadata.h>
#include <mysql/udf_registration_types.h>
#include <random>
#include <string>


extern REQUIRES_SERVICE_PLACEHOLDER(mysql_udf_metadata);

namespace mysql {
namespace plugins {

constexpr std::string_view default_charset = "utf8mb4";

std::string random_string(std::size_t length, bool letter_start);

std::string random_number(std::size_t length);

std::size_t random_number(std::size_t min, std::size_t max);

std::string random_credit_card();

std::string random_iban(std::string_view const& country, std::size_t length);

std::string random_ssn();

std::string random_uuid();

std::string random_uk_nin();

std::string random_us_phone();

bool get_arg_character_set(UDF_ARGS *args, std::size_t index, std::string& charset);

bool set_return_value_charset(UDF_INIT *initid,
                              std::string_view const& charset = "utf8mb4");

bool set_return_value_charset_to_match_arg(UDF_INIT *initid, UDF_ARGS *args,
                                           std::size_t index);

}  // namespace plugins
}  // namespace mysql

#endif  //_UTILS_H
