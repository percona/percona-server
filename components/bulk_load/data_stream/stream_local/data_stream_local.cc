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

#include "data_stream_local.h"

#include <mysql/components/services/log_builtins.h>
#include <mysqld_error.h>

#include <sstream>
#include <utility>
#include <cassert>

namespace Bulk_load {

DataStreamLocal::DataStreamLocal(std::filesystem::path path)
  : m_file_path{std::move(path)} {}

DataStreamLocal::~DataStreamLocal() {
  m_stream.close();
}

bool DataStreamLocal::open() noexcept {
  if (m_file_path.empty()) {
    LogComponentErr(ERROR_LEVEL, ER_BULK_LOADER_COMPONENT_ERROR,
                    "Empty file name");
    return false;
  }

  if (!std::filesystem::exists(m_file_path)) {
    LogComponentErr(ERROR_LEVEL, ER_BULK_LOADER_COMPONENT_ERROR,
                    "Cannot find file");
    return false;
  }

  m_data_size = std::filesystem::file_size(m_file_path);

  if (m_data_size == 0) {
    LogComponentErr(ERROR_LEVEL, ER_BULK_LOADER_COMPONENT_ERROR,
                    "Empty file");
    return false;
  }

  m_stream.open(m_file_path, std::ios_base::in | std::ios_base::binary);

  if (!m_stream.is_open()) {
    std::stringstream error;
    error << "Failed to open file: " << strerror(errno);
    LogComponentErr(ERROR_LEVEL, ER_BULK_LOADER_COMPONENT_ERROR,
                    error.str().c_str());
    return false;
  }

  return true;
}

size_t DataStreamLocal::read(char *buffer, size_t size) noexcept {
  m_stream.read(buffer, size);
  return m_stream.gcount();
}

}  // namespace Bulk_load
