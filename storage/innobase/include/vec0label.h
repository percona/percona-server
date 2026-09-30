/*****************************************************************************

Copyright (c) 2026, Percona Inc.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is designed to work with certain software (including
but not limited to OpenSSL) that is licensed under separate terms,
as designated in a particular file or component or in included license
documentation.  The authors of MySQL hereby grant you an additional
permission to link the program and your derivative works with the
separately licensed software that they have either included with
the program or referenced in the documentation.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/

/** @file include/vec0label.h
The hidden label column percona_vec_aux_id of a vector-indexed table, and
its per-table counter.

Vec_label_counter owns the counter: assigning labels, redo-logging each
advance, and carrying the value through the table definition. The counter
state itself lives in dict_table_t (vec_aux_autoinc_next_id and
vec_aux_autoinc_persisted), like autoinc. The free functions below read
and write the column in rows and records. */

#ifndef vec0label_h
#define vec0label_h

#include "data0types.h"
#include "dict0types.h"
#include "mtr0types.h"
#include "rem0types.h"
#include "row0types.h"
#include "univ.i"

namespace dd {
class Properties;
}

/** The per-table counter behind percona_vec_aux_id. */
class Vec_label_counter {
 public:
  /** Atomically assign the next percona_vec_aux_id for a row about to be
  inserted. Valid ids start at 1. Written into the hidden percona_vec_aux_id
  dfield by the INSERT path. See the implementation comment for the phase-1
  persistence caveat. */
  static uint64_t assign(dict_table_t *table);

  /** Write redo logs for the hidden vec_idx_id counter of a
  vector-indexed table when it advances past the persisted watermark -
  the dict_table_autoinc_log analog.
  @param[in,out]  table   table whose counter advanced
  @param[in]      value   counter value AFTER the assignment
  @param[in,out]  mtr     mini-transaction carrying the redo record
  @return true if the change should be persisted to the DD buffer table
  immediately (mirror of dict_table_autoinc_log's contract). */
  static bool log(dict_table_t *table, uint64_t value, mtr_t *mtr);

  /** Store the next percona_vec_aux_id label in a table definition.
  @param[in,out]  se_private_data  dd::Table::se_private_data
  @param[in]      next_id          the counter; 0 stores nothing */
  static void write_to_dd(dd::Properties &se_private_data, uint64_t next_id);

  /** Read the next percona_vec_aux_id label from a table definition.
  @param[in]  se_private_data  dd::Table::se_private_data
  @return the counter, or 0 if the definition predates it */
  static uint64_t read_from_dd(const dd::Properties &se_private_data);

  /** Restore the label counter of a table being opened from its
  definition. No-op for tables without the hidden column.
  @param[in,out]  table            the table being opened
  @param[in]      se_private_data  its dd::Table::se_private_data */
  static void load_from_dd(dict_table_t *table,
                           const dd::Properties &se_private_data);

 private:
  /** Raise the persisted watermark of a table's vector label counter to
  value, never lowering it: a CAS-max, since racing assigners raise it
  concurrently. log() calls it before writing the record, and says why
  that order is the safe one.
  @param[in,out]  table  the table
  @param[in]      value  the id about to be logged */
  static void raise_watermark(dict_table_t *table, uint64_t value);
};

/** Write the hidden percona_vec_aux_id dfield in `row` with the next id from
the per-table counter. No-op for tables without the hidden column.
Allocations come from `heap` so they outlive this call. */
void vec_label_write(dict_table_t *table, dtuple_t *row, byte *buf);

/** Read the label written into a row's hidden percona_vec_aux_id column.
@param[in]  table  the base table
@param[in]  row    the row, as converted for InnoDB
@return the label; never 0 for a written row */
uint64_t vec_label_from_dtuple(const dict_table_t *table, const dtuple_t *row);

/** Read the label written into a clustered index record.

Reading straight from the record is what lets the label be checked without
adding the hidden column to the MySQL row template.
@param[in]  table  the base table
@param[in]  rec    a record of `index` containing percona_vec_aux_id
@param[in]  index  the index `rec` belongs to
@return the label; never 0 for a written row */
uint64_t vec_label_from_rec(const dict_table_t *table, const rec_t *rec,
                            const dict_index_t *index);

/** Fill an update field so it sets the hidden label column to `label`.
@param[in]      table   the base table
@param[in,out]  ufield  the update field to fill
@param[in]      label   the new label */
void vec_label_update(dict_table_t *table, upd_field_t *ufield,
                      uint64_t *next_label);

#endif /* vec0label_h */
