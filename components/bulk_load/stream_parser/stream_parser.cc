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

#include "stream_parser.h"

#include <mysql/components/services/log_builtins.h>
#include <mysqld_error.h>

#include <cassert>
#include <coroutine>
#include <iostream>

namespace Bulk_load {

StreamParser::StreamParser(size_t num_columns,
                           ParserParamsPtr parser_params,
                           DataStreamAbstract *data_stream)
  : m_num_columns{num_columns},
    m_parser_params{std::move(parser_params)},
    m_data_stream{data_stream},
    m_unparsed_stream_size{m_data_stream->get_data_size()},
    m_current_buffer_idx{0},
    m_row{} {
  m_buffer.reset(static_cast<char *>(malloc(m_parser_params->m_buffer_size)));
  m_columns.reserve(m_num_columns);
  m_columns_length.reserve(m_num_columns);
}

ParserIterator StreamParser::row_iterator() noexcept {
  while (m_unparsed_stream_size > 0) {
    const auto len_to_read = std::min<size_t>(m_unparsed_stream_size, m_parser_params->m_buffer_size);
    m_unparsed_buffer_size = m_data_stream->read(m_buffer.get(), len_to_read);
    m_current_buffer_idx = 0;

    if (m_unparsed_buffer_size == 0) {
      LogComponentErr(ERROR_LEVEL, ER_BULK_LOADER_COMPONENT_ERROR,
                      "Failed to read data");
      co_return {true};
    }

    int column_data_begin = -1;
    int column_data_end = -1;
    size_t column_idx = 0;
    bool row_ready = false;

    while (m_unparsed_buffer_size > 0) {
      if (m_buffer.get()[m_current_buffer_idx] == m_parser_params->m_column_enclose_char) {
        if (column_data_begin == -1) {
          column_data_begin = column_data_end = m_current_buffer_idx;
        }
        else {
          column_data_end = m_current_buffer_idx;

          m_columns[column_idx] = column_data_begin;
          m_columns_length[column_idx] = column_data_end - column_data_begin;

          column_data_begin = column_data_end = -1;
          ++column_idx;
        }
      }
      else if (m_buffer.get()[m_current_buffer_idx] == m_parser_params->m_column_terminator[0]) {

      }
      else if (m_buffer.get()[m_current_buffer_idx] == m_parser_params->m_row_terminator[0]) {
        row_ready = true;
      }

      ++m_current_buffer_idx;
      --m_unparsed_buffer_size;

      if (row_ready) {
        co_yield {m_columns, m_columns_length};

//        for (size_t i = 0; i < column_idx; ++i) {
//          auto tmp = std::string(m_buffer.get()[m_columns[i]], m_columns_length[i]);
//          std::cout << "!!! row: " << tmp << std::endl;
//        }

        column_idx = 0;
      }
    }

    assert(m_unparsed_buffer_size <= m_unparsed_stream_size);
    m_unparsed_stream_size -= m_unparsed_buffer_size;
  }

  co_return {false};
}

}  // namespace Bulk_load
