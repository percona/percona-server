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

#include "mysql/components/services/bulk_data_service.h"

#include <cstddef>
#include <vector>

namespace Bulk_load {

struct ParserRow {
  explicit ParserRow(size_t num_columns);
  void set_column(char *column_begin, const char *column_end) noexcept;
  bool process_columns(Rows_text &text_rows) noexcept;
  void reset() noexcept;
  bool check_full() const noexcept;

  bool m_is_last_row;
  const size_t m_num_columns;
  size_t m_column_idx;
  size_t m_row_idx;
  std::vector<char *> m_column_ptrs;
  std::vector<size_t> m_column_lens;
};

}  // namespace Bulk_load
