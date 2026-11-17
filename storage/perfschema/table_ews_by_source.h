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

#ifndef TABLE_EWS_BY_SOURCE_H
#define TABLE_EWS_BY_SOURCE_H

/**
  @file storage/perfschema/table_ews_by_source.h
  Table EVENTS_WAITS_SUMMARY_BY_SOURCE (declarations).
*/

#include <sys/types.h>

#include "my_base.h"
#include "storage/perfschema/pfs_column_types.h"
#include "storage/perfschema/pfs_engine_table.h"
#include "storage/perfschema/pfs_latch_source.h"
#include "storage/perfschema/table_helper.h"

class Field;
class Plugin_table;
struct TABLE;
struct THR_LOCK;

/**
  @addtogroup performance_schema_tables
  @{
*/

/** A row of PERFORMANCE_SCHEMA.EVENTS_WAITS_SUMMARY_BY_SOURCE. */
struct row_ews_by_source {
  PFS_event_name_row m_event_name;
  char m_source[COL_SOURCE_SIZE];
  uint m_source_length;
  uint m_operation;
  PFS_stat_row m_wait;
  PFS_stat_row m_hold;
};

/** Table PERFORMANCE_SCHEMA.EVENTS_WAITS_SUMMARY_BY_SOURCE. */
class table_ews_by_source : public PFS_engine_table {
 public:
  static PFS_engine_table_share m_share;
  static PFS_engine_table *create(PFS_engine_table_share *);
  static int delete_all_rows();
  static ha_rows get_row_count();

  void reset_position() override;

  int rnd_next() override;
  int rnd_pos(const void *pos) override;

 protected:
  int read_row_values(TABLE *table, unsigned char *buf, Field **fields,
                      bool read_all) override;

  table_ews_by_source();

 public:
  ~table_ews_by_source() override = default;

 protected:
  int make_row(PFS_latch_source_stat *stat);

 private:
  static THR_LOCK m_table_lock;
  static Plugin_table m_table_def;

  row_ews_by_source m_row;
  PFS_simple_index m_pos;
  PFS_simple_index m_next_pos;
};

/** @} */
#endif
