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

#include "parser_row.h"

#include <cassert>

namespace Bulk_load {

ParserRow::ParserRow(size_t num_columns)
    : m_is_last_row{false},
      m_num_columns{num_columns},
      m_column_idx{0},
      m_row_idx{0} {
  m_column_ptrs.reserve(m_num_columns);
  m_column_lens.reserve(m_num_columns);
}

void ParserRow::set_column(char *column_begin, const char *column_end) noexcept {
  assert(column_begin <= column_end);
  m_column_ptrs[m_column_idx] = column_begin;
  m_column_lens[m_column_idx] = column_end - column_begin;
  ++m_column_idx;
}

bool ParserRow::process_columns(Rows_text &text_rows) noexcept {
  size_t column_idx = 0;
  bool is_success = text_rows.process_columns(
    m_row_idx,
    [this, &column_idx](Column_text &column, bool is_last_col) {
      if (column_idx >= m_num_columns) {
        return false;
      }
      if (column_idx + 1 == m_num_columns && !is_last_col) {
        return false;
      }
      if (column_idx + 1 < m_num_columns && is_last_col) {
        return false;
      }

      column.m_data_ptr = m_column_ptrs[column_idx];
      column.m_data_len = m_column_lens[column_idx];
      ++column_idx;

      return true;
    });

  ++m_row_idx;

  return is_success;
}

void ParserRow::reset() noexcept {
  m_column_idx = 0;
}

bool ParserRow::check_full() const noexcept {
  return m_column_idx >= m_num_columns;
}

}  // namespace Bulk_load
