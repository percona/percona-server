
#include "buffered_error_log.h"
#include <string>

Buffered_error_logger buffered_error_log;
char *buffered_error_log_filename = nullptr;

void Buffered_error_logger::resize(std::size_t buffer_size) {
  std::lock_guard<std::mutex> lk{data_mtx};

  if (data.get() != nullptr && buffer_size == data->capacity()) {
    return;
  }

  // Write out what we have currently, that way we don't have
  // to deal with old data in the buffer
  write_to_disk_();

  if (buffer_size == 0) {
    data.reset();
    return;
  }

  data_t new_buffer(new std::string());
  new_buffer->reserve(buffer_size);
  data.swap(new_buffer);
}

Buffered_error_logger::~Buffered_error_logger() { write_to_disk(); }

void Buffered_error_logger::log(const char *msg, size_t len) {
  std::lock_guard<std::mutex> lk{data_mtx};
  if (data.get() == nullptr || data->capacity() == 0) return;
  const auto msg_end = data->size() + len;
  if (msg_end > data->capacity() - 1) {
    write_to_disk_();
  }
  *data += msg;
  *data += '\n';
}

void Buffered_error_logger::write_to_disk() {
  std::lock_guard<std::mutex> lk{data_mtx};
  write_to_disk_();
}

bool Buffered_error_logger::is_enabled() {
  std::lock_guard<std::mutex> lk{data_mtx};
  return data.get() != nullptr && data->size() != 0;
}

void Buffered_error_logger::write_to_disk_() {
  // Check the buffer before the filename: close() drops the buffer to mark
  // the logger as done, and after that the filename may already have been
  // freed by sys_var_end().
  if (data.get() == nullptr || data->size() == 0) {
    return;
  }
  if (buffered_error_log_filename == nullptr ||
      strlen(buffered_error_log_filename) == 0) {
    return;
  }
  auto fdd = fopen(buffered_error_log_filename, "a");
  fwrite(data->data(), data->size(), 1, fdd);
  fclose(fdd);

  // the C++ standard doesn't guarantee that clear doesn't deallocate the
  // buffer but it seems to be the case in libstdc++ and libc++ just to be on
  // the safe side, we call reserve after clear
  const auto curr_size = data->capacity();
  data->clear();
  data->reserve(curr_size);
}

void Buffered_error_logger::close() {
  std::lock_guard<std::mutex> lk{data_mtx};
  // Release the buffer instead of clearing buffered_error_log_filename.
  // The string is owned by the buffered_error_log_filename system variable
  // and has to stay reachable until sys_var_end() frees it; clearing it here
  // would leak whatever the last SET GLOBAL allocated. Dropping the buffer
  // makes write_to_disk_() a no-op, so the destructor - which runs after
  // sys_var_end() - never looks at the dangling filename either.
  data.reset();
}
