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

/** @file vec/vec0aux.cc
Auxiliary tables for vector (HNSW) indexes. Phase 1: creation, drop, rename,
naming. No population — that lands in PS-11300. */

#include "vec0aux.h"

#include <cstdio>
#include <cstring>

#include "data0type.h"
#include "dict0boot.h"
#include "dict0dd.h"
#include "dict0dict.h"
#include "dict0mem.h"
#include "fts0fts.h"
#include "fts0priv.h"
#include "row0mysql.h"
#include "trx0trx.h"
#include "univ.i"
#include "ut0new.h"

const char *VEC_AUX_PREFIX = "percona_vec_";

namespace {

/** Extract the flags2 bits an aux table should inherit from its parent —
file_per_table, encryption, temporary — plus DICT_TF2_VEC_AUX. Same set the
FTS aux path preserves (see fts_get_table_flags2_for_aux_tables in fts0fts.cc;
that helper is file-static so we re-derive it here), except that the aux
marker is ours and not DICT_TF2_AUX.

That distinction is load-bearing. is_aux() covers both kinds, but
dd_table_open_on_id dispatches on is_fts_aux() and then asserts the name
parses as an FTS aux name. Stamping DICT_TF2_AUX here would send every vec
aux down that branch and trip the assertion on the first I_S or SYS_INDEXES
scan that opens one. The DD reload path in dict0dd.cc reconstructs
DICT_TF2_VEC_AUX from the on-disk name; creation has to agree with it. */
inline uint32_t aux_flags2_from_parent(const dict_table_t *parent) {
  return (parent->flags2 & DICT_TF2_USE_FILE_PER_TABLE) |
         (parent->flags2 & DICT_TF2_ENCRYPTION_FILE_PER_TABLE) |
         (parent->flags2 & DICT_TF2_TEMPORARY) | DICT_TF2_VEC_AUX;
}

/** Build the database-prefix portion of a parent name "db/tbl" — returns
the byte length of "db/" (including the slash) or 0 if `parent_name` has no
slash. */
size_t db_prefix_len(const char *parent_name) {
  const char *slash =
      static_cast<const char *>(memchr(parent_name, '/', strlen(parent_name)));
  return slash != nullptr ? static_cast<size_t>(slash - parent_name) + 1 : 0;
}

}  // namespace

const char *vec_index_token(Vec_index_type type) {
  switch (type) {
    case Vec_index_type::HNSW:
      return "hnsw";
  }
  ut_error;
}

void vec_aux_get_table_name(const dict_table_t *parent, space_index_t index_id,
                            Vec_index_type type, char *name_out,
                            size_t name_out_len) {
  ut_a(parent != nullptr);
  ut_a(name_out != nullptr);
  ut_a(name_out_len >= MAX_FULL_NAME_LEN);

  const char *parent_name = parent->name.m_name;
  const size_t db_len = db_prefix_len(parent_name);

  const char *token = vec_index_token(type);
  /* '_' is the field separator — a token containing it would make the
  name unparseable (contract in vec0aux.h). */
  ut_ad(strchr(token, '_') == nullptr);

  char table_id_str[FTS_AUX_MIN_TABLE_ID_LENGTH];
  int n = fts_write_object_id(parent->id, table_id_str);
  ut_a(n > 0);

  char index_id_str[FTS_AUX_MIN_TABLE_ID_LENGTH];
  n = fts_write_object_id(index_id, index_id_str);
  ut_a(n > 0);

  const int written = snprintf(
      name_out, name_out_len, "%.*s%s%s_%s_%s", static_cast<int>(db_len),
      parent_name, VEC_AUX_PREFIX, token, table_id_str, index_id_str);
  ut_a(written > 0);
  ut_a(static_cast<size_t>(written) < name_out_len);
}







bool vec_index_type_by_token(const char *token, size_t len,
                             Vec_index_type *type_out) {
  static constexpr Vec_index_type known[] = {Vec_index_type::HNSW};
  for (const Vec_index_type t : known) {
    const char *tok = vec_index_token(t);
    if (strlen(tok) == len && memcmp(tok, token, len) == 0) {
      if (type_out != nullptr) *type_out = t;
      return true;
    }
  }
  return false;
}

/* Match the FULL computed shape "<db>/percona_vec_<type>_<tid>_<iid>":
the prefix, a type token that resolves in the token table, and two
parseable object ids. Anything else is an ordinary user table that
merely starts the same way.

Same rule as fts_is_aux_table_name, which validates
<prefix><hex_id>_<suffix> and so leaves "fts_data" available. Matching
on the prefix alone would reserve a slice of the user namespace
permanently and reject existing tables on upgrade, to catch a collision
this check catches anyway. */
bool vec_aux_parse_table_name(const char *name, table_id_t *parent_id_out,
                              space_index_t *index_id_out,
                              Vec_index_type *type_out) {
  if (name == nullptr) return false;
  const char *slash = strchr(name, '/');
  const char *after_db = slash != nullptr ? slash + 1 : name;
  if (strncmp(after_db, VEC_AUX_PREFIX, strlen(VEC_AUX_PREFIX)) != 0) {
    return false;
  }
  const char *token = after_db + strlen(VEC_AUX_PREFIX);

  const char *token_end = strchr(token, '_');
  if (token_end == nullptr || token_end == token) return false;
  Vec_index_type type = Vec_index_type::HNSW;
  if (!vec_index_type_by_token(token, static_cast<size_t>(token_end - token),
                               &type)) {
    return false;
  }

  const char *tail = token_end + 1;
  table_id_t pid = 0;
  if (!fts_read_object_id(&pid, tail)) return false;
  const char *sep = strchr(tail, '_');
  if (sep == nullptr) return false;
  space_index_t iid = 0;
  if (!fts_read_object_id(&iid, sep + 1)) return false;

  if (parent_id_out != nullptr) *parent_id_out = pid;
  if (index_id_out != nullptr) *index_id_out = iid;
  if (type_out != nullptr) *type_out = type;
  return true;
}

bool vec_aux_is_aux_table_name(const char *name) {
  return vec_aux_parse_table_name(name, nullptr, nullptr, nullptr);
}

bool vec_aux_table_has_vector_index(const dict_table_t *table) {
  if (table == nullptr) return false;
  for (const dict_index_t *idx = UT_LIST_GET_FIRST(table->indexes);
       idx != nullptr; idx = UT_LIST_GET_NEXT(indexes, idx)) {
    if (idx->is_vector()) return true;
  }
  return false;
}

void vec_add_aux_id_column(dict_table_t *table, mem_heap_t *heap) {
  dict_mem_table_add_col(
      table, heap, VEC_AUX_ID_COL_NAME, DATA_INT,
      dtype_form_prtype(DATA_NOT_NULL | DATA_UNSIGNED | DATA_BINARY_TYPE, 0),
      sizeof(uint64_t), false);
  DICT_TF2_FLAG_SET(table, DICT_TF2_HAS_VEC_AUX_COL);
}

namespace {

/** Allocate and fully populate the in-memory dict_table_t for one vector
aux table. Schema is fixed:
  id        BIGINT UNSIGNED NOT NULL PRIMARY KEY,
  vec       BLOB NOT NULL,
  base_pk   BIGINT UNSIGNED NOT NULL, -- the base row this node describes
  level     TINYINT NOT NULL,
  neighbors BLOB NOT NULL */
dict_table_t *create_in_mem_vec_aux_table(const char *aux_name,
                                          const dict_table_t *parent,
                                          mem_heap_t *heap) {
  dict_table_t *t =
      dict_mem_table_create(aux_name, parent->space, VEC_AUX_TABLE_NUM_COLS, 0,
                            0, parent->flags, aux_flags2_from_parent(parent));

  if (DICT_TF_HAS_SHARED_SPACE(parent->flags)) {
    ut_ad(parent->space == fil_space_get_id_by_name(parent->tablespace()));
    t->tablespace = mem_heap_strdup(t->heap, parent->tablespace);
  }
  if (DICT_TF_HAS_DATA_DIR(parent->flags)) {
    ut_ad(parent->data_dir_path != nullptr);
    t->data_dir_path = mem_heap_strdup(t->heap, parent->data_dir_path);
  }

  /* id BIGINT UNSIGNED NOT NULL */
  dict_mem_table_add_col(t, heap, "id", DATA_INT, DATA_NOT_NULL | DATA_UNSIGNED,
                         VEC_AUX_ID_COL_LEN, true);

  /* vec BLOB NOT NULL */
  dict_mem_table_add_col(
      t, heap, "vec", DATA_BLOB,
      (DATA_MTYPE_MAX << 16) | DATA_BINARY_TYPE | DATA_NOT_NULL,
      VEC_AUX_VEC_COL_LEN, true);

  /* base_pk BIGINT UNSIGNED NOT NULL — the base row this node describes.
  Stored here rather than reached through a secondary index on the base
  table: a secondary index carries no per-record trx_id, so its MVCC leans
  on PAGE_MAX_TRX_ID with a clustered fallback. Keeping the primary key in
  the aux row makes the lookup a plain point read. */
  dict_mem_table_add_col(t, heap, "base_pk", DATA_INT,
                         DATA_NOT_NULL | DATA_UNSIGNED, VEC_AUX_BASE_PK_COL_LEN,
                         true);

  /* level TINYINT NOT NULL — stored as 1-byte INT */
  dict_mem_table_add_col(t, heap, "level", DATA_INT, DATA_NOT_NULL,
                         VEC_AUX_LEVEL_COL_LEN, true);

  /* neighbors BLOB NOT NULL */
  dict_mem_table_add_col(
      t, heap, "neighbors", DATA_BLOB,
      (DATA_MTYPE_MAX << 16) | DATA_BINARY_TYPE | DATA_NOT_NULL,
      VEC_AUX_NEIGHBORS_COL_LEN, true);

  return t;
}

}  // namespace

dberr_t vec_aux_create_one_table(trx_t *trx, const dict_table_t *parent,
                                 space_index_t index_id) {
  ut_a(trx != nullptr);
  ut_a(parent != nullptr);

  char aux_name[MAX_FULL_NAME_LEN];
  vec_aux_get_table_name(parent, index_id, Vec_index_type::HNSW, aux_name,
                         sizeof(aux_name));

  mem_heap_t *heap = mem_heap_create(1024, UT_LOCATION_HERE);
  dict_table_t *aux = create_in_mem_vec_aux_table(aux_name, parent, heap);

  dberr_t err = row_create_table_for_mysql(aux, nullptr, nullptr, trx, nullptr);

  if (err == DB_SUCCESS) {
    dict_index_t *cidx =
        dict_mem_index_create(aux_name, "VEC_AUX_TABLE_PK", aux->space,
                              DICT_UNIQUE | DICT_CLUSTERED, 1);
    cidx->add_field("id", 0, true);

    const trx_dict_op_t saved_op = trx_get_dict_operation(trx);
    err = row_create_index_for_mysql(cidx, trx, nullptr, nullptr);
    trx->dict_operation = saved_op;
  }

  mem_heap_free(heap);

  if (err != DB_SUCCESS) {
    trx->error_state = err;
    ib::warn(ER_IB_MSG_465) << "Failed to create vector aux table " << aux_name;
  }
  return err;
}

dberr_t vec_aux_create_all_tables(trx_t *trx, const dict_table_t *parent) {
  ut_a(trx != nullptr);
  ut_a(parent != nullptr);

  for (const dict_index_t *idx = UT_LIST_GET_FIRST(parent->indexes);
       idx != nullptr; idx = UT_LIST_GET_NEXT(indexes, idx)) {
    if (!idx->is_vector()) continue;
    dberr_t err = vec_aux_create_one_table(trx, parent, idx->id);
    if (err != DB_SUCCESS) return err;
  }
  return DB_SUCCESS;
}

bool vec_aux_create_dd_tables(dict_table_t *parent) {
  ut_a(parent != nullptr);

  /* DEVIATION FROM FTS: fts_create_index_dd_tables (fts0fts.cc) gates
  each iteration on `index->fill_dd` so a re-entrant CREATE-time call
  registers exactly the pending aux tables. Vec has no `fill_dd` per-
  index gate because PS-11264 currently allows at most one vector
  index per table (see dd::create_dd_table validation) — so the loop
  either finds zero vec indexes or exactly one, and idempotency isn't
  a concern. If phase 2 lifts the one-vec-index cap AND supports
  partial DD materialization, mirror fts's fill_dd gate here. */
  for (const dict_index_t *idx = UT_LIST_GET_FIRST(parent->indexes);
       idx != nullptr; idx = UT_LIST_GET_NEXT(indexes, idx)) {
    if (!idx->is_vector()) continue;

    char aux_name[MAX_FULL_NAME_LEN];
    vec_aux_get_table_name(parent, idx->id, Vec_index_type::HNSW, aux_name,
                           sizeof(aux_name));
    dict_table_t *aux = dd_table_open_on_name_in_mem(aux_name, false);
    ut_a(aux != nullptr);
    const bool ok = dd_create_vec_aux_table(parent, aux);
    dd_table_close(aux, nullptr, nullptr, false);
    if (!ok) return false;
  }
  return true;
}

dberr_t vec_aux_drop_one_table(trx_t *trx, const dict_table_t *parent,
                               space_index_t index_id) {
  ut_a(trx != nullptr);
  ut_a(parent != nullptr);

  char aux_name[MAX_FULL_NAME_LEN];
  vec_aux_get_table_name(parent, index_id, Vec_index_type::HNSW, aux_name,
                         sizeof(aux_name));

  const bool file_per_table = dict_table_is_file_per_table(parent);

  dberr_t err = row_drop_table_for_mysql(aux_name, trx, false, nullptr);
  if (err != DB_SUCCESS && err != DB_TABLE_NOT_FOUND) {
    ib::warn(ER_IB_MSG_466) << "Failed to drop vector aux table " << aux_name
                            << " err=" << static_cast<int>(err);
    return err;
  }

  /* row_drop_table_for_mysql only tears down dict_sys + the .ibd. The
  matching dd::Table + dd::Tablespace entries created by
  dd_create_vec_aux_table linger until we explicitly drop them; reuse
  dd_drop_fts_table for that, which is generic across aux-table kinds.
  dict_sys mutex must be released around the DD client call.

  DEVIATION FROM FTS: fts_drop_table drops the DD entry inline only
  when called with aux_vec == nullptr; on the DROP TABLE path it
  instead pushes the aux name into aux_vec and the caller
  (row_drop_table_for_mysql's funct_exit) drops the DD entries AFTER
  the parent drop trx commits. Vec has no aux_vec mode — the DD drop
  always happens here, potentially under an open parent-drop trx.
  Acceptable in phase 1 (empty aux, one aux per index, no partial-
  batch window); the aux_vec deferral is the upgrade path if
  PS-11300's crash-atomicity work needs it. */
  const bool dict_locked = trx->dict_operation_lock_mode == RW_X_LATCH;
  if (dict_locked) {
    dict_sys_mutex_exit();
  }
  (void)dd_drop_fts_table(aux_name, file_per_table);
  if (dict_locked) {
    dict_sys_mutex_enter();
  }

  /* Treat NOT_FOUND from the in-memory drop as success — covers tables
  created before this code landed. */
  return err == DB_TABLE_NOT_FOUND ? DB_SUCCESS : err;
}

dberr_t vec_aux_drop_all_tables(trx_t *trx, dict_table_t *parent) {
  ut_a(trx != nullptr);
  ut_a(parent != nullptr);

  for (const dict_index_t *idx = UT_LIST_GET_FIRST(parent->indexes);
       idx != nullptr; idx = UT_LIST_GET_NEXT(indexes, idx)) {
    if (!idx->is_vector()) continue;
    dberr_t err = vec_aux_drop_one_table(trx, parent, idx->id);
    if (err != DB_SUCCESS) return err;
  }
  return DB_SUCCESS;
}

void vec_aux_detach_tables(const dict_table_t *parent, bool dict_locked) {
  ut_a(parent != nullptr);

  if (!dict_locked) {
    dict_sys_mutex_enter();
  }

  for (const dict_index_t *idx = UT_LIST_GET_FIRST(parent->indexes);
       idx != nullptr; idx = UT_LIST_GET_NEXT(indexes, idx)) {
    if (!idx->is_vector()) continue;

    char aux_name[MAX_FULL_NAME_LEN];
    vec_aux_get_table_name(parent, idx->id, Vec_index_type::HNSW, aux_name,
                           sizeof(aux_name));

    dict_table_t *aux = dd_table_open_on_name_in_mem(aux_name, true);
    if (aux != nullptr) {
      if (!aux->can_be_evicted) {
        dict_table_allow_eviction(aux);
      }
      dd_table_close(aux, nullptr, nullptr, true);
    }
  }

  if (!dict_locked) {
    dict_sys_mutex_exit();
  }
}

namespace {

/** Build the post-rename aux name. Given the OLD aux name
"old_db/vec_<tid>_<iid>" and the parent's NEW name "new_db/<tbl>", write
"new_db/vec_<tid>_<iid>" into `out`. Mirrors what fts_rename_one_aux_table
does inline. */
void rebuild_aux_name_with_new_db(const char *old_aux_name,
                                  const char *new_parent_name, char *out,
                                  size_t out_len) {
  const ulint new_db_len = dict_get_db_name_len(new_parent_name);
  const ulint old_db_len = dict_get_db_name_len(old_aux_name);
  /* +1 for the slash; +1 for NUL */
  const size_t needed = strlen(old_aux_name) + new_db_len - old_db_len + 1;
  ut_a(needed <= out_len);

  memcpy(out, new_parent_name, new_db_len);
  const char *old_slash = strchr(old_aux_name, '/');
  ut_a(old_slash != nullptr);
  const size_t suffix_len = strlen(old_slash); /* includes leading '/' */
  memcpy(out + new_db_len, old_slash, suffix_len + 1 /* NUL */);
}

}  // namespace

dberr_t vec_aux_rename_tables(trx_t *trx, dict_table_t *parent,
                              const char *new_parent_name, bool replay) {
  ut_a(trx != nullptr);
  ut_a(parent != nullptr);
  ut_a(new_parent_name != nullptr);

  for (const dict_index_t *idx = UT_LIST_GET_FIRST(parent->indexes);
       idx != nullptr; idx = UT_LIST_GET_NEXT(indexes, idx)) {
    if (!idx->is_vector()) continue;

    char old_aux_name[MAX_FULL_NAME_LEN];
    vec_aux_get_table_name(parent, idx->id, Vec_index_type::HNSW, old_aux_name,
                           sizeof(old_aux_name));

    char new_aux_name[MAX_FULL_NAME_LEN];
    rebuild_aux_name_with_new_db(old_aux_name, new_parent_name, new_aux_name,
                                 sizeof(new_aux_name));

    dberr_t err = row_rename_table_for_mysql(old_aux_name, new_aux_name,
                                             nullptr, trx, replay);
    if (err != DB_SUCCESS) {
      ib::warn(ER_IB_MSG_466)
          << "Failed to rename vector aux table " << old_aux_name << " -> "
          << new_aux_name << " err=" << static_cast<int>(err);
      return err;
    }

    /* Update the DD entry (dd::Table parent schema_id + dd::Tablespace
    file_name) — reuses dd_rename_fts_table since aux tables are
    DD-registered with the same shape. dict_sys mutex must be released
    around the DD client call. */
    if (!replay) {
      dict_table_t *aux = dict_table_check_if_in_cache_low(new_aux_name);
      ut_ad(aux != nullptr);
      if (aux != nullptr) {
        aux->acquire();
        dict_sys_mutex_exit();
        const bool ok = dd_rename_fts_table(aux, old_aux_name);
        dict_sys_mutex_enter();
        aux->release();
        if (!ok) {
          ib::warn(ER_IB_MSG_466)
              << "Failed to rename DD entry for vector aux " << old_aux_name;
          return DB_ERROR;
        }
      }
    }
  }
  return DB_SUCCESS;
}
