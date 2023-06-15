#ifndef _UTILS_STRING_H
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

#define _UTILS_STRING_H

#include <string>
#include <string_view>

#define MYSQLPP_CHARSET_SUPPORT
#include <mysqlpp/udf_wrappers.hpp>

namespace mysql {
namespace plugins {
std::string &ltrim(std::string &s);

std::string convert(std::string_view const &src, const char *src_cs,
                    const char *dst_cs);

std::string decide_masking_char(mysqlpp::udf_context const &args, std::size_t argno,
                                const char *&original_charset,
                                std::string_view def = "X");

std::string mask_inner(const char *str, std::size_t str_length,
                       std::size_t margin1, std::size_t margin2,
                       const char *original_charset,
                       std::string_view mask_char);

std::string &rtrim(std::string &s);

std::string &tolower(std::string &s);

std::string &trim(std::string &s);

}  // namespace plugins
}  // namespace mysql

#endif  // _UTILS_STRING_H
