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

#include <stdexcept>

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
}  // namespace

FileWriterBuffering::FileWriterBuffering(
    std::unique_ptr<FileWriterBase> file_writer, size_t size, bool drop_if_full)
    : FileWriterDecoratorBase(std::move(file_writer)),
      m_drop_if_full{drop_if_full},
      m_buf(size, Malloc_allocator<char>(key_memory_audit_log_filter_buffer)),
      m_write_pos{0},
      m_flush_pos{0},
      m_flush_worker_thread{0},
      m_flush_worker_running{true},
      m_state{FileBufferState::COMPLETE} {
#ifdef HAVE_PSI_INTERFACE
  mysql_mutex_register(AUDIT_LOG_FILTER_PSI_CATEGORY, mutex_key_list,
                       array_elements(mutex_key_list));
  mysql_cond_register(AUDIT_LOG_FILTER_PSI_CATEGORY, cond_key_list,
                      array_elements(cond_key_list));
#endif /* HAVE_PSI_INTERFACE */

  mysql_mutex_init(key_log_mutex, &m_mutex, MY_MUTEX_INIT_FAST);
  mysql_cond_init(key_log_flushed_cond, &m_flushed_cond);
  mysql_cond_init(key_log_written_cond, &m_written_cond);
  if (pthread_create(
          &m_flush_worker_thread, nullptr,
          [](void *arg) -> void * {
            static_cast<FileWriterBuffering *>(arg)->flush_worker();
            return nullptr;
          },
          this) != 0) {
    mysql_cond_destroy(&m_written_cond);
    mysql_cond_destroy(&m_flushed_cond);
    mysql_mutex_destroy(&m_mutex);
    throw std::runtime_error("failed to create file writer thread");
  }
}

FileWriterBuffering::~FileWriterBuffering() { shutdown(); }

bool FileWriterBuffering::open() noexcept {
  return FileWriterDecoratorBase::open();
}

void FileWriterBuffering::close() noexcept {
  mysql_mutex_lock(&m_mutex);
  while (m_flush_pos != m_write_pos) {
    mysql_cond_signal(&m_written_cond);
    mysql_cond_wait(&m_flushed_cond, &m_mutex);
  }
  mysql_mutex_unlock(&m_mutex);

  FileWriterDecoratorBase::close();
}

void FileWriterBuffering::shutdown() noexcept {
  mysql_mutex_lock(&m_mutex);
  if (!m_flush_worker_running) {
    mysql_mutex_unlock(&m_mutex);
    return;
  }
  m_flush_worker_running = false;
  mysql_cond_signal(&m_written_cond);
  mysql_mutex_unlock(&m_mutex);
  pthread_join(m_flush_worker_thread, nullptr);
  mysql_cond_destroy(&m_flushed_cond);
  mysql_cond_destroy(&m_written_cond);
  mysql_mutex_destroy(&m_mutex);
  m_flush_worker_thread = 0;
}

void FileWriterBuffering::pause() noexcept {
  mysql_mutex_lock(&m_mutex);
  while (m_flush_pos != m_write_pos) {
    mysql_cond_signal(&m_written_cond);
    mysql_cond_wait(&m_flushed_cond, &m_mutex);
  }
}

void FileWriterBuffering::resume() noexcept { mysql_mutex_unlock(&m_mutex); }

void FileWriterBuffering::flush_worker() noexcept {
  my_thread_init();

  while (true) {
    mysql_mutex_lock(&m_mutex);

    // Wait for new data or until worker thread is stopped.
    while (m_flush_worker_running && m_flush_pos == m_write_pos) {
      timespec abs_time{};
      set_timespec(&abs_time, 1);
      mysql_cond_timedwait(&m_written_cond, &m_mutex, &abs_time);
    }

    // End worker thread if it has been stopped and there is no more data to be
    // flushed.
    if (!m_flush_worker_running && m_flush_pos == m_write_pos) {
      mysql_mutex_unlock(&m_mutex);
      break;
    }

    if (m_flush_pos >= m_write_pos % m_buf.size()) {
      m_state = (m_write_pos % m_buf.size() == 0) ? FileBufferState::COMPLETE
                                                  : FileBufferState::INCOMPLETE;
      mysql_mutex_unlock(&m_mutex);
      FileWriterDecoratorBase::write(&m_buf[m_flush_pos],
                                     m_buf.size() - m_flush_pos);
      mysql_mutex_lock(&m_mutex);
      m_flush_pos = 0;
      m_write_pos %= m_buf.size();
    } else {
      const size_t flushlen = m_write_pos - m_flush_pos;
      mysql_mutex_unlock(&m_mutex);
      FileWriterDecoratorBase::write(&m_buf[m_flush_pos], flushlen);
      mysql_mutex_lock(&m_mutex);
      m_flush_pos += flushlen;
      m_state = FileBufferState::COMPLETE;
    }

    assert(m_write_pos >= m_flush_pos);

    mysql_cond_broadcast(&m_flushed_cond);
    mysql_mutex_unlock(&m_mutex);
  }

  my_thread_end();
}

void FileWriterBuffering::write(const char *record, size_t size) noexcept {
  DBUG_EXECUTE_IF("audit_log_write_full_buffer", {
    if (size > m_buf.size()) {
      size = m_buf.size() - m_write_pos;
    } else {
      return;
    }
  });

  if (size > m_buf.size()) {
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
  if (m_write_pos + size <= m_flush_pos + m_buf.size()) {
    const size_t wrlen =
        std::min(size, m_buf.size() - (m_write_pos % m_buf.size()));
    memcpy(&m_buf[m_write_pos % m_buf.size()], record, wrlen);
    if (wrlen < size) {
      memcpy(&m_buf[0], record + wrlen, size - wrlen);
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

  if (m_write_pos > m_flush_pos + m_buf.size() / 2) {
    mysql_cond_signal(&m_written_cond);
  }

  mysql_mutex_unlock(&m_mutex);
}

}  // namespace audit_log_filter::log_writer
