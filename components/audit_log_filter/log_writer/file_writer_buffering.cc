/* Copyright (c) 2023 Percona LLC and/or its affiliates. All rights reserved.

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

#include "components/audit_log_filter/log_writer/file_writer_buffering.h"

#include "components/audit_log_filter/audit_psi_info.h"
#include "components/audit_log_filter/sys_vars.h"

#include "my_dbug.h"
#include "my_sys.h"
#include "my_systime.h"
#include "template_utils.h"

namespace audit_log_filter::log_writer {
namespace {
#if defined(HAVE_PSI_INTERFACE)
/* These belong to the service initialization */
PSI_mutex_key key_log_mutex;
PSI_mutex_info mutex_key_list[] = {{&key_log_mutex,
                                    "audit_filter_buffer::mutex",
                                    PSI_FLAG_SINGLETON, 0, PSI_DOCUMENT_ME}};

PSI_cond_key key_log_written_cond, key_log_flushed_cond;
PSI_cond_info cond_key_list[] = {
    {&key_log_written_cond, "audit_filter_buffer::written_cond",
     PSI_FLAG_SINGLETON, 0, PSI_DOCUMENT_ME},
    {&key_log_flushed_cond, "audit_filter_buffer::flushed_cond",
     PSI_FLAG_SINGLETON, 0, PSI_DOCUMENT_ME}};
#endif

void *buffer_flush_worker(void *arg) {
  auto *buffer = static_cast<FileWriterBuffering *>(arg);

  my_thread_init();
  while (!buffer->check_flush_stopped()) {
    buffer->flush_worker();
  }
  my_thread_end();

  return nullptr;
}
}  // namespace

FileWriterBuffering::FileWriterBuffering(
    std::unique_ptr<FileWriterBase> file_writer, size_t size, bool drop_if_full)
    : FileWriterDecoratorBase(std::move(file_writer)),
      m_size{size},
      m_drop_if_full{drop_if_full},
      m_buf{nullptr},
      m_write_pos{0},
      m_flush_pos{0},
      m_flush_worker_thread{0},
      m_stop_flush_worker{false} {}

FileWriterBuffering::~FileWriterBuffering() {
  if (m_buf != nullptr) {
    shutdown();
  }
}

bool FileWriterBuffering::init() noexcept {
  m_buf = static_cast<char *>(
      my_malloc(key_memory_audit_log_filter_buffer, m_size, MY_ZEROFILL));

  if (m_buf == nullptr) {
    return false;
  }

#ifdef HAVE_PSI_INTERFACE
  mysql_mutex_register(AUDIT_LOG_FILTER_PSI_CATEGORY, mutex_key_list,
                       array_elements(mutex_key_list));
  mysql_cond_register(AUDIT_LOG_FILTER_PSI_CATEGORY, cond_key_list,
                      array_elements(cond_key_list));
#endif /* HAVE_PSI_INTERFACE */

  m_state = FileBufferState::COMPLETE;
  m_write_pos = 0;
  m_flush_pos = 0;
  m_stop_flush_worker = false;

  mysql_mutex_init(key_log_mutex, &m_mutex, MY_MUTEX_INIT_FAST);
  mysql_cond_init(key_log_flushed_cond, &m_flushed_cond);
  mysql_cond_init(key_log_written_cond, &m_written_cond);
  pthread_create(&m_flush_worker_thread, nullptr, buffer_flush_worker, this);

  return FileWriterDecoratorBase::init();
}

bool FileWriterBuffering::open() noexcept {
  return FileWriterDecoratorBase::open();
}

void FileWriterBuffering::close() noexcept {
  mysql_mutex_lock(&m_mutex);
  while (m_flush_pos != m_write_pos) {
    mysql_cond_wait(&m_flushed_cond, &m_mutex);
  }
  mysql_mutex_unlock(&m_mutex);

  FileWriterDecoratorBase::close();
}

void FileWriterBuffering::shutdown() noexcept {
  m_stop_flush_worker = true;

  if (m_buf != nullptr) {
    pthread_join(m_flush_worker_thread, nullptr);
    mysql_cond_destroy(&m_flushed_cond);
    mysql_cond_destroy(&m_written_cond);
    mysql_mutex_destroy(&m_mutex);
    my_free(m_buf);
    m_buf = nullptr;
    m_flush_worker_thread = 0;
  }
}

void FileWriterBuffering::pause() noexcept {
  mysql_mutex_lock(&m_mutex);
  while (m_state == FileBufferState::INCOMPLETE) {
    mysql_cond_wait(&m_flushed_cond, &m_mutex);
  }
}

void FileWriterBuffering::resume() noexcept { mysql_mutex_unlock(&m_mutex); }

bool FileWriterBuffering::check_flush_stopped() const noexcept {
  return m_stop_flush_worker && m_flush_pos == m_write_pos;
}

void FileWriterBuffering::flush_worker() noexcept {
  mysql_mutex_lock(&m_mutex);

  while (m_flush_pos == m_write_pos) {
    if (m_stop_flush_worker) {
      mysql_mutex_unlock(&m_mutex);
      return;
    }
    timespec abs_time{};
    set_timespec(&abs_time, 1);
    mysql_cond_timedwait(&m_written_cond, &m_mutex, &abs_time);
  }

  if (m_flush_pos >= m_write_pos % m_size) {
    m_state = (m_write_pos % m_size == 0) ? FileBufferState::COMPLETE
                                          : FileBufferState::INCOMPLETE;
    mysql_mutex_unlock(&m_mutex);
    FileWriterDecoratorBase::write(m_buf + m_flush_pos, m_size - m_flush_pos);
    mysql_mutex_lock(&m_mutex);
    m_flush_pos = 0;
    m_write_pos %= m_size;
  } else {
    const size_t flushlen = m_write_pos - m_flush_pos;
    mysql_mutex_unlock(&m_mutex);
    FileWriterDecoratorBase::write(m_buf + m_flush_pos, flushlen);
    mysql_mutex_lock(&m_mutex);
    m_flush_pos += flushlen;
    m_state = FileBufferState::COMPLETE;
  }

  assert(m_write_pos >= m_flush_pos);

  mysql_cond_broadcast(&m_flushed_cond);
  mysql_mutex_unlock(&m_mutex);
}

void FileWriterBuffering::write(const char *record, size_t size) noexcept {
  DBUG_EXECUTE_IF("audit_log_write_full_buffer", {
    if (size > m_size) {
      size = m_size - m_write_pos;
    } else {
      return;
    }
  });

  if (size > m_size) {
    if (!m_drop_if_full) {
      /* pause flushing thread and write out one record bypassing the buffer */
      pause();
      FileWriterDecoratorBase::write(record, size);
      resume();

      SysVars::inc_direct_writes();
    } else {
      SysVars::inc_events_lost();
      SysVars::update_event_max_drop_size(size);
    }

    return;
  }

  mysql_mutex_lock(&m_mutex);

loop:
  if (m_write_pos + size <= m_flush_pos + m_size) {
    const size_t wrlen = std::min(size, m_size - (m_write_pos % m_size));
    memcpy(m_buf + (m_write_pos % m_size), record, wrlen);
    if (wrlen < size) {
      memcpy(m_buf, record + wrlen, size - wrlen);
    }
    m_write_pos = m_write_pos + size;
    assert(m_write_pos >= m_flush_pos);
  } else {
    if (!m_drop_if_full) {
      SysVars::inc_write_waits();
      mysql_cond_wait(&m_flushed_cond, &m_mutex);
      goto loop;
    }

    SysVars::inc_events_lost();
    SysVars::update_event_max_drop_size(size);
  }

  if (m_write_pos > m_flush_pos + m_size / 2) {
    mysql_cond_signal(&m_written_cond);
  }

  mysql_mutex_unlock(&m_mutex);
}

}  // namespace audit_log_filter::log_writer
