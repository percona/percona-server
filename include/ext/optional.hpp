/* Copyright (c) 2020 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; version 2 of
   the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#ifndef EXT_OPTIONAL_HPP
#define EXT_OPTIONAL_HPP

#include <optional>

namespace ext {

// This using declaration is deprecated and should not be used.
// All new code should use std::optional directly.
//
// Originally, when MySQL code was still uising c++14, it used to refer to
// boost::string_view.
// In c++17 it is no longer needed and left here only for compatibility
// with old third party code.
using std::optional;

}  // namespace ext

#endif
