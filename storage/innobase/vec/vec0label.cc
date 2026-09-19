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

  /* Persist the advance as dynamic metadata, autoinc-style: the redo
  record makes the id durable the moment it is consumed, so a label can
  never be reissued - not across restart, not across crash, and whether
  or not the id ever reaches the aux table. Rolled-back inserts consume
  ids that the aux maximum cannot see, which is why the aux cannot be
  the source of truth for this.

  The write runs outside any active mini-transaction, so it gets a
  dedicated one. Upstream avoids that by logging into the row's own mtr
  (WL#6204: "we should not introduce a new mtr ... mtr_commit would be
  time consuming"), which we could do from row_ins_clust_index_entry_low -
  at the price of covering the paths that never reach it, the DDL builder
  among them. Logging where the id is assigned covers every one of them. */
  mtr_t mtr;
  mtr.start();
  const bool persist = log(table, id, &mtr);
  mtr.commit();

  /* The record for `id` is committed to the log and the watermark was
  raised before it was written, so a checkpoint landing here sees a
  watermark that already covers the record. Parking a test here is how
  vector_counter_stale_buffer.test pins that. */
  DEBUG_SYNC_C("vec_id_record_committed");

  if (persist) {
    dict_table_persist_to_dd_table_buffer(table);
  }

  return id;
}

void Vec_label_counter::raise_watermark(dict_table_t *table, uint64_t value) {
  uint64_t prev = table->vec_aux_autoinc_persisted.load();
  while (prev < value &&
         !table->vec_aux_autoinc_persisted.compare_exchange_weak(prev, value)) {
  }
}

bool Vec_label_counter::log(dict_table_t *table, uint64_t value, mtr_t *mtr) {
  /* Raise the watermark FIRST, before the dirty handshake below. That
  order is the whole interlock with the checkpoint, and it is upstream's:
  dict_table_autoinc_log raises, then marks dirty, then logs.

  Whatever the thread then reads from dirty_status, the record it is about
  to write is accounted for:

    reads METADATA_DIRTY     the read beat the checkpoint's store of
                             METADATA_BUFFERED, so the raise - earlier in
                             program order - beat the checkpoint's read of
                             the watermark, and the buffer already has it.

    reads METADATA_BUFFERED  dict_table_mark_dirty blocks on
                             dict_persist->mutex until the checkpoint has
                             taken its LSN, so this record lands above the
                             cap and recovery scans it.

  There is no third case. Raising after the record instead leaves both
  branches meaningless: a
  checkpoint in between writes the OLD value to DDTableBuffer, clears the
  dirty flag, and moves the checkpoint past a record nothing will read
  again. vector_counter_stale_buffer.test is that window.

  DEVIATION FROM dict_table_autoinc_log: no per-table mutex. The autoinc
  watermark is a plain integer whose read-compare-store max needs
  autoinc_persisted_mutex; ours is a std::atomic and a CAS-max gives the
  same never-regress guarantee. The interlock above needs only that the
  raise precede the dirty read in program order, which it does.

  And no skip test. Upstream logs only when the value exceeds the
  watermark (WL#6204: "We only write logs when counter is 0 or is bigger
  than table::autoinc_persisted"), which saves redo but lets a racing
  assigner with a smaller value write nothing at all, trusting a record
  that may never be written - see PS-autoinc-persist-crash-window.md. We
  always log. The mtr the caller hands us is committed either way, so the
  cost is one record in an mtr that was being committed anyway. */
  raise_watermark(table, value);

  if (table->dirty_status.load() != METADATA_DIRTY) {
    dict_table_mark_dirty(table);
  }
  ut_ad(table->in_dirty_dict_tables_list);

  PersistentTableMetadata metadata(table->id, table->version);
  metadata.set_vec_next_id(value);

  Persister *persister = dict_persist->persisters->get(PM_TABLE_VEC_IDX_ID);
  persister->write_log(table->id, metadata, mtr);
  /* No need to flush due to performance reason */

  return (dict_persist->check_persist_immediately());
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
