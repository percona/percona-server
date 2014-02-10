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

#include <mysql/plugin.h>
#include <mysql/plugin_audit.h>
#include <my_global.h>
#include <my_sys.h>
#include <my_list.h>
#include <my_pthread.h>
#include <typelib.h>
#include <limits.h>
#include <string.h>

static volatile ulonglong starttime= 0;
static volatile ulonglong concurrency= 0;
static volatile ulonglong busystart= 0;
static volatile ulonglong busytime= 0;
static volatile ulonglong totaltime= 0;
static volatile ulonglong queries= 0;

#ifdef HAVE_PSI_INTERFACE
PSI_mutex_key key_thd_list_mutex;
#endif
mysql_mutex_t thd_list_mutex;

LIST *thd_list_root= NULL;

typedef struct sm_thd_data_struct {
  ulonglong start;
  ulonglong duration;
  ulonglong queries;
  LIST *backref;
} sm_thd_data_t;

typedef enum { CTL_ON= 0, CTL_OFF= 1, CTL_RESET= 2 } sm_ctl_t;
static const char* sm_ctl_names[]= { "ON", "OFF", "RESET", NullS };
static TYPELIB sm_ctl_typelib= {
  array_elements(sm_ctl_names) - 1,
  "",
  sm_ctl_names,
  NULL
};

static
void sm_ctl_update(MYSQL_THD thd, struct st_mysql_sys_var *var,
                   void *var_ptr, const void *save);

static ulong sm_ctl= CTL_OFF;

static
MYSQL_THDVAR_ULONGLONG(thd_data,
  PLUGIN_VAR_READONLY | PLUGIN_VAR_NOSYSVAR | PLUGIN_VAR_NOCMDOPT,
  "scalability metrics data", NULL, NULL, 0, 0, ULONGLONG_MAX, 0);


static MYSQL_SYSVAR_ENUM(
  control,                           /* name */
  sm_ctl,                            /* var */
  PLUGIN_VAR_RQCMDARG,
  "Control the scalability metrics. Use this to turn ON/OFF or RESET metrics.",
  NULL,                              /* check func. */
  sm_ctl_update,                     /* update func. */
  CTL_OFF,                           /* default */
  &sm_ctl_typelib                    /* typelib */
);

static
ulonglong sm_clock_time_get()
{
#if (defined HAVE_CLOCK_GETTIME)
  struct timespec ts;
#ifdef CLOCK_MONOTONIC_RAW
  clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
#else
  clock_gettime(CLOCK_MONOTONIC, &ts);
#endif
  return((ulonglong) ts.tv_sec * 1000000000 + ts.tv_nsec);
#else
  /* since output values measured in microseconds anyway,
  100 nanoseconds precision should be enough here */
  return(my_getsystime() * 100);
#endif
}


/* Get duration in microseconds */
static
ulonglong sm_clock_time_duration(ulonglong beg, ulonglong end)
{
  return((end - beg) / 1000);
}

static
sm_thd_data_t *sm_thd_data_get(MYSQL_THD thd)
{
  sm_thd_data_t *thd_data = (sm_thd_data_t *) (intptr) THDVAR(thd, thd_data);
  if (unlikely(thd_data == NULL))
  {
    thd_data= calloc(sizeof(sm_thd_data_t), 1);
    mysql_mutex_lock(&thd_list_mutex);
    thd_data->backref= list_push(thd_list_root, thd_data);
    mysql_mutex_unlock(&thd_list_mutex);
    THDVAR(thd, thd_data)= (ulonglong) (intptr) thd_data;
  }
  return thd_data;
}


static
void sm_thd_data_release(MYSQL_THD thd)
{
  sm_thd_data_t *thd_data = (sm_thd_data_t *) (intptr) THDVAR(thd, thd_data);
  if (likely(thd_data != NULL && thd_data->backref != NULL))
  {
    (void) __sync_add_and_fetch(&queries, thd_data->queries);
    (void) __sync_add_and_fetch(&totaltime, thd_data->duration);
    mysql_mutex_lock(&thd_list_mutex);
    thd_list_root= list_delete(thd_list_root, thd_data->backref);
    mysql_mutex_unlock(&thd_list_mutex);
    free(thd_data->backref);
    free(thd_data);
    THDVAR(thd, thd_data)= 0;
  }
}

static
int sm_reset_one(void *data, void *argument __attribute__((unused)))
{
  sm_thd_data_t *thd_data= (sm_thd_data_t *) data;
  thd_data->queries= 0;
  thd_data->duration= 0;
  return(0);
}

static
void sm_reset()
{
  starttime= sm_clock_time_get();
  busytime= totaltime= queries= 0;
  mysql_mutex_lock(&thd_list_mutex);
  list_walk(thd_list_root, sm_reset_one, NULL);
  mysql_mutex_unlock(&thd_list_mutex);
}


static
void sm_ctl_update(MYSQL_THD thd __attribute__((unused)),
                   struct st_mysql_sys_var *var __attribute__((unused)),
                   void *var_ptr __attribute__((unused)),
                   const void *save) {
  ulong new_val= *((sm_ctl_t*) save);

  if (new_val != sm_ctl)
    sm_reset();

  if (new_val != CTL_RESET)
  {
    sm_ctl= new_val;

    if (new_val == CTL_OFF)
    {
      mysql_mutex_lock(&thd_list_mutex);
      list_free(thd_list_root, TRUE);
      thd_list_root= NULL;
      mysql_mutex_unlock(&thd_list_mutex);
    }
  }

}


static
int sm_plugin_init(void *arg __attribute__((unused)))
{
  mysql_mutex_init(key_thd_list_mutex, &thd_list_mutex, MY_MUTEX_INIT_FAST);

  sm_reset();

  return(0);
}


static
int sm_plugin_deinit(void *arg __attribute__((unused)))
{
  list_free(thd_list_root, TRUE);
  thd_list_root= NULL;

  mysql_mutex_destroy(&thd_list_mutex);

  return(0);
}

static
void sm_query_started(MYSQL_THD thd,
                      const char* query __attribute__((unused))) {

  sm_thd_data_t *thd_data= sm_thd_data_get(thd);

  if (__sync_bool_compare_and_swap(&concurrency, 0, 1))
  {
    thd_data->start= sm_clock_time_get();
    busystart= thd_data->start;
  }
  else
  {
    thd_data->start= sm_clock_time_get();
    (void) __sync_add_and_fetch(&concurrency, 1);
  }
}

static
void sm_query_finished(MYSQL_THD thd,
                       const char* query __attribute__((unused))) {

  sm_thd_data_t *thd_data= sm_thd_data_get(thd);
  ulonglong end, save_busystart;

  if (thd_data->start != 0)
  {
    save_busystart= busystart;
    if (__sync_sub_and_fetch(&concurrency, 1) == 0)
    {
      end= sm_clock_time_get();
      (void) __sync_add_and_fetch(&busytime,
                           sm_clock_time_duration(save_busystart, end));
    }
    else
    {
      end= sm_clock_time_get();
    }

    thd_data->duration+= sm_clock_time_duration(thd_data->start, end);
    thd_data->queries++;
  }
}

static
void sm_query_failed(MYSQL_THD thd,
                     const char* query,
                     int err __attribute__((unused))) {

  /* currently there is no difference between success and failure */

  sm_query_finished(thd, query);

}


static
int sm_elapsedtime(MYSQL_THD thd __attribute__((unused)),
                   struct st_mysql_show_var* var,
                   char *buff)
{
  *((ulonglong*)buff)= (sm_ctl == CTL_ON) ?
                      sm_clock_time_duration(starttime, sm_clock_time_get()) :
                      0;
  var->type= SHOW_LONGLONG;
  var->value= buff;
  return(0);
}


static
int sm_sum_queries(void *data, void *argument)
{
  sm_thd_data_t *thd_data= (sm_thd_data_t *) data;
  *((ulonglong *) argument)+= thd_data->queries;
  return(0);
}


static
int sm_queries(MYSQL_THD thd __attribute__((unused)),
               struct st_mysql_show_var* var,
               char *buff)
{
  ulonglong sum_queries= 0;

  if (sm_ctl == CTL_ON)
  {
    mysql_mutex_lock(&thd_list_mutex);
    list_walk(thd_list_root, sm_sum_queries, (unsigned char *) &sum_queries);
    mysql_mutex_unlock(&thd_list_mutex);
  }
  *((ulonglong *) buff)= queries + sum_queries;
  var->type= SHOW_LONGLONG;
  var->value= buff;
  return(0);
}


static
int sm_sum_totaltime(void *data, void *argument)
{
  sm_thd_data_t *thd_data= (sm_thd_data_t *) data;
  *((ulonglong *) argument)+= thd_data->duration;
  return(0);
}


static
int sm_totaltime(MYSQL_THD thd __attribute__((unused)),
               struct st_mysql_show_var* var,
               char *buff)
{
  ulonglong sum_totaltime= 0;

  if (sm_ctl == CTL_ON)
  {
    mysql_mutex_lock(&thd_list_mutex);
    list_walk(thd_list_root, sm_sum_totaltime,
              (unsigned char *) &sum_totaltime);
    mysql_mutex_unlock(&thd_list_mutex);
  }
  *((ulonglong *) buff)= totaltime + sum_totaltime;
  var->type= SHOW_LONGLONG;
  var->value= buff;
  return(0);
}


static void sm_notify(MYSQL_THD thd, unsigned int event_class,
                      const void *event)
{

  if (event_class == MYSQL_AUDIT_GENERAL_CLASS)
  {
    const struct mysql_event_general *event_general=
      (const struct mysql_event_general *) event;

    if (sm_ctl != CTL_ON)
    {
      return;
    }

    if (event_general->general_command &&
        event_general->event_subclass == MYSQL_AUDIT_GENERAL_LOG &&
        strcmp(event_general->general_command, "Query") == 0)
    {
      sm_query_started(thd, event_general->general_query);
    }
    else if (event_general->general_command &&
        event_general->event_subclass == MYSQL_AUDIT_GENERAL_LOG &&
        strcmp(event_general->general_command, "Execute") == 0)
    {
      sm_query_started(thd, event_general->general_query);
    }
    else if (event_general->general_query &&
        event_general->event_subclass == MYSQL_AUDIT_GENERAL_RESULT)
    {
      sm_query_finished(thd, event_general->general_query);
    }
    else if (event_general->general_query &&
        event_general->event_subclass == MYSQL_AUDIT_GENERAL_ERROR)
    {
      sm_query_failed(thd, event_general->general_query,
                                event_general->general_error_code);
    }

  }
  else if (event_class == MYSQL_AUDIT_CONNECTION_CLASS)
  {
    const struct mysql_event_connection *event_connection=
      (const struct mysql_event_connection *) event;
    switch (event_connection->event_subclass)
    {
    case MYSQL_AUDIT_CONNECTION_CONNECT:
      sm_thd_data_get(thd);
      break;
    case MYSQL_AUDIT_CONNECTION_DISCONNECT:
      sm_thd_data_release(thd);
      break;
    default:
      break;
    }
  }
}

/*
 * Plugin system vars
 */
static struct st_mysql_sys_var* scalability_metrics_system_variables[] =
{
  MYSQL_SYSVAR(thd_data),
  MYSQL_SYSVAR(control),
  NULL
};

/*
  Plugin type-specific descriptor
*/
static struct st_mysql_audit scalability_metrics_descriptor=
{
  MYSQL_AUDIT_INTERFACE_VERSION,                    /* interface version    */
  NULL,                                             /* release_thd function */
  sm_notify,                                        /* notify function      */
  { MYSQL_AUDIT_GENERAL_CLASSMASK |
    MYSQL_AUDIT_CONNECTION_CLASSMASK }              /* class mask           */
};

/*
  Plugin status variables for SHOW STATUS
*/

static struct st_mysql_show_var simple_status[]=
{
  { "scalability_metrics_elapsedtime", (char *) &sm_elapsedtime, SHOW_FUNC },
  { "scalability_metrics_queries", (char *) &sm_queries, SHOW_FUNC },
  { "scalability_metrics_concurrency", (char *) &concurrency, SHOW_LONGLONG },
  { "scalability_metrics_totaltime", (char *) &sm_totaltime, SHOW_FUNC },
  { "scalability_metrics_busytime", (char *) &busytime, SHOW_LONGLONG },
  { 0, 0, 0}
};


/*
  Plugin library descriptor
*/

mysql_declare_plugin(scalability_metrics)
{
  MYSQL_AUDIT_PLUGIN,                     /* type                            */
  &scalability_metrics_descriptor,        /* descriptor                      */
  "scalability_metrics",                  /* name                            */
  "Percona LLC and/or its affiliates",    /* author                          */
  "Scalability metrics",                  /* description                     */
  PLUGIN_LICENSE_GPL,
  sm_plugin_init,                         /* init function (when loaded)     */
  sm_plugin_deinit,                       /* deinit function (when unloaded) */
  0x0001,                                 /* version                         */
  simple_status,                          /* status variables                */
  scalability_metrics_system_variables,   /* system variables                */
  NULL,
  0,
}
mysql_declare_plugin_end;

