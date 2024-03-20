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

#include <mutex>

#include "logger.h"
#include "worker.h"

Worker::Worker(Config &config, Storage &storage, DataProvider &data_provider,
               Logger &logger)
    : config_(config),
      storage_(storage),
      data_provider_(data_provider),
      logger_(logger),
      stop_worker_thd_(false),
      caller_active_(ATOMIC_FLAG_INIT) {}

bool Worker::start() {
  std::thread thd(&Worker::worker_thd_fn, this);
  std::swap(thd_, thd);
  return false;
}

bool Worker::stop() {
  if (caller_active_.test_and_set()) {
    logger_.info("worker active. unload prohibited");
    return true;
  }
  stop_worker_thd_.store(true);
  cv_.notify_all();
  thd_.join();

  return false;
}

void Worker::worker_thd_fn() {
  std::mutex m;

  auto grace_interval = config_.grace_interval();
  logger_.info("Applying Telemetry grace interval %ld seconds",
               grace_interval.count());
  {
    std::unique_lock<std::mutex> lock(m);
    if (std::cv_status::timeout != cv_.wait_for(lock, grace_interval)) {
      // the thread was signaled to exit
      return;
    }
  }

  /* If the server is still initializing after waiting for grace_period time,
     data_provider will fail to scrape (all) metrics. It will provide empty
     report which wont't cause file creation in storage. */

  data_provider_.thread_access_begin();

  logger_.info("Scrape interval: %ld seconds",
               config_.scrape_interval().count());
  logger_.info("History keep interval: %ld seconds",
               config_.history_keep_interval().count());
  logger_.info("Storage dir: %s", config_.telemetry_storage_dir_path().c_str());

  while (true) {
    if (stop_worker_thd_.load()) {
      break;
    }

    if (!caller_active_.test_and_set()) {
      // Just in case, catch all exceptions.
      // Telemetry component should not impact the server in any way.
      try {
        storage_.store_report(data_provider_.get_report());
      } catch (const std::exception &e) {
        logger_.warning("%s", e.what());
      } catch (...) {
        logger_.warning("%s", "unknown exception caught");
      }

      caller_active_.clear();

      std::unique_lock<std::mutex> lock(m);
      cv_.wait_for(lock, config_.scrape_interval());
      // Timed out or signaled to exit. Go on anyway.
    }
  }

  data_provider_.thread_access_end();
}
