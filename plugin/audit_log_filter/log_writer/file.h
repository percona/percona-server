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

#ifndef AUDIT_LOG_FILTER_LOG_WRITER_FILE_H_INCLUDED
#define AUDIT_LOG_FILTER_LOG_WRITER_FILE_H_INCLUDED

#include "base.h"
#include "file_handle.h"

#include <mutex>
#include <string>

namespace audit_log_filter::log_writer {

class FileWriterBase;

using FileWriterPtr = std::unique_ptr<FileWriterBase>;

template <>
class LogWriter<AuditLogHandlerType::File> : public LogWriterBase {
 public:
  LogWriter() = delete;
  explicit LogWriter(
      std::unique_ptr<log_record_formatter::LogRecordFormatterBase> formatter);
  ~LogWriter() override;

  /**
   * @brief Init log writer.
   *
   * @return true in case of success, false otherwise
   */
  bool init() noexcept override;

  /**
   * @brief Open log writer.
   *
   * @return true in case of success, false otherwise
   */
  bool open() noexcept override;

  /**
   * @brief Close log writer.
   *
   * @return true in case of success, false otherwise
   */
  bool close() noexcept override;

  /**
   * @brief Write audit record to log.
   *
   * @param record String representation of audit record
   * @param print_separator Add lor record separator before a record
   *                        if set to true
   */
  void write(const std::string &record, bool print_separator) noexcept override;

  /**
   * @brief Get current log file size in bytes.
   *
   * @return Current log file size in bytes
   */
  [[nodiscard]] uint64_t get_log_size() const noexcept override;

  /**
   * @brief Prune outdated log files.
   */
  void prune() noexcept override;

 private:
  /**
   * @brief Rotate log file.
   *
   * @param record File rotation result
   */
  void rotate(FileRotationResult *result) noexcept override;

  /**
   * @brief Implement actual file opening logic.
   *
   * @return true in case file opened successfully, false otherwise
   */
  bool do_open_file() noexcept;

  /**
   * @brief Implement actual file closing logic.
   *
   * @return true in case file closed successfully, false otherwise
   */
  bool do_close_file() noexcept;

  /**
   * @brief Implement actual file writing logic.
   *
   * @param record String representation of audit record
   * @param print_separator Add lor record separator before a record
   *                        if set to true
   */
  void do_write(const std::string &record, bool print_separator) noexcept;

  /**
   * @brief Implement actual file rotation logic.
   *
   * @param record File rotation result
   */
  void do_rotate(FileRotationResult *result) noexcept;

 private:
  bool m_is_rotating;
  bool m_is_log_empty;
  bool m_is_opened;
  FileWriterPtr m_file_writer{};
  FileHandle m_file_handle;
  std::mutex m_write_lock;
};

using LogWriterFile = LogWriter<AuditLogHandlerType::File>;

}  // namespace audit_log_filter::log_writer

#endif  // AUDIT_LOG_FILTER_LOG_WRITER_FILE_H_INCLUDED
