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

#ifndef EXT_STRING_VIEW_HPP
#define EXT_STRING_VIEW_HPP

#include <boost/utility/string_view.hpp>

namespace ext {

// TODO: the following 'using' directives can be changed to
//   using std::string_view;
// once MySQL source code switches to c++17
using boost::string_view;

}  // namespace ext

#endif
