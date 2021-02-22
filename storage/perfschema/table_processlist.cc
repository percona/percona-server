/* Copyright (c) 2008, 2020, Oracle and/or its affiliates.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2.0,
  as published by the Free Software Foundation.

  This program is also distributed with certain software (including
  but not limited to OpenSSL) that is licensed under separate terms,
  as designated in a particular file or component or in included license
  documentation.  The authors of MySQL hereby grant you an additional
  permission to link the program and your derivative works with the
  separately licensed software that they have included with MySQL.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License, version 2.0, for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  @file storage/perfschema/table_processlist.cc
  TABLE PROCESSLIST.
*/

#include "storage/perfschema/table_processlist.h"

#include "lex_string.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_thread.h"
#include "sql/auth/auth_acls.h"
#include "sql/current_thd.h"
#include "sql/field.h"
#include "sql/plugin_table.h"
#include "sql/sql_class.h"
#include "sql/sql_parse.h"
#include "sql/table.h"
#include "storage/perfschema/pfs_buffer_container.h"
#include "storage/perfschema/pfs_global.h"
#include "storage/perfschema/pfs_instr.h"
#include "storage/perfschema/pfs_instr_class.h"

THR_LOCK table_processlist::m_table_lock;

Plugin_table table_processlist::m_table_def(
    /* Schema name */
    "performance_schema",
    /* Name */
    "processlist",
    /* Definition */
    "  ID BIGINT unsigned,\n"
    "  USER VARCHAR(32),\n"
    "  HOST VARCHAR(255) CHARACTER SET ASCII default null,\n"
    "  DB VARCHAR(64),\n"
    "  COMMAND VARCHAR(16),\n"
    "  TIME BIGINT,\n"
    "  STATE VARCHAR(64),\n"
    "  INFO LONGTEXT,\n"
    "  TIME_MS BIGINT unsigned not null,\n"
    "  ROWS_SENT BIGINT unsigned not null,\n"
    "  ROWS_EXAMINED BIGINT unsigned not null,\n"
    "  PRIMARY KEY (ID) USING HASH\n",
    /* Options */
    " ENGINE=PERFORMANCE_SCHEMA",
    /* Tablespace */
    nullptr);

PFS_engine_table_share table_processlist::m_share = {
    &pfs_readonly_processlist_acl,
    table_processlist::create,
    nullptr, /* write_row */
    nullptr, /* delete_all_rows */
    cursor_by_thread::get_row_count,
    sizeof(PFS_simple_index), /* ref length */
    &m_table_lock,
    &m_table_def,
    false, /* perpetual */
    PFS_engine_table_proxy(),
    {0},
    false /* m_in_purgatory */
};

PFS_engine_table *table_processlist::create(PFS_engine_table_share *) {
  return new table_processlist();
}

table_processlist::table_processlist() : cursor_by_thread(&m_share) {
  m_row_priv.m_auth = PROCESSLIST_DENIED;
}

int table_processlist::set_access(void) {
  THD *thd = current_thd;
  if (thd == nullptr) {
    /* Robustness, no user session. */
    m_row_priv.m_auth = PROCESSLIST_DENIED;
    return 0;
  }

  LEX_CSTRING client_priv_user = thd->security_context()->priv_user();
  if (client_priv_user.length == 0) {
    /* Anonymous user. */
    m_row_priv.m_auth = PROCESSLIST_DENIED;
    return 0;
  }

  if (thd->security_context()->check_access(PROCESS_ACL)) {
    /* PROCESS_ACL granted. */
    m_row_priv.m_auth = PROCESSLIST_ALL;
    return 0;
  }

  /* Authenticated user, PROCESS_ACL not granted. */
  m_row_priv.m_auth = PROCESSLIST_USER_ONLY;
  m_row_priv.m_priv_user_length =
      std::min(client_priv_user.length, sizeof(m_row_priv.m_priv_user));
  memcpy(m_row_priv.m_priv_user, client_priv_user.str,
         m_row_priv.m_priv_user_length);
  return 0;
}

int table_processlist::rnd_init(bool scan MY_ATTRIBUTE((unused))) {
  set_access();
  return 0;
}

bool PFS_index_processlist_by_processlist_id::match(PFS_thread *pfs) {
  if (m_fields >= 1) {
    if (!m_key.match(pfs)) {
      return false;
    }
  }
  return true;
}

int table_processlist::index_init(uint idx, bool) {
  PFS_index_threads *result = nullptr;
  switch (idx) {
    case 0:
      result = PFS_NEW(PFS_index_processlist_by_processlist_id);
      break;
    default:
      DBUG_ASSERT(false);
  }
  m_opened_index = result;
  m_index = result;
  set_access();
  return 0;
}

int table_processlist::make_row(PFS_thread *pfs) {
  pfs_optimistic_state lock;
  pfs_optimistic_state session_lock;
  pfs_optimistic_state stmt_lock;
  PFS_stage_class *stage_class;
  PFS_thread_class *safe_class;

  if (m_row_priv.m_auth == PROCESSLIST_DENIED) {
    return HA_ERR_END_OF_FILE;
  }

  /* Protect this reader against thread termination */
  pfs->m_lock.begin_optimistic_lock(&lock);

  safe_class = sanitize_thread_class(pfs->m_class);
  if (unlikely(safe_class == nullptr)) {
    return HA_ERR_RECORD_DELETED;
  }

  /* Ignore background threads. */
  if (pfs->m_username_length == 0 || pfs->m_processlist_id == 0)
    return HA_ERR_RECORD_DELETED;

  m_row.m_processlist_id = pfs->m_processlist_id;

  /* Protect this reader against session attribute changes */
  pfs->m_session_lock.begin_optimistic_lock(&session_lock);

  /* Maintain user/host compatibility with the legacy SHOW PROCESSLIST. */
  const char *username = pfs->m_username;
  uint username_len = pfs->m_username_length;
  uint hostname_len = pfs->m_hostname_length;

  if (pfs->m_class->is_system_thread()) {
    if (username_len == 0 ||
        (!strncmp(username, "root", 4) && username_len == 4)) {
      username = "system user";
      username_len = strlen(username);
      hostname_len = 0;
    }
  } else {
    if (username_len == 0) {
      username = "unauthenticated user";
      username_len = strlen(username);
      hostname_len = 0;
    }
  }

  m_row.m_username_length = username_len;
  if (unlikely(m_row.m_username_length > sizeof(m_row.m_username))) {
    return HA_ERR_RECORD_DELETED;
  }

  if (m_row.m_username_length != 0) {
    memcpy(m_row.m_username, username, username_len);
  }

  m_row.m_hostname_length = hostname_len;
  if (unlikely(m_row.m_hostname_length > sizeof(m_row.m_hostname))) {
    return HA_ERR_RECORD_DELETED;
  }

  if (m_row.m_hostname_length != 0) {
    memcpy(m_row.m_hostname, pfs->m_hostname, m_row.m_hostname_length);
  }

  if (!pfs->m_session_lock.end_optimistic_lock(&session_lock)) {
    /*
      One of the columns:
      - USER
      - HOST
      is being updated.
      Do not discard the entire row.
      Do not loop waiting for a stable value.
      Just return NULL values.
    */
    m_row.m_username_length = 0;
    m_row.m_hostname_length = 0;
  }

  /* Enforce row filtering. */
  if (m_row_priv.m_auth == PROCESSLIST_USER_ONLY) {
    if (m_row.m_username_length != m_row_priv.m_priv_user_length) {
      return HA_ERR_RECORD_DELETED;
    }
    if (strncmp(m_row.m_username, m_row_priv.m_priv_user,
                m_row_priv.m_priv_user_length) != 0) {
      return HA_ERR_RECORD_DELETED;
    }
  }

  /* Protect this reader against statement attributes changes */
  pfs->m_stmt_lock.begin_optimistic_lock(&stmt_lock);

  m_row.m_dbname_length = pfs->m_dbname_length;
  if (unlikely(m_row.m_dbname_length > sizeof(m_row.m_dbname))) {
    return HA_ERR_RECORD_DELETED;
  }

  if (m_row.m_dbname_length != 0) {
    memcpy(m_row.m_dbname, pfs->m_dbname, m_row.m_dbname_length);
  }

  m_row.m_processlist_info_ptr = &pfs->m_processlist_info[0];
  m_row.m_processlist_info_length = pfs->m_processlist_info_length;

  if (!pfs->m_stmt_lock.end_optimistic_lock(&stmt_lock)) {
    /*
      One of the columns:
      - DB
      - INFO
      is being updated.
      Do not discard the entire row.
      Do not loop waiting for a stable value.
      Just return NULL values.
    */
    m_row.m_dbname_length = 0;
    m_row.m_processlist_info_length = 0;
  }

  /* Dirty read, sanitize the command. */
  m_row.m_command = pfs->m_command;
  if ((m_row.m_command < 0) || (m_row.m_command > COM_END)) {
    m_row.m_command = COM_END;
  }

  m_row.m_start_time = pfs->m_start_time;

  stage_class = find_stage_class(pfs->m_stage);
  if (stage_class != nullptr) {
    m_row.m_processlist_state_ptr =
        stage_class->m_name + stage_class->m_prefix_length;
    m_row.m_processlist_state_length =
        stage_class->m_name_length - stage_class->m_prefix_length;
    if (m_row.m_processlist_state_length > 64) {
      /*
        Column STATE is VARCHAR(64)
        for compatibility reasons with the historical
        INFORMATION_SCHEMA.PROCESSLIST table.
        Stages however can have longer names.
        We silently truncate data here,
        so it fits into STATE.
      */
      m_row.m_processlist_state_length = 64;
    }
  } else {
    m_row.m_processlist_state_length = 0;
  }

  m_row.m_port = pfs->m_peer_port;

  if (m_row.m_hostname_length > 0 && m_row.m_port != 0) {
    /* Create HOST:PORT. */
    std::string host(m_row.m_hostname, m_row.m_hostname_length);
    std::string host_ip = host + ":" + std::to_string(m_row.m_port);
    m_row.m_hostname_length =
        std::min((int)HOST_AND_PORT_LENGTH, (int)host_ip.length());
    memcpy(m_row.m_hostname, host_ip.c_str(), m_row.m_hostname_length);
  }

  m_row.m_start_time_usec = pfs->m_start_time_usec;
  m_row.m_rows_sent = pfs->m_rows_sent;
  m_row.m_rows_examined = pfs->m_rows_examined;

  if (!pfs->m_lock.end_optimistic_lock(&lock)) {
    return HA_ERR_RECORD_DELETED;
  }

  return 0;
}

int table_processlist::read_row_values(TABLE *table, unsigned char *buf,
                                       Field **fields, bool read_all) {
  Field *f;

  /* Set the null bits */
  DBUG_ASSERT(table->s->null_bytes == 1);
  buf[0] = 0;

  for (; (f = *fields); fields++) {
    if (read_all || bitmap_is_set(table->read_set, f->field_index())) {
      switch (f->field_index()) {
        case 0: /* ID */
          if (m_row.m_processlist_id != 0) {
            set_field_ulonglong(f, m_row.m_processlist_id);
          } else {
            f->set_null();
          }
          break;
        case 1: /* USER */
          if (m_row.m_username_length > 0) {
            set_field_varchar_utf8(f, m_row.m_username,
                                   m_row.m_username_length);
          } else {
            f->set_null();
          }
          break;
        case 2: /* HOST */
          if (m_row.m_hostname_length > 0) {
            set_field_varchar_utf8(f, m_row.m_hostname,
                                   m_row.m_hostname_length);
          } else {
            f->set_null();
          }
          break;
        case 3: /* DB */
          if (m_row.m_dbname_length > 0) {
            set_field_varchar_utf8(f, m_row.m_dbname, m_row.m_dbname_length);
          } else {
            f->set_null();
          }
          break;
        case 4: /* COMMAND */
          if (m_row.m_processlist_id != 0)
            set_field_varchar_utf8(f, command_name[m_row.m_command].str,
                                   (uint)command_name[m_row.m_command].length);
          else {
            f->set_null();
          }
          break;
        case 5: /* TIME */
          if (m_row.m_start_time) {
            time_t now = my_time(0);
            ulonglong elapsed =
                (now > m_row.m_start_time ? now - m_row.m_start_time : 0);
            set_field_ulonglong(f, elapsed);
          } else {
            f->set_null();
          }
          break;
        case 6: /* STATE */
          /* For compatibility, leave blank if state is NULL. */
          set_field_varchar_utf8(f, m_row.m_processlist_state_ptr,
                                 m_row.m_processlist_state_length);
          break;
        case 7: /* INFO */
          if (m_row.m_processlist_info_length > 0)
            set_field_blob(f, m_row.m_processlist_info_ptr,
                           m_row.m_processlist_info_length);
          else {
            f->set_null();
          }
          break;
        case 8: /* TIME_MS */
          if (m_row.m_start_time_usec) {
            auto now = my_micro_time();
            auto elapsed =
                (now > m_row.m_start_time_usec ? now - m_row.m_start_time_usec
                                               : 0) /
                1000;
            set_field_ulonglong(f, elapsed);
          } else {
            f->set_null();
          }
          break;
        case 9: /* ROWS_SENT */
          set_field_ulonglong(f, m_row.m_rows_sent);
          break;
        case 10: /* ROWS_EXAMINED */
          set_field_ulonglong(f, m_row.m_rows_examined);
          break;
        default:
          DBUG_ASSERT(false);
      }
    }
  }
  return 0;
}
