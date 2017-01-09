/* Copyright (c) 2008, 2010, Oracle and/or its affiliates. All rights reserved.
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

#ifndef TABLE_MALLOC_STATS_H
#define TABLE_MALLOC_STATS_H

/**
  @file storage/perfschema/table_malloc_stats_summary.h
  Table MALLOC_STATSE (declarations).
*/

#include "pfs_column_types.h"
#include "pfs_engine_table.h"
#include "pfs_instr_class.h"
#include "pfs_instr.h"
#include "table_helper.h"

/**
  @addtogroup Performance_schema_tables
  @{
*/

/**
  A row of table
  PERFORMANCE_SCHEMA.MALLOC_STATS_SUMMARY.
*/

#define NUM_SUMMARY_STAT 6
struct row_malloc_stats_summary
{
	long stat[NUM_SUMMARY_STAT];
};

/** Table PERFORMANCE_SCHEMA.MALLOC_STATS_SUMMARY */
class table_malloc_stats_summary : public PFS_engine_table
{
public:
  /** Table share */
  static PFS_engine_table_share m_share;
  static PFS_engine_table* create();

  virtual int rnd_next();
  virtual int rnd_pos(const void *pos);
  virtual void reset_position(void);

private:
  virtual int read_row_values(TABLE *table,
                              unsigned char *buf,
                              Field **fields,
                              bool read_all);

  table_malloc_stats_summary();

public:
  ~table_malloc_stats_summary()
  { }

private:
  void make_row();

  /** Table share lock. */
  static THR_LOCK m_table_lock;
  /** Fields definition. */
  static TABLE_FIELD_DEF m_field_def;

  /** Current row. */
  row_malloc_stats_summary m_row;
  /** True if the current row exists. */
  bool m_row_exists;
  /** Current position. */
  PFS_simple_index m_pos;
  /** Next position. */
  PFS_simple_index m_next_pos;
};

/**
  A row of table
  PERFORMANCE_SCHEMA.MALLOC_STATS_SUMMARY.
*/

#define NUM_STAT 4
struct row_malloc_stats
{
	int type;
	long stat[NUM_STAT];
};

/** Table PERFORMANCE_SCHEMA.MALLOC_STATS_SUMMARY */
class table_malloc_stats : public PFS_engine_table
{
public:
  /** Table share */
  static PFS_engine_table_share m_share;
  static PFS_engine_table* create();

  virtual int rnd_next();
  virtual int rnd_pos(const void *pos);
  virtual void reset_position(void);

private:
  virtual int read_row_values(TABLE *table,
                              unsigned char *buf,
                              Field **fields,
                              bool read_all);

  table_malloc_stats();

public:
  ~table_malloc_stats()
  { }

private:
  void make_row(int index);

  /** Table share lock. */
  static THR_LOCK m_table_lock;
  /** Fields definition. */
  static TABLE_FIELD_DEF m_field_def;

  /** Current row. */
  row_malloc_stats m_row;
  /** True if the current row exists. */
  bool m_row_exists;
  /** Current position. */
  PFS_simple_index m_pos;
  /** Next position. */
  PFS_simple_index m_next_pos;
};

/** @} */
#endif
