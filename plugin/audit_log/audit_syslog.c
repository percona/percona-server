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

#include <syslog.h>
#include <string.h>
#include "audit_handler.h"

typedef struct audit_handler_syslog_data_struct audit_handler_syslog_data_t;

struct audit_handler_syslog_data_struct
{
  size_t struct_size;
  int priority;
  logger_prolog_func_t header;
  logger_epilog_func_t footer;
};

static int audit_handler_syslog_write(audit_handler_t *handler,
                                      const char *buf, size_t len);
static int audit_handler_syslog_flush(audit_handler_t *handler);
static int audit_handler_syslog_close(audit_handler_t *handler);


audit_handler_t *audit_handler_syslog_open(audit_handler_syslog_config_t *opts)
{
  audit_handler_t *handler= (audit_handler_t*)
       calloc(sizeof(audit_handler_t) + sizeof(audit_handler_syslog_data_t), 1);
  if (handler != NULL)
  {
    audit_handler_syslog_data_t *data=
                                   (audit_handler_syslog_data_t*) (handler + 1);
    MY_STAT stat_arg;

    data->struct_size= sizeof(audit_handler_syslog_data_t);
    data->priority= opts->priority;
    data->header= opts->header;
    data->footer= opts->footer;
    openlog(opts->ident, 0, opts->facility);
    memset(&stat_arg, 0, sizeof(stat_arg));
    opts->header(&stat_arg, NULL, 0);
    handler->data= data;
    handler->write= audit_handler_syslog_write;
    handler->flush= audit_handler_syslog_flush;
    handler->close= audit_handler_syslog_close;
  }
  return handler;
}

static
int audit_handler_syslog_write(audit_handler_t *handler,
                               const char *buf, size_t len)
{
  audit_handler_syslog_data_t *data=
                                   (audit_handler_syslog_data_t*) handler->data;
  DBUG_ASSERT(data->struct_size == sizeof(audit_handler_syslog_data_t));
  syslog(data->priority, "%s", buf);
  return len;
}

static
int audit_handler_syslog_flush(audit_handler_t *handler)
{
  audit_handler_syslog_data_t *data=
                                   (audit_handler_syslog_data_t*) handler->data;
  MY_STAT stat_arg;
  memset(&stat_arg, 0, sizeof(stat_arg));
  data->header(&stat_arg, NULL, 0);
  data->footer(NULL, 0);
  return 0;
}

static
int audit_handler_syslog_close(audit_handler_t *handler)
{
  audit_handler_syslog_data_t *data=
                                   (audit_handler_syslog_data_t*) handler->data;
  data->footer(NULL, 0);
  closelog();
  free(handler);
  return 0;
}
