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

#ifndef RPL_OPT_TRACKER_H_
#define RPL_OPT_TRACKER_H_

#include <my_systime.h>
#include <mysql/components/library_mysys/option_usage_data.h>
#include <mysql/components/minimal_chassis.h>
#include <mysql/components/services/bits/psi_thread_bits.h>
#include <mysql/components/services/mysql_option_tracker.h>
#include <string>

/**
  Class to track the state and usage data of Replication features.
*/
class Rpl_opt_tracker {
 public:
  Rpl_opt_tracker(SERVICE_TYPE_NO_CONST(registry_registration) *
                      srv_registry_registration,
                  SERVICE_TYPE_NO_CONST(registry_registration) *
                      srv_registry_registration_no_lock);
  virtual ~Rpl_opt_tracker();
  /* Prevent user from invoking default assignment function. */
  Rpl_opt_tracker &operator=(const Rpl_opt_tracker &info);
  /* Prevent user from invoking default constructor function. */
  Rpl_opt_tracker(const Rpl_opt_tracker &info);

  /**
    The thread worker that periodically tracks the replication
    features.
  */
  void worker();

  /**
    Start the thread that periodically tracks the replication
    features.
  */
  void start_worker();

  /**
    Stop the thread that periodically tracks the replication
    features.
  */
  void stop_worker();

  /**
    Tracks the Replication Replica feature, including the usage data.
    It only updates usage data if the feature is enabled.

    @param enabled  true:  tracks as enabled
                    false: tracks as disabled
  */
  void track_replication_replica(bool enabled);

 private:
  /**
    Tracks the Binary Log feature, including the usage data.
    It only updates usage data if the feature is enabled.
    Internal method to be used after the mysql_option_tracker_option
    service is acquired.

    @param enabled  true:  tracks as enabled
                    false: tracks as disabled
  */
  void track_binary_log_internal(bool enabled);

  /**
    Helper method to get Replication Replica feature status.

    @return Replication Replica feature status
      @retval true   enabled
      @retval false  disabled
  */
  static bool is_replication_replica_enabled();

  /**
    Tracks the Replication Replica feature, including the usage data.
    It only updates usage data if the feature is enabled.
    Internal method to be used after the mysql_option_tracker_option
    service is acquired.

    @param enabled  true:  tracks as enabled
                    false: tracks as disabled
  */
  void track_replication_replica_internal(bool enabled);

  /**
    Tracks the Group Replication feature usage data.
    It only updates usage data if the feature is enabled.

    @param enabled  true:  tracks as enabled
                    false: tracks as disabled
  */
  void track_group_replication_usage_internal(bool enabled);

  /**
    Helper method to acquire the mysql_option_tracker_option
    service.

    @return the operation status
      @retval false  Successful
      @retval true   Error
  */
  bool acquire_option_tracker_service();

  /**
    Helper method to release the mysql_option_tracker_option
    service.
  */
  void release_option_tracker_service();

  SERVICE_TYPE_NO_CONST(registry_registration) *
      m_srv_registry_registration_no_lock{nullptr};
  SERVICE_TYPE(mysql_option_tracker_option) * m_option_tracker_service{nullptr};
  my_h_service m_option_tracker_handle{nullptr};

  my_thread_handle m_thread_id;
  bool m_stop_worker{false};
  static constexpr const Timeout_type s_tracking_period{3600};  // 1 hour

  static const std::string s_c_name_mysql_server;
  static const std::string s_f_name_binary_log;
  static const std::string s_f_name_replication_replica;
  static const std::string s_f_name_group_replication;

  Option_usage_data m_option_usage_binary_log;
  Option_usage_data m_option_usage_replication_replica;
  Option_usage_data m_option_usage_group_replication;
};

/**
  The rpl_opt_tracker singleton.
*/
extern Rpl_opt_tracker *rpl_opt_tracker;

#endif /* RPL_OPT_TRACKER_H_ */
