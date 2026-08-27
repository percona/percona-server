/*****************************************************************************

Copyright (c) 2026, Percona Inc.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/

/**
@file vec/vec0hnsw.cc
The HNSW runtime and the persistence callbacks behind it.
*/

#include "univ.i"

#include "vec0hnsw.h"

#include <variant>
#include "dict0dict.h"
#include "mach0data.h"
#include "sql/field.h"
#include "sql/table.h"
#include "ut0new.h"
#include "vec0aux.h"
#include "vec0dml.h"

#include "vector-common/vector_distance.h"

vec_t::~vec_t() {
  /* Destroying the graph destroys the arena it holds by value, which
  frees every node in one go — the arena has no per-block free and does
  not need one. */
  ut::delete_(hnsw);
  hnsw = nullptr;
}

dberr_t vec_persist_insert(Vec_ctx *ctx, uint64_t id, uint64_t base_pk,
                           const char *q, uint8_t layer,
                           const std::vector<byte> &neighbors) {
  ut_a(ctx->aux != nullptr);
  ut_a(id != 0); /* 0 is the empty-slot sentinel; record 0 is metadata */

  vec_aux_row_t row;
  row.id = id;
  row.vec = reinterpret_cast<const float *>(q);
  row.dims = ctx->vec_bytes / sizeof(float);
  row.base_pk = base_pk;
  row.level = layer;
  row.neighbors = neighbors.data();
  row.neighbors_len = neighbors.size();

  return vec_aux_insert(ctx->trx, ctx->aux, row);
}

dberr_t vec_persist_update_neighbors(Vec_ctx *ctx, uint64_t id,
                                     const std::vector<byte> &neighbors) {
  ut_a(ctx->aux != nullptr);
  return vec_aux_update_row(ctx->trx, ctx->aux, id, neighbors.data(),
                            neighbors.size());
}

dberr_t vec_persist_entry_point(Vec_ctx *ctx, uint64_t id) {
  ut_a(ctx->aux != nullptr);

  /* The entry point lives in aux record 0. Record 0 can never collide
  with a node: the class reserves graph node id 0 as its empty-slot
  sentinel, so no node is ever assigned it. base_pk carries the entry
  point's id; vec and neighbors are empty, level is 0.

  Update first, insert on miss. After the very first node, update is the
  common case, and this fires only when the graph's top layer changes —
  a couple of hundred times over an index's life at most. */
  byte buf[8];
  mach_write_to_8(buf, id);
  const uint64_t entry = id;

  dberr_t err = vec_aux_update_row(ctx->trx, ctx->aux, 0, nullptr, 0, &entry);
  if (err != DB_RECORD_NOT_FOUND) return err;

  vec_aux_row_t row;
  row.id = 0;
  row.vec = nullptr;
  row.dims = 0;
  row.base_pk = id;
  row.level = 0;
  row.neighbors = nullptr;
  row.neighbors_len = 0;
  return vec_aux_insert(ctx->trx, ctx->aux, row);
}

vec_t *vec_runtime_open(dict_index_t *index, const KEY *key, const TABLE *form,
                        THD *thd) {
  ut_a(index != nullptr);
  ut_a(index->is_vector());
  ut_a(key != nullptr);

  if (index->vec != nullptr) {
    return static_cast<vec_t *>(index->vec);
  }

  /* The values the user wrote in WITH(...), round-tripped through the
  DD and parsed by the open-time overload added for exactly this. */
  storage::innobase::vec::VectorIndexParam vip;
  if (storage::innobase::vec::parse_options(*key, vip)) {
    return nullptr;
  }
  const auto *hnsw_param = std::get_if<storage::innobase::vec::HnswParam>(&vip);
  if (hnsw_param == nullptr) {
    return nullptr;
  }

  /* Dimension is a property of the column, not of WITH(...), so it has
  to come from the Field.

  key_part[0].field is not usable directly — for a vector key part
  get_index_prefix_len() reports 1, so the KEY_PART_INFO describes a
  1-byte prefix rather than the column. Its field_index() is still
  correct, though, and indexing form->field with it is exactly the dance
  create_index() does (ha_innodb.cc) to see past a forged prefix
  field. */
  ut_a(key->user_defined_key_parts == 1);
  const Field *f = form->field[key->key_part[0].field->field_index()];
  if (f == nullptr || f->type() != MYSQL_TYPE_VECTOR) return nullptr;
  const Field_vector *field = down_cast<const Field_vector *>(f);

  const uint32_t dims = field->get_max_dimensions();
  if (dims == 0 || dims == UINT32_MAX) {
    return nullptr;
  }

  auto *vec = ut::new_withkey<vec_t>(UT_NEW_THIS_FILE_PSI_KEY);
  if (vec == nullptr) return nullptr;

  vec->index_id = index->id;
  vec->table = index->table;
  vec->dims = dims;
  vec->m = static_cast<uint32_t>(hnsw_param->M);
  vec->ef_construction = static_cast<uint32_t>(hnsw_param->ef_construction);
  vec->loaded = false;

  (void)thd;
  index->vec = vec;
  return vec;
}
