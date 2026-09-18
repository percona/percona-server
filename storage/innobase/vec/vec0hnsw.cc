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
#include "btr0pcur.h"
#include "dict0dd.h"
#include "dict0dict.h"
#include "lock0lock.h"
#include "mach0data.h"
#include "my_dbug.h"
#include "sql/field.h"
#include "sql/table.h"
#include "trx0roll.h"
#include "ut0new.h"
#include "vec0aux.h"
#include "vec0dml.h"

#include "vector-common/vector_distance.h"

vec_t::~vec_t() {
  /* Destroying the graph destroys the arena it holds by value, which
  frees every node in one go - the arena has no per-block free and does
  not need one. */
  ut::delete_(hnsw);
  hnsw = nullptr;
}

/** Commit the aux sub-transaction and immediately start a fresh one, so that
no row lock taken by a callback outlives that callback.

This is what keeps concurrent INSERTs from aborting each other. One
transaction spanning the whole graph insert holds an X lock on every row it
touches until `insert()` returns, and the rows nearest the entry point are
rewired by almost every insert - so inserts queue on them and are rolled back
by the deadlock detector or the lock wait timeout. Six connections inserting
120 rows each committed 286 of 720 before this; afterwards, 720 of 720 with no
lock waits at all.

Deadlock becomes impossible rather than merely rarer. Each mini-transaction
takes one row lock, having waited for it holding nothing, and then commits, so
no transaction ever waits while holding - which is the precondition for a
cycle.

The graph is unaffected: `lock_node` already serialises every callback
(hnsw.h), and that is what actually protects a node's row. The
transaction contributes undo and redo, not exclusion.

What is given up is per-insert atomicity: a callback failing midway leaves the
earlier callbacks committed, so the aux keeps a node whose base row may never
commit. That is the orphan the design's "Rollback, and why orphans are
acceptable" accepts and filters at read
time - and it is the better direction to diverge in, because the in-memory
rewire cannot be undone either. Rolling the whole insert back left memory
holding a node the aux had discarded.

An index build opts out via ctx->commit_aux_trx: there trx is the ALTER's own
transaction, not a sub-transaction, and committing it per callback would
commit the DDL a node at a time. */
static void vec_ctx_step_commit(Vec_ctx *ctx) {
  if (!ctx->commit_aux_trx) return;
  trx_commit_for_mysql(ctx->trx);
  trx_start_internal(ctx->trx, UT_LOCATION_HERE);
}

dberr_t vec_persist_insert(Vec_ctx *ctx, uint64_t id, uint64_t base_pk,
                           const char *q, uint8_t layer,
                           const std::vector<byte> &neighbors) {
  ut_ad(ctx->aux != nullptr);
  ut_ad(id != 0); /* 0 is the empty-slot sentinel; record 0 is metadata */

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
  ut_ad(ctx->aux != nullptr);
  const dberr_t err = vec_aux_update_row(ctx->trx, ctx->aux, id,
                                         neighbors.data(), neighbors.size());

  /* A neighbour with no aux row yet is not an error.

  The graph is shared between transactions; the aux writes are not. Two
  concurrent inserts see each other's nodes in memory the moment they are
  linked, but each writes its own rows on its own sub-transaction. So this
  insert can be asked to rewire a neighbour whose row belongs to an insert
  that rolled back - or, in the window before its callback commits, one that
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
  ut_ad(ctx->aux != nullptr);

  /* This callback commits on its own, after the node's row already has
  (vec_ctx_step_commit), so a crash between the two leaves record 0 naming
  the *previous* entry point while a higher node exists. HNSW::validate()
  reports that as inconsistent - "entry point must sit on the highest
  layer" - so a debug build can flag it. It is not a bug, for three
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
  common case, and this fires only when the graph's top layer changes -
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
  ut_ad(index != nullptr);
  ut_ad(index->is_vector());
  ut_ad(key != nullptr);

  if (vec_t *existing = vec_runtime_get(index); existing != nullptr) {
    return existing;
  }

  /* The values the user wrote in WITH(...), round-tripped through the
  DD and parsed by the open-time overload added for exactly this. */
  storage::innobase::vec::VectorIndexParam vip;
  if (storage::innobase::vec::parse_options(*key, vip)) {
    ib::error(ER_IB_MSG_456)
        << "Failed to open vector runtime for index " << index->name
        << " on table " << index->table->name << ": could not parse the"
        << " index's WITH(...) options; vector search on it will not"
        << " work until the table is reopened.";
    return nullptr;
  }
  const auto *hnsw_param = std::get_if<storage::innobase::vec::HnswParam>(&vip);
  if (hnsw_param == nullptr) {
    ib::error(ER_IB_MSG_456)
        << "Failed to open vector runtime for index " << index->name
        << " on table " << index->table->name << ": WITH(...) options do"
        << " not describe an HNSW index; vector search on it will not"
        << " work until the table is reopened.";
    return nullptr;
  }

  /* Dimension is a property of the column, not of WITH(...), so it has
  to come from the Field.

  key_part[0].field is not usable directly - for a vector key part
  get_index_prefix_len() reports 1, so the KEY_PART_INFO describes a
  1-byte prefix rather than the column. Its field_index() is still
  correct, though, and indexing form->field with it is exactly the dance
  create_index() does (ha_innodb.cc) to see past a forged prefix
  field. */
  ut_ad(key->user_defined_key_parts == 1);
  const Field *f = form->field[key->key_part[0].field->field_index()];
  if (f == nullptr || f->type() != MYSQL_TYPE_VECTOR) {
    ib::error(ER_IB_MSG_456)
        << "Failed to open vector runtime for index " << index->name
        << " on table " << index->table->name << ": the indexed column is"
        << " not a VECTOR column; vector search on it will not work"
        << " until the table is reopened.";
    return nullptr;
  }
  const Field_vector *field = down_cast<const Field_vector *>(f);

  const uint32_t dims = field->get_max_dimensions();
  if (dims == 0 || dims == UINT32_MAX) {
    ib::error(ER_IB_MSG_456)
        << "Failed to open vector runtime for index " << index->name
        << " on table " << index->table->name << ": invalid vector"
        << " dimension " << dims << "; vector search on it will not work"
        << " until the table is reopened.";
    return nullptr;
  }

  auto *vec = ut::new_withkey<vec_t>(
      UT_NEW_THIS_FILE_PSI_KEY, index->id, index->table, dims,
      static_cast<uint32_t>(hnsw_param->M),
      static_cast<uint32_t>(hnsw_param->ef_construction), hnsw_param->dist);
  if (vec == nullptr) {
    ib::error(ER_IB_MSG_456)
        << "Failed to open vector runtime for index " << index->name
        << " on table " << index->table->name << ": out of memory;"
        << " vector search on it will not work until the table is"
        << " reopened.";
    return nullptr;
  }

  /* Publish, or lose the race and use the winner. Two sessions opening
  the same table both find dict_index_t::vec null - ha_innobase::open
  takes no latch that would order them - so both build a runtime and one
  of them must give way. Before this was a compare-exchange the loser's
  object was simply overwritten and leaked, and had any caller used the
  returned pointer there would have been two graphs on one index: two
  arenas both charging innodb_hnsw_max_memory, inserts landing in one
  graph and searches reading the other.

  The loser's object is safe to destroy: nothing has been loaded into it
  yet, so ~vec_t deletes a null graph and no arena bytes are involved. */
  std::atomic_ref<Vec_runtime *> slot(index->vec);
  Vec_runtime *expected = nullptr;
  if (!slot.compare_exchange_strong(expected, vec, std::memory_order_release,
                                    std::memory_order_acquire)) {
    ut::delete_(vec);
    return static_cast<vec_t *>(expected);
  }
  return vec;
}

/** Open the aux table for one DML operation.

No MDL on the aux itself: the caller holds MDL on the BASE table, and every
DDL that can drop an aux takes exclusive base MDL first. Fast path is the
dict cache; fall back to the DD only when it has been evicted, and then take
MDL because the fallback can block. */
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
means - the alternative, reading every row at startup, is the "huge load
operation at the moment the index is first used after restart" this
design exists to avoid.

An aux with no record 0 is an EMPTY index, not a broken one: record 0 is
written when the first node is inserted, so its absence means no node
has ever been inserted. */
static dberr_t vec_runtime_load(vec_t *vec, dict_table_t *aux, THD *thd) {
  ut_ad(vec->hnsw == nullptr);

  /* innodb_hnsw_max_memory, at the entry to the load. Same charge check
  as vec_add_node: is the budget already spent, not would this fit. What
  this call allocates directly is the graph object and the entry-point
  node; the rest of the graph arrives node by node through
  Vec_persistor::load_node_cb, which checks again per node. Both are
  needed - this one so a cold index cannot start loading into a budget
  that is already gone, that one so the load cannot run past it. */
  if (srv_hnsw_max_memory != 0 &&
      Vec_arena::global_bytes() >= srv_hnsw_max_memory) {
    return DB_VEC_OUT_OF_MEMORY;
  }

  vec->hnsw =
      ut::new_withkey<Vec_hnsw>(UT_NEW_THIS_FILE_PSI_KEY, vec->dims, vec->dist,
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

  /* Graph node id 0 is the class's reserved empty-slot sentinel; no real
  node is ever assigned it (vec_persist_entry_point). A record 0 naming it
  as the entry point is therefore corrupt, not merely empty - and passing
  it on would hit ut_a(id != 0) in vec_persist_load_node instead of
  failing gracefully.

  DB_INDEX_CORRUPT, not DB_CORRUPTION: convert_error_code_to_mysql maps
  DB_CORRUPTION to HA_ERR_CRASHED, which reports the *base table* as
  crashed. Only this vector index's runtime is bad; the base table and
  its other indexes are fine, so this must stay index-scoped. */
  if (entry_point == 0) {
    ut::delete_(vec->hnsw);
    vec->hnsw = nullptr;
    return DB_INDEX_CORRUPT;
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
  into the cached index, zeroing only n_uniq - a vector index has no
  B-tree ordering, but it does have its field.

  Searching for the column instead cannot work: VECTOR, BLOB, TEXT and
  JSON all map to DATA_BLOB in the dictionary, so any blob ordered ahead
  of the vector column would win.

  Ignore the field's prefix_len - get_index_prefix_len() reports 1 for a
  vector key part, which describes nothing about the column. */
  ut_ad(index->n_fields == 1);
  const ulint col_no = dict_col_get_no(index->get_field(0)->col);
  ut_ad(col_no < dtuple_get_n_fields(row));

  const dfield_t *df = dtuple_get_nth_field(row, col_no);
  if (dfield_is_null(df)) return nullptr;
  *len = dfield_get_len(df);
  return static_cast<const char *>(dfield_get_data(df));
}

/** Load the graph once, under load_mutex.

A corrupt aux is sticky: it will not read correctly next time either, so the
index is marked corrupt and every statement on it fails with
ER_INDEX_CORRUPT until DROP and re-ADD rebuild it. Any other failure is
treated as transient - the runtime stays unloaded and the next statement
retries.

Also refuses a graph already known to be short of nodes.
@param[in,out]  vec    the runtime
@param[in]      index  the index owning it, for marking it corrupt
@param[in]      aux    the aux table, already open
@param[in]      thd    session
@return DB_SUCCESS, or the reason the graph is not usable */
static dberr_t vec_runtime_load_once(vec_t *vec, dict_index_t *index,
                                     dict_table_t *aux, THD *thd) {
  /* Index-scoped, not DB_CORRUPTION: only this index's graph is bad, and
  DB_CORRUPTION would report the base table as crashed. */
  if (vec->corrupted_hnsw.load(std::memory_order_acquire)) {
    return DB_INDEX_CORRUPT;
  }

  if (vec->loaded.load(std::memory_order_acquire)) return DB_SUCCESS;

  std::lock_guard<std::mutex> g(vec->load_mutex);
  if (vec->loaded.load(std::memory_order_relaxed)) return DB_SUCCESS;

  const dberr_t err = vec_runtime_load(vec, aux, thd);
  if (err == DB_INDEX_CORRUPT || err == DB_CORRUPTION) {
    dict_set_corrupted(index);
  }
  return err;
}

/** Record that a node failed to load mid-statement. HNSW has marked it lost
and will not retry, so the graph can no longer answer correctly. The graph
is not freed here - searches may be walking it - so a flag stands in until
the runtime is built again.
@param[in,out]  vec  the runtime */
static void vec_runtime_set_corrupted(vec_t *vec) {
  vec->corrupted_hnsw.store(true, std::memory_order_release);
}

/** Insert one node into the graph and, through the persistor, the aux.

Shared by INSERT and by a vector-column UPDATE, because to the graph
they are the same operation: a node is immutable, so a changed vector is
a new node rather than an edit of the old one. */
static dberr_t vec_add_node(vec_t *vec, dict_index_t *index,
                            dict_table_t *table, uint64_t label,
                            uint64_t base_pk, const char *q, THD *thd) {
  /* innodb_hnsw_max_memory, checked BEFORE insert() starts mutating.

  Refused here, at the entry to the operation, rather than in
  Vec_arena::allocate(): the arena has no per-block free, so a refusal
  partway through a rewire cannot be unwound.

  A charge check, not a prediction - it asks whether the budget is spent,
  not whether this insert fits. Taken outside the graph's lock, so the
  overshoot is one insert's allocation per thread already past it. */
  if (srv_hnsw_max_memory != 0 &&
      Vec_arena::global_bytes() >= srv_hnsw_max_memory) {
    return DB_VEC_OUT_OF_MEMORY;
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
  callback with this flag - the entire cost of the extra commits was the
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

  {
    const dberr_t lerr = vec_runtime_load_once(vec, index, aux, thd);
    if (lerr != DB_SUCCESS) {
      trx_rollback_to_savepoint(aux_trx, nullptr);
      trx_free_for_background(aux_trx);
      vec_aux_close_for_dml(aux, thd, &mdl);
      return lerr;
    }
  }

  /* Unlocked: the class is thread-safe for concurrent insert() and search,
  and the only operation it is not thread-safe for - init_from_entry_point -
  cannot be running, because reaching here means `loaded` is already true. */
  vec->hnsw->insert(label, base_pk, q, &ctx);

  /* Commits whatever the last callback left open - often nothing, since
  each callback commits its own work. */
  if (ctx.err == DB_SUCCESS) {
    trx_commit_for_mysql(aux_trx);
  } else {
    /* Whatever went wrong, a node may have failed to load on the way in
    and HNSW will not retry it. */
    vec_runtime_set_corrupted(vec);

    /* trx_rollback_to_savepoint, not trx_rollback_for_mysql: the aux
    transaction is a BACKGROUND trx, so it is not in the MySQL trx list that
    trx_rollback_for_mysql asserts membership of.

    This now rolls back only the callback that failed: everything before it
    was committed by vec_ctx_step_commit. The earlier rows stand, which is
    the orphan the design's rollback section accepts, and is the direction
    that keeps the aux tracking memory rather than diverging from it. */
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

struct Vec_build {
  Vec_build(uint32_t dims_, uint32_t m_, uint32_t ef_construction_,
            vec_dist_func_t *dist_, dict_index_t *index_)
      : dims(dims_), index(index_) {
    graph = ut::new_withkey<Vec_build_hnsw>(UT_NEW_THIS_FILE_PSI_KEY, dims_,
                                            dist_, m_, ef_construction_);
  }

  ~Vec_build() {
    if (graph != nullptr) ut::delete_(graph);
  }

  Vec_build(const Vec_build &) = delete;
  Vec_build &operator=(const Vec_build &) = delete;

  /* The graph is built with a persistor that writes nothing, and the aux
  is written afterwards from a walk of the finished graph. Persisting as
  we insert would rewrite a node's row every time a later insert rewires
  it; the walk writes each row once, with its final neighbour list. */
  Vec_build_hnsw *graph{nullptr};
  Vec_null_persistor::Context null_ctx{};
  uint32_t dims{};
  dict_index_t *index{nullptr};
};

dberr_t vec_update_row(trx_t *trx [[maybe_unused]], dict_table_t *table,
                       uint64_t label, const char *q, ulint q_len,
                       uint64_t base_pk, THD *thd) {
  ut_ad(label != 0);

  for (dict_index_t *index = table->first_index(); index != nullptr;
       index = index->next()) {
    if (!index->is_vector()) continue;
    vec_t *vec = vec_runtime_get(index);
    if (vec == nullptr) continue;
    if (q_len != vec->dims * sizeof(float)) return DB_CORRUPTION;
    const dberr_t err = vec_add_node(vec, index, table, label, base_pk, q, thd);
    if (err != DB_SUCCESS) return err;
  }
  return DB_SUCCESS;
}

dberr_t vec_insert_row(trx_t *trx [[maybe_unused]], dict_table_t *table,
                       const dtuple_t *row, THD *thd) {
  for (dict_index_t *index = table->first_index(); index != nullptr;
       index = index->next()) {
    if (!index->is_vector()) continue;

    vec_t *vec = vec_runtime_get(index);
    if (vec == nullptr) continue;

    ulint vec_len = 0;
    const char *q = vec_row_vector_bytes(index, row, &vec_len);
    if (q == nullptr) continue;
    if (vec_len != vec->dims * sizeof(float)) return DB_CORRUPTION;

    const uint64_t label = vec_get_aux_id_from_row(table, row);
    /* 0 is impossible per this column's contract - the aux reserves it
    for its metadata record. Skip the row rather than create a node under
    it. */
    if (label == 0) {
      continue;
    }

    /* base_pk is the base row's PRIMARY KEY, not the label. A search
    returns base_pk so the caller can fetch the row; the label
    identifies the node and is what the read path compares against the
    row's hidden column. The design allows a single-column BIGINT
    UNSIGNED primary key, so it is the first clustered field. */
    const dict_index_t *clust = table->first_index();
    ut_ad(dict_index_get_n_unique(clust) == 1);
    const ulint pk_col = clust->get_col_no(0);
    const dfield_t *pk_df = dtuple_get_nth_field(row, pk_col);
    ut_ad(!dfield_is_null(pk_df) && dfield_get_len(pk_df) == 8);
    const uint64_t base_pk =
        mach_read_from_8(static_cast<const byte *>(dfield_get_data(pk_df)));

    const dberr_t err = vec_add_node(vec, index, table, label, base_pk, q, thd);
    if (err != DB_SUCCESS) return err;
  }
  return DB_SUCCESS;
}
