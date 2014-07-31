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


#ifndef AUDIT_HANDLER_INCLUDED
#define AUDIT_HANDLER_INCLUDED

#include <my_global.h>

#include "logger.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct audit_handler_struct audit_handler_t;
typedef struct audit_handler_file_config_struct audit_handler_file_config_t;
typedef struct audit_handler_syslog_config_struct audit_handler_syslog_config_t;
typedef struct audit_handler_buffered_struct audit_handler_buffered_t;
typedef void * audit_handler_data_t;


typedef enum { OPT_ROTATE_ON_SIZE, OPT_ROTATIONS } audit_handler_option_t;

struct audit_handler_struct
{
  int (*write)(audit_handler_t *, const char *, size_t);
  int (*flush)(audit_handler_t *);
  int (*close)(audit_handler_t *);
  void (*set_option)(audit_handler_t *, audit_handler_option_t, void *);
  audit_handler_data_t data;
};

struct audit_handler_file_config_struct
{
  const char *name;
  size_t rotate_on_size;
  size_t rotations;
  my_bool sync_on_write;
  my_bool use_buffer;
  size_t buffer_size;
  my_bool can_drop_data;
  logger_prolog_func_t header;
  logger_epilog_func_t footer;
};

struct audit_handler_syslog_config_struct
{
  const char *ident;
  int facility;
  int priority;
  logger_prolog_func_t header;
  logger_epilog_func_t footer;
};

static inline
int audit_handler_write(audit_handler_t *handler, const char *buf, size_t len)
{
  if (handler->write != NULL)
  {
    return handler->write(handler, buf, len);
  }
  return len;
}

static inline
int audit_handler_flush(audit_handler_t *handler)
{
  if (handler->flush != NULL)
  {
    return handler->flush(handler);
  }
  return 0;
}

static inline
int audit_handler_close(audit_handler_t *handler)
{
  if (handler->close != NULL)
  {
    return handler->close(handler);
  }
  return 0;
}

static inline
void audit_handler_set_option(audit_handler_t *handler,
                              audit_handler_option_t opt, void *val)
{
  if (handler->set_option != NULL)
  {
    handler->set_option(handler, opt, val);
  }
}

audit_handler_t *audit_handler_file_open(audit_handler_file_config_t *opts);
audit_handler_t *audit_handler_syslog_open(audit_handler_syslog_config_t *opts);

#ifdef __cplusplus
}
#endif

#endif
