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

One aux table per vector index, named
"<db>/percona_vec_<type>_<table_id>_<index_id>".
All DDL goes through the InnoDB C API (dict_mem_*, row_create_*_for_mysql,
row_drop_table_for_mysql, row_rename_table_for_mysql) - never through
pars_sql/que_eval_sql, which serializes on the global pars_mutex. */

#ifndef vec0aux_h
#define vec0aux_h

#include "data0types.h"
#include "dict0mem.h"
#include "trx0trx.h"
#include "univ.i"

class THD;

/** Lowercase on-disk / DD prefix shared by all vector aux tables. */
extern const char *VEC_AUX_PREFIX;

/** Number of user columns in a vector aux table.

Vector HNSW has different semantics: one aux row per graph vertex, all
vertices in one table. A single fixed schema (id, vec, base_pk, level,
neighbors) is sufficient and simpler. If phase 2 needs additional shape
variance (e.g., a separate CONFIG aux for HNSW parameters), we'd add it
symmetrically then. */
constexpr ulint VEC_AUX_TABLE_NUM_COLS = 5;

/** Column lengths in a vector aux table. */
constexpr ulint VEC_AUX_ID_COL_LEN = 8;        /* BIGINT UNSIGNED */
constexpr ulint VEC_AUX_VEC_COL_LEN = 0;       /* BLOB: 0 = variable */
constexpr ulint VEC_AUX_BASE_PK_COL_LEN = 8;   /* BIGINT UNSIGNED */
constexpr ulint VEC_AUX_LEVEL_COL_LEN = 1;     /* TINYINT */
constexpr ulint VEC_AUX_NEIGHBORS_COL_LEN = 0; /* BLOB: 0 = variable */

/** Registered index TYPEs. Adding one is adding an enumerator plus a
row in the name table in vec0aux.cc - the type token is part of every
aux table name, so the datadir stays self-describing.

Numbering starts at 1 so a zeroed or otherwise uninitialized value is
not a valid TYPE. */
enum class Vec_index_type : uint8_t { HNSW = 1 };

/** The registered token for a TYPE, e.g. "hnsw" - the string embedded in
aux table names (percona_vec_<token>_<tid>_<iid>) and printed by
SHOW CREATE.
Tokens are lowercase ASCII identifiers and MUST NOT contain '_', which is
the aux-name field separator. */
[[nodiscard]] const char *vec_index_token(Vec_index_type type);

/** Resolve a token back to its TYPE. Returns false for an unknown token,
which is how a reserved percona_vec_ name that is not one of our aux
tables is told apart from one that is.
@param[in]      token           first byte of the token
@param[in]      len             token length, not NUL-terminated
@param[out]     type_out        the resolved TYPE, untouched on failure
@return true iff the token is registered */
[[nodiscard]] bool vec_index_type_by_token(const char *token, size_t len,
                                           Vec_index_type *type_out);

/** Build the on-disk aux table name for one vector index:
"<db>/percona_vec_<type>_<parent_table_id>_<index_id>", e.g.
"test/percona_vec_hnsw_4a_5b" (SPANN R4: the registry's type token makes
the datadir self-describing and gives every TYPE its own namespace -
spann's three tables become percona_vec_spann_<t>_<i>[/_meta/_dead]
without ambiguity).

@param[in]      parent          parent table that owns the vector index
@param[in]      index_id        id of the vector index (from dict_index_t)
@param[in]      type            the index's registered TYPE (names the
                                token embedded in the name)
@param[out]     name_out        destination buffer (>= MAX_FULL_NAME_LEN)
@param[in]      name_out_len    size of destination buffer */
void vec_aux_get_table_name(const dict_table_t *parent, space_index_t index_id,
                            Vec_index_type type, char *name_out,
                            size_t name_out_len);

/** True if `name` is a complete vector aux table name (ANY type):
VEC_AUX_PREFIX, a type token the registry knows, and exactly two hex id
fields, the second ending the string - so "percona_vec_hnsw_1_2" is one of
ours while a user table merely called "percona_vec_data" is not.

Two uses. It reserves the name at CREATE (ha_innobase::create) and at RENAME
(ha_innobase::rename_table), and it recognises an aux table by its name at
DD load (dd_open_table_one), where DICT_TF2_VEC_AUX and the parent id are
reconstructed from it. */
[[nodiscard]] bool vec_aux_is_aux_table_name(const char *name);

/** Parse a "<db>/percona_vec_<type>_<parent_id>_<index_id>" name into its
components. The type token must resolve in the registry
(vec_index_by_name) - a percona_vec_-prefixed name that does not parse is a
reserved-but-invalid name, never an aux table. Used at DD reload time
(dd_open_table_one) to reconstruct dict_table_t::parent_id and
DICT_TF2_VEC_AUX from the on-disk name. Any output pointer may be
nullptr. Returns false if `name` does not match the vector aux
pattern.
@param[in]      name            aux table name, "db/tbl" or bare "tbl"
@param[out]     parent_id_out   parent table id, may be nullptr
@param[out]     index_id_out    vector index id, may be nullptr
@param[out]     type_out        the index TYPE, may be nullptr
@return true iff `name` is a vector aux table name */
[[nodiscard]] bool vec_aux_parse_table_name(const char *name,
                                            table_id_t *parent_id_out,
                                            space_index_t *index_id_out,
                                            Vec_index_type *type_out = nullptr);

/** Create one aux table for a single vector index. Uses the InnoDB C API
only (no pars_sql).
@param[in,out] trx        transaction
@param[in]     parent     parent table - its space, flags and flags2 are
                          inherited so the aux lives in the right place
@param[in]     index_id   id of the vector index this aux belongs to
@return DB_SUCCESS on success */
[[nodiscard]] dberr_t vec_aux_create_one_table(trx_t *trx,
                                               const dict_table_t *parent,
                                               space_index_t index_id);

/** Create aux tables for every vector index already attached to `parent`. */
[[nodiscard]] dberr_t vec_aux_create_all_tables(trx_t *trx,
                                                const dict_table_t *parent);

/** DD-register every vector aux table already attached to `parent`. The
in-memory dict_table_t entries must have been created by
@ref vec_aux_create_all_tables / @ref vec_aux_create_one_table first.
Returns true on success. */
[[nodiscard]] bool vec_aux_create_dd_table(dict_table_t *parent,
                                           const dict_index_t *index);

/** Take an exclusive MDL on every vector aux table belonging to `parent`, so
nothing can be reading one while we drop it. The aux tables are hidden, so
no MDL was taken for them when the server locked the parent.
@param[in]  thd     thread taking the locks
@param[in]  parent  parent that owns the vector indexes
@return DB_SUCCESS, or DB_ERROR if a lock could not be taken */
[[nodiscard]] dberr_t vec_aux_lock_all_tables(THD *thd,
                                              const dict_table_t *parent);

/** Drop the aux table for a single vector index. */
[[nodiscard]] dberr_t vec_aux_drop_one_table(trx_t *trx,
                                             const dict_table_t *parent,
                                             space_index_t index_id);

/** Drop every vector aux table belonging to `parent`. */
[[nodiscard]] dberr_t vec_aux_drop_all_tables(trx_t *trx, dict_table_t *parent);

/** Flip every vector aux table belonging to `parent` from pinned
(can_be_evicted=false, the default from row_create_table_for_mysql) to
evictable, so dict_sys can LRU them out later. Called on both success and
fail paths of ALTER prepare - the "make evictable" side of aux lifecycle.
Safe on aux tables that aren't currently cached (skips silently).
@param[in]  parent          parent that owns the vector indexes
@param[in]  dict_locked     true iff caller already holds dict_sys mutex */
void vec_aux_detach_tables(const dict_table_t *parent, bool dict_locked);

/** The vector index on @p table, or nullptr. At most one exists. */
[[nodiscard]] const dict_index_t *vec_index_of(const dict_table_t *table);

[[nodiscard]] inline dict_index_t *vec_index_of(dict_table_t *table) {
  return const_cast<dict_index_t *>(
      vec_index_of(static_cast<const dict_table_t *>(table)));
}

/** Rename every vector aux table belonging to `parent` after the parent itself
has been renamed to `new_parent_name`. Only the db-prefix portion of the aux
name changes - the suffix is keyed by (table_id, index_id) which are
invariant under RENAME. Caller must have verified that the schema actually
changed (cross-schema rename); no early-out check here.
@param[in,out] trx                transaction
@param[in]     parent             dict_table_t of the parent (still
                                  registered under its OLD name in dict_sys)
@param[in]     new_parent_name    new full name of the parent ("db/tbl")
@param[in]     replay             whether running inside crash-recovery
                                  replay
@return DB_SUCCESS on success */
[[nodiscard]] dberr_t vec_aux_rename_tables(trx_t *trx, dict_table_t *parent,
                                            const char *new_parent_name,
                                            bool replay);

#endif /* vec0aux_h */
