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

/** @file vec/vec0label.cc
The hidden label column percona_vec_aux_id and its per-table counter. */

#include "vec0label.h"

#include <debug_sync.h>

#include "current_thd.h"
#include "data0data.h"
#include "dict0dd.h"
#include "dict0dict.h"
#include "dict0mem.h"
#include "mach0data.h"
#include "mem0mem.h"
#include "mtr0mtr.h"
#include "rem0rec.h"
#include "row0upd.h"
#include "univ.i"
#include "vec0aux.h"

uint64_t Vec_label_counter::assign(dict_table_t *table) {
  ut_ad(table != nullptr);
  ut_ad(DICT_TF2_FLAG_IS_SET(table, DICT_TF2_HAS_VEC_AUX_COL));
  /* fetch_add returns the OLD value, so +1 makes the first assignment
  1 and never 0. That is not cosmetic: the class reserves graph node id
  0 as the empty-neighbour sentinel, which is what lets aux record 0
  hold the entry point instead of a node. */
  const uint64_t id =
      table->vec_aux_autoinc_next_id.fetch_add(1, std::memory_order_acq_rel) +
      1;
  ut_ad(id != 0);

  /* Persist the advance as dynamic metadata, autoinc-style: the id is
  written to DDTableBuffer the moment it is consumed, so a label can never
  be reissued - not across restart, not across crash, and whether or not
  the id ever reaches the aux table. Rolled-back inserts consume ids that
  the aux maximum cannot see, which is why the aux cannot be the source of
  truth for this. */
  persist_counter(table, id);

  /* `id` is in DDTableBuffer, so a crash landing here cannot hand it out
  again. Parking a test here is how percona_vector.counter_stale_buffer
  pins that. */
  DEBUG_SYNC_C("vec_id_record_committed");

  return id;
}

void Vec_label_counter::raise_watermark(dict_table_t *table, uint64_t value) {
  uint64_t prev = table->vec_aux_autoinc_persisted.load();
  while (prev < value &&
         !table->vec_aux_autoinc_persisted.compare_exchange_weak(prev, value)) {
  }
}

void Vec_label_counter::persist_counter(dict_table_t *table, uint64_t value) {
  /* Raise the watermark FIRST, then mark the table dirty and write it to
  DDTableBuffer. Any write of the table's metadata that starts after the
  raise - ours, or a concurrent one from another assigner, eviction or
  dict_persist_to_dd_table_buffer() - reads a watermark that covers
  `value`. Ours always runs: dict_table_mark_dirty() makes the table
  METADATA_DIRTY, so dict_table_persist_to_dd_table_buffer() either
  writes it, or finds that a writer that took dict_persist->mutex after
  the mark - and so after the raise - has already done so.

  DEVIATION FROM dict_table_autoinc_persist: no preallocation and no fast
  path. Autoinc skips the write while a value preallocated ahead of it
  (innodb_autoinc_preallocate) covers it, and writes the exact value back
  at shutdown. Here every assignment is written, so DDTableBuffer always
  holds the exact counter and no label is burnt across restarts; the write
  is small next to the graph insert that follows it.

  DEVIATION FROM the autoinc watermark: no per-table mutex. The autoinc
  watermark needs autoinc_persisted_mutex because the slow path lowers it
  back to the exact value; ours only ever grows, so a CAS-max suffices. */
  raise_watermark(table, value);

  dict_table_mark_dirty(table);
  dict_table_persist_to_dd_table_buffer(table);
}

void Vec_label_counter::write_to_dd(dd::Properties &se_private_data,
                                    uint64_t next_id) {
  if (next_id == 0) return;
  se_private_data.set(dd_table_key_strings[DD_TABLE_VEC_NEXT_ID], next_id);
}

uint64_t Vec_label_counter::read_from_dd(
    const dd::Properties &se_private_data) {
  uint64_t next_id = 0;
  if (se_private_data.exists(dd_table_key_strings[DD_TABLE_VEC_NEXT_ID])) {
    se_private_data.get(dd_table_key_strings[DD_TABLE_VEC_NEXT_ID], &next_id);
  }
  return next_id;
}

void Vec_label_counter::load_from_dd(dict_table_t *table,
                                     const dd::Properties &se_private_data) {
  /* Restore the label counter from the definition. The buffered dynamic
  metadata is applied later and takes the maximum, so a value there that
  is further ahead still wins; this only ensures the counter survives an
  ALTER that invalidated that buffer. */
  if (DICT_TF2_FLAG_IS_SET(table, DICT_TF2_HAS_VEC_AUX_COL)) {
    const uint64_t next_id = read_from_dd(se_private_data);
    if (next_id > table->vec_aux_autoinc_next_id.load()) {
      table->vec_aux_autoinc_next_id.store(next_id);
      table->vec_aux_autoinc_persisted.store(next_id);
    }
  }
}

void vec_label_write(dict_table_t *table, dtuple_t *row, byte *buf) {
  ut_ad(table != nullptr);
  ut_ad(row != nullptr);
  ut_ad(buf != nullptr);
  if (!DICT_TF2_FLAG_IS_SET(table, DICT_TF2_HAS_VEC_AUX_COL)) {
    return;
  }
  ut_ad(table->vec_aux_col != ULINT_UNDEFINED);
  ut_ad(table->vec_aux_col < dtuple_get_n_fields(row));

  const uint64_t id = Vec_label_counter::assign(table);
  mach_write_to_8(buf, id);

  dfield_t *dfield = dtuple_get_nth_field(row, table->vec_aux_col);
  dfield_set_data(dfield, buf, VEC_AUX_ID_LEN);
}

uint64_t vec_label_from_dtuple(const dict_table_t *table, const dtuple_t *row) {
  ut_ad(table->vec_aux_col != ULINT_UNDEFINED);
  ut_ad(table->vec_aux_col < dtuple_get_n_fields(row));

  const dfield_t *df = dtuple_get_nth_field(row, table->vec_aux_col);
  ut_ad(!dfield_is_null(df));
  ut_ad(dfield_get_len(df) == 8);
  return mach_read_from_8(static_cast<const byte *>(dfield_get_data(df)));
}

uint64_t vec_label_from_rec(const dict_table_t *table, const rec_t *rec,
                            const dict_index_t *index) {
  ut_ad(table->vec_aux_col != ULINT_UNDEFINED);

  ulint offsets_[REC_OFFS_NORMAL_SIZE];
  ulint *offsets = offsets_;
  mem_heap_t *heap = nullptr;

  rec_offs_init(offsets_);
  offsets = rec_get_offsets(rec, index, offsets, ULINT_UNDEFINED,
                            UT_LOCATION_HERE, &heap);

  const ulint pos = index->get_col_pos(table->vec_aux_col);
  ut_ad(pos != ULINT_UNDEFINED);

  ulint len;
  const byte *data = rec_get_nth_field(nullptr, rec, offsets, pos, &len);
  ut_ad(len == 8);
  const uint64_t label = mach_read_from_8(data);

  if (heap != nullptr) {
    mem_heap_free(heap);
  }
  return label;
}
