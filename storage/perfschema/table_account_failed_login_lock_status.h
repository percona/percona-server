/* Copyright (c) 2025 Percona LLC and/or its affiliates. All rights
   reserved.

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

#ifndef TABLE_ACCOUNT_FAILED_LOGIN_LOCK_STATUS_H
#define TABLE_ACCOUNT_FAILED_LOGIN_LOCK_STATUS_H

/**
  @file storage/perfschema/table_account_failed_login_lock_status.h
  Table ACCOUNT_FAILED_LOGIN_LOCK_STATUS (declarations).
*/

#include "storage/perfschema/pfs_engine_table.h"
#include "storage/perfschema/pfs_name.h"
#include "storage/perfschema/table_helper.h"

/**
  A row of PERFORMANCE_SCHEMA.ACCOUNT_FAILED_LOGIN_LOCK_STATUS.
*/
struct row_account_failed_login_lock_status {
  /** Column USER. */
  PFS_user_name m_user_name;
  /** Column HOST. */
  PFS_host_name m_host_name;
  /** Column IS_TRACKING_ACTIVE. */
  bool m_is_tracking_active;
  /** Column MAX_ATTEMPTS. */
  uint m_max_attempts;
  /** Column PASSWORD_LOCK_DAYS. */
  int m_password_lock_days;
  /** Column IS_LOCKED. */
  bool m_is_locked;
  /** Column REMAINING_ATTEMPTS. */
  uint m_remaining_attempts;
  /** Column REMAINING_DAYS_LOCKED. */
  int m_remaining_days_locked;
};

/**
  Helper fo index over (USER, HOST) pair of fields.
*/
class PFS_index_account_failed_login_lock_status_by_user_host
    : public PFS_engine_index {
 public:
  PFS_index_account_failed_login_lock_status_by_user_host()
      : PFS_engine_index(&m_key_1, &m_key_2),
        m_key_1("USER"),
        m_key_2("HOST") {}

  ~PFS_index_account_failed_login_lock_status_by_user_host() override = default;

  bool match(row_account_failed_login_lock_status *row);

 private:
  PFS_key_user m_key_1;
  PFS_key_host m_key_2;
};

/**
  Table PERFORMANCE_SCHEMA.ACCOUNT_FAILED_LOGIN_LOCK_STATUS.
*/
class table_account_failed_login_lock_status : public PFS_engine_table {
 public:
  /** Table share. */
  static PFS_engine_table_share m_share;

  /** Implementation of PFS_engine_table factory/open_table callback.*/
  static PFS_engine_table *create(PFS_engine_table_share *);

  /** Implementation of row count callback. */
  static ha_rows get_row_count();

  /*
    Implementations of PFS_engine_table's abstract methods for scanning
    through table rows, reading row at position and et cetera.
  */
  int rnd_next() override;

  int rnd_pos(const void *pos) override;

  void reset_position() override;

  int index_init(uint idx, bool sorted) override;

  int index_next() override;

  ~table_account_failed_login_lock_status() override = default;

 protected:
  table_account_failed_login_lock_status();

  int read_row_values(TABLE *table, unsigned char *buf, Field **fields,
                      bool read_all) override;

 private:
  /**
    Prepare P_S.ACCOUNT_FAILED_LOGIN_LOCK_STATUS row from ACL_USER entry taking
    into account today's day number (since year 0).
  */
  static void make_row(long now_day, const ACL_USER *entry,
                       row_account_failed_login_lock_status *row);

  /** Build contents P_S.ACCOUNT_FAILED_LOGIN_LOCK_STATUS from ACL cache. */
  void materialize(THD *thd);

 private:
  /** Table THR_LOCK lock. */
  static THR_LOCK m_table_lock;
  /** Table definition. */
  static Plugin_table m_table_def;

  row_account_failed_login_lock_status *m_all_rows;
  uint m_row_count;

  /** Current row. */
  row_account_failed_login_lock_status *m_row;
  /** Current position. */
  PFS_simple_index m_pos;
  /** Next position. */
  PFS_simple_index m_next_pos;

  /** Current index. */
  PFS_index_account_failed_login_lock_status_by_user_host *m_opened_index;
};

/** @} */

#endif
