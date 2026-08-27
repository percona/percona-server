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

/** @file include/vec0aux.h
Auxiliary tables for vector (HNSW) indexes.

One aux table per vector index, named "<db>/vec_<table_id>_<index_id>".
All DDL goes through the InnoDB C API (dict_mem_*, row_create_*_for_mysql,
row_drop_table_for_mysql, row_rename_table_for_mysql) — never through
pars_sql/que_eval_sql, which serializes on the global pars_mutex. */

#ifndef vec0aux_h
#define vec0aux_h

#include "data0types.h"
#include "dict0mem.h"
#include "trx0trx.h"
#include "univ.i"

/** Lowercase on-disk / DD prefix shared by all vector aux tables. */
extern const char *VEC_AUX_PREFIX;

/** Hidden auxiliary column added to a base table that owns >= 1 vector
index. Type: BIGINT UNSIGNED NOT NULL; no secondary index. */
#define VEC_AUX_ID_COL_NAME "percona_vec_aux_id"

/** Number of user columns in a vector aux table.

DEVIATION FROM FTS: FTS uses multiple aux table shapes selected by
suffix — 6 per-index shapes (INDEX_1..INDEX_5, DELETED_CACHE) plus 5
per-table common shapes (CONFIG, DELETED, ADDED, BEING_DELETED,
BEING_DELETED_CACHE) — because FTS's inverted-index storage splits
tokens across hash buckets and keeps per-index state separate from
per-table state. Vector HNSW has different semantics: one aux row
per graph vertex, all vertices in one table. A single fixed schema
(id, base_pk, vec, level, neighbors) is sufficient and simpler. If
phase 2 needs additional shape variance (e.g., a separate CONFIG
aux for HNSW parameters), we'd add it symmetrically then. */
constexpr ulint VEC_AUX_TABLE_NUM_COLS = 5;

/** Column lengths in a vector aux table. */
constexpr ulint VEC_AUX_ID_COL_LEN = 8;        /* BIGINT UNSIGNED */
constexpr ulint VEC_AUX_VEC_COL_LEN = 0;       /* BLOB: 0 = variable */
constexpr ulint VEC_AUX_BASE_PK_COL_LEN = 8;   /* BIGINT UNSIGNED */
constexpr ulint VEC_AUX_LEVEL_COL_LEN = 1;     /* TINYINT */
constexpr ulint VEC_AUX_NEIGHBORS_COL_LEN = 0; /* BLOB: 0 = variable */

/** Registered index TYPEs. Adding one is adding an enumerator plus a
row in the name table in vec0aux.cc — the type token is part of every
aux table name, so the datadir stays self-describing. */
enum class Vec_index_type : uint8_t { HNSW = 0 };

/** The registered token for a TYPE, e.g. "hnsw" — the string embedded in
aux table names (vec_<token>_<tid>_<iid>) and printed by SHOW CREATE.
Tokens are lowercase ASCII identifiers and MUST NOT contain '_', which is
the aux-name field separator. */
[[nodiscard]] const char *vec_index_token(Vec_index_type type);

/** Resolve a token back to its TYPE. Returns false for an unknown token,
which is how a reserved "vec_" name that is not one of our aux tables is
told apart from one that is. */
[[nodiscard]] bool vec_index_type_by_token(const char *token, size_t len,
                                           Vec_index_type *type_out);

/** Build the on-disk aux table name for one vector index:
"<db>/vec_<type>_<parent_table_id>_<index_id>", e.g.
"test/vec_hnsw_4a_5b" (SPANN R4: the registry's type token makes the
datadir self-describing and gives every TYPE its own namespace —
spann's three tables become vec_spann_<t>_<i>[/_meta/_dead] without
ambiguity).

@param[in]      parent          parent table that owns the vector index
@param[in]      index_id        id of the vector index (from dict_index_t)
@param[in]      type            the index's registered TYPE (names the
                                token embedded in the name)
@param[out]     name_out        destination buffer (>= MAX_FULL_NAME_LEN)
@param[in]      name_out_len    size of destination buffer */
void vec_aux_get_table_name(const dict_table_t *parent, space_index_t index_id,
                            Vec_index_type type, char *name_out,
                            size_t name_out_len);

/** True if `name` starts with the reserved "vec_" prefix (ALL types).
Used to hide aux tables from INFORMATION_SCHEMA / SHOW TABLES and to
reserve the namespace at CREATE. */
bool vec_aux_is_aux_table_name(const char *name);

/** Parse a "<db>/vec_<type>_<parent_id>_<index_id>" name into its
components. The type token must resolve in the registry
(vec_index_by_name) — a vec_-prefixed name that does not parse is a
reserved-but-invalid name, never an aux table. Used at DD reload time
(dd_open_table_one) to reconstruct dict_table_t::parent_id and
DICT_TF2_VEC_AUX from the on-disk name. Any output pointer may be
nullptr. Returns false if `name` does not match the vector aux
pattern. */
bool vec_aux_parse_table_name(const char *name, table_id_t *parent_id_out,
                              space_index_t *index_id_out,
                              Vec_index_type *type_out = nullptr);

/** Create one aux table for a single vector index. Uses the InnoDB C API
only (no pars_sql).
@param[in,out] trx        transaction
@param[in]     parent     parent table — its space, flags and flags2 are
                          inherited so the aux lives in the right place
@param[in]     index_id   id of the vector index this aux belongs to
@return DB_SUCCESS on success */
dberr_t vec_aux_create_one_table(trx_t *trx, const dict_table_t *parent,
                                 space_index_t index_id);

/** Create aux tables for every vector index already attached to `parent`. */
dberr_t vec_aux_create_all_tables(trx_t *trx, const dict_table_t *parent);

/** DD-register every vector aux table already attached to `parent`.
The in-memory dict_table_t entries must have been created by
@ref vec_aux_create_all_tables / @ref vec_aux_create_one_table first.
Mirrors fts_create_index_dd_tables. Returns true on success. */
bool vec_aux_create_dd_tables(dict_table_t *parent);

/** Drop the aux table for a single vector index. */
dberr_t vec_aux_drop_one_table(trx_t *trx, const dict_table_t *parent,
                               space_index_t index_id);

/** Drop every vector aux table belonging to `parent`. */
dberr_t vec_aux_drop_all_tables(trx_t *trx, dict_table_t *parent);

/** Flip every vector aux table belonging to `parent` from pinned
(can_be_evicted=false, the default from row_create_table_for_mysql)
to evictable, so dict_sys can LRU them out later. Mirrors
fts_detach_aux_tables. Called on both success and fail paths of ALTER
prepare — the "make evictable" side of aux lifecycle. Safe on aux
tables that aren't currently cached (skips silently).
@param[in]  parent          parent that owns the vector indexes
@param[in]  dict_locked     true iff caller already holds dict_sys mutex */
void vec_aux_detach_tables(const dict_table_t *parent, bool dict_locked);

/** True iff `table` has at least one vector index attached. */
bool vec_aux_table_has_vector_index(const dict_table_t *table);

/** Rename every vector aux table belonging to `parent` after the parent
itself has been renamed to `new_parent_name`. Mirrors fts_rename_aux_tables.
Only the db-prefix portion of the aux name changes — the suffix is
keyed by (table_id, index_id) which are invariant under RENAME. Caller
must have verified that the schema actually changed (cross-schema
rename); no early-out check here.

@param[in,out] trx                transaction
@param[in]     parent             dict_table_t of the parent (still
                                  registered under its OLD name in dict_sys)
@param[in]     new_parent_name    new full name of the parent ("db/tbl")
@param[in]     replay             whether running inside crash-recovery
                                  replay
@return DB_SUCCESS on success */
dberr_t vec_aux_rename_tables(trx_t *trx, dict_table_t *parent,
                              const char *new_parent_name, bool replay);

/** Add the hidden percona_vec_aux_id column (BIGINT UNSIGNED NOT NULL) to the
in-memory `dict_table_t` and set DICT_TF2_HAS_VEC_AUX_COL. Mirrors
fts_add_doc_id_column for FTS_DOC_ID. Called both at CREATE time and
during DD load when the dd::Table has a hidden percona_vec_aux_id.

@param[in,out]  table   dict_table_t under construction
@param[in,out]  heap    memory heap for column allocation */
void vec_add_aux_id_column(dict_table_t *table, mem_heap_t *heap);

/** Read the label stamped into a row's hidden percona_vec_aux_id column.

The analog of fts_get_doc_id_from_row: after the insert path has stamped
the column, this is how the graph side learns which label the row got.
@param[in]  table  the base table
@param[in]  row    the row, as converted for InnoDB
@return the label; never 0 for a stamped row */
uint64_t vec_get_aux_id_from_row(const dict_table_t *table,
                                 const dtuple_t *row);

/** The table column the vector index covers.

The table-level form of what vec_row_vector_bytes() does per index: the
column is index->get_field(0)->col, because a DICT_VECTOR index carries
its key part like any other index. It is not searched for — VECTOR,
BLOB, TEXT and JSON all map to DATA_BLOB, so no type test can pick it
out. Ignore the field's prefix_len, which is 1 for a vector key part and
describes nothing about the column.

Assumes at most one vector index per table (design section 23).

@param[in]  table  base table
@return the column number, or ULINT_UNDEFINED if the table has no
vector index */
ulint vec_indexed_col_no(const dict_table_t *table);

/** Does this update field change a column covered by a vector index?
@param[in]  table   the base table
@param[in]  ufield  one field of the update vector
@return true if it does */
bool vec_upd_changes_indexed_vector(const dict_table_t *table,
                                    const upd_field_t *ufield);

/** Fill an update field so it sets the hidden label column to `label`.

The mirror of fts_update_doc_id: this is how a fresh label joins the
user's UPDATE rather than being written by a second statement.
@param[in]      table   the base table
@param[in,out]  ufield  the update field to fill
@param[in]      label   the new label */
void vec_update_aux_id(dict_table_t *table, upd_field_t *ufield,
                       uint64_t *next_label);

/** The new vector value carried by an update vector, if it changes one.
@param[in]   table  the base table
@param[in]   update the update vector
@param[out]  len    its length in bytes
@return the bytes, or nullptr if this update does not change the vector */
const char *vec_upd_new_vector(const dict_table_t *table, const upd_t *update,
                               ulint *len);

/** The primary key of the row an update node is positioned on.
@param[in]   table  the base table
@param[in]   node   the update node
@param[out]  pk     the key
@return true if it could be read */
bool vec_upd_row_pk(const dict_table_t *table, const upd_node_t *node,
                    uint64_t *pk);

/** Atomically assign the next percona_vec_aux_id for a row about to be
inserted. Valid ids start at 1. Stamped into the hidden percona_vec_aux_id
dfield by the INSERT path. See the implementation comment for the phase-1
persistence caveat. */
uint64_t vec_assign_next_aux_id(dict_table_t *table);

/** Stamp the hidden percona_vec_aux_id dfield in `row` with the next id from
the per-table counter. No-op for tables without the hidden column.
Allocations come from `heap` so they outlive this call. Called from
the INSERT path (mirrors fts_create_doc_id). */
void vec_stamp_aux_id(dict_table_t *table, dtuple_t *row, mem_heap_t *heap);

#endif /* vec0aux_h */
