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

#include "components/audit_log_filter/log_file_timestamp.h"
#include <ctime>
#include <regex>
#include <tuple>

namespace audit_log_filter {

LogFileTimestamp::LogFileTimestamp(const std::filesystem::path &path) {
  // 1. Finds the first timestamp ("YYYYMMDDTHHMMSS").
  // 2. Optionally, looks for a sequence number ("-seq").
  // 3. Optionally, matches ".enc", which is used to check if we're dealing
  //    with rotation or encryption key timestamp.
  static const std::regex pattern(
      R"(^.*?\.(\d{4})(\d{2})(\d{2})T(\d{2})(\d{2})(\d{2})(-(\d+))?(\.enc)?.*)");

  auto filename = path.filename().string();
  std::smatch match;
  if (!std::regex_match(filename, match, pattern) || match[9].length() != 0) {
    // If we weren't able to find any timestamp, or we were able to find
    // only timestamp of an encryption key, we create empty (non-rotated)
    // timestamp.
    return;
  }

  std::tm tm = {};
  tm.tm_year = std::stoi(match[1]) - 1900;
  tm.tm_mon = std::stoi(match[2]) - 1;
  tm.tm_mday = std::stoi(match[3]);
  tm.tm_hour = std::stoi(match[4]);
  tm.tm_min = std::stoi(match[5]);
  tm.tm_sec = std::stoi(match[6]);
  tm.tm_isdst = -1;

  timestamp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
  seq = match[8].length() != 0 ? std::stoul(match[8]) : 0;
}

bool operator<(const LogFileTimestamp &lhs,
               const LogFileTimestamp &rhs) noexcept {
  if (!lhs.timestamp && !rhs.timestamp) return false;
  if (!lhs.timestamp) return false;
  if (!rhs.timestamp) return true;

  return std::tie(lhs.timestamp.value(), lhs.seq) <
         std::tie(rhs.timestamp.value(), rhs.seq);
}

}  // namespace audit_log_filter
