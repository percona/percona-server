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
@file include/vec0hnsw.h
The HNSW runtime: the graph, its persistor, and the state one open
vector index keeps in memory.
*/

#ifndef vec0hnsw_h
#define vec0hnsw_h

#include <atomic>
#include <cstdint>
#include <limits>
#include <mutex>
#include <unordered_set>
#include <vector>

#include "db0err.h"
#include "dict0mem.h"
#include "srv0srv.h"
#include "trx0trx.h"
#include "univ.i"
#include "ut0rnd.h"
#include "vec0arena.h"
#include "vec0aux.h"
#include "vec0dml.h"
#include "vec0index.h"
#include "vec0vec.h"

#include "vector-common/hnsw.h"

class THD;
class Flush_observer;

/** Everything a callback needs that is NOT a property of the index.

The class requires the persistor itself to be stateless - "no mutable
per-call, per-transaction, or per-thread fields... callbacks must not
rely on data written to Persistor members during a prior call" - because
one persistor instance is stored by value for the index's lifetime and
serves every caller. So all of it lives here and is passed in. */
struct Vec_ctx {
  /** Sub-transaction the aux writes ride. Not the user's transaction:
  the aux must survive a rollback of the statement that caused it, or
  the graph would keep nodes the aux no longer describes. */
  trx_t *trx{nullptr};
  /** The aux table, opened for this operation with MDL held. */
  dict_table_t *aux{nullptr};
  THD *thd{nullptr};
  /** M of the owning index; needed to size a neighbour blob. */
  uint32_t m{0};
  /** Bytes per vector; needed to write and read the vec column. */
  uint32_t vec_bytes{0};
  /** First failure. The callbacks return void, so this is how they
  report: each one short-circuits when it is already set, and the caller
  inspects it once insert() returns. */
  dberr_t err{DB_SUCCESS};
  /** Whether a callback may commit trx and start it again.

  True for DML, where trx is a background sub-transaction and committing
  per callback is what stops concurrent inserts deadlocking on each
  other's neighbour rows.

  False for an index build, where trx is the ALTER's own transaction.
  Committing there would commit the DDL itself a node at a time: it ends
  the transaction the dictionary changes are being made in, clears
  trx->dict_operation, drops the locks the ALTER holds, and marks a user
  transaction internal so its GTID is no longer persisted. Nothing is
  gained either, because an index under construction is invisible, so
  there is no second writer to deadlock against. */
  bool commit_aux_trx{true};
};

/* The persistor's shims forward here. Ordinary functions, so their
bodies live in the .cc rather than in every translation unit that
instantiates the graph. */

dberr_t vec_persist_insert(Vec_ctx *ctx, uint64_t id, uint64_t base_pk,
                           const char *q, uint8_t layer,
                           const std::vector<byte> &neighbors);

dberr_t vec_persist_update_neighbors(Vec_ctx *ctx, uint64_t id,
                                     const std::vector<byte> &neighbors);

dberr_t vec_persist_entry_point(Vec_ctx *ctx, uint64_t id);

/** Fill an unloaded node from its aux row.

A template only because LoadNodeHandle is nested in the instantiation,
which also means it must be defined wherever it is instantiated - the
graph's search path can call it, so the definition cannot live in a .cc.

A node is faulted in when traversal reaches a stub - one created by a
neighbour list naming an id whose node has not been read yet. */
template <typename Hnsw>
dberr_t vec_persist_load_node(Vec_ctx *ctx, Hnsw &hnsw,
                              typename Hnsw::LoadNodeHandle handle) {
  const uint64_t id = hnsw.load_node_id(handle);

  /* Record 0 is the metadata record, never a node. The id comes off a
  neighbour list read from the aux table, so a 0 here means the aux is
  corrupt - report it rather than fault in the metadata as a node. */
  ut_ad(id != 0);
  if (id == 0) return DB_CORRUPTION;

  mem_heap_t *heap = mem_heap_create(1024, UT_LOCATION_HERE);
  vec_aux_read_t node;
  dberr_t err = vec_aux_read_node(ctx->aux, id, heap, &node);
  if (err != DB_SUCCESS) {
    mem_heap_free(heap);
    /* A miss here is never benign: this id came off a neighbour list, so
    the graph says the node must exist. DB_RECORD_NOT_FOUND would reach
    the client as HA_ERR_NO_ACTIVE_RECORD, indistinguishable from an
    ordinary missing row. Report it as what it is. */
    return err == DB_RECORD_NOT_FOUND ? DB_ANN_NODE_NOT_FOUND : err;
  }

  if (node.vec_len != ctx->vec_bytes) {
    mem_heap_free(heap);
    return DB_CORRUPTION;
  }

  /* The neighbour blob must cover exactly the node's slots. Checking it
  is not paranoia: the count is derived from level and M rather than
  stored, so a mismatch means the row and the index disagree about the
  shape of the graph, and load_node_neighbors would read past the blob. */
  if (node.neighbors_len != vec_aux_neighbors_blob_len(node.level, ctx->m)) {
    mem_heap_free(heap);
    return DB_CORRUPTION;
  }

  /* Order matters: load_node_neighbors sizes its allocation from the
  layer, so the layer has to be set first. */
  hnsw.load_set_layer(handle, node.level);
  hnsw.load_set_vec(handle, reinterpret_cast<const char *>(node.vec));
  hnsw.load_set_base_pk(handle, node.base_pk);

  std::vector<uint64_t> ids;
  ids.reserve(node.neighbors_len / 8);
  for (ulint off = 0; off + 8 <= node.neighbors_len; off += 8) {
    ids.push_back(mach_read_from_8(node.neighbors + off));
  }
  hnsw.load_node_neighbors(handle, ids);

  mem_heap_free(heap);
  return DB_SUCCESS;
}

/** Flatten a neighbour range into the on-disk blob: one big-endian id per
slot, 0 for an empty slot, no header. Shared by the two writers - the
persistor, which writes a node as it is inserted, and the build, which
writes it once at the end from a walk - so the layout has one definition.
@param[in]   nbrs  neighbour ids, in slot order
@param[out]  out   the blob, cleared first */
template <typename NeighborIds>
inline void vec_flatten_neighbors(NeighborIds nbrs, std::vector<byte> &out) {
  out.clear();
  for (uint64_t id : nbrs) {
    byte buf[8];
    mach_write_to_8(buf, id);
    out.insert(out.end(), buf, buf + 8);
  }
}

/** Sink for the graph's persistence callbacks.

Stateless by contract - every member of the "state" a callback needs is
in Vec_ctx. The callbacks are member templates because the neighbour
range type is nested inside the instantiation that needs this class, so
naming it here would be circular; each one is a thin shim that converts
the range and forwards to an ordinary function in vec0hnsw.cc. */
struct Vec_persistor {
  using Context = Vec_ctx;

  template <typename NeighborIds>
  void insert_cb(Context *ctx, uint64_t id, uint64_t base_pk, const char *q,
                 uint8_t layer, NeighborIds nbrs) {
    if (ctx->err != DB_SUCCESS) return;
    std::vector<byte> blob;
    vec_flatten_neighbors(nbrs, blob);
    ctx->err = vec_persist_insert(ctx, id, base_pk, q, layer, blob);
  }

  template <typename NeighborIds>
  void update_neighbors_cb(Context *ctx, uint64_t id, NeighborIds nbrs) {
    if (ctx->err != DB_SUCCESS) return;
    std::vector<byte> blob;
    vec_flatten_neighbors(nbrs, blob);
    ctx->err = vec_persist_update_neighbors(ctx, id, blob);
  }

  void update_entry_point_cb(Context *ctx, uint64_t id) {
    if (ctx->err != DB_SUCCESS) return;
    ctx->err = vec_persist_entry_point(ctx, id);
  }

  /** Returns false on failure, which marks the node NODE_LOST rather than
  leaving a half-filled COMPLETE one. The first error is kept in ctx->err
  so the statement fails rather than answering from a partial graph. */
  template <typename Hnsw>
  bool load_node_cb(Context *ctx, Hnsw &hnsw,
                    typename Hnsw::LoadNodeHandle handle) {
    if (ctx->err != DB_SUCCESS) return false;

    /* NOT the place for the innodb_hnsw_max_memory check, however much
    it looks like it: this is where a cold graph grows, but a false
    return here is indistinguishable from "this node is gone". load_node
    (hnsw.h) calls set_lost() on it, NODE_LOST is never retried, and
    every later search skips that node - so refusing one fault for a
    transient reason leaves the graph permanently short of nodes and
    answering queries with fewer rows and no error at all. Measured:
    with the check here, a search after a refused load returned one row
    where three were correct.

    So the budget is checked at the entry to a load or an insert instead
    (vec_runtime_load, vec_add_node), which bounds when a graph may
    start growing but lets one statement overshoot by whatever it
    faults. Metering each fault needs a load_node_cb that can say
    "failed, try again later" as opposed to "lost" - the same API gap
    raised in review of this header, where the callback is to be reworked
    to report errors properly. This check belongs there. */

    const dberr_t err = vec_persist_load_node(ctx, hnsw, handle);
    if (err != DB_SUCCESS) {
      ctx->err = err;
      return false;
    }
    return true;
  }
};

/** A Persistor that writes nothing.

An index build inserts every row into a graph that no reader can see yet,
and persisting during that build is wasted work: each insert rewires its
neighbours, so a node's row would be rewritten every time a later insert
touches it - O(N x M x log N) row updates to arrive at a state that is
only correct once the last row is in. Building against this persistor and
then walking the finished graph (HNSW::for_each_node) writes each node
once, with its final neighbour list.

Context is an empty tag: there is no error to carry, because none of
these can fail. load_node_cb asserts because a build never faults a node
in - every node it has, it inserted. */
struct Vec_null_persistor {
  struct Context {};

  template <typename NeighborIds>
  void insert_cb(Context *, uint64_t, uint64_t, const char *, uint8_t,
                 NeighborIds) {}
  template <typename NeighborIds>
  void update_neighbors_cb(Context *, uint64_t, NeighborIds) {}
  void update_entry_point_cb(Context *, uint64_t) {}
  template <typename Hnsw>
  bool load_node_cb(Context *, Hnsw &, typename Hnsw::LoadNodeHandle) {
    ut_error;
    return false;
  }
};

/** UniformRandomBitGenerator for the graph's layer draw, over InnoDB's RNG.

HNSW does not synchronise RandomEngine access, and its contract requires a
thread-safe one once inserts may run concurrently (hnsw.h) - which
they now may. Its default, std::default_random_engine, is not, so the graph
would race on the RNG state inside random_layer().

ut::random_64() is thread-safe by construction rather than by locking: its
state is `extern thread_local uint64_t random_seed` (ut0rnd.h), seeded
per thread from this_thread_hash + my_timer_cycles(). Each thread draws
from its own stream, so there is no shared state and no mutex - which is
why this is preferable to wrapping a std:: engine, and why it is what the
rest of InnoDB uses.

The seed argument is accepted and ignored. HNSW passes one (default 42)
because a std:: engine needs it; ut0rnd seeds itself per thread and has no
per-object state to seed. One consequence, recorded in section 40: layer
assignment is no longer reproducible across runs, so no test may record
per-node `level` or `nb`. */
class Vec_random_engine {
 public:
  using result_type = uint64_t;

  explicit Vec_random_engine(uint32_t /* seed */ = 42) {}

  static constexpr result_type min() { return 0; }
  static constexpr result_type max() {
    return std::numeric_limits<result_type>::max();
  }

  result_type operator()() { return ut::random_64(); }
};

/** The instantiation. This line is the whole "registration": the
compiler substitutes our types, m_persistor becomes a real Vec_persistor,
and every callback call inside the class is ordinary name resolution. A
signature that does not match is a compile error, which is the only
registration check there is. */
using Vec_hnsw = HNSW<Vec_arena, Vec_persistor, Vec_random_engine>;

/** The same graph, built without persisting anything: what an index build
uses before writing the aux in one pass. Same arena and same RNG, so it
behaves identically - only the persistor differs. */
using Vec_build_hnsw = HNSW<Vec_arena, Vec_null_persistor, Vec_random_engine>;

/** In-memory state of one open vector index. */
struct vec_t : public Vec_runtime {
  /** Everything the runtime knows about its index is settled here, before
  the object is published into dict_index_t::vec, and const afterwards.
  That is not tidiness: publication is the only synchronisation these
  fields get, so a later assignment to any of them would be a data race
  against every reader. Making them const means such a patch does not
  compile. */
  vec_t(space_index_t index_id_, dict_table_t *table_, uint32_t dims_,
        uint32_t m_, uint32_t ef_construction_, vec_dist_func_t *dist_)
      : index_id(index_id_),
        table(table_),
        dims(dims_),
        m(m_),
        ef_construction(ef_construction_),
        dist(dist_) {}

  ~vec_t() override;

  /** The graph. Owns its arena and its persistor by value. */
  Vec_hnsw *hnsw{nullptr};
  /** Serialises the one-time build of the graph. Cold path only - nothing
  on this object is locked once `loaded` is true.

  The HNSW class is thread-safe for everything we do afterwards: concurrent
  insert() and search on one instance, and concurrent loads of the same
  lazily faulted node (load_node() takes a striped lock and re-checks the
  node state under it). None of that needs a latch from us.

  The one thing the class declines is init_from_entry_point() running
  alongside insert or search (hnsw.h) - it mutates m_nodes and the arena
  without taking m_global_lock, which insert() does take. This mutex makes
  that unreachable rather than merely unlikely: `loaded` goes false to true
  exactly once, and only after the graph is fully built, so no thread can
  reach insert() or k_nn_search() until init has finished. There is nobody
  to exclude, so the hot paths take nothing at all.

  Not std::call_once: vec_runtime_load reports failure by *returning*
  DB_OUT_OF_MEMORY rather than throwing, and call_once would consume the
  flag on a normal return - leaving the index permanently unloaded after a
  transient failure. Here a failure simply leaves `loaded` false and the
  next statement retries. */
  std::mutex load_mutex;
  /** The index this runtime belongs to. */
  const space_index_t index_id;
  /** Base table, for opening the aux and reading the label counter. */
  dict_table_t *const table;
  const uint32_t dims;
  const uint32_t m;
  const uint32_t ef_construction;
  /** The distance kernel this index's metric selects, resolved once by
  parse_options. The graph is built with it rather than with a kernel
  chosen here, so the index's metric option is what decides. */
  vec_dist_func_t *const dist;
  /** True once the graph has been built from the aux table. Atomic, with
  release/acquire ordering: it publishes the `hnsw` pointer to every thread
  that sees it true, which is what lets the hot paths run unlocked. */
  std::atomic<bool> loaded{false};

  /** Set when a node failed to load during a search or an insert. HNSW has
  marked that node lost and never retries it, so this graph would answer
  later queries with fewer rows and no error. Once set, every statement on
  this index fails instead. Cleared only by building the runtime again -
  a reopen after eviction, DROP and re-ADD, or a restart.

  A flag rather than freeing and reloading the graph: readers do not take
  load_mutex once `loaded` is true, so freeing `hnsw` here would run
  concurrently with searches already walking it. */
  std::atomic<bool> corrupted_hnsw{false};
};

/** The runtime attached to `index`, or nullptr if it has none yet.

dict_index_t::vec is written by whichever session opens the table first
and read by every session after it, with no latch between them, so the
access is atomic: a release store publishes the object and an acquire
load here guarantees that a reader seeing the pointer also sees the
fields written before it. std::atomic_ref rather than making the member
std::atomic because dict_index_t is never constructed - it is zeroed and
dict_mem_fill_index_struct() stands in for a constructor - so a member
with a real constructor would not have one called.
@param[in]  index  vector index
@return the runtime, or nullptr */
[[nodiscard]] inline vec_t *vec_runtime_get(const dict_index_t *index) {
  /* const_cast: atomic_ref needs a non-const lvalue, and the read itself
  does not modify the index. */
  std::atomic_ref<Vec_runtime *> slot(const_cast<dict_index_t *>(index)->vec);
  return static_cast<vec_t *>(slot.load(std::memory_order_acquire));
}

/** Open (lazily create) the runtime for a vector index.

Takes the KEY because that is where the parameters are: M, metric and
ef_construction come back from the DD on the KEY the SQL layer builds,
and the row-level code that needs the graph has only dict objects.
@param[in,out]  index  the vector index
@param[in]      key    the KEY describing it
@param[in]      form   the open TABLE, for the vector column's dimension
@param[in]      thd    session, for error reporting
@return the runtime, or nullptr if the parameters could not be read */
vec_t *vec_runtime_open(dict_index_t *index, const KEY *key, const TABLE *form,
                        THD *thd);

/** Add one row's vector to every vector index on the table.

Called after the base row is inserted, with the row that carries the
label already written into its hidden column.

The aux writes ride a SUB-TRANSACTION, not the caller's. That is the
whole point of the design: the graph is an in-memory cache whose only
durable form is the aux table, and a node that survives in memory while
its aux rows roll back would leave the two permanently disagreeing. So
the aux commits independently, and the invariant is one-directional -
the aux is a superset of the committed base rows. Orphans are filtered
at read time by looking base_pk up under the reader's view.
@param[in,out]  trx    the user's transaction (for the base row, not the aux)
@param[in,out]  table  the base table
@param[in]      row    the inserted row, label already written
@param[in]      thd    session
@return DB_SUCCESS, or an error */
dberr_t vec_insert_row(trx_t *trx, dict_table_t *table, const dtuple_t *row,
                       THD *thd);

/** One open streaming kNN scan.

Opaque by design: it owns the class's `NNSearchContext`, which is neither
copyable nor movable, plus the aux table and the MDL ticket that have to
stay alive for the whole scan - `nn_search_next` faults nodes in through
`load_node_cb`, which reads the aux. Allocated by vec_knn_open and
released by vec_knn_close; the handler holds only the pointer. */
struct vec_search_t;

/** The vector index on @p table, or nullptr. At most one exists. */
dict_index_t *vec_index_of(dict_table_t *table);

/** State of one vector index build, owned by the ddl::Builder that is
building that index. Opaque so the DDL layer needs none of the graph's
headers. */
struct Vec_build;

/** Add the new node for a vector-column UPDATE.

A node is immutable, so a changed vector is an INSERT of a new node
under the label calc_row_difference already put into the update vector.
The superseded node is left exactly as it is - it is still the right
answer for read views that predate this statement, and removing it would
break their isolation rather than tidy up.
@param[in,out]  trx    the user's transaction
@param[in,out]  table  the base table
@param[in]      label  the fresh label, from trx->vec_next_label
@param[in]      q      the new vector, dims * sizeof(float) bytes
@param[in]      base_pk  the row's primary key, unchanged by this update
@param[in]      thd    session
@return DB_SUCCESS, or an error */
dberr_t vec_update_row(trx_t *trx, dict_table_t *table, uint64_t label,
                       const char *q, ulint q_len, uint64_t base_pk, THD *thd);

#endif /* vec0hnsw_h */
