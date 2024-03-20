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
