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


#ifndef AUDIT_LOG_BUFFER_INCLUDED
#define AUDIT_LOG_BUFFER_INCLUDED

#include <string.h> // for size_t
#include "logger.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct audit_log_buffer audit_log_buffer_t;

typedef int (*audit_log_write_func)(void *data, const char *buf, size_t len,
                                    log_record_state_t state);

audit_log_buffer_t *audit_log_buffer_init(size_t size, int drop_if_full,
                                 audit_log_write_func write_func, void *data);
void audit_log_buffer_shutdown(audit_log_buffer_t *log);
int audit_log_buffer_write(audit_log_buffer_t *log,
                           const char *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif
