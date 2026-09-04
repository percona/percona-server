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

#include <algorithm>

#include "srv0srv.h"

#include <variant>
#include "dict0dd.h"
#include "dict0dict.h"
#include "mach0data.h"
#include "sql/field.h"
#include "sql/table.h"
#include "trx0roll.h"
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

/** Commit the aux sub-transaction and immediately start a fresh one, so that
no row lock taken by a callback outlives that callback.

This is what keeps concurrent INSERTs from aborting each other. One
transaction spanning the whole graph insert holds an X lock on every row it
touches until `insert()` returns, and the rows nearest the entry point are
rewired by almost every insert — so inserts queue on them and are rolled back
by the deadlock detector or the lock wait timeout. Six connections inserting
120 rows each committed 286 of 720 before this; afterwards, 720 of 720 with no
lock waits at all.

Deadlock becomes impossible rather than merely rarer. Each mini-transaction
takes one row lock, having waited for it holding nothing, and then commits, so
no transaction ever waits while holding — which is the precondition for a
cycle.

The graph is unaffected: `lock_node` already serialises every callback
(hnsw.h), and that is what actually protects a node's row. The
transaction contributes undo and redo, not exclusion.

What is given up is per-insert atomicity: a callback failing midway leaves the
earlier callbacks committed, so the aux keeps a node whose base row may never
commit. That is the orphan the design's "Rollback, and why orphans are
acceptable" accepts and filters at read
time — and it is the better direction to diverge in, because the in-memory
rewire cannot be undone either. Rolling the whole insert back left memory
holding a node the aux had discarded. */
static void vec_ctx_step_commit(Vec_ctx *ctx) {
  trx_commit_for_mysql(ctx->trx);
  trx_start_internal(ctx->trx, UT_LOCATION_HERE);
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

  const dberr_t err = vec_aux_insert(ctx->trx, ctx->aux, row);
  if (err == DB_SUCCESS) vec_ctx_step_commit(ctx);
  return err;
}

dberr_t vec_persist_update_neighbors(Vec_ctx *ctx, uint64_t id,
                                     const std::vector<byte> &neighbors) {
  ut_a(ctx->aux != nullptr);
  const dberr_t err = vec_aux_update_row(ctx->trx, ctx->aux, id,
                                         neighbors.data(), neighbors.size());

  /* A neighbour with no aux row yet is not an error.

  The graph is shared between transactions; the aux writes are not. Two
  concurrent inserts see each other's nodes in memory the moment they are
  linked, but each writes its own rows on its own sub-transaction. So this
  insert can be asked to rewire a neighbour whose row belongs to an insert
  that rolled back — or, in the window before its callback commits, one that
  has not committed yet.

  Skipping costs one edge on disk, which is the divergence section 13
  already accepts: a cost in recall, never in correctness, and repaired by
  the next insert that rewires the same neighbourhood. Failing instead
  would let one statement abort another's, at random, purely because they
  landed near each other in the graph. */
  if (err == DB_RECORD_NOT_FOUND) return DB_SUCCESS;
  if (err == DB_SUCCESS) vec_ctx_step_commit(ctx);
  return err;
}

dberr_t vec_persist_entry_point(Vec_ctx *ctx, uint64_t id) {
  ut_a(ctx->aux != nullptr);

  /* This callback commits on its own, after the node's row already has
  (vec_ctx_step_commit), so a crash between the two leaves record 0 naming
  the *previous* entry point while a higher node exists. HNSW::validate()
  reports that as inconsistent — "entry point must sit on the highest
  layer" — so a debug build can flag it. It is not a bug, for three
  reasons.

  The lag is bounded at one layer. random_layer() caps a draw at
  current_max_layer + 1 (hnsw.h), so a node can never be created more
  than one layer above the entry point of the moment, which is exactly what
  record 0 still names.

  It only ever involves an orphan. Every write here happens during the
  statement, below the LSN of the user's commit, so any base row that
  actually committed has its entry-point update durable too. A lagging
  record 0 therefore belongs to a node whose row never became visible, and
  checks 1 and 2 refuse to return it regardless.

  And it self-heals through the ordinary growth path, not a repair path:
  because the cap is max_layer + 1, the hierarchy always climbs one layer
  at a time, so the next insert drawing above the current entry point takes
  the spot and rewrites record 0. */

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
  if (err == DB_SUCCESS) {
    vec_ctx_step_commit(ctx);
    return err;
  }
  if (err != DB_RECORD_NOT_FOUND) return err;

  vec_aux_row_t row;
  row.id = 0;
  row.vec = nullptr;
  row.dims = 0;
  row.base_pk = id;
  row.level = 0;
  row.neighbors = nullptr;
  row.neighbors_len = 0;
  err = vec_aux_insert(ctx->trx, ctx->aux, row);
  if (err == DB_SUCCESS) vec_ctx_step_commit(ctx);
  return err;
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

/** Open the aux table for one DML operation.

No MDL on the aux itself: the caller holds MDL on the BASE table, and
every DDL that can drop an aux takes exclusive base MDL first. That is
the same protection argument FTS relies on for its own aux DML. Fast
path is the dict cache; fall back to the DD only when it has been
evicted, and then take MDL because the fallback can block. */
static dict_table_t *vec_aux_open_for_dml(dict_table_t *base,
                                          space_index_t index_id, THD *thd,
                                          MDL_ticket **mdl) {
  char aux_name[MAX_FULL_NAME_LEN];
  vec_aux_get_table_name(base, index_id, Vec_index_type::HNSW, aux_name,
                         sizeof(aux_name));

  *mdl = nullptr;
  dict_table_t *aux = dd_table_open_on_name_in_mem(aux_name, false);
  if (aux == nullptr && thd != nullptr) {
    aux =
        dd_table_open_on_name(thd, mdl, aux_name, false, DICT_ERR_IGNORE_NONE);
  }
  return aux;
}

static void vec_aux_close_for_dml(dict_table_t *aux, THD *thd,
                                  MDL_ticket **mdl) {
  dd_table_close(aux, *mdl != nullptr ? thd : nullptr, mdl, false);
}

/** Build the graph for an open runtime.

Only the entry point is read here. Every other node is faulted in on
demand when traversal reaches it, which is what init_from_entry_point
means — the alternative, reading every row at startup, is the "huge load
operation at the moment the index is first used after restart" this
design exists to avoid.

An aux with no record 0 is an EMPTY index, not a broken one: record 0 is
written when the first node is inserted, so its absence means no node
has ever been inserted. */
static dberr_t vec_runtime_load(vec_t *vec, dict_table_t *aux, THD *thd) {
  ut_a(vec->hnsw == nullptr);

  vec->hnsw = ut::new_withkey<Vec_hnsw>(UT_NEW_THIS_FILE_PSI_KEY, vec->dims,
                                        &vector_distance_euclidean_squared,
                                        vec->m, vec->ef_construction);
  if (vec->hnsw == nullptr) return DB_OUT_OF_MEMORY;

  mem_heap_t *heap = mem_heap_create(256, UT_LOCATION_HERE);
  vec_aux_read_t meta;
  const dberr_t err = vec_aux_read_node(aux, 0, heap, &meta);
  const uint64_t entry_point = meta.base_pk;
  mem_heap_free(heap);

  if (err == DB_RECORD_NOT_FOUND) {
    /* Empty index. The graph stays empty and the first insert will
    write record 0. */
    vec->loaded.store(true, std::memory_order_release);
    return DB_SUCCESS;
  }
  if (err != DB_SUCCESS) {
    ut::delete_(vec->hnsw);
    vec->hnsw = nullptr;
    return err;
  }

  Vec_ctx ctx;
  ctx.aux = aux;
  ctx.thd = thd;
  ctx.m = vec->m;
  ctx.vec_bytes = vec->dims * sizeof(float);
  ctx.err = DB_SUCCESS;

  vec->hnsw->init_from_entry_point(entry_point, &ctx);
  if (ctx.err != DB_SUCCESS) {
    ut::delete_(vec->hnsw);
    vec->hnsw = nullptr;
    return ctx.err;
  }

  vec->loaded.store(true, std::memory_order_release);
  return DB_SUCCESS;
}

/** Read one row's vector column as raw float bytes.
@return the bytes, or nullptr if the row has no usable vector */
static const char *vec_row_vector_bytes(const dict_index_t *index,
                                        const dtuple_t *row, ulint *len) {
  /* The column the index covers, taken from the index rather than
  searched for.

  A DICT_VECTOR index carries its key part like any other index:
  dict_index_add_col() runs for it on the CREATE path (create_index,
  ha_innodb.cc) and on the DD-open path (dd_fill_one_dict_index,
  dict0dd.cc), and dict_index_build_internal_vec() copies those fields
  into the cached index, zeroing only n_uniq — a vector index has no
  B-tree ordering, but it does have its field.

  Searching for the column instead cannot work: VECTOR, BLOB, TEXT and
  JSON all map to DATA_BLOB in the dictionary, so any blob ordered ahead
  of the vector column would win.

  Ignore the field's prefix_len — get_index_prefix_len() reports 1 for a
  vector key part, which describes nothing about the column. */
  ut_a(index->n_fields == 1);
  const ulint col_no = dict_col_get_no(index->get_field(0)->col);
  ut_a(col_no < dtuple_get_n_fields(row));

  const dfield_t *df = dtuple_get_nth_field(row, col_no);
  if (dfield_is_null(df)) return nullptr;
  *len = dfield_get_len(df);
  return static_cast<const char *>(dfield_get_data(df));
}

/** Insert one node into the graph and, through the persistor, the aux.

Shared by INSERT and by a vector-column UPDATE, because to the graph
they are the same operation: a node is immutable, so a changed vector is
a new node rather than an edit of the old one. */
static dberr_t vec_add_node(vec_t *vec, dict_table_t *table, uint64_t label,
                            uint64_t base_pk, const char *q, THD *thd) {
  /* innodb_hnsw_max_memory, checked BEFORE insert() starts mutating.

  Vec_arena::allocate() is the single point every graph byte passes
  through and would be the natural place to refuse — but refusing there
  returns nullptr, which hnsw.h turns into a throw (four sites, e.g.
  Node::create) partway through a rewire, with neighbours already
  relinked and no per-block free to unwind with. So the refusal happens
  here instead, at the entry to the operation, where nothing has been
  touched yet and DB_OUT_OF_MEMORY simply fails the statement.

  This is a charge check, not a prediction: it asks whether the budget is
  already spent, not whether this insert would fit. Sizing the insert is
  not possible from outside the class — sizeof(Node) is private, and one
  insert also allocates stubs for lazily loaded neighbours and a copy of
  the query vector. The budget can therefore be exceeded by at most what
  one insert allocates, which is the price of refusing before mutating
  rather than during. */
  if (srv_hnsw_max_memory != 0 &&
      vec_arena_global_bytes() >= srv_hnsw_max_memory) {
    return DB_OUT_OF_MEMORY;
  }

  MDL_ticket *mdl = nullptr;
  dict_table_t *aux = vec_aux_open_for_dml(table, vec->index_id, thd, &mdl);
  if (aux == nullptr) return DB_TABLE_NOT_FOUND;

  /* The sub-transaction. Aux writes must not roll back with the
  statement: the graph is an in-memory cache whose only durable form is
  the aux, and a node surviving in memory while its rows rolled back
  would leave the two permanently disagreeing.

  One trx_t, reused rather than one per callback: each callback commits it
  and starts it again (vec_ctx_step_commit), so the object is allocated once
  per insert while the locks live only as long as the callback that took
  them. */
  trx_t *aux_trx = trx_allocate_for_background();

  /* Never fsync the redo log for the aux sub-transaction.

  Its records sit in the log buffer below the user statement's LSN, and the
  user's commit calls log_write_up_to() for its own higher LSN, which makes
  everything below durable, ours included. The invariant "aux superset of
  committed base rows" therefore holds by LSN ordering, and a flush here
  buys nothing. If the user's transaction never commits, the aux rows are an
  orphan at worst, which the design's rollback section accepts.

  This is what makes committing per callback affordable. Measured on an idle
  128-core box, RelWithDebInfo, 40000 single-threaded inserts: 8.5s for one
  commit per insert, 17.7s for one per callback, and 8.55s for one per
  callback with this flag — the entire cost of the extra commits was the
  fsync.

  trx_commit_low honours it by setting must_flush_log_later instead of
  calling trx_flush_log_if_needed (trx0trx.cc). Only
  trx_commit_complete_for_mysql consumes that, and it is reached solely from
  the user-transaction handler path (ha_innodb.cc), never by a
  background trx, so the deferred flush is simply never performed. */
  aux_trx->flush_log_later = true;

  trx_start_internal(aux_trx, UT_LOCATION_HERE);

  Vec_ctx ctx;
  ctx.trx = aux_trx;
  ctx.aux = aux;
  ctx.thd = thd;
  ctx.m = vec->m;
  ctx.vec_bytes = vec->dims * sizeof(float);
  ctx.err = DB_SUCCESS;

  /* Lazy build, once. Double-checked on the atomic, so a warm index takes
  no lock here and none at all below. */
  if (!vec->loaded.load(std::memory_order_acquire)) {
    std::lock_guard<std::mutex> g(vec->load_mutex);
    if (!vec->loaded.load(std::memory_order_relaxed)) {
      const dberr_t lerr = vec_runtime_load(vec, aux, thd);
      if (lerr != DB_SUCCESS) {
        trx_rollback_to_savepoint(aux_trx, nullptr);
        trx_free_for_background(aux_trx);
        vec_aux_close_for_dml(aux, thd, &mdl);
        return lerr;
      }
    }
  }

  /* Unlocked: the class is thread-safe for concurrent insert() and search,
  and the only operation it is not thread-safe for — init_from_entry_point —
  cannot be running, because reaching here means `loaded` is already true. */
  vec->hnsw->insert(label, base_pk, q, &ctx);

  /* Commits whatever the last callback left open — often nothing, since
  each callback commits its own work. */
  if (ctx.err == DB_SUCCESS) {
    trx_commit_for_mysql(aux_trx);
  } else {
    /* trx_rollback_to_savepoint, not trx_rollback_for_mysql: the aux
    transaction is a BACKGROUND trx, so it is not in the MySQL trx list
    that trx_rollback_for_mysql asserts membership of. This is the call
    fts_sql_rollback makes, for that same reason (fts0sql.cc).

    This now rolls back only the callback that failed: everything before it
    was committed by vec_ctx_step_commit. The earlier rows stand, which is
    the orphan the design's rollback section accepts, and is the direction
    that keeps the aux
    tracking memory rather than diverging from it. */
    trx_rollback_to_savepoint(aux_trx, nullptr);
  }
  trx_free_for_background(aux_trx);
  vec_aux_close_for_dml(aux, thd, &mdl);

  return ctx.err;
}

dict_index_t *vec_index_of(dict_table_t *table) {
  for (dict_index_t *index = table->first_index(); index != nullptr;
       index = index->next()) {
    if (index->is_vector()) return index;
  }
  return nullptr;
}

uint32_t vec_index_dims(const dict_index_t *index) {
  if (index == nullptr || index->vec == nullptr) return 0;
  return static_cast<const vec_t *>(index->vec)->dims;
}


dberr_t vec_knn_search(dict_index_t *index, const float *q, size_t k,
                       size_t ef_search, std::vector<vec_hit_t> *out, THD *thd,
                       const std::unordered_set<uint64_t> *exclude) {
  ut_a(index != nullptr && index->is_vector());
  ut_a(q != nullptr && out != nullptr);
  out->clear();

  if (index->vec == nullptr) return DB_TABLE_NOT_FOUND;
  auto *vec = static_cast<vec_t *>(index->vec);

  MDL_ticket *mdl = nullptr;
  dict_table_t *aux =
      vec_aux_open_for_dml(vec->table, vec->index_id, thd, &mdl);
  if (aux == nullptr) return DB_TABLE_NOT_FOUND;

  if (!vec->loaded.load(std::memory_order_acquire)) {
    std::lock_guard<std::mutex> g(vec->load_mutex);
    if (!vec->loaded.load(std::memory_order_relaxed)) {
      const dberr_t lerr = vec_runtime_load(vec, aux, thd);
      if (lerr != DB_SUCCESS) {
        vec_aux_close_for_dml(aux, thd, &mdl);
        return lerr;
      }
    }
  }

  Vec_ctx ctx;
  ctx.trx = nullptr;
  ctx.aux = aux;
  ctx.thd = thd;
  ctx.m = vec->m;
  ctx.vec_bytes = vec->dims * sizeof(float);
  ctx.err = DB_SUCCESS;

  /* Unlocked, like the insert path. A search does mutate — it faults
  unloaded stubs in through load_node_cb — but load_node() takes a striped
  lock and re-checks the node state under it, so two threads faulting the
  same node cannot collide. */
  {
    const size_t want = exclude == nullptr ? k : k + exclude->size();
    const auto hits =
        vec->hnsw->k_nn_search(reinterpret_cast<const char *>(q), want,
                               std::max(ef_search, want), &ctx);
    out->reserve(hits.size());
    for (const auto &h : hits) out->push_back({h.id, h.base_pk});
  }

  if (exclude != nullptr && !out->empty()) {
    out->erase(std::remove_if(out->begin(), out->end(),
                              [exclude](const vec_hit_t &h) {
                                return exclude->count(h.id) != 0;
                              }),
               out->end());
  }

  const dberr_t err = ctx.err;
  vec_aux_close_for_dml(aux, thd, &mdl);
  return err;
}

/* An open streaming scan. Held by the handler for the life of one
vector scan, which is why the aux table and its MDL live here rather than
being re-taken per batch: nn_search_next faults nodes in through
load_node_cb, and that reads ctx.aux. */
struct vec_search_t {
  vec_t *vec{nullptr};
  dict_table_t *aux{nullptr};
  MDL_ticket *mdl{nullptr};
  THD *thd{nullptr};
  Vec_ctx ctx;
  Vec_hnsw::NNSearchContext nn;
};

dberr_t vec_knn_open(dict_index_t *index, const float *q, size_t batch_size,
                     size_t ef_search, THD *thd, vec_search_t **out) {
  ut_a(index != nullptr && index->is_vector());
  ut_a(q != nullptr && out != nullptr);
  ut_a(batch_size > 0);
  *out = nullptr;

  if (index->vec == nullptr) return DB_TABLE_NOT_FOUND;
  auto *vec = static_cast<vec_t *>(index->vec);

  MDL_ticket *mdl = nullptr;
  dict_table_t *aux =
      vec_aux_open_for_dml(vec->table, vec->index_id, thd, &mdl);
  if (aux == nullptr) return DB_TABLE_NOT_FOUND;

  if (!vec->loaded.load(std::memory_order_acquire)) {
    std::lock_guard<std::mutex> g(vec->load_mutex);
    if (!vec->loaded.load(std::memory_order_relaxed)) {
      const dberr_t lerr = vec_runtime_load(vec, aux, thd);
      if (lerr != DB_SUCCESS) {
        vec_aux_close_for_dml(aux, thd, &mdl);
        return lerr;
      }
    }
  }

  auto *s = ut::new_withkey<vec_search_t>(UT_NEW_THIS_FILE_PSI_KEY);
  if (s == nullptr) {
    vec_aux_close_for_dml(aux, thd, &mdl);
    return DB_OUT_OF_MEMORY;
  }
  s->vec = vec;
  s->aux = aux;
  s->mdl = mdl;
  s->thd = thd;
  s->ctx.trx = nullptr;
  s->ctx.aux = aux;
  s->ctx.thd = thd;
  s->ctx.m = vec->m;
  s->ctx.vec_bytes = vec->dims * sizeof(float);
  s->ctx.err = DB_SUCCESS;

  /* Unlocked, like every other graph access: the class is thread-safe for
  concurrent search, and a search mutates only by faulting stubs in, which
  load_node() serialises under its own striped lock. */
  vec->hnsw->nn_search_start(&s->nn, reinterpret_cast<const char *>(q),
                             batch_size, std::max(ef_search, batch_size),
                             &s->ctx);
  if (s->ctx.err != DB_SUCCESS) {
    const dberr_t err = s->ctx.err;
    vec_knn_close(s);
    return err;
  }

  *out = s;
  return DB_SUCCESS;
}

bool vec_knn_next(vec_search_t *s, vec_hit_t *hit) {
  ut_a(s != nullptr && hit != nullptr);
  if (s->ctx.err != DB_SUCCESS) return false;

  const auto next = s->vec->hnsw->nn_search_next(&s->nn);
  if (s->ctx.err != DB_SUCCESS) return false;
  if (!next.first) return false;

  hit->id = next.second.id;
  hit->base_pk = next.second.base_pk;
  return true;
}

dberr_t vec_knn_error(const vec_search_t *s) {
  return s == nullptr ? DB_SUCCESS : s->ctx.err;
}

void vec_knn_close(vec_search_t *s) {
  if (s == nullptr) return;
  s->nn.reset();
  if (s->aux != nullptr) {
    vec_aux_close_for_dml(s->aux, s->thd, &s->mdl);
  }
  ut::delete_(s);
}

dberr_t vec_build_index(trx_t *trx, dict_table_t *table,
                        dict_index_t *vec_index, uint32_t dims, uint32_t m,
                        uint32_t ef_construction, THD *thd) {
  ut_a(trx != nullptr);
  ut_a(vec_index != nullptr && vec_index->is_vector());
  ut_a(dims != 0 && m != 0);

  /* Same pre-flight as the DML path (design: "Memory limits"): refuse before
  building anything rather than throwing partway through. */
  if (srv_hnsw_max_memory != 0 &&
      vec_arena_global_bytes() >= srv_hnsw_max_memory) {
    return DB_OUT_OF_MEMORY;
  }

  std::vector<vec_base_row_t> rows;
  dberr_t err = vec_base_collect_rows(table, vec_index, dims, &rows);
  if (err != DB_SUCCESS) return err;
  if (rows.empty()) return DB_SUCCESS;

  MDL_ticket *mdl = nullptr;
  dict_table_t *aux = vec_aux_open_for_dml(table, vec_index->id, thd, &mdl);
  if (aux == nullptr) return DB_TABLE_NOT_FOUND;

  Vec_ctx ctx;
  ctx.trx = trx;
  ctx.aux = aux;
  ctx.thd = thd;
  ctx.m = m;
  ctx.vec_bytes = dims * sizeof(float);
  ctx.err = DB_SUCCESS;

  /* A private graph, discarded below. It is not installed on the index:
  a half-built graph must never be reachable, and if the ALTER fails
  there is nothing to unwind. */
  auto *graph = ut::new_withkey<Vec_hnsw>(UT_NEW_THIS_FILE_PSI_KEY, dims,
                                          &vector_distance_euclidean_squared,
                                          m, ef_construction);
  if (graph == nullptr) {
    vec_aux_close_for_dml(aux, thd, &mdl);
    return DB_OUT_OF_MEMORY;
  }

  for (const vec_base_row_t &row : rows) {
    /* Label 0 is the empty-slot sentinel and can never be a node. A row
    carrying it means the stamping path missed it. */
    ut_a(row.id != 0);
    graph->insert(row.id, row.base_pk,
                  reinterpret_cast<const char *>(row.vec.data()), &ctx);
    if (ctx.err != DB_SUCCESS) break;

    if (srv_hnsw_max_memory != 0 &&
        vec_arena_global_bytes() >= srv_hnsw_max_memory) {
      ctx.err = DB_OUT_OF_MEMORY;
      break;
    }
  }

  err = ctx.err;
  ut::delete_(graph);
  vec_aux_close_for_dml(aux, thd, &mdl);
  return err;
}

dberr_t vec_update_row(trx_t *trx [[maybe_unused]], dict_table_t *table,
                       uint64_t label, const char *q, ulint q_len,
                       uint64_t base_pk, THD *thd) {
  ut_a(label != 0);

  for (dict_index_t *index = table->first_index(); index != nullptr;
       index = index->next()) {
    if (!index->is_vector() || index->vec == nullptr) continue;
    auto *vec = static_cast<vec_t *>(index->vec);
    if (q_len != vec->dims * sizeof(float)) return DB_CORRUPTION;
    const dberr_t err = vec_add_node(vec, table, label, base_pk, q, thd);
    if (err != DB_SUCCESS) return err;
  }
  return DB_SUCCESS;
}

dberr_t vec_insert_row(trx_t *trx [[maybe_unused]], dict_table_t *table,
                       const dtuple_t *row, THD *thd) {
  for (dict_index_t *index = table->first_index(); index != nullptr;
       index = index->next()) {
    if (!index->is_vector() || index->vec == nullptr) continue;

    auto *vec = static_cast<vec_t *>(index->vec);

    ulint vec_len = 0;
    const char *q = vec_row_vector_bytes(index, row, &vec_len);
    if (q == nullptr) continue;
    if (vec_len != vec->dims * sizeof(float)) return DB_CORRUPTION;

    const uint64_t label = vec_get_aux_id_from_row(table, row);
    ut_a(label != 0);

    /* base_pk is the base row's PRIMARY KEY, not the label. A search
    returns base_pk so the caller can fetch the row; the label
    identifies the node and is what the read path compares against the
    row's hidden column. The design allows a single-column BIGINT
    UNSIGNED primary key, so it is the first clustered field. */
    const dict_index_t *clust = table->first_index();
    ut_a(dict_index_get_n_unique(clust) == 1);
    const ulint pk_col = clust->get_col_no(0);
    const dfield_t *pk_df = dtuple_get_nth_field(row, pk_col);
    ut_a(!dfield_is_null(pk_df) && dfield_get_len(pk_df) == 8);
    const uint64_t base_pk =
        mach_read_from_8(static_cast<const byte *>(dfield_get_data(pk_df)));

    const dberr_t err = vec_add_node(vec, table, label, base_pk, q, thd);
    if (err != DB_SUCCESS) return err;
  }
  return DB_SUCCESS;
}
