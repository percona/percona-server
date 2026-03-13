/* Copyright (c) 2025 Percona LLC and/or its affiliates. All rights reserved.

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

#ifndef AUDIT_LOG_FILTER_LOG_FILE_TIMESTAMP_H_INCLUDED
#define AUDIT_LOG_FILTER_LOG_FILE_TIMESTAMP_H_INCLUDED

#include <chrono>
#include <cstddef>
#include <filesystem>
#include <optional>

namespace audit_log_filter {

/**
 * @brief Combines a timestamp and a sequence number in a sortable object.
 *
 * It orders log file names, considering the following scenarios:
 * - non-rotated files (timestamp is empty)
 * - rotated files without timestamp collisions (seq == 0)
 * - rotated files with timestamp collisions (seq != 0)
 */
struct LogFileTimestamp {
  LogFileTimestamp() = default;
  LogFileTimestamp(const std::filesystem::path &path);

  friend bool operator<(const LogFileTimestamp &lhs,
                        const LogFileTimestamp &rhs) noexcept;

  std::optional<std::chrono::system_clock::time_point> timestamp;
  std::size_t seq = 0;
};

}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_LOG_FILE_TIMESTAMP_H_INCLUDED
