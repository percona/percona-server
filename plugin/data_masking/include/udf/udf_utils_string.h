#ifndef _UTILS_STRING_H
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

#define _UTILS_STRING_H

#include <string>

namespace mysql {
namespace plugins {
std::string &ltrim(std::string &s);

std::string mask_inner(const char *str, const long str_length,
                       const int margin1, const int margin2,
                       const char mask_char);

std::string mask_outer(const char *str, const unsigned long str_length,
                       const long margin1, const long margin2,
                       const char mask_char);

std::string &rtrim(std::string &s);

std::string &tolower(std::string &s);

std::string &trim(std::string &s);

}  // namespace plugins
}  // namespace mysql

#endif  // _UTILS_STRING_H
