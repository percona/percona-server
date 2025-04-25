/* Copyright (c) 2024, Oracle and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is designed to work with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have either included with
   the program or referenced in the documentation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "sql/rpl_opt_tracker.h"

#include <my_dbug.h>
#include <mysql/components/services/log_builtins.h>
#include <mysql/components/util/weak_service_reference.h>
#include "sql/mysqld.h"
#include "sql/replication.h"
#include "sql/rpl_group_replication.h"
#include "sql/rpl_msr.h"

const std::string Rpl_opt_tracker::s_c_name_mysql_server{"mysql_server"};
const std::string Rpl_opt_tracker::s_f_name_binary_log{"Binary Log"};
const std::string Rpl_opt_tracker::s_f_name_replication_replica{
    "Replication Replica"};
const std::string Rpl_opt_tracker::s_f_name_group_replication{
    "Group Replication"};

static const std::string s_name("mysql_option_tracker_option");
static const std::string c_name_mysql_server_replication(
    "mysql_server_replication");
typedef weak_service_reference<SERVICE_TYPE(mysql_option_tracker_option),
                               c_name_mysql_server_replication, s_name>
    srv_weak_option_option;

static void *launch_thread(void *arg) {
  Rpl_opt_tracker *handler = (Rpl_opt_tracker *)arg;
  handler->worker();
  return nullptr;
}

Rpl_opt_tracker::Rpl_opt_tracker(SERVICE_TYPE_NO_CONST(registry_registration) *
                                     srv_registry_registration,
                                 SERVICE_TYPE_NO_CONST(registry_registration) *
                                     srv_registry_registration_no_lock)
    : m_srv_registry_registration_no_lock(srv_registry_registration_no_lock),
      m_option_usage_binary_log(s_f_name_binary_log.c_str(), srv_registry),
      m_option_usage_replication_replica(s_f_name_replication_replica.c_str(),
                                         srv_registry),
      m_option_usage_group_replication(s_f_name_group_replication.c_str(),
                                       srv_registry) {
  srv_weak_option_option::init(
      srv_registry, srv_registry_registration,
      [&](SERVICE_TYPE(mysql_option_tracker_option) * opt) {
        opt->define(s_f_name_binary_log.c_str(), s_c_name_mysql_server.c_str(),
                    opt_bin_log ? 1 : 0);
        opt->define(s_f_name_replication_replica.c_str(),
                    s_c_name_mysql_server.c_str(),
                    is_replication_replica_enabled() ? 1 : 0);
        return false;
      },
      false);
}

Rpl_opt_tracker::~Rpl_opt_tracker() {
  srv_weak_option_option::deinit(
      srv_registry_no_lock, m_srv_registry_registration_no_lock,
      [&](SERVICE_TYPE(mysql_option_tracker_option) * opt) {
        opt->undefine(s_f_name_binary_log.c_str());
        opt->undefine(s_f_name_replication_replica.c_str());
        return false;
      });
}

bool Rpl_opt_tracker::acquire_option_tracker_service() {
  if (srv_registry->acquire("mysql_option_tracker_option",
                            &m_option_tracker_handle)) {
    return true;
  }
  m_option_tracker_service =
      reinterpret_cast<SERVICE_TYPE(mysql_option_tracker_option) *>(
          m_option_tracker_handle);
  return false;
}

void Rpl_opt_tracker::release_option_tracker_service() {
  m_option_tracker_service = nullptr;
  if (nullptr != m_option_tracker_handle) {
    srv_registry->release(m_option_tracker_handle);
    m_option_tracker_handle = nullptr;
  }
}

bool Rpl_opt_tracker::is_replication_replica_enabled() {
  bool replication_replica_enabled = false;

  if (server_id != 0) {
    channel_map.rdlock();
    if (is_slave_configured()) {
      replication_replica_enabled =
          (channel_map.get_number_of_configured_channels() > 0);
    }
    channel_map.unlock();
  }

  return replication_replica_enabled;
}

void Rpl_opt_tracker::track_binary_log_internal(bool enabled) {
  m_option_tracker_service->set_enabled(s_f_name_binary_log.c_str(),
                                        enabled ? 1 : 0);

  if (enabled) {
    m_option_usage_binary_log.set(true);
  }
}

void Rpl_opt_tracker::track_replication_replica(bool enabled) {
  if (acquire_option_tracker_service()) {
    return;
  }

  track_replication_replica_internal(enabled);
  release_option_tracker_service();
}

void Rpl_opt_tracker::track_replication_replica_internal(bool enabled) {
  m_option_tracker_service->set_enabled(s_f_name_replication_replica.c_str(),
                                        enabled ? 1 : 0);

  if (enabled) {
    m_option_usage_replication_replica.set(true);
  }
}

void Rpl_opt_tracker::track_group_replication_usage_internal(bool enabled) {
  if (enabled) {
    m_option_usage_group_replication.set(true);
  }
}

void Rpl_opt_tracker::worker() {
  THD *thd = new THD;
  my_thread_init();
  thd->set_new_thread_id();
  thd->thread_stack = reinterpret_cast<char *>(&thd);
  thd->set_command(COM_DAEMON);
  thd->security_context()->skip_grants();
  thd->system_thread = SYSTEM_THREAD_BACKGROUND;
  thd->store_globals();
  thd->set_time();

  for (;;) {
    /*
      Only track features if the option tracker service is
      installed.
    */
    if (!acquire_option_tracker_service()) {
      /*
        Binary Log
      */
      rpl_opt_tracker->track_binary_log_internal(opt_bin_log);

      /*
        Replication Replica
      */
      rpl_opt_tracker->track_replication_replica_internal(
          is_replication_replica_enabled());

      /*
        Group Replication
      */
      rpl_opt_tracker->track_group_replication_usage_internal(
          is_group_replication_running());

      release_option_tracker_service();
    }

    mysql_mutex_lock(&LOCK_rpl_opt_tracker);
    if (m_stop_worker || thd->killed) {
      break;
    }

    THD_ENTER_COND(thd, &COND_rpl_opt_tracker, &LOCK_rpl_opt_tracker,
                   &stage_suspending, nullptr);
    struct timespec nowtime;
    set_timespec(&nowtime, 0);
    struct timespec abstime;
    set_timespec(&abstime, s_tracking_period);
    DBUG_EXECUTE_IF("rpl_opt_tracker_small_tracking_period",
                    set_timespec(&abstime, 1););

    while (cmp_timespec(&nowtime, &abstime) <= 0) {
      mysql_cond_timedwait(&COND_rpl_opt_tracker, &LOCK_rpl_opt_tracker,
                           &abstime);
      if (m_stop_worker || thd->killed) {
        break;
      }
      set_timespec(&nowtime, 0);
    }

    if (m_stop_worker || thd->killed) {
      break;
    }
    mysql_mutex_unlock(&LOCK_rpl_opt_tracker);
    THD_EXIT_COND(thd, nullptr);
  }

  mysql_mutex_unlock(&LOCK_rpl_opt_tracker);
  THD_EXIT_COND(thd, nullptr);

  thd->release_resources();
  thd->restore_globals();
  delete thd;
  my_thread_end();
  my_thread_exit(nullptr);
}

void Rpl_opt_tracker::start_worker() {
  my_thread_attr_t attr;
  if (my_thread_attr_init(&attr)) {
    LogErr(WARNING_LEVEL, ER_FAILED_TO_CREATE_RPL_OPT_TRACKER_THREAD);
    return;
  }

  if (
#ifndef _WIN32
      pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM) ||
#endif
      mysql_thread_create(key_thread_rpl_opt_tracker, &m_thread_id, &attr,
                          launch_thread, (void *)this)) {
    LogErr(WARNING_LEVEL, ER_FAILED_TO_CREATE_RPL_OPT_TRACKER_THREAD);
  }

  (void)my_thread_attr_destroy(&attr);
}

void Rpl_opt_tracker::stop_worker() {
  mysql_mutex_lock(&LOCK_rpl_opt_tracker);
  m_stop_worker = true;
  mysql_cond_signal(&COND_rpl_opt_tracker);
  mysql_mutex_unlock(&LOCK_rpl_opt_tracker);

  if (m_thread_id.thread != null_thread_initializer) {
    my_thread_join(&m_thread_id, nullptr);
    m_thread_id.thread = null_thread_initializer;
  }
}
