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

#include "parser_params.h"
#include "parser_iterator.h"
#include "parser_row.h"
#include "components/bulk_load/data_stream/data_stream_abstract.h"

#include <vector>
#include <memory>
#include <cstddef>

namespace Bulk_load {

class StreamParser {
  enum class State {
    GetColumnStart,
    GetColumnEnd,
    GetRowEnd,
    RowReady,
    Error,
    End
  };

 public:
  StreamParser(size_t buffer_size,
               size_t num_columns,
               ParserParamsPtr parser_params,
               DataStreamAbstract *data_stream);

  ParserGenerator row_iterator() noexcept;

 private:
  State handle_column_enclose_char() noexcept;
  State handle_column_terminator() noexcept;
  State handle_row_terminator() noexcept;

  bool read_stream() noexcept;

 private:
  const size_t m_buffer_size;
  ParserParamsPtr m_parser_params;
  DataStreamAbstract *m_data_stream;
  size_t m_unparsed_stream_size;

  std::unique_ptr<char[]> m_buffer = nullptr;
  char *m_buffer_begin = nullptr;
  char *m_buffer_end = nullptr;
  char *m_buffer_ptr = nullptr;

  char *m_column_begin = nullptr;
  char *m_column_end = nullptr;

  ParserRow m_row;
};

}  // namespace Bulk_load
