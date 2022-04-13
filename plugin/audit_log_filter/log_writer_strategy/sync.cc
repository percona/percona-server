/* Copyright (c) 2022 Percona LLC and/or its affiliates. All rights reserved.

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

#include "plugin/audit_log_filter/log_writer_strategy/sync.h"
#include "plugin/audit_log_filter/log_writer/file_handle.h"

namespace audit_log_filter::log_writer_strategy {

bool FileWriterStrategySync::do_open_file(
    log_writer::FileHandle *file_handle, const std::filesystem::path &file_path,
    const size_t file_buffer_size [[maybe_unused]]) noexcept {
  return file_handle->open_file(file_path);
}

bool FileWriterStrategySync::do_close_file(
    log_writer::FileHandle *file_handle) noexcept {
  return file_handle->close_file();
}

void FileWriterStrategySync::do_write(log_writer::FileHandle *file_handle,
                                      const std::string &record) noexcept {
  file_handle->write_file(record);
}

}  // namespace audit_log_filter::log_writer_strategy
