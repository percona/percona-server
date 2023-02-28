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

#include "file_writer_compressing.h"

namespace audit_log_filter::log_writer {

bool FileWriterDecoratorBase::init() noexcept { return m_file_writer->init(); }

bool FileWriterDecoratorBase::open() noexcept { return m_file_writer->open(); }

void FileWriterDecoratorBase::close() noexcept { m_file_writer->close(); }

void FileWriterDecoratorBase::write(const char *record, size_t size) noexcept {
  m_file_writer->write(record, size);
}

}  // namespace audit_log_filter::log_writer
