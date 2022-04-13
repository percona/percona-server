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

#ifndef AUDIT_LOG_FILTER_LOG_WRITER_FILE_HANDLE_H_INCLUDED
#define AUDIT_LOG_FILTER_LOG_WRITER_FILE_HANDLE_H_INCLUDED

#include "file_buffer.h"
#include "mysql/plugin_audit.h"

#include <filesystem>
#include <fstream>
#include <vector>

namespace audit_log_filter::log_writer {

struct PruneFileInfo {
  std::filesystem::path path;
  ulonglong size;
  ulonglong age;
};

using PruneFilesList = std::vector<PruneFileInfo>;

class FileHandle {
 public:
  /**
   * @brief Open file.
   *
   * @param file_path File path
   * @return true in case of success, false otherwise
   */
  bool open_file(std::filesystem::path file_path) noexcept;

  /**
   * @brief Close file.
   *
   * @return true in case of success, false otherwise
   */
  bool close_file() noexcept;

  /**
   * @brief Initialize file write buffer.
   *
   * @param buffer_size Buffer size in bytes
   * @param drop_if_full Indicates if messages should be dropped in case
   *                     buffer is full
   * @return true in case of success, false otherwise
   */
  bool init_buffer(size_t buffer_size, bool drop_if_full) noexcept;

  /**
   * @brief Close file close buffer.
   */
  void close_buffer() noexcept;

  /**
   * @brief Write record to a file.
   *
   * @param record Log record
   */
  void write_file(const std::string &record) noexcept;

  /**
   * @brief Write record to a file.
   *
   * @param record Log record
   * @param size Log record size
   */
  void write_file(const char *record, size_t size) noexcept;

  /**
   * @brief Write record to a buffer.
   *
   * @param record Log record
   */
  void write_buffer(const std::string &record) noexcept;

  /**
   * @brief Get current file size in bytes.
   *
   * @return Current file size in bytes
   */
  [[nodiscard]] uint64_t get_file_size() const noexcept;

  /**
   * @brief Get total logs size in bytes.
   *
   * @param working_dir_name Working directory name
   * @param file_name Log file name
   * @return Total logs size in bytes
   */
  [[nodiscard]] static uint64_t get_total_log_size(
      const std::string &working_dir_name,
      const std::string &file_name) noexcept;

  /**
   * @brief Remove log footer from the end of a file.
   *
   * @param file_path File path
   * @param expected_footer Expected log footer
   */
  void remove_file_footer(const std::filesystem::path &file_path,
                          const std::string &expected_footer) const noexcept;

  /**
   * @brief Rotate file.
   *
   * @param working_dir_name Working directory name
   * @param file_name File name
   * @return Instance of std::error_code holding operation result
   */
  static std::error_code rotate(const std::string &working_dir_name,
                                const std::string &file_name) noexcept;

  /**
   * @brief Get list of rotated log files which may be a subject for pruning.
   *
   * @param working_dir_name Working directory name
   * @param file_name File name
   * @return List of rotated log files
   */
  static PruneFilesList get_prune_files(const std::string &working_dir_name,
                                        const std::string &file_name) noexcept;

  /**
   * @brief Remove a file.
   *
   * @param path File path
   * @return true in case file removed successfully, false otherwise
   */
  static bool remove_file(const std::filesystem::path &path) noexcept;

 private:
  std::fstream m_file;
  std::filesystem::path m_path;
  FileBuffer m_buffer;
  mysql_mutex_t m_lock;
};

}  // namespace audit_log_filter::log_writer

#endif  // AUDIT_LOG_FILTER_LOG_WRITER_FILE_HANDLE_H_INCLUDED
