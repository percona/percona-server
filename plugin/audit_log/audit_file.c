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

#include "audit_handler.h"
#include "buffer.h"

typedef struct audit_handler_file_data_struct audit_handler_file_data_t;

struct audit_handler_file_data_struct
{
  size_t struct_size;
  LOGGER_HANDLE *logger;
  logger_prolog_func_t header;
  logger_epilog_func_t footer;
  my_bool sync_on_write;
  my_bool use_buffer;
  audit_log_buffer_t *buffer;
};

static
int audit_handler_file_write(audit_handler_t *handler,
                             const char *buf, size_t len);
static
int audit_handler_file_flush(audit_handler_t *handler);
static
int audit_handler_file_close(audit_handler_t *handler);
static
int audit_handler_file_write_nobuf(LOGGER_HANDLE *logger,
                                   const char *buf, size_t len,
                                   log_record_state_t state);
static
int audit_handler_file_write_buf(audit_log_buffer_t *buffer,
                                 const char *buf, size_t len);
static
void audit_handler_file_set_option(audit_handler_t *handler,
                                   audit_handler_option_t opt, void *val);

static
int write_callback(void *data, const char *buf, size_t len,
                   log_record_state_t state)
{
  audit_handler_t *handler= (audit_handler_t *) data;
  audit_handler_file_data_t *hdata= (audit_handler_file_data_t*) handler->data;

  DBUG_ASSERT(hdata->struct_size == sizeof(audit_handler_file_data_t));

  return audit_handler_file_write_nobuf(hdata->logger, buf, len, state);
}


audit_handler_t *audit_handler_file_open(audit_handler_file_config_t *opts)
{
  audit_handler_t *handler= (audit_handler_t*)
         calloc(sizeof(audit_handler_t) + sizeof(audit_handler_file_data_t), 1);
  if (handler != NULL)
  {
    audit_handler_file_data_t *data= (audit_handler_file_data_t*) (handler + 1);
    data->struct_size= sizeof(audit_handler_file_data_t);
    data->footer= opts->footer;
    data->header= opts->header;
    data->sync_on_write= opts->sync_on_write;
    data->use_buffer= opts->use_buffer;
    if (data->use_buffer)
    {
      data->buffer= audit_log_buffer_init(opts->buffer_size,
                                       opts->can_drop_data,
                                       write_callback, handler);
      if (data->buffer == NULL)
        goto error;
    }
    data->logger= logger_open(opts->name, opts->rotate_on_size,
                              opts->rotate_on_size ? opts->rotations : 0,
                              !opts->use_buffer, opts->header);
    if (data->logger == NULL)
    {
      goto error;
    }
    handler->data= data;
    handler->write= audit_handler_file_write;
    handler->flush= audit_handler_file_flush;
    handler->close= audit_handler_file_close;
    handler->set_option= audit_handler_file_set_option;
    goto success;
error:
    if (data->use_buffer)
    {
      free(data->buffer);
    }
    free(handler);
    handler= NULL;
  }
success:
  return handler;
}

static
int audit_handler_file_write_nobuf(LOGGER_HANDLE *logger,
                                   const char *buf, size_t len,
                                   log_record_state_t state)
{
  return logger_write(logger, buf, len, state);
}

static
int audit_handler_file_write_buf(audit_log_buffer_t *buffer,
                                 const char *buf, size_t len)
{
  return audit_log_buffer_write(buffer, buf, len);
}

static
int audit_handler_file_write(audit_handler_t *handler,
                             const char *buf, size_t len)
{
  audit_handler_file_data_t *data= (audit_handler_file_data_t*) handler->data;
  int res;

  DBUG_ASSERT(data->struct_size == sizeof(audit_handler_file_data_t));

  if (data->use_buffer)
  {
    DBUG_ASSERT(data->buffer);
    res= audit_handler_file_write_buf(data->buffer, buf, len);
  }
  else
  {
    DBUG_ASSERT(data->logger);
    res= audit_handler_file_write_nobuf(data->logger, buf, len,
                                        LOG_RECORD_COMPLETE);

    if (data->sync_on_write)
    {
      logger_sync(data->logger);
    }
  }

  return res;
}

static
int audit_handler_file_flush(audit_handler_t *handler)
{
  audit_handler_file_data_t *data= (audit_handler_file_data_t*) handler->data;
  LOGGER_HANDLE* logger;

  DBUG_ASSERT(data->struct_size == sizeof(audit_handler_file_data_t));

  logger= data->logger;

  return logger_reopen(logger, data->header, data->footer);
}

static
int audit_handler_file_close(audit_handler_t *handler)
{
  audit_handler_file_data_t *data= (audit_handler_file_data_t*) handler->data;
  int res;
  LOGGER_HANDLE* logger;

  DBUG_ASSERT(data->struct_size == sizeof(audit_handler_file_data_t));

  logger= data->logger;

  if (data->use_buffer)
  {
    audit_log_buffer_shutdown(data->buffer);
  }

  res= logger_close(logger, data->footer);

  free(handler);

  return res;
}

static
void audit_handler_file_set_option(audit_handler_t *handler,
                                   audit_handler_option_t opt, void *val)
{
  audit_handler_file_data_t *data= (audit_handler_file_data_t*) handler->data;

  switch (opt)
  {
  case OPT_ROTATIONS:
    logger_set_size_limit(data->logger, *(ulonglong*)(val));
    break;
  case OPT_ROTATE_ON_SIZE:
    logger_set_rotations(data->logger, *(ulonglong*)(val));
    break;
  }
}
