/* Copyright (c) 2023 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#include "components/audit_log_filter/json_reader/file_reader.h"

#include "components/audit_log_filter/audit_error_log.h"
#include "components/audit_log_filter/audit_log_reader.h"

#include <cstring>

namespace audit_log_filter::json_reader {

FileReader::~FileReader() { close_file_handle(); }

bool FileReader::init() noexcept { return true; }

bool FileReader::open(FileInfo *file_info) noexcept {
  m_fp = fopen(file_info->name.c_str(), "r");

  if (m_fp == nullptr) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to open file for reading: %s",
                    file_info->name.c_str());
    return false;
  }

  return true;
}

void FileReader::close() noexcept { close_file_handle(); }

ReadStatus FileReader::read(unsigned char *out_buffer,
                            const size_t out_buffer_size,
                            size_t *read_size) noexcept {
  *read_size = std::fread(out_buffer, 1, out_buffer_size, m_fp);

  if (*read_size != out_buffer_size) {
    const auto err = std::ferror(m_fp);

    if (err != 0) {
      LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, "Failed to read: %s",
                      std::strerror(err));
      return ReadStatus::Error;
    }
  }

  return *read_size == out_buffer_size ? ReadStatus::Ok : ReadStatus::Eof;
}

void FileReader::close_file_handle() noexcept {
  if (m_fp != nullptr) {
    fclose(m_fp);
    m_fp = nullptr;
  }
}

}  // namespace audit_log_filter::json_reader
