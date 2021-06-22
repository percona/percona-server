
#pragma once

#include <stdio.h>
#include <cstring>
#include <memory>
#include <mutex>
#include <vector>

extern char *buffered_error_log_filename;

/** Stores log messages in a fixed size buffer, which can be dumped by large
 * chunks instead of line by line, to increase performance for high throughput
 * scenarios.
 *
 * The buffer is also dumped by the crash reporting code and during shutdown to
 * ensure that nothing is lost, only delayed.
 */
class Buffered_error_logger {
  // We are using a pointer to string to guarantee that we can decrease the size
  // of the buffer
  using data_t = std::unique_ptr<std::string>;
  // memory buffer storing the log messages
  std::mutex data_mtx;
  data_t data;

 public:
  void resize(std::size_t buffer_size);

  ~Buffered_error_logger();

  void log(const char *msg, size_t len);

  void write_to_disk();

  void close();

  bool is_enabled();

 private:
  // requires holding data_mtx
  void write_to_disk_();
};

extern Buffered_error_logger buffered_error_log;
