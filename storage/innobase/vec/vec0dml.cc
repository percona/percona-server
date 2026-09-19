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
Parser-free DML on vector-index auxiliary tables. See vec0dml.h for why the
internal SQL parser is not used. */

#include "vec0dml.h"

#include "my_dbug.h"

#include <algorithm>
#include <limits>

#include "btr0load.h"
#include "btr0pcur.h"
#include "buf0flu.h"
#include "dict0dict.h"
#include "lob0lob.h"
#include "mach0data.h"
#include "pars0pars.h"
#include "que0que.h"
#include "read0types.h"
#include "row0ins.h"
#include "row0mysql.h"
#include "row0row.h"
#include "row0upd.h"
#include "row0vers.h"
#include "scope_guard.h"
#include "trx0roll.h"
#include "trx0undo.h"
#include "vec0aux.h"

/* Aux table user-column ordinals, fixed by create_in_mem_vec_aux_table
(vec0aux.cc): id, vec, base_pk, level, neighbors. */
constexpr ulint VEC_AUX_COL_ID = 0;
constexpr ulint VEC_AUX_COL_VEC = 1;
constexpr ulint VEC_AUX_COL_BASE_PK = 2;
constexpr ulint VEC_AUX_COL_LEVEL = 3;
constexpr ulint VEC_AUX_COL_NEIGHBORS = 4;

/** Set one aux field, mapping a zero-length value to an empty value rather
than SQL NULL - every aux column is NOT NULL. Defined below; both writers
use it. */
static void vec_aux_set_dfield(dfield_t *df, const void *data, ulint len,
                               mem_heap_t *heap);

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

/** Bottom-up build of a vector aux table.

vec_aux_insert drives the row API: undo per row, redo per row, and an
insert into the middle of a tree that is being built left to right anyway.
An index build does not need any of that. The aux table is created by this
ALTER and dropped if it rolls back, so there is nothing for row undo to
undo, and Btree_load writes its pages with MTR_LOG_NO_REDO.

The price of no redo is that nothing else will get those pages to disk:
they reach it only because the Flush_observer below flushes them before
the statement commits. The DDL's own observer covers the table being
altered, not this one - Flush_observer is per tablespace - so the aux gets
its own.

Rows must arrive in ascending id order. Btree_load appends; it does not
sort. HNSW::for_each_node_sorted is what supplies that. */
struct Vec_aux_bulk {
  Vec_aux_bulk(trx_t *trx, dict_table_t *aux_, Flush_observer *observer_)
      : aux(aux_),
        clust(aux_->first_index()),
        observer(observer_),
        load(ut::new_withkey<Btree_load>(UT_NEW_THIS_FILE_PSI_KEY, clust,
                                         trx->id, observer_)),
        heap(mem_heap_create(1024, UT_LOCATION_HERE)) {
    /* Every row carries the same system columns: this transaction, and a
    roll pointer flagged as an insert with no undo behind it - the same
    pair ddl::bulk uses. */
    trx_write_trx_id(trx_id_buf, trx->id);
    trx_write_roll_ptr(roll_ptr_buf, trx_undo_build_roll_ptr(true, 0, 0, 0));
  }

  ~Vec_aux_bulk() {
    if (heap != nullptr) mem_heap_free(heap);
    if (load != nullptr) ut::delete_(load);
    /* Flushing or freeing it here would be flushing half a statement's pages,
    and ~Flush_observer would assert on the rest. */
  }

  Vec_aux_bulk(const Vec_aux_bulk &) = delete;
  Vec_aux_bulk &operator=(const Vec_aux_bulk &) = delete;

  dict_table_t *aux{};
  dict_index_t *clust{};
  Flush_observer *observer{};
  Btree_load *load{};
  mem_heap_t *heap{};
  byte trx_id_buf[DATA_TRX_ID_LEN]{};
  byte roll_ptr_buf[DATA_ROLL_PTR_LEN]{};
  uint64_t n_rows{};
};

/** Fill one user dfield of the aux row tuple with a heap-duplicated
value (the run loop may retry after lock waits; values must be stable). */
static void vec_aux_set_dfield(dfield_t *df, const void *data, ulint len,
                               mem_heap_t *heap) {
  /* Every column of the aux table is NOT NULL, so there is no SQL NULL
  case to handle here - and mapping a zero-length value onto NULL would
  be wrong rather than merely unused: a node with no neighbours yet has
  an EMPTY neighbour blob, not a missing one, and handing SQL_NULL to a
  NOT NULL column trips rec_get_converted_size_comp_prefix_low. Length 0
  still needs a non-null data pointer for dfield_set_data. */
  if (len == 0) {
    static const byte empty = 0;
    dfield_set_data(df, &empty, 0);
    return;
  }
  ut_ad(data != nullptr);
  void *copy = mem_heap_dup(heap, data, len);
  dfield_set_data(df, copy, len);
}

static void vec_aux_set_field(dtuple_t *tuple, ulint col_no, const void *data,
                              ulint len, mem_heap_t *heap) {
  vec_aux_set_dfield(dtuple_get_nth_field(tuple, col_no), data, len, heap);
}

dberr_t vec_aux_insert(trx_t *trx, dict_table_t *aux,
                       const vec_aux_row_t &row) {
  ut_ad(trx != nullptr);
  ut_ad(aux != nullptr);

  /* A transient write failure - the kind a lock wait or a full tablespace
  produces. Injected because the distinction it proves (a failure the
  graph recovers from, against one that leaves it inconsistent) has no
  other way in from SQL. */
  DBUG_EXECUTE_IF("vec_aux_insert_transient_fail",
                  return DB_OUT_OF_FILE_SPACE;);
  /* Record 0 is index metadata, not a node: it names the graph's entry
  point and legitimately carries no vector and no neighbours. Every real
  node has both - id 0 is reserved as the empty-slot sentinel, so a node
  can never occupy record 0. */
  ut_ad(row.vec != nullptr || (row.id == 0 && row.dims == 0));
  ut_ad(row.neighbors != nullptr || row.neighbors_len == 0);

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
  exec builds the fork/thr only - no SQL parser involved), fill the row,
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

  /* que_graph_free() is the standard, complete teardown for a graph
  built this way: it recurses per node type (freeing, for
  QUE_NODE_INSERT, node->entry_sys_heap -- an allocation of its own,
  independent of heap above, made by ins_node_create in row0ins.cc)
  and then frees the graph's own heap, which que_fork_create pointed
  at our heap. A parser-free path like this one never runs that
  recursion on its own, so without this guard node->entry_sys_heap
  leaks on every call. */
  auto graph_guard =
      create_scope_guard([thr]() { que_graph_free(thr->graph); });

  /* Activate the fork, as every other MySQL-interface caller does
  (row0mysql.cc does it for ins_graph, sel_graph and upd_graph).
  pars_complete_graph_for_exec leaves the fork QUE_FORK_COMMAND_WAIT, and
  that is the first thing que_thr_stop() tests (que0que.cc), so it
  reports "stop this thread" - which RecLock::prepare treats as impossible
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
      return err;
    }
    ut_ad(node->state == INS_NODE_INSERT_ENTRIES ||
          node->state == INS_NODE_ALLOC_ROW_ID);
  }

  que_thr_stop_for_mysql_no_error(thr, trx);
  return DB_SUCCESS;
}

dberr_t vec_aux_update_row(trx_t *trx, dict_table_t *aux, uint64_t id,
                           const byte *neighbors, ulint neighbors_len,
                           const uint64_t *new_base_pk) {
  ut_ad(trx != nullptr);
  ut_ad(aux != nullptr);
  ut_ad(neighbors != nullptr || neighbors_len == 0);

  mem_heap_t *heap = mem_heap_create(1024, UT_LOCATION_HERE);
  dict_index_t *clust = aux->first_index();

  /* Standard update machinery - the same upd_node + row_upd_step every
  SQL UPDATE runs on. In a regular UPDATE the preceding row_search_mvcc
  read positions the cursor and takes the locks; we know the PK and
  skip the search, so we position and lock ourselves below.
  (Self-positioned-upd_node implementation reference: the FK-cascade
  code, row0ins.cc; its run loop touches thr->prebuilt, which we
  don't have - hence a private loop.) */
  upd_node_t *node = row_create_update_node_for_mysql(aux, heap);

  /* Search tuple for the target row's PK. */
  dtuple_t *ref = dtuple_create(heap, 1);
  dict_index_copy_types(ref, clust, 1);
  byte id_buf[8];
  mach_write_to_8(id_buf, id);
  dfield_set_data(dtuple_get_nth_field(ref, 0), id_buf, sizeof(id_buf));

  que_thr_t *thr = pars_complete_graph_for_exec(node, trx, heap, nullptr);

  /* que_graph_free() is the standard, complete teardown for a graph
  built this way: it recurses per node type (freeing, for
  QUE_NODE_UPDATE, node->pcur -- including the record buffer
  store_position() below populates -- node->update->per_stmt_heap and
  node->heap, all allocations of their own, independent of heap above,
  made by row_create_update_node_for_mysql/upd_node_create in
  row0mysql.cc/row0upd.cc) and then frees the graph's own heap, which
  que_fork_create pointed at our heap. A parser-free path like this
  one never runs that recursion on its own, so without this guard all
  three leak on every call. */
  auto graph_guard =
      create_scope_guard([thr]() { que_graph_free(thr->graph); });

  /* Activate the fork, as every other MySQL-interface caller does
  (row0mysql.cc does it for ins_graph, sel_graph and upd_graph).
  pars_complete_graph_for_exec leaves the fork QUE_FORK_COMMAND_WAIT, and
  that is the first thing que_thr_stop() tests (que0que.cc), so it
  reports "stop this thread" - which RecLock::prepare treats as impossible
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
    /* Length 0 still needs a non-null data pointer, same as
    vec_aux_set_field: dfield_set_data (and, downstream,
    rec_set_nth_field_low's memcpy) must never see a null source. */
    static const byte empty_neighbors = 0;
    void *copy = neighbors_len != 0
                     ? mem_heap_dup(heap, neighbors, neighbors_len)
                     : const_cast<byte *>(&empty_neighbors);
    dfield_set_data(&uf->new_val, copy, neighbors_len);
    col->copy_type(dfield_get_type(&uf->new_val));
  }

  /* Only record 0 changes its base_pk here: it names the entry point
  (vec_persist_entry_point). A base-row primary-key change never comes
  here - it gets a new node under a fresh label, and the old node keeps
  its old key for read views that still see the old row
  (vector_update_pk.test). DELETE writes nothing either, for the same
  reason. */
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
      return err;
    }
  }

  que_thr_stop_for_mysql_no_error(thr, trx);
  return DB_SUCCESS;
}

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

dberr_t vec_aux_read_node(dict_table_t *aux, uint64_t id, mem_heap_t *heap,
                          vec_aux_read_t *out) {
  ut_ad(aux != nullptr);
  ut_ad(out != nullptr);

  /* Make a node the graph names look absent, which is what a neighbour
  list pointing at a row that is no longer there does. Record 0 is left
  alone: it carries the entry point rather than a node, and a miss on it
  means the index is empty, which is a different path entirely. */
  DBUG_EXECUTE_IF(
      "vec_aux_node_missing", if (id != 0) { return DB_RECORD_NOT_FOUND; });

  dict_index_t *clust = aux->first_index();

  dtuple_t *ref = dtuple_create(heap, 1);
  dict_index_copy_types(ref, clust, 1);
  byte key[8];
  mach_write_to_8(key, id);
  dfield_set_data(dtuple_get_nth_field(ref, 0), key, sizeof key);

  mtr_t mtr;
  mtr_start(&mtr);
  btr_pcur_t pcur;
  if (!row_search_on_row_ref(&pcur, BTR_SEARCH_LEAF, aux, ref, &mtr)) {
    pcur.close();
    mtr_commit(&mtr);
    return DB_RECORD_NOT_FOUND;
  }
  const rec_t *rec = pcur.get_rec();

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
  /* id 0 is the metadata row, not a graph node: corrupting its level
  would be caught by vec_runtime_load before the graph is ever built, so
  the hook skips it and exercises the node path instead. */
  DBUG_EXECUTE_IF("vec_aux_corrupt_level_len", if (id != 0) len = 0;);
  if (len != 1) {
    /* Defaulting to level 0 here would hand the graph a node at the
    wrong layer, and the neighbour blob is sized from the level - so a
    length that happens to match would be accepted and a node quietly
    misplaced. The base_pk field above already refuses a bad length;
    this one now does too. */
    err = DB_CORRUPTION;
    goto done;
  }
  out->level = p[0];

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
