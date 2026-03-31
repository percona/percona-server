/* Copyright (c) 2022 Percona LLC and/or its affiliates. All rights reserved.

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

#define ALLOW_COMPONENT_INCLUDE // for plugin.h
#include "components/audit_log_filter/log_writer/file_handle.h"
#include "components/audit_log_filter/audit_error_log.h"
#include "components/audit_log_filter/audit_psi_info.h"
#include "components/audit_log_filter/log_writer/file_name.h"
#include "components/audit_log_filter/sys_vars.h"

#include <mysql/psi/mysql_mutex.h>
#include "my_dbug.h"
#include "my_sys.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <regex>
#include <sstream>
#include <string>

namespace {
const std::string kRotationTimeFormat{"%Y%m%dT%H%M%S"};

bool is_regular_file(const std::filesystem::directory_entry &entry) noexcept {
  std::error_code ec;
  return entry.is_regular_file(ec) && !ec;
}

bool get_entry_file_size(const std::filesystem::directory_entry &entry,
                         uintmax_t &size) noexcept {
  std::error_code ec;
  size = entry.file_size(ec);
  if (ec) {
    size = 0;
    return false;
  }
  return true;
}

template <typename Callback>
bool for_each_directory_entry(const std::string &working_dir_name,
                              const Callback &callback) noexcept {
  std::error_code ec;
  auto it = std::filesystem::directory_iterator(working_dir_name, ec);
  const auto end = std::filesystem::directory_iterator{};

  if (ec) {
    LogComponentErr(WARNING_LEVEL, ER_AUDIT_LOG_DIR_LISTING_FAILURE,
                    working_dir_name.c_str(), ec.message().c_str());
    return false;
  }

  while (it != end) {
    if (!callback(*it)) {
      return true;
    }

    it.increment(ec);
    if (ec) {
      LogComponentErr(WARNING_LEVEL, ER_AUDIT_LOG_DIR_ITERATE_FAILURE,
                      working_dir_name.c_str(), ec.message().c_str());
      return false;
    }
  }

  return true;
}
}

#if defined(HAVE_PSI_INTERFACE)
static PSI_mutex_key key_LOCK_audit_filter_service;
static PSI_mutex_info mutex_list[] = {
    {&key_LOCK_audit_filter_service, "file_handle::m_lock", PSI_FLAG_SINGLETON,
     PSI_VOLATILITY_UNKNOWN, PSI_DOCUMENT_ME}};
#else
#define key_LOCK_audit_filter_service nullptr
#endif /*HAVE_PSI_INTERFACE && !FLOGGER_NO_PSI*/

namespace audit_log_filter::log_writer {

bool FileHandle::open_file(const std::filesystem::path &file_path, bool direct_io,
                           bool flush_on_write) noexcept {
  assert(m_file < 0 && m_path.empty());
  m_path = std::move(file_path);
  m_flush_on_write = false;
  const auto path_str = m_path.string();

#ifdef O_DIRECT
  if (direct_io) {
    void *buf = nullptr;
    if (posix_memalign(&buf, kDirectIOBlockSize, kDirectIOBlockSize) != 0) {
      LogComponentErr(WARNING_LEVEL, ER_LOG_PRINTF_MSG,
                      "Audit Log Filter: failed to allocate aligned buffer "
                      "for O_DIRECT, falling back to buffered I/O");
      direct_io = false;
    } else {
      m_dio_buf.reset(static_cast<char *>(buf));
      std::memset(m_dio_buf.get(), 0, kDirectIOBlockSize);
    }
  }

  if (direct_io) {
    struct stat file_stat {};
    uint64_t file_size = 0;
    if (::stat(path_str.c_str(), &file_stat) == 0) {
      file_size = static_cast<uint64_t>(file_stat.st_size);
    }

    m_file_offset =
        file_size & ~(static_cast<uint64_t>(kDirectIOBlockSize) - 1);
    m_dio_buf_used = static_cast<size_t>(file_size - m_file_offset);

    if (m_dio_buf_used > 0) {
      int tmp_fd = ::open(path_str.c_str(), O_RDONLY);
      if (tmp_fd >= 0) {
        const auto bytes_read = ::pread(tmp_fd, m_dio_buf.get(), m_dio_buf_used,
                                        static_cast<off_t>(m_file_offset));
        ::close(tmp_fd);

        if (bytes_read != static_cast<ssize_t>(m_dio_buf_used)) {
          LogComponentErr(WARNING_LEVEL, ER_LOG_PRINTF_MSG,
                          "Audit Log Filter: failed to preload existing log "
                          "tail for O_DIRECT, falling back to buffered I/O");
          m_dio_buf.reset();
          m_dio_buf_used = 0;
          m_file_offset = 0;
          direct_io = false;
        }
      } else {
        LogComponentErr(WARNING_LEVEL, ER_LOG_PRINTF_MSG,
                        "Audit Log Filter: failed to preload existing log "
                        "tail for O_DIRECT, falling back to buffered I/O");
        m_dio_buf.reset();
        m_dio_buf_used = 0;
        m_file_offset = 0;
        direct_io = false;
      }
    }

    if (direct_io) {
      m_file = my_open(path_str.c_str(), O_RDWR | O_CREAT | O_DIRECT, MYF(0));
      if (m_file < 0) {
        LogComponentErr(WARNING_LEVEL, ER_LOG_PRINTF_MSG,
                        "Audit Log Filter: O_DIRECT open failed, "
                        "falling back to buffered I/O");
        m_dio_buf.reset();
        m_dio_buf_used = 0;
        m_file_offset = 0;
        direct_io = false;
      } else {
        m_direct_io = true;
        m_flush_on_write = flush_on_write;
      }
    }
  }
#else
  if (direct_io) {
    LogComponentErr(WARNING_LEVEL, ER_LOG_PRINTF_MSG,
                    "Audit Log Filter: O_DIRECT is not supported on this "
                    "platform, falling back to buffered I/O");
    direct_io = false;
  }
#endif

  if (!direct_io) {
    m_file = my_open(path_str.c_str(), O_WRONLY | O_APPEND | O_CREAT, MYF(0));
    m_direct_io = false;
  }

  if (m_file < 0) {
    m_path.clear();
    return false;
  }

#ifdef HAVE_PSI_INTERFACE
  mysql_mutex_register(AUDIT_LOG_FILTER_PSI_CATEGORY, mutex_list,
                       array_elements(mutex_list));
#endif /* HAVE_PSI_INTERFACE */

  mysql_mutex_init(key_LOCK_audit_filter_service, &m_lock, MY_MUTEX_INIT_FAST);

  return true;
}

bool FileHandle::close_file() noexcept {
  FileHandleLockGuard lock_guard{&m_lock};
  if (m_file < 0 && m_path.empty()) {
    return true;
  }

  if (m_direct_io) {
    flush_direct();
    m_dio_buf.reset();
    m_dio_buf_used.store(0, std::memory_order_relaxed);
    m_file_offset.store(0, std::memory_order_relaxed);
    m_direct_io = false;
  }

  const int close_result = (m_file >= 0) ? my_close(m_file, MYF(0)) : 0;
  m_file = -1;
  m_path.clear();

  mysql_mutex_destroy(&m_lock);

  return close_result == 0;
}

void FileHandle::write_file(const std::string &record) noexcept {
  write_file(record.c_str(), record.length());
}

void FileHandle::write_file(const char *record, const size_t size) noexcept {
  assert(m_file >= 0);
  if (m_direct_io) {
    write_file_direct(record, size);
    return;
  }
  [[maybe_unused]] const auto result = my_write(
      m_file, reinterpret_cast<const uchar *>(record), size, MYF(MY_NABP));
}

uint64_t FileHandle::get_file_size() const noexcept {
  assert(m_file >= 0);
  if (m_direct_io) {
    return m_file_offset.load(std::memory_order_relaxed) +
           m_dio_buf_used.load(std::memory_order_relaxed);
  }
  const auto size = my_seek(m_file, 0, MY_SEEK_END, MYF(0));
  return size == MY_FILEPOS_ERROR ? 0 : static_cast<uint64_t>(size);
}

std::filesystem::path FileHandle::get_file_path() const noexcept {
  assert(m_file >= 0);
  return m_path;
}

void FileHandle::flush() noexcept {
  assert(m_file >= 0);
  if (m_direct_io) {
    flush_direct();
  }
}

void FileHandle::sync() noexcept {
  assert(m_file >= 0);

  if (m_direct_io) {
    flush_direct();
  }

  DBUG_EXECUTE_IF("audit_log_filter_log_sync_call", {
    LogComponentErr(WARNING_LEVEL, ER_LOG_PRINTF_MSG,
                    "Audit Log Filter synchronous strategy issued a file sync");
  });

  if (my_sync(m_file, MYF(0)) != 0) {
    const auto path_str = m_path.string();
    std::string error_message = "Failed to sync audit log file: ";
    error_message += path_str;
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, error_message.c_str());
  }
}

void FileHandle::write_file_direct(const char *record,
                                   const size_t size) noexcept {
  assert(m_file >= 0 && m_direct_io && m_dio_buf != nullptr);

  const char *ptr = record;
  size_t remaining = size;
  size_t buf_used = m_dio_buf_used.load(std::memory_order_relaxed);
  uint64_t offset = m_file_offset.load(std::memory_order_relaxed);

  while (remaining > 0) {
    const size_t space = kDirectIOBlockSize - buf_used;
    const size_t to_copy = std::min(remaining, space);

    std::memcpy(m_dio_buf.get() + buf_used, ptr, to_copy);
    buf_used += to_copy;
    ptr += to_copy;
    remaining -= to_copy;

    if (buf_used == kDirectIOBlockSize) {
      auto result = my_pwrite(
          m_file, reinterpret_cast<const uchar *>(m_dio_buf.get()),
          kDirectIOBlockSize, static_cast<my_off_t>(offset), MYF(MY_NABP));
      if (result == MY_FILE_ERROR) {
        if (fallback_to_buffered_io(offset, buf_used) && remaining > 0) {
          write_file(ptr, remaining);
        }
        return;
      }
      offset += kDirectIOBlockSize;
      buf_used = 0;
    }
  }

  m_dio_buf_used.store(buf_used, std::memory_order_relaxed);
  m_file_offset.store(offset, std::memory_order_relaxed);

  if (m_flush_on_write) {
    flush_direct();
  }
}

void FileHandle::flush_direct() noexcept {
  assert(m_file >= 0 && m_direct_io && m_dio_buf != nullptr);

  const size_t buf_used = m_dio_buf_used.load(std::memory_order_relaxed);
  const uint64_t offset = m_file_offset.load(std::memory_order_relaxed);

  if (buf_used == 0) return;

  std::memset(m_dio_buf.get() + buf_used, 0, kDirectIOBlockSize - buf_used);

  auto result = my_pwrite(
      m_file, reinterpret_cast<const uchar *>(m_dio_buf.get()),
      kDirectIOBlockSize, static_cast<my_off_t>(offset), MYF(MY_NABP));
  DBUG_EXECUTE_IF("audit_log_filter_direct_flush_fail", {
    errno = EINVAL;
    result = MY_FILE_ERROR;
  });
  DBUG_EXECUTE_IF("audit_log_filter_direct_flush_fail_once", {
    errno = EINVAL;
    result = MY_FILE_ERROR;
    DBUG_SET("-d,audit_log_filter_direct_flush_fail_once");
  });

  if (result == MY_FILE_ERROR) {
    fallback_to_buffered_io(offset, buf_used);
    return;
  }

  if (::ftruncate(m_file, static_cast<off_t>(offset + buf_used)) != 0) {
    fallback_to_buffered_io(offset, buf_used);
  }
}

bool FileHandle::fallback_to_buffered_io(uint64_t offset,
                                         size_t buf_used) noexcept {
  assert(m_file >= 0 && m_direct_io && m_dio_buf != nullptr);

  const auto path_str = m_path.string();
  const auto logical_size = offset + buf_used;
  uint64_t actual_size = 0;
  struct stat file_stat {};
  File buffered_file = -1;
  bool using_current_fd = false;
  const bool force_reopen_fail = DBUG_EVALUATE_IF(
      "audit_log_filter_direct_fallback_reopen_fail", true, false);

  if (::fstat(m_file, &file_stat) == 0 && file_stat.st_size > 0) {
    actual_size = static_cast<uint64_t>(file_stat.st_size);
  }

#ifdef O_DIRECT
  const int current_flags = ::fcntl(m_file, F_GETFL);
  if (current_flags >= 0 &&
      ::fcntl(m_file, F_SETFL, (current_flags | O_APPEND) & ~O_DIRECT) == 0) {
    buffered_file = m_file;
    using_current_fd = true;
  }
#endif

  if (buffered_file < 0 && !force_reopen_fail) {
    buffered_file =
        my_open(path_str.c_str(), O_WRONLY | O_APPEND | O_CREAT, MYF(0));
  }
  if (buffered_file < 0) {
    if (force_reopen_fail) {
      errno = EMFILE;
    }
    LogComponentErr(WARNING_LEVEL, ER_LOG_PRINTF_MSG,
                    "Audit Log Filter: failed to reopen log file without "
                    "O_DIRECT after direct I/O failure");
    return false;
  }

  if (actual_size > logical_size &&
      ::ftruncate(buffered_file, static_cast<off_t>(logical_size)) != 0) {
    if (!using_current_fd) {
      my_close(buffered_file, MYF(0));
    }
    LogComponentErr(WARNING_LEVEL, ER_LOG_PRINTF_MSG,
                    "Audit Log Filter: failed to remove padded bytes while "
                    "falling back from O_DIRECT");
    return false;
  }

  const size_t persisted_tail = actual_size > offset
                                    ? static_cast<size_t>(std::min<uint64_t>(
                                          actual_size - offset, buf_used))
                                    : 0;

  if (persisted_tail < buf_used &&
      my_write(
          buffered_file,
          reinterpret_cast<const uchar *>(m_dio_buf.get() + persisted_tail),
          buf_used - persisted_tail, MYF(MY_NABP)) == MY_FILE_ERROR) {
    if (!using_current_fd) {
      my_close(buffered_file, MYF(0));
    }
    LogComponentErr(WARNING_LEVEL, ER_LOG_PRINTF_MSG,
                    "Audit Log Filter: failed to flush pending bytes while "
                    "falling back from O_DIRECT");
    return false;
  }

  if (!using_current_fd) {
    if (my_close(m_file, MYF(0)) != 0) {
      LogComponentErr(WARNING_LEVEL, ER_LOG_PRINTF_MSG,
                      "Audit Log Filter: failed to close O_DIRECT file handle "
                      "during fallback to buffered I/O");
    }
    m_file = buffered_file;
  }

  m_dio_buf.reset();
  m_dio_buf_used.store(0, std::memory_order_relaxed);
  m_file_offset.store(0, std::memory_order_relaxed);
  m_direct_io = false;
  m_flush_on_write = false;

  LogComponentErr(WARNING_LEVEL, ER_LOG_PRINTF_MSG,
                  "Audit Log Filter: O_DIRECT write failed, falling back to "
                  "buffered I/O");
  return true;
}

bool FileHandle::get_not_rotated_file_path(
    const std::string &working_dir_name, const std::string &file_name,
    std::filesystem::path &file_path) noexcept {
  file_path.clear();
  const auto base_file_name = FileName::from_path(file_name).get_base_name();

  if (base_file_name.empty()) {
    return true;
  }

  if (!for_each_directory_entry(working_dir_name, [&](const auto &entry) {
        if (!is_regular_file(entry)) {
          return true;
        }

        if (entry.path().filename().string().find(base_file_name) ==
            std::string::npos) {
          return true;
        }

        const auto parsed_file_name =
            FileName::from_path(entry.path().filename());
        if (parsed_file_name.get_base_name() == base_file_name &&
            !parsed_file_name.is_rotated()) {
          file_path = entry.path();
          return false;
        }
        return true;
      })) {
    return false;
  }

  return true;
}

bool FileHandle::get_total_log_size(const std::string &working_dir_name,
                                    const std::string &file_name,
                                    uint64_t &total_size) noexcept {
  total_size = 0;
  auto base_name = std::filesystem::path{file_name}.filename();
  while (base_name.has_extension()) {
    base_name.replace_extension();
  }

  if (base_name.empty()) {
    return true;
  }

  uint64_t size = 0;
  if (!for_each_directory_entry(working_dir_name, [&](const auto &entry) {
        auto entry_file_name = entry.path().filename();

        while (entry_file_name.has_extension()) {
          entry_file_name.replace_extension();
        }

        if (is_regular_file(entry) && entry_file_name == base_name) {
          uintmax_t fsize = 0;
          if (get_entry_file_size(entry, fsize)) {
            size += fsize;
          }
        }
        return true;
      })) {
    return false;
  }

  total_size = size;
  return true;
}

bool FileHandle::remove_file(const std::filesystem::path &path) noexcept {
  std::error_code ec;
  return std::filesystem::remove(path, ec);
}

bool FileHandle::remove_file_footer(
    const std::filesystem::path &file_path,
    const std::string &expected_footer) noexcept {
  assert(expected_footer.length() > 0);
  const auto path_str = file_path.string();
  const auto file = my_open(path_str.c_str(), O_RDWR, MYF(0));
  if (file < 0) {
    return false;
  }

  const auto current_size = my_seek(file, 0, MY_SEEK_END, MYF(0));
  if (current_size == MY_FILEPOS_ERROR) {
    my_close(file, MYF(0));
    return false;
  }

  if (static_cast<size_t>(current_size) < expected_footer.length()) {
    my_close(file, MYF(0));
    return true;
  }

  const auto footer_size = static_cast<my_off_t>(expected_footer.length());
  if (my_seek(file, current_size - footer_size, MY_SEEK_SET, MYF(0)) ==
      MY_FILEPOS_ERROR) {
    my_close(file, MYF(0));
    return false;
  }

  std::string file_footer(expected_footer.length(), '\0');
  if (my_read(file, reinterpret_cast<uchar *>(file_footer.data()),
              expected_footer.length(), MYF(MY_NABP)) == MY_FILE_ERROR) {
    my_close(file, MYF(0));
    return true;
  }

  if (expected_footer != file_footer) {
    my_close(file, MYF(0));
    return true;
  }

  const auto new_size = current_size - footer_size;
  if (my_chsize(file, new_size, 0, MYF(0)) != 0) {
    char errbuf[MYSYS_STRERROR_SIZE];
    LogComponentErr(WARNING_LEVEL, ER_AUDIT_LOG_FOOTER_REMOVE_FAILURE,
                    path_str.c_str(),
                    my_strerror(errbuf, sizeof(errbuf), my_errno()));
    my_close(file, MYF(0));
    return false;
  }

  my_close(file, MYF(0));

  return true;
}

void FileHandle::rotate(const std::filesystem::path &current_file_path,
                        FileRotationResult *result) noexcept {
  std::error_code ec;
  if (!std::filesystem::exists(current_file_path, ec)) {
    result->error_code = ec.value();
    if (ec) {
      result->status_string = ec.message();
    }
    return;
  }

  std::time_t t =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

  DBUG_EXECUTE_IF("audit_log_filter_debug_timestamp", {
    t = std::chrono::system_clock::to_time_t(
        SysVars::get_debug_time_point_for_rotation());
  });

  const auto filename_str = current_file_path.filename().string();
  auto first_ext_pos = filename_str.find_first_of('.');

  std::string base_file_name_str;
  std::string extensions_str;

  if (first_ext_pos == std::string::npos) {
    base_file_name_str = filename_str;
  } else {
    base_file_name_str = filename_str.substr(0, first_ext_pos);
    extensions_str = filename_str.substr(first_ext_pos);
  }

  std::filesystem::path new_file_path;
  std::string new_file_name_str;
  std::size_t seq = 0;

  do {
    std::string seq_str;
    if (seq != 0) {
      seq_str = "-" + std::to_string(seq);
    }
    seq++;

    std::stringstream new_file_name;
    new_file_name << base_file_name_str << "."
                  << std::put_time(std::localtime(&t),
                                   kRotationTimeFormat.c_str())
                  << seq_str << extensions_str;
    new_file_name_str = new_file_name.str();

    new_file_path = current_file_path;
    new_file_path.replace_filename(new_file_name_str);
  } while (std::filesystem::exists(new_file_path, ec));

  if (ec) {
    result->error_code = ec.value();
    result->status_string = ec.message();
    return;
  }

  std::filesystem::rename(current_file_path, new_file_path, ec);

  result->error_code = ec.value();

  if (result->error_code == 0) {
    result->status_string = new_file_name_str;
  } else {
    result->status_string = ec.message();
  }
}

bool FileHandle::get_prune_files(const std::string &working_dir_name,
                                 const std::string &file_name,
                                 PruneFilesList &prune_files) noexcept {
  prune_files.clear();

  const auto base_file_name = FileName::from_path(file_name).get_base_name();

  if (base_file_name.empty()) {
    return true;
  }

  auto time_now = std::chrono::system_clock::now();

  DBUG_EXECUTE_IF("audit_log_filter_debug_timestamp", {
    // This will return the time of the newest rotated log + 1 minute so
    // file age will be calculated properly for files which are subject
    // for age based pruning.
    time_now = SysVars::get_debug_time_point_for_rotation();
  });

  if (!for_each_directory_entry(working_dir_name, [&](const auto &entry) {
        if (is_regular_file(entry) &&
            entry.path().filename().string().find(base_file_name) !=
                std::string::npos) {
          auto parsed_file_name = FileName::from_path(entry.path().filename());

          if (parsed_file_name.is_rotated()) {
            auto timestamp =
                parsed_file_name.get_rotation_time().timestamp.value();
            uintmax_t fsize = 0;
            if (get_entry_file_size(entry, fsize)) {
              prune_files.push_back(
                  {entry.path(), fsize, time_now - timestamp});
            }
          }
        }
        return true;
      })) {
    return false;
  }

  return true;
}

bool FileHandle::get_log_names_list(
    const std::string &working_dir_name, const std::string &file_name,
    std::vector<std::string> &log_names) noexcept {
  log_names.clear();

  auto base_file_name =
      std::filesystem::path{file_name}.replace_extension().string();

  if (base_file_name.empty()) {
    return true;
  }

  if (!for_each_directory_entry(working_dir_name, [&](const auto &entry) {
        const auto name = entry.path().filename().string();

        if (is_regular_file(entry) &&
            name.find(base_file_name) != std::string::npos) {
          log_names.push_back(name);
        }
        return true;
      })) {
    return false;
  }

  return true;
}

}  // namespace audit_log_filter::log_writer
