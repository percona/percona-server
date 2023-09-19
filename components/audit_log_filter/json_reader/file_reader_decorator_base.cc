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

#include "components/audit_log_filter/json_reader/file_reader_decorator_base.h"

namespace audit_log_filter::json_reader {

FileReaderDecoratorBase::FileReaderDecoratorBase(
    std::unique_ptr<FileReaderBase> file_reader)
    : m_file_reader{std::move(file_reader)} {}

bool FileReaderDecoratorBase::init() noexcept { return m_file_reader->init(); }

bool FileReaderDecoratorBase::open(FileInfo *file_info) noexcept {
  return m_file_reader->open(file_info);
}

void FileReaderDecoratorBase::close() noexcept { m_file_reader->close(); }

ReadStatus FileReaderDecoratorBase::read(unsigned char *out_buffer,
                                         const size_t out_buffer_size,
                                         size_t *read_size) noexcept {
  return m_file_reader->read(out_buffer, out_buffer_size, read_size);
}

}  // namespace audit_log_filter::json_reader
