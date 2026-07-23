/* Copyright (c) 2015, 2026, Oracle and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is designed to work with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have either included with
   the program or referenced in the documentation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

#ifndef XCOM_STANDALONE
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstdio>
#include <ctime>

#include "my_dir.h"
#include "my_io.h"
#include "my_sys.h"
#include "my_systime.h"
#endif /* XCOM_STANDALONE */

#include "plugin/group_replication/libmysqlgcs/include/mysql/gcs/gcs_logging_system.h"

size_t Gcs_file_sink::m_max_file_size = 0;

void *consumer_function(void *ptr);

Gcs_async_buffer::Gcs_async_buffer(Sink_interface *sink, int buffer_size)
    : m_buffer(std::vector<Gcs_log_event>(buffer_size)),
      m_buffer_size(buffer_size),
      m_write_index(0),
      m_read_index(0),
      m_number_entries(0),
      m_terminated(false),
      m_initialized(false),
      m_sink(sink),
      m_consumer(new My_xp_thread_impl()),
      m_wait_for_events_cond(new My_xp_cond_impl()),
      m_free_buffer_cond(new My_xp_cond_impl()),
      m_free_buffer_mutex(new My_xp_mutex_impl()) {}

Gcs_async_buffer::~Gcs_async_buffer() {
  delete m_consumer;
  delete m_wait_for_events_cond;
  delete m_free_buffer_cond;
  delete m_free_buffer_mutex;
  delete m_sink;
}

Sink_interface *Gcs_async_buffer::get_sink() const { return m_sink; }

enum_gcs_error Gcs_async_buffer::initialize() {
  int ret_thread;
  enum_gcs_error const ret_sink = m_sink->initialize();

  if (ret_sink == GCS_NOK) {
    /* purecov: begin deadcode */
    std::cerr << "Unable to create associated sink." << std::endl;
    return GCS_NOK;
    /* purecov: end */
  }

  if (m_initialized) return GCS_OK;

  /*
    Set that log events are not ready to be consumed. This is necessary when
    the same Gcs_async_buffer is reused several times and is not dynamically
    created every time group replication is started.
  */
  for (auto &it : m_buffer) {
    it.set_event(false);
  }

  m_wait_for_events_cond->init(
      key_GCS_COND_Gcs_async_buffer_m_wait_for_events_cond);
  m_free_buffer_cond->init(key_GCS_COND_Gcs_async_buffer_m_free_buffer_cond);
  m_free_buffer_mutex->init(key_GCS_MUTEX_Gcs_async_buffer_m_free_buffer_mutex,
                            nullptr);

  m_terminated = false;
  if ((ret_thread =
           m_consumer->create(key_GCS_THD_Gcs_ext_logger_impl_m_consumer,
                              nullptr, consumer_function, (void *)this))) {
    /* purecov: begin deadcode */
    std::cerr << "Unable to create Gcs_async_buffer consumer thread, "
              << ret_thread << std::endl;

    m_wait_for_events_cond->destroy();
    m_free_buffer_cond->destroy();
    m_free_buffer_mutex->destroy();

    return GCS_NOK;
    /* purecov: end */
  }

  m_initialized = true;

  return GCS_OK;
}

enum_gcs_error Gcs_async_buffer::finalize() {
  if (!m_initialized) return GCS_OK;

  // Wake up consumer before leaving
  m_free_buffer_mutex->lock();
  m_terminated = true;
  m_free_buffer_cond->broadcast();
  m_wait_for_events_cond->signal();
  m_free_buffer_mutex->unlock();

  // Wait for consumer to finish processing events
  m_consumer->join(nullptr);

  m_wait_for_events_cond->destroy();
  m_free_buffer_cond->destroy();
  m_free_buffer_mutex->destroy();

  m_sink->finalize();

  m_initialized = false;

  return GCS_OK;
}

Gcs_log_event &Gcs_async_buffer::get_entry() {
  /*
    Atomically get an entry to a buffer to write the message content into it,
    provided there is one. If there is no free entry, the caller will block
    until one is available.

    Note that the caller will be responsible for copying any message content
    to the buffer directly.
  */
  return m_buffer[get_write_index()];
}

int64_t Gcs_async_buffer::get_write_index() {
  int64_t write_index = 0;

  /*
    Get an entry to a buffer to write the message content into it, provided
    there is one. If there is no free entry, the caller will block until
    the consumer thread reads some entries and write them to another sink.

    The m_number_entries member variable determines how many pending requests
    there are. Note that the copy to the buffer is made outside the critical
    section. In order to notify the consumer that the content is ready, an
    atomic variable is updated inside the set_event method.
  */
  m_free_buffer_mutex->lock();
  assert(m_number_entries <= m_buffer_size && m_number_entries >= 0);
  while (m_number_entries == m_buffer_size) {
    /* purecov: begin deadcode */
    wake_up_consumer();
    m_free_buffer_cond->wait(m_free_buffer_mutex->get_native_mutex());
    /* purecov: end */
  }
  write_index = m_write_index++;
  m_number_entries++;
  m_free_buffer_mutex->unlock();

  return get_index(write_index);
}

void Gcs_async_buffer::notify_entry(Gcs_log_event &buffer_entry) {
  buffer_entry.set_event(true);

  /*
    Wake up the consumer that may have slept because, previously, there was
    nothing to be consumed.
  */
  wake_up_consumer();
}

inline void Gcs_async_buffer::produce_events(const char *message,
                                             size_t message_size) {
  Gcs_log_event &entry = get_entry();
  char *buffer = entry.get_buffer();
  size_t const size = std::min(entry.get_max_buffer_size(), message_size);
  strncpy(buffer, message, size);
  entry.set_buffer_size(size);
  notify_entry(entry);
}

inline void Gcs_async_buffer::produce_events(const std::string &message) {
  produce_events(message.c_str(), message.length());
}

void Gcs_async_buffer::consume_events() {
  int64_t number_entries = 0;
  bool is_terminated = false;

  do {
    /*
      Fetch information on possible new entries and whether the asynchronous
      queue has been stopped or not. Note that it is better to do it here and
      acquire the locks again rather than letting the consumer thread sleep.
      User threads might have produced new content after the number of entries
      variable has been updated further down.
    */
    m_free_buffer_mutex->lock();
    number_entries = m_number_entries;
    is_terminated = m_terminated;

    if (number_entries == 0) {
      if (!is_terminated) sleep_consumer();
      m_free_buffer_mutex->unlock();
    } else {
      /*
        Consume the entries in chunks to avoid acquiring and releasing the
        mutex for every entry. However, try to avoid really big chunks to
        give the user threads the chance to produce new content if there are
        no free slots.
      */
      m_free_buffer_mutex->unlock();
      int64_t to_read, read;
      int64_t const max_entries = (m_buffer_size / 25);
      assert(number_entries != 0);
      if (number_entries > max_entries && max_entries != 0)
        /* purecov: begin deadcode */
        number_entries = max_entries;
      /* purecov: end */
      to_read = number_entries, read = number_entries;
      while (to_read != 0) {
        m_buffer[get_index(m_read_index)].flush_event(*m_sink);
        m_read_index++;
        to_read--;
      }

      /*
        Update the number of entries consumed thus allowing user threads
        to produce new content if there were no free slots available before.
      */
      m_free_buffer_mutex->lock();
      m_number_entries -= read;
      m_free_buffer_cond->signal();
      m_free_buffer_mutex->unlock();
    }
  } while (!is_terminated || number_entries != 0);
}

void *consumer_function(void *ptr) {
  auto *l = static_cast<Gcs_async_buffer *>(ptr);
  l->consume_events();

  My_xp_thread_util::exit(nullptr);

  return nullptr;
}

std::string Gcs_async_buffer::get_information() const {
  std::stringstream ss;

  ss << "asynchronous:"
     << ":" << m_sink->get_information();

  return ss.str();
}

/* purecov: begin deadcode */
Gcs_output_sink::Gcs_output_sink() : m_initialized(false) {}

enum_gcs_error Gcs_output_sink::initialize() {
  int ret_out = 0;

  if (!m_initialized) {
    ret_out = setvbuf(stdout, nullptr, _IOLBF, BUFSIZ);
  }

  if (ret_out == 0) {
    m_initialized = true;
  } else {
    int errno_gcs = 0;
#if defined(_WIN32)
    errno_gcs = WSAGetLastError();
#else
    errno_gcs = errno;
#endif
    std::cerr << "Unable to invoke setvbuf correctly! " << strerror(errno_gcs)
              << std::endl;
  }
  return ret_out ? GCS_NOK : GCS_OK;
}

enum_gcs_error Gcs_output_sink::finalize() { return GCS_OK; }

void Gcs_output_sink::log_event(const std::string &message) {
  std::cout << message;
}

void Gcs_output_sink::log_event(const char *message, size_t message_size) {
  std::cout.width(message_size);
  std::cout << message;
}

const std::string Gcs_output_sink::get_information() const {
  return std::string("stdout");
}

Gcs_default_logger::Gcs_default_logger(Gcs_async_buffer *sink) : m_sink(sink) {}

enum_gcs_error Gcs_default_logger::initialize() { return m_sink->initialize(); }

enum_gcs_error Gcs_default_logger::finalize() { return m_sink->finalize(); }

void Gcs_default_logger::log_event(const gcs_log_level_t level,
                                   const std::string &message) {
  std::stringstream log;
  log << gcs_log_levels[level] << message << std::endl;
  m_sink->produce_events(log.str());
}

Gcs_default_debugger::Gcs_default_debugger(
    Gcs_async_buffer *sink,
    std::shared_ptr<Clock_timestamp_interface> clock_timestamp_provider)
    : m_sink(sink), m_clock_timestamp_provider(clock_timestamp_provider) {}

enum_gcs_error Gcs_default_debugger::initialize() {
  return m_sink->initialize();
}

enum_gcs_error Gcs_default_debugger::finalize() { return m_sink->finalize(); }

/**
  Reference to the default debugger which is used internally by GCS and XCOM.
*/
Gcs_default_debugger *Gcs_debug_manager::m_debugger = nullptr;

#ifndef XCOM_STANDALONE

/*
  Generate a conflict-safe rotation name for the active trace file.
  Form: <base>.YYYYMMDDTHHMMSS (UTC).
  If the timestamped file already exists:
    <base>.YYYYMMDDTHHMMSS.1, <base>.YYYYMMDDTHHMMSS.2, ...
    up to .1000

  If timestamp generation fails:
    <base>.1, <base>.2, ...
    up to .1000

  @note Entire usage of gcs_generate_rotated_name is inside XCOM_STANDALONE.

  @returns
    0: on success and sets out_path;
    1: if all 1000 names are taken.
*/
static int gcs_generate_rotated_name(const std::string &base_name,
                                     std::string &out_path) {
  char ts_buf[32];
  std::string rotation_base = base_name;
  MY_STAT st_buf;

  const unsigned long long us = my_micro_time();
  const time_t sec = static_cast<time_t>(us / 1000000);

  struct tm tm_info;
  if (gmtime_r(&sec, &tm_info) != nullptr) {
    const int len =
        snprintf(ts_buf, sizeof(ts_buf), "%04d%02d%02dT%02d%02d%02d",
                 tm_info.tm_year + 1900, tm_info.tm_mon + 1, tm_info.tm_mday,
                 tm_info.tm_hour, tm_info.tm_min, tm_info.tm_sec);

    if (len >= 0 && static_cast<size_t>(len) < sizeof(ts_buf)) {
      rotation_base += ".";
      rotation_base += ts_buf;
    }
  }
  if (my_stat(rotation_base.c_str(), &st_buf, MYF(0)) == nullptr) {
    out_path = rotation_base;
    return 0;
  }

  for (int n = 1; n <= 1000; ++n) {
    std::string candidate = rotation_base + "." + std::to_string(n);

    if (my_stat(candidate.c_str(), &st_buf, MYF(0)) == nullptr) {
      out_path = candidate;
      return 0;
    }
  }

  MYSQL_GCS_LOG_ERROR("GCS debug trace: all rotation names for '"
                      << rotation_base.c_str()
                      << "' are taken. Cannot rotate.");
  return 1;
}

Gcs_file_sink::Gcs_file_sink(const std::string &file_name,
                             const std::string &dir_name)
    : m_fd(0),
      m_file_name(file_name),
      m_dir_name(dir_name),
      m_initialized(false) {}

enum_gcs_error Gcs_file_sink::get_file_name(char *file_name_buffer) const {
  unsigned int const flags = MY_REPLACE_DIR | MY_REPLACE_EXT | MY_SAFE_PATH;

  /*
    Absolute paths or references to the home directory are not allowed.
    If the file name contains either or the other, it is ignored and the
    file is created in the path defined in m_dir_name.
  */
  assert(file_name_buffer != nullptr);

  /*
    Format the file name that will be used to store debug information.
  */
  if (fn_format(file_name_buffer, m_file_name.c_str(), m_dir_name.c_str(), "",
                flags) == nullptr)
    return GCS_NOK;

  return GCS_OK;
}

enum_gcs_error Gcs_file_sink::initialize() {
  MY_STAT f_stat;
  char file_name_buffer[FN_REFLEN];

  if (m_initialized) return GCS_OK;

  if (get_file_name(file_name_buffer)) {
    MYSQL_GCS_LOG_ERROR("Error validating file name '" << m_file_name << "'.");
    return GCS_NOK;
  }

  /*
    Check if the directory where the file will be created exists
    and has the appropriate permissions.
  */
  if (my_access(m_dir_name.c_str(), (F_OK | W_OK))) {
    MYSQL_GCS_LOG_ERROR("Error in associated permissions to path '"
                        << m_dir_name.c_str() << "'.");
    return GCS_NOK;
  }

  /*
    Check if the file exists and if so whether the owner has write
    permissions.
  */
  if (my_stat(file_name_buffer, &f_stat, MYF(0)) != nullptr) {
    if (!(f_stat.st_mode & MY_S_IWRITE)) {
      MYSQL_GCS_LOG_ERROR("Error in associated permissions to file '"
                          << file_name_buffer << "'.");
      return GCS_NOK;
    }
  }
  /*
    If size-based rotation is enabled, open a new timestamped file so each
    GR run is cleanly separated.
  */
  if (m_max_file_size > 0) {
    std::string new_name;
    if (gcs_generate_rotated_name(file_name_buffer, new_name) == 0) {
      strncpy(file_name_buffer, new_name.c_str(), FN_REFLEN - 1);
      file_name_buffer[FN_REFLEN - 1] = '\0';
    }
    /* If name generation fails fall through and open canonical (appends). */
  }

  if ((m_fd = my_create(file_name_buffer, 0640, O_CREAT | O_WRONLY | O_APPEND,
                        MYF(0))) < 0) {
    int errno_gcs = 0;
#if defined(_WIN32)
    errno_gcs = WSAGetLastError();
#else
    errno_gcs = errno;
#endif
    MYSQL_GCS_LOG_ERROR("Error openning file '" << file_name_buffer << "':"
                                                << strerror(errno_gcs) << ".");
    return GCS_NOK;
  }

  m_current_path = file_name_buffer;
  m_current_file_size = 0;
  m_initialized = true;

  return GCS_OK;
}

enum_gcs_error Gcs_file_sink::finalize() {
  if (!m_initialized) return GCS_OK;

  my_sync(m_fd, MYF(0));
  my_close(m_fd, MYF(0));

  m_initialized = false;

  return GCS_OK;
}

bool Gcs_file_sink::should_log_error() {
  const ulonglong now = my_micro_time();
  if (now - m_last_logged_error_time < ERROR_LOG_INTERVAL_US) {
    return false;
  }
  m_last_logged_error_time = now;
  return true;
}

bool Gcs_file_sink::should_check_file() {
  const ulonglong now = my_micro_time();
  if (now - m_last_stat_check_time < STAT_CHECK_INTERVAL_US) {
    return false;
  }
  m_last_stat_check_time = now;
  return true;
}

/*
  Since reopen() is called only when file is renamed/deleted, there is no need
  to suppress logging to avoid flooding since stats itself is controlled by
  should_check_file().
*/
enum_gcs_error Gcs_file_sink::reopen() {
  if (!m_initialized) return GCS_OK;
  char file_name_buffer[FN_REFLEN];

  if (get_file_name(file_name_buffer)) {
    MYSQL_GCS_LOG_ERROR("GCS debug trace reopen: error validating file name '"
                        << m_file_name << "'.");
    return GCS_NOK;
  }

  std::string new_name = file_name_buffer;
  if (m_max_file_size > 0) {
    if (gcs_generate_rotated_name(file_name_buffer, new_name) != 0) {
      MYSQL_GCS_LOG_ERROR(
          "GCS debug trace reopen: could not generate new file "
          "name for '"
          << file_name_buffer << "'.");
      return GCS_NOK;
    }
  }

  File new_fd =
      my_create(new_name.c_str(), 0640, O_CREAT | O_WRONLY | O_APPEND, MYF(0));
  /*
    We have not closed existing file descriptors, so even if my_create fails
    call to existing fd are still safe. write continues to write to detached
    file descriptor without any error.
    We keep the old fd active in case of error.
   */
  if (new_fd < 0) {
    int errno_gcs = 0;
#if defined(_WIN32)
    errno_gcs = WSAGetLastError();
#else
    errno_gcs = errno;
#endif
    MYSQL_GCS_LOG_ERROR("GCS debug trace reopen: error opening file '"
                        << new_name.c_str() << "': " << strerror(errno_gcs)
                        << ".");
    return GCS_NOK;
  }

  /* ignore any error, we have new FD */
  my_sync(m_fd, MYF(0));
  my_close(m_fd, MYF(0));
  m_fd = new_fd;
  m_current_path = new_name;
  m_initialized = true;
  m_current_file_size = 0;

  return GCS_OK;
}

enum_gcs_error Gcs_file_sink::rotate() {
  if (!m_initialized) return GCS_OK;
  char file_name_buffer[FN_REFLEN];

  if (get_file_name(file_name_buffer)) {
    if (should_log_error()) {
      MYSQL_GCS_LOG_ERROR("GCS debug trace rotate: error validating file name '"
                          << m_file_name << "'.");
    }
    return GCS_NOK;
  }

  std::string new_name = file_name_buffer;
  if (gcs_generate_rotated_name(file_name_buffer, new_name) != 0) {
    if (should_log_error()) {
      MYSQL_GCS_LOG_ERROR(
          "GCS debug trace rotate: could not generate new file "
          "name for '"
          << file_name_buffer << "'.");
    }
    return GCS_NOK;
  }

  File new_fd =
      my_create(new_name.c_str(), 0640, O_CREAT | O_WRONLY | O_APPEND, MYF(0));
  if (new_fd < 0) {
    int errno_gcs = 0;
#if defined(_WIN32)
    errno_gcs = WSAGetLastError();
#else
    errno_gcs = errno;
#endif
    if (should_log_error()) {
      MYSQL_GCS_LOG_ERROR("GCS debug trace rotate: error opening new file '"
                          << new_name << "': " << strerror(errno_gcs) << ".");
    }
    return GCS_NOK;
  }

  my_sync(m_fd, MYF(0));
  my_close(m_fd, MYF(0));
  m_fd = new_fd;
  m_current_path = new_name;
  m_initialized = true;
  m_current_file_size = 0;

  return GCS_OK;
}

void Gcs_file_sink::log_event(const std::string &message) {
  log_event(message.c_str(), message.length());
}

void Gcs_file_sink::log_event(const char *message, size_t message_size) {
  size_t written;

  written = my_write(m_fd, (const uchar *)message, message_size, MYF(0));

  if (written != MY_FILE_ERROR) {
    m_current_file_size += written;
    if (m_max_file_size > 0 && m_current_file_size >= m_max_file_size) {
      rotate();
    } else if (should_check_file()) {
      /*
        Check if the current trace file was moved or deleted.
        It was noted when the trace file was moved/deleted, no debug trace
        was later generated in the session.

        Note: the check is done after the write (and only once per
        should_check_file() interval), so if the file is removed between our
        write and this check, the line just written goes to the orphaned inode
        and some logs are lost until reopen() creates a new file.
      */
      MY_STAT path_st, fd_st;
      bool path_gone =
          (my_stat(m_current_path.c_str(), &path_st, MYF(0)) == nullptr);
      bool ino_changed = !path_gone && my_fstat(m_fd, &fd_st) == 0 &&
                         path_st.st_ino != fd_st.st_ino;
      if (path_gone || ino_changed) reopen();
    }
  }

  if (written == MY_FILE_ERROR) {
    int errno_gcs = 0;
#if defined(_WIN32)
    errno_gcs = WSAGetLastError();
#else
    errno_gcs = errno;
#endif
    MYSQL_GCS_LOG_ERROR("Error writting to debug file: " << strerror(errno_gcs)
                                                         << ".");
  }
}

const std::string Gcs_file_sink::get_information() const {
  std::string invalid("invalid");

  if (!m_initialized) return invalid;
  /*
    Although m_current_path can now be mutated from a different thread by
    rotate()/reopen() during logging, it is safe to read it here without a
    mutex: get_information() is only called from the initialization path (see
    Gcs_xcom_interface::initialize_logging), before any concurrent logging
    thread can rotate/reopen the file.
  */
  return std::string(m_current_path);
}

void Gcs_clock_timestamp_provider::get_timestamp_as_c_string(char *buffer,
                                                             size_t *size) {
  std::string ts{};
  get_timestamp_as_string(ts);
  size_t n = std::min(ts.size(), *size);
  strncpy(buffer, ts.c_str(), n);
  *size = n;
}

void Gcs_clock_timestamp_provider::get_timestamp_as_string(std::string &out) {
  std::stringstream ss;
  auto now{std::chrono::system_clock::now()};

  // get the microseconds part (floor to the second and then diff it)
  auto us{duration_cast<std::chrono::microseconds>(now.time_since_epoch())};
  auto us_floored{duration_cast<std::chrono::microseconds>(
      std::chrono::floor<std::chrono::seconds>(us))};
  auto delta_us{us - us_floored};

  // get 'now' in a format that we can pass to put_time
  auto utc_time_t{std::chrono::system_clock::to_time_t(now)};
  auto utc_time{std::gmtime(&utc_time_t)};

  // print the line with 6 digits after the '.' and pad with zero if need be
  ss << std::put_time(utc_time, "%FT%T.") << std::setfill('0') << std::setw(6)
     << delta_us.count() << "Z";
  out = ss.str();
}

#endif /* XCOM_STANDALONE */
