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

#include <time.h>
#include <string.h>
#include <stdio.h>

#include <my_global.h>
#include <mysql/plugin.h>
#include <mysql/plugin_audit.h>
#include <typelib.h>
#include <mysql_version.h>
#include <mysql_com.h>
#include <my_pthread.h>

#include "logger.h"
#include "buffer.h"

#define PLUGIN_VERSION 0x0001


enum audit_log_policy_t { ALL, NONE, LOGINS, QUERIES };
enum audit_log_strategy_t
  { ASYNCHRONOUS, PERFORMANCE, SEMISYNCHRONOUS, SYNCHRONOUS };
enum audit_log_format_t { OLD, NEW };

static LOGGER_HANDLE *audit_file_logger= NULL;
static audit_log_buffer_t *audit_log_buffer= NULL;
static ulonglong record_id= 0;
static time_t log_file_time= 0;
char *audit_log_file;
char default_audit_log_file[]= "audit.log";
ulong audit_log_policy= ALL;
ulong audit_log_strategy= ASYNCHRONOUS;
ulonglong audit_log_buffer_size= 1048576;
ulonglong audit_log_rotate_on_size= 0;
ulonglong audit_log_rotations= 0;
char audit_log_flush= FALSE;
ulong audit_log_format= OLD;


static
void init_record_id(off_t size)
{
  record_id= size;
}


static
ulonglong next_record_id()
{
  return __sync_add_and_fetch(&record_id, 1);
}


#define MAX_RECORD_ID_SIZE  50
#define MAX_TIMESTAMP_SIZE  25


static
void fprintf_timestamp(FILE *file)
{
  char timebuf[50];
  struct tm tm;
  time_t curtime;

  memset(&tm, 0, sizeof(tm));
  time(&curtime);
  localtime_r(&curtime, &tm);

  strftime(timebuf, sizeof(timebuf), "%FT%T", gmtime_r(&curtime, &tm));

  fprintf(file, "%s audit_log: ", timebuf);
}


static
char *make_timestamp(char *buf, size_t buf_len, time_t t)
{
  struct tm tm;

  memset(&tm, 0, sizeof(tm));
  strftime(buf, buf_len, "%FT%T UTC", gmtime_r(&t, &tm));

  return buf;
}

static
char *make_record_id(char *buf, size_t buf_len)
{
  struct tm tm;
  size_t len;

  memset(&tm, 0, sizeof(tm));
  len= snprintf(buf, buf_len, "%llu_", next_record_id());

  strftime(buf + len, buf_len - len,
           "%FT%T", gmtime_r(&log_file_time, &tm));

  return buf;
}

static
void xml_escape(const char *in, size_t *inlen, char* out, size_t *outlen)
{
  char* outstart = out;
  const char* base = in;
  char* outend = out + *outlen;
  const char* inend;

  inend = in + (*inlen);

  while ((in < inend) && (out < outend))
  {
    if (*in == '<')
    {
      if (outend - out < 4)
        break;
      *out++ = '&';
      *out++ = 'l';
      *out++ = 't';
      *out++ = ';';
    }
    else if (*in == '>')
    {
      if (outend - out < 4)
        break;
      *out++ = '&';
      *out++ = 'g';
      *out++ = 't';
      *out++ = ';';
    }
    else if (*in == '&')
    {
      if (outend - out < 5)
        break;
      *out++ = '&';
      *out++ = 'a';
      *out++ = 'm';
      *out++ = 'p';
      *out++ = ';';
    }
    else if (*in == '\r')
    {
      if (outend - out < 5)
        break;
      *out++ = '&';
      *out++ = '#';
      *out++ = '1';
      *out++ = '3';
      *out++ = ';';
    }
    else
    {
      *out++ = *in;
    }
    ++in;
  }
  *outlen = out - outstart;
  *inlen = in - base;
}


static
void attr_escape(const char *in, size_t *inlen, char* out, size_t *outlen)
{
  char* outstart = out;
  const char* base = in;
  char* outend = out + *outlen;
  const char* inend;

  inend = in + (*inlen);

  while ((in < inend) && (out < outend))
  {
    if (*in == '"')
    {
      if (outend - out < 2)
        break;
      *out++ = '\\';
      *out++ = '"';
    }
    else
    {
      *out++ = *in;
    }
    ++in;
  }
  *outlen = out - outstart;
  *inlen = in - base;
}


static
char *xml_escape_string(const char *in, size_t inlen,
                        char *out, size_t outlen)
{
  if (in != NULL)
  {
    --outlen;
    xml_escape(in, &inlen, out, &outlen);
    out[outlen]= 0;
  }
  else
  {
    out= 0;
  }
  return out;
}

static
char *attr_escape_string(const char *in, size_t inlen,
                         char *out, size_t outlen)
{
  if (in != NULL)
  {
    --outlen;
    attr_escape(in, &inlen, out, &outlen);
    out[outlen]= 0;
  }
  else
  {
    out= 0;
  }
  return out;
}

static
char *escape_string(const char *in, size_t inlen,
                         char *out, size_t outlen)
{
  typedef char *(*escape_func_t)(const char *, size_t, char *, size_t);
  const escape_func_t escape_func[] = { attr_escape_string, xml_escape_string };

  return escape_func[audit_log_format](in, inlen, out, outlen);
}

static
void logger_write_safe(LOGGER_HANDLE *log, const char *buffer, size_t size)
{
  static int write_error= 0;

  if (log != NULL)
  {
    if (logger_write(log, buffer, size) < 0)
    {
      if (!write_error)
      {
        write_error= 1;
        fprintf_timestamp(stderr);
        fprintf(stderr, "Error writing to file %s. ", audit_log_file);
        perror("Error: ");
      }
    }
    else
    {
      write_error= 0;
    }
  }
}


static 
void logger_write_safe_void(void *log, const char *buffer, size_t size)
{
  logger_write_safe((LOGGER_HANDLE *)log, buffer, size);
}


static
void audit_log_write_without_buffer(const char *buf, size_t len)
{
  logger_write_safe(audit_file_logger, buf, len);
  if (audit_log_strategy == SYNCHRONOUS && audit_file_logger != NULL)
  {
    logger_sync(audit_file_logger);
  }
}


static
void audit_log_write(const char *buf, size_t len)
{
  switch (audit_log_strategy)
  {
    case ASYNCHRONOUS:
    case PERFORMANCE:
      if (audit_log_buffer != NULL)
        audit_log_buffer_write(audit_log_buffer, buf, len);
      break;
    case SEMISYNCHRONOUS:
    case SYNCHRONOUS:
      audit_log_write_without_buffer(buf, len);
      break;
    default:
      DBUG_ASSERT(0);
  }
}



/* Defined in MySQL server */
extern int orig_argc;
extern char **orig_argv;
extern char server_version[SERVER_VERSION_LENGTH];

static
char *make_argv(char *buf, size_t len, int argc, char **argv)
{
  size_t left= len;

  buf[0]= 0;
  while (argc > 0 && left > 0)
  {
    left-= my_snprintf(buf + len - left, left,
                       "%s%c", *argv, argc > 1 ? ' ' : 0);
    argc--; argv++;
  }

  return buf;
}

static
size_t audit_log_audit_record(char *buf, size_t buflen,
                              const char *name, time_t t)
{
  char id_str[MAX_RECORD_ID_SIZE];
  char timestamp[MAX_TIMESTAMP_SIZE];
  char arg_buf[512];
  const char *format_string[] = {
                     "<AUDIT_RECORD\n"
                     "  \"NAME\"=\"%s\"\n"
                     "  \"RECORD\"=\"%s\"\n"
                     "  \"TIMESTAMP\"=\"%s\"\n"
                     "  \"MYSQL_VERSION\"=\"%s\"\n"
                     "  \"STARTUP_OPTIONS\"=\"%s\"\n"
                     "  \"OS_VERSION\"=\""MACHINE_TYPE"-"SYSTEM_TYPE"\",\n"
                     "/>\n",
                     "<AUDIT_RECORD>\n"
                     "  <NAME>%s</NAME>\n"
                     "  <RECORD>%s</RECORD>\n"
                     "  <TIMESTAMP>%s</TIMESTAMP>\n"
                     "  <MYSQL_VERSION>%s</MYSQL_VERSION>\n"
                     "  <STARTUP_OPTIONS>%s</STARTUP_OPTIONS>\n"
                     "  <OS_VERSION>"MACHINE_TYPE"-"SYSTEM_TYPE"</OS_VERSION>\n"
                     "</AUDIT_RECORD>\n" };

  return my_snprintf(buf, buflen,
                     format_string[audit_log_format],
                     name,
                     make_record_id(id_str, sizeof(id_str)),
                     make_timestamp(timestamp, sizeof(timestamp), t),
                     server_version,
                     make_argv(arg_buf, sizeof(arg_buf),
                               orig_argc - 1, orig_argv + 1));
}


static
size_t audit_log_general_record(char *buf, size_t buflen,
                                const char *name, time_t t, int status,
                                const struct mysql_event_general *event)
{
  char id_str[MAX_RECORD_ID_SIZE];
  char timestamp[MAX_TIMESTAMP_SIZE];
  char query[512];
  const char *format_string[] = {
                     "<AUDIT_RECORD\n"
                     "  \"NAME\"=\"%s\"\n"
                     "  \"RECORD\"=\"%s\"\n"
                     "  \"TIMESTAMP\"=\"%s\"\n"
                     "  \"COMMAND_CLASS\"=\"%s\"\n"
                     "  \"CONNECTION_ID\"=\"%lu\"\n"
                     "  \"STATUS\"=\"%d\"\n"
                     "  \"SQLTEXT\"=\"%s\"\n"
                     "  \"USER\"=\"%s\"\n"
                     "  \"HOST\"=\"%s\"\n"
                     "  \"OS_USER\"=\"%s\"\n"
                     "  \"IP\"=\"%s\"\n"
                     "/>\n",
                     "<AUDIT_RECORD>\n"
                     "  <NAME>%s</NAME>\n"
                     "  <RECORD>%s</RECORD>\n"
                     "  <TIMESTAMP>%s</TIMESTAMP>\n"
                     "  <COMMAND_CLASS>%s</COMMAND_CLASS>\n"
                     "  <CONNECTION_ID>%lu</CONNECTION_ID>\n"
                     "  <STATUS>%d</STATUS>\n"
                     "  <SQLTEXT>%s</SQLTEXT>\n"
                     "  <USER>%s</USER>\n"
                     "  <HOST>%s</HOST>\n"
                     "  <OS_USER>%s</OS_USER>\n"
                     "  <IP>%s</IP>\n"
                     "</AUDIT_RECORD>\n" };

  return my_snprintf(buf, buflen,
                     format_string[audit_log_format],
                     name,
                     make_record_id(id_str, sizeof(id_str)),
                     make_timestamp(timestamp, sizeof(timestamp), t),
                     event->general_sql_command.str,
                     event->general_thread_id, status,
                     escape_string(event->general_query,
                                   event->general_query_length,
                                   query, sizeof(query)),
                     event->general_user,
                     event->general_host.str,
                     event->general_external_user.str,
                     event->general_ip.str);
}

static
size_t audit_log_connection_record(char *buf, size_t buflen,
                                   const char *name, time_t t,
                                   const struct mysql_event_connection *event)
{
  char id_str[MAX_RECORD_ID_SIZE];
  char timestamp[MAX_TIMESTAMP_SIZE];
  const char *format_string[] = {
                     "<AUDIT_RECORD\n"
                     "  \"NAME\"=\"%s\"\n"
                     "  \"RECORD\"=\"%s\"\n"
                     "  \"TIMESTAMP\"=\"%s\"\n"
                     "  \"CONNECTION_ID\"=\"%lu\"\n"
                     "  \"STATUS\"=\"%d\"\n"
                     "  \"USER\"=\"%s\"\n"
                     "  \"PRIV_USER\"=\"%s\"\n"
                     "  \"OS_LOGIN\"=\"%s\"\n"
                     "  \"PROXY_USER\"=\"%s\"\n"
                     "  \"HOST\"=\"%s\"\n"
                     "  \"IP\"=\"%s\"\n"
                     "  \"DB\"=\"%s\"\n"
                     "/>\n",
                     "<AUDIT_RECORD>\n"
                     "  <NAME>%s</NAME>\n"
                     "  <RECORD>%s</RECORD>\n"
                     "  <TIMESTAMP>%s</TIMESTAMP>\n"
                     "  <CONNECTION_ID>%lu</CONNECTION_ID>\n"
                     "  <STATUS>%d</STATUS>\n"
                     "  <USER>%s</USER>\n"
                     "  <PRIV_USER>%s</PRIV_USER>\n"
                     "  <OS_LOGIN>%s</OS_LOGIN>\n"
                     "  <PROXY_USER>%s</PROXY_USER>\n"
                     "  <HOST>%s</HOST>\n"
                     "  <IP>%s</IP>\n"
                     "  <DB>%s</DB>\n"
                     "</AUDIT_RECORD>\n" };

  return my_snprintf(buf, buflen,
                     format_string[audit_log_format],
                      name,
                      make_record_id(id_str, sizeof(id_str)),
                      make_timestamp(timestamp, sizeof(timestamp), t),
                      event->thread_id,
                      event->status,
                      event->user ? event->user : "",
                      event->priv_user ? event->priv_user : "",
                      event->external_user ? event->external_user : "",
                      event->proxy_user ? event->proxy_user : "",
                      event->host ? event->host : "",
                      event->ip ? event->ip : "",
                      event->database ? event->database : "");
}


static
int init_new_log_file()
{
  MY_STAT stat_arg;

  audit_file_logger= logger_open(audit_log_file, audit_log_rotate_on_size,
                            audit_log_rotate_on_size ? audit_log_rotations : 0,
                            audit_log_strategy >= SEMISYNCHRONOUS,
                            &stat_arg);
  if (audit_file_logger == NULL)
  {
    fprintf_timestamp(stderr);
    fprintf(stderr, "Cannot open file %s. ", audit_log_file);
    perror("Error: ");
    return(1);
  }

  log_file_time= stat_arg.st_mtime;

  init_record_id(stat_arg.st_size);

  return(0);
}


static
int reopen_log_file()
{
  MY_STAT stat_arg;

  if (logger_reopen(audit_file_logger, &stat_arg))
  {
    fprintf_timestamp(stderr);
    fprintf(stderr, "Cannot open file %s. ", audit_log_file);
    perror("Error: ");
    return(1);
  }

  log_file_time= stat_arg.st_mtime;

  init_record_id(stat_arg.st_size);

  return(0);
}


static
void close_log_file()
{
  if (audit_file_logger != NULL)
    logger_close(audit_file_logger);
}


static
int audit_log_plugin_init(void *arg __attribute__((unused)))
{
  char buf[1024];
  size_t len;

  if (init_new_log_file())
    return(1);

  if (audit_log_strategy < SEMISYNCHRONOUS)
  {
    audit_log_buffer= audit_log_buffer_init(audit_log_buffer_size,
      audit_log_strategy == PERFORMANCE, logger_write_safe_void,
      audit_file_logger);
  }

  len= audit_log_audit_record(buf, sizeof(buf), "Audit", time(NULL));
  audit_log_write(buf, len);

  return(0);
}


static
int audit_log_plugin_deinit(void *arg __attribute__((unused)))
{
  char buf[1024];
  size_t len;

  len= audit_log_audit_record(buf, sizeof(buf), "NoAudit", time(NULL));
  audit_log_write(buf, len);

  if (audit_log_buffer != NULL)
    audit_log_buffer_shutdown(audit_log_buffer);

  close_log_file();

  return(0);
}


static
int is_event_class_allowed_by_policy(unsigned int class,
                                     enum audit_log_policy_t policy)
{
  static unsigned int class_mask[]=
  {
    MYSQL_AUDIT_GENERAL_CLASSMASK | MYSQL_AUDIT_CONNECTION_CLASSMASK, /* ALL */
    0,                                                             /* NONE */
    MYSQL_AUDIT_CONNECTION_CLASSMASK,                              /* LOGINS */
    MYSQL_AUDIT_GENERAL_CLASSMASK,                                 /* QUERIES */
  };

  return (class_mask[policy] & (1 << class)) != 0;
}


static
void audit_log_notify(MYSQL_THD thd __attribute__((unused)),
                      unsigned int event_class,
                      const void *event)
{
  char buf[1024];
  size_t len;

  if (!is_event_class_allowed_by_policy(event_class, audit_log_policy))
    return;

  if (event_class == MYSQL_AUDIT_GENERAL_CLASS)
  {
    const struct mysql_event_general *event_general=
      (const struct mysql_event_general *) event;
    switch (event_general->event_subclass)
    {
    case MYSQL_AUDIT_GENERAL_STATUS:
      if (event_general->general_command_length == 4 &&
          strncmp(event_general->general_command, "Quit", 4) == 0)
        break;
      len= audit_log_general_record(buf, sizeof(buf),
                                    event_general->general_command,
                                    event_general->general_time,
                                    event_general->general_error_code,
                                    event_general);
      audit_log_write(buf, len);
      break;
    }
  }
  else if (event_class == MYSQL_AUDIT_CONNECTION_CLASS)
  {
    const struct mysql_event_connection *event_connection=
      (const struct mysql_event_connection *) event;
    switch (event_connection->event_subclass)
    {
    case MYSQL_AUDIT_CONNECTION_CONNECT:
      len= audit_log_connection_record(buf, sizeof(buf),
                                       "Connect", time(NULL), event_connection);
      audit_log_write(buf, len);
      break;
    case MYSQL_AUDIT_CONNECTION_DISCONNECT:
      len= audit_log_connection_record(buf, sizeof(buf),
                                       "Quit", time(NULL), event_connection);
      audit_log_write(buf, len);
      break;
   case MYSQL_AUDIT_CONNECTION_CHANGE_USER:
      len= audit_log_connection_record(buf, sizeof(buf),
                                       "Change user", time(NULL),
                                       event_connection);
      audit_log_write(buf, len);
      break;
    default:
      break;
    }
  }
}


/*
 * Plugin system vars
 */

static MYSQL_SYSVAR_STR(file, audit_log_file,
  PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_READONLY,
  "The name of the log file.", NULL, NULL, default_audit_log_file);

static const char *audit_log_policy_names[]=
                    { "ALL", "NONE", "LOGINS", "QUERIES", 0 };

static TYPELIB audit_log_policy_typelib=
{
    array_elements(audit_log_policy_names) - 1, "audit_log_policy_typelib",
    audit_log_policy_names, NULL
};

static MYSQL_SYSVAR_ENUM(policy, audit_log_policy, PLUGIN_VAR_RQCMDARG,
       "The policy controlling the information written by the audit log "
       "plugin to its log file.", NULL, NULL, ALL,
       &audit_log_policy_typelib);

static const char *audit_log_strategy_names[]=
  { "ASYNCHRONOUS", "PERFORMANCE", "SEMISYNCHRONOUS", "SYNCHRONOUS", 0 };
static TYPELIB audit_log_strategy_typelib=
{
    array_elements(audit_log_strategy_names) - 1, "audit_log_strategy_typelib",
    audit_log_strategy_names, NULL
};

static MYSQL_SYSVAR_ENUM(strategy, audit_log_strategy,
       PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_READONLY,
       "The logging method used by the audit log plugin.", NULL, NULL,
       ASYNCHRONOUS, &audit_log_strategy_typelib);

static const char *audit_log_format_names[]=
  { "OLD", "NEW", 0 };
static TYPELIB audit_log_format_typelib=
{
    array_elements(audit_log_format_names) - 1, "audit_log_format_typelib",
    audit_log_format_names, NULL
};

static MYSQL_SYSVAR_ENUM(format, audit_log_format,
       PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_READONLY,
       "The audit log file format.", NULL, NULL,
       ASYNCHRONOUS, &audit_log_format_typelib);

static MYSQL_SYSVAR_ULONGLONG(buffer_size, audit_log_buffer_size,
  PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_READONLY,
  "The size of the buffer for asynchronous logging.",
  NULL, NULL, 1048576UL, 4096UL, ULONGLONG_MAX, 4096UL);

static
void audit_log_rotate_on_size_update(
          MYSQL_THD thd __attribute__((unused)),
          struct st_mysql_sys_var *var __attribute__((unused)),
          void *var_ptr __attribute__((unused)),
          const void *save)
{
  ulonglong new_val= *(ulonglong *)(save);

  if (audit_file_logger)
    logger_set_size_limit(audit_file_logger, new_val);
}

static MYSQL_SYSVAR_ULONGLONG(rotate_on_size, audit_log_rotate_on_size,
  PLUGIN_VAR_RQCMDARG,
  "Maximum size of the log to start the rotation.",
  NULL, audit_log_rotate_on_size_update, 0UL, 0UL, ULONGLONG_MAX, 4096UL);

static
void audit_log_rotations_update(
          MYSQL_THD thd __attribute__((unused)),
          struct st_mysql_sys_var *var __attribute__((unused)),
          void *var_ptr __attribute__((unused)),
          const void *save)
{
  ulonglong new_val= *(ulonglong *)(save);

  if (audit_file_logger)
    logger_set_rotations(audit_file_logger, new_val);
}

static MYSQL_SYSVAR_ULONGLONG(rotations, audit_log_rotations,
  PLUGIN_VAR_RQCMDARG,
  "Maximum number of rotations to keep.",
  NULL, audit_log_rotations_update, 0UL, 0UL, 999UL, 1UL);

static
void audit_log_flush_update(
          MYSQL_THD thd __attribute__((unused)),
          struct st_mysql_sys_var *var __attribute__((unused)),
          void *var_ptr __attribute__((unused)),
          const void *save)
{
  char new_val= *(const char *)(save);

  if (new_val != audit_log_flush && new_val == TRUE)
  {
    audit_log_flush= TRUE;
    reopen_log_file();
    audit_log_flush= FALSE;
  }
}

static MYSQL_SYSVAR_BOOL(flush, audit_log_flush,
       PLUGIN_VAR_OPCMDARG, "Flush the log file.", NULL,
       audit_log_flush_update, 0);

static struct st_mysql_sys_var* audit_log_system_variables[] =
{
  MYSQL_SYSVAR(file),
  MYSQL_SYSVAR(policy),
  MYSQL_SYSVAR(strategy),
  MYSQL_SYSVAR(format),
  MYSQL_SYSVAR(buffer_size),
  MYSQL_SYSVAR(rotate_on_size),
  MYSQL_SYSVAR(rotations),
  MYSQL_SYSVAR(flush),
  NULL
};


/*
  Plugin type-specific descriptor
*/
static struct st_mysql_audit audit_log_descriptor=
{
  MYSQL_AUDIT_INTERFACE_VERSION,                    /* interface version    */
  NULL,                                             /* release_thd function */
  audit_log_notify,                                 /* notify function      */
  { MYSQL_AUDIT_GENERAL_CLASSMASK |
    MYSQL_AUDIT_CONNECTION_CLASSMASK }              /* class mask           */
};

/*
  Plugin status variables for SHOW STATUS
*/

static struct st_mysql_show_var audit_log_status_variables[]=
{
  { 0, 0, 0}
};


/*
  Plugin library descriptor
*/

mysql_declare_plugin(audit_log)
{
  MYSQL_AUDIT_PLUGIN,                     /* type                            */
  &audit_log_descriptor,                  /* descriptor                      */
  "audit_log",                            /* name                            */
  "Percona LLC and/or its affiliates.",   /* author                          */
  "Audit log",                            /* description                     */
  PLUGIN_LICENSE_GPL,
  audit_log_plugin_init,                  /* init function (when loaded)     */
  audit_log_plugin_deinit,                /* deinit function (when unloaded) */
  PLUGIN_VERSION,                         /* version                         */
  audit_log_status_variables,             /* status variables                */
  audit_log_system_variables,             /* system variables                */
  NULL,
  0,
}
mysql_declare_plugin_end;

