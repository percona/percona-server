/*
   Copyright (c) 2013, 2018, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "connection_handler_manager.h"

#include "mysql/thread_pool_priv.h"    // create_thd
#include "mysql/service_thd_wait.h"
#include "mysqld_error.h"              // ER_*
#include "channel_info.h"              // Channel_info
#include "connection_handler_impl.h"   // Per_thread_connection_handler
#include "mysqld.h"                    // max_connections
#include "plugin_connection_handler.h" // Plugin_connection_handler
#include "sql_callback.h"              // MYSQL_CALLBACK
#include "sql_class.h"                 // THD


// Initialize static members
uint Connection_handler_manager::connection_count= 0;
uint Connection_handler_manager::extra_connection_count= 0;
ulong Connection_handler_manager::max_used_connections= 0;
ulong Connection_handler_manager::max_used_connections_time= 0;
THD_event_functions* Connection_handler_manager::event_functions= NULL;
THD_event_functions* Connection_handler_manager::saved_event_functions= NULL;
mysql_mutex_t Connection_handler_manager::LOCK_connection_count;
mysql_cond_t Connection_handler_manager::COND_connection_count;
#ifndef EMBEDDED_LIBRARY
Connection_handler_manager* Connection_handler_manager::m_instance= NULL;
ulong Connection_handler_manager::thread_handling=
  SCHEDULER_ONE_THREAD_PER_CONNECTION;
uint Connection_handler_manager::max_threads= 0;


/**
  Helper functions to allow mysys to call the thread scheduler when
  waiting for locks.
*/

static void scheduler_wait_lock_begin()
{
  THD* thd= current_thd;
  MYSQL_CALLBACK(thd->scheduler, thd_wait_begin, (thd, THD_WAIT_TABLE_LOCK));
}

static void scheduler_wait_lock_end()
{
  THD* thd= current_thd;
  MYSQL_CALLBACK(thd->scheduler, thd_wait_end, (thd));
}

static void scheduler_wait_sync_begin()
{
  THD* thd= current_thd;
  if (likely(thd))
    MYSQL_CALLBACK(thd->scheduler, thd_wait_begin, (thd, THD_WAIT_SYNC));
}

static void scheduler_wait_sync_end()
{
  THD* thd= current_thd;
  if (likely(thd))
    MYSQL_CALLBACK(thd->scheduler, thd_wait_end, (thd));
}


bool Connection_handler_manager::valid_connection_count(
                                               bool extra_port_connection)
{
  bool connection_accepted= true;
  mysql_mutex_lock(&LOCK_connection_count);
  if (extra_port_connection)
  {
    if (extra_connection_count > extra_max_connections)
    {
      connection_accepted= false;
      m_connection_errors_max_connection++;
    }
  }
  else if (connection_count > max_connections)
  {
    connection_accepted= false;
    m_connection_errors_max_connection++;
  }
  mysql_mutex_unlock(&LOCK_connection_count);
  return connection_accepted;
}


bool Connection_handler_manager::check_and_incr_conn_count(
                                               bool extra_port_connection)
{
  bool connection_accepted= true;
  mysql_mutex_lock(&LOCK_connection_count);
  /*
    Here we allow max_connections + 1 clients to connect
    (by checking before we increment by 1).

    The last connection is reserved for SUPER users. This is
    checked later during authentication where valid_connection_count()
    is called for non-SUPER users only.
  */
  if (extra_port_connection)
  {
    if (extra_connection_count > extra_max_connections)
    {
      connection_accepted= false;
      m_connection_errors_max_connection++;
    }
    else
    {
      ++extra_connection_count;
    }
  }
  else if (connection_count > max_connections)
  {
    connection_accepted= false;
    m_connection_errors_max_connection++;
  }
  else
  {
    ++connection_count;
    if (connection_count > max_used_connections)
    {
      max_used_connections= connection_count;
      max_used_connections_time= (ulong)my_time(0);
    }
  }
  mysql_mutex_unlock(&LOCK_connection_count);
  return connection_accepted;
}


#ifdef HAVE_PSI_INTERFACE
static PSI_mutex_key key_LOCK_connection_count;

static PSI_mutex_info all_conn_manager_mutexes[]=
{
  { &key_LOCK_connection_count, "LOCK_connection_count", PSI_FLAG_GLOBAL}
};

static PSI_cond_key key_COND_connection_count;

static PSI_cond_info all_conn_manager_conds[]=
{
  { &key_COND_connection_count, "COND_connection_count", PSI_FLAG_GLOBAL}
};
#endif

bool Connection_handler_manager::init()
{
  // This is a static member function.
  Per_thread_connection_handler::init();

  Connection_handler *connection_handler= NULL;
  switch (Connection_handler_manager::thread_handling)
  {
  case SCHEDULER_ONE_THREAD_PER_CONNECTION:
    connection_handler= new (std::nothrow) Per_thread_connection_handler();
    break;
  case SCHEDULER_NO_THREADS:
    connection_handler= new (std::nothrow) One_thread_connection_handler();
    break;
  case SCHEDULER_THREAD_POOL:
    connection_handler= new (std::nothrow) Thread_pool_connection_handler();
    break;
  default:
    DBUG_ASSERT(false);
  }

  Connection_handler *extra_connection_handler=
    new (std::nothrow) Per_thread_connection_handler();

  if (connection_handler == NULL || extra_connection_handler == NULL)
  {
    // This is a static member function.
    Per_thread_connection_handler::destroy();
    return true;
  }

  m_instance= new (std::nothrow)
    Connection_handler_manager(connection_handler, extra_connection_handler);

  if (m_instance == NULL)
  {
    delete connection_handler;
    // This is a static member function.
    Per_thread_connection_handler::destroy();
    return true;
  }

#ifdef HAVE_PSI_INTERFACE
  int count= array_elements(all_conn_manager_mutexes);
  mysql_mutex_register("sql", all_conn_manager_mutexes, count);

  count= array_elements(all_conn_manager_conds);
  mysql_cond_register("sql", all_conn_manager_conds, count);
#endif

  mysql_mutex_init(key_LOCK_connection_count,
                   &LOCK_connection_count, MY_MUTEX_INIT_FAST);

  mysql_cond_init(key_COND_connection_count, &COND_connection_count);
  max_threads= connection_handler->get_max_threads();

  // Init common callback functions.
  thr_set_lock_wait_callback(scheduler_wait_lock_begin,
                             scheduler_wait_lock_end);
  thr_set_sync_wait_callback(scheduler_wait_sync_begin,
                             scheduler_wait_sync_end);
  return false;
}

void Connection_handler_manager::wait_till_no_connection()
{
  mysql_mutex_lock(&LOCK_connection_count);
  while (connection_count > 0 && extra_connection_count > 0)
  {
    mysql_cond_wait(&COND_connection_count, &LOCK_connection_count);
  }
  mysql_mutex_unlock(&LOCK_connection_count);
}

void Connection_handler_manager::destroy_instance()
{
  Per_thread_connection_handler::destroy();

  if (m_instance != NULL)
  {
    delete m_instance;
    m_instance= NULL;
    mysql_mutex_destroy(&LOCK_connection_count);
    mysql_cond_destroy(&COND_connection_count);
  }
}

void Connection_handler_manager::reset_max_used_connections()
{
  mysql_mutex_lock(&LOCK_connection_count);
  max_used_connections= connection_count;
  max_used_connections_time= (ulong)my_time(0);
  mysql_mutex_unlock(&LOCK_connection_count);
}

void Connection_handler_manager::load_connection_handler(
                                Connection_handler* conn_handler)
{
  // We don't support loading more than one dynamic connection handler
  DBUG_ASSERT(Connection_handler_manager::thread_handling !=
              SCHEDULER_TYPES_COUNT);
  m_saved_connection_handler= m_connection_handler;
  m_saved_thread_handling= Connection_handler_manager::thread_handling;
  m_connection_handler= conn_handler;
  Connection_handler_manager::thread_handling= SCHEDULER_TYPES_COUNT;
  max_threads= m_connection_handler->get_max_threads();
}


bool Connection_handler_manager::unload_connection_handler()
{
  DBUG_ASSERT(m_saved_connection_handler != NULL);
  if (m_saved_connection_handler == NULL)
    return true;
  delete m_connection_handler;
  m_connection_handler= m_saved_connection_handler;
  Connection_handler_manager::thread_handling= m_saved_thread_handling;
  m_saved_connection_handler= NULL;
  m_saved_thread_handling= 0;
  max_threads= m_connection_handler->get_max_threads();
  return false;
}


void
Connection_handler_manager::process_new_connection(Channel_info* channel_info)
{
  if (abort_loop
      || !check_and_incr_conn_count(channel_info->is_on_extra_port()))
  {
    channel_info->send_error_and_close_channel(ER_CON_COUNT_ERROR, 0, true);
    sql_print_warning("%s", ER_DEFAULT(ER_CON_COUNT_ERROR));
    delete channel_info;
    return;
  }

  Connection_handler* handler= channel_info->is_on_extra_port()
      ? m_extra_connection_handler : m_connection_handler;

  if (handler->add_connection(channel_info))
  {
    inc_aborted_connects();
    delete channel_info;
  }
}


THD* create_thd(Channel_info* channel_info)
{
  THD* thd= channel_info->create_thd();
  if (thd == NULL)
    channel_info->send_error_and_close_channel(ER_OUT_OF_RESOURCES, 0, false);

  return thd;
}


void destroy_channel_info(Channel_info* channel_info)
{
  delete channel_info;
}

void dec_connection_count()
{
  Connection_handler_manager::dec_connection_count(false);
}

void increment_aborted_connects()
{
  Connection_handler_manager::get_instance()->inc_aborted_connects();
}
#endif // !EMBEDDED_LIBRARY


extern "C"
{
int my_connection_handler_set(Connection_handler_functions *chf,
                              THD_event_functions *tef)
{
  DBUG_ASSERT(chf != NULL && tef != NULL);
  if (chf == NULL || tef == NULL)
    return 1;

  Plugin_connection_handler *conn_handler=
    new (std::nothrow) Plugin_connection_handler(chf);
  if (conn_handler == NULL)
    return 1;

#ifndef EMBEDDED_LIBRARY
  Connection_handler_manager::get_instance()->
    load_connection_handler(conn_handler);
#endif
  Connection_handler_manager::saved_event_functions=
    Connection_handler_manager::event_functions;
  Connection_handler_manager::event_functions= tef;
  return 0;
}


int my_connection_handler_reset()
{
  Connection_handler_manager::event_functions=
    Connection_handler_manager::saved_event_functions;
#ifndef EMBEDDED_LIBRARY
  return Connection_handler_manager::get_instance()->
    unload_connection_handler();
#else
  return 0;
#endif
}
};
