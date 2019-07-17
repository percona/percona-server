/* Copyright (C) 2012 Monty Program Ab

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
   USA */

#include <string.h>
#include "my_dir.h"
#include "my_sys.h"
#include "mysql/plugin.h"
#include "sql/mysqld.h"

#include "audit_log.h"
#include "logger.h"

#ifndef FLOGGER_NO_PSI
#define flogger_mutex_init(A, B, C) \
  if ((B)->thread_safe) mysql_mutex_init(A, &((B)->lock), C)

#define flogger_mutex_destroy(A) \
  if ((A)->thread_safe) mysql_mutex_destroy(&((A)->lock))

#define flogger_mutex_lock(A) \
  if ((A)->thread_safe) mysql_mutex_lock(&((A)->lock))

#define flogger_mutex_unlock(A) \
  if ((A)->thread_safe) mysql_mutex_unlock(&((A)->lock))
#else
#define flogger_mutex_init(A, B, C) \
  if ((B)->thread_safe) pthread_mutex_init(&((B)->lock.m_mutex), C)

#define flogger_mutex_destroy(A) \
  if ((A)->thread_safe) pthread_mutex_destroy(&((A)->lock.m_mutex))

#define flogger_mutex_lock(A) \
  if ((A)->thread_safe) pthread_mutex_lock(&((A)->lock.m_mutex))

#define flogger_mutex_unlock(A) \
  if ((A)->thread_safe) pthread_mutex_unlock(&((A)->lock.m_mutex))
#endif /*!FLOGGER_NO_PSI*/

#if defined(HAVE_PSI_INTERFACE) && !defined(FLOGGER_NO_PSI)
/* These belong to the service initialization */
static PSI_mutex_key key_LOCK_logger_service;
static PSI_mutex_info mutex_list[] = {
    {&key_LOCK_logger_service, "file_logger::lock", PSI_FLAG_SINGLETON,
     PSI_VOLATILITY_UNKNOWN, PSI_DOCUMENT_ME}};
#else
#define key_LOCK_logger_service nullptr
#endif /*HAVE_PSI_INTERFACE && !FLOGGER_NO_PSI*/

struct LOGGER_HANDLE {
  File file;
  char path[FN_REFLEN];
  unsigned long long size_limit;
  unsigned int rotations;
  size_t path_len;
  mysql_mutex_t lock;
  bool thread_safe;
};

#define LOG_FLAGS (O_APPEND | O_CREAT | O_WRONLY)

static constexpr unsigned int n_dig(unsigned int i) noexcept {
  return (i == 0) ? 0 : ((i < 10) ? 1 : ((i < 100) ? 2 : 3));
}

LOGGER_HANDLE *logger_open(const char *path, unsigned long long size_limit,
                           unsigned int rotations, bool thread_safe,
                           logger_prolog_func_t header) noexcept {
  /*
    I don't think we ever need more rotations,
    but if it's so, the rotation procedure should be adapted to it.
  */
  if (rotations > 999) return 0;

  LOGGER_HANDLE new_log;
  new_log.rotations = rotations;
  new_log.size_limit = size_limit;
  new_log.path_len = strlen(
      fn_format(new_log.path, path, mysql_data_home, "", MY_UNPACK_FILENAME));
  new_log.thread_safe = thread_safe;

  if (new_log.path_len + n_dig(rotations) + 1 > FN_REFLEN) {
    errno = ENAMETOOLONG;
    /* File path too long */
    return 0;
  }

  if ((new_log.file = my_open(new_log.path, LOG_FLAGS, 0666)) < 0) {
    errno = my_errno();
    /* Check errno for the cause */
    return 0;
  }

  MY_STAT stat_arg;
  if (my_fstat(new_log.file, &stat_arg)) {
    errno = my_errno();
    my_close(new_log.file, MYF(0));
    new_log.file = -1;
    return 0;
  }

  LOGGER_HANDLE *l_perm;
  if (!(l_perm = (LOGGER_HANDLE *)my_malloc(key_memory_audit_log_logger_handle,
                                            sizeof(LOGGER_HANDLE), MYF(0)))) {
    my_close(new_log.file, MYF(0));
    new_log.file = -1;
    return 0; /* End of memory */
  }
  *l_perm = new_log;

  flogger_mutex_init(key_LOCK_logger_service, l_perm, MY_MUTEX_INIT_FAST);

  char buf[128];
  size_t len;
  len = header(&stat_arg, buf, sizeof(buf));
  my_write(l_perm->file, (uchar *)buf, len, MYF(0));

  return l_perm;
}

int logger_close(LOGGER_HANDLE *log, logger_epilog_func_t footer) noexcept {
  File file = log->file;
  char buf[128];
  const size_t len = footer(buf, sizeof(buf));
  my_write(file, (uchar *)buf, len, MYF(0));

  flogger_mutex_destroy(log);
  my_free(log);
  int result;
  if ((result = my_close(file, MYF(0)))) errno = my_errno();
  return result;
}

int logger_reopen(LOGGER_HANDLE *log, logger_prolog_func_t header,
                  logger_epilog_func_t footer) noexcept {
  flogger_mutex_lock(log);

  char buf[128];
  size_t len = footer(buf, sizeof(buf));
  my_write(log->file, (uchar *)buf, len, MYF(0));

  int result = 0;
  if ((result = my_close(log->file, MYF(0)))) {
    errno = my_errno();
    goto error;
  }

  if ((log->file = my_open(log->path, LOG_FLAGS, MYF(0))) < 0) {
    errno = my_errno();
    result = 1;
    goto error;
  }

  MY_STAT stat_arg;
  if ((result = my_fstat(log->file, &stat_arg))) {
    errno = my_errno();
    goto error;
  }

  len = header(&stat_arg, buf, sizeof(buf));
  my_write(log->file, (uchar *)buf, len, MYF(0));

error:
  flogger_mutex_unlock(log);

  return result;
}

static char *logname(const LOGGER_HANDLE &log, char *buf, size_t buf_len,
                     unsigned int n_log) noexcept {
  snprintf(buf + log.path_len, buf_len, ".%0*u", n_dig(log.rotations), n_log);
  return buf;
}

static int do_rotate(LOGGER_HANDLE *log) {
  if (log->rotations == 0) return 0;

  char namebuf[FN_REFLEN];
  memcpy(namebuf, log->path, log->path_len);

  int result;
  char *buf_new = logname(*log, namebuf, sizeof(namebuf), log->rotations);
  char *buf_old = log->path;
  for (auto i = log->rotations - 1; i > 0; i--) {
    logname(*log, buf_old, FN_REFLEN, i);
    if (!access(buf_old, F_OK) &&
        (result = my_rename(buf_old, buf_new, MYF(0))))
      goto exit;
    char *tmp = buf_old;
    buf_old = buf_new;
    buf_new = tmp;
  }
  if ((result = my_close(log->file, MYF(0)))) goto exit;
  namebuf[log->path_len] = 0;
  result = my_rename(namebuf, logname(*log, log->path, FN_REFLEN, 1), MYF(0));
  log->file = my_open(namebuf, LOG_FLAGS, MYF(0));
exit:
  errno = my_errno();
  return log->file < 0 || result;
}

int logger_write(LOGGER_HANDLE *log, const char *buffer, size_t size,
                 log_record_state_t state) noexcept {
  flogger_mutex_lock(log);

  int result = my_write(log->file, (uchar *)buffer, size, MYF(0));

  if (state == log_record_state_t::COMPLETE && log->rotations > 0) {
    my_off_t filesize;
    if ((filesize = my_tell(log->file, MYF(0))) == (my_off_t)-1 ||
        ((unsigned long long)filesize >= log->size_limit && do_rotate(log))) {
      result = -1;
      errno = my_errno();
    }
  }

  flogger_mutex_unlock(log);
  return result;
}

void logger_init_mutexes() noexcept {
#if defined(HAVE_PSI_INTERFACE) && !defined(FLOGGER_NO_PSI) && \
    !defined(FLOGGER_NO_THREADSAFE)
  mysql_mutex_register(AUDIT_LOG_PSI_CATEGORY, mutex_list,
                       array_elements(mutex_list));
#endif /*HAVE_PSI_INTERFACE && !FLOGGER_NO_PSI*/
}

int logger_sync(LOGGER_HANDLE *log) noexcept {
  return my_sync(log->file, MYF(0));
}

void logger_set_size_limit(LOGGER_HANDLE *log,
                           unsigned long long size_limit) noexcept {
  log->size_limit = size_limit;
}

void logger_set_rotations(LOGGER_HANDLE *log, unsigned int rotations) noexcept {
  log->rotations = rotations;
}
