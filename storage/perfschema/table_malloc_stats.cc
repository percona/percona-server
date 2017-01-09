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

/**
  @file storage/perfschema/table_malloc_stats_summary.cc
  Table SOCKET_EVENT_NAMES (implementation).
*/

#include "my_global.h"
#include "my_pthread.h"
#include "pfs_instr.h"
#include "pfs_column_types.h"
#include "pfs_column_values.h"
#include "table_malloc_stats.h"
#include "pfs_global.h"

THR_LOCK table_malloc_stats_summary::m_table_lock;

static uint64_t epoch = 0;

static const char* summary_stat_type[NUM_SUMMARY_STAT] = { "stats.allocated", "stats.active", "stats.mapped", "stats.resident", "stats.retained", "stats.metadata" };

static const TABLE_FIELD_TYPE field_types_summary[]=
{
  { { C_STRING_WITH_LEN("ALLOCATED") }, { C_STRING_WITH_LEN("bigint(20)") }, {NULL, 0} },
  { { C_STRING_WITH_LEN("ACTIVE")    }, { C_STRING_WITH_LEN("bigint(20)") }, {NULL, 0} },
  { { C_STRING_WITH_LEN("MAPPED")    }, { C_STRING_WITH_LEN("bigint(20)") }, {NULL, 0} },
  { { C_STRING_WITH_LEN("RESIDENT")  }, { C_STRING_WITH_LEN("bigint(20)") }, {NULL, 0} },
  { { C_STRING_WITH_LEN("RETAINED")  }, { C_STRING_WITH_LEN("bigint(20)") }, {NULL, 0} },
  { { C_STRING_WITH_LEN("METADATA")  }, { C_STRING_WITH_LEN("bigint(20)") }, {NULL, 0} }
};

TABLE_FIELD_DEF
table_malloc_stats_summary::m_field_def=
{ 6, field_types_summary };

PFS_engine_table_share
table_malloc_stats_summary::m_share=
{
  { C_STRING_WITH_LEN("malloc_stats_summary") },
  &pfs_readonly_acl,
  &table_malloc_stats_summary::create,
  NULL, /* write_row */
  NULL, /* delete_all_rows */
  NULL, /* get_row_count */
  1, /* records */
  sizeof(PFS_simple_index),
  &m_table_lock,
  &m_field_def,
  false /* checked */
};

PFS_engine_table* table_malloc_stats_summary::create(void)
{
  return new table_malloc_stats_summary();
}

table_malloc_stats_summary::table_malloc_stats_summary()
  : PFS_engine_table(&m_share, &m_pos),
  m_row_exists(false), m_pos(0), m_next_pos(0)
{
}

void table_malloc_stats_summary::reset_position(void)
{
  m_pos.m_index= 0;
  m_next_pos.m_index= 0;
}

int table_malloc_stats_summary::rnd_next(void)
{
  m_pos.set_at(&m_next_pos);

  if (!m_pos.m_index)
  {
    make_row();
    m_next_pos.set_after(&m_pos);
    return 0;
  }

  return HA_ERR_END_OF_FILE;
}

int table_malloc_stats_summary::rnd_pos(const void *pos)
{
  set_position(pos);

  if (!m_pos.m_index)
  {
    make_row();
    return 0;
  }

  return HA_ERR_RECORD_DELETED;
}


void table_malloc_stats_summary::make_row()
{
  memset(&m_row, 0, sizeof(m_row));

  size_t sz = sizeof(size_t);

  epoch++;
  jemalloc_mallctl("epoch", NULL, 0, &epoch, sz);

  for(unsigned i = 0; i < NUM_SUMMARY_STAT; i++)
    jemalloc_mallctl(summary_stat_type[i], &m_row.stat[i], &sz, NULL, 0);

  m_row_exists= true;
}

int table_malloc_stats_summary::read_row_values(TABLE *table,
                                          unsigned char *,
                                          Field **fields,
                                          bool read_all)
{
  Field *f;

  if (unlikely(!m_row_exists))
    return HA_ERR_RECORD_DELETED;

  /* Set the null bits */
  DBUG_ASSERT(table->s->null_bytes == 1);

  for (; (f= *fields) ; fields++)
  {
    if (read_all || bitmap_is_set(table->read_set, f->field_index))
    {
      DBUG_ASSERT(f->field_index < NUM_SUMMARY_STAT);
      set_field_ulonglong(f, m_row.stat[f->field_index]);
    } // if
  } // for

  return 0;
}

THR_LOCK table_malloc_stats::m_table_lock;

static const char* row_type[] = { "small", "large", "huge" };
#define NUM_ROWS (sizeof(row_type) / sizeof(row_type[0]))
static const char* stat_type[NUM_STAT] = { "allocated", "nmalloc", "ndmalloc", "nrequests" };

static const TABLE_FIELD_TYPE field_types[]=
{
  { { C_STRING_WITH_LEN("TYPE") }, { C_STRING_WITH_LEN("char(8)") }, {NULL, 0} },
  { { C_STRING_WITH_LEN("ALLOCATED") }, { C_STRING_WITH_LEN("bigint(20)") }, {NULL, 0} },
  { { C_STRING_WITH_LEN("NMALLOC")    }, { C_STRING_WITH_LEN("bigint(20)") }, {NULL, 0} },
  { { C_STRING_WITH_LEN("NDALLOC")    }, { C_STRING_WITH_LEN("bigint(20)") }, {NULL, 0} },
  { { C_STRING_WITH_LEN("NREQUESTS")  }, { C_STRING_WITH_LEN("bigint(20)") }, {NULL, 0} }
};

TABLE_FIELD_DEF
table_malloc_stats::m_field_def=
{ 5, field_types };

PFS_engine_table_share
table_malloc_stats::m_share=
{
  { C_STRING_WITH_LEN("malloc_stats") },
  &pfs_readonly_acl,
  &table_malloc_stats::create,
  NULL, /* write_row */
  NULL, /* delete_all_rows */
  NULL, /* get_row_count */
  5, /* records */
  sizeof(PFS_simple_index),
  &m_table_lock,
  &m_field_def,
  false /* checked */
};

PFS_engine_table* table_malloc_stats::create(void)
{
  return new table_malloc_stats();
}

table_malloc_stats::table_malloc_stats()
  : PFS_engine_table(&m_share, &m_pos),
  m_row_exists(false), m_pos(0), m_next_pos(0)
{
}

void table_malloc_stats::reset_position(void)
{
  m_pos.m_index= 0;
  m_next_pos.m_index= 0;
}

int table_malloc_stats::rnd_next(void)
{
  m_pos.set_at(&m_next_pos);

  if (m_pos.m_index < NUM_ROWS)
  {
    make_row(m_pos.m_index);
    m_next_pos.set_after(&m_pos);
    return 0;
  }

  return HA_ERR_END_OF_FILE;
}

int table_malloc_stats::rnd_pos(const void *pos)
{
  set_position(pos);

  if (m_pos.m_index < NUM_ROWS)
  {
    make_row(m_pos.m_index);
    return 0;
  }

  return HA_ERR_RECORD_DELETED;
}


void table_malloc_stats::make_row(int index)
{
  char s[64];
  size_t sz;

  memset(&m_row, 0, sizeof(m_row));

  epoch++;
  jemalloc_mallctl("epoch", NULL, 0, &epoch, sz);

  m_row.type = index;

  unsigned n = 0;
  sz = sizeof(unsigned);
  /* Reading the index of merged arenas statistics, which equals to
   * number of arenas. */
  jemalloc_mallctl("arenas.narenas", &n, &sz, NULL, 0);

  sz = sizeof(size_t);
  for(unsigned i = 0; i < NUM_STAT; i++)
  {
    sprintf(s, "stats.arenas.%u.%s.%s", n, row_type[m_row.type], stat_type[i]);
    jemalloc_mallctl(s, &m_row.stat[i], &sz, NULL, 0);
  }

  m_row_exists= true;
}

int table_malloc_stats::read_row_values(TABLE *table,
                                          unsigned char *,
                                          Field **fields,
                                          bool read_all)
{
  Field *f;

  if (unlikely(!m_row_exists))
    return HA_ERR_RECORD_DELETED;

  /* Set the null bits. If table has at least one VARCHAR and no nullable fields
   * this should be checked against 0 instead of 1 */
  DBUG_ASSERT(table->s->null_bytes == 1);

  for (; (f= *fields) ; fields++)
  {
    if (read_all || bitmap_is_set(table->read_set, f->field_index))
    {
      if(!f->field_index) { /* TYPE */
        const char* tp = row_type[m_row.type];
        set_field_char_utf8(f, tp, strlen(tp));
      } else if (f->field_index <= NUM_STAT) /* ALLOCATED, NMALLOC, NDALLOC, NREQUESTS */
        set_field_ulonglong(f, m_row.stat[f->field_index - 1]);
      else
        DBUG_ASSERT(false);
    } // if
  } // for

  return 0;
}

