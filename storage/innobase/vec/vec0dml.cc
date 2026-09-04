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

/** @file vec/vec0dml.cc
Parser-free DML on vector-index auxiliary tables. See vec0dml.h for the
DEVIATION FROM FTS rationale (no fts_parse_sql / pars_mutex). */

#include "vec0dml.h"

#include <algorithm>
#include <limits>

#include "btr0pcur.h"
#include "dict0dict.h"
#include "lob0lob.h"
#include "mach0data.h"
#include "pars0pars.h"
#include "que0que.h"
#include "read0types.h"
#include "row0ins.h"
#include "row0mysql.h"
#include "row0upd.h"
#include "row0vers.h"
#include "trx0roll.h"
#include "vec0aux.h"

/* Aux table user-column ordinals, fixed by create_in_mem_vec_aux_table
(vec0aux.cc): id, vec, base_pk, level, neighbors. */
constexpr ulint VEC_AUX_COL_ID = 0;
constexpr ulint VEC_AUX_COL_VEC = 1;
constexpr ulint VEC_AUX_COL_BASE_PK = 2;
constexpr ulint VEC_AUX_COL_LEVEL = 3;
constexpr ulint VEC_AUX_COL_NEIGHBORS = 4;

/* Neighbour slots serialize as a flat big-endian array of ids, one per
slot, with 0 for an empty slot. No header: the class reserves graph node
id 0 as the empty-slot sentinel precisely so a persistor "need not store
a per-layer neighbor count" (hnsw.h). The slot count is (level + 2) * M,
recoverable at load time from the level stored in the row and the M
stored with the index, so writing it again would be redundant and would
be a second copy of the truth to keep consistent. */

ulint vec_aux_neighbors_blob_len(uint8_t level, uint32_t m) {
  return (static_cast<ulint>(level) + 2) * m * 8;
}

/** Fill one user dfield of the aux row tuple with a heap-duplicated
value (the run loop may retry after lock waits; values must be stable). */
static void vec_aux_set_field(dtuple_t *tuple, ulint col_no, const void *data,
                              ulint len, mem_heap_t *heap) {
  dfield_t *df = dtuple_get_nth_field(tuple, col_no);

  /* Every column of the aux table is NOT NULL, so there is no SQL NULL
  case to handle here — and mapping a zero-length value onto NULL would
  be wrong rather than merely unused: a node with no neighbours yet has
  an EMPTY neighbour blob, not a missing one, and handing SQL_NULL to a
  NOT NULL column trips rec_get_converted_size_comp_prefix_low. Length 0
  still needs a non-null data pointer for dfield_set_data. */
  if (len == 0) {
    static const byte empty = 0;
    dfield_set_data(df, &empty, 0);
    return;
  }
  ut_a(data != nullptr);
  void *copy = mem_heap_dup(heap, data, len);
  dfield_set_data(df, copy, len);
}

dberr_t vec_aux_insert(trx_t *trx, dict_table_t *aux,
                       const vec_aux_row_t &row) {
  ut_a(trx != nullptr);
  ut_a(aux != nullptr);
  /* Record 0 is index metadata, not a node: it names the graph's entry
  point and legitimately carries no vector and no neighbours. Every real
  node has both — id 0 is reserved as the empty-slot sentinel, so a node
  can never occupy record 0. */
  ut_a(row.vec != nullptr || (row.id == 0 && row.dims == 0));
  ut_a(row.neighbors != nullptr || row.neighbors_len == 0);

  /* The aux column is TINYINT; the HNSW level is geometrically
  distributed and cannot plausibly reach 127, but never store a
  truncated level. */
  if (row.level < 0 || row.level > 127) {
    return DB_CORRUPTION;
  }

  mem_heap_t *heap = mem_heap_create(1024, UT_LOCATION_HERE);

  /* Mirror of row_get_prebuilt_insert_row + row_insert_for_mysql's run
  loop (row0mysql.cc), minus the prebuilt: build an INS_DIRECT node on a
  private heap, complete a query graph for it (pars_complete_graph_for_
  exec builds the fork/thr only — no SQL parser involved), fill the row,
  and drive row_ins_step with the standard error handling. */
  ins_node_t *node = ins_node_create(INS_DIRECT, aux, heap);

  dtuple_t *tuple = dtuple_create(heap, aux->get_n_cols());
  dict_table_copy_types(tuple, aux);
  ins_node_set_new_row(node, tuple);

  byte id_buf[8];
  mach_write_to_8(id_buf, row.id);
  vec_aux_set_field(tuple, VEC_AUX_COL_ID, id_buf, sizeof(id_buf), heap);
  vec_aux_set_field(tuple, VEC_AUX_COL_VEC, row.vec, row.dims * sizeof(float),
                    heap);
  byte base_pk_buf[8];
  mach_write_to_8(base_pk_buf, row.base_pk);
  vec_aux_set_field(tuple, VEC_AUX_COL_BASE_PK, base_pk_buf,
                    sizeof(base_pk_buf), heap);
  const byte level_byte = static_cast<byte>(row.level);
  vec_aux_set_field(tuple, VEC_AUX_COL_LEVEL, &level_byte, 1, heap);
  vec_aux_set_field(tuple, VEC_AUX_COL_NEIGHBORS, row.neighbors,
                    row.neighbors_len, heap);

  que_thr_t *thr = pars_complete_graph_for_exec(node, trx, heap, nullptr);
  /* Activate the fork, as every other MySQL-interface caller does
  (row0mysql.cc does it for ins_graph, sel_graph and upd_graph).
  pars_complete_graph_for_exec leaves the fork QUE_FORK_COMMAND_WAIT, and
  that is the first thing que_thr_stop() tests (que0que.cc), so it
  reports "stop this thread" — which RecLock::prepare treats as impossible
  and answers with ut_error (lock0lock.cc).

  A lock that is granted immediately never enqueues and never reaches that
  test, which is why this was invisible for as long as graph mutation was
  serialised: no two sub-transactions could contend for the same aux row. */
  thr->graph->state = QUE_FORK_ACTIVE;

  auto savept = trx_savept_take(trx);

  que_thr_move_to_run_state_for_mysql(thr, trx);

  /* Each call is its own mini-statement on the trx: take the IX table
  lock explicitly (cheap when already held by an earlier row of the same
  statement). */
  node->state = INS_NODE_SET_IX_LOCK;

  dberr_t err;
  for (;;) {
    thr->run_node = node;
    thr->prev_node = node;

    row_ins_step(thr);

    err = trx->error_state;
    if (err == DB_SUCCESS) {
      break;
    }

    que_thr_stop_for_mysql(thr);
    thr->lock_state = QUE_THR_LOCK_ROW;
    const bool was_lock_wait = row_mysql_handle_errors(&err, trx, thr, &savept);
    thr->lock_state = QUE_THR_LOCK_NOLOCK;

    if (!was_lock_wait) {
      mem_heap_free(heap);
      return err;
    }
    ut_ad(node->state == INS_NODE_INSERT_ENTRIES ||
          node->state == INS_NODE_ALLOC_ROW_ID);
  }

  que_thr_stop_for_mysql_no_error(thr, trx);
  mem_heap_free(heap);
  return DB_SUCCESS;
}

dberr_t vec_aux_update_row(trx_t *trx, dict_table_t *aux, uint64_t id,
                           const byte *neighbors, ulint neighbors_len,
                           const uint64_t *new_base_pk) {
  ut_a(trx != nullptr);
  ut_a(aux != nullptr);
  ut_a(neighbors != nullptr || neighbors_len == 0);

  mem_heap_t *heap = mem_heap_create(1024, UT_LOCATION_HERE);
  dict_index_t *clust = aux->first_index();

  /* Standard update machinery — the same upd_node + row_upd_step every
  SQL UPDATE runs on. In a regular UPDATE the preceding row_search_mvcc
  read positions the cursor and takes the locks; we know the PK and
  skip the search, so we position and lock ourselves below.
  (Self-positioned-upd_node implementation reference: the FK-cascade
  code, row0ins.cc; its run loop touches thr->prebuilt, which we
  don't have — hence a private loop.) */
  upd_node_t *node = row_create_update_node_for_mysql(aux, heap);

  /* Search tuple for the target row's PK. */
  dtuple_t *ref = dtuple_create(heap, 1);
  dict_index_copy_types(ref, clust, 1);
  byte id_buf[8];
  mach_write_to_8(id_buf, id);
  dfield_set_data(dtuple_get_nth_field(ref, 0), id_buf, sizeof(id_buf));

  que_thr_t *thr = pars_complete_graph_for_exec(node, trx, heap, nullptr);
  /* Activate the fork, as every other MySQL-interface caller does
  (row0mysql.cc does it for ins_graph, sel_graph and upd_graph).
  pars_complete_graph_for_exec leaves the fork QUE_FORK_COMMAND_WAIT, and
  that is the first thing que_thr_stop() tests (que0que.cc), so it
  reports "stop this thread" — which RecLock::prepare treats as impossible
  and answers with ut_error (lock0lock.cc).

  A lock that is granted immediately never enqueues and never reaches that
  test, which is why this was invisible for as long as graph mutation was
  serialised: no two sub-transactions could contend for the same aux row. */
  thr->graph->state = QUE_FORK_ACTIVE;

  auto savept = trx_savept_take(trx);

  que_thr_move_to_run_state_for_mysql(thr, trx);

  /* Take the locks row_search_mvcc would have taken for a regular
  UPDATE: IX on the table, explicit X on the record (row_upd_clust_step
  asserts both via lock_trx_has_rec_x_lock). Position the cursor, take
  the locks, retry on lock waits with the standard
  row_mysql_handle_errors machinery. */
  mtr_t mtr;
  mem_heap_t *offset_heap = nullptr;
  for (;;) {
    mtr_start(&mtr);
    node->pcur->open_no_init(clust, ref, PAGE_CUR_LE, BTR_SEARCH_LEAF, 0, &mtr,
                             UT_LOCATION_HERE);
    const rec_t *rec = node->pcur->get_rec();
    if (!page_rec_is_user_rec(rec) ||
        node->pcur->get_low_match() < dict_index_get_n_unique(clust)) {
      mtr_commit(&mtr);
      que_thr_stop_for_mysql_no_error(thr, trx);
      if (offset_heap != nullptr) {
        mem_heap_free(offset_heap);
      }
      mem_heap_free(heap);
      return DB_RECORD_NOT_FOUND;
    }

    dberr_t lerr = lock_table(0, aux, LOCK_IX, thr);
    if (lerr == DB_SUCCESS) {
      ulint *offsets = rec_get_offsets(rec, clust, nullptr, ULINT_UNDEFINED,
                                       UT_LOCATION_HERE, &offset_heap);
      /* Not lock_clust_rec_modify_check_and_lock: that one uses
      lock_rec_lock(impl=true), which creates NO explicit lock when
      uncontended (the caller is expected to modify the record in the
      same mtr, making the lock implicit via the new trx id). We modify
      in a LATER mtr, so we need the explicit X lock a SELECT ... FOR
      UPDATE would take. */
      lerr = lock_clust_rec_read_check_and_lock(
          lock_duration_t::REGULAR, node->pcur->get_block(), rec, clust,
          offsets, SELECT_ORDINARY, LOCK_X, LOCK_REC_NOT_GAP, thr);
      if (lerr == DB_SUCCESS_LOCKED_REC) {
        lerr = DB_SUCCESS;
      }
    }

    if (lerr == DB_SUCCESS) {
      node->pcur->store_position(&mtr);
      mtr_commit(&mtr);
      break;
    }

    mtr_commit(&mtr);
    trx->error_state = lerr;
    que_thr_stop_for_mysql(thr);
    thr->lock_state = QUE_THR_LOCK_ROW;
    const bool was_lock_wait =
        row_mysql_handle_errors(&lerr, trx, thr, &savept);
    thr->lock_state = QUE_THR_LOCK_NOLOCK;
    if (!was_lock_wait) {
      if (offset_heap != nullptr) {
        mem_heap_free(offset_heap);
      }
      mem_heap_free(heap);
      return lerr;
    }
  }
  if (offset_heap != nullptr) {
    mem_heap_free(offset_heap);
  }

  /* Single-field (or two-field, with the tombstone) update vector. */
  upd_t *update = upd_create(2, heap);
  update->table = aux;
  ulint n_fields = 0;

  {
    upd_field_t *uf = upd_get_nth_field(update, n_fields++);
    const dict_col_t *col = aux->get_col(VEC_AUX_COL_NEIGHBORS);
    upd_field_set_field_no(uf, dict_col_get_clust_pos(col, clust), clust);
    void *copy = neighbors_len != 0
                     ? mem_heap_dup(heap, neighbors, neighbors_len)
                     : nullptr;
    dfield_set_data(&uf->new_val, copy, neighbors_len);
    col->copy_type(dfield_get_type(&uf->new_val));
  }

  /* A primary-key change on the base row re-points the node at the new
  key (design: "UPDATE"). DELETE does NOT come through here: it writes nothing
  at all, because the node has to stay for read views still entitled to
  the row. The old branch nulled a row_ref column here as a tombstone;
  this design has no tombstone and base_pk is NOT NULL. */
  if (new_base_pk != nullptr) {
    upd_field_t *uf = upd_get_nth_field(update, n_fields++);
    const dict_col_t *col = aux->get_col(VEC_AUX_COL_BASE_PK);
    upd_field_set_field_no(uf, dict_col_get_clust_pos(col, clust), clust);
    byte *buf = static_cast<byte *>(mem_heap_alloc(heap, 8));
    mach_write_to_8(buf, *new_base_pk);
    dfield_set_data(&uf->new_val, buf, 8);
    col->copy_type(dfield_get_type(&uf->new_val));
  }

  update->n_fields = n_fields;
  node->update = update;
  node->update_n_fields = n_fields;
  node->cmpl_info = 0;
  node->state = UPD_NODE_UPDATE_CLUSTERED;

  dberr_t err;
  for (;;) {
    thr->run_node = node;
    thr->prev_node = node;

    row_upd_step(thr);

    err = trx->error_state;
    if (err == DB_SUCCESS) {
      break;
    }

    que_thr_stop_for_mysql(thr);
    thr->lock_state = QUE_THR_LOCK_ROW;
    const bool was_lock_wait = row_mysql_handle_errors(&err, trx, thr, &savept);
    thr->lock_state = QUE_THR_LOCK_NOLOCK;

    if (!was_lock_wait) {
      mem_heap_free(heap);
      return err;
    }
  }

  que_thr_stop_for_mysql_no_error(thr, trx);
  mem_heap_free(heap);
  return DB_SUCCESS;
}

/** Copy one (possibly externally stored) field of an aux clustered-index
record into a byte vector.
@return true on success */

/** Copy one record field onto a heap, materialising an off-page BLOB.

The vector and neighbour columns are BLOBs, so a large VECTOR(n) can be
stored off-page; reading the record field directly would hand back a
20-byte reference rather than the data. */
static bool vec_aux_copy_field(const dict_index_t *clust, const rec_t *rec,
                               const ulint *offsets, ulint pos,
                               mem_heap_t *heap, const byte **out,
                               ulint *out_len) {
  ulint len;
  const byte *data = rec_get_nth_field(clust, rec, offsets, pos, &len);

  if (len == UNIV_SQL_NULL) {
    *out = nullptr;
    *out_len = 0;
    return true;
  }

  if (rec_offs_nth_extern(clust, offsets, pos)) {
    data = lob::btr_rec_copy_externally_stored_field(
        nullptr, clust, rec, offsets, dict_table_page_size(clust->table), pos,
        &len, nullptr, false, heap);
    if (data == nullptr) return false;
    *out = data;
    *out_len = len;
    return true;
  }

  *out = static_cast<const byte *>(mem_heap_dup(heap, data, len));
  *out_len = len;
  return true;
}

dberr_t vec_base_collect_rows(dict_table_t *base,
                              const dict_index_t *vec_index, uint32_t dims,
                              std::vector<vec_base_row_t> *rows) {
  ut_a(base != nullptr);
  ut_a(vec_index != nullptr);
  ut_a(rows != nullptr);
  ut_a(dims != 0);
  ut_a(base->vec_aux_col != ULINT_UNDEFINED);

  rows->clear();

  dict_index_t *clust = base->first_index();

  /* The column comes from the index, never from a scan for a BLOB —
  VECTOR, BLOB, TEXT and JSON all collapse to DATA_BLOB. See
  vec_indexed_col_no(). */
  ut_a(vec_index->n_fields == 1);
  const dict_col_t *vec_col = vec_index->get_field(0)->col;
  const ulint pos_vec = dict_col_get_clust_pos(vec_col, clust);
  const ulint pos_id =
      dict_col_get_clust_pos(base->get_col(base->vec_aux_col), clust);

  mem_heap_t *offset_heap = nullptr;
  mem_heap_t *row_heap = mem_heap_create(2048, UT_LOCATION_HERE);
  dberr_t err = DB_SUCCESS;

  mtr_t mtr;
  mtr_start(&mtr);
  btr_pcur_t pcur;
  pcur.open_at_side(true /* left */, clust, BTR_SEARCH_LEAF, true, 0, &mtr);

  ulint n_scanned = 0;

  while (pcur.move_to_next_user_rec(&mtr) == DB_SUCCESS) {
    const rec_t *rec = pcur.get_rec();

    ulint *offsets = rec_get_offsets(rec, clust, nullptr, ULINT_UNDEFINED,
                                     UT_LOCATION_HERE, &offset_heap);

    if (!rec_get_deleted_flag(rec, dict_table_is_comp(base))) {
      const byte *vec_data = nullptr;
      ulint vec_len = 0;
      if (!vec_aux_copy_field(clust, rec, offsets, pos_vec, row_heap, &vec_data,
                              &vec_len)) {
        err = DB_CORRUPTION;
        break;
      }
      /* An indexed vector column is NOT NULL (sql_table.cc), so a null
      here means the record does not match the index we are building. */
      if (vec_data != nullptr) {
        if (vec_len != dims * sizeof(float)) {
          err = DB_CORRUPTION;
          break;
        }

        ulint id_len = 0;
        const byte *id_ptr =
            rec_get_nth_field(clust, rec, offsets, pos_id, &id_len);
        ut_a(id_len == 8);

        ulint pk_len = 0;
        const byte *pk_ptr = rec_get_nth_field(clust, rec, offsets, 0, &pk_len);
        ut_a(pk_len == 8);

        const float *f = reinterpret_cast<const float *>(vec_data);
        rows->push_back({mach_read_from_8(id_ptr),
                         std::vector<float>(f, f + dims),
                         mach_read_from_8(pk_ptr)});

        mem_heap_empty(row_heap);
      }
    }

    /* Batch the mtr so one long scan does not pin pages for its whole
    duration; store and restore the cursor across the boundary. */
    if (++n_scanned % 512 == 0) {
      pcur.store_position(&mtr);
      mtr_commit(&mtr);
      mtr_start(&mtr);
      pcur.restore_position(BTR_SEARCH_LEAF, &mtr, UT_LOCATION_HERE);
    }
  }

  pcur.close();
  mtr_commit(&mtr);

  if (offset_heap != nullptr) mem_heap_free(offset_heap);
  mem_heap_free(row_heap);

  return err;
}

dberr_t vec_aux_read_node(dict_table_t *aux, uint64_t id, mem_heap_t *heap,
                          vec_aux_read_t *out) {
  ut_a(aux != nullptr);
  ut_a(out != nullptr);

  dict_index_t *clust = aux->first_index();

  dtuple_t *ref = dtuple_create(heap, 1);
  dict_index_copy_types(ref, clust, 1);
  byte key[8];
  mach_write_to_8(key, id);
  dfield_set_data(dtuple_get_nth_field(ref, 0), key, sizeof key);

  mtr_t mtr;
  mtr_start(&mtr);
  btr_pcur_t pcur;
  pcur.open_no_init(clust, ref, PAGE_CUR_LE, BTR_SEARCH_LEAF, 0, &mtr,
                    UT_LOCATION_HERE);

  const rec_t *rec = pcur.get_rec();
  if (!page_rec_is_user_rec(rec) ||
      pcur.get_low_match() < dict_index_get_n_unique(clust)) {
    pcur.close();
    mtr_commit(&mtr);
    return DB_RECORD_NOT_FOUND;
  }

  mem_heap_t *offs_heap = nullptr;
  ulint *offsets = rec_get_offsets(rec, clust, nullptr, ULINT_UNDEFINED,
                                   UT_LOCATION_HERE, &offs_heap);

  /* Column positions in the RECORD, not user-column ordinals: a
  clustered record is the key, then DB_TRX_ID and DB_ROLL_PTR, then the
  rest. Using the ordinal directly lands on DB_ROLL_PTR. */
  const ulint p_vec =
      dict_col_get_clust_pos(aux->get_col(VEC_AUX_COL_VEC), clust);
  const ulint p_base_pk =
      dict_col_get_clust_pos(aux->get_col(VEC_AUX_COL_BASE_PK), clust);
  const ulint p_level =
      dict_col_get_clust_pos(aux->get_col(VEC_AUX_COL_LEVEL), clust);
  const ulint p_nb =
      dict_col_get_clust_pos(aux->get_col(VEC_AUX_COL_NEIGHBORS), clust);

  dberr_t err = DB_SUCCESS;
  ulint len;
  const byte *p;

  p = rec_get_nth_field(clust, rec, offsets, p_base_pk, &len);
  if (len != 8) {
    err = DB_CORRUPTION;
    goto done;
  }
  out->base_pk = mach_read_from_8(p);

  p = rec_get_nth_field(clust, rec, offsets, p_level, &len);
  out->level = len == 1 ? p[0] : 0;

  if (!vec_aux_copy_field(clust, rec, offsets, p_vec, heap, &out->vec,
                          &out->vec_len) ||
      !vec_aux_copy_field(clust, rec, offsets, p_nb, heap, &out->neighbors,
                          &out->neighbors_len)) {
    err = DB_CORRUPTION;
  }

done:
  if (offs_heap != nullptr) mem_heap_free(offs_heap);
  pcur.close();
  mtr_commit(&mtr);
  return err;
}
