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

#ifndef AUDIT_LOG_FILTER_LOG_WRITER_FILE_BUFFER_H_INCLUDED
#define AUDIT_LOG_FILTER_LOG_WRITER_FILE_BUFFER_H_INCLUDED

#include "mysql/psi/mysql_cond.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/psi/psi_cond.h"
#include "mysql/psi/psi_mutex.h"
#include "mysql/service_mysql_alloc.h"

#include <cstddef>
#include <functional>
#include <memory>

namespace audit_log_filter::log_writer {

enum class FileBufferState { COMPLETE, INCOMPLETE };

using LogFileWriteFunc = std::function<void(const char *, size_t)>;

class FileBuffer {
 public:
  FileBuffer() = default;
  FileBuffer(FileBuffer &other) = delete;
  FileBuffer(FileBuffer &&other) = delete;
  ~FileBuffer();

  /**
   * @brief Init file write buffer.
   *
   * @param size Buffer size in bytes
   * @param drop_if_full Indicates if message should be dropped in case
   *                     buffer is full
   * @param write_func Callback function used to write data to a file
   * @return true in case of success, false otherwise
   */
  bool init(size_t size, bool drop_if_full,
            LogFileWriteFunc write_func) noexcept;

  /**
   * @brief Shutdown file write buffer.
   */
  void shutdown() noexcept;

  /**
   * @brief Write data to a buffer.
   *
   * @param buf Data to be written
   * @param len Data length
   */
  void write(const char *buf, size_t len) noexcept;

  /**
   * @brief Flush buffer content to a file.
   */
  void flush() noexcept;

  /**
   * @brief Check if flushing thread is stopped.
   *
   * @return true in case flushing thread is stopped, false otherwise
   */
  inline bool check_flush_stopped() const noexcept;

 private:
  /**
   * @brief Pause flushing thread.
   */
  void pause() noexcept;

  /**
   * @brief Resume flushing thread.
   */
  void resume() noexcept;

 private:
  char *m_buf{nullptr};
  size_t m_size{0};
  size_t m_write_pos{0};
  size_t m_flush_pos{0};
  pthread_t m_flush_worker_thread{0};
  bool m_stop_flush_worker{false};
  bool m_drop_if_full{false};
  LogFileWriteFunc m_write_func;
  mysql_mutex_t m_mutex;
  mysql_cond_t m_flushed_cond;
  mysql_cond_t m_written_cond;
  FileBufferState m_state;
};

}  // namespace audit_log_filter::log_writer

#endif  // AUDIT_LOG_FILTER_LOG_WRITER_FILE_BUFFER_H_INCLUDED
