/* Copyright (c) 2024 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; version 2 of
   the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#ifndef PERCONA_TELEMETRY_LOGGER_H
#define PERCONA_TELEMETRY_LOGGER_H

#include <mysql/components/component_implementation.h>

#define LOG_COMPONENT_TAG CURRENT_COMPONENT_NAME_STR
#include <my_compiler.h>
#include <mysql/components/services/log_builtins.h>
#include "common.h"

class Logger {
 public:
  Logger(SERVICE_TYPE(log_builtins) & log_builtins_service,
         SERVICE_TYPE(log_builtins_string) & log_builtins_string_service,
         loglevel log_level = SYSTEM_LEVEL);

  ~Logger() = default;

  Logger(const Logger &rhs) = delete;
  Logger(Logger &&rhs) = delete;
  Logger &operator=(const Logger &rhs) = delete;
  Logger &operator=(Logger &&rhs) = delete;

  void info(const char *format, ...) MY_ATTRIBUTE((format(printf, 2, 3)));
  void warning(const char *format, ...) MY_ATTRIBUTE((format(printf, 2, 3)));
  void error(const char *format, ...) MY_ATTRIBUTE((format(printf, 2, 3)));

 private:
  loglevel log_level_;
};

#endif /* PERCONA_TELEMETRY_LOGGER_H */
