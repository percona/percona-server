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

#ifndef PERCONA_TELEMETRY_WORKER_H
#define PERCONA_TELEMETRY_WORKER_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <thread>

#include "config.h"
#include "data_provider.h"
#include "storage.h"

class Logger;

class Worker {
 public:
  Worker(Config &config, Storage &storage, DataProvider &data_provider,
         Logger &logger);

  bool start();
  bool stop();

 private:
  void worker_thd_fn();
  Config &config_;
  Storage &storage_;
  DataProvider &data_provider_;
  Logger &logger_;
  std::atomic_bool stop_worker_thd_;
  std::atomic_flag caller_active_;
  std::condition_variable cv_;

  std::thread thd_;
};

#endif /* PERCONA_TELEMETRY_WORKER_H */
