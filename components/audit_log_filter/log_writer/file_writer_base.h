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

#ifndef AUDIT_LOG_FILTER_LOG_WRITER_FILE_WRITER_BASE_H_INCLUDED
#define AUDIT_LOG_FILTER_LOG_WRITER_FILE_WRITER_BASE_H_INCLUDED

#include <string>

namespace audit_log_filter::log_writer {

class FileHandle;

class FileWriterBase {
 public:
  virtual ~FileWriterBase() = default;

  /**
   * @brief Init file writer.
   *
   * @return true in case of success, false otherwise
   */
  virtual bool init() noexcept = 0;

  /**
   * @brief Prepare writer for work with newly opened log file.
   *
   * @return true in case of success, false otherwise
   */
  virtual bool open() noexcept = 0;

  /**
   * @brief Finish working with currently active log file before
   *        actually closing it.
   */
  virtual void close() noexcept = 0;

  /**
   * @brief Write file.
   *
   * @param record Log record
   * @param size Log record size
   */
  virtual void write(const char *record, size_t size) noexcept = 0;
};

}  // namespace audit_log_filter::log_writer

#endif  // AUDIT_LOG_FILTER_LOG_WRITER_FILE_WRITER_BASE_H_INCLUDED
