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

#include "text_rows_reader.h"

#include "components/bulk_load/stream_parser/csv_parser.h"

#include <mysql/components/services/log_builtins.h>
#include <mysqld_error.h>

#include <cassert>
#include <iostream>
#include <unordered_map>

namespace Bulk_load {

TextRowsReader::TextRowsReader(size_t buffer_size,
                               size_t num_columns,
                               ParserParamsPtr parser_params,
                               DataSourceBase *data_source)
  : m_buffer_size{buffer_size},
    m_parser_params{std::move(parser_params)},
    m_data_source{data_source},
    m_remaining_size_to_read{m_data_source->get_data_size()},
    m_text_row{num_columns} {}

bool TextRowsReader::read_data() noexcept {
  if (m_remaining_size_to_read == 0) {
    return true;
  }

  const auto len_to_read = std::min<size_t>(m_remaining_size_to_read,
                                            m_buffer_size);
  const auto read_len = m_data_source->read(m_buffer.get(), len_to_read);

  if (read_len == 0) {
    LogComponentErr(ERROR_LEVEL, ER_BULK_LOADER_COMPONENT_ERROR,
                    "Failed to read data");
    return false;
  }

  m_remaining_size_to_read -= read_len;
  m_buffer_ptr = m_buffer_begin;
  m_buffer_end = m_buffer_begin + read_len;

  return true;
}

RoutineGenerator<RowResult> TextRowsReader::row_iterator() noexcept {
  m_buffer = std::unique_ptr<char[]>(new (std::nothrow) char[m_buffer_size]);

  if (m_buffer == nullptr) {
    LogComponentErr(ERROR_LEVEL, ER_BULK_LOADER_COMPONENT_ERROR,
                    "Failed to allocate buffer");
    co_return {true, nullptr};
  }

  m_buffer_begin = m_buffer.get();
  m_buffer_end = m_buffer_begin;
  m_buffer_ptr = m_buffer_begin;

  auto parser = CsvParser(&m_text_row,
                          m_parser_params->m_column_terminator,
                          m_parser_params->m_row_terminator,
                          m_parser_params->m_escape_char,
                          m_parser_params->m_column_enclose_char);

  auto state{CsvParserState::Ok};

  while (m_remaining_size_to_read > 0) {
    if ((state == CsvParserState::EndOfBuffer || m_buffer_ptr == m_buffer_end) && !read_data()) {
      co_return {true, nullptr};
    }

    while (m_buffer_ptr != m_buffer_end && state != CsvParserState::EndOfBuffer) {
      state = parser.parse_line(m_buffer_ptr, m_buffer_end);

      if (state == CsvParserState::Error) {
        co_return {true, nullptr};
      }

      m_text_row.m_is_last_row =
          m_buffer_ptr == m_buffer_end && m_remaining_size_to_read == 0;

      co_yield {false, &m_text_row};
    }
  }

  co_return {false, nullptr};
}

}  // namespace Bulk_load
