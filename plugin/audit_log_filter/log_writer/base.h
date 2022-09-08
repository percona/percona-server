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

#ifndef AUDIT_LOG_FILTER_LOG_WRITER_BASE_H_INCLUDED
#define AUDIT_LOG_FILTER_LOG_WRITER_BASE_H_INCLUDED

#include "plugin/audit_log_filter/audit_record.h"
#include "plugin/audit_log_filter/log_writer_strategy.h"

#include <memory>
#include <mutex>

namespace audit_log_filter {
namespace log_record_formatter {
class LogRecordFormatterBase;
}

namespace log_writer {

enum class AuditLogHandlerType {
  File,
  Syslog,
  TypesCount  // This item must be last in the list
};

class LogWriterBase {
 public:
  explicit LogWriterBase(
      std::unique_ptr<log_record_formatter::LogRecordFormatterBase> formatter);

  virtual ~LogWriterBase() = default;

  /**
   * @brief Open log writer.
   *
   * @return true in case of success, false otherwise
   */
  virtual bool open() noexcept = 0;

  /**
   * @brief Close log writer.
   *
   * @return true in case of success, false otherwise
   */
  virtual bool close() noexcept = 0;

  /**
   * @brief Write audit record to log.
   *
   * @param record Audit record represented as an instance of
   *               @ref AuditRecordVariant
   */
  void write(AuditRecordVariant record) noexcept;

  /**
   * @brief Write audit record to log.
   *
   * @param record String representation of audit record
   * @param print_separator Add lor record separator before a record
   *                        if set to true
   */
  virtual void write(const std::string &record,
                     bool print_separator) noexcept = 0;

  /**
   * @brief Rotate log file.
   */
  virtual void rotate() noexcept {}

  /**
   * @brief Prune outdated log files.
   */
  virtual void prune() noexcept = 0;

  /**
   * @brief Get current log file size in bytes.
   *
   * @return Current log file size in bytes
   */
  [[nodiscard]] virtual uint64_t get_log_size() const noexcept = 0;

  /**
   * @brief Get log formatter associated with the log writer.
   *
   * @return Pointer to log formatter instance
   */
  [[nodiscard]] log_record_formatter::LogRecordFormatterBase *
  get_formatter() noexcept {
    return m_formatter.get();
  }

 protected:
  /**
   * @brief Init log formatter.
   */
  void init_formatter() noexcept;

 private:
  std::unique_ptr<log_record_formatter::LogRecordFormatterBase> m_formatter;
  std::mutex m_write_mutex;
};

template <AuditLogHandlerType HandlerType>
class LogWriter;

}  // namespace log_writer
}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_LOG_WRITER_BASE_H_INCLUDED
