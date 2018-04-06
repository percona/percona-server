/*
   Copyright (c) 2007, 2016, Oracle and/or its affiliates. All rights reserved.

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

/*
  Functions to autenticate and handle reqests for a connection
*/

#include "my_global.h"
#include "sql_priv.h"
#ifndef __WIN__
#include <netdb.h>        // getservbyname, servent
#endif
#include "sql_audit.h"
#include "sql_connect.h"
#include "my_global.h"
#include "probes_mysql.h"
#include "unireg.h"                    // REQUIRED: for other includes
#include "sql_parse.h"                          // sql_command_flags,
                                                // execute_init_command,
                                                // do_command
#include "sql_db.h"                             // mysql_change_db
#include "hostname.h" // inc_host_errors, ip_to_hostname,
                      // reset_host_errors
#include "sql_acl.h"  // acl_getroot, NO_ACCESS, SUPER_ACL
#include "sql_callback.h"
#include "debug_sync.h"


#if defined(HAVE_OPENSSL) && !defined(EMBEDDED_LIBRARY)
/*
  Without SSL the handshake consists of one packet. This packet
  has both client capabilites and scrambled password.
  With SSL the handshake might consist of two packets. If the first
  packet (client capabilities) has CLIENT_SSL flag set, we have to
  switch to SSL and read the second packet. The scrambled password
  is in the second packet and client_capabilites field will be ignored.
  Maybe it is better to accept flags other than CLIENT_SSL from the
  second packet?
*/
#define SSL_HANDSHAKE_SIZE      2
#define NORMAL_HANDSHAKE_SIZE   6
#define MIN_HANDSHAKE_SIZE      2
#else
#define MIN_HANDSHAKE_SIZE      6
#endif /* HAVE_OPENSSL && !EMBEDDED_LIBRARY */

// Uses the THD to update the global stats by user name and client IP
void update_global_user_stats(THD* thd, bool create_user, time_t now);

HASH global_user_stats;
HASH global_client_stats;
HASH global_thread_stats;
// Protects global_user_stats and global_client_stats
extern mysql_mutex_t LOCK_global_user_client_stats;

HASH global_table_stats;
extern mysql_mutex_t LOCK_global_table_stats;

HASH global_index_stats;
extern mysql_mutex_t LOCK_global_index_stats;

/*
  Get structure for logging connection data for the current user
*/

#ifndef NO_EMBEDDED_ACCESS_CHECKS

// Increments connection count for user.
static int increment_connection_count(THD* thd, bool use_lock);

static HASH hash_user_connections;

int get_or_create_user_conn(THD *thd, const char *user,
                            const char *host,
                            const USER_RESOURCES *mqh)
{
  int return_val= 0;
  size_t temp_len, user_len;
  char temp_user[USER_HOST_BUFF_SIZE];
  struct  user_conn *uc;

  DBUG_ASSERT(user != 0);
  DBUG_ASSERT(host != 0);

  user_len= strlen(user);
  temp_len= (strmov(strmov(temp_user, user)+1, host) - temp_user)+1;
  mysql_mutex_lock(&LOCK_user_conn);
  if (!(uc = (struct  user_conn *) my_hash_search(&hash_user_connections,
					       (uchar*) temp_user, temp_len)))
  {
    /* First connection for user; Create a user connection object */
    if (!(uc= ((struct user_conn*)
	       my_malloc(sizeof(struct user_conn) + temp_len+1,
			 MYF(MY_WME)))))
    {
      /* MY_WME ensures an error is set in THD. */
      return_val= 1;
      goto end;
    }
    uc->user=(char*) (uc+1);
    memcpy(uc->user,temp_user,temp_len+1);
    uc->host= uc->user + user_len +  1;
    uc->len= temp_len;
    uc->connections= uc->questions= uc->updates= uc->conn_per_hour= 0;
    uc->user_resources= *mqh;
    uc->reset_utime= thd->thr_create_utime;
    if (my_hash_insert(&hash_user_connections, (uchar*) uc))
    {
      /* The only possible error is out of memory, MY_WME sets an error. */
      my_free(uc);
      return_val= 1;
      goto end;
    }
  }
  thd->set_user_connect(uc);
  thd->increment_user_connections_counter();
end:
  mysql_mutex_unlock(&LOCK_user_conn);
  return return_val;

}

extern "C" uchar *get_key_user_stats(USER_STATS *user_stats, size_t *length,
                         my_bool not_used __attribute__((unused)))
{
  *length= user_stats->user_len;
  return (uchar*) user_stats->user;
}

extern "C" uchar *get_key_thread_stats(THREAD_STATS *thread_stats, size_t *length,
                         my_bool not_used __attribute__((unused)))
{
  *length= sizeof(my_thread_id);
  return (uchar *) &(thread_stats->id);
}

void free_user_stats(USER_STATS* user_stats)
{
  my_free((char *) user_stats);
}

void free_thread_stats(THREAD_STATS* thread_stats)
{
  my_free((char *) thread_stats);
}

void init_user_stats(USER_STATS *user_stats,
                     const char *user,
                     const char *priv_user,
                     uint total_connections,
                     uint total_ssl_connections,
                     uint concurrent_connections,
                     time_t connected_time,
                     double busy_time,
                     double cpu_time,
                     ulonglong bytes_received,
                     ulonglong bytes_sent,
                     ulonglong binlog_bytes_written,
                     ha_rows rows_fetched,
                     ha_rows rows_updated,
                     ha_rows rows_read,
                     ulonglong select_commands,
                     ulonglong update_commands,
                     ulonglong other_commands,
                     ulonglong commit_trans,
                     ulonglong rollback_trans,
                     ulonglong denied_connections,
                     ulonglong lost_connections,
                     ulonglong access_denied_errors,
                     ulonglong empty_queries)
{
  DBUG_ENTER("init_user_stats");
  DBUG_PRINT("info",
             ("Add user_stats entry for user %s - priv_user %s",
              user, priv_user));
  strncpy(user_stats->user, user, sizeof(user_stats->user));
  strncpy(user_stats->priv_user, priv_user, sizeof(user_stats->priv_user));

  user_stats->user_len=               strlen(user_stats->user);
  user_stats->priv_user_len=          strlen(user_stats->priv_user);

  user_stats->total_connections=      total_connections;
  user_stats->total_ssl_connections=  total_ssl_connections;
  user_stats->concurrent_connections= concurrent_connections;
  user_stats->connected_time=         connected_time;
  user_stats->busy_time=              busy_time;
  user_stats->cpu_time=               cpu_time;
  user_stats->bytes_received=         bytes_received;
  user_stats->bytes_sent=             bytes_sent;
  user_stats->binlog_bytes_written=   binlog_bytes_written;
  user_stats->rows_fetched=           rows_fetched;
  user_stats->rows_updated=           rows_updated;
  user_stats->rows_read=              rows_read;
  user_stats->select_commands=        select_commands;
  user_stats->update_commands=        update_commands;
  user_stats->other_commands=         other_commands;
  user_stats->commit_trans=           commit_trans;
  user_stats->rollback_trans=         rollback_trans;
  user_stats->denied_connections=     denied_connections;
  user_stats->lost_connections=       lost_connections;
  user_stats->access_denied_errors=   access_denied_errors;
  user_stats->empty_queries=          empty_queries;
  DBUG_VOID_RETURN;
}

void init_thread_stats(THREAD_STATS *thread_stats,
                     my_thread_id id,
                     uint total_connections,
                     uint total_ssl_connections,
                     uint concurrent_connections,
                     time_t connected_time,
                     double busy_time,
                     double cpu_time,
                     ulonglong bytes_received,
                     ulonglong bytes_sent,
                     ulonglong binlog_bytes_written,
                     ha_rows rows_fetched,
                     ha_rows rows_updated,
                     ha_rows rows_read,
                     ulonglong select_commands,
                     ulonglong update_commands,
                     ulonglong other_commands,
                     ulonglong commit_trans,
                     ulonglong rollback_trans,
                     ulonglong denied_connections,
                     ulonglong lost_connections,
                     ulonglong access_denied_errors,
                     ulonglong empty_queries)
{
  DBUG_ENTER("init_thread_stats");
  DBUG_PRINT("info",
             ("Add thread_stats entry for thread %lu",
              id));
  thread_stats->id= id;

  thread_stats->total_connections=      total_connections;
  thread_stats->total_ssl_connections=  total_ssl_connections;
  thread_stats->concurrent_connections= concurrent_connections;
  thread_stats->connected_time=         connected_time;
  thread_stats->busy_time=              busy_time;
  thread_stats->cpu_time=               cpu_time;
  thread_stats->bytes_received=         bytes_received;
  thread_stats->bytes_sent=             bytes_sent;
  thread_stats->binlog_bytes_written=   binlog_bytes_written;
  thread_stats->rows_fetched=           rows_fetched;
  thread_stats->rows_updated=           rows_updated;
  thread_stats->rows_read=              rows_read;
  thread_stats->select_commands=        select_commands;
  thread_stats->update_commands=        update_commands;
  thread_stats->other_commands=         other_commands;
  thread_stats->commit_trans=           commit_trans;
  thread_stats->rollback_trans=         rollback_trans;
  thread_stats->denied_connections=     denied_connections;
  thread_stats->lost_connections=       lost_connections;
  thread_stats->access_denied_errors=   access_denied_errors;
  thread_stats->empty_queries=          empty_queries;
  DBUG_VOID_RETURN;
}

void init_global_user_stats(void)
{
  if (my_hash_init(&global_user_stats, system_charset_info, max_connections,
                0, 0, (my_hash_get_key)get_key_user_stats,
                (my_hash_free_key)free_user_stats, 0)) {
    sql_print_error("Initializing global_user_stats failed.");
    exit(1);
  }
}

void init_global_client_stats(void)
{
  if (my_hash_init(&global_client_stats, system_charset_info, max_connections,
                0, 0, (my_hash_get_key)get_key_user_stats,
                (my_hash_free_key)free_user_stats, 0)) {
    sql_print_error("Initializing global_client_stats failed.");
    exit(1);
  }
}

void init_global_thread_stats(void)
{
  if (my_hash_init(&global_thread_stats, &my_charset_bin, max_connections,
                0, 0, (my_hash_get_key) get_key_thread_stats,
                (my_hash_free_key) free_thread_stats, 0))
  {
    sql_print_error("Initializing global_client_stats failed.");
    exit(1);
  }
}

extern "C" uchar *get_key_table_stats(TABLE_STATS *table_stats, size_t *length,
                                     my_bool not_used __attribute__((unused)))
{
  *length= table_stats->table_len;
  return (uchar*) table_stats->table;
}

extern "C" void free_table_stats(TABLE_STATS* table_stats)
{
  my_free((char*) table_stats);
}

void init_global_table_stats(void)
{
  if (my_hash_init(&global_table_stats, system_charset_info, max_connections,
                0, 0, (my_hash_get_key)get_key_table_stats,
                (my_hash_free_key)free_table_stats, 0)) {
    sql_print_error("Initializing global_table_stats failed.");
    exit(1);
  }
}

extern "C" uchar *get_key_index_stats(INDEX_STATS *index_stats, size_t *length,
                                     my_bool not_used __attribute__((unused)))
{
  *length= index_stats->index_len;
  return (uchar*) index_stats->index;
}

extern "C" void free_index_stats(INDEX_STATS* index_stats)
{
  my_free((char*) index_stats);
}

void init_global_index_stats(void)
{
  if (my_hash_init(&global_index_stats, system_charset_info, max_connections,
                0, 0, (my_hash_get_key)get_key_index_stats,
                (my_hash_free_key)free_index_stats, 0)) {
    sql_print_error("Initializing global_index_stats failed.");
    exit(1);
  }
}

void free_global_user_stats(void)
{
  my_hash_free(&global_user_stats);
}

void free_global_thread_stats(void)
{
  my_hash_free(&global_thread_stats);
}

void free_global_table_stats(void)
{
  my_hash_free(&global_table_stats);
}

void free_global_index_stats(void)
{
  my_hash_free(&global_index_stats);
}

void free_global_client_stats(void)
{
  my_hash_free(&global_client_stats);
}

// 'mysql_system_user' is used for when the user is not defined for a THD.
static char mysql_system_user[] = "#mysql_system#";

// Returns 'user' if it's not NULL.  Returns 'mysql_system_user' otherwise.
static char* get_valid_user_string(char* user) {
  return user ? user : mysql_system_user;
}

// Increments the global stats connection count for an entry from
// global_client_stats or global_user_stats. Returns 0 on success
// and 1 on error.
static int increment_count_by_name(const char *name, const char *role_name,
                                   HASH *users_or_clients, THD *thd)
{
  USER_STATS* user_stats;

  if (!(user_stats = (USER_STATS *) my_hash_search(users_or_clients,
                                                   (uchar*) name,
                                                   strlen(name))))
  {
    if (acl_is_utility_user(thd->security_ctx->user,
                            thd->security_ctx->get_host()->ptr(),
                            thd->security_ctx->get_ip()->ptr()))
      return 0;

    // First connection for this user or client
    if (!(user_stats = ((USER_STATS *)
                        my_malloc(sizeof(USER_STATS), MYF(MY_WME | MY_ZEROFILL)))))
    {
      return 1; // Out of memory
    }

    init_user_stats(user_stats, name, role_name,
                    0, 0, 0,   // connections
                    0, 0, 0,   // time
                    0, 0, 0,   // bytes sent, received and written
                    0, 0, 0,   // rows fetched, updated and read
                    0, 0, 0,   // select, update and other commands
                    0, 0,      // commit and rollback trans
                    thd->diff_denied_connections,
                    0,         // lost connections
                    0,         // access denied errors
                    0);        // empty queries

    if (my_hash_insert(users_or_clients, (uchar *) user_stats))
    {
      my_free((char *) user_stats);
      return 1; // Out of memory
    }
  }
  user_stats->total_connections++;
  if (thd->net.vio &&  thd->net.vio->type == VIO_TYPE_SSL)
    user_stats->total_ssl_connections++;
  return 0;
}

static int increment_count_by_id(my_thread_id id,
                                 HASH *users_or_clients, THD *thd)
{
  THREAD_STATS* thread_stats;

  if (!(thread_stats = (THREAD_STATS *) my_hash_search(users_or_clients,
                                                       (uchar*) &id,
                                                       sizeof(my_thread_id))))
  {
    if (acl_is_utility_user(thd->security_ctx->user,
                            thd->security_ctx->get_host()->ptr(),
                            thd->security_ctx->get_ip()->ptr()))
      return 0;

    // First connection for this user or client
    if (!(thread_stats = ((THREAD_STATS *)
                        my_malloc(sizeof(THREAD_STATS), MYF(MY_WME | MY_ZEROFILL)))))
    {
      return 1; // Out of memory
    }

    init_thread_stats(thread_stats, id,
                    0, 0, 0,      // connections
                    0, 0, 0,   // time
                    0, 0, 0,   // bytes sent, received and written
                    0, 0, 0,   // rows fetched, updated and read
                    0, 0, 0,   // select, update and other commands
                    0, 0,      // commit and rollback trans
                    thd->diff_denied_connections,
                    0,         // lost connections
                    0,         // access denied errors
                    0);        // empty queries

    if (my_hash_insert(users_or_clients, (uchar *) thread_stats))
    {
      my_free((char *) thread_stats);
      return 1; // Out of memory
    }
  }
  thread_stats->total_connections++;
  if (thd->net.vio && thd->net.vio->type == VIO_TYPE_SSL)
    thread_stats->total_ssl_connections++;
  return 0;
}

/* Increments the global user and client stats connection count.  If 'use_lock'
   is true, LOCK_global_user_client_stats will be locked/unlocked.  Returns
   0 on success, 1 on error.
*/
static int increment_connection_count(THD* thd, bool use_lock)
{
  char* user_string=         get_valid_user_string(thd->main_security_ctx.user);
  const char* client_string= get_client_host(thd);
  int return_value=          0;

  if (acl_is_utility_user(thd->security_ctx->user,
                          thd->security_ctx->get_host()->ptr(),
                          thd->security_ctx->get_ip()->ptr()))
    return return_value;

  if (use_lock)
    mysql_mutex_lock(&LOCK_global_user_client_stats);

  if (increment_count_by_name(user_string, user_string,
                              &global_user_stats, thd))
  {
    return_value= 1;
    goto end;
  }
  if (increment_count_by_name(client_string,
                              user_string,
                              &global_client_stats, thd))
  {
    return_value= 1;
    goto end;
  }
  if (opt_thread_statistics)
  {
    if (increment_count_by_id(thd->thread_id, &global_thread_stats, thd))
    {
      return_value= 1;
      goto end;
    }
 }

end:
  if (use_lock)
    mysql_mutex_unlock(&LOCK_global_user_client_stats);
  return return_value;
}

// Used to update the global user and client stats.
static void update_global_user_stats_with_user(THD* thd,
                                               USER_STATS* user_stats,
                                               time_t now)
{
  user_stats->connected_time+=       now - thd->last_global_update_time;
//thd->last_global_update_time=      now;
  user_stats->busy_time+=            thd->diff_total_busy_time;
  user_stats->cpu_time+=             thd->diff_total_cpu_time;
  user_stats->bytes_received+=       thd->diff_total_bytes_received;
  user_stats->bytes_sent+=           thd->diff_total_bytes_sent;
  user_stats->binlog_bytes_written+= thd->diff_total_binlog_bytes_written;
  user_stats->rows_fetched+=         thd->diff_total_sent_rows;
  user_stats->rows_updated+=         thd->diff_total_updated_rows;
  user_stats->rows_read+=            thd->diff_total_read_rows;
  user_stats->select_commands+=      thd->diff_select_commands;
  user_stats->update_commands+=      thd->diff_update_commands;
  user_stats->other_commands+=       thd->diff_other_commands;
  user_stats->commit_trans+=         thd->diff_commit_trans;
  user_stats->rollback_trans+=       thd->diff_rollback_trans;
  user_stats->denied_connections+=   thd->diff_denied_connections;
  user_stats->lost_connections+=     thd->diff_lost_connections;
  user_stats->access_denied_errors+= thd->diff_access_denied_errors;
  user_stats->empty_queries+=        thd->diff_empty_queries;
}

static void update_global_thread_stats_with_thread(THD* thd,
                                               THREAD_STATS* thread_stats,
                                               time_t now)
{
  thread_stats->connected_time+=       now - thd->last_global_update_time;
//thd->last_global_update_time=        now;
  thread_stats->busy_time+=            thd->diff_total_busy_time;
  thread_stats->cpu_time+=             thd->diff_total_cpu_time;
  thread_stats->bytes_received+=       thd->diff_total_bytes_received;
  thread_stats->bytes_sent+=           thd->diff_total_bytes_sent;
  thread_stats->binlog_bytes_written+= thd->diff_total_binlog_bytes_written;
  thread_stats->rows_fetched+=         thd->diff_total_sent_rows;
  thread_stats->rows_updated+=         thd->diff_total_updated_rows;
  thread_stats->rows_read+=            thd->diff_total_read_rows;
  thread_stats->select_commands+=      thd->diff_select_commands;
  thread_stats->update_commands+=      thd->diff_update_commands;
  thread_stats->other_commands+=       thd->diff_other_commands;
  thread_stats->commit_trans+=         thd->diff_commit_trans;
  thread_stats->rollback_trans+=       thd->diff_rollback_trans;
  thread_stats->denied_connections+=   thd->diff_denied_connections;
  thread_stats->lost_connections+=     thd->diff_lost_connections;
  thread_stats->access_denied_errors+= thd->diff_access_denied_errors;
  thread_stats->empty_queries+=        thd->diff_empty_queries;
}

// Updates the global stats of a user or client
void update_global_user_stats(THD* thd, bool create_user, time_t now)
{
  char* user_string=         get_valid_user_string(thd->main_security_ctx.user);
  const char* client_string= get_client_host(thd);

  USER_STATS* user_stats;
  THREAD_STATS* thread_stats;

  if (acl_is_utility_user(thd->security_ctx->user,
                          thd->security_ctx->get_host()->ptr(),
                          thd->security_ctx->get_ip()->ptr()))
    return;

  mysql_mutex_lock(&LOCK_global_user_client_stats);

  // Update by user name
  if ((user_stats = (USER_STATS *) my_hash_search(&global_user_stats,
                                                  (uchar *) user_string,
                                                  strlen(user_string))))
  {
    // Found user.
    update_global_user_stats_with_user(thd, user_stats, now);
  }
  else
  {
    // Create the entry
    if (create_user)
    {
      increment_count_by_name(user_string, user_string,
                              &global_user_stats, thd);
    }
  }

  // Update by client IP
  if ((user_stats = (USER_STATS *) my_hash_search(&global_client_stats,
                                                  (uchar *) client_string,
                                                  strlen(client_string))))
  {
    // Found by client IP
    update_global_user_stats_with_user(thd, user_stats, now);
  }
  else
  {
    // Create the entry
    if (create_user)
    {
      increment_count_by_name(client_string,
                              user_string,
                              &global_client_stats, thd);
    }
  }

  if (opt_thread_statistics)
  {
    // Update by thread ID
    if ((thread_stats = (THREAD_STATS *) my_hash_search(&global_thread_stats,
                                                        (uchar *) &(thd->thread_id),
                                                        sizeof(my_thread_id))))
    {
      // Found by thread ID
      update_global_thread_stats_with_thread(thd, thread_stats, now);
    }
    else
    {
      // Create the entry
      if (create_user)
      {
        increment_count_by_id(thd->thread_id,
                              &global_thread_stats, thd);
      }
    }
  }

  thd->last_global_update_time = now;
  thd->reset_diff_stats();

  mysql_mutex_unlock(&LOCK_global_user_client_stats);
}

static void clear_stats_concurrent_connections(HASH* stats)
{
  for (ulong idx= 0; idx < stats->records; idx++)
  {
    USER_STATS* const user_stats=
      reinterpret_cast<USER_STATS*>(my_hash_element(stats, idx));
    user_stats->concurrent_connections= 0;
  }
}

static void inc_stats_concurrent_conn(HASH* stats,
				      const char* user_string, int cnt)
{
  USER_STATS* const user_stats=
    reinterpret_cast<USER_STATS*>(my_hash_search(stats,
          reinterpret_cast<const uchar*>(user_string),
          strlen(user_string)));
  if (user_stats)
    user_stats->concurrent_connections+= cnt;
}

/**
  Update number of concurrent connections for user_stats and client_stats
  based on account resource limits
*/
void refresh_concurrent_conn_stats()
{
  mysql_mutex_lock(&LOCK_user_conn);

  mysql_mutex_lock(&LOCK_global_user_client_stats);
  clear_stats_concurrent_connections(&global_user_stats);
  clear_stats_concurrent_connections(&global_client_stats);
  mysql_mutex_unlock(&LOCK_global_user_client_stats);

  for (ulong idx= 0; idx < hash_user_connections.records; idx++)
  {
    const struct user_conn* const uc=
      reinterpret_cast<struct user_conn*>(
          my_hash_element(&hash_user_connections, idx));
    mysql_mutex_lock(&LOCK_global_user_client_stats);
    inc_stats_concurrent_conn(&global_user_stats, uc->user, uc->connections);
    inc_stats_concurrent_conn(&global_client_stats, uc->host, uc->connections);
    mysql_mutex_unlock(&LOCK_global_user_client_stats);
  }

  mysql_mutex_unlock(&LOCK_user_conn);
}

/*
  check if user has already too many connections
  
  SYNOPSIS
  check_for_max_user_connections()
  thd			Thread handle
  uc			User connect object

  NOTES
    If check fails, we decrease user connection count, which means one
    shouldn't call decrease_user_connections() after this function.

  RETURN
    0	ok
    1	error
*/

int check_for_max_user_connections(THD *thd, const USER_CONN *uc)
{
  int error=0;
  DBUG_ENTER("check_for_max_user_connections");

  mysql_mutex_lock(&LOCK_user_conn);
  if (global_system_variables.max_user_connections &&
      !uc->user_resources.user_conn &&
      global_system_variables.max_user_connections < (uint) uc->connections)
  {
    my_error(ER_TOO_MANY_USER_CONNECTIONS, MYF(0), uc->user);
    error=1;
    goto end;
  }
  thd->time_out_user_resource_limits();
  if (uc->user_resources.user_conn &&
      uc->user_resources.user_conn < uc->connections)
  {
    my_error(ER_USER_LIMIT_REACHED, MYF(0), uc->user,
             "max_user_connections",
             (long) uc->user_resources.user_conn);
    error= 1;
    goto end;
  }
  if (uc->user_resources.conn_per_hour &&
      uc->user_resources.conn_per_hour <= uc->conn_per_hour)
  {
    my_error(ER_USER_LIMIT_REACHED, MYF(0), uc->user,
             "max_connections_per_hour",
             (long) uc->user_resources.conn_per_hour);
    error=1;
    goto end;
  }
  thd->increment_con_per_hour_counter();

end:
  if (error)
  {
    statistic_increment(denied_connections, &LOCK_status);
    thd->decrement_user_connections_counter();
    /*
      The thread may returned back to the pool and assigned to a user
      that doesn't have a limit. Ensure the user is not using resources
      of someone else.
    */
    thd->set_user_connect(NULL);
  }
  mysql_mutex_unlock(&LOCK_user_conn);
  DBUG_RETURN(error);
}


/*
  Decrease user connection count

  SYNOPSIS
    decrease_user_connections()
    uc			User connection object

  NOTES
    If there is a n user connection object for a connection
    (which only happens if 'max_user_connections' is defined or
    if someone has created a resource grant for a user), then
    the connection count is always incremented on connect.

    The user connect object is not freed if some users has
    'max connections per hour' defined as we need to be able to hold
    count over the lifetime of the connection.
*/

void decrease_user_connections(USER_CONN *uc)
{
  DBUG_ENTER("decrease_user_connections");
  mysql_mutex_lock(&LOCK_user_conn);
  DBUG_ASSERT(uc->connections);
  if (!--uc->connections && !mqh_used)
  {
    /* Last connection for user; Delete it */
    (void) my_hash_delete(&hash_user_connections,(uchar*) uc);
  }
  mysql_mutex_unlock(&LOCK_user_conn);
  DBUG_VOID_RETURN;
}

/*
   Decrements user connections count from the USER_CONN held by THD
   And removes USER_CONN from the hash if no body else is using it.

   SYNOPSIS
     release_user_connection()
     THD  Thread context object.
 */
void release_user_connection(THD *thd)
{
  const USER_CONN *uc= thd->get_user_connect();
  DBUG_ENTER("release_user_connection");

  if (uc)
  {
    mysql_mutex_lock(&LOCK_user_conn);
    DBUG_ASSERT(uc->connections > 0);
    thd->decrement_user_connections_counter();
    if (!uc->connections && !mqh_used)
    {
      /* Last connection for user; Delete it */
      (void) my_hash_delete(&hash_user_connections,(uchar*) uc);
    }
    mysql_mutex_unlock(&LOCK_user_conn);
    thd->set_user_connect(NULL);
  }

  DBUG_VOID_RETURN;
}


/*
  Check if maximum queries per hour limit has been reached
  returns 0 if OK.
*/

bool check_mqh(THD *thd, uint check_command)
{
  bool error= 0;
  const USER_CONN *uc=thd->get_user_connect();
  DBUG_ENTER("check_mqh");
  DBUG_ASSERT(uc != 0);

  mysql_mutex_lock(&LOCK_user_conn);

  thd->time_out_user_resource_limits();

  /* Check that we have not done too many questions / hour */
  if (uc->user_resources.questions)
  {
    thd->increment_questions_counter();
    if ((uc->questions - 1) >= uc->user_resources.questions)
    {
      my_error(ER_USER_LIMIT_REACHED, MYF(0), uc->user, "max_questions",
               (long) uc->user_resources.questions);
      error=1;
      goto end;
    }
  }
  if (check_command < (uint) SQLCOM_END)
  {
    /* Check that we have not done too many updates / hour */
    if (uc->user_resources.updates &&
        (sql_command_flags[check_command] & CF_CHANGES_DATA))
    {
      thd->increment_updates_counter();
      if ((uc->updates - 1) >= uc->user_resources.updates)
      {
        my_error(ER_USER_LIMIT_REACHED, MYF(0), uc->user, "max_updates",
                 (long) uc->user_resources.updates);
        error=1;
        goto end;
      }
    }
  }
end:
  mysql_mutex_unlock(&LOCK_user_conn);
  DBUG_RETURN(error);
}
#else

int check_for_max_user_connections(THD *thd, const USER_CONN *uc)
{
  return 0;
}

void decrease_user_connections(USER_CONN *uc)
{
  return;
}

void release_user_connection(THD *thd)
{
  const USER_CONN *uc= thd->get_user_connect();
  DBUG_ENTER("release_user_connection");

  if (uc)
  {
    thd->set_user_connect(NULL);
  }

  DBUG_VOID_RETURN;
}

void refresh_concurrent_conn_stats()
{
}

#endif /* NO_EMBEDDED_ACCESS_CHECKS */

/*
  Check for maximum allowable user connections, if the mysqld server is
  started with corresponding variable that is greater then 0.
*/

extern "C" uchar *get_key_conn(user_conn *buff, size_t *length,
			      my_bool not_used __attribute__((unused)))
{
  *length= buff->len;
  return (uchar*) buff->user;
}


extern "C" void free_user(struct user_conn *uc)
{
  my_free(uc);
}


void init_max_user_conn(void)
{
#ifndef NO_EMBEDDED_ACCESS_CHECKS
  (void)
    my_hash_init(&hash_user_connections,system_charset_info,max_connections,
                 0,0, (my_hash_get_key) get_key_conn,
                 (my_hash_free_key) free_user, 0);
#endif
}


void free_max_user_conn(void)
{
#ifndef NO_EMBEDDED_ACCESS_CHECKS
  my_hash_free(&hash_user_connections);
#endif /* NO_EMBEDDED_ACCESS_CHECKS */
}


void reset_mqh(LEX_USER *lu, bool get_them= 0)
{
#ifndef NO_EMBEDDED_ACCESS_CHECKS
  mysql_mutex_lock(&LOCK_user_conn);
  if (lu)  // for GRANT
  {
    USER_CONN *uc;
    uint temp_len=lu->user.length+lu->host.length+2;
    char temp_user[USER_HOST_BUFF_SIZE];

    memcpy(temp_user,lu->user.str,lu->user.length);
    memcpy(temp_user+lu->user.length+1,lu->host.str,lu->host.length);
    temp_user[lu->user.length]='\0'; temp_user[temp_len-1]=0;
    if ((uc = (struct  user_conn *) my_hash_search(&hash_user_connections,
                                                   (uchar*) temp_user,
                                                   temp_len)))
    {
      uc->questions=0;
      get_mqh(temp_user,&temp_user[lu->user.length+1],uc);
      uc->updates=0;
      uc->conn_per_hour=0;
    }
  }
  else
  {
    /* for FLUSH PRIVILEGES and FLUSH USER_RESOURCES */
    for (uint idx=0;idx < hash_user_connections.records; idx++)
    {
      USER_CONN *uc=(struct user_conn *)
        my_hash_element(&hash_user_connections, idx);
      if (get_them)
	get_mqh(uc->user,uc->host,uc);
      uc->questions=0;
      uc->updates=0;
      uc->conn_per_hour=0;
    }
  }
  mysql_mutex_unlock(&LOCK_user_conn);
#endif /* NO_EMBEDDED_ACCESS_CHECKS */
}


/**
  Set thread character set variables from the given ID

  @param  thd         thread handle
  @param  cs_number   character set and collation ID

  @retval  0  OK; character_set_client, collation_connection and
              character_set_results are set to the new value,
              or to the default global values.

  @retval  1  error, e.g. the given ID is not supported by parser.
              Corresponding SQL error is sent.
*/

bool thd_init_client_charset(THD *thd, uint cs_number)
{
  CHARSET_INFO *cs;
  /*
   Use server character set and collation if
   - opt_character_set_client_handshake is not set
   - client has not specified a character set
   - client character set is the same as the servers
   - client character set doesn't exists in server
  */
  if (!opt_character_set_client_handshake ||
      !(cs= get_charset(cs_number, MYF(0))) ||
      !my_strcasecmp(&my_charset_latin1,
                     global_system_variables.character_set_client->name,
                     cs->name))
  {
    if (!is_supported_parser_charset(
         global_system_variables.character_set_client))
    {
      /* Disallow non-supported parser character sets: UCS2, UTF16, UTF32 */
      my_error(ER_WRONG_VALUE_FOR_VAR, MYF(0), "character_set_client",
               global_system_variables.character_set_client->csname);
      return true;
    }
    thd->variables.character_set_client=
      global_system_variables.character_set_client;
    thd->variables.collation_connection=
      global_system_variables.collation_connection;
    thd->variables.character_set_results=
      global_system_variables.character_set_results;
  }
  else
  {
    if (!is_supported_parser_charset(cs))
    {
      /* Disallow non-supported parser character sets: UCS2, UTF16, UTF32 */
      my_error(ER_WRONG_VALUE_FOR_VAR, MYF(0), "character_set_client",
               cs->csname);
      return true;
    }    
    thd->variables.character_set_results=
      thd->variables.collation_connection= 
      thd->variables.character_set_client= cs;
  }
  return false;
}


/*
  Initialize connection threads
*/

bool init_new_connection_handler_thread()
{
  pthread_detach_this_thread();
  if (my_thread_init())
    return 1;
  return 0;
}

#ifndef EMBEDDED_LIBRARY
/*
  Perform handshake, authorize client and update thd ACL variables.

  SYNOPSIS
    check_connection()
    thd  thread handle

  RETURN
     0  success, thd is updated.
     1  error
*/

static int check_connection(THD *thd)
{
  uint connect_errors= 0;
  NET *net= &thd->net;

  DBUG_PRINT("info",
             ("New connection received on %s", vio_description(net->vio)));
#ifdef SIGNAL_WITH_VIO_CLOSE
  thd->set_active_vio(net->vio);
#endif

  if (!thd->main_security_ctx.get_host()->length())     // If TCP/IP connection
  {
    char ip[NI_MAXHOST];

    if (vio_peer_addr(net->vio, ip, &thd->peer_port, NI_MAXHOST))
    {
      my_error(ER_BAD_HOST_ERROR, MYF(0));
      return 1;
    }
    /* BEGIN : DEBUG */
    DBUG_EXECUTE_IF("addr_fake_ipv4",
                    {
                      struct sockaddr *sa= (sockaddr *) &net->vio->remote;
                      sa->sa_family= AF_INET;
                      struct in_addr *ip4= &((struct sockaddr_in *)sa)->sin_addr;
                      /* See RFC 5737, 192.0.2.0/23 is reserved */
                      const char* fake= "192.0.2.4";
                      ip4->s_addr= inet_addr(fake);
                      strcpy(ip, fake);
                    };);
    /* END   : DEBUG */

    thd->main_security_ctx.set_ip(my_strdup(ip, MYF(MY_WME)));
    if (!(thd->main_security_ctx.get_ip()->length()))
      return 1; /* The error is set by my_strdup(). */
    thd->main_security_ctx.host_or_ip= thd->main_security_ctx.get_ip()->ptr();
    if (!(specialflag & SPECIAL_NO_RESOLVE))
    {
      char *host= (char *) thd->main_security_ctx.get_host()->ptr();
      if (ip_to_hostname(&net->vio->remote,
                         thd->main_security_ctx.get_ip()->ptr(),
                         &host, &connect_errors))
      {
        my_error(ER_BAD_HOST_ERROR, MYF(0));
        return 1;
      }
      thd->main_security_ctx.set_host(host);
      /* Cut very long hostnames to avoid possible overflows */
      if (thd->main_security_ctx.get_host()->length())
      {
        if (thd->main_security_ctx.get_host()->ptr() != my_localhost)
          thd->main_security_ctx.set_host(thd->main_security_ctx.get_host()->ptr(),
                               min(thd->main_security_ctx.get_host()->length(),
                               HOSTNAME_LENGTH));
        thd->main_security_ctx.host_or_ip=
                        thd->main_security_ctx.get_host()->ptr();
      }
      if (connect_errors > max_connect_errors)
      {
        my_error(ER_HOST_IS_BLOCKED, MYF(0), thd->main_security_ctx.host_or_ip);
        return 1;
      }
    }
    DBUG_PRINT("info",("Host: %s  ip: %s",
		       (thd->main_security_ctx.get_host()->length() ?
                        thd->main_security_ctx.get_host()->ptr() : 
                        "unknown host"),
		       (thd->main_security_ctx.get_ip()->length() ?
                        thd->main_security_ctx.get_ip()->ptr()
                        : "unknown ip")));
    if (acl_check_host(thd->main_security_ctx.get_host()->ptr(),
                       thd->main_security_ctx.get_ip()->ptr()))
    {
      my_error(ER_HOST_NOT_PRIVILEGED, MYF(0),
               thd->main_security_ctx.host_or_ip);
      return 1;
    }
  }
  else /* Hostname given means that the connection was on a socket */
  {
    DBUG_PRINT("info",("Host: %s", thd->main_security_ctx.get_host()->ptr()));
    thd->main_security_ctx.host_or_ip= thd->main_security_ctx.get_host()->ptr();
    thd->main_security_ctx.set_ip("");
    /* Reset sin_addr */
    bzero((char*) &net->vio->remote, sizeof(net->vio->remote));
  }
  vio_keepalive(net->vio, TRUE);
  
  if (thd->packet.alloc(thd->variables.net_buffer_length))
    return 1; /* The error is set by alloc(). */

  return acl_authenticate(thd, connect_errors, 0);
}


/*
  Setup thread to be used with the current thread

  SYNOPSIS
    bool setup_connection_thread_globals()
    thd    Thread/connection handler

  RETURN
    0   ok
    1   Error (out of memory)
        In this case we will close the connection and increment status
*/

bool setup_connection_thread_globals(THD *thd)
{
  if (thd->store_globals())
  {
    close_connection(thd, ER_OUT_OF_RESOURCES);
    statistic_increment(aborted_connects,&LOCK_status);
    MYSQL_CALLBACK(thd->scheduler, end_thread, (thd, 0));
    return 1;                                   // Error
  }
  return 0;
}


/*
  Autenticate user, with error reporting

  SYNOPSIS
   login_connection()
   thd        Thread handler

  NOTES
    Connection is not closed in case of errors

  RETURN
    0    ok
    1    error
*/


bool login_connection(THD *thd)
{
  NET *net= &thd->net;
  int error;
  DBUG_ENTER("login_connection");
  DBUG_PRINT("info", ("login_connection called by thread %lu",
                      thd->thread_id));

  /* Use "connect_timeout" value during connection phase */
  my_net_set_read_timeout(net, connect_timeout);
  my_net_set_write_timeout(net, connect_timeout);

  error= check_connection(thd);
  thd->protocol->end_statement();

  if (error)
  {						// Wrong permissions
#ifdef _WIN32
    if (vio_type(net->vio) == VIO_TYPE_NAMEDPIPE)
      my_sleep(1000);				/* must wait after eof() */
#endif
    statistic_increment(aborted_connects,&LOCK_status);
    thd->diff_denied_connections++;
    DBUG_RETURN(1);
  }
  /* Connect completed, set read/write timeouts back to default */
  my_net_set_read_timeout(net, thd->variables.net_read_timeout);
  my_net_set_write_timeout(net, thd->variables.net_write_timeout);

  thd->reset_stats();

  // Updates global user connection stats.
  if (opt_userstat && increment_connection_count(thd, true))
    DBUG_RETURN(1);

  DBUG_RETURN(0);
}


/*
  Close an established connection

  NOTES
    This mainly updates status variables
*/

void end_connection(THD *thd)
{
  NET *net= &thd->net;
  plugin_thdvar_cleanup(thd);

  /*
    The thread may returned back to the pool and assigned to a user
    that doesn't have a limit. Ensure the user is not using resources
    of someone else.
  */
  release_user_connection(thd);

  if (thd->killed || (net->error && net->vio != 0))
  {
    statistic_increment(aborted_threads,&LOCK_status);
    thd->diff_lost_connections++;
  }

  if (net->error && net->vio != 0)
  {
    if (!thd->killed && thd->variables.log_warnings > 1)
    {
      Security_context *sctx= thd->security_ctx;

      sql_print_warning(ER(ER_NEW_ABORTING_CONNECTION),
                        thd->thread_id,(thd->db ? thd->db : "unconnected"),
                        sctx->user ? sctx->user : "unauthenticated",
                        sctx->host_or_ip,
                        (thd->stmt_da->is_error() ? thd->stmt_da->message() :
                         ER(ER_UNKNOWN_ERROR)));
    }
  }
}


/*
  Initialize THD to handle queries
*/

void prepare_new_connection_state(THD* thd)
{
  Security_context *sctx= thd->security_ctx;

  if (thd->client_capabilities & CLIENT_COMPRESS)
    thd->net.compress=1;				// Use compression

  /*
    Much of this is duplicated in create_embedded_thd() for the
    embedded server library.
    TODO: refactor this to avoid code duplication there
  */
  thd->proc_info= 0;
  thd->command= COM_SLEEP;
  thd->set_time();
  thd->init_for_queries();

  if (opt_init_connect.length && !(sctx->master_access & SUPER_ACL))
  {
    execute_init_command(thd, &opt_init_connect, &LOCK_sys_init_connect);
    if (thd->is_error())
    {
      ulong packet_length;
      NET *net= &thd->net;

      sql_print_warning(ER(ER_NEW_ABORTING_CONNECTION),
                        thd->thread_id,
                        thd->db ? thd->db : "unconnected",
                        sctx->user ? sctx->user : "unauthenticated",
                        sctx->host_or_ip, "init_connect command failed");
      sql_print_warning("%s", thd->stmt_da->message());

      thd->lex->current_select= 0;
      my_net_set_read_timeout(net, thd->variables.net_wait_timeout);
      thd->clear_error();
      net_new_transaction(net);
      packet_length= my_net_read(net);
      /*
        If my_net_read() failed, my_error() has been already called,
        and the main Diagnostics Area contains an error condition.
      */
      if (packet_length != packet_error)
        my_error(ER_NEW_ABORTING_CONNECTION, MYF(0),
                 thd->thread_id,
                 thd->db ? thd->db : "unconnected",
                 sctx->user ? sctx->user : "unauthenticated",
                 sctx->host_or_ip, "init_connect command failed");

      thd->server_status&= ~SERVER_STATUS_CLEAR_SET;
      thd->protocol->end_statement();
      thd->killed = THD::KILL_CONNECTION;
      return;
    }

    thd->proc_info=0;
    thd->set_time();
    thd->init_for_queries();
  }
}


/*
  Thread handler for a connection

  SYNOPSIS
    handle_one_connection()
    arg		Connection object (THD)

  IMPLEMENTATION
    This function (normally) does the following:
    - Initialize thread
    - Initialize THD to be used with this thread
    - Authenticate user
    - Execute all queries sent on the connection
    - Take connection down
    - End thread  / Handle next connection using thread from thread cache
*/

pthread_handler_t handle_one_connection(void *arg)
{
  THD *thd= (THD*) arg;

  mysql_thread_set_psi_id(thd->thread_id);

  do_handle_one_connection(thd);
  return 0;
}

bool thd_prepare_connection(THD *thd)
{
  bool rc;
  lex_start(thd);
  rc= login_connection(thd);
  MYSQL_AUDIT_NOTIFY_CONNECTION_CONNECT(thd);
  if (rc)
    return rc;

  MYSQL_CONNECTION_START(thd->thread_id, &thd->security_ctx->priv_user[0],
                         (char *) thd->security_ctx->host_or_ip);

  prepare_new_connection_state(thd);
  return FALSE;
}

bool thd_is_connection_alive(THD *thd)
{
  NET *net= &thd->net;
  if (!net->error &&
      net->vio != 0 &&
      !(thd->killed == THD::KILL_CONNECTION))
    return TRUE;
  return FALSE;
}

void do_handle_one_connection(THD *thd_arg)
{
  THD *thd= thd_arg;

  thd->thr_create_utime= my_micro_time();

  if (MYSQL_CALLBACK_ELSE(thd->scheduler, init_new_connection_thread, (), 0))
  {
    close_connection(thd, ER_OUT_OF_RESOURCES);
    statistic_increment(aborted_connects,&LOCK_status);
    MYSQL_CALLBACK(thd->scheduler, end_thread, (thd, 0));
    return;
  }

  /*
    If a thread was created to handle this connection:
    increment slow_launch_threads counter if it took more than
    slow_launch_time seconds to create the thread.
  */
  if (thd->prior_thr_create_utime)
  {
    ulong launch_time= (ulong) (thd->thr_create_utime -
                                thd->prior_thr_create_utime);
    if (launch_time >= slow_launch_time*1000000L)
      statistic_increment(slow_launch_threads, &LOCK_status);
    thd->prior_thr_create_utime= 0;
  }

  /*
    handle_one_connection() is normally the only way a thread would
    start and would always be on the very high end of the stack ,
    therefore, the thread stack always starts at the address of the
    first local variable of handle_one_connection, which is thd. We
    need to know the start of the stack so that we could check for
    stack overruns.
  */
  thd->thread_stack= (char*) &thd;
  if (setup_connection_thread_globals(thd))
    return;

  DBUG_EXECUTE_IF("after_thread_setup",
                  {
                  const char act[]=
                  "now signal thread_setup";
                  DBUG_ASSERT(!debug_sync_set_action(current_thd,
                                                     STRING_WITH_LEN(act)));
                  };);

  for (;;)
  {
    bool rc;
    bool create_user= TRUE;

    rc= thd_prepare_connection(thd);
    if (rc)
    {
      create_user= FALSE;
      goto end_thread;
    }

    while (thd_is_connection_alive(thd))
    {
      mysql_audit_release(thd);
      if (do_command(thd))
	break;
    }
    end_connection(thd);
   
end_thread:
    close_connection(thd);

    if (unlikely(opt_userstat))
    {
      thd->update_stats(false);
      update_global_user_stats(thd, create_user, time(NULL));
    }

    if (MYSQL_CALLBACK_ELSE(thd->scheduler, end_thread, (thd, 1), 0))
      return;                                 // Probably no-threads

    /*
      If end_thread() returns, we are either running with
      thread-handler=no-threads or this thread has been schedule to
      handle the next connection.
    */
    thd= current_thd;
    thd->thread_stack= (char*) &thd;
  }
}
#endif /* EMBEDDED_LIBRARY */
