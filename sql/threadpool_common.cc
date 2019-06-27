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

#include <my_global.h>
#include <violite.h>
#include <sql_priv.h>
#include <sql_class.h>
#include <my_pthread.h>
#include <scheduler.h>
#include <sql_connect.h>
#include <sql_audit.h>
#include <debug_sync.h>
#include <threadpool.h>
#include <global_threads.h>
#include <probes_mysql.h>


/* Threadpool parameters */

uint threadpool_min_threads;
uint threadpool_idle_timeout;
uint threadpool_size;
uint threadpool_stall_limit;
uint threadpool_max_threads;
uint threadpool_oversubscribe;

/* Stats */
TP_STATISTICS tp_stats;


extern "C" pthread_key(struct st_my_thread_var*, THR_KEY_mysys);
extern bool do_command(THD*);

/*
  Worker threads contexts, and THD contexts.
  =========================================
  
  Both worker threads and connections have their sets of thread local variables 
  At the moment it is mysys_var (this has specific data for dbug, my_error and 
  similar goodies), and PSI per-client structure.

  Whenever query is executed following needs to be done:

  1. Save worker thread context.
  2. Change TLS variables to connection specific ones using thread_attach(THD*).
     This function does some additional work , e.g setting up 
     thread_stack/thread_ends_here pointers.
  3. Process query
  4. Restore worker thread context.

  Connection login and termination follows similar schema w.r.t saving and 
  restoring contexts. 

  For both worker thread, and for the connection, mysys variables are created 
  using my_thread_init() and freed with my_thread_end().

*/
class Worker_thread_context
{
#ifdef HAVE_PSI_THREAD_INTERFACE
  PSI_thread * const psi_thread;
#endif
  st_my_thread_var * const mysys_var;

public:
  Worker_thread_context() :
#ifdef HAVE_PSI_THREAD_INTERFACE
    psi_thread(PSI_THREAD_CALL(get_thread)()),
#endif
    mysys_var(static_cast<st_my_thread_var *>(pthread_getspecific(THR_KEY_mysys)))
  {
  }

  ~Worker_thread_context()
  {
#ifdef HAVE_PSI_THREAD_INTERFACE
    PSI_THREAD_CALL(set_thread)(psi_thread);
#endif
    pthread_setspecific(THR_KEY_mysys,mysys_var);
    pthread_setspecific(THR_THD, 0);
    pthread_setspecific(THR_MALLOC, 0);
  }
};


/*
  Attach/associate the connection with the OS thread,
*/
static bool thread_attach(THD* thd)
{
  pthread_setspecific(THR_KEY_mysys,thd->mysys_var);
  thd->thread_stack=(char*)&thd;
  thd->store_globals();
#ifdef HAVE_PSI_THREAD_INTERFACE
  PSI_THREAD_CALL(set_thread)(thd->event_scheduler.m_psi);
#endif
  mysql_socket_set_thread_owner(thd->net.vio->mysql_socket);
  return 0;
}

#ifdef HAVE_PSI_STATEMENT_INTERFACE
extern PSI_statement_info stmt_info_new_packet;
#endif

#ifdef HAVE_PSI_INTERFACE
static void threadpool_net_before_header_psi_noop(struct st_net * /* net */,
                                                  void * /* user_data */,
                                                  size_t /* count */)
{ }

static void threadpool_net_after_header_psi(struct st_net *net,
                                            void *user_data,
                                            size_t /* count */, my_bool rc)
{
  THD *thd;
  thd= static_cast<THD*> (user_data);
  DBUG_ASSERT(thd != NULL);

  if (thd->m_server_idle)
  {
    /*
      The server just got data for a network packet header,
      from the network layer.
      The IDLE event is now complete, since we now have a message to process.
      We need to:
      - start a new STATEMENT event
      - start a new STAGE event, within this statement,
      - start recording SOCKET WAITS events, within this stage.
      The proper order is critical to get events numbered correctly,
      and nested in the proper parent.
    */
    MYSQL_END_IDLE_WAIT(thd->m_idle_psi);

    if (! rc)
    {
      thd->m_statement_psi= MYSQL_START_STATEMENT(&thd->m_statement_state,
                                                  stmt_info_new_packet.m_key,
                                                  thd->db, thd->db_length,
                                                  thd->charset());

      THD_STAGE_INFO(thd, stage_init);
    }

    /*
      TODO: consider recording a SOCKET event for the bytes just read,
      by also passing count here.
    */
    MYSQL_SOCKET_SET_STATE(net->vio->mysql_socket, PSI_SOCKET_STATE_ACTIVE);

    thd->m_server_idle = false;
  }
}
#endif

static void threadpool_init_net_server_extension(THD *thd)
{
#ifdef HAVE_PSI_INTERFACE
  /* Start with a clean state for connection events. */
  thd->m_idle_psi= NULL;
  thd->m_statement_psi= NULL;
  thd->m_server_idle= false;
  /* Hook up the NET_SERVER callback in the net layer. */
  thd->m_net_server_extension.m_user_data= thd;
  thd->m_net_server_extension.m_before_header= threadpool_net_before_header_psi_noop;
  thd->m_net_server_extension.m_after_header= threadpool_net_after_header_psi;
  /* Activate this private extension for the mysqld server. */
  thd->net.extension= & thd->m_net_server_extension;
#else
  thd->net.extension= NULL;
#endif
}

int threadpool_add_connection(THD *thd)
{
  int retval=1;
  Worker_thread_context worker_context;

  /*
    Create a new connection context: mysys_thread_var and PSI thread
    Store them in THD.
  */

  pthread_setspecific(THR_KEY_mysys, 0);
  my_thread_init();
  thd->mysys_var= (st_my_thread_var *)pthread_getspecific(THR_KEY_mysys);
  if (!thd->mysys_var)
  {
    /* Out of memory? */
    return 1;
  }

  /* Create new PSI thread for use with the THD. */
#ifdef HAVE_PSI_THREAD_INTERFACE
  thd->event_scheduler.m_psi=
    PSI_THREAD_CALL(new_thread)(key_thread_one_connection, thd, thd->thread_id);
#endif


  /* Login. */
  thread_attach(thd);
  ulonglong now= my_micro_time();
  thd->prior_thr_create_utime= now;
  thd->start_utime= now;
  thd->thr_create_utime= now;

  if (!setup_connection_thread_globals(thd))
  {
    lex_start(thd);
    bool rc= login_connection(thd);
    MYSQL_AUDIT_NOTIFY_CONNECTION_CONNECT(thd);
    if (!rc)
    {
      prepare_new_connection_state(thd);
      
      /* 
        Check if THD is ok, as prepare_new_connection_state()
        can fail, for example if init command failed.
      */
      if (thd_is_connection_alive(thd))
      {
        retval= 0;
        thd->net.reading_or_writing= 1;
        thd->skip_wait_timeout= true;
        MYSQL_SOCKET_SET_STATE(thd->net.vio->mysql_socket, PSI_SOCKET_STATE_IDLE);
        thd->m_server_idle= true;
        threadpool_init_net_server_extension(thd);
      }

      MYSQL_CONNECTION_START(thd->thread_id, &thd->security_ctx->priv_user[0],
                             (char *) thd->security_ctx->host_or_ip);
    }
  }
  thd->system_tid = -1;
  return retval;
}


void threadpool_remove_connection(THD *thd)
{
  Worker_thread_context worker_context;

  thread_attach(thd);
  thd->net.reading_or_writing= 0;

  end_connection(thd);
  close_connection(thd, 0);

  thd->release_resources();

  remove_global_thread(thd);
  dec_connection_count(thd);

  delete thd;

  /*
    Free resources associated with this connection: 
    mysys thread_var and PSI thread.
  */
  my_thread_end();

  mysql_cond_broadcast(&COND_thread_count);
}

/**
 Process a single client request or a single batch.
*/
int threadpool_process_request(THD *thd)
{
  int retval= 0;
  Worker_thread_context worker_context;

  thread_attach(thd);

  if (thd->killed == THD::KILL_CONNECTION)
  {
    /* 
      killed flag was set by timeout handler 
      or KILL command. Return error.
    */
    retval= 1;
    goto end;
  }


  /*
    In the loop below, the flow is essentially the copy of thead-per-connections
    logic, see do_handle_one_connection() in sql_connect.c

    The goal is to execute a single query, thus the loop is normally executed 
    only once. However for SSL connections, it can be executed multiple times 
    (SSL can preread and cache incoming data, and vio->has_data() checks if it 
    was the case).
  */
  for(;;)
  {
    Vio *vio;
    thd->net.reading_or_writing= 0;
    mysql_audit_release(thd);

    if ((retval= do_command(thd)) != 0)
      goto end;

    if (!thd_is_connection_alive(thd))
    {
      retval= 1;
      goto end;
    }

    vio= thd->net.vio;
    if (!vio->has_data(vio))
    { 
      /* More info on this debug sync is in sql_parse.cc*/
      DEBUG_SYNC(thd, "before_do_command_net_read");
      thd->net.reading_or_writing= 1;
      goto end;
    }
    if (!thd->m_server_idle) {
      MYSQL_SOCKET_SET_STATE(thd->net.vio->mysql_socket, PSI_SOCKET_STATE_IDLE);
      MYSQL_START_IDLE_WAIT(thd->m_idle_psi, &thd->m_idle_state);
      thd->m_server_idle= true;
    }
  }

end:
  if (!retval && !thd->m_server_idle) {
    MYSQL_SOCKET_SET_STATE(thd->net.vio->mysql_socket, PSI_SOCKET_STATE_IDLE);
    MYSQL_START_IDLE_WAIT(thd->m_idle_psi, &thd->m_idle_state);
    thd->m_server_idle= true;
  }

  thd->system_tid = -1;
  return retval;
}


static scheduler_functions tp_scheduler_functions=
{
  0,                                  // max_threads
  NULL,
  NULL,
  tp_init,                            // init
  NULL,                               // init_new_connection_thread
  tp_add_connection,                  // add_connection
  tp_wait_begin,                      // thd_wait_begin
  tp_wait_end,                        // thd_wait_end
  tp_post_kill_notification,          // post_kill_notification
  NULL,                               // end_thread
  tp_end                              // end
};

void pool_of_threads_scheduler(struct scheduler_functions *func,
    ulong *arg_max_connections,
    uint *arg_connection_count)
{
  *func = tp_scheduler_functions;
  func->max_threads= threadpool_max_threads;
  func->max_connections= arg_max_connections;
  func->connection_count= arg_connection_count;
  scheduler_init();
}
