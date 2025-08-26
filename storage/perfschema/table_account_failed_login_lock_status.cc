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

/**
  @file storage/perfschema/table_account_failed_login_lock_status.cc
  Table ACCOUNT_FAILED_LOGIN_LOCK_STATUS (implementation).
*/

#include "storage/perfschema/table_account_failed_login_lock_status.h"

#include "sql/auth/sql_auth_cache.h"
#include "sql/field.h"
#include "sql/mysqld.h"
#include "sql/plugin_table.h"
#include "sql/tztime.h"
#include "thr_lock.h"

THR_LOCK table_account_failed_login_lock_status::m_table_lock;

Plugin_table table_account_failed_login_lock_status::m_table_def(
    /* Schema name */
    "performance_schema",
    /* Name */
    "account_failed_login_lock_status",
    /* Definition */
    /*
      Note that USER and HOST made nullable to be able to reuse PFS_key_user
      and PFS_key_host helpers. Normally they can't be NULL in ACL subsystem.
    */
    "  USER CHAR(32) COLLATE utf8mb4_bin DEFAULT NULL,\n"
    "  HOST CHAR(255) CHARACTER SET ascii DEFAULT NULL,\n"
    "  IS_TRACKING_ACTIVE ENUM ('YES', 'NO') NOT NULL,\n"
    "  MAX_ATTEMPTS INT UNSIGNED NOT NULL,\n"
    "  PASSWORD_LOCK_DAYS INT NOT NULL,\n"
    "  IS_LOCKED ENUM ('YES', 'NO') DEFAULT NULL,\n"
    "  REMAINING_ATTEMPTS INT UNSIGNED DEFAULT NULL,\n"
    "  REMAINING_DAYS_LOCKED INT DEFAULT NULL,\n"
    "  UNIQUE KEY `ACCOUNT` (USER, HOST) USING HASH\n",
    /* Options */
    " ENGINE=PERFORMANCE_SCHEMA",
    /* Tablespace */
    nullptr);

PFS_engine_table_share table_account_failed_login_lock_status::m_share = {
    &pfs_readonly_acl,
    table_account_failed_login_lock_status::create, /* open_table */
    nullptr,                                        /* write_row */
    nullptr,                                        /* delete_all_rows */
    table_account_failed_login_lock_status::get_row_count,
    sizeof(PFS_simple_index), /* ref length */
    &m_table_lock,
    &m_table_def,
    true /* perpetual */,
    PFS_engine_table_proxy(),
    {0},  /* m_ref_count */
    false /* m_in_purgatory */
};

bool PFS_index_account_failed_login_lock_status_by_user_host::match(
    row_account_failed_login_lock_status *row) {
  /*
    Unlike HASH indexes in MEMORY and NDB engines P_S HASH indexes
    do not require values of the whole key for lookup and support
    lookup by prefix columns of the key (@sa HA_ONLY_WHOLE_INDEX).
    E.g. in cause of our index lookup by only USER column value is
    supported in addition to lookup by both USER and HOST values.
    Hence we support such matches in the below code.
  */
  if (m_fields >= 1) {
    if (!m_key_1.match(&row->m_user_name)) {
      return false;
    }
  }

  if (m_fields >= 2) {
    if (!m_key_2.match(&row->m_host_name)) {
      return false;
    }
  }

  return true;
}

table_account_failed_login_lock_status::table_account_failed_login_lock_status()
    : PFS_engine_table(&m_share, &m_pos),
      m_all_rows(nullptr),
      m_row_count(0),
      m_row(nullptr),
      m_pos(0),
      m_next_pos(0) {}

PFS_engine_table *table_account_failed_login_lock_status::create(
    PFS_engine_table_share *) {
  auto *t = new table_account_failed_login_lock_status();
  if (t != nullptr) {
    THD *thd = current_thd;
    assert(thd != nullptr);
    t->materialize(thd);
  }
  return t;
}

ha_rows table_account_failed_login_lock_status::get_row_count() {
  THD *thd = current_thd;
  assert(thd != nullptr);

  Acl_cache_lock_guard acl_cache_lock{thd, Acl_cache_lock_mode::READ_MODE};
  // Play safe and pretend that table is empty if we fail to acquire ACL lock.
  if (!acl_cache_lock.lock()) return 0;

  // We return empty table in --skip-grant-tables mode.
  return acl_users ? acl_users->size() : 0;
}

void table_account_failed_login_lock_status::make_row(
    long now_day, const ACL_USER *entry,
    row_account_failed_login_lock_status *row) {
  row->m_user_name.set(entry->user, entry->get_username_length());
  row->m_host_name.set(entry->host.get_host(), entry->host.get_host_len());
  row->m_is_tracking_active = entry->password_locked_state.is_active();
  row->m_max_attempts =
      entry->password_locked_state.get_failed_login_attempts();
  row->m_password_lock_days =
      entry->password_locked_state.get_password_lock_time_days();

  // The below code is aligned with ACL_USER::Password_lock_state::update().

  if (row->m_is_tracking_active) {
    long daynr_locked = entry->password_locked_state.get_daynr_locked();

    if (daynr_locked == 0) {
      // Account is marked as unlocked in ACL cache.
      row->m_is_locked = false;
      row->m_remaining_attempts =
          entry->password_locked_state.get_remaining_login_attempts();
      row->m_remaining_days_locked = 0;
    } else {
      long days_remaining =
          entry->password_locked_state.get_remaining_days_locked(now_day);

      if (days_remaining == 0) {
        // Account marked as locked in ACL cache, but will be unlocked
        // automatically if connect happens around now.
        row->m_is_locked = false;
        row->m_remaining_attempts = row->m_max_attempts;
        row->m_remaining_days_locked = 0;
      } else {
        // Account marked as locked in ACL cache. It won't be unlocked if
        // connect happens around now.
        // It might be even locked forever (in this case days_remaining==-1).
        row->m_is_locked = true;
        row->m_remaining_attempts = 0;
        row->m_remaining_days_locked = days_remaining;
      }
    }
  } else {
    // Safety. Values of this fields are set to NULL by code
    // in read_row_values() if tracking is disabled.
    row->m_is_locked = false;
    row->m_remaining_attempts = 0;
    row->m_remaining_days_locked = 0;
  }
}

void table_account_failed_login_lock_status::materialize(THD *thd) {
  assert(m_all_rows == nullptr);
  assert(m_row_count == 0);

  // Get today's number since year 0. Use @@global.time_zone time zone for this.
  // This is the time zone which is set for new connections, and which will
  // be used by check whether it is time to unlock account, done at connect
  // time by ACL_USER::Password_lock_state::update().
  mysql_mutex_lock(&LOCK_global_system_variables);
  Time_zone *tz = global_system_variables.time_zone;
  mysql_mutex_unlock(&LOCK_global_system_variables);

  MYSQL_TIME tm_now;
  tz->gmt_sec_to_TIME(&tm_now, thd->query_start_timeval_trunc(6));

  long now_day = calc_daynr(tm_now.year, tm_now.month, tm_now.day);

  Acl_cache_lock_guard acl_cache_lock{thd, Acl_cache_lock_mode::READ_MODE};
  // Play safe and pretend that table is empty if we fail to acquire ACL lock.
  if (!acl_cache_lock.lock()) return;

  // Return empty table in --skip-grant-tables mode.
  if (acl_users == nullptr) return;

  row_account_failed_login_lock_status *rows =
      (row_account_failed_login_lock_status *)thd->alloc(
          acl_users->size() * sizeof(row_account_failed_login_lock_status));
  if (rows == nullptr) return;

  uint index = 0;
  row_account_failed_login_lock_status *row = rows;

  for (const auto &user_entry : *acl_users) {
    make_row(now_day, &user_entry, row);

    index++;
    row++;
  }

  m_all_rows = rows;
  m_row_count = index;
}

int table_account_failed_login_lock_status::rnd_next() {
  int result;

  m_pos.set_at(&m_next_pos);

  if (m_pos.m_index < m_row_count) {
    m_row = &m_all_rows[m_pos.m_index];
    m_next_pos.set_after(&m_pos);
    result = 0;
  } else {
    m_row = nullptr;
    result = HA_ERR_END_OF_FILE;
  }

  return result;
}

int table_account_failed_login_lock_status::rnd_pos(const void *pos) {
  set_position(pos);
  assert(m_pos.m_index < m_row_count);
  m_row = &m_all_rows[m_pos.m_index];
  return 0;
}

void table_account_failed_login_lock_status::reset_position() {
  m_pos.m_index = 0;
  m_next_pos.m_index = 0;
}

int table_account_failed_login_lock_status::index_init(uint, bool) {
  PFS_index_account_failed_login_lock_status_by_user_host *result = nullptr;
  result = PFS_NEW(PFS_index_account_failed_login_lock_status_by_user_host);
  m_opened_index = result;
  m_index = result;
  return 0;
}

int table_account_failed_login_lock_status::index_next() {
  int result;

  for (m_pos.set_at(&m_next_pos); m_pos.m_index < m_row_count; m_pos.next()) {
    m_row = &m_all_rows[m_pos.m_index];

    if (m_opened_index->match(m_row)) {
      m_next_pos.set_after(&m_pos);
      result = 0;
      return result;
    }
  }

  m_row = nullptr;
  result = HA_ERR_END_OF_FILE;

  return result;
}

int table_account_failed_login_lock_status::read_row_values(TABLE *table,
                                                            unsigned char *buf,
                                                            Field **fields,
                                                            bool read_all) {
  Field *f;

  assert(m_row);

  /* Setting the null bits. */
  assert(table->s->null_bytes == 1);
  buf[0] = 0;

  for (; (f = *fields); fields++) {
    if (read_all || bitmap_is_set(table->read_set, f->field_index())) {
      switch (f->field_index()) {
        case 0: /* USER */
          set_nullable_field_user_name(f, &m_row->m_user_name);
          break;
        case 1: /* HOST */
          set_nullable_field_host_name(f, &m_row->m_host_name);
          break;
        case 2: /* IS_TRACKING_ACTIVE */
          set_field_enum(f, m_row->m_is_tracking_active ? ENUM_YES : ENUM_NO);
          break;
        case 3: /* MAX_ATTEMPTS */
          set_field_ulong(f, m_row->m_max_attempts);
          break;
        case 4: /* PASSWORD_LOCK_DAYS */
          set_field_long(f, m_row->m_password_lock_days);
          break;
        case 5: /* IS_LOCKED */
          if (m_row->m_is_tracking_active)
            set_field_enum(f, m_row->m_is_locked ? ENUM_YES : ENUM_NO);
          else
            f->set_null();
          break;
        case 6: /* REMAINING_ATTEMPTS */
          if (m_row->m_is_tracking_active)
            set_field_ulong(f, m_row->m_remaining_attempts);
          else
            f->set_null();
          break;
        case 7: /* REMAINING_DAYS_LOCKED */
          if (m_row->m_is_tracking_active)
            set_field_long(f, m_row->m_remaining_days_locked);
          else
            f->set_null();
          break;
        default:
          assert(false);
      }
    }
  }

  return 0;
}
