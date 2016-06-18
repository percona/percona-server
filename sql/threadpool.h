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
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA */
#ifndef THREADPOOL_INCLUDED
#define THREADPOOL_INCLUDED

#include "conn_handler/connection_handler_manager.h"

#define MAX_THREAD_GROUPS 128

enum tp_high_prio_mode_t {
  TP_HIGH_PRIO_MODE_TRANSACTIONS,
  TP_HIGH_PRIO_MODE_STATEMENTS,
  TP_HIGH_PRIO_MODE_NONE
};

typedef struct st_mysql_show_var SHOW_VAR;

/* Threadpool parameters */
extern uint threadpool_min_threads;  /* Minimum threads in pool */
extern uint threadpool_idle_timeout; /* Shutdown idle worker threads  after this timeout */
extern uint threadpool_size; /* Number of parallel executing threads */
extern uint threadpool_stall_limit;  /* time interval in 10 ms units for stall checks*/
extern uint threadpool_max_threads;  /* Maximum threads in pool */
extern uint threadpool_oversubscribe;  /* Maximum active threads in group */

/* Possible values for thread_pool_high_prio_mode */
extern const char *threadpool_high_prio_mode_names[];

/* Common thread pool routines, suitable for different implementations */
extern void threadpool_remove_connection(THD *thd);
extern int  threadpool_process_request(THD *thd);
extern int  threadpool_add_connection(THD *thd);

/*
  Functions used by scheduler. 
  OS-specific implementations are in
  threadpool_unix.cc or threadpool_win.cc
*/
extern bool tp_init();
extern void tp_wait_begin(THD *, int);
extern void tp_wait_end(THD*);
extern void tp_post_kill_notification(THD *thd);
extern void tp_end(void);

extern THD_event_functions tp_event_functions;

/* Used in SHOW for threadpool_idle_thread_count */
extern int  tp_get_idle_thread_count();

/*
  Threadpool statistics
*/
struct TP_STATISTICS
{
  /* Current number of worker thread. */
  volatile int32 num_worker_threads;
};

extern TP_STATISTICS tp_stats;


/* Functions to set threadpool parameters */
extern void tp_set_min_threads(uint val);
extern void tp_set_max_threads(uint val);
extern void tp_set_threadpool_size(uint val);
extern void tp_set_threadpool_stall_limit(uint val);

extern int show_threadpool_idle_threads(THD *thd, SHOW_VAR *var, char *buff);

#endif /* THREADPOOL_INCLUDED */
