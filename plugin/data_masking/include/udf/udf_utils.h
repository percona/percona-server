#ifndef _UTILS_H
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

#define _UTILS_H

#include <random>
#include <string>

namespace mysql {
namespace plugins {
std::string random_string(unsigned long length, bool letter_start);

std::string random_number(const unsigned int length);

unsigned int random_number(const unsigned int min, const unsigned int max);

std::string random_credit_card();

std::string random_ssn();

std::string random_us_phone();

}  // namespace plugins
}  // namespace mysql

#endif  //_UTILS_H
