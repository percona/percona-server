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

#include "mysql/plugin_audit.h"

#include <filesystem>
#include <fstream>
#include <string>
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
   * @brief Get current file size in bytes.
   *
   * @return Current file size in bytes
   */
  [[nodiscard]] uint64_t get_file_size() const noexcept;

  /**
   * @brief Get current file path.
   *
   * @return Current log file path
   */
  [[nodiscard]] std::filesystem::path get_file_path() const noexcept;

  /**
   * @brief Flush data to a log file.
   */
  void flush() noexcept;

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
  static void remove_file_footer(const std::filesystem::path &file_path,
                                 const std::string &expected_footer) noexcept;

  /**
   * @brief Rotate file.
   *
   * @param current_file_path Current file path
   * @return Instance of std::error_code holding operation result
   */
  static std::error_code rotate(
      const std::filesystem::path &current_file_path) noexcept;

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

  /**
   * @brief Find path to not rotated log file if any.
   *
   * @param working_dir_name Working directory name
   * @param base_file_name Base file name
   * @return Path to not rotated log file
   */
  static std::filesystem::path get_not_rotated_file_path(
      const std::string &working_dir_name,
      const std::string &base_file_name) noexcept;

  /**
   * @brief Get list of currently existent audit log file names.
   *
   * @param working_dir_name Working directory name
   * @param file_name Base file name
   * @return List of audit log file names
   */
  static std::vector<std::string> get_log_names_list(
      const std::string &working_dir_name,
      const std::string &file_name) noexcept;

 private:
  std::fstream m_file;
  std::filesystem::path m_path;
  mysql_mutex_t m_lock;
};

}  // namespace audit_log_filter::log_writer

#endif  // AUDIT_LOG_FILTER_LOG_WRITER_FILE_HANDLE_H_INCLUDED
