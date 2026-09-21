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
#include "current_thd.h"
#include "dict0dd.h"
#include "dict0dict.h"
#include "ha_prototypes.h"
#include "lock0lock.h"
#include "mach0data.h"
#include "my_dbug.h"
#include "my_sys.h"
#include "mysqld_error.h"
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

/** The InnoDB error for a graph result that is not HNSW_SUCCESS.

One mapping in one place, rather than every call site deciding. Only the
two OOM results are the graph's own failure; the other two mean something
about the caller:

  HNSW_NOT_FOUND    the graph names a node the aux does not have. The one
                    permanent failure here: the two disagree, and no retry
                    changes that.
  HNSW_OOM_GRAPH    the arena or the node map could not grow.
  HNSW_OOM_CONTEXT  a per-operation scratch allocation failed.

                    Both OOM results are transient. The class does not
                    leave the graph inconsistent for either: the node
                    being inserted is marked NODE_FAILED and a stub that
                    could not be filled stays NODE_DUMMY, so both are
                    retried rather than skipped forever. Nothing durable
                    changed, and the next statement gets a clean attempt.
  HNSW_ERROR_CB     one of our own callbacks failed and put the reason in
                    ctx->err, so that reason is what the client should
                    see - this result carries no information we do not
                    already have.

nn_search_next is the one caller for which HNSW_NOT_FOUND is not a
failure at all - it is the end of the scan - and it says so before
asking here.

DB_VEC_OUT_OF_MEMORY rather than DB_OUT_OF_MEMORY for the OOM pair:
row_mysql_handle_errors does not list DB_OUT_OF_MEMORY and so reaches its
ib::fatal arm, which would turn a failed allocation on the INSERT path
into a dead server.
@param[in]  rc   the graph's result
@param[in]  ctx  the context the callbacks reported through, or nullptr
                 where there is none - an index build's persistor cannot
                 fail, so it has no error to carry
@return the error to fail the statement with */
dberr_t vec_hnsw_dberr(HnswResult rc, const Vec_ctx *ctx) {
  switch (rc) {
    case HNSW_SUCCESS:
      return DB_SUCCESS;
    case HNSW_NOT_FOUND:
      return DB_INDEX_CORRUPT;
    case HNSW_OOM_GRAPH:
    case HNSW_OOM_CONTEXT:
      return DB_VEC_OUT_OF_MEMORY;
    case HNSW_ERROR_CB:
      /* Every callback sets ctx->err before returning this, and the early
      exit each one takes is itself conditional on ctx->err already being
      set - so an unset error here means a callback grew a path that
      forgot to. */
      ut_ad(ctx != nullptr && ctx->err != DB_SUCCESS);
      return (ctx != nullptr && ctx->err != DB_SUCCESS) ? ctx->err : DB_ERROR;
  }
  ut_d(ut_error);
  ut_o(return DB_ERROR);
}

void vec_report_memory_ceiling(THD *thd) {
  if (thd == nullptr) return;
  my_error(ER_CAPACITY_EXCEEDED, MYF(0),
           static_cast<ulonglong>(srv_hnsw_max_memory),
           "innodb_hnsw_max_memory",
           "The vector index graph was left unchanged. Raise"
           " innodb_hnsw_max_memory and retry.");
}

void vec_report_missing_node(THD *thd, uint64_t id) {
  if (thd == nullptr) return;
  ib_errf(thd, IB_LOG_LEVEL_ERROR, ER_INNODB_INDEX_CORRUPT,
          "the vector index's graph names node " UINT64PF
          ", which its auxiliary table does not have. DROP and re-create"
          " the index to rebuild it from the base rows.",
          id);
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

/** Record why no runtime could be built, for the statements that will need
one. Every failure below also logs, for the operator; this is what the
client gets to see.
@param[in,out]  index  the vector index
@param[in]      err    the reason
@return nullptr, so a failing path can return this directly */
static vec_t *vec_runtime_open_failed(dict_index_t *index, dberr_t err) {
  ut_ad(err != DB_SUCCESS);
  std::atomic_ref<dberr_t> slot(index->vec_open_err);
  slot.store(err, std::memory_order_release);
  return nullptr;
}

dberr_t vec_runtime_unavailable(const dict_index_t *index) {
  std::atomic_ref<dberr_t> slot(
      const_cast<dict_index_t *>(index)->vec_open_err);
  const dberr_t err = slot.load(std::memory_order_acquire);
  /* Nothing but ha_innobase::open() builds a runtime, and it records why
  when it cannot - so an unset reason means no open has run for this
  index, which a statement that got this far must have done. */
  ut_ad(err != DB_ERROR_UNSET);
  return err == DB_ERROR_UNSET ? DB_INDEX_CORRUPT : err;
}

vec_t *vec_runtime_open(dict_index_t *index, const KEY *key, const TABLE *form,
                        THD *thd) {
  ut_ad(index != nullptr);
  ut_ad(index->is_vector());
  ut_ad(key != nullptr);

  if (vec_t *existing = vec_runtime_get(index); existing != nullptr) {
    return existing;
  }

  /* Stands in for the allocation failure below - the one reason here that
  a retry can get past. */
  DBUG_EXECUTE_IF("vec_runtime_open_fail",
                  return vec_runtime_open_failed(index, DB_VEC_OUT_OF_MEMORY););

  /* The values the user wrote in WITH(...), round-tripped through the
  DD and parsed by the open-time overload added for exactly this. */
  storage::innobase::vec::VectorIndexParam vip;
  if (storage::innobase::vec::parse_options(*key, vip)) {
    ib::error(ER_IB_MSG_456)
        << "Failed to open vector runtime for index " << index->name
        << " on table " << index->table->name << ": could not parse the"
        << " index's WITH(...) options; vector search on it will not"
        << " work until the table is reopened.";
    return vec_runtime_open_failed(index, DB_INDEX_CORRUPT);
  }
  const auto *hnsw_param = std::get_if<storage::innobase::vec::HnswParam>(&vip);
  if (hnsw_param == nullptr) {
    ib::error(ER_IB_MSG_456)
        << "Failed to open vector runtime for index " << index->name
        << " on table " << index->table->name << ": WITH(...) options do"
        << " not describe an HNSW index; vector search on it will not"
        << " work until the table is reopened.";
    return vec_runtime_open_failed(index, DB_INDEX_CORRUPT);
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
    return vec_runtime_open_failed(index, DB_INDEX_CORRUPT);
  }
  const Field_vector *field = down_cast<const Field_vector *>(f);

  const uint32_t dims = field->get_max_dimensions();
  if (dims == 0 || dims == UINT32_MAX) {
    ib::error(ER_IB_MSG_456)
        << "Failed to open vector runtime for index " << index->name
        << " on table " << index->table->name << ": invalid vector"
        << " dimension " << dims << "; vector search on it will not work"
        << " until the table is reopened.";
    return vec_runtime_open_failed(index, DB_INDEX_CORRUPT);
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
    return vec_runtime_open_failed(index, DB_VEC_OUT_OF_MEMORY);
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

  /* An earlier open may have failed and recorded why; this one did not. */
  std::atomic_ref<dberr_t> err_slot(index->vec_open_err);
  err_slot.store(DB_SUCCESS, std::memory_order_release);
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

  /* innodb_hnsw_max_memory, at the entry to the load: refuse to START
  building a graph on a budget that is already gone. How far this load
  then gets is bounded per faulted node in load_node_cb. Same charge check
  as vec_add_node: is the budget already spent, not would this fit. What
  this call allocates directly is the graph object and the entry-point
  node; the rest of the graph arrives node by node through
  Vec_persistor::load_node_cb, which checks again per node. Both are
  needed - this one so a cold index cannot start loading into a budget
  that is already gone, that one so the load cannot run past it. */
  if (srv_hnsw_max_memory != 0 &&
      Vec_arena::global_bytes() >= srv_hnsw_max_memory) {
    vec_report_memory_ceiling(thd);
    return DB_VEC_OUT_OF_MEMORY;
  }

  vec->hnsw =
      ut::new_withkey<Vec_hnsw>(UT_NEW_THIS_FILE_PSI_KEY, vec->dims, vec->dist,
                                vec->m, vec->ef_construction);
  /* An allocation failure is the one branch here that no test can reach
  by ordinary means, and it is also the one that used to be fatal, so it
  gets a hook. */
  DBUG_EXECUTE_IF("vec_graph_alloc_fail", {
    ut::delete_(vec->hnsw);
    vec->hnsw = nullptr;
  });

  /* Same reason as the mapping above: this runs on the INSERT path too,
  where DB_OUT_OF_MEMORY reaches row_mysql_handle_errors' ib::fatal arm. */
  if (vec->hnsw == nullptr) return DB_VEC_OUT_OF_MEMORY;

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

  /* Loads exactly the entry-point node. HNSW_NOT_FOUND means record 0
  names a row the aux does not have, which is the graph and its table
  disagreeing; the instance is unusable either way and must go. */
  const HnswResult irc = vec->hnsw->init_from_entry_point(entry_point, &ctx);
  if (irc != HNSW_SUCCESS) {
    ut::delete_(vec->hnsw);
    vec->hnsw = nullptr;
    /* A failure that reached our load_node_cb left its reason in ctx->err,
    and for a missing row the callback has already reported it - so that
    reason is returned rather than re-derived, and nothing is printed
    twice.

    ctx->err is untouched when the class failed before consulting us:
    init_from_entry_point allocates the entry-point node and inserts it
    into the node map first, and either can fail with HNSW_OOM_GRAPH. The
    mapping covers that, which is why this is not an assert. */
    return ctx.err != DB_SUCCESS ? ctx.err : vec_hnsw_dberr(irc, &ctx);
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
    vec_report_memory_ceiling(thd);
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
  const HnswResult irc = vec->hnsw->insert(label, base_pk, q, &ctx);
  /* Only a failure inside the class itself arrives without a reason: every
  one of our callbacks sets ctx->err before returning a result. */
  if (irc != HNSW_SUCCESS && ctx.err == DB_SUCCESS) {
    ctx.err = vec_hnsw_dberr(irc, &ctx);
  }

  /* Commits whatever the last callback left open - often nothing, since
  each callback commits its own work. */
  if (ctx.err == DB_SUCCESS) {
    trx_commit_for_mysql(aux_trx);
  } else {
    /* Only the graph and the aux disagreeing makes the runtime unusable:
    a node it named is gone, the class has marked that stub NODE_LOST, and
    nothing retries a lost node - so every later search would answer with
    fewer rows and no error at all. Everything else that reaches here is
    transient (a failed allocation, a lock wait, an I/O error) and leaves a
    stub NODE_DUMMY, which the next statement retries.

    This marked the runtime corrupt for any failure at all, which was the
    honest reading while a callback could only say "gone": one lock wait
    cost a rebuild. The result codes tell the two apart now. */
    if (ctx.err == DB_INDEX_CORRUPT) vec_runtime_set_corrupted(vec);

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

uint32_t vec_index_dims(const dict_index_t *index) {
  if (index == nullptr) return 0;
  const vec_t *vec = vec_runtime_get(index);
  return vec == nullptr ? 0 : vec->dims;
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

dberr_t vec_ann_open(dict_index_t *index, const float *q, size_t batch_size,
                     size_t ef_search, THD *thd, vec_search_t **out) {
  ut_ad(index != nullptr && index->is_vector());
  ut_ad(q != nullptr && out != nullptr);
  ut_ad(batch_size > 0);
  *out = nullptr;

  auto *vec = vec_runtime_get(index);
  if (vec == nullptr) return DB_TABLE_NOT_FOUND;

  MDL_ticket *mdl = nullptr;
  dict_table_t *aux =
      vec_aux_open_for_dml(vec->table, vec->index_id, thd, &mdl);
  if (aux == nullptr) return DB_TABLE_NOT_FOUND;

  {
    const dberr_t lerr = vec_runtime_load_once(vec, index, aux, thd);
    if (lerr != DB_SUCCESS) {
      vec_aux_close_for_dml(aux, thd, &mdl);
      return lerr;
    }
  }

  auto *s = ut::new_withkey<vec_search_t>(UT_NEW_THIS_FILE_PSI_KEY);
  if (s == nullptr) {
    vec_aux_close_for_dml(aux, thd, &mdl);
    return DB_VEC_OUT_OF_MEMORY;
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
  const HnswResult src = vec->hnsw->nn_search_start(
      &s->nn, reinterpret_cast<const char *>(q), batch_size,
      std::max(ef_search, batch_size), &s->ctx);
  if (src != HNSW_SUCCESS && s->ctx.err == DB_SUCCESS) {
    s->ctx.err = vec_hnsw_dberr(src, &s->ctx);
  }
  if (s->ctx.err != DB_SUCCESS) {
    const dberr_t err = s->ctx.err;
    vec_ann_close(s);
    return err;
  }

  *out = s;
  return DB_SUCCESS;
}

bool vec_ann_next(vec_search_t *s, vec_hit_t *hit) {
  ut_ad(s != nullptr && hit != nullptr);
  if (s->ctx.err != DB_SUCCESS) return false;

  const auto next = s->vec->hnsw->nn_search_next(&s->nn);
  if (s->ctx.err != DB_SUCCESS) return false;
  /* HNSW_NOT_FOUND is the ordinary end of the scan here, not a failure -
  the one caller for which that result is not an error at all. */
  if (next.first == HNSW_NOT_FOUND) return false;
  if (next.first != HNSW_SUCCESS) {
    s->ctx.err = vec_hnsw_dberr(next.first, &s->ctx);
    return false;
  }

  hit->id = next.second.id;
  hit->base_pk = next.second.base_pk;
  return true;
}

dberr_t vec_ann_error(const vec_search_t *s) {
  return s == nullptr ? DB_SUCCESS : s->ctx.err;
}

void vec_ann_close(vec_search_t *s) {
  if (s == nullptr) return;
  s->nn.reset();
  if (s->aux != nullptr) {
    vec_aux_close_for_dml(s->aux, s->thd, &s->mdl);
  }
  ut::delete_(s);
}

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

Vec_build *vec_build_start(dict_index_t *index, const TABLE *altered_table,
                           dberr_t *err) {
  ut_ad(index != nullptr && index->is_vector());

  /* Anything below this point that is not the memory check is this index's
  own KEY not being where it should be - a defect in the caller or in the
  DD round-trip, never a resource shortage. Reporting it as
  DB_OUT_OF_MEMORY would send whoever reads the error chasing free memory
  that was never the problem, so every such branch reports DB_ERROR
  instead and logs which check failed. */
  const auto fail_config = [&](const char *why) -> Vec_build * {
    *err = DB_ERROR;
    ib::error(ER_IB_MSG_456)
        << "Failed to start the vector index build for index " << index->name
        << " on table " << index->table->name << ": " << why
        << "; the build cannot proceed.";
    return nullptr;
  };

  if (altered_table == nullptr) return fail_config("no altered table");

  /* Same pre-flight as the DML path (design: "Memory limits"): refuse
  before building anything rather than throwing partway through. This is
  the one branch that is an actual resource shortage. */
  if (srv_hnsw_max_memory != 0 &&
      Vec_arena::global_bytes() >= srv_hnsw_max_memory) {
    vec_report_memory_ceiling(current_thd);
    *err = DB_VEC_OUT_OF_MEMORY;
    return nullptr;
  }

  /* M and ef_construction exist only in the index definition the ALTER is
  producing - the dictionary carries neither - so they are read from the
  KEY here rather than plumbed down from the handler. */
  const KEY *vkey = nullptr;
  for (uint k = 0; k < altered_table->s->keys; k++) {
    if ((altered_table->key_info[k].flags & HA_VECTOR) != 0 &&
        innobase_strcasecmp(altered_table->key_info[k].name, index->name) ==
            0) {
      vkey = &altered_table->key_info[k];
      break;
    }
  }

  /* Test-only: let an MTR test force the "KEY not found" branch below
  without needing a genuinely corrupt DD round-trip. */
  DBUG_EXECUTE_IF("vec_build_start_key_not_found", vkey = nullptr;);

  if (vkey == nullptr) {
    return fail_config("no matching vector KEY in the altered table");
  }

  storage::innobase::vec::VectorIndexParam vip;
  if (storage::innobase::vec::parse_options(*vkey, vip)) {
    return fail_config("could not parse the index's WITH(...) options");
  }

  const auto *hp = std::get_if<storage::innobase::vec::HnswParam>(&vip);
  if (hp == nullptr) {
    return fail_config("WITH(...) options do not describe an HNSW index");
  }

  /* Same resolution as vec_runtime_open: the key part describes a 1-byte
  prefix, but its field_index() is correct. */
  const Field *f = altered_table->field[vkey->key_part[0].field->field_index()];
  if (f == nullptr || f->type() != MYSQL_TYPE_VECTOR) {
    return fail_config("the indexed column is not a VECTOR column");
  }

  const uint32_t dims =
      down_cast<const Field_vector *>(f)->get_max_dimensions();
  if (dims == 0 || hp->M == 0) {
    return fail_config("invalid vector dimensions or M");
  }

  auto *b = ut::new_withkey<Vec_build>(
      UT_NEW_THIS_FILE_PSI_KEY, dims, static_cast<uint32_t>(hp->M),
      static_cast<uint32_t>(hp->ef_construction), hp->dist, index);

  if (b == nullptr) {
    *err = DB_VEC_OUT_OF_MEMORY;
    return nullptr;
  }
  if (b->graph == nullptr) {
    ut::delete_(b);
    *err = DB_VEC_OUT_OF_MEMORY;
    return nullptr;
  }
  *err = DB_SUCCESS;
  return b;
}

dberr_t vec_build_add_row(Vec_build *b, dict_table_t *table,
                          const dtuple_t *row) {
  ut_ad(b != nullptr && b->graph != nullptr);

  ulint vec_len = 0;
  const char *q = vec_row_vector_bytes(b->index, row, &vec_len);
  if (q == nullptr) return DB_SUCCESS;
  if (vec_len != b->dims * sizeof(float)) return DB_CORRUPTION;

  /* Label 0 is the empty-slot sentinel and can never be a node. A row
  carrying it means the writing path missed it. */
  const uint64_t id = vec_get_aux_id_from_row(table, row);
  ut_ad(id != 0);

  const dfield_t *pk_df = nullptr;
  const dict_index_t *clust = table->first_index();
  ut_ad(dict_index_get_n_unique(clust) == 1);
  pk_df = dtuple_get_nth_field(row, clust->get_col_no(0));
  ut_ad(!dfield_is_null(pk_df) && dfield_get_len(pk_df) == 8);
  const uint64_t base_pk =
      mach_read_from_8(static_cast<const byte *>(dfield_get_data(pk_df)));

  const HnswResult irc = b->graph->insert(id, base_pk, q, &b->null_ctx);
  if (irc != HNSW_SUCCESS) {
    /* Out of memory, and nothing else: Vec_null_persistor's write
    callbacks all return HNSW_SUCCESS, and its load_node_cb is ut_error
    because a build never faults a node in. So the ALTER fails on a graph
    that could not grow, and the statement rolls the aux back with it. */
    ut_ad(irc == HNSW_OOM_GRAPH || irc == HNSW_OOM_CONTEXT);
    return vec_hnsw_dberr(irc, nullptr);
  }

  /* innodb_hnsw_max_memory. The whole graph is in memory before any of it
  is durable, so this is the only thing bounding a build. Several scan
  threads can pass this together and overshoot by a node each, which is
  bounded by the thread count and cheaper than serialising them. */
  if (srv_hnsw_max_memory != 0 &&
      Vec_arena::global_bytes() >= srv_hnsw_max_memory) {
    vec_report_memory_ceiling(current_thd);
    return DB_VEC_OUT_OF_MEMORY;
  }
  return DB_SUCCESS;
}

dberr_t vec_build_write_aux(Vec_build *b, trx_t *trx, dict_table_t *table,
                            THD *thd, Flush_observer *observer) {
  ut_ad(b != nullptr && b->graph != nullptr);
  ut_ad(trx != nullptr);

  if (b->graph->size() == 0) return DB_SUCCESS;

  MDL_ticket *mdl = nullptr;
  dict_table_t *aux = vec_aux_open_for_dml(table, b->index->id, thd, &mdl);
  if (aux == nullptr) return DB_TABLE_NOT_FOUND;

  /* Record 0 first, then one row per node in ascending id order. The aux
  table is keyed by id, so the whole sequence is an append and the tree is
  built left to right instead of being inserted into at random. All on the
  ALTER's transaction, so a failure here rolls the aux back with the rest
  of the statement.

  Record 0 is not a node: id 0 is the empty-slot sentinel, so the row is
  free to hold the entry point in base_pk. */
  vec_aux_row_t meta;
  meta.id = 0;
  meta.vec = nullptr;
  meta.dims = 0;
  meta.base_pk = b->graph->entry_point_id();
  meta.level = 0;
  meta.neighbors = nullptr;
  meta.neighbors_len = 0;

  Vec_aux_bulk *bulk = vec_aux_bulk_start(trx, aux, observer);

  if (bulk == nullptr) {
    vec_aux_close_for_dml(aux, thd, &mdl);
    return DB_VEC_OUT_OF_MEMORY;
  }

  dberr_t err = vec_aux_bulk_insert(bulk, meta);
  std::vector<byte> neighbors;
  size_t written = 0;

  const HnswResult wrc = b->graph->for_each_node_sorted(
      [&](uint64_t id, uint64_t base_pk, const char *vec, uint8_t layer,
          Vec_build_hnsw::NeighborIdRange nbrs) -> HnswResult {
        vec_flatten_neighbors(nbrs, neighbors);

        vec_aux_row_t row;
        row.id = id;
        row.vec = reinterpret_cast<const float *>(vec);
        row.dims = b->dims;
        row.base_pk = base_pk;
        row.level = layer;
        row.neighbors = neighbors.data();
        row.neighbors_len = neighbors.size();

        err = vec_aux_bulk_insert(bulk, row);
        /* The walk ends here, and err is the reason - the class only needs
        to know that its visitor failed. vec_aux_bulk_finish below is what
        rolls the load back. */
        if (err != DB_SUCCESS) return HNSW_ERROR_CB;

        ++written;
        return HNSW_SUCCESS;
      });

  /* The walk's own failure, with no err to go with it: building the sorted
  id list is the only thing it does that can fail on its own account. */
  if (err == DB_SUCCESS && wrc != HNSW_SUCCESS) {
    ut_ad(wrc == HNSW_OOM_CONTEXT);
    err = DB_VEC_OUT_OF_MEMORY;
  }

  /* Every node the graph holds has to be written, and the walk hands over
  only the complete ones. A finished build should have nothing else: it
  inserts every node itself, so it holds no lazily loaded stubs, and an
  insert that failed left a node behind only by failing - which took the
  ALTER down before reaching here. So a count that does not match means
  that abort was missed.

  Writing the rest anyway is the one outcome to avoid: the skipped node is
  still named by the neighbour lists that were written, which is the
  graph-and-aux disagreement the load path refuses. Failing the ALTER
  leaves the table as it was. */
  if (err == DB_SUCCESS && written != b->graph->size()) {
    ut_ad(written == b->graph->size());
    ib::error(ER_IB_MSG_456)
        << "Vector index " << b->index->name << " on table "
        << b->index->table->name << " built " << b->graph->size()
        << " graph nodes but only " << written
        << " are complete; the index cannot be written and the statement"
        << " will fail.";
    err = DB_ERROR;
  }

  err = vec_aux_bulk_finish(bulk, err);

  vec_aux_close_for_dml(aux, thd, &mdl);
  return err;
}

void vec_build_free(Vec_build *b) {
  if (b != nullptr) ut::delete_(b);
}

dberr_t vec_update_row(trx_t *trx [[maybe_unused]], dict_table_t *table,
                       uint64_t label, const char *q, ulint q_len,
                       uint64_t base_pk, THD *thd) {
  ut_ad(label != 0);

  for (dict_index_t *index = table->first_index(); index != nullptr;
       index = index->next()) {
    if (!index->is_vector()) continue;
    vec_t *vec = vec_runtime_get(index);
    if (vec == nullptr) return vec_runtime_unavailable(index);
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

    /* No runtime means the open that should have built one failed, and
    ha_innobase::open() carried on so the table stays readable and
    droppable. This statement cannot carry on: the row would be written
    with a hidden label that no node is ever created under, the index
    would answer without it for good, and nothing reconciles the two
    afterwards. */
    vec_t *vec = vec_runtime_get(index);
    if (vec == nullptr) return vec_runtime_unavailable(index);

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
