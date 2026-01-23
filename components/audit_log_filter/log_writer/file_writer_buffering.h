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

#ifndef AUDIT_LOG_FILTER_LOG_WRITER_FILE_WRITER_BUFFERING_H_INCLUDED
#define AUDIT_LOG_FILTER_LOG_WRITER_FILE_WRITER_BUFFERING_H_INCLUDED

#include "file_writer_decorator_base.h"

#include "mysql/psi/mysql_cond.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/psi/psi_cond.h"
#include "mysql/psi/psi_mutex.h"
#include "mysql/service_mysql_alloc.h"
#include "sql/malloc_allocator.h"

#include <cstddef>
#include <memory>
#include <vector>

namespace audit_log_filter::log_writer {

enum class FileBufferState { COMPLETE, INCOMPLETE };

class FileWriterBuffering final : public FileWriterDecoratorBase {
 public:
  FileWriterBuffering(std::unique_ptr<FileWriterBase> file_writer, size_t size,
                      bool drop_if_full);
  FileWriterBuffering(FileWriterBuffering &other) = delete;
  FileWriterBuffering(FileWriterBuffering &&other) = delete;
  ~FileWriterBuffering() final;

  /**
   * @brief Prepare writer for work with newly opened log file.
   *
   * @return true in case of success, false otherwise
   */
  bool open() noexcept override;

  /**
   * @brief Finish working with currently active log file before
   *        actually closing it.
   */
  void close() noexcept override;

  /**
   * @brief Write file.
   *
   * @param record Log record
   * @param size Log record size
   */
  void write(const char *record, size_t size) noexcept override;

 private:
  /**
   * @brief Flush worker method used by flush thread.
   */
  void flush_worker() noexcept;

  /**
   * @brief Shutdown file write buffer.
   */
  void shutdown() noexcept;

  /**
   * @brief Pause flushing thread.
   */
  void pause() noexcept;

  /**
   * @brief Resume flushing thread.
   */
  void resume() noexcept;

 private:
  const bool m_drop_if_full;
  std::vector<char, Malloc_allocator<char>> m_buf;
  size_t m_write_pos;
  size_t m_flush_pos;
  pthread_t m_flush_worker_thread;
  bool m_flush_worker_running;
  mysql_mutex_t m_mutex;
  mysql_cond_t m_flushed_cond;
  mysql_cond_t m_written_cond;
  FileBufferState m_state;
};

}  // namespace audit_log_filter::log_writer

#endif  // AUDIT_LOG_FILTER_LOG_WRITER_FILE_WRITER_BUFFERING_H_INCLUDED
