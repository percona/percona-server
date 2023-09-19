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

#include "components/audit_log_filter/log_writer/file_name.h"

#include <regex>

namespace audit_log_filter::log_writer {

FileName FileName::from_path(std::filesystem::path filename) noexcept {
  bool is_compressed{false};
  bool is_encrypted{false};
  bool is_rotated{false};

  std::string key_id_str;
  std::string rotation_time_str;

  if (filename.has_extension() && filename.extension().compare(".enc") == 0) {
    is_encrypted = true;
    filename.replace_extension();
    key_id_str = filename.extension().string();
    filename.replace_extension();
  }

  if (filename.has_extension() && filename.extension().compare(".gz") == 0) {
    is_compressed = true;
    filename.replace_extension();
  }

  static const std::regex rotation_time_regex(R"(\.(\d{8}T\d{6}))");
  std::smatch pieces_match;

  while (filename.has_extension()) {
    auto extension_str = filename.extension().string();

    if (std::regex_match(extension_str, pieces_match, rotation_time_regex)) {
      is_rotated = true;
      rotation_time_str = pieces_match[1].str();
    }

    filename.replace_extension();
  }

  return FileName{
      is_compressed,     is_encrypted,          is_rotated,
      filename.string(), std::move(key_id_str), std::move(rotation_time_str)};
}

bool FileName::is_compressed() const noexcept { return m_is_compressed; }

bool FileName::is_encrypted() const noexcept { return m_is_encrypted; }

bool FileName::is_rotated() const noexcept { return m_is_rotated; }

std::string FileName::get_base_name() const noexcept { return m_base_name; }

std::string FileName::get_rotation_time() const noexcept {
  return m_rotation_time;
}

}  // namespace audit_log_filter::log_writer
