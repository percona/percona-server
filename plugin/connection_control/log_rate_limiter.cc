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

#include "log_rate_limiter.h"

#include "mysqld_error.h"

#include <chrono>

namespace {

constexpr uint64_t RATE_LIMIT_INTERVAL_SECONDS = 60;
constexpr uint64_t RATE_LIMIT_MESSAGES_PER_INTERVAL = 2;

}  // namespace

namespace connection_control {

LogRateLimiter::LogRateLimiter()
    : m_sent_messages_per_interval_count{0},
      m_delayed_connections_per_interval_count{0},
      m_interval_start_timestamp{static_cast<uint64_t>(
          std::chrono::duration_cast<std::chrono::seconds>(
              std::chrono::system_clock::now().time_since_epoch())
              .count())} {}

void LogRateLimiter::report_delayed_connection(
    const Sql_string &user_host, bool is_threshold_crossed,
    Error_handler *error_handler) noexcept {
  const uint64_t timestamp_now =
      std::chrono::duration_cast<std::chrono::seconds>(
          std::chrono::system_clock::now().time_since_epoch())
          .count();

  if (timestamp_now - m_interval_start_timestamp >
      RATE_LIMIT_INTERVAL_SECONDS) {
    if (m_delayed_connections_per_interval_count > 0 &&
        m_delayed_connections_per_interval_count >
            m_sent_messages_per_interval_count) {
      error_handler->handle_info(
          ER_CONN_CONTROL_DELAYED_CONN_STATS,
          static_cast<unsigned long long>(
              m_delayed_connections_per_interval_count),
          static_cast<unsigned long long>(timestamp_now -
                                          m_interval_start_timestamp));
    }

    m_interval_start_timestamp = timestamp_now;
    m_sent_messages_per_interval_count = 0;
    m_delayed_connections_per_interval_count = 0;
  }

  m_delayed_connections_per_interval_count++;

  if (is_threshold_crossed &&
      m_sent_messages_per_interval_count < RATE_LIMIT_MESSAGES_PER_INTERVAL) {
    m_sent_messages_per_interval_count++;

    if (m_sent_messages_per_interval_count >=
        RATE_LIMIT_MESSAGES_PER_INTERVAL) {
      const auto remaining_interval_seconds =
          RATE_LIMIT_INTERVAL_SECONDS -
          (timestamp_now - m_interval_start_timestamp);
      error_handler->handle_info(
          ER_CONN_CONTROL_FAILED_CONN_THRESHOLD_REACHED_WITH_WARN,
          user_host.c_str(),
          static_cast<unsigned long long>(remaining_interval_seconds));
    } else {
      error_handler->handle_info(ER_CONN_CONTROL_FAILED_CONN_THRESHOLD_REACHED,
                                 user_host.c_str());
    }
  }
}

}  // namespace connection_control
