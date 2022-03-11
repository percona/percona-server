/* Copyright (c) 2000, 2021, Oracle and/or its affiliates.

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
  @file
  Implementations of basic iterators, ie. those that have no children
  and don't take any refs (they typically read directly from a table
  in some way). See row_iterator.h.
*/

#include "sql/records.h"

#include <string.h>
#include <algorithm>
#include <atomic>
#include <new>

#include "my_base.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "sql/debug_sync.h"
#include "sql/handler.h"
#include "sql/item.h"
#include "sql/join_optimizer/access_path.h"
#include "sql/join_optimizer/bit_utils.h"
#include "sql/key.h"
#include "sql/opt_explain.h"
#include "sql/range_optimizer/range_optimizer.h"  // QUICK_SELECT_I
#include "sql/sql_class.h"                        // THD
#include "sql/sql_const.h"
#include "sql/sql_executor.h"
#include "sql/sql_optimizer.h"
#include "sql/sql_sort.h"
#include "sql/sql_tmp_table.h"
#include "sql/table.h"
#include "sql/timing_iterator.h"

using std::string;
using std::vector;

template <bool Reverse>
IndexScanIterator<Reverse>::IndexScanIterator(THD *thd, TABLE *table, int idx,
                                              bool use_order,
                                              double expected_rows,
                                              ha_rows *examined_rows)
    : TableRowIterator(thd, table),
      m_record(table->record[0]),
      m_idx(idx),
      m_use_order(use_order),
      m_expected_rows(expected_rows),
      m_examined_rows(examined_rows) {}

template <bool Reverse>
IndexScanIterator<Reverse>::~IndexScanIterator() {
  if (table() && table()->key_read) {
    table()->set_keyread(false);
  }
}

template <bool Reverse>
bool IndexScanIterator<Reverse>::Init() {
  if (!table()->file->inited) {
    if (table()->covering_keys.is_set(m_idx) && !table()->no_keyread) {
      table()->set_keyread(true);
    }

    int error = table()->file->ha_index_init(m_idx, m_use_order);
    if (error) {
      PrintError(error);
      return true;
    }

    if (set_record_buffer(table(), m_expected_rows)) {
      return true;
    }
  }
  m_first = true;
  return false;
}

// Doxygen gets confused by the explicit specializations.

//! @cond
template <>
int IndexScanIterator<false>::Read() {  // Forward read.
  int error;
  if (m_first) {
    error = table()->file->prepare_index_scan();
    if (error) return HandleError(error);
    error = table()->file->ha_index_first(m_record);
    m_first = false;
  } else {
    error = table()->file->ha_index_next(m_record);
  }
  if (error) return HandleError(error);
  if (m_examined_rows != nullptr) {
    ++*m_examined_rows;
  }
  return 0;
}

template <>
int IndexScanIterator<true>::Read() {  // Backward read.
  int error;
  if (m_first) {
    error = table()->file->prepare_index_scan();
    if (error) return HandleError(error);
    error = table()->file->ha_index_last(m_record);
    m_first = false;
  } else {
    error = table()->file->ha_index_prev(m_record);
  }
  if (error) return HandleError(error);
  if (m_examined_rows != nullptr) {
    ++*m_examined_rows;
  }
  return 0;
}
//! @endcond

template class IndexScanIterator<true>;
template class IndexScanIterator<false>;

/**
  create_table_access_path is used to scan by using a number of different
  methods. Which method to use is set-up in this call so that you can
  create an iterator from the returned access path and fetch rows through
  said iterator afterwards.

  @param thd      Thread handle
  @param table    Table the data [originally] comes from
  @param quick    QUICK_SELECT_I to scan the table with, or nullptr
  @param table_ref
                  Position for the table, must be non-nullptr for
                  WITH RECURSIVE
  @param position Place to get cost information from, or nullptr
  @param count_examined_rows
    See AccessPath::count_examined_rows.
 */
AccessPath *create_table_access_path(THD *thd, TABLE *table,
                                     QUICK_SELECT_I *quick,
                                     TABLE_LIST *table_ref, POSITION *position,
                                     bool count_examined_rows) {
  AccessPath *path;
  if (quick != nullptr) {
    path = NewIndexRangeScanAccessPath(thd, table, quick, count_examined_rows);
  } else if (table_ref != nullptr && table_ref->is_recursive_reference()) {
    path = NewFollowTailAccessPath(thd, table, count_examined_rows);
  } else {
    path = NewTableScanAccessPath(thd, table, count_examined_rows);
  }
  if (position != nullptr) {
    SetCostOnTableAccessPath(*thd->cost_model(), position,
                             /*is_after_filter=*/false, path);
  }
  return path;
}

unique_ptr_destroy_only<RowIterator> init_table_iterator(
    THD *thd, TABLE *table, QUICK_SELECT_I *quick, TABLE_LIST *table_ref,
    POSITION *position, bool ignore_not_found_rows, bool count_examined_rows) {
  unique_ptr_destroy_only<RowIterator> iterator;

  empty_record(table);

  if (table->unique_result.io_cache &&
      my_b_inited(table->unique_result.io_cache)) {
    DBUG_PRINT("info", ("using SortFileIndirectIterator"));
    iterator = NewIterator<SortFileIndirectIterator>(
        thd, Mem_root_array<TABLE *>{table}, table->unique_result.io_cache,
        ignore_not_found_rows, /*has_null_flags=*/false,
        /*examined_rows=*/nullptr);
    table->unique_result.io_cache =
        nullptr;  // Now owned by SortFileIndirectIterator.
  } else if (table->unique_result.has_result_in_memory()) {
    /*
      The Unique class never puts its results into table->sort's
      Filesort_buffer.
    */
    assert(!table->unique_result.sorted_result_in_fsbuf);
    DBUG_PRINT("info", ("using SortBufferIndirectIterator (unique)"));
    iterator = NewIterator<SortBufferIndirectIterator>(
        thd, Mem_root_array<TABLE *>{table}, &table->unique_result,
        ignore_not_found_rows, /*has_null_flags=*/false,
        /*examined_rows=*/nullptr);
  } else {
    AccessPath *path = create_table_access_path(thd, table, quick, table_ref,
                                                position, count_examined_rows);
    iterator = CreateIteratorFromAccessPath(thd, path,
                                            /*join=*/nullptr,
                                            /*eligible_for_batch_mode=*/false);
  }
  if (iterator->Init()) {
    return nullptr;
  }
  return iterator;
}

/**
  The default implementation of unlock-row method of RowIterator,
  used in all access methods except EQRefIterator.
*/
void TableRowIterator::UnlockRow() { m_table->file->unlock_row(); }

void TableRowIterator::SetNullRowFlag(bool is_null_row) {
  if (is_null_row) {
    m_table->set_null_row();
  } else {
    m_table->reset_null_row();
  }
}

int TableRowIterator::HandleError(int error) {
  if (thd()->killed) {
    thd()->send_kill_message();
    return 1;
  }

  if (error == HA_ERR_END_OF_FILE || error == HA_ERR_KEY_NOT_FOUND) {
    m_table->set_no_row();
    return -1;
  } else {
    PrintError(error);
    return 1;
  }
}

void TableRowIterator::PrintError(int error) {
  m_table->file->print_error(error, MYF(0));
}

void TableRowIterator::StartPSIBatchMode() {
  m_table->file->start_psi_batch_mode();
}

void TableRowIterator::EndPSIBatchModeIfStarted() {
  m_table->file->end_psi_batch_mode_if_started();
}

IndexRangeScanIterator::IndexRangeScanIterator(THD *thd, TABLE *table,
                                               QUICK_SELECT_I *quick,
                                               double expected_rows,
                                               ha_rows *examined_rows)
    : TableRowIterator(thd, table),
      m_quick(quick),
      m_expected_rows(expected_rows),
      m_examined_rows(examined_rows) {}

/// Is this range scan a Rowid-Ordered Retrieval (ROR) index merge?
static bool is_ror(const QUICK_SELECT_I *quick) {
  switch (quick->get_type()) {
    case QS_TYPE_ROR_INTERSECT:
    case QS_TYPE_ROR_UNION:
      return true;
    default:
      return false;
  }
}

/// Does this TABLE have a primary key with a BLOB component?
static bool has_blob_primary_key(const TABLE *table) {
  if (table->s->is_missing_primary_key()) {
    return false;
  }

  const KEY &key = table->key_info[table->s->primary_key];
  return std::any_of(key.key_part, key.key_part + key.user_defined_key_parts,
                     [](const KEY_PART_INFO &key_part) {
                       return Overlaps(key_part.key_part_flag, HA_BLOB_PART);
                     });
}

/// Should a record buffer be requested for this range scan?
static bool should_request_record_buffer(const QUICK_SELECT_I *quick) {
  // We don't try to set up record buffers for loose index scans, because they
  // usually cannot read expected_rows_to_fetch rows in one go anyway.
  if (quick->is_loose_index_scan()) {
    return false;
  }

  // Rowid-ordered retrievals may add the primary key to the read_set at a later
  // stage. If the primary key contains a BLOB component, a record buffer cannot
  // be used, since BLOBs require storage space outside of the record. So don't
  // request a buffer in this case, even though the current read_set gives the
  // impression that using a record buffer would be fine.
  if (is_ror(quick) &&
      Overlaps(quick->m_table->file->ha_table_flags(),
               HA_PRIMARY_KEY_REQUIRED_FOR_POSITION) &&
      has_blob_primary_key(quick->m_table)) {
    return false;
  }

  // Otherwise, request a row buffer.
  return true;
}

bool IndexRangeScanIterator::Init() {
  empty_record(table());

  /*
    Only attempt to allocate a record buffer the first time the handler is
    initialized.
  */
  const bool first_init = !table()->file->inited;

  int error = m_quick->reset();
  if (error) {
    // Ensures error status is propagated back to client.
    (void)report_handler_error(table(), error);
    return true;
  }

  if (first_init && table()->file->inited &&
      should_request_record_buffer(m_quick)) {
    if (set_record_buffer(table(), m_expected_rows)) {
      return true; /* purecov: inspected */
    }
  }

  m_seen_eof = false;
  return false;
}

int IndexRangeScanIterator::Read() {
  if (m_seen_eof) {
    return -1;
  }

  int tmp;
  while ((tmp = m_quick->get_next())) {
    if (thd()->killed || (tmp != HA_ERR_RECORD_DELETED)) {
      int error_code = HandleError(tmp);
      if (error_code == -1) {
        m_seen_eof = true;
      }
      return error_code;
    }
  }

  if (m_examined_rows != nullptr) {
    ++*m_examined_rows;
  }
  return 0;
}

TableScanIterator::TableScanIterator(THD *thd, TABLE *table,
                                     double expected_rows,
                                     ha_rows *examined_rows)
    : TableRowIterator(thd, table),
      m_record(table->record[0]),
      m_expected_rows(expected_rows),
      m_examined_rows(examined_rows) {}

TableScanIterator::~TableScanIterator() {
  if (table()->file != nullptr) {
    table()->file->ha_index_or_rnd_end();
  }
}

bool TableScanIterator::Init() {
  empty_record(table());

  /*
    Only attempt to allocate a record buffer the first time the handler is
    initialized.
  */
  const bool first_init = !table()->file->inited;

  int error = table()->file->ha_rnd_init(true);
  if (error) {
    PrintError(error);
    return true;
  }

  if (first_init && set_record_buffer(table(), m_expected_rows)) {
    return true; /* purecov: inspected */
  }

  return false;
}

int TableScanIterator::Read() {
  int tmp;
  while ((tmp = table()->file->ha_rnd_next(m_record))) {
    /*
      ha_rnd_next can return RECORD_DELETED for MyISAM when one thread is
      reading and another deleting without locks.
    */
    if (tmp == HA_ERR_RECORD_DELETED && !thd()->killed) continue;
    return HandleError(tmp);
  }
  if (m_examined_rows != nullptr) {
    ++*m_examined_rows;
  }
  return 0;
}

FollowTailIterator::FollowTailIterator(THD *thd, TABLE *table,
                                       double expected_rows,
                                       ha_rows *examined_rows)
    : TableRowIterator(thd, table),
      m_record(table->record[0]),
      m_expected_rows(expected_rows),
      m_examined_rows(examined_rows) {}

FollowTailIterator::~FollowTailIterator() {
  if (table()->file != nullptr) {
    table()->file->ha_index_or_rnd_end();
  }
}

bool FollowTailIterator::Init() {
  empty_record(table());

  // BeginMaterialization() must be called before this.
  assert(m_stored_rows != nullptr);

  /*
    Only attempt to allocate a record buffer the first time the handler is
    initialized.
  */
  const bool first_init = !table()->file->inited;

  if (first_init) {
    // Before starting a new WITH RECURSIVE execution,
    // MaterializeIterator::Init() does ha_index_or_rnd_end() on all read
    // cursors of recursive members, which sets file->inited = false, so we can
    // use that as a signal.
    if (!table()->is_created()) {
      // Recursive references always refer to a temporary table,
      // which do not exist at resolution time; thus, we need to
      // connect to it on first run here.
      assert(table()->in_use == nullptr || table()->in_use == thd());
      table()->in_use = thd();
      if (open_tmp_table(table())) {
        return true;
      }
    }

    int error = table()->file->ha_rnd_init(true);
    if (error) {
      PrintError(error);
      return true;
    }

    if (first_init && set_record_buffer(table(), m_expected_rows)) {
      return true; /* purecov: inspected */
    }

    // The first seen record will start a new iteration.
    m_read_rows = 0;
    m_recursive_iteration_count = 0;
    m_end_of_current_iteration = 0;
  } else {
    // Just continue where we left off last time.
  }

  m_inited = true;
  return false;
}

int FollowTailIterator::Read() {
  if (m_read_rows == *m_stored_rows) {
    /*
      Return EOF without even checking if there are more rows
      (there isn't), so that we can continue reading when there are.
      There are two underlying reasons why we need to do this,
      depending on the storage engine in use:

      1. For both MEMORY and InnoDB, when they report EOF,
         the scan stays blocked at EOF forever even if new rows
         are inserted later. (InnoDB has a supremum record, and
         MEMORY increments info->current_record unconditionally.)

      2. Specific to MEMORY, inserting records that are deduplicated
         away can corrupt cursors that hit EOF. Consider the following
         scenario:

         - write 'A'
         - write 'A': allocates a record, hits a duplicate key error, leaves
           the allocated place as "deleted record".
         - init scan
         - read: finds 'A' at #0
         - read: finds deleted record at #1, properly skips over it, moves to
           EOF
         - even if we save the read position at this point, it's "after #1"
         - close scan
         - write 'B': takes the place of deleted record, i.e. writes at #1
         - write 'C': writes at #2
         - init scan, reposition at saved position
         - read: still after #1, so misses 'B'.

         In this scenario, the table is formed of real records followed by
         deleted records and then EOF.

       To avoid these problems, we keep track of the number of rows in the
       table by holding the m_stored_rows pointer into the MaterializeIterator,
       and simply avoid hitting EOF.
     */
    return -1;
  }

  if (m_read_rows == m_end_of_current_iteration) {
    // We have started a new iteration. Check to see if we have passed the
    // user-set limit.
    if (++m_recursive_iteration_count >
        thd()->variables.cte_max_recursion_depth) {
      my_error(ER_CTE_MAX_RECURSION_DEPTH, MYF(0), m_recursive_iteration_count);
      return 1;
    }
    m_end_of_current_iteration = *m_stored_rows;

#ifdef ENABLED_DEBUG_SYNC
    if (m_recursive_iteration_count == 4) {
      DEBUG_SYNC(thd(), "in_WITH_RECURSIVE");
    }
#endif
  }

  // Read the actual row.
  //
  // We can never have MyISAM here, so we don't need the checks
  // for HA_ERR_RECORD_DELETED that TableScanIterator has.
  int err = table()->file->ha_rnd_next(m_record);
  if (err) {
    return HandleError(err);
  }

  ++m_read_rows;

  if (m_examined_rows != nullptr) {
    ++*m_examined_rows;
  }
  return 0;
}

bool FollowTailIterator::RepositionCursorAfterSpillToDisk() {
  if (!m_inited) {
    // Spill-to-disk happened before we got to read a single row,
    // so the table has not been initialized yet. It will start
    // at the first row when we actually get to Init(), which is fine.
    return false;
  }
  return reposition_innodb_cursor(table(), m_read_rows);
}
