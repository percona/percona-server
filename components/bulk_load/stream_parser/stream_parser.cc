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
#include <unordered_map>
#include <functional>

namespace Bulk_load {
namespace {

}  // namespace

StreamParser::StreamParser(size_t buffer_size,
                           size_t num_columns,
                           ParserParamsPtr parser_params,
                           DataStreamAbstract *data_stream)
  : m_buffer_size{buffer_size},
    m_parser_params{std::move(parser_params)},
    m_data_stream{data_stream},
    m_unparsed_stream_size{m_data_stream->get_data_size()},
    m_row{num_columns} {}

bool StreamParser::read_stream() noexcept {
  if (m_unparsed_stream_size == 0) {
    return true;
  }

  const auto len_to_read = std::min<size_t>(m_unparsed_stream_size,
                                            m_buffer_size);
  const auto read_len = m_data_stream->read(m_buffer.get(), len_to_read);

  if (read_len == 0) {
    LogComponentErr(ERROR_LEVEL, ER_BULK_LOADER_COMPONENT_ERROR,
                    "Failed to read data");
    return false;
  }

  m_unparsed_stream_size -= read_len;
  m_buffer_ptr = m_buffer_begin;
  m_buffer_end = m_buffer_begin + read_len;

  return true;
}

StreamParser::State StreamParser::handle_column_enclose_char() noexcept {
  if (m_column_begin == nullptr) {
    m_column_begin = m_buffer_ptr + 1;
    return State::GetColumnEnd;
  }

  if (m_column_end == nullptr) {
    m_column_end = m_buffer_ptr;
    m_row.set_column(m_column_begin, m_column_end);
    return m_row.check_full() ? State::GetRowEnd : State::GetColumnStart;
  }

  return State::Error;
}

StreamParser::State StreamParser::handle_column_terminator() noexcept {
  m_column_begin = nullptr;
  m_column_end = nullptr;
  return State::GetColumnStart;
}

StreamParser::State StreamParser::handle_row_terminator() noexcept {
  m_row.reset();
  m_column_begin = nullptr;
  m_column_end = nullptr;
  m_row.m_is_last_row = m_buffer_ptr + 1 == m_buffer_end;
  return State::RowReady;
}

ParserGenerator StreamParser::row_iterator() noexcept {
  static std::unordered_map<char, State (StreamParser::*)()> handler = {
    {static_cast<char>(m_parser_params->m_column_enclose_char),
       &StreamParser::handle_column_enclose_char},
    {m_parser_params->m_column_terminator[0],
       &StreamParser::handle_column_terminator},
    {m_parser_params->m_row_terminator[0],
       &StreamParser::handle_row_terminator},
  };

  m_buffer = std::unique_ptr<char[]>(new (std::nothrow) char[m_buffer_size]);

  if (m_buffer == nullptr) {
    LogComponentErr(ERROR_LEVEL, ER_BULK_LOADER_COMPONENT_ERROR,
                    "Failed to allocate buffer");
    co_return {true, nullptr};
  }

  m_buffer_begin = m_buffer.get();
  m_buffer_end = m_buffer_begin;
  m_buffer_ptr = m_buffer_begin;

  m_column_begin = nullptr;
  m_column_end = nullptr;

  auto state = State::GetColumnStart;

  while (m_unparsed_stream_size > 0) {
    if (m_buffer_ptr == m_buffer_end && !read_stream()) {
      co_return {true, nullptr};
    }

    while (m_buffer_ptr != m_buffer_end) {
      if (handler.contains(*m_buffer_ptr)) {
        state = (this->*handler[*m_buffer_ptr])();
      }

      if (state == State::RowReady) {
        co_yield {false, &m_row};
      }
      else if (state == State::Error) {
        co_return {true, nullptr};
      }

      ++m_buffer_ptr;
    }
  }

  co_return {false, nullptr};
}

}  // namespace Bulk_load
