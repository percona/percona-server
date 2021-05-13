/* Copyright (c) 2008, 2011, Oracle and/or its affiliates. All rights reserved.
   Copyright (c) 2016 Uber Technologies, Inc.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software Foundation,
  51 Franklin Street, Suite 500, Boston, MA 02110-1335 USA */

#include <atomic>

#include "storage/perfschema/table_malloc_stats.h"

#include "sql/field.h"
#include "sql/plugin_table.h"
#include "sql/sql_profile.h"
#include "storage/perfschema/pfs_column_types.h"
#include "storage/perfschema/pfs_column_values.h"
#include "storage/perfschema/pfs_global.h"
#include "storage/perfschema/pfs_instr.h"

THR_LOCK table_malloc_stats_totals::m_table_lock;

static std::atomic<uint64_t> epoch{0};

static void set_jemalloc_epoch() {
  auto epoch_copy = epoch++;
  size_t sz = sizeof(epoch_copy);
  jemalloc_mallctl("epoch", nullptr, nullptr, &epoch_copy, sz);
}

static const char *summary_stat_type[NUM_SUMMARY_STAT] = {
    "stats.allocated", "stats.active",   "stats.mapped",
    "stats.resident",  "stats.retained", "stats.metadata"};

Plugin_table table_malloc_stats_totals::m_table_def(
    /* Schema name */
    "performance_schema",
    /* Name. Do not use `summary` or `history` in the name.
     * It'd be tried to be truncated in some tests. */
    "malloc_stats_totals",
    /* Definition */
    " ALLOCATED bigint(20) unsigned not null,\n"
    " ACTIVE bigint(20) unsigned not null,\n"
    " MAPPED bigint(20) unsigned not null,\n"
    " RESIDENT bigint(20) unsigned not null,\n"
    " RETAINED bigint(20) unsigned not null,\n"
    " METADATA bigint(20) unsigned not null\n",
    /* Options */
    " ENGINE=PERFORMANCE_SCHEMA",
    /* Tablespace */
    nullptr);

PFS_engine_table_share table_malloc_stats_totals::m_share = {
    &pfs_readonly_acl,
    table_malloc_stats_totals::create,
    nullptr, /* write_row */
    nullptr, /* delete_all_rows */
    table_malloc_stats_totals::get_row_count,
    sizeof(PFS_simple_index),
    &m_table_lock,
    &m_table_def,
    false, /* perpetual */
    PFS_engine_table_proxy(),
    {0},
    false /* m_in_purgatory */
};

PFS_engine_table *table_malloc_stats_totals::create(PFS_engine_table_share *) {
  return new table_malloc_stats_totals();
}

ha_rows table_malloc_stats_totals::get_row_count() { return 1; }

table_malloc_stats_totals::table_malloc_stats_totals()
    : PFS_engine_table(&m_share, &m_pos),
      m_row_exists(false),
      m_pos(0),
      m_next_pos(0) {}

void table_malloc_stats_totals::reset_position(void) {
  m_pos.m_index = 0;
  m_next_pos.m_index = 0;
}

int table_malloc_stats_totals::rnd_next(void) {
  m_pos.set_at(&m_next_pos);

  if (!m_pos.m_index) {
    make_row();
    m_next_pos.set_after(&m_pos);
    return 0;
  }

  return HA_ERR_END_OF_FILE;
}

int table_malloc_stats_totals::rnd_pos(const void *pos) {
  set_position(pos);

  if (!m_pos.m_index) {
    make_row();
    return 0;
  }

  return HA_ERR_RECORD_DELETED;
}

void table_malloc_stats_totals::make_row() {
  set_jemalloc_epoch();

  memset(&m_row, 0, sizeof(m_row));
  size_t sz = sizeof(m_row.stat[0]);
  for (unsigned i = 0; i < NUM_SUMMARY_STAT; i++)
    jemalloc_mallctl(summary_stat_type[i], &m_row.stat[i], &sz, nullptr, 0);

  m_row_exists = true;
}

int table_malloc_stats_totals::read_row_values(TABLE *table, unsigned char *,
                                               Field **fields, bool read_all) {
  Field *f;

  if (unlikely(!m_row_exists)) return HA_ERR_RECORD_DELETED;

  /* Set the null bits */
  assert(table->s->null_bytes == 1);

  for (; (f = *fields); fields++) {
    if (read_all || bitmap_is_set(table->read_set, f->field_index())) {
      assert(f->field_index() < NUM_SUMMARY_STAT);
      set_field_ulonglong(f, m_row.stat[f->field_index()]);
    }  // if
  }    // for

  return 0;
}

THR_LOCK table_malloc_stats::m_table_lock;

static const char *row_type[] = {"small", "large", "huge"};
#define NUM_ROWS (sizeof(row_type) / sizeof(row_type[0]))
static const char *stat_type[NUM_STAT] = {"allocated", "nmalloc", "ndmalloc",
                                          "nrequests"};

Plugin_table table_malloc_stats::m_table_def(
    /* Schema name */
    "performance_schema",
    /* Name */
    "malloc_stats",
    /* Definition */
    " TYPE char(8) not null,\n"
    " ALLOCATED bigint(20) unsigned not null,\n"
    " NMALLOC bigint(20) unsigned not null,\n"
    " NDALLOC bigint(20) unsigned not null,\n"
    " NREQUESTS bigint(20) unsigned not null\n",
    /* Options */
    " ENGINE=PERFORMANCE_SCHEMA",
    /* Tablespace */
    nullptr);

PFS_engine_table_share table_malloc_stats::m_share = {
    &pfs_readonly_acl,
    table_malloc_stats::create,
    nullptr, /* write_row */
    nullptr, /* delete_all_rows */
    table_malloc_stats::get_row_count,
    sizeof(PFS_simple_index),
    &m_table_lock,
    &m_table_def,
    false, /* perpetual */
    PFS_engine_table_proxy(),
    {0},
    false /* m_in_purgatory */
};

PFS_engine_table *table_malloc_stats::create(PFS_engine_table_share *) {
  return new table_malloc_stats();
}

ha_rows table_malloc_stats::get_row_count() { return 5; }

table_malloc_stats::table_malloc_stats()
    : PFS_engine_table(&m_share, &m_pos),
      m_row_exists(false),
      m_pos(0),
      m_next_pos(0) {}

void table_malloc_stats::reset_position(void) {
  m_pos.m_index = 0;
  m_next_pos.m_index = 0;
}

int table_malloc_stats::rnd_next(void) {
  m_pos.set_at(&m_next_pos);

  if (m_pos.m_index < NUM_ROWS) {
    make_row(m_pos.m_index);
    m_next_pos.set_after(&m_pos);
    return 0;
  }

  return HA_ERR_END_OF_FILE;
}

int table_malloc_stats::rnd_pos(const void *pos) {
  set_position(pos);

  if (m_pos.m_index < NUM_ROWS) {
    make_row(m_pos.m_index);
    return 0;
  }

  return HA_ERR_RECORD_DELETED;
}

void table_malloc_stats::make_row(int index) {
  set_jemalloc_epoch();

  unsigned n = 0;
  size_t sz = sizeof(n);
  /* Reading the index of merged arenas statistics, which equals to
   * number of arenas. */
  jemalloc_mallctl("arenas.narenas", &n, &sz, nullptr, 0);

  memset(&m_row, 0, sizeof(m_row));
  m_row.type = index;
  sz = sizeof(m_row.stat[0]);
  for (unsigned i = 0; i < NUM_STAT; i++) {
    char s[64];
    snprintf(s, sizeof(s), "stats.arenas.%u.%s.%s", n, row_type[m_row.type],
             stat_type[i]);
    jemalloc_mallctl(s, &m_row.stat[i], &sz, nullptr, 0);
  }

  m_row_exists = true;
}

int table_malloc_stats::read_row_values(TABLE *table, unsigned char *,
                                        Field **fields, bool read_all) {
  Field *f;

  if (unlikely(!m_row_exists)) return HA_ERR_RECORD_DELETED;

  /* Set the null bits. If table has at least one VARCHAR and no nullable fields
   * this should be checked against 0 instead of 1 */
  assert(table->s->null_bytes == 1);

  for (; (f = *fields); fields++) {
    if (read_all || bitmap_is_set(table->read_set, f->field_index())) {
      if (!f->field_index()) { /* TYPE */
        const char *tp = row_type[m_row.type];
        set_field_char_utf8mb4(f, tp, strlen(tp));
      } else if (f->field_index() <=
                 NUM_STAT) /* ALLOCATED, NMALLOC, NDALLOC, NREQUESTS */
        set_field_ulonglong(f, m_row.stat[f->field_index() - 1]);
      else
        assert(false);
    }  // if
  }    // for

  return 0;
}
