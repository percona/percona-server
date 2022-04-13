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

#ifndef AUDIT_LOG_FILTER_LOG_WRITER_STRATEGY_BASE_H_INCLUDED
#define AUDIT_LOG_FILTER_LOG_WRITER_STRATEGY_BASE_H_INCLUDED

#include <filesystem>
#include <string>

namespace audit_log_filter {

namespace log_writer {
class FileHandle;
}

namespace log_writer_strategy {

enum class AuditLogStrategyType {
  Asynchronous,
  Performance,
  Semisynchronous,
  Synchronous,
  // This item must be last in the list
  StrategiesCount
};

class FileWriterStrategyBase {
 public:
  /**
   * @brief Open file.
   *
   * @param file_handle File handle instance of type @ref log_writer::FileHandle
   * @param file_path File path
   * @param file_buffer_size File write buffer size in bytes
   * @return true in case file is opened successfully, false otherwise
   */
  virtual bool do_open_file(log_writer::FileHandle *file_handle,
                            const std::filesystem::path &file_path,
                            size_t file_buffer_size) noexcept = 0;

  /**
   * @brief Close file.
   *
   * @param file_handle File handle instance of type @ref log_writer::FileHandle
   * @return true in case file is closed successfully, false otherwise
   */
  virtual bool do_close_file(log_writer::FileHandle *file_handle) noexcept = 0;

  /**
   * @brief Write file.
   *
   * @param file_handle File handle instance of type @ref log_writer::FileHandle
   * @param record Data to be written
   */
  virtual void do_write(log_writer::FileHandle *file_handle,
                        const std::string &record) noexcept = 0;

  virtual ~FileWriterStrategyBase() = default;
};

template <AuditLogStrategyType StrategyType>
class FileWriterStrategy;

}  // namespace log_writer_strategy
}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_LOG_WRITER_STRATEGY_BASE_H_INCLUDED
