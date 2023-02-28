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

#include "file_writer.h"

#include "file_handle.h"

namespace audit_log_filter::log_writer {

FileWriter::FileWriter(FileHandle &file_handle, bool sync_on_write)
    : m_file_handle{file_handle}, m_sync_on_write{sync_on_write} {}

bool FileWriter::init() noexcept {
  return true;  // nothing to do
}

bool FileWriter::open() noexcept {
  return true;  // nothing to do
}

void FileWriter::close() noexcept { m_file_handle.flush(); }

void FileWriter::write(const char *record, size_t size) noexcept {
  m_file_handle.write_file(record, size);
}

}  // namespace audit_log_filter::log_writer
