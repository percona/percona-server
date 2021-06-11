/* Copyright (C) 2014 Percona and Sergey Vojtovich

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,  USA */

#ifndef MYSQL_SERVER
#define MYSQL_SERVER
#endif
#include <sql_class.h>
#include <table.h>
#include <sql_show.h>
#include <mysql/plugin_audit.h>
#include <sp_instr.h>
#include <sql_parse.h>
#include <sql_prepare.h>
#include "query_response_time.h"


ulong opt_query_response_time_range_base= QRT_DEFAULT_BASE;
static my_bool opt_query_response_time_stats= FALSE;
static my_bool opt_query_response_time_flush= FALSE;

class qrt_atomic_flag
{
  public:
    explicit qrt_atomic_flag(bool initial_value):
      value_(initial_value ? 1 : 0)
    {}
    void set()
    {
      my_atomic_store32(&value_, 1);
    }
    void clear()
    {
      my_atomic_store32(&value_, 0);
    }
    bool is_set() const
    {
      int32 res= my_atomic_load32(const_cast<volatile int32*>(&value_));
      return res != 0;
    }

  private:
    qrt_atomic_flag(const qrt_atomic_flag&);
    qrt_atomic_flag& operator = (const qrt_atomic_flag&);

    volatile int32 value_;
};
static qrt_atomic_flag qrt_vars_initialized(false);

static void query_response_time_flush_update(
              MYSQL_THD thd __attribute__((unused)),
              struct st_mysql_sys_var *var __attribute__((unused)),
              void *tgt __attribute__((unused)),
              const void *save __attribute__((unused)))
{
  query_response_time_flush();
}


static MYSQL_SYSVAR_ULONG(range_base, opt_query_response_time_range_base,
       PLUGIN_VAR_RQCMDARG,
       "Select base of log for query_response_time ranges."
       "WARNING: change of this variable take effect only after next "
       "FLUSH QUERY_RESPONSE_TIME execution.",
       NULL, NULL, QRT_DEFAULT_BASE, 2, QRT_MAXIMUM_BASE, 1);
static MYSQL_SYSVAR_BOOL(stats, opt_query_response_time_stats,
       PLUGIN_VAR_OPCMDARG,
       "Enable and disable collection of query times.",
       NULL, NULL, FALSE);
static MYSQL_SYSVAR_BOOL(flush, opt_query_response_time_flush,
       PLUGIN_VAR_NOCMDOPT,
       "Update of this variable flushes statistics and re-reads "
       "query_response_time_range_base.",
       NULL, query_response_time_flush_update, FALSE);
#ifndef NDEBUG
static MYSQL_THDVAR_ULONGLONG(exec_time_debug, PLUGIN_VAR_NOCMDOPT,
       "Pretend queries take this many microseconds. When 0 (the default) use "
       "the actual execution time. Used only for debugging.",
       NULL, NULL, 0, 0, LONG_TIMEOUT, 1);
#endif

enum session_stat
{
  session_stat_global,
  session_stat_on,
  session_stat_off
};

static const char *session_stat_names[]= {"GLOBAL", "ON", "OFF", NullS};
static TYPELIB session_stat_typelib= { array_elements(session_stat_names) - 1,
                                       "", session_stat_names, NULL};

static MYSQL_THDVAR_ENUM(session_stats, PLUGIN_VAR_RQCMDARG,
       "Controls query response time statistics collection for the current "
       "session: ON - enable, OFF - disable, GLOBAL - use "
       "query_response_time_stats value", NULL, NULL,
       session_stat_global, &session_stat_typelib);

static struct st_mysql_sys_var *query_response_time_info_vars[]=
{
  MYSQL_SYSVAR(range_base),
  MYSQL_SYSVAR(stats),
  MYSQL_SYSVAR(flush),
#ifndef NDEBUG
  MYSQL_SYSVAR(exec_time_debug),
#endif
  MYSQL_SYSVAR(session_stats),
  NULL
};


ST_FIELD_INFO query_response_time_fields_info[] =
{
  { "TIME",
    QRT_TIME_STRING_LENGTH,
    MYSQL_TYPE_STRING,
    0,
    0,
    "",
    SKIP_OPEN_TABLE },
  { "COUNT",
    MY_INT32_NUM_DECIMAL_DIGITS,
    MYSQL_TYPE_LONG,
    0,
    MY_I_S_UNSIGNED,
    "",
    SKIP_OPEN_TABLE },
  { "TOTAL",
    QRT_TIME_STRING_LENGTH,
    MYSQL_TYPE_STRING,
    0,
    0,
    "",
    SKIP_OPEN_TABLE },
  { 0, 0, MYSQL_TYPE_NULL, 0, 0, 0, 0 }
};


static int query_response_time_info_init(void *p)
{
  ST_SCHEMA_TABLE *i_s_query_response_time= (ST_SCHEMA_TABLE *) p;
  i_s_query_response_time->fields_info= query_response_time_fields_info;
  if (!my_strcasecmp(system_charset_info, i_s_query_response_time->table_name,
                     "QUERY_RESPONSE_TIME"))
    i_s_query_response_time->fill_table= query_response_time_fill;
  else if (!my_strcasecmp(system_charset_info,
                          i_s_query_response_time->table_name,
                          "QUERY_RESPONSE_TIME_READ"))
    i_s_query_response_time->fill_table= query_response_time_fill_ro;
  else if (!my_strcasecmp(system_charset_info,
                          i_s_query_response_time->table_name,
                          "QUERY_RESPONSE_TIME_WRITE"))
    i_s_query_response_time->fill_table= query_response_time_fill_rw;
  else
    assert(0);
  query_response_time_init();
  return 0;
}

static int query_response_time_info_init_main(void *p)
{
  int res= query_response_time_info_init(p);
  qrt_vars_initialized.set();
  return res;
}

static int query_response_time_info_deinit(void *arg __attribute__((unused)))
{
  opt_query_response_time_stats= FALSE;
  query_response_time_free();
  return 0;
}

static int query_response_time_info_deinit_main(void *arg)
{
  qrt_vars_initialized.clear();
  return query_response_time_info_deinit(arg);
}

static struct st_mysql_information_schema query_response_time_info_descriptor=
{ MYSQL_INFORMATION_SCHEMA_INTERFACE_VERSION };

static bool query_response_time_should_log(MYSQL_THD thd)
{
  const enum session_stat session_stat_val= qrt_vars_initialized.is_set() ?
    static_cast<session_stat>(THDVAR(thd, session_stats)) :
    session_stat_off;
  return (session_stat_val == session_stat_on)
    || (session_stat_val == session_stat_global
        && opt_query_response_time_stats);
}

static int query_response_time_audit_notify(MYSQL_THD thd,
                                            mysql_event_class_t event_class,
                                            const void *event)
{
  const struct mysql_event_general *event_general=
    (const struct mysql_event_general *) event;
  assert(event_class == MYSQL_AUDIT_GENERAL_CLASS);
  if (event_general->event_subclass == MYSQL_AUDIT_GENERAL_STATUS &&
      query_response_time_should_log(thd))
  {
    /*
     Get sql command id of currently executed statement
     inside of stored function or procedure. If the command is "PREPARE"
     don't get the statement inside of "PREPARE". If the statement
     is not inside of stored function or procedure get sql command id
     of the statement itself.
    */
    enum_sql_command sql_command=
      (
        thd->lex->sql_command != SQLCOM_PREPARE &&
        thd->sp_runtime_ctx &&
        thd->stmt_arena &&
        ((sp_lex_instr *)thd->stmt_arena)->get_command() >= 0
      ) ?
      (enum_sql_command)((sp_lex_instr *)thd->stmt_arena)->get_command() :
      thd->lex->sql_command;
    if (sql_command == SQLCOM_EXECUTE)
    {
      const LEX_CSTRING *name=
        (
          thd->sp_runtime_ctx &&
          thd->stmt_arena &&
          ((sp_lex_instr *)thd->stmt_arena)->get_prepared_stmt_name()
        )                                                               ?
        /* If we are inside of SP */
        ((sp_lex_instr *)thd->stmt_arena)->get_prepared_stmt_name()     :
        /* otherwise */
        &thd->lex->prepared_stmt_name;
      Prepared_statement *stmt= thd->stmt_map.find_by_name(*name);
      /* In case of EXECUTE <non-existing-PS>, keep SQLCOM_EXECUTE as the
      command. */
      if (likely(stmt && stmt->lex))
        sql_command= stmt->lex->sql_command;
    }
    QUERY_TYPE query_type=
      (sql_command_flags[sql_command] & CF_CHANGES_DATA) ? WRITE : READ;
#ifndef NDEBUG
    if (THDVAR(thd, exec_time_debug)) {
      ulonglong t = THDVAR(thd, exec_time_debug);
      if ((thd->lex->sql_command == SQLCOM_SET_OPTION) ||
          (thd->lex->spname && thd->stmt_arena && thd->sp_runtime_ctx &&
              ((sp_lex_instr *)thd->stmt_arena)->get_command() ==
              SQLCOM_SET_OPTION )) {
          t = 0;
      }
      query_response_time_collect(query_type, t);
    }
    else
#endif
      query_response_time_collect(query_type,
                                  thd->utime_after_query -
                                  thd->utime_after_lock);
  }
  return 0;
}


static struct st_mysql_audit query_response_time_audit_descriptor=
{
  MYSQL_AUDIT_INTERFACE_VERSION, NULL, query_response_time_audit_notify,
  { MYSQL_AUDIT_GENERAL_ALL, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};


mysql_declare_plugin(query_response_time)
{
  MYSQL_INFORMATION_SCHEMA_PLUGIN,
  &query_response_time_info_descriptor,
  "QUERY_RESPONSE_TIME",
  "Percona and Sergey Vojtovich",
  "Query Response Time Distribution INFORMATION_SCHEMA Plugin",
  PLUGIN_LICENSE_GPL,
  query_response_time_info_init_main,
  query_response_time_info_deinit_main,
  0x0100,
  NULL,
  query_response_time_info_vars,
  (void *)"1.0",
  0,
},
{
  MYSQL_INFORMATION_SCHEMA_PLUGIN,
  &query_response_time_info_descriptor,
  "QUERY_RESPONSE_TIME_READ",
  "Percona and Sergey Vojtovich",
  "Query Response Time Distribution INFORMATION_SCHEMA Plugin",
  PLUGIN_LICENSE_GPL,
  query_response_time_info_init,
  query_response_time_info_deinit,
  0x0100,
  NULL,
  NULL,
  (void *)"1.0",
  0,
},
{
  MYSQL_INFORMATION_SCHEMA_PLUGIN,
  &query_response_time_info_descriptor,
  "QUERY_RESPONSE_TIME_WRITE",
  "Percona and Sergey Vojtovich",
  "Query Response Time Distribution INFORMATION_SCHEMA Plugin",
  PLUGIN_LICENSE_GPL,
  query_response_time_info_init,
  query_response_time_info_deinit,
  0x0100,
  NULL,
  NULL,
  (void *)"1.0",
  0,
},
{
  MYSQL_AUDIT_PLUGIN,
  &query_response_time_audit_descriptor,
  "QUERY_RESPONSE_TIME_AUDIT",
  "Percona and Sergey Vojtovich",
  "Query Response Time Distribution Audit Plugin",
  PLUGIN_LICENSE_GPL,
  NULL,
  NULL,
  0x0100,
  NULL,
  NULL,
  (void *)"1.0",
  0,
}
mysql_declare_plugin_end;
