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

#ifndef LOG_RATE_LIMITER_H
#define LOG_RATE_LIMITER_H

#include "connection_control_interfaces.h"

#include <atomic>

namespace connection_control {

class LogRateLimiter {
 public:
  LogRateLimiter();

  void report_delayed_connection(const Sql_string &user_host,
                                 bool is_threshold_crossed,
                                 Error_handler *error_handler) noexcept;

 private:
  uint m_sent_messages_per_interval_count;
  uint64_t m_delayed_connections_per_interval_count;
  uint64_t m_interval_start_timestamp;
};

}  // namespace connection_control

#endif  // LOG_RATE_LIMITER_H
