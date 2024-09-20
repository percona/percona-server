/* Copyright (c) 2024, Percona and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation. The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA */

#pragma once

#include <memory>
#include <string>
#include <utility>
#include <cstddef>

namespace Bulk_load {

struct ParserParams {
  ParserParams(std::string column_terminator, std::string row_terminator,
               unsigned char escape_char, unsigned char column_enclose_char,
               size_t count_row_skip)
      : m_column_terminator{std::move(column_terminator)},
        m_row_terminator{std::move(row_terminator)},
        m_escape_char{escape_char},
        m_column_enclose_char{column_enclose_char},
        m_count_row_skip{count_row_skip} {}
  std::string m_column_terminator;
  std::string m_row_terminator;
  unsigned char m_escape_char;
  unsigned char m_column_enclose_char;
  size_t m_count_row_skip;
};

using ParserParamsPtr = std::unique_ptr<ParserParams>;

}  // namespace Bulk_load
