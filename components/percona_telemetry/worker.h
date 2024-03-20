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
