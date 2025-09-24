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
#include "components/audit_log_filter/log_file_timestamp.h"

namespace audit_log_filter::log_writer {

class FileName {
 private:
  FileName(bool is_compressed, bool is_encrypted,
           std::string base_file_name_str, std::string key_id_str,
           const LogFileTimestamp &rotation_time)
      : m_is_compressed{is_compressed},
        m_is_encrypted{is_encrypted},
        m_base_name{std::move(base_file_name_str)},
        m_keyring_key_id{std::move(key_id_str)},
        m_rotation_time{rotation_time} {}

 public:
  static FileName from_path(std::filesystem::path filename) noexcept;

  [[nodiscard]] bool is_compressed() const noexcept;
  [[nodiscard]] bool is_encrypted() const noexcept;
  [[nodiscard]] bool is_rotated() const noexcept;

  [[nodiscard]] const std::string &get_base_name() const noexcept;
  [[nodiscard]] const LogFileTimestamp &get_rotation_time() const noexcept;

 private:
  bool m_is_compressed;
  bool m_is_encrypted;
  std::string m_base_name;
  std::string m_keyring_key_id;
  LogFileTimestamp m_rotation_time;
};

}  // namespace audit_log_filter::log_writer

#endif  // AUDIT_LOG_FILTER_LOG_WRITER_FILE_NAME_H_INCLUDED
