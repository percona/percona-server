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

#ifndef AUDIT_LOG_FILTER_LOG_WRITER_FILE_NAME_H_INCLUDED
#define AUDIT_LOG_FILTER_LOG_WRITER_FILE_NAME_H_INCLUDED

#include <filesystem>
#include <string>

namespace audit_log_filter::log_writer {

class FileName {
 private:
  FileName(bool is_compressed, bool is_encrypted, bool is_rotated,
           std::string base_file_name_str, std::string key_id_str,
           std::string rotation_time_str)
      : m_is_compressed{is_compressed},
        m_is_encrypted{is_encrypted},
        m_is_rotated{is_rotated},
        m_base_name{std::move(base_file_name_str)},
        m_keyring_key_id{std::move(key_id_str)},
        m_rotation_time{std::move(rotation_time_str)} {}

 public:
  static FileName from_path(std::filesystem::path filename) noexcept;

  [[nodiscard]] bool is_compressed() const noexcept;
  [[nodiscard]] bool is_encrypted() const noexcept;
  [[nodiscard]] bool is_rotated() const noexcept;

  [[nodiscard]] std::string get_base_name() const noexcept;
  [[nodiscard]] std::string get_rotation_time() const noexcept;

 private:
  const bool m_is_compressed;
  const bool m_is_encrypted;
  const bool m_is_rotated;
  const std::string m_base_name;
  const std::string m_keyring_key_id;
  const std::string m_rotation_time;
};

}  // namespace audit_log_filter::log_writer

#endif  // AUDIT_LOG_FILTER_LOG_WRITER_FILE_NAME_H_INCLUDED
