/* Copyright (c) 2026 Percona LLC and/or its affiliates. All rights reserved.

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
  @file storage/perfschema/table_ews_by_source.cc
  Table EVENTS_WAITS_SUMMARY_BY_SOURCE (implementation).
*/

#include "storage/perfschema/table_ews_by_source.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <algorithm>

#include "lex_string.h"
#include "my_base.h"
#include "my_bitmap.h"
#include "sql/field.h"
#include "sql/plugin_table.h"
#include "sql/table.h"
#include "storage/perfschema/pfs_column_types.h"
#include "storage/perfschema/pfs_engine_table.h"
#include "storage/perfschema/pfs_instr_class.h"
#include "storage/perfschema/pfs_latch_source.h"
#include "storage/perfschema/pfs_lock.h"
#include "storage/perfschema/pfs_stat.h"
#include "storage/perfschema/pfs_timer.h"
#include "storage/perfschema/table_helper.h"
#include "string_with_len.h"

THR_LOCK table_ews_by_source::m_table_lock;

Plugin_table table_ews_by_source::m_table_def(
    /* Schema name */
    "performance_schema",
    /* Name */
    "events_waits_summary_by_source",
    /* Definition */
    "  EVENT_NAME VARCHAR(128) not null,\n"
    "  SOURCE VARCHAR(64) not null,\n"
    "  OPERATION VARCHAR(32) not null,\n"
    "  COUNT_STAR BIGINT unsigned not null,\n"
    "  SUM_TIMER_WAIT BIGINT unsigned not null,\n"
    "  MIN_TIMER_WAIT BIGINT unsigned not null,\n"
    "  AVG_TIMER_WAIT BIGINT unsigned not null,\n"
    "  MAX_TIMER_WAIT BIGINT unsigned not null,\n"
    "  COUNT_HOLD BIGINT unsigned not null,\n"
    "  SUM_TIMER_HOLD BIGINT unsigned not null,\n"
    "  MIN_TIMER_HOLD BIGINT unsigned not null,\n"
    "  AVG_TIMER_HOLD BIGINT unsigned not null,\n"
    "  MAX_TIMER_HOLD BIGINT unsigned not null\n",
    /* Options */
    " ENGINE=PERFORMANCE_SCHEMA",
    /* Tablespace */
    nullptr);

PFS_engine_table_share table_ews_by_source::m_share = {
    &pfs_truncatable_acl,
    table_ews_by_source::create,
    nullptr, /* write_row */
    table_ews_by_source::delete_all_rows,
    table_ews_by_source::get_row_count,
    sizeof(PFS_simple_index),
    &m_table_lock,
    &m_table_def,
    false, /* perpetual */
    PFS_engine_table_proxy(),
    {0},
    false /* m_in_purgatory */
};

/** Names of the acquire operations, see enum_operation_type. */
static LEX_CSTRING operation_name(uint operation) {
  switch (operation) {
    case OPERATION_TYPE_LOCK:
      return {STRING_WITH_LEN("lock")};
    case OPERATION_TYPE_TRYLOCK:
      return {STRING_WITH_LEN("try_lock")};
    case OPERATION_TYPE_READLOCK:
      return {STRING_WITH_LEN("read_lock")};
    case OPERATION_TYPE_WRITELOCK:
      return {STRING_WITH_LEN("write_lock")};
    case OPERATION_TYPE_TRYREADLOCK:
      return {STRING_WITH_LEN("try_read_lock")};
    case OPERATION_TYPE_TRYWRITELOCK:
      return {STRING_WITH_LEN("try_write_lock")};
    case OPERATION_TYPE_SHAREDLOCK:
      return {STRING_WITH_LEN("shared_lock")};
    case OPERATION_TYPE_SHAREDEXCLUSIVELOCK:
      return {STRING_WITH_LEN("shared_exclusive_lock")};
    case OPERATION_TYPE_EXCLUSIVELOCK:
      return {STRING_WITH_LEN("exclusive_lock")};
    case OPERATION_TYPE_TRYSHAREDLOCK:
      return {STRING_WITH_LEN("try_shared_lock")};
    case OPERATION_TYPE_TRYSHAREDEXCLUSIVELOCK:
      return {STRING_WITH_LEN("try_shared_exclusive_lock")};
    case OPERATION_TYPE_TRYEXCLUSIVELOCK:
      return {STRING_WITH_LEN("try_exclusive_lock")};
    default:
      assert(false);
      return {STRING_WITH_LEN("")};
  }
}

PFS_engine_table *table_ews_by_source::create(PFS_engine_table_share *) {
  return new table_ews_by_source();
}

int table_ews_by_source::delete_all_rows() {
  reset_latch_source();
  return 0;
}

ha_rows table_ews_by_source::get_row_count() { return latch_source_max; }

table_ews_by_source::table_ews_by_source()
    : PFS_engine_table(&m_share, &m_pos), m_pos(0), m_next_pos(0) {
  m_normalizer = time_normalizer::get_wait();
}

void table_ews_by_source::reset_position() {
  m_pos.m_index = 0;
  m_next_pos.m_index = 0;
}

int table_ews_by_source::rnd_next() {
  for (m_pos.set_at(&m_next_pos); m_pos.m_index < latch_source_max;
       m_pos.next()) {
    if (make_row(&latch_source_stat_array[m_pos.m_index]) == 0) {
      m_next_pos.set_after(&m_pos);
      return 0;
    }
  }

  return HA_ERR_END_OF_FILE;
}

int table_ews_by_source::rnd_pos(const void *pos) {
  set_position(pos);
  assert(m_pos.m_index < latch_source_max);
  if (make_row(&latch_source_stat_array[m_pos.m_index]) == 0) {
    return 0;
  }
  return HA_ERR_RECORD_DELETED;
}

int table_ews_by_source::make_row(PFS_latch_source_stat *stat) {
  pfs_optimistic_state lock;

  if (!stat->m_lock.is_populated()) {
    return HA_ERR_RECORD_DELETED;
  }

  /* Copy the row, then check it was not freed or reused meanwhile. */
  stat->m_lock.begin_optimistic_lock(&lock);
  const PFS_latch_source_key key = stat->m_key;
  PFS_single_stat wait;
  PFS_single_stat hold;
  stat->m_wait.copy_to(&wait);
  stat->m_hold.copy_to(&hold);

  if (!stat->m_lock.end_optimistic_lock(&lock) || wait.m_count == 0) {
    return HA_ERR_RECORD_DELETED;
  }

  m_row.m_event_name.make_row(key.m_class);
  m_row.m_operation = key.m_operation;

  const int length =
      snprintf(m_row.m_source, sizeof(m_row.m_source), "%.*s:%u",
               static_cast<int>(strnlen(key.m_file, sizeof(key.m_file))),
               key.m_file, key.m_line);
  m_row.m_source_length = static_cast<uint>(
      std::min<size_t>(std::max(length, 0), sizeof(m_row.m_source) - 1));

  m_row.m_wait.set(m_normalizer, &wait);
  m_row.m_hold.set(m_normalizer, &hold);
  return 0;
}

int table_ews_by_source::read_row_values(TABLE *table, unsigned char *,
                                         Field **fields, bool read_all) {
  Field *f;

  /* Set the null bits */
  assert(table->s->null_bytes == 0);

  for (; (f = *fields); fields++) {
    if (read_all || bitmap_is_set(table->read_set, f->field_index())) {
      switch (f->field_index()) {
        case 0: /* EVENT_NAME */
          m_row.m_event_name.set_field(f);
          break;
        case 1: /* SOURCE */
          set_field_varchar_utf8mb4(f, m_row.m_source, m_row.m_source_length);
          break;
        case 2: /* OPERATION */
        {
          const LEX_CSTRING name = operation_name(m_row.m_operation);
          set_field_varchar_utf8mb4(f, name.str, name.length);
          break;
        }
        case 3: /* COUNT_STAR */
        case 4: /* SUM_TIMER_WAIT */
        case 5: /* MIN_TIMER_WAIT */
        case 6: /* AVG_TIMER_WAIT */
        case 7: /* MAX_TIMER_WAIT */
          m_row.m_wait.set_field(f->field_index() - 3, f);
          break;
        default: /* COUNT_HOLD, SUM/MIN/AVG/MAX_TIMER_HOLD */
          m_row.m_hold.set_field(f->field_index() - 8, f);
          break;
      }
    }
  }

  return 0;
}
