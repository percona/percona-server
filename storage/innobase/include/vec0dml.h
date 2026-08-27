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

/** @file include/vec0dml.h
Row-level DML on vector-index auxiliary tables through the InnoDB query-graph
C API — insert, targeted neighbor update, and MVCC-consistent full load.

DEVIATION FROM FTS: FTS performs its aux-table DML through the internal SQL
parser (fts_parse_sql / fts_eval_sql), which serializes every operation on
the global pars_mutex. Vector aux DML runs on every user INSERT, so it uses
the same parser-free query-graph machinery row0mysql itself uses
(ins_node/upd_node + pars_complete_graph_for_exec) on the user transaction:
full redo/undo/locking, no global mutex. */

#ifndef vec0dml_h
#define vec0dml_h

#include <cstdint>
#include <tuple>
#include <vector>

#include "db0err.h"
#include "dict0mem.h"
#include "trx0trx.h"
#include "univ.i"

/** One materialized aux row for insertion. Pointers are caller-owned;
vec_aux_insert copies what it needs. */
struct vec_aux_row_t {
  /** percona_vec_aux_id of the base row == HNSW label == aux PK.
  Never 0: the class reserves graph node id 0 as the empty-neighbour
  sentinel, which is what lets record 0 of the aux hold index metadata
  (the entry point) instead of a node. */
  uint64_t id;
  /** vector data, dims * sizeof(float) bytes */
  const float *vec;
  uint32_t dims;
  /** the base row this node describes. NOT NULL — there is no tombstone
  in this design: a deleted base row keeps its node, and the read path
  filters it by looking base_pk up under the caller's read view. */
  uint64_t base_pk;
  /** HNSW level of this node; must be in [0, 127] */
  int level;
  /** neighbour slots, flat big-endian ids, 0 = empty slot */
  const byte *neighbors;
  ulint neighbors_len;
};

/** Bytes a node's neighbour blob occupies.

The blob is a flat big-endian array of graph ids, one per slot, with 0
for an empty slot. There is no header: the class reserves graph node id 0
as the empty-slot sentinel precisely so a persistor "need not store a
per-layer neighbor count" (hnsw.h). The slot count is (level + 2) * M,
recoverable from the level stored in the row and the M stored with the
index, so writing it again would only create a second copy of the truth.
@param[in]  level  the node's top layer
@param[in]  m      the index's M
@return blob length in bytes */
ulint vec_aux_neighbors_blob_len(uint8_t level, uint32_t m);

/** Insert one node into a vector aux table.

Parser-free: builds an INS_DIRECT node on a private heap and drives
row_ins_step, the same machinery row_insert_for_mysql runs, without the
prebuilt and without pars_sql (which would take the global pars_mutex).
@param[in,out]  trx  transaction to insert on
@param[in,out]  aux  the aux table, already open with MDL held
@param[in]      row  the node to write
@return DB_SUCCESS, or an error */
dberr_t vec_aux_insert(trx_t *trx, dict_table_t *aux, const vec_aux_row_t &row);

/** Update one node's neighbour slots, and optionally its base_pk.

Positioned by primary key rather than by search, so it takes the locks a
searched UPDATE's read would have taken: IX on the table and an explicit
X on the record.

Passing new_base_pk re-points the node at a new primary key, which is
what a base-row primary-key change needs (design section 10). DELETE does
not come through here at all: it writes nothing, because the node has to
stay for read views still entitled to the row.
@param[in,out]  trx            transaction to update on
@param[in,out]  aux            the aux table, already open with MDL held
@param[in]      id             the node to update
@param[in]      neighbors      new neighbour blob
@param[in]      neighbors_len  its length
@param[in]      new_base_pk    new base primary key, or nullptr to leave it
@return DB_SUCCESS, DB_RECORD_NOT_FOUND, or an error */
dberr_t vec_aux_update_row(trx_t *trx, dict_table_t *aux, uint64_t id,
                           const byte *neighbors, ulint neighbors_len,
                           const uint64_t *new_base_pk = nullptr);

/** One node read back from the aux table. Pointers are into a caller
supplied heap and live as long as it does. */
struct vec_aux_read_t {
  const byte *vec{nullptr};
  ulint vec_len{0};
  uint64_t base_pk{0};
  const byte *neighbors{nullptr};
  ulint neighbors_len{0};
  uint8_t level{0};
};

/** Read one node from a vector aux table by label.

A plain point lookup on the aux primary key, which is what the label is.
No read view is taken: the graph is shared by every transaction rather
than being per-transaction state, so there is no snapshot it belongs to.
A node read here that later turns out to belong to a rolled-back
statement becomes an orphan, and the read path already filters orphans
by resolving base_pk under the reader's own view.
@param[in]      aux   the aux table, open
@param[in]      id    the label
@param[in,out]  heap  heap the returned bytes are copied onto
@param[out]     out   the node
@return DB_SUCCESS, DB_RECORD_NOT_FOUND, or an error */
dberr_t vec_aux_read_node(dict_table_t *aux, uint64_t id, mem_heap_t *heap,
                          vec_aux_read_t *out);

#endif /* vec0dml_h */
