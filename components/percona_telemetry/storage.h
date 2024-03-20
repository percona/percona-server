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

#ifndef PERCONA_TELEMETRY_STORAGE_H
#define PERCONA_TELEMETRY_STORAGE_H

#include <chrono>
#include <memory>
#include <string>
#include "config.h"

class Logger;

class Storage {
 public:
  Storage(Config &config, Logger &logger);
  bool store_report(const std::string &report);

 private:
  void clean_old_reports(std::chrono::seconds current_time);

  Config &config_;
  Logger &logger_;
  std::string uuid_;
};

#endif /* PERCONA_TELEMETRY_STORAGE_H */
