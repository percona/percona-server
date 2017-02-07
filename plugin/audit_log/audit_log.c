/* Copyright (c) 2014-2016 Percona LLC and/or its affiliates. All rights reserved.

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
#include <my_sys.h>
#include <m_ctype.h>
#include <mysql/plugin.h>
#include <mysql/plugin_audit.h>
#include <mysql/service_security_context.h>
#include <mysqld_error.h>
#include <typelib.h>
#include <mysql_version.h>
#include <mysql_com.h>
#include <syslog.h>

#include "audit_log.h"
#include "logger.h"
#include "buffer.h"
#include "audit_handler.h"
#include "filter.h"

#define PLUGIN_VERSION 0x0002


enum audit_log_policy_t { ALL, NONE, LOGINS, QUERIES };
enum audit_log_strategy_t
  { ASYNCHRONOUS, PERFORMANCE, SEMISYNCHRONOUS, SYNCHRONOUS };
enum audit_log_format_t { OLD, NEW, JSON, CSV };
enum audit_log_handler_t { HANDLER_FILE, HANDLER_SYSLOG };

typedef void (*escape_buf_func_t)(const char *, size_t *, char *, size_t *);

static audit_handler_t *log_handler= NULL;
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
ulong audit_log_handler= HANDLER_FILE;
char *audit_log_syslog_ident;
char default_audit_log_syslog_ident[] = "percona-audit";
ulong audit_log_syslog_facility= 0;
ulong audit_log_syslog_priority= 0;
static char *audit_log_exclude_accounts= NULL;
static char *audit_log_include_accounts= NULL;
static char *audit_log_exclude_databases= NULL;
static char *audit_log_include_databases= NULL;
static char *audit_log_exclude_commands= NULL;
static char *audit_log_include_commands= NULL;

PSI_memory_key key_memory_audit_log_logger_handle;
PSI_memory_key key_memory_audit_log_handler;
PSI_memory_key key_memory_audit_log_buffer;
PSI_memory_key key_memory_audit_log_accounts;
PSI_memory_key key_memory_audit_log_databases;
PSI_memory_key key_memory_audit_log_commands;

static PSI_memory_info all_audit_log_memory[]=
{
  {&key_memory_audit_log_logger_handle, "audit_log_logger_handle", 0},
  {&key_memory_audit_log_handler, "audit_log_handler", 0},
  {&key_memory_audit_log_buffer, "audit_log_buffer", 0},
  {&key_memory_audit_log_accounts, "audit_log_accounts", 0},
  {&key_memory_audit_log_databases, "audit_log_databases", 0},
  {&key_memory_audit_log_commands, "audit_log_commands", 0},
};

static int audit_log_syslog_facility_codes[]=
  { LOG_USER,   LOG_AUTHPRIV, LOG_CRON,   LOG_DAEMON, LOG_FTP,
    LOG_KERN,   LOG_LPR,      LOG_MAIL,   LOG_NEWS,
#if (defined LOG_SECURITY)
    LOG_SECURITY,
#endif
    LOG_SYSLOG, LOG_AUTH,     LOG_UUCP,   LOG_LOCAL0, LOG_LOCAL1,
    LOG_LOCAL2, LOG_LOCAL3,   LOG_LOCAL4, LOG_LOCAL5, LOG_LOCAL6,
    LOG_LOCAL7, 0};


static const char *audit_log_syslog_facility_names[]=
  { "LOG_USER",   "LOG_AUTHPRIV", "LOG_CRON",   "LOG_DAEMON", "LOG_FTP",
    "LOG_KERN",   "LOG_LPR",      "LOG_MAIL",   "LOG_NEWS",
#if (defined LOG_SECURITY)
    "LOG_SECURITY",
#endif
    "LOG_SYSLOG", "LOG_AUTH",     "LOG_UUCP",   "LOG_LOCAL0", "LOG_LOCAL1",
    "LOG_LOCAL2", "LOG_LOCAL3",   "LOG_LOCAL4", "LOG_LOCAL5", "LOG_LOCAL6",
    "LOG_LOCAL7", 0 };


static const int audit_log_syslog_priority_codes[]=
  { LOG_INFO,   LOG_ALERT, LOG_CRIT, LOG_ERR, LOG_WARNING,
    LOG_NOTICE, LOG_EMERG,  LOG_DEBUG, 0 };


static const char *audit_log_syslog_priority_names[]=
  { "LOG_INFO",   "LOG_ALERT", "LOG_CRIT", "LOG_ERR", "LOG_WARNING",
    "LOG_NOTICE", "LOG_EMERG", "LOG_DEBUG", 0 };

static MYSQL_PLUGIN plugin_ptr;

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

void plugin_thdvar_safe_update(MYSQL_THD thd, struct st_mysql_sys_var *var,
                               char **dest, const char *value);

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
  len= my_snprintf(buf, buf_len, "%llu_", next_record_id());

  strftime(buf + len, buf_len - len,
           "%FT%T", gmtime_r(&log_file_time, &tm));

  return buf;
}

typedef struct
{
  char character;
  size_t length;
  const char *replacement;
} escape_rule_t;

static
void escape_buf(const char *in, size_t *inlen, char *out, size_t *outlen,
                const escape_rule_t *escape_rules)
{
  char* outstart = out;
  const char* base = in;
  char* outend = out + *outlen;
  const char* inend;
  const escape_rule_t *rule;
  my_bool replaced;

  inend = in + (*inlen);

  while ((in < inend) && (out < outend))
  {
    replaced= FALSE;
    for (rule= escape_rules; rule->character; rule++)
    {
      if (*in == rule->character)
      {
        if ((outend - out) < (int) rule->length)
          goto end_of_buffer;
        memcpy(out, rule->replacement, rule->length);
        out += rule->length;
        replaced= TRUE;
        break;
      }
    }
    if (!replaced)
      *out++ = *in;
    ++in;
  }
end_of_buffer:
  *outlen = out - outstart;
  *inlen = in - base;
}

static
void xml_escape(const char *in, size_t *inlen, char *out, size_t *outlen)
{
  const escape_rule_t rules[]=
  {
    { '<',  4, "&lt;" },
    { '>',  4, "&gt;" },
    { '&',  5, "&amp;" },
    { '\r', 5, "&#13;" },
    { '\n', 5, "&#10;" },
    { '\t', 5, "&#9;" },
    { '"',  6, "&quot;" },
    { 0,  0, NULL }
  };

  escape_buf(in, inlen, out, outlen, rules);
}

static
void json_escape(const char *in, size_t *inlen, char *out, size_t *outlen)
{
  const escape_rule_t rules[]=
  {
    { '\\', 2, "\\\\" },
    { '"',  2, "\\\"" },
    { '\r',  2, "\\r" },
    { '\n',  2, "\\n" },
    { '/',  2, "\\/" },
    { '\b',  2, "\\b" },
    { '\f',  2, "\\f" },
    { '\t',  2, "\\t" },
    { 0,  0, NULL }
  };

  escape_buf(in, inlen, out, outlen, rules);
}

static
void csv_escape(const char *in, size_t *inlen, char *out, size_t *outlen)
{
  const escape_rule_t rules[]=
  {
    { '"',  2, "\"\"" },
    { 0,  0, NULL }
  };

  escape_buf(in, inlen, out, outlen, rules);
}

static const escape_buf_func_t format_escape_func[]=
  { xml_escape, xml_escape, json_escape, csv_escape };

/*
  Calculate the size of the otput bufer needed to escape the string.

  @param[in]  in           Input string
  @param[in]  len          Length of the input string

  @return
    size of the otput bufer including trailing zero
*/
static
size_t calculate_escape_string_buf_len(const char *in, size_t len)
{
  char tmp[128];
  size_t full_outlen= 0;

  while (len > 0)
  {
    size_t tmp_size= sizeof(tmp);
    size_t inlen= len;
    format_escape_func[audit_log_format](in, &inlen, tmp, &tmp_size);
    in+= inlen;
    len-= inlen;
    full_outlen+= tmp_size;
  }
  return full_outlen + 1;
}

/*
  Escape string according to audit_log_format.

  @param[in]  in           Input string
  @param[in]  inlen        Length of the input string
  @param[in]  out          Output buffer
  @param[in]  outlen       Length of the output buffer
  @param[out] endptr       A pointer to the character after the
                           last escaped character in the output
                           buffer
  @param[out] full_outlen  Length of the output buffer that would
                           be needed to store complete non-truncated
                           escaped input buffer

  @return
    pointer to the beginning of the output buffer
*/
static
char *escape_string(const char *in, size_t inlen,
                    char *out, size_t outlen,
                    char **endptr, size_t *full_outlen)
{
  if (outlen == 0)
  {
    if (endptr)
      *endptr= out;
    if (full_outlen)
      *full_outlen+= calculate_escape_string_buf_len(in, inlen);
  }
  else if (in != NULL)
  {
    size_t inlen_res= inlen;
    --outlen;
    format_escape_func[audit_log_format](in, &inlen_res, out, &outlen);
    out[outlen]= 0;
    if (endptr)
      *endptr= out + outlen + 1;
    if (full_outlen)
    {
      *full_outlen+= outlen;
      *full_outlen+= calculate_escape_string_buf_len(in + inlen_res,
                                                     inlen - inlen_res);
    }
  }
  else
  {
    *out= 0;
    if (endptr)
      *endptr= out + 1;
    if (full_outlen)
      ++(*full_outlen);
  }
  return out;
}

static
void my_plugin_perror(void)
{
  char errbuf[MYSYS_STRERROR_SIZE];
  my_strerror(errbuf, sizeof(errbuf), errno);
  my_plugin_log_message(&plugin_ptr, MY_ERROR_LEVEL, "Error: %s", errbuf);
}

static
void audit_log_write(const char *buf, size_t len)
{
  static int write_error= 0;

  if (audit_handler_write(log_handler, buf, len) < 0)
  {
    if (!write_error)
    {
      write_error= 1;
      my_plugin_log_message(&plugin_ptr, MY_ERROR_LEVEL,
                            "Error writing to file %s.", audit_log_file);
      my_plugin_perror();
    }
  }
  else
  {
    write_error= 0;
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
char *audit_log_audit_record(char *buf, size_t buflen,
                             const char *name, time_t t,
                             size_t *outlen)
{
  char id_str[MAX_RECORD_ID_SIZE];
  char timestamp[MAX_TIMESTAMP_SIZE];
  char arg_buf[512];
  const char *format_string[] = {
                     "<AUDIT_RECORD\n"
                     "  NAME=\"%s\"\n"
                     "  RECORD=\"%s\"\n"
                     "  TIMESTAMP=\"%s\"\n"
                     "  MYSQL_VERSION=\"%s\"\n"
                     "  STARTUP_OPTIONS=\"%s\"\n"
                     "  OS_VERSION=\""MACHINE_TYPE"-"SYSTEM_TYPE"\"\n"
                     "/>\n",

                     "<AUDIT_RECORD>\n"
                     "  <NAME>%s</NAME>\n"
                     "  <RECORD>%s</RECORD>\n"
                     "  <TIMESTAMP>%s</TIMESTAMP>\n"
                     "  <MYSQL_VERSION>%s</MYSQL_VERSION>\n"
                     "  <STARTUP_OPTIONS>%s</STARTUP_OPTIONS>\n"
                     "  <OS_VERSION>"MACHINE_TYPE"-"SYSTEM_TYPE"</OS_VERSION>\n"
                     "</AUDIT_RECORD>\n",

                     "{\"audit_record\":{\"name\":\"%s\",\"record\":\"%s\","
                     "\"timestamp\":\"%s\",\"mysql_version\":\"%s\","
                     "\"startup_optionsi\":\"%s\","
                     "\"os_version\":\""MACHINE_TYPE"-"SYSTEM_TYPE"\"}}\n",

                     "\"%s\",\"%s\",\"%s\",\"%s\",\"%s\","
                     "\""MACHINE_TYPE"-"SYSTEM_TYPE"\"\n" };

  *outlen= snprintf(buf, buflen,
                    format_string[audit_log_format],
                    name,
                    make_record_id(id_str, sizeof(id_str)),
                    make_timestamp(timestamp, sizeof(timestamp), t),
                    server_version,
                    make_argv(arg_buf, sizeof(arg_buf),
                              orig_argc - 1, orig_argv + 1));

  /* make sure that record is not truncated */
  DBUG_ASSERT(buf + *outlen <= buf + buflen);

  return buf;
}


static
char *audit_log_general_record(char *buf, size_t buflen,
                               const char *name, time_t t, int status,
                               const struct mysql_event_general *event,
                               const char *default_db,
                               size_t *outlen)
{
  char id_str[MAX_RECORD_ID_SIZE];
  char timestamp[MAX_TIMESTAMP_SIZE];
  char *query, *user, *host, *external_user, *ip, *db;
  char *endptr= buf, *endbuf= buf + buflen;
  size_t full_outlen= 0, buflen_estimated;
  size_t query_length;

  const char *format_string[] = {
                     "<AUDIT_RECORD\n"
                     "  NAME=\"%s\"\n"
                     "  RECORD=\"%s\"\n"
                     "  TIMESTAMP=\"%s\"\n"
                     "  COMMAND_CLASS=\"%s\"\n"
                     "  CONNECTION_ID=\"%lu\"\n"
                     "  STATUS=\"%d\"\n"
                     "  SQLTEXT=\"%s\"\n"
                     "  USER=\"%s\"\n"
                     "  HOST=\"%s\"\n"
                     "  OS_USER=\"%s\"\n"
                     "  IP=\"%s\"\n"
                     "  DB=\"%s\"\n"
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
                     "  <DB>%s</DB>\n"
                     "</AUDIT_RECORD>\n",

                     "{\"audit_record\":"
                       "{\"name\":\"%s\","
                       "\"record\":\"%s\","
                       "\"timestamp\":\"%s\","
                       "\"command_class\":\"%s\","
                       "\"connection_id\":\"%lu\","
                       "\"status\":%d,"
                       "\"sqltext\":\"%s\","
                       "\"user\":\"%s\","
                       "\"host\":\"%s\","
                       "\"os_user\":\"%s\","
                       "\"ip\":\"%s\","
                       "\"db\":\"%s\"}}\n",

                     "\"%s\",\"%s\",\"%s\",\"%s\",\"%lu\",%d,\"%s\",\"%s\","
                     "\"%s\",\"%s\",\"%s\",\"%s\"\n" };

  query_length= my_charset_utf8mb4_general_ci.mbmaxlen *
                event->general_query.length;

  if (query_length < (size_t) (endbuf - endptr))
  {
    uint errors;
    query_length= my_convert(endptr, query_length,
                             &my_charset_utf8mb4_general_ci,
                             event->general_query.str,
                             event->general_query.length,
                             event->general_charset, &errors);
    query= endptr;
    endptr+= query_length;

    full_outlen+= query_length;

    query= escape_string(query, query_length, endptr, endbuf - endptr,
                         &endptr, &full_outlen);
  }
  else
  {
    endptr= endbuf;
    query= escape_string(event->general_query.str, event->general_query.length,
                         endptr, endbuf - endptr, &endptr, &full_outlen);
    full_outlen+= full_outlen * my_charset_utf8mb4_general_ci.mbmaxlen;
  }

  user= escape_string(event->general_user.str, event->general_user.length,
                      endptr, endbuf - endptr, &endptr, &full_outlen);
  host= escape_string(event->general_host.str, event->general_host.length,
                      endptr, endbuf - endptr, &endptr, &full_outlen);
  external_user= escape_string(event->general_external_user.str,
                               event->general_external_user.length,
                               endptr, endbuf - endptr, &endptr, &full_outlen);
  ip= escape_string(event->general_ip.str, event->general_ip.length,
                    endptr, endbuf - endptr, &endptr, &full_outlen);
  db= escape_string(default_db, strlen(default_db),
                    endptr, endbuf - endptr, &endptr, &full_outlen);

  buflen_estimated= full_outlen * 2 +
                    strlen(format_string[audit_log_format]) +
                    strlen(name) +
                    event->general_sql_command.length +
                    20 + /* general_thread_id */
                    20 + /* status */
                    MAX_RECORD_ID_SIZE + MAX_TIMESTAMP_SIZE;
  if (buflen_estimated > buflen)
  {
    *outlen= buflen_estimated;
    return NULL;
  }

  *outlen= snprintf(endptr, endbuf - endptr,
                    format_string[audit_log_format],
                    name,
                    make_record_id(id_str, sizeof(id_str)),
                    make_timestamp(timestamp, sizeof(timestamp), t),
                    event->general_sql_command.str,
                    event->general_thread_id,
                    status, query, user, host, external_user, ip, db);

  /* make sure that record is not truncated */
  DBUG_ASSERT(endptr + *outlen <= buf + buflen);

  return endptr;
}

static
char *audit_log_connection_record(char *buf, size_t buflen,
                                  const char *name, time_t t,
                                  const struct mysql_event_connection *event,
                                  size_t *outlen)
{
  char id_str[MAX_RECORD_ID_SIZE];
  char timestamp[MAX_TIMESTAMP_SIZE];
  char *user, *priv_user, *external_user, *proxy_user, *host, *ip, *database;
  char *endptr= buf, *endbuf= buf + buflen;

  const char *format_string[] = {
                     "<AUDIT_RECORD\n"
                     "  NAME=\"%s\"\n"
                     "  RECORD=\"%s\"\n"
                     "  TIMESTAMP=\"%s\"\n"
                     "  CONNECTION_ID=\"%lu\"\n"
                     "  STATUS=\"%d\"\n"
                     "  USER=\"%s\"\n"
                     "  PRIV_USER=\"%s\"\n"
                     "  OS_LOGIN=\"%s\"\n"
                     "  PROXY_USER=\"%s\"\n"
                     "  HOST=\"%s\"\n"
                     "  IP=\"%s\"\n"
                     "  DB=\"%s\"\n"
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
                     "</AUDIT_RECORD>\n",

                     "{\"audit_record\":"
                       "{\"name\":\"%s\","
                       "\"record\":\"%s\","
                       "\"timestamp\":\"%s\","
                       "\"connection_id\":\"%lu\","
                       "\"status\":%d,"
                       "\"user\":\"%s\","
                       "\"priv_user\":\"%s\","
                       "\"os_login\":\"%s\","
                       "\"proxy_user\":\"%s\","
                       "\"host\":\"%s\","
                       "\"ip\":\"%s\","
                       "\"db\":\"%s\"}}\n",

                     "\"%s\",\"%s\",\"%s\",\"%lu\",%d,\"%s\",\"%s\",\"%s\","
                     "\"%s\",\"%s\",\"%s\",\"%s\"\n" };

  user= escape_string(event->user.str, event->user.length,
                      endptr, endbuf - endptr, &endptr, NULL);
  priv_user= escape_string(event->priv_user.str,
                           event->priv_user.length,
                           endptr, endbuf - endptr, &endptr, NULL);
  external_user= escape_string(event->external_user.str,
                               event->external_user.length,
                               endptr, endbuf - endptr, &endptr, NULL);
  proxy_user= escape_string(event->proxy_user.str, event->proxy_user.length,
                            endptr, endbuf - endptr, &endptr, NULL);
  host= escape_string(event->host.str, event->host.length,
                      endptr, endbuf - endptr, &endptr, NULL);
  ip= escape_string(event->ip.str, event->ip.length,
                    endptr, endbuf - endptr, &endptr, NULL);
  database= escape_string(event->database.str, event->database.length,
                          endptr, endbuf - endptr, &endptr, NULL);

  DBUG_ASSERT((endptr - buf) * 2 +
              strlen(format_string[audit_log_format]) +
              strlen(name) +
              MAX_RECORD_ID_SIZE +
              MAX_TIMESTAMP_SIZE +
              20 + /* event->thread_id */
              20 /* event->status */
              < buflen);

  *outlen= snprintf(endptr, endbuf - endptr,
                    format_string[audit_log_format],
                    name,
                    make_record_id(id_str, sizeof(id_str)),
                    make_timestamp(timestamp, sizeof(timestamp), t),
                    event->connection_id,
                    event->status, user, priv_user,external_user,
                    proxy_user, host, ip, database);

  /* make sure that record is not truncated */
  DBUG_ASSERT(endptr + *outlen <= buf + buflen);

  return endptr;
}

static
size_t audit_log_header(MY_STAT *stat, char *buf, size_t buflen)
{
  const char *format_string[] = {
                     "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                     "<AUDIT>\n",
                     "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                     "<AUDIT>\n",
                     "",
                     "" };

  DBUG_ASSERT(strcmp(system_charset_info->csname, "utf8") == 0);

  log_file_time= stat->st_mtime;

  init_record_id(stat->st_size);

  if (buf == NULL)
  {
    return 0;
  }

  return my_snprintf(buf, buflen, format_string[audit_log_format]);
}


static
size_t audit_log_footer(char *buf, size_t buflen)
{
  const char *format_string[] = {
                     "</AUDIT>\n",
                     "</AUDIT>\n",
                     "",
                     "" };

  if (buf == NULL)
  {
    return 0;
  }

  return my_snprintf(buf, buflen, format_string[audit_log_format]);
}

static
int init_new_log_file()
{
  if (audit_log_handler == HANDLER_FILE)
  {
    audit_handler_file_config_t opts;
    opts.name= audit_log_file;
    opts.rotate_on_size= audit_log_rotate_on_size;
    opts.rotations= audit_log_rotations;
    opts.sync_on_write= audit_log_strategy == SYNCHRONOUS;
    opts.use_buffer= audit_log_strategy < SEMISYNCHRONOUS;
    opts.buffer_size= audit_log_buffer_size;
    opts.can_drop_data= audit_log_strategy == PERFORMANCE;
    opts.header= audit_log_header;
    opts.footer= audit_log_footer;

    log_handler= audit_handler_file_open(&opts);
    if (log_handler == NULL)
    {
      my_plugin_log_message(&plugin_ptr, MY_ERROR_LEVEL,
                            "Cannot open file %s.", audit_log_file);
      my_plugin_perror();
      return(1);
    }
  }
  else
  {
    audit_handler_syslog_config_t opts;
    opts.facility= audit_log_syslog_facility_codes[audit_log_syslog_facility];
    opts.ident= audit_log_syslog_ident;
    opts.priority= audit_log_syslog_priority_codes[audit_log_syslog_priority];
    opts.header= audit_log_header;
    opts.footer= audit_log_footer;

    log_handler= audit_handler_syslog_open(&opts);
    if (log_handler == NULL)
    {
      my_plugin_log_message(&plugin_ptr, MY_ERROR_LEVEL,
                            "Cannot open syslog.");
      my_plugin_perror();
      return(1);
    }
  }

  return(0);
}


static
int reopen_log_file()
{
  if (audit_handler_flush(log_handler))
  {
    my_plugin_log_message(&plugin_ptr, MY_ERROR_LEVEL, "Cannot open file %s.",
                          audit_log_file);
    my_plugin_perror();
    return(1);
  }

  return(0);
}

typedef struct
{
  /* number of included databases */
  int databases_included;
  /* number of excluded databases */
  int databases_excluded;
  /* number of accessed databases */
  int databases_accessed;
  /* query */
  const char *query;
} query_stack_frame;

typedef struct
{
  size_t size;
  size_t top;
  query_stack_frame *frames;
} query_stack;

/*
 Struct to store various THD specific data
 */
typedef struct
{
  /* size of allocated large buffer for record formatting */
  size_t record_buffer_size;
  /* large buffer for record formatting */
  char *record_buffer;
  /* skip session logging */
  my_bool skip_session;
  /* skip logging for the next query */
  my_bool skip_query;
  /* default database */
  char db[NAME_LEN + 1];
  /* default database candidate */
  char init_db_query[NAME_LEN + 1];
  /* call stack */
  query_stack stack;
} audit_log_thd_local;

/*
 Return pointer to THD specific data.
 */
static
audit_log_thd_local *get_thd_local(MYSQL_THD thd);

/*
 Allocate and return buffer of given size.
 */
static
char *get_record_buffer(MYSQL_THD thd, size_t size);

/*
 Allocate and return given number of stack frames.
 */
static
query_stack_frame *realloc_stack_frames(MYSQL_THD thd, size_t size);


static
int audit_log_plugin_init(MYSQL_PLUGIN plugin_info)
{
  char buf[1024];
  size_t len;
  int count;

  plugin_ptr= plugin_info;

  count= array_elements(all_audit_log_memory);
  mysql_memory_register(AUDIT_LOG_PSI_CATEGORY, all_audit_log_memory, count);
  logger_init_mutexes();

  if (init_new_log_file())
    return(1);

  if (audit_log_audit_record(buf, sizeof(buf), "Audit", time(NULL), &len))
    audit_log_write(buf, len);

  audit_log_filter_init();

  if (audit_log_exclude_accounts != NULL && audit_log_include_accounts != NULL)
  {
    my_plugin_log_message(&plugin_ptr, MY_ERROR_LEVEL,
                          "Both 'audit_log_exclude_accounts' and "
                          "'audit_log_include_accounts' are not NULL\n");
    goto validation_error;
  }

  if (audit_log_exclude_commands != NULL && audit_log_include_commands != NULL)
  {
    my_plugin_log_message(&plugin_ptr, MY_ERROR_LEVEL,
                          "Both 'audit_log_exclude_commands' and "
                          "'audit_log_include_commands' are not NULL\n");
    goto validation_error;
  }

  if (audit_log_exclude_databases != NULL
      && audit_log_include_databases != NULL)
  {
    my_plugin_log_message(&plugin_ptr, MY_ERROR_LEVEL,
                          "Both 'audit_log_exclude_databases' and "
                          "'audit_log_include_databases' are not NULL\n");
    goto validation_error;
  }

  if (audit_log_exclude_accounts != NULL)
  {
    audit_log_exclude_accounts= my_strdup(PSI_NOT_INSTRUMENTED,
                                          audit_log_exclude_accounts,
                                          MYF(MY_FAE));
    audit_log_set_exclude_accounts(audit_log_exclude_accounts);
  }
  if (audit_log_include_accounts != NULL)
  {
    audit_log_include_accounts= my_strdup(PSI_NOT_INSTRUMENTED,
                                          audit_log_include_accounts,
                                          MYF(MY_FAE));
    audit_log_set_exclude_accounts(audit_log_include_accounts);
  }
  if (audit_log_exclude_commands != NULL)
  {
    audit_log_exclude_commands= my_strdup(PSI_NOT_INSTRUMENTED,
                                          audit_log_exclude_commands,
                                          MYF(MY_FAE));
    audit_log_set_exclude_commands(audit_log_exclude_commands);
  }
  if (audit_log_include_commands != NULL)
  {
    audit_log_include_commands= my_strdup(PSI_NOT_INSTRUMENTED,
                                          audit_log_include_commands,
                                          MYF(MY_FAE));
    audit_log_set_include_commands(audit_log_include_commands);
  }
  if (audit_log_exclude_databases != NULL)
  {
    audit_log_exclude_databases= my_strdup(PSI_NOT_INSTRUMENTED,
                                          audit_log_exclude_databases,
                                          MYF(MY_FAE));
    audit_log_set_exclude_databases(audit_log_exclude_databases);
  }
  if (audit_log_include_databases != NULL)
  {
    audit_log_include_databases= my_strdup(PSI_NOT_INSTRUMENTED,
                                          audit_log_include_databases,
                                          MYF(MY_FAE));
    audit_log_set_include_databases(audit_log_include_databases);
  }

  return 0;

validation_error:

  audit_log_exclude_accounts= audit_log_include_accounts= NULL;
  audit_log_exclude_commands= audit_log_include_commands= NULL;
  audit_log_exclude_databases= audit_log_include_databases= NULL;

  return 1;
}


static
int audit_log_plugin_deinit(void *arg MY_ATTRIBUTE((unused)))
{
  char buf[1024];
  size_t len;

  if (audit_log_audit_record(buf, sizeof(buf), "NoAudit", time(NULL), &len))
    audit_log_write(buf, len);

  audit_handler_close(log_handler);

  audit_log_filter_destroy();

  my_free(audit_log_include_accounts);
  my_free(audit_log_exclude_accounts);

  my_free(audit_log_include_databases);
  my_free(audit_log_exclude_databases);

  my_free(audit_log_include_commands);
  my_free(audit_log_exclude_commands);

  return(0);
}


static
int is_event_class_allowed_by_policy(mysql_event_class_t class,
                                     enum audit_log_policy_t policy)
{
  static unsigned int class_mask[]=
  {
    /* ALL */
    (1 << MYSQL_AUDIT_GENERAL_CLASS) | (1 << MYSQL_AUDIT_CONNECTION_CLASS),
    0,                                                        /* NONE */
    (1 << MYSQL_AUDIT_CONNECTION_CLASS),                      /* LOGINS */
    (1 << MYSQL_AUDIT_GENERAL_CLASS),                         /* QUERIES */
  };

  return (class_mask[policy] & (1 << class)) != 0;
}

static
const char *next_word(const char *str, size_t *len,
                      const struct charset_info_st *charset)
{
  while (*str && my_isspace(charset, *str))
  {
    if (*str == '/' && str[1] == '*' && str[2] == '!')
      str+= 3;
    else if (*str == '/' && str[1] == '*')
    {
      while (*str && !(*str == '*' && str[1] == '/'))
        str++;
    }
    else
      str++;
  }

  *len= 0;
  while (str[*len] && my_isvar(charset, str[*len]))
    (*len)++;

  if (*len == 0 && *str == '`')
  {
    (*len)++;
    while (str[*len])
    {
      if (str[*len] == '`' && str[*len + 1] == '`')
        (*len)++;
      else if (str[*len] == '`')
        break;
      (*len)++;
    }
    (*len)++;
  }

  return str;
}


static
my_bool audit_log_update_thd_local(MYSQL_THD thd,
                                   audit_log_thd_local *local,
                                   unsigned int event_class,
                                   const void *event)
{
  DBUG_ASSERT(audit_log_include_accounts == NULL ||
              audit_log_exclude_accounts == NULL);

  DBUG_ASSERT(audit_log_include_databases == NULL ||
              audit_log_exclude_databases == NULL);

  DBUG_ASSERT(audit_log_include_commands == NULL ||
              audit_log_exclude_commands == NULL);

  if (event_class == MYSQL_AUDIT_CONNECTION_CLASS)
  {
    const struct mysql_event_connection *event_connection=
      (const struct mysql_event_connection *) event;
    LEX_STRING priv_user, priv_host;
    MYSQL_SECURITY_CONTEXT ctx;

    if (thd_get_security_context(thd, &ctx))
    {
      my_message(ER_AUDIT_API_ABORT, "Error: can not get security context",
                 MYF(0));
      return FALSE;
    }

    if (security_context_get_option(ctx, "priv_user", &priv_user))
    {
      my_message(ER_AUDIT_API_ABORT, "Error: can not get priv_user from "
                 "security context", MYF(0));
      return FALSE;
    }

    if (security_context_get_option(ctx, "priv_host", &priv_host))
    {
      my_message(ER_AUDIT_API_ABORT, "Error: can not get priv_host from "
                 "security context", MYF(0));
      return FALSE;
    }

    local->skip_session= FALSE;
    if (audit_log_include_accounts != NULL &&
        !audit_log_check_account_included(priv_user.str, priv_user.length,
                                          priv_host.str, priv_host.length))
      local->skip_session= TRUE;
    if (audit_log_exclude_accounts != NULL &&
        audit_log_check_account_excluded(priv_user.str, priv_user.length,
                                         priv_host.str, priv_host.length))
      local->skip_session= TRUE;

    if (event_connection->status == 0)
    {
      /* track default DB change */
      DBUG_ASSERT(event_connection->database.length <= sizeof(local->db));
      memcpy(local->db, event_connection->database.str,
             event_connection->database.length);
      local->db[event_connection->database.length]= 0;
    }
  }
  else if (event_class == MYSQL_AUDIT_GENERAL_CLASS)
  {
    const struct mysql_event_general *event_general=
      (const struct mysql_event_general *) event;

    if (event_general->event_subclass == MYSQL_AUDIT_GENERAL_STATUS)
    {
      local->skip_query= FALSE;

      if (local->stack.frames[local->stack.top].query
          == event_general->general_query.str)
      {
        local->skip_query|= audit_log_include_databases
              && local->stack.frames[local->stack.top].databases_accessed > 0
              && local->stack.frames[local->stack.top].databases_included == 0;

        local->skip_query|= audit_log_exclude_databases
              && local->stack.frames[local->stack.top].databases_accessed > 0
              && local->stack.frames[local->stack.top].databases_excluded
                 == local->stack.frames[local->stack.top].databases_accessed;

        local->stack.frames[local->stack.top].databases_included= 0;
        local->stack.frames[local->stack.top].databases_accessed= 0;
        local->stack.frames[local->stack.top].databases_excluded= 0;
        local->stack.frames[local->stack.top].query= NULL;

        if (local->stack.top > 0)
          --local->stack.top;
       }

      local->skip_query|= audit_log_include_commands
            && !audit_log_check_command_included(
                     event_general->general_sql_command.str,
                     event_general->general_sql_command.length);

      local->skip_query|= audit_log_exclude_commands
            && audit_log_check_command_excluded(
                     event_general->general_sql_command.str,
                     event_general->general_sql_command.length);

      if (!local->skip_query &&
          ((event_general->general_command.length == 4 &&
            strncmp(event_general->general_command.str, "Quit", 4) == 0) ||
           (event_general->general_command.length == 11 &&
            strncmp(event_general->general_command.str,
                    "Change user", 11) == 0)))
        local->skip_query= TRUE;
    }

    if (event_general->event_subclass == MYSQL_AUDIT_GENERAL_LOG &&
        event_general->general_command.length == 7 &&
        strncmp(event_general->general_command.str, "Init DB", 7) == 0 &&
        event_general->general_query.str != NULL &&
        strpbrk("\n\r\t ", event_general->general_query.str) == NULL)
    {
      /* Database is about to be changed. Server doesn't provide database
      name in STATUS event, so remember it now. */

      DBUG_ASSERT(event_general->general_query.length <= sizeof(local->db));
      memcpy(local->db, event_general->general_query.str,
             event_general->general_query.length);
      local->db[event_general->general_query.length]= 0;
    }
    if (event_general->event_subclass == MYSQL_AUDIT_GENERAL_STATUS &&
        event_general->general_sql_command.length == 9 &&
        strncmp(event_general->general_sql_command.str, "change_db", 9) == 0 &&
        event_general->general_command.length == 5 &&
        strncmp(event_general->general_command.str, "Query", 5) == 0 &&
        event_general->general_error_code == 0)
    {
      /* it's "use dbname" query */

      size_t len;
      const char *word;

      word= next_word(event_general->general_query.str, &len,
                      event_general->general_charset);
      if (strncasecmp("use", word, len) == 0)
      {
        uint errors;

        word= next_word(word + len, &len, event_general->general_charset);
        if (*word == '`')
        {
          word++;
          len-= 2;
        }
        len= my_convert(local->db, sizeof(local->db) - 1, system_charset_info,
                        word, len, event_general->general_charset, &errors);
        local->db[len]= 0;
      }
    }
  }
  else if (event_class == MYSQL_AUDIT_TABLE_ACCESS_CLASS)
  {
    const struct mysql_event_table_access *event_table=
      (const struct mysql_event_table_access *) event;

    if (local->stack.frames[local->stack.top].query != event_table->query.str
        && local->stack.frames[local->stack.top].query != NULL)
    {
      if (++local->stack.top >= local->stack.size)
        realloc_stack_frames(thd, local->stack.size * 2);
    }
    local->stack.frames[local->stack.top].query= event_table->query.str;

    ++local->stack.frames[local->stack.top].databases_accessed;

    if (audit_log_include_databases != NULL &&
        audit_log_check_database_included(event_table->table_database.str,
                                          event_table->table_database.length))
      ++local->stack.frames[local->stack.top].databases_included;

    if (audit_log_exclude_databases != NULL &&
        audit_log_check_database_excluded(event_table->table_database.str,
                                          event_table->table_database.length))
      ++local->stack.frames[local->stack.top].databases_excluded;
  }
  return TRUE;
}


static
int audit_log_notify(MYSQL_THD thd MY_ATTRIBUTE((unused)),
                     mysql_event_class_t event_class,
                     const void *event)
{
  char buf[4096];
  char *log_rec = NULL;
  char *allocated_buf= get_record_buffer(thd, 0);
  size_t len, buflen;
  audit_log_thd_local *local= get_thd_local(thd);

  if (!audit_log_update_thd_local(thd, local, event_class, event))
    return 1;

  if (!is_event_class_allowed_by_policy(event_class, audit_log_policy))
    return 0;

  if (local->skip_session)
    return 0;

  if (event_class == MYSQL_AUDIT_GENERAL_CLASS)
  {
    const struct mysql_event_general *event_general=
      (const struct mysql_event_general *) event;
    switch (event_general->event_subclass)
    {
    case MYSQL_AUDIT_GENERAL_STATUS:
      if (local->skip_query)
        break;

      /* use allocated buffer if available */
      if (allocated_buf != NULL)
      {
        log_rec= allocated_buf;
        buflen= local->record_buffer_size;
      }
      else
      {
        log_rec= buf;
        buflen= sizeof(buf);
      }
      log_rec= audit_log_general_record(log_rec, buflen,
                                        event_general->general_command.str,
                                        event_general->general_time,
                                        event_general->general_error_code,
                                        event_general, local->db,
                                        &len);
      if (len > buflen)
      {
        buflen= len * 4;
        log_rec= audit_log_general_record(get_record_buffer(thd, buflen),
                                          buflen,
                                          event_general->general_command.str,
                                          event_general->general_time,
                                          event_general->general_error_code,
                                          event_general, local->db,
                                          &len);
      }
      if (log_rec)
        audit_log_write(log_rec, len);
      break;
    case MYSQL_AUDIT_GENERAL_LOG:
    case MYSQL_AUDIT_GENERAL_ERROR:
    case MYSQL_AUDIT_GENERAL_RESULT:
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
      log_rec= audit_log_connection_record(buf, sizeof(buf), "Connect",
                                           time(NULL), event_connection, &len);
      break;
    case MYSQL_AUDIT_CONNECTION_DISCONNECT:
      log_rec= audit_log_connection_record(buf, sizeof(buf), "Quit",
                                           time(NULL), event_connection, &len);
      break;
   case MYSQL_AUDIT_CONNECTION_CHANGE_USER:
      log_rec= audit_log_connection_record(buf, sizeof(buf), "Change user",
                                           time(NULL), event_connection, &len);
      break;
    default:
      break;
    }
    if (log_rec)
      audit_log_write(log_rec, len);
  }
  return 0;
}


/*
 * Plugin system vars
 */

static MYSQL_SYSVAR_STR(file, audit_log_file,
  PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_READONLY | PLUGIN_VAR_MEMALLOC,
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
       "The logging method used by the audit log plugin, "
       "if FILE handler is used.", NULL, NULL,
       ASYNCHRONOUS, &audit_log_strategy_typelib);

static const char *audit_log_format_names[]=
  { "OLD", "NEW", "JSON", "CSV", 0 };
static TYPELIB audit_log_format_typelib=
{
  array_elements(audit_log_format_names) - 1, "audit_log_format_typelib",
  audit_log_format_names, NULL
};

static MYSQL_SYSVAR_ENUM(format, audit_log_format,
       PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_READONLY,
       "The audit log file format.", NULL, NULL,
       ASYNCHRONOUS, &audit_log_format_typelib);

static const char *audit_log_handler_names[]=
  { "FILE", "SYSLOG", 0 };
static TYPELIB audit_log_handler_typelib=
{
  array_elements(audit_log_handler_names) - 1, "audit_log_handler_typelib",
  audit_log_handler_names, NULL
};

static MYSQL_SYSVAR_ENUM(handler, audit_log_handler,
       PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_READONLY,
       "The audit log handler.", NULL, NULL,
       HANDLER_FILE, &audit_log_handler_typelib);

static MYSQL_SYSVAR_ULONGLONG(buffer_size, audit_log_buffer_size,
  PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_READONLY,
  "The size of the buffer for asynchronous logging, "
  "if FILE handler is used.",
  NULL, NULL, 1048576UL, 4096UL, ULLONG_MAX, 4096UL);

static
void audit_log_rotate_on_size_update(
          MYSQL_THD thd MY_ATTRIBUTE((unused)),
          struct st_mysql_sys_var *var MY_ATTRIBUTE((unused)),
          void *var_ptr MY_ATTRIBUTE((unused)),
          const void *save)
{
  ulonglong new_val= *(ulonglong *)(save);

  audit_handler_set_option(log_handler, OPT_ROTATE_ON_SIZE, &new_val);

  audit_log_rotate_on_size= new_val;
}

static MYSQL_SYSVAR_ULONGLONG(rotate_on_size, audit_log_rotate_on_size,
  PLUGIN_VAR_RQCMDARG,
  "Maximum size of the log to start the rotation, if FILE handler is used.",
  NULL, audit_log_rotate_on_size_update, 0UL, 0UL, ULLONG_MAX, 4096UL);

static
void audit_log_rotations_update(
          MYSQL_THD thd MY_ATTRIBUTE((unused)),
          struct st_mysql_sys_var *var MY_ATTRIBUTE((unused)),
          void *var_ptr MY_ATTRIBUTE((unused)),
          const void *save)
{
  ulonglong new_val= *(ulonglong *)(save);

  audit_handler_set_option(log_handler, OPT_ROTATIONS, &new_val);

  audit_log_rotations= new_val;
}

static MYSQL_SYSVAR_ULONGLONG(rotations, audit_log_rotations,
  PLUGIN_VAR_RQCMDARG,
  "Maximum number of rotations to keep, if FILE handler is used.",
  NULL, audit_log_rotations_update, 0UL, 0UL, 999UL, 1UL);

static
void audit_log_flush_update(
          MYSQL_THD thd MY_ATTRIBUTE((unused)),
          struct st_mysql_sys_var *var MY_ATTRIBUTE((unused)),
          void *var_ptr MY_ATTRIBUTE((unused)),
          const void *save)
{
  char new_val= *(const char *)(save);

  if (new_val != audit_log_flush && new_val)
  {
    audit_log_flush= TRUE;
    reopen_log_file();
    audit_log_flush= FALSE;
  }
}

static MYSQL_SYSVAR_BOOL(flush, audit_log_flush,
       PLUGIN_VAR_OPCMDARG, "Flush the log file.", NULL,
       audit_log_flush_update, 0);

static MYSQL_SYSVAR_STR(syslog_ident, audit_log_syslog_ident,
  PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_READONLY | PLUGIN_VAR_MEMALLOC,
  "The string that will be prepended to each log message, "
  "if SYSLOG handler is used.",
  NULL, NULL, default_audit_log_syslog_ident);

static TYPELIB audit_log_syslog_facility_typelib=
{
  array_elements(audit_log_syslog_facility_names) - 1,
  "audit_log_syslog_facility_typelib",
  audit_log_syslog_facility_names, NULL
};

static MYSQL_SYSVAR_ENUM(syslog_facility, audit_log_syslog_facility,
       PLUGIN_VAR_RQCMDARG,
       "The syslog facility to assign to messages, if SYSLOG handler is used.",
       NULL, NULL, 0,
       &audit_log_syslog_facility_typelib);

static TYPELIB audit_log_syslog_priority_typelib=
{
  array_elements(audit_log_syslog_priority_names) - 1,
  "audit_log_syslog_priority_typelib",
  audit_log_syslog_priority_names, NULL
};

static MYSQL_SYSVAR_ENUM(syslog_priority, audit_log_syslog_priority,
       PLUGIN_VAR_RQCMDARG,
       "Priority to be assigned to all messages written to syslog.",
       NULL, NULL, 0,
       &audit_log_syslog_priority_typelib);

static MYSQL_THDVAR_STR(record_buffer,
                        PLUGIN_VAR_READONLY | PLUGIN_VAR_MEMALLOC | \
                        PLUGIN_VAR_NOSYSVAR | PLUGIN_VAR_NOCMDOPT,
                        "Buffer for query formatting.", NULL, NULL, "");

static MYSQL_THDVAR_STR(query_stack,
                        PLUGIN_VAR_READONLY | PLUGIN_VAR_MEMALLOC | \
                        PLUGIN_VAR_NOSYSVAR | PLUGIN_VAR_NOCMDOPT,
                        "Query stack.", NULL, NULL, "");

static
int
audit_log_exclude_accounts_validate(
          MYSQL_THD thd MY_ATTRIBUTE((unused)),
          struct st_mysql_sys_var *var MY_ATTRIBUTE((unused)),
          void *save,
          struct st_mysql_value *value)
{
  const char *new_val;
  char buf[80];
  int len= sizeof(buf);

  if (audit_log_include_accounts)
    return 1;

  new_val = value->val_str(value, buf, &len);

  *(const char **)(save) = new_val;

  return 0;
}

static
void audit_log_exclude_accounts_update(
          MYSQL_THD thd MY_ATTRIBUTE((unused)),
          struct st_mysql_sys_var *var MY_ATTRIBUTE((unused)),
          void *var_ptr MY_ATTRIBUTE((unused)),
          const void *save)
{
  const char *new_val= *(const char **)(save);

  DBUG_ASSERT(audit_log_include_accounts == NULL);

  my_free(audit_log_exclude_accounts);
  audit_log_exclude_accounts= NULL;

  if (new_val != NULL)
  {
    audit_log_exclude_accounts= my_strdup(PSI_NOT_INSTRUMENTED,
                                          new_val, MYF(MY_FAE));
    audit_log_set_exclude_accounts(audit_log_exclude_accounts);
  }
  else
  {
    audit_log_set_exclude_accounts("");
  }
}

static MYSQL_SYSVAR_STR(exclude_accounts, audit_log_exclude_accounts,
       PLUGIN_VAR_RQCMDARG,
       "Comma separated list of accounts "
       "for which events should not be logged.",
       audit_log_exclude_accounts_validate,
       audit_log_exclude_accounts_update, NULL);

static
int
audit_log_include_accounts_validate(
          MYSQL_THD thd MY_ATTRIBUTE((unused)),
          struct st_mysql_sys_var *var MY_ATTRIBUTE((unused)),
          void *save,
          struct st_mysql_value *value)
{
  const char *new_val;
  char buf[80];
  int len= sizeof(buf);

  if (audit_log_exclude_accounts)
    return 1;

  new_val = value->val_str(value, buf, &len);

  *(const char **)(save) = new_val;

  return 0;
}

static
void audit_log_include_accounts_update(
          MYSQL_THD thd MY_ATTRIBUTE((unused)),
          struct st_mysql_sys_var *var MY_ATTRIBUTE((unused)),
          void *var_ptr MY_ATTRIBUTE((unused)),
          const void *save)
{
  const char *new_val= *(const char **)(save);

  DBUG_ASSERT(audit_log_exclude_accounts == NULL);

  my_free(audit_log_include_accounts);
  audit_log_include_accounts= NULL;

  if (new_val != NULL)
  {
    audit_log_include_accounts= my_strdup(PSI_NOT_INSTRUMENTED,
                                          new_val, MYF(MY_FAE));
    audit_log_set_include_accounts(audit_log_include_accounts);
  }
  else
  {
    audit_log_set_include_accounts("");
  }
}

static MYSQL_SYSVAR_STR(include_accounts, audit_log_include_accounts,
       PLUGIN_VAR_RQCMDARG,
       "Comma separated list of accounts for which events should be logged.",
       audit_log_include_accounts_validate,
       audit_log_include_accounts_update, NULL);

static
int
audit_log_exclude_databases_validate(
          MYSQL_THD thd MY_ATTRIBUTE((unused)),
          struct st_mysql_sys_var *var MY_ATTRIBUTE((unused)),
          void *save,
          struct st_mysql_value *value)
{
  const char *new_val;
  char buf[80];
  int len= sizeof(buf);

  if (audit_log_include_databases)
    return 1;

  new_val = value->val_str(value, buf, &len);

  *(const char **)(save) = new_val;

  return 0;
}

static
void audit_log_exclude_databases_update(
          MYSQL_THD thd MY_ATTRIBUTE((unused)),
          struct st_mysql_sys_var *var MY_ATTRIBUTE((unused)),
          void *var_ptr MY_ATTRIBUTE((unused)),
          const void *save)
{
  const char *new_val= *(const char **)(save);

  DBUG_ASSERT(audit_log_include_databases == NULL);

  my_free(audit_log_exclude_databases);
  audit_log_exclude_databases= NULL;

  if (new_val != NULL)
  {
    audit_log_exclude_databases= my_strdup(PSI_NOT_INSTRUMENTED,
                                          new_val, MYF(MY_FAE));
    audit_log_set_exclude_databases(audit_log_exclude_databases);
  }
  else
  {
    audit_log_set_exclude_databases("");
  }
}

static MYSQL_SYSVAR_STR(exclude_databases, audit_log_exclude_databases,
       PLUGIN_VAR_RQCMDARG,
       "Comma separated list of databases "
       "for which events should not be logged.",
       audit_log_exclude_databases_validate,
       audit_log_exclude_databases_update, NULL);

static
int
audit_log_include_databases_validate(
          MYSQL_THD thd MY_ATTRIBUTE((unused)),
          struct st_mysql_sys_var *var MY_ATTRIBUTE((unused)),
          void *save,
          struct st_mysql_value *value)
{
  const char *new_val;
  char buf[80];
  int len= sizeof(buf);

  if (audit_log_exclude_databases)
    return 1;

  new_val = value->val_str(value, buf, &len);

  *(const char **)(save) = new_val;

  return 0;
}

static
void audit_log_include_databases_update(
          MYSQL_THD thd MY_ATTRIBUTE((unused)),
          struct st_mysql_sys_var *var MY_ATTRIBUTE((unused)),
          void *var_ptr MY_ATTRIBUTE((unused)),
          const void *save)
{
  const char *new_val= *(const char **)(save);

  DBUG_ASSERT(audit_log_exclude_databases == NULL);

  my_free(audit_log_include_databases);
  audit_log_include_databases= NULL;

  if (new_val != NULL)
  {
    audit_log_include_databases= my_strdup(PSI_NOT_INSTRUMENTED,
                                          new_val, MYF(MY_FAE));
    audit_log_set_include_databases(audit_log_include_databases);
  }
  else
  {
    audit_log_set_include_databases("");
  }
}

static MYSQL_SYSVAR_STR(include_databases, audit_log_include_databases,
       PLUGIN_VAR_RQCMDARG,
       "Comma separated list of databases for which events should be logged.",
       audit_log_include_databases_validate,
       audit_log_include_databases_update, NULL);

static
int
audit_log_exclude_commands_validate(
          MYSQL_THD thd MY_ATTRIBUTE((unused)),
          struct st_mysql_sys_var *var MY_ATTRIBUTE((unused)),
          void *save,
          struct st_mysql_value *value)
{
  const char *new_val;
  char buf[80];
  int len= sizeof(buf);

  if (audit_log_include_commands)
    return 1;

  new_val = value->val_str(value, buf, &len);

  *(const char **)(save) = new_val;

  return 0;
}

static
void audit_log_exclude_commands_update(
          MYSQL_THD thd MY_ATTRIBUTE((unused)),
          struct st_mysql_sys_var *var MY_ATTRIBUTE((unused)),
          void *var_ptr MY_ATTRIBUTE((unused)),
          const void *save)
{
  const char *new_val= *(const char **)(save);

  DBUG_ASSERT(audit_log_include_commands == NULL);

  my_free(audit_log_exclude_commands);
  audit_log_exclude_commands= NULL;

  if (new_val != NULL)
  {
    audit_log_exclude_commands= my_strdup(PSI_NOT_INSTRUMENTED,
                                          new_val, MYF(MY_FAE));
    audit_log_set_exclude_commands(audit_log_exclude_commands);
  }
  else
  {
    audit_log_set_exclude_commands("");
  }
}

static MYSQL_SYSVAR_STR(exclude_commands, audit_log_exclude_commands,
       PLUGIN_VAR_RQCMDARG,
       "Comma separated list of commands "
       "for which events should not be logged.",
       audit_log_exclude_commands_validate,
       audit_log_exclude_commands_update, NULL);

static
int
audit_log_include_commands_validate(
          MYSQL_THD thd MY_ATTRIBUTE((unused)),
          struct st_mysql_sys_var *var MY_ATTRIBUTE((unused)),
          void *save,
          struct st_mysql_value *value)
{
  const char *new_val;
  char buf[80];
  int len= sizeof(buf);

  if (audit_log_exclude_commands)
    return 1;

  new_val = value->val_str(value, buf, &len);

  *(const char **)(save) = new_val;

  return 0;
}

static
void audit_log_include_commands_update(
          MYSQL_THD thd MY_ATTRIBUTE((unused)),
          struct st_mysql_sys_var *var MY_ATTRIBUTE((unused)),
          void *var_ptr MY_ATTRIBUTE((unused)),
          const void *save)
{
  const char *new_val= *(const char **)(save);

  DBUG_ASSERT(audit_log_exclude_commands == NULL);

  my_free(audit_log_include_commands);
  audit_log_include_commands= NULL;

  if (new_val != NULL)
  {
    audit_log_include_commands= my_strdup(PSI_NOT_INSTRUMENTED,
                                          new_val, MYF(MY_FAE));
    audit_log_set_include_commands(audit_log_include_commands);
  }
  else
  {
    audit_log_set_include_commands("");
  }
}

static MYSQL_SYSVAR_STR(include_commands, audit_log_include_commands,
       PLUGIN_VAR_RQCMDARG,
       "Comma separated list of commands for which events should be logged.",
       audit_log_include_commands_validate,
       audit_log_include_commands_update, NULL);

static MYSQL_THDVAR_STR(local,
       PLUGIN_VAR_READONLY | PLUGIN_VAR_MEMALLOC | \
       PLUGIN_VAR_NOSYSVAR | PLUGIN_VAR_NOCMDOPT,
       "Local store.", NULL, NULL, "");

static MYSQL_THDVAR_ULONG(local_ptr,
       PLUGIN_VAR_READONLY | PLUGIN_VAR_NOSYSVAR | PLUGIN_VAR_NOCMDOPT,
       "Local store ptr.", NULL, NULL, 0, 0, ULONG_MAX, 0);

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
  MYSQL_SYSVAR(handler),
  MYSQL_SYSVAR(syslog_ident),
  MYSQL_SYSVAR(syslog_priority),
  MYSQL_SYSVAR(syslog_facility),
  MYSQL_SYSVAR(record_buffer),
  MYSQL_SYSVAR(query_stack),
  MYSQL_SYSVAR(exclude_accounts),
  MYSQL_SYSVAR(include_accounts),
  MYSQL_SYSVAR(exclude_databases),
  MYSQL_SYSVAR(include_databases),
  MYSQL_SYSVAR(exclude_commands),
  MYSQL_SYSVAR(include_commands),
  MYSQL_SYSVAR(local),
  MYSQL_SYSVAR(local_ptr),
  NULL
};

char thd_local_init_buf[sizeof(audit_log_thd_local)];

void MY_ATTRIBUTE((constructor)) audit_log_so_init()
{
  memset(thd_local_init_buf, 1, sizeof(thd_local_init_buf) - 1);
  thd_local_init_buf[sizeof(thd_local_init_buf) - 1]= 0;
}

/*
 Return pointer to THD specific data.
 */
static
audit_log_thd_local *get_thd_local(MYSQL_THD thd)
{
  audit_log_thd_local *local= (audit_log_thd_local *) THDVAR(thd, local_ptr);

  compile_time_assert(sizeof(THDVAR(thd, local_ptr)) >= sizeof(void *));

  if (unlikely(local == NULL))
  {
    THDVAR_SET(thd, local, thd_local_init_buf);
    local= (audit_log_thd_local *) THDVAR(thd, local);
    memset(local, 0, sizeof(audit_log_thd_local));
    THDVAR(thd, local_ptr)= (ulong) local;

    realloc_stack_frames(thd, 4);
  }
  return local;
}


/*
 Allocate and return buffer of given size.
 */
static
char *get_record_buffer(MYSQL_THD thd, size_t size)
{
  audit_log_thd_local *local= get_thd_local(thd);
  char *buf= local->record_buffer;

  if (local->record_buffer_size < size)
  {
    local->record_buffer_size= size;

    buf = (char *) my_malloc(PSI_NOT_INSTRUMENTED, size, MYF(MY_FAE));
    memset(buf, 1, size - 1);
    buf[size - 1]= 0;

    THDVAR_SET(thd, record_buffer, buf);

    my_free(buf);

    buf = (char *) THDVAR(thd, record_buffer);
    local->record_buffer = buf;
  }

  return buf;
}


/*
 Allocate and return given number of stack frames.
 */
static
query_stack_frame *realloc_stack_frames(MYSQL_THD thd, size_t size)
{
  audit_log_thd_local *local= get_thd_local(thd);
  query_stack_frame *stack= (query_stack_frame *) THDVAR(thd, query_stack);

  if (local->stack.size < size)
  {
    char *buf= (char *) my_malloc(PSI_NOT_INSTRUMENTED,
                                  (local->stack.size + size) *
                                  sizeof(query_stack_frame),
                                  MYF(MY_FAE));
    memset(buf + local->stack.size * sizeof(query_stack_frame), 1,
           size * sizeof(query_stack_frame) - 1);
    buf[(local->stack.size + size) * sizeof(query_stack_frame) - 1]= 0;
    if (local->stack.size > 0)
      memcpy(buf, stack, local->stack.size * sizeof(query_stack_frame));
    THDVAR_SET(thd, query_stack,
               buf + local->stack.size * sizeof(query_stack_frame));
    stack= (query_stack_frame *) THDVAR(thd, query_stack);
    memset(stack, 0, size * sizeof(query_stack_frame));
    if (local->stack.size > 0)
      memcpy(stack, buf, local->stack.size * sizeof(query_stack_frame));
    local->stack.frames= stack;
    local->stack.size= size;
    my_free(buf);
  }

  return stack;
}


/*
  Plugin type-specific descriptor
*/
static struct st_mysql_audit audit_log_descriptor=
{
  MYSQL_AUDIT_INTERFACE_VERSION,                /* interface version    */
  NULL,                                         /* release_thd function */
  audit_log_notify,                             /* notify function      */
  { MYSQL_AUDIT_GENERAL_ALL,
    MYSQL_AUDIT_CONNECTION_ALL,
    0, 0,
    MYSQL_AUDIT_TABLE_ACCESS_ALL,
    0, 0, 0, 0, 0 }                             /* class mask           */
};

/*
  Plugin status variables for SHOW STATUS
*/

static struct st_mysql_show_var audit_log_status_variables[]=
{
  {NullS, NullS, SHOW_LONG, SHOW_SCOPE_GLOBAL}
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

