/* Copyright (c) 2014 Percona LLC and/or its affiliates. All rights reserved.

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

#include "buffer.h"

#include "audit_log.h"
#include "my_dbug.h"
#include "my_sys.h"
#include "my_systime.h"
#include "my_thread.h"
#include "mysql/psi/mysql_cond.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/psi/psi_cond.h"
#include "mysql/psi/psi_mutex.h"
#include "mysql/service_mysql_alloc.h"
#include "thr_mutex.h"

struct audit_log_buffer_t {
  char *buf;
  size_t size;
  size_t write_pos;
  size_t flush_pos;
  pthread_t flush_worker_thread;
  bool stop;
  bool drop_if_full;
  void *write_func_data;
  audit_log_write_func write_func;
  mysql_mutex_t mutex;
  mysql_cond_t flushed_cond;
  mysql_cond_t written_cond;
  log_record_state_t state;
};

#if defined(HAVE_PSI_INTERFACE)
/* These belong to the service initialization */
static PSI_mutex_key key_log_mutex;
static PSI_mutex_info mutex_key_list[] = {
    {&key_log_mutex, "audit_log_buffer::mutex", PSI_FLAG_SINGLETON, 0,
     PSI_DOCUMENT_ME}};

static PSI_cond_key key_log_written_cond, key_log_flushed_cond;
static PSI_cond_info cond_key_list[] = {
    {&key_log_written_cond, "audit_log_buffer::written_cond",
     PSI_FLAG_SINGLETON, 0, PSI_DOCUMENT_ME},
    {&key_log_flushed_cond, "audit_log_buffer::flushed_cond",
     PSI_FLAG_SINGLETON, 0, PSI_DOCUMENT_ME}};

#endif

static void audit_log_flush(audit_log_buffer_t *log) {
  mysql_mutex_lock(&log->mutex);
  while (log->flush_pos == log->write_pos) {
    if (log->stop) {
      mysql_mutex_unlock(&log->mutex);
      return;
    }
    timespec abstime;
    set_timespec(&abstime, 1);
    mysql_cond_timedwait(&log->written_cond, &log->mutex, &abstime);
  }

  if (log->flush_pos >= log->write_pos % log->size) {
    log->state = log_record_state_t::INCOMPLETE;
    mysql_mutex_unlock(&log->mutex);
    log->write_func(log->write_func_data, log->buf + log->flush_pos,
                    log->size - log->flush_pos, log_record_state_t::INCOMPLETE);
    mysql_mutex_lock(&log->mutex);
    log->flush_pos = 0;
    log->write_pos %= log->size;
  } else {
    const size_t flushlen = log->write_pos - log->flush_pos;
    mysql_mutex_unlock(&log->mutex);
    log->write_func(log->write_func_data, log->buf + log->flush_pos, flushlen,
                    log_record_state_t::COMPLETE);
    mysql_mutex_lock(&log->mutex);
    log->flush_pos += flushlen;
    log->state = log_record_state_t::COMPLETE;
  }
  DBUG_ASSERT(log->write_pos >= log->flush_pos);
  mysql_cond_broadcast(&log->flushed_cond);
  mysql_mutex_unlock(&log->mutex);
}

static void *audit_log_flush_worker(void *arg) {
  audit_log_buffer_t *log = (audit_log_buffer_t *)arg;

  my_thread_init();
  while (!(log->stop && log->flush_pos == log->write_pos)) {
    audit_log_flush(log);
  }
  my_thread_end();

  return nullptr;
}

audit_log_buffer_t *audit_log_buffer_init(size_t size, bool drop_if_full,
                                          audit_log_write_func write_func,
                                          void *data) noexcept {
  audit_log_buffer_t *log = (audit_log_buffer_t *)my_malloc(
      key_memory_audit_log_buffer, sizeof(audit_log_buffer_t) + size,
      MY_ZEROFILL);

#ifdef HAVE_PSI_INTERFACE
  mysql_mutex_register(AUDIT_LOG_PSI_CATEGORY, mutex_key_list,
                       array_elements(mutex_key_list));
  mysql_cond_register(AUDIT_LOG_PSI_CATEGORY, cond_key_list,
                      array_elements(cond_key_list));
#endif /* HAVE_PSI_INTERFACE */

  if (log != nullptr) {
    log->buf = ((char *)log + sizeof(audit_log_buffer_t));
    log->drop_if_full = drop_if_full;
    log->write_func = write_func;
    log->write_func_data = data;
    log->size = size;
    log->state = log_record_state_t::COMPLETE;

    mysql_mutex_init(key_log_mutex, &log->mutex, MY_MUTEX_INIT_FAST);
    mysql_cond_init(key_log_flushed_cond, &log->flushed_cond);
    mysql_cond_init(key_log_written_cond, &log->written_cond);
    pthread_create(&log->flush_worker_thread, nullptr, audit_log_flush_worker,
                   log);
  }

  return log;
}

void audit_log_buffer_shutdown(audit_log_buffer_t *log) noexcept {
  log->stop = true;

  pthread_join(log->flush_worker_thread, nullptr);
  mysql_cond_destroy(&log->flushed_cond);
  mysql_cond_destroy(&log->written_cond);
  mysql_mutex_destroy(&log->mutex);

  my_free(log);
}

void audit_log_buffer_pause(audit_log_buffer_t *log) noexcept {
  mysql_mutex_lock(&log->mutex);
  while (log->state == log_record_state_t::INCOMPLETE) {
    mysql_cond_wait(&log->flushed_cond, &log->mutex);
  }
}

void audit_log_buffer_resume(audit_log_buffer_t *log) noexcept {
  mysql_mutex_unlock(&log->mutex);
}

int audit_log_buffer_write(audit_log_buffer_t *log, const char *buf,
                           size_t len) {
  if (len > log->size) {
    if (!log->drop_if_full) {
      /* pause flushing thread and write out one record bypassing the buffer */
      audit_log_buffer_pause(log);
      log->write_func(log->write_func_data, buf, len,
                      log_record_state_t::COMPLETE);
      audit_log_buffer_resume(log);
    }
    audit_log_buffer_size_overflow.fetch_add(1, std::memory_order_relaxed);
    return (0);
  }

  mysql_mutex_lock(&log->mutex);
loop:
  if (log->write_pos + len <= log->flush_pos + log->size) {
    const size_t wrlen =
        std::min(len, log->size - (log->write_pos % log->size));
    memcpy(log->buf + (log->write_pos % log->size), buf, wrlen);
    if (wrlen < len) memcpy(log->buf, buf + wrlen, len - wrlen);
    log->write_pos = log->write_pos + len;
    DBUG_ASSERT(log->write_pos >= log->flush_pos);
  } else {
    if (!log->drop_if_full) {
      mysql_cond_wait(&log->flushed_cond, &log->mutex);
      goto loop;
    }
  }
  if (log->write_pos > log->flush_pos + log->size / 2) {
    mysql_cond_signal(&log->written_cond);
  }
  mysql_mutex_unlock(&log->mutex);

  return (0);
}
