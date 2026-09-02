/* Copyright (c) 2026 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#ifndef HNSW_H_INCLUDED
#define HNSW_H_INCLUDED

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <limits>
#include <mutex>
#include <new>
#include <queue>
#include <random>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "my_pointer_arithmetic.h"

typedef double vec_dist_func_t(const char *a, const char *b, uint32_t dims);

/**
  Basic class for in-memory Hierarchical Navigable Small World (HNSW) vector
  index.

  Approximate nearest-neighbor index over float vectors, following the
  algorithms from: https://arxiv.org/abs/1603.09320

  Supports insert, k-NN search, and streaming NN search.

  Thread safety: concurrent insert() and search (k_nn_search / streaming) on
  the same instance are supported (assuming that insert ids are new and unique),
  provided RandomEngine is thread-safe if inserts may run concurrently.
  init_from_entry_point() and validate() must not run concurrently with insert
  or search. Persistor callbacks must not re-enter the same HNSW instance.

  @tparam ArenaAllocator
    Arena allocator for graph nodes (and trailing vector / neighbor storage).
    Must provide void *allocate(size_t size); there is no per-block free.
    Returned memory must stay valid for this object's lifetime; destroying
    the allocator frees the arena. HNSW does not destroy Nodes and must not
    outlive the allocator. allocate() may return nullptr (asserted).
    Not assumed thread-safe; HNSW serializes allocate() calls.

  @tparam Persistor
    Persistence / SE callback sink. Default-constructed once and stored by
    value for this index's lifetime (same ownership pattern as ArenaAllocator).
    Must be stateless: no mutable per-call, per-transaction, or per-thread
    fields. All such state belongs in Context, passed as the first argument
    to every callback (and via insert() / search APIs). Callbacks must not
    rely on data written to Persistor members during a prior call.
    Must provide:
      - nested type Context (call-scoped state, e.g. transaction / THD);
      - insert_cb(Context *, id, base_pk, q, layer, NeighborIdRange);
      - update_neighbors_cb(Context *, id, NeighborIdRange);
      - update_entry_point_cb(Context *, id);
      - load_node_cb(Context *, HNSW &, LoadNodeHandle) -> bool: fill a
        NODE_DUMMY node from storage. @p handle is opaque; use only the
        HNSW load_* helpers on it. On success must call, in order:
        load_set_layer(), load_set_vec(), load_set_base_pk(), then
        load_node_neighbors() (allocates neighbor storage and wires slots),
        then return true. HNSW then marks the node NODE_COMPLETE.
        On failure return false without relying on a partial fill; HNSW
        marks the node NODE_LOST. Neighbor ids use NeighborIdRange layout
        (0 = empty slot; size (layer + 2) * M). Stubs created for
        referenced neighbors stay unloaded until touched.
        Returning true without completing the load_* sequence leaves a
        corrupt COMPLETE node — do not do that.
        A callback that must never run is fine when every node is always
        resident (unit tests with NullPersistor).

    insert(), k_nn_search(), and streaming search may call load_node_cb when
    they touch unloaded stubs; pass PersistorContext * through those calls.

    One insert() may invoke update_neighbors_cb once per neighbor whose
    reverse edges changed; call order among those updates is unspecified.

    NeighborIdRange yields uint64_t ids over all neighbor slots (0 for empty).
    Range size is available via NeighborIdRange::size().
    Graph node id 0 is reserved for that empty-slot sentinel and must not be
    used as a real node id.

    Lifetime: both vector pointer on insert_cb and NeighborIdRange (and
    iterators / references obtained from it) are valid only for the
    duration of that callback invocation. Persistor must copy any data it
    needs to retain before returning.

    Transactions: current assumption that each callback is executed using
    its own transaction, which commits at callback return, independent of
    user's transaction.

    Cold start / recovery uses init_from_entry_point(), which loads the
    entry-point node via load_node_cb and requires success (asserted); the
    entry point is then NODE_COMPLETE.

    Index metadata is not persisted by HNSW itself. The class users must
    store it alongside the graph (at minimum: vector dimensions, M, distance
    kind / dist_func) and construct a reloaded HNSW with the same dimensions,
    M, and dist_func as the index that produced the persisted data.
    Neighbor slot layout and vector size depend on M and dimensions;
    mismatches corrupt the graph or yield wrong search results.

    A no-op Persistor is sufficient for many tests.

  @tparam RandomEngine
    Uniform random bit generator used for random layer assignment on insert
    (constructible from the constructor @p seed). HNSW does not synchronize
    access to the engine: if concurrent insert() calls are expected, the
    provided engine must be thread-safe. The default
    std::default_random_engine is not thread-safe.

  @todo (not in order of priority)
        1) Better error handling (e.g. errors from persistor callbacks, OOMs).
        2) Support for deletes (might be unnecessary).
        3) Support for transactions (might be unnecessary).
        4) Support for arbitrary PKs.
        5) Memory limits/expulsion strategy.
        6) Performance optimizations (visited set?).
*/
template <typename ArenaAllocator, typename Persistor,
          typename RandomEngine = std::default_random_engine>
class HNSW {
 public:
  using PersistorContext = typename Persistor::Context;

  /**
    One hit from k_nn_search() / nn_search_next().

    @c id is the unique graph node id; @c base_pk is the base-table row pk
    (duplicates allowed across nodes).
  */
  struct SearchHit {
    uint64_t id;
    uint64_t base_pk;

    bool operator==(const SearchHit &other) const {
      return id == other.id && base_pk == other.base_pk;
    }
    bool operator!=(const SearchHit &other) const { return !(*this == other); }
  };

  /**
    Lifecycle of an in-memory graph node.

    Transitions (only via Node setters / load_node()):
      Newly inserted node case:
        NODE_NEW  --set_linking()--> NODE_LINKING --set_complete()-->
          NODE_COMPLETE
      Loaded node case:
        NODE_DUMMY  --set_complete()--> NODE_COMPLETE   (successful lazy load)
        NODE_DUMMY  --set_lost()-----> NODE_LOST        (failed lazy load)

    Meaning:
      NODE_NEW      Fresh insert allocation (Node::create(..., NODE_NEW)).
                    Id is known; layer, vector, base_pk, and neighbor storage
                    are filled while still NEW, then set_linking() publishes
                    the node as linkable. Must not appear in any neighbor
                    list or be reachable from search; not a loadable stub.
      NODE_LINKING  Insert in progress: layer and vector are set, and neighbor
                    storage is allocated, but the neighbor list is not fully
                    constructed and must not be relied on (slots may still be
                    empty or only partially wired). The node may already appear
                    in other nodes' neighbor lists; search skips LINKING until
                    NODE_COMPLETE.
      NODE_COMPLETE Fully published: neighbor lists are complete and safe for
                    search and for Persistor neighbor snapshots. Entry point is
                    always COMPLETE.
      NODE_DUMMY    Lazy-load stub: id known, layer/vec/neighbors not valid yet.
                    Created via Node::create(..., NODE_DUMMY) for neighbor ids
                    seen before the node is loaded (see load_node_neighbors()).
      NODE_LOST     Lazy load failed; accessors must not be used. May remain
                    as a neighbor-slot pointer; search skips it.
                    The main scenario where NODE_LOST nodes can occur is when
                    insertion crash before calling insert_cb, but after some
                    concurrent insertion already persisted neighbor lists of
                    a node to which the lost node was added as a neighbor.

    Search expands only NODE_COMPLETE neighbors (loads DUMMY, skips LINKING
    and LOST). NODE_NEW must not be observed on search/insert graph edges
    (debug builds assert). Insert reverse-edge selection may also use
    NODE_LINKING so a concurrent in-progress insert can be linked before it
    is published; that path uses the LINKING node's vector, not its neighbor
    list.
  */
  enum NodeState : uint8_t {
    NODE_NEW,
    NODE_LINKING,
    NODE_COMPLETE,
    NODE_DUMMY,
    NODE_LOST
  };

  class NeighborIdIterator;
  struct NeighborIdRange;

  /**
    Opaque handle to an in-memory graph node passed to Persistor::load_node_cb.
    Do not dereference; use the load_* helpers on HNSW instead.
  */
  using LoadNodeHandle = void *;

  /**
    Construct an empty in-memory HNSW index.

    @param dimensions      Number of float elements per vector; must be > 0.
    @param dist_func       Distance function over two vectors of @p dimensions.
    @param M               Max out-degree on layers > 0 (layer 0 uses 2*M);
                           must be >= 2.
    @param ef_construction Size of the dynamic candidate list during INSERT;
                           clamped to at least M.
    @param seed            Seed passed to RandomEngine for random layer
                           assignment (default 42).
  */
  HNSW(size_t dimensions, vec_dist_func_t *dist_func, size_t M,
       size_t ef_construction, uint32_t seed = 42)
      : m_dimensions(dimensions),
        m_dist_func(dist_func),
        m_M(M),
        m_ef_construction(std::max(ef_construction, M)),
        m_layer_factor(1 / std::log(M)),
        m_rng(seed) {
    assert(dimensions > 0);
    assert(M >= 2);
  }
  HNSW(const HNSW &) = delete;
  HNSW &operator=(const HNSW &) = delete;
  HNSW(HNSW &&) = delete;
  HNSW &operator=(HNSW &&) = delete;

  /**
    Insert a vector into the HNSW graph.

    Corresponds to INSERT (algorithm 1) in the HNSW paper.

    @param id       Unique graph node id (must be non-zero: 0 is reserved as
                    the empty-neighbor sentinel for Persistor; must not already
                    exist in the index).
    @param base_pk  Primary key of the corresponding base-table row. Duplicates
                    are allowed (e.g. after updating an indexed vector, a new
                    graph node may share the same base_pk).
    @param q        Vector to insert.
    @param persistor_ctx  Persistor call context (e.g. transaction); passed
                          through to Persistor callbacks. Defaults to nullptr.
  */
  void insert(uint64_t id, uint64_t base_pk, const char *q,
              PersistorContext *persistor_ctx = nullptr) {
    assert(id != 0);
    Node *entry_point = m_entry_point.load();
    uint8_t max_layer = entry_point == nullptr ? 0 : entry_point->layer();
    const uint8_t target_layer = random_layer(max_layer);

    Node *new_node;
    {
      // Protect allocator and nodes maps ops.
      std::scoped_lock lock(m_global_lock);

      new_node = Node::create(m_allocator, *this, id, NODE_NEW);

      auto inserted [[maybe_unused]] = m_nodes.emplace(id, new_node);
      assert(inserted.second);

      new_node->set_layer(target_layer);
      new_node->alloc_neighbors(m_allocator, *this);
    }

    new_node->set_base_pk(base_pk);
    new_node->set_vec(*this, q);
    new_node->set_linking();

    if (entry_point == nullptr) {
      std::scoped_lock lock(m_entry_point_lock);
      entry_point = m_entry_point.load();
      if (entry_point == nullptr) {
        new_node->set_complete();
        m_persistor.insert_cb(persistor_ctx, id, base_pk, q, target_layer,
                              neighbor_ids(new_node));
        m_persistor.update_entry_point_cb(persistor_ctx, id);
        m_entry_point.store(new_node);
        return;
      } else {
        max_layer = entry_point->layer();
      }
    }

    // TODO: Think about possible optimizations of this.
    std::vector<Node *> scratch_buffer(get_Mmax(0));

    // HNSW entry point is always complete.
    assert(entry_point->state() == NODE_COMPLETE);

    NodeDist nearest_entry = {entry_point, dist(q, entry_point)};
    for (int l = max_layer; l > target_layer; --l) {
      nearest_entry = search_layer_ef_1(q, nearest_entry, l, persistor_ctx,
                                        scratch_buffer.data());
    }

    // search_layer_ef_1() post condition: the nearest entry node is complete.
    assert(nearest_entry.node->state() == NODE_COMPLETE);

    SearchLayerResult nearest{nearest_entry};
    std::set<Node *> updated_neighbors;

    for (int l = std::min(max_layer, target_layer); l >= 0; --l) {
      const size_t Mmax = get_Mmax(l);
      nearest = search_layer(q, std::move(nearest), m_ef_construction, l,
                             persistor_ctx, scratch_buffer.data());
      // Paper and reference implementation both use M when selecting neighbors
      // for layer 0 for newly inserted node, but it seems to be a typo.
      // Both MariaDB and pgVector use 2 * M for layer 0, so we do too.
      //
      // Note that thanks to the fact that nodes in the process of being
      // linked (NODE_LINKING state) are never choosen as neighbors for
      // concurrent insertions we can safely iterate over and modify
      // the new node's neighbors without any additional locking.
      Node **new_node_neighbors = new_node->neighbors_begin(*this, l);
      const size_t n [[maybe_unused]] =
          select_neighbors(q, nearest, Mmax, new_node_neighbors);
      assert(n <= Mmax);

      for (Node *const *it = new_node_neighbors;
           it != new_node->neighbors_end(*this, l) && *it != nullptr; ++it) {
        Node *neighbor = *it;
        // search_layer() + select_neighbors() post condition:
        // all neighbors of the newly inserted node are complete.
        assert(neighbor->state() == NODE_COMPLETE);

        // Back-linking neighbors requires locking though.
        lock_node(neighbor);
        Node **it2 = neighbor->neighbors_begin(*this, l);
        while (it2 != neighbor->neighbors_end(*this, l) && *it2 != nullptr) {
          ++it2;
        }
        if (it2 != neighbor->neighbors_end(*this, l)) {
          *it2 = new_node;
        } else {
          // Copy existing neighbors to scratch buffer to avoid
          // holding the lock while performing expensive calculations
          // and potential loads of not yet loaded nodes.
          //
          // Note that after we unlock node and before we do the re-lock +
          // write-back below, another insert may append to or rewrite this
          // same neighbor list. Blindly installing our pruned selection can
          // then drop that concurrent update. Worst case is a slightly worse
          // local graph (lost edge / suboptimal neighbors), which we accept.
          std::copy(neighbor->neighbors_begin(*this, l),
                    neighbor->neighbors_end(*this, l), scratch_buffer.data());
          unlock_node(neighbor);

          std::vector<NodeDist> candidate_neighbors;
          candidate_neighbors.reserve(Mmax + 1);
          for (size_t i = 0; i < Mmax; ++i) {
            Node *nb = scratch_buffer[i];
            assert(nb != nullptr);
            NodeState nb_state = nb->state();

            // Nodes in the NODE_NEW state are not yet part of the graph.
            assert(nb_state != NODE_NEW);

            // Skip nodes which are lost.
            //
            // NB: Note that unlike in search case we don't skip nodes in the
            //     process of being linked here. The rationale is:
            // 1) They have vector set so should be fine for select_neighbors()
            //    algorithm.
            // 2) We will have at least one such node, the newly inserted node
            //    itself, in the candidate list anyway.
            // 3) They represent work done by concurrent insertions, which we
            //    probably don't want to throw away.
            if (nb_state == NODE_LOST) continue;

            // Load the neighbor if it is not loaded yet, skip if it is lost.
            if (nb_state == NODE_DUMMY && !load_node(persistor_ctx, nb))
              continue;

            candidate_neighbors.push_back({nb, dist(neighbor->vec(), nb)});
          }
          candidate_neighbors.push_back(
              {new_node, dist(neighbor->vec(), new_node)});

          const size_t selected =
              select_neighbors(neighbor->vec(), candidate_neighbors, Mmax,
                               scratch_buffer.data());
          assert(selected <= Mmax);

          // If some candidates were skipped as LOST / failed lazy load,
          // select_neighbors may return fewer than Mmax; clear the tail so
          // write-back does not leave stale pointers past the packed prefix.
          for (size_t i = selected; i < Mmax; ++i) {
            scratch_buffer[i] = nullptr;
          }

          lock_node(neighbor);
          std::copy(scratch_buffer.data(), scratch_buffer.data() + Mmax,
                    neighbor->neighbors_begin(*this, l));
        }
        unlock_node(neighbor);
        updated_neighbors.insert(neighbor);
      }
    }

    m_persistor.insert_cb(persistor_ctx, id, base_pk, q, target_layer,
                          neighbor_ids(new_node));

    // Mark the node as complete. Doing this after calling persistor insert
    // callback ensures that nodes marked as such are always known to persistor.
    new_node->set_complete();

    for (Node *neighbor : updated_neighbors) {
      // search_layer() + select_neighbors() post condition:
      // all neighbors of newly inserted node are complete.
      // They have proper neighbor lists set and are known to persistor already.
      assert(neighbor->state() == NODE_COMPLETE);
      // Lock the neighbor so we can safely read its neighbor lists.
      // TODO: Think about possible optimizations of this/not holding
      //       the lock during the callback.
      lock_node(neighbor);
      m_persistor.update_neighbors_cb(persistor_ctx, neighbor->id(),
                                      neighbor_ids(neighbor));
      unlock_node(neighbor);
    }

    if (target_layer > max_layer) {
      std::scoped_lock lock(m_entry_point_lock);
      // Check if our new node still should be made the entry point.
      // Another node that just has been inserted concurrently might
      // already have taken the spot.
      if (target_layer > m_entry_point.load()->layer()) {
        m_persistor.update_entry_point_cb(persistor_ctx, id);
        m_entry_point.store(new_node);
      }
    }
  }

  /**
    Approximate k-nearest-neighbor search.

    Corresponds to K-NN-SEARCH (algorithm 5) in the HNSW paper.

    Mutates the in memory representation of the index: may load not yet
    loaded graph nodes via Persistor::load_node_cb when the search visits
    stubs (lazy-loaded graph nodes).

    @param q          Query vector.
    @param k          Number of nearest neighbors to return; must be > 0.
    @param ef_search  Size of the dynamic candidate list on layer 0 (clamped
                      to at least k).
    @param persistor_ctx  Persistor call context for lazy load during search.

    @return Up to k SearchHit values (graph id + base_pk) for q's nearest
            neighbors ordered by increasing distance.
  */
  std::vector<SearchHit> k_nn_search(
      const char *q, size_t k, size_t ef_search,
      PersistorContext *persistor_ctx = nullptr) {
    assert(k > 0);

    Node *const entry_point = m_entry_point.load();

    if (entry_point == nullptr) {
      return {};
    }

    // Entry point is always complete.
    assert(entry_point->state() == NODE_COMPLETE);

    std::vector<Node *> scratch_buffer(get_Mmax(0));

    const uint8_t max_layer = entry_point->layer();
    NodeDist nearest_entry = {entry_point, dist(q, entry_point)};
    for (int l = max_layer; l > 0; --l) {
      nearest_entry = search_layer_ef_1(q, nearest_entry, l, persistor_ctx,
                                        scratch_buffer.data());
    }
    // search_layer_ef_1() post condition: the nearest entry node is complete.
    assert(nearest_entry.node->state() == NODE_COMPLETE);

    // Asking for more rows than the configured search width silently
    // widens the search.
    SearchLayerResult nearest = search_layer(
        q, SearchLayerResult{nearest_entry}, std::max(ef_search, k), 0,
        persistor_ctx, scratch_buffer.data());
    // 'nearest' is a max-heap, so we need to throw away
    // the extra elements to get the closest k elements.
    while (nearest.size() > k) {
      nearest.pop();
    }
    // Use trick to return closest k elements in correct order.
    std::vector<SearchHit> result(nearest.size());
    for (size_t i = nearest.size(); i > 0; --i) {
      Node *node = nearest.top().node;
      result[i - 1] = {node->id(), node->base_pk()};
      nearest.pop();
    }
    return result;
  }

 private:
  class Node;

  struct NodeDist {
    Node *node;
    double distance;
  };

  struct NodeDistMinCmp {
    bool operator()(const NodeDist &a, const NodeDist &b) const {
      return a.distance > b.distance;
    }
  };

  struct NodeDistMaxCmp {
    bool operator()(const NodeDist &a, const NodeDist &b) const {
      return a.distance < b.distance;
    }
  };

  typedef std::priority_queue<NodeDist, std::vector<NodeDist>, NodeDistMinCmp>
      NodeDistMinQueue;

 public:
  /**
    Mutable state for a batched streaming nearest-neighbor search.

    Owned by the caller. Pass the same instance to nn_search_start() and
    repeated nn_search_next() calls. Call reset() before starting another
    search on the same context (asserted in debug builds). Destruction
    also releases resources.

    Not thread-safe: a single NNSearchContext must not be shared across
    threads. Concurrent streaming searches need a distinct context each.

    The HNSW index must outlive an in-progress search on this context.
    nn_search_start() stores the PersistorContext * passed to it; later
    nn_search_next() batch refills may invoke Persistor::load_node_cb through
    that pointer (see nn_search_start()).
  */
  class NNSearchContext {
   public:
    NNSearchContext() = default;
    ~NNSearchContext() { reset(); }
    NNSearchContext(const NNSearchContext &) = delete;
    NNSearchContext &operator=(const NNSearchContext &) = delete;
    NNSearchContext(NNSearchContext &&) = delete;
    NNSearchContext &operator=(NNSearchContext &&) = delete;

    /**
      Release the query copy and clear search state.
      Required before a subsequent nn_search_start() on this context.
    */
    void reset() {
      free(const_cast<char *>(m_query_vec));
      m_query_vec = nullptr;
      m_batch_size = 0;
      m_ef_search = 0;

      m_visited = std::unordered_set<Node *>();
      m_discarded = {};
      m_scratch_buffer.clear();

      m_results_batch.clear();
      m_current_batch_pos = 0;
      m_seen_distance = -std::numeric_limits<double>::infinity();
    }

   private:
    void init(const HNSW &hnsw, const char *q, size_t batch_size,
              size_t ef_search, PersistorContext *persistor_ctx) {
      char *query_vec = (char *)malloc(hnsw.m_dimensions * sizeof(float));
      // TODO: revisit once we add memory limits.
      if (query_vec == nullptr) throw std::bad_alloc();
      memcpy(query_vec, q, hnsw.m_dimensions * sizeof(float));
      assert(m_query_vec == nullptr);
      m_query_vec = query_vec;
      m_batch_size = batch_size;
      m_ef_search = ef_search;
      m_persistor_ctx = persistor_ctx;
      m_scratch_buffer.resize(hnsw.get_Mmax(0));
      assert(m_visited.empty());
      assert(m_discarded.empty());
      assert(m_results_batch.empty());
      assert(m_current_batch_pos == 0);
      assert(m_seen_distance == -std::numeric_limits<double>::infinity());
    }

    // Search parameters
    // Query vector is owned via malloc/free (not the HNSW arena):
    // NNSearchContext outlives individual next() calls but is much
    // shorter-lived than the index, and must be releasable on
    // reset()/destruction independently of m_allocator.
    const char *m_query_vec{nullptr};
    size_t m_batch_size{0};
    size_t m_ef_search{0};

    /// Persistor context for loading nodes.
    PersistorContext *m_persistor_ctx{nullptr};

    // Search state
    //
    // TODO: Is it possible to use minimal seen distance + result from
    //       previous batch as the only search state, like MariaDB does?
    //       Our implementation is more complex, but should be more exact.
    std::unordered_set<Node *> m_visited;
    // Min-heap of nodes which were removed from the tentative result set
    // and neighbors of nodes which are or were present in the result set
    // which themselves were considered to be not good enough to be included
    // into it while preparing the current/previous batch.
    // We use this "ring" of nodes to prime the algorithm in order to produce
    // the next batch.
    NodeDistMinQueue m_discarded;

    /// Scratch for lock-and-copy of a node's neighbor layer (size 2*M).
    std::vector<Node *> m_scratch_buffer;

    // Results
    std::vector<NodeDist> m_results_batch;
    size_t m_current_batch_pos{0};
    double m_seen_distance{-std::numeric_limits<double>::infinity()};

    friend class HNSW;
  };

  /**
    Start a streaming approximate nearest-neighbor search on @p ctx.

    Mutates the in-memory representation of the index: like k_nn_search(),
    may load not yet loaded graph nodes via Persistor::load_node_cb while
    traversing the graph. Pass @p persistor_ctx for lazy-loaded indexes;
    nullptr is valid only when all nodes are resident.

    @param ctx            Fresh or reset() context owned by the caller.
    @param q              Query vector (copied into @p ctx).
    @param batch_size     Results fetched per internal batch; must be > 0.
    @param ef_search      Search width (clamped to at least batch_size).
    @param persistor_ctx  Persistor call context; stored in @p ctx for the
                          duration of this search (including nn_search_next()
                          batch refills).

    @note This API is not intended to scan the entire or large part of the
          index as it is approximate and likely to omit some nodes.
          Optimizer should avoid using this API for queries which are likely
          to do so.
  */
  void nn_search_start(NNSearchContext *ctx, const char *q, size_t batch_size,
                       size_t ef_search,
                       PersistorContext *persistor_ctx = nullptr) {
    assert(batch_size > 0);
    size_t ef = std::max(batch_size, ef_search);
    ctx->init(*this, q, batch_size, ef, persistor_ctx);

    Node *const entry_point = m_entry_point.load();

    if (entry_point == nullptr) {
      return;
    }
    // Entry point is always complete.
    assert(entry_point->state() == NODE_COMPLETE);
    const uint8_t max_layer = entry_point->layer();
    NodeDist nearest_entry = {entry_point, dist(q, entry_point)};
    for (int l = max_layer; l > 0; --l) {
      nearest_entry = search_layer_ef_1(q, nearest_entry, l, persistor_ctx,
                                        ctx->m_scratch_buffer.data());
    }
    // search_layer_ef_1() post condition: the nearest entry node is complete.
    assert(nearest_entry.node->state() == NODE_COMPLETE);

    SearchLayerResult nearest{nearest_entry};
    NodeDistMinQueue candidates;
    candidates.push(nearest_entry);

    assert(ctx->m_visited.empty());
    ctx->m_visited.insert(nearest_entry.node);

    assert(ctx->m_discarded.empty());

    search_layer_core(q, &nearest, &candidates, &ctx->m_visited,
                      &ctx->m_discarded, ef, 0, persistor_ctx,
                      ctx->m_scratch_buffer.data());

    // We can safely ignore nodes left in candidates heap. There can
    // be only nodes in it which were part of the tentative result
    // set in the past but were removed from it at some point.
    // They will be present in the discarded heap as well.

    // Intuitively, using the whole nearest/result set should negatively impact
    // recall. Quick tests were not conclusive. TODO: Thoroughly investigate
    // this.
    while (nearest.size() > batch_size) {
      // search_layer_core() post condition:
      // all nodes in the nearest set are complete.
      assert(nearest.top().node->state() == NODE_COMPLETE);
      ctx->m_discarded.push(nearest.top());
      nearest.pop();
    }

    ctx->m_results_batch.resize(nearest.size());
    for (size_t i = nearest.size(); i > 0; --i) {
      // search_layer_core() post condition:
      // all nodes in the nearest set are complete.
      assert(nearest.top().node->state() == NODE_COMPLETE);
      ctx->m_results_batch[i - 1] = nearest.top();
      nearest.pop();
    }
    assert(ctx->m_current_batch_pos == 0);
  }

  /**
    Return the next neighbor from a streaming search, or
    {false, {0, 0}} when done.

    Yields SearchHit values (graph id + base_pk) in non-decreasing distance
    order.

    Mutates the in-memory representation of the index: uses the
    PersistorContext * stored by nn_search_start() and may call
    Persistor::load_node_cb.

    @note Since underlying algorithm is not exact, it might sometimes produce
          results out of order (then they belong to different batches).
          We solve this problem by omitting offending nodes from the result.

          Due to this and due to HNSW inherent properties this API is not
          intended for scanning the entire index or large part of it.
          Optimizer should avoid using this API for such cases.

    @param ctx  Context previously passed to nn_search_start().
  */
  std::pair<bool, SearchHit> nn_search_next(NNSearchContext *ctx) {
    while (true) {
      if (ctx->m_current_batch_pos >= ctx->m_results_batch.size()) {
        if (ctx->m_current_batch_pos < ctx->m_batch_size) {
          // The last batch was too short. This means that we have reached the
          // end of the graph. There should be no leftover discarded nodes.
          assert(ctx->m_discarded.empty());
          return {false, {0, 0}};
        } else {
          // If there are no discarded nodes left, we have reached the end of
          // the graph and there will be no more results.
          if (ctx->m_discarded.empty()) {
            return {false, {0, 0}};
          }

          SearchLayerResult nearest;
          NodeDistMinQueue candidates;

          while (nearest.size() < ctx->m_ef_search &&
                 !ctx->m_discarded.empty()) {
            nearest.push(ctx->m_discarded.top());
            // Some of the nodes from the discarded heap (those which were
            // removed from the tentative result set) might had their
            // neighbors considered already in the past. This is OK.
            // The visited set will filter such nodes' neighbors out.
            // OTOH, nodes from discarded heap which were added there
            // without entering the result set (or nodes which were
            // removed from the result set before their neighbors were
            // considered), do not have their neighbors inspected yet.
            // Hence we need to add them to the candidates heap.
            candidates.push(ctx->m_discarded.top());
            assert(ctx->m_visited.count(ctx->m_discarded.top().node) > 0);
            // Earlier search_layer_core() post condition:
            // all nodes in the discarded heap are complete.
            // Upcoming search_layer_core() pre condition:
            // all nodes in the candidates heap must be complete.
            assert(ctx->m_discarded.top().node->state() == NODE_COMPLETE);
            ctx->m_discarded.pop();
          }

          search_layer_core(ctx->m_query_vec, &nearest, &candidates,
                            &ctx->m_visited, &ctx->m_discarded,
                            ctx->m_ef_search, 0, ctx->m_persistor_ctx,
                            ctx->m_scratch_buffer.data());

          // Similarly to nn_search_start(), we can safely ignore nodes left
          // in candidates heap. They will be present in the discarded heap.

          // Again, the idea is that using the whole nearest/result set here
          // should negatively impact recall. Quick tests were not conclusive.
          // TODO: Thoroughly investigate this.
          while (nearest.size() > ctx->m_batch_size) {
            // search_layer_core() post condition:
            // all nodes in the nearest set are complete.
            assert(nearest.top().node->state() == NODE_COMPLETE);
            ctx->m_discarded.push(nearest.top());
            nearest.pop();
          }

          // Since search_layer_core() got some candidates, there must be at
          // least one result.
          assert(nearest.size() > 0);

          ctx->m_results_batch.resize(nearest.size());
          for (size_t i = nearest.size(); i > 0; --i) {
            // search_layer_core() post condition:
            // all nodes in the nearest set are complete.
            assert(nearest.top().node->state() == NODE_COMPLETE);
            ctx->m_results_batch[i - 1] = nearest.top();
            nearest.pop();
          }
          ctx->m_current_batch_pos = 0;
        }
      }
      const NodeDist &result = ctx->m_results_batch[ctx->m_current_batch_pos];
      ctx->m_current_batch_pos++;

      // The result from the new batch might be closer than the last seen
      // result, from the previous batch. If so, we must skip it to ensure
      // non-decreasing distance order. This is accepted trade-off/consequence
      // of using approximate algorithm.
      if (result.distance < ctx->m_seen_distance) continue;

      ctx->m_seen_distance = result.distance;
      return {true, {result.node->id(), result.node->base_pk()}};
    }
  }

  /**
    Load the index starting from a persisted entry-point node id.

    The HNSW instance must already have been constructed with the same
    dimensions, M, and dist_func as the index whose graph was persisted
    (see @tparam Persistor). Only the entry-point node is loaded immediately;
    other nodes load on demand during insert() or search.

    @note This API is not thread-safe. It must not be called concurrently
          with any insert or search operations.

    @param id             Graph node id of the persisted entry point.
    @param persistor_ctx  Context passed to load_node_cb..
  */
  void init_from_entry_point(uint64_t id, PersistorContext *persistor_ctx) {
    assert(m_entry_point.load() == nullptr);
    assert(m_nodes.size() == 0);
    Node *node = Node::create(m_allocator, *this, id, NODE_DUMMY);
    m_nodes.insert({id, node});
    bool loaded [[maybe_unused]] = load_node(persistor_ctx, node);
    assert(loaded);
    m_entry_point.store(node);
    assert(m_entry_point.load()->state() == NODE_COMPLETE);
  }

  /**
    Graph id of a NODE_DUMMY stub or loaded node referenced by @p handle.
    Valid before load_set_* calls (id is known at stub creation).
  */
  uint64_t load_node_id(LoadNodeHandle handle) const {
    const Node *node = static_cast<const Node *>(handle);
    return node->id();
  }

  /**
    Set top layer for a NODE_DUMMY stub (Persistor::load_node_cb).
    Must be called before load_node_neighbors().
  */
  void load_set_layer(LoadNodeHandle handle, uint8_t layer) {
    Node *node = static_cast<Node *>(handle);
    node->set_layer(layer);
  }

  /**
    Copy vector data into a NODE_DUMMY stub (Persistor::load_node_cb).
    @p q must point to @p dimensions floats (same layout as insert()).
  */
  void load_set_vec(LoadNodeHandle handle, const char *q) {
    Node *node = static_cast<Node *>(handle);
    node->set_vec(*this, q);
  }

  /**
    Set base-table row pk for a NODE_DUMMY stub (Persistor::load_node_cb).
  */
  void load_set_base_pk(LoadNodeHandle handle, uint64_t base_pk) {
    Node *node = static_cast<Node *>(handle);
    node->set_base_pk(base_pk);
  }

  /**
    Allocate neighbor storage and fill slots from persisted node ids.
    0-id means slot is empty. If a neighbor id is not yet in m_nodes, creates
    a NODE_DUMMY stub for it.

    Called from Persistor::load_node_cb after load_set_layer().
    @p ids must cover exactly all neighbor slots (size (layer+2)*M).
  */
  template <typename Range>
  void load_node_neighbors(LoadNodeHandle handle, Range ids) {
    Node *node = static_cast<Node *>(handle);
    assert(node != nullptr);
    assert(node->state() == NODE_DUMMY);

    // Protect all allocations and nodes map ops in this function.
    std::scoped_lock lock(m_global_lock);

    node->alloc_neighbors(m_allocator, *this);
    Node **neighbor_out = node->all_neighbors_begin(*this);
    Node **const neighbor_end [[maybe_unused]] = node->all_neighbors_end(*this);
    for (uint64_t id : ids) {
      assert(neighbor_out < neighbor_end);
      if (id == 0) {
        *neighbor_out++ = nullptr;
        continue;
      }
      auto it = m_nodes.find(id);
      Node *neighbor_node;
      if (it != m_nodes.end()) {
        neighbor_node = it->second;
      } else {
        neighbor_node = Node::create(m_allocator, *this, id, NODE_DUMMY);
        auto inserted [[maybe_unused]] = m_nodes.emplace(id, neighbor_node);
        assert(inserted.second);
      }
      *neighbor_out++ = neighbor_node;
    }
    assert(neighbor_out == neighbor_end);
  }

#ifndef NDEBUG
  /**
    Check internal graph consistency (debug builds only).

    Requires every node in m_nodes to be fully loaded/completed.
    Must not be called until all nodes that will be checked
    have been loaded/completed.

    @note This API is not thread-safe. It is intended for
          use in unit tests and debug builds only.
  */
  bool validate() const {
    // Empty index <=> no entry point.
    if (m_nodes.empty()) {
      return m_entry_point.load() == nullptr;
    }
    // Non-empty index must have an entry point.
    if (m_entry_point.load() == nullptr) {
      return false;
    }

    uint8_t max_layer = 0;
    bool found_ep = false;
    for (const auto &kv : m_nodes) {
      const Node *node = kv.second;
      // Map key must match node id; pointer must be non-null.
      if (node == nullptr || node->id() != kv.first) {
        return false;
      }
      max_layer = std::max(max_layer, node->layer());
      if (node == m_entry_point.load()) {
        found_ep = true;
      }
    }
    // Entry point must be in the map and sit on the highest layer.
    if (!found_ep || m_entry_point.load()->layer() != max_layer) {
      return false;
    }

    for (const auto &kv : m_nodes) {
      const Node *node = kv.second;
      for (uint8_t lc = 0; lc <= node->layer(); ++lc) {
        const size_t Mmax = get_Mmax(lc);
        Node *const *begin = node->neighbors_begin(*this, lc);
        Node *const *end = node->neighbors_end(*this, lc);

        // Neighbor slot span must equal Mmax for this layer.
        if (static_cast<size_t>(end - begin) != Mmax) {
          return false;
        }

        size_t degree = 0;
        bool seen_null = false;
        std::unordered_set<const Node *> seen_neighbors;
        for (Node *const *it = begin; it != end; ++it) {
          if (*it == nullptr) {
            seen_null = true;
            continue;
          }
          // No holes: non-null after a null slot.
          if (seen_null) {
            return false;
          }
          const Node *nb = *it;
          // No self-loops.
          if (nb == node) {
            return false;
          }
          // No duplicate neighbors on the same layer.
          if (!seen_neighbors.insert(nb).second) {
            return false;
          }
          // Neighbor must still be present in m_nodes under its id.
          const auto nit = m_nodes.find(nb->id());
          if (nit == m_nodes.end() || nit->second != nb) {
            return false;
          }
          // Neighbor must exist on this layer (its top layer >= lc).
          if (nb->layer() < lc) {
            return false;
          }
          ++degree;
        }
        // Degree cannot exceed Mmax (also implied by packed slots).
        if (degree > Mmax) {
          return false;
        }
      }
    }
    return true;
  }
#endif  // NDEBUG

 private:
  /**
    Graph node allocated as two arena blocks:

      [ Node | float vector ]
      [ neighbor slots ]

    Each region is ALIGN_SIZE-aligned. The vector holds m_dimensions floats.
    Neighbor storage is (m_layer + 2) * M Node* slots (layer 0 needs 2*M,
    layers 1..m_layer need M each). Within that array, layers are packed
    high-to-low: layer L starts at offset (m_layer - L) * M and spans
    get_Mmax(L) slots; unused slots are nullptr.
    Arena-owned: do not delete.
  */
  class Node {
   public:
    /**
      Allocate and initialize a node in @p allocator (see class layout).

      @param state  Initial lifecycle state: NODE_NEW for insert(),
                    NODE_DUMMY for lazy-load stubs in load_node_neighbors().
    */
    static Node *create(ArenaAllocator &allocator, const HNSW &hnsw,
                        uint64_t id, NodeState state) {
      // TODO: Needs to be adjusted to support quantization.
      const size_t vec_size = hnsw.m_dimensions * sizeof(float);
      void *raw_mem =
          allocator.allocate(ALIGN_SIZE(sizeof(Node)) + ALIGN_SIZE(vec_size));
      if (raw_mem == nullptr) throw std::bad_alloc();
      Node *node = new (raw_mem) Node(id, state);
      return node;
    }

    Node(const Node &) = delete;
    Node &operator=(const Node &) = delete;
    Node(Node &&) = delete;
    Node &operator=(Node &&) = delete;

    uint64_t id() const { return m_id; }

    NodeState state() const { return m_state.load(); }

    void set_linking() {
      assert(m_state.load() == NODE_NEW);
      m_state.store(NODE_LINKING);
    }

    void set_complete() {
#ifndef NDEBUG
      // While in this particular case doing single load vs multiple in a
      // single assert expression is not important, we do this for the sake
      // of consistency with other places where it is important.
      const NodeState s = m_state.load();
      assert(s == NODE_NEW ||  // the very first node case.
             s == NODE_LINKING || s == NODE_DUMMY);
#endif
      m_state.store(NODE_COMPLETE);
    }

    void set_lost() {
      assert(m_state.load() == NODE_DUMMY);
      m_state.store(NODE_LOST);
    }

    uint8_t layer() const {
#ifndef NDEBUG
      const NodeState s = m_state.load();
      assert(s == NODE_COMPLETE || s == NODE_LINKING);
#endif
      return m_layer;
    }
    void set_layer(uint8_t layer) {
#ifndef NDEBUG
      const NodeState s = m_state.load();
      assert(s == NODE_NEW || s == NODE_DUMMY);
#endif
      m_layer = layer;
    }

    const char *vec() const {
      return reinterpret_cast<const char *>(const_cast<Node *>(this)) +
             ALIGN_SIZE(sizeof(Node));
    }
    void set_vec(const HNSW &hnsw, const char *v) {
#ifndef NDEBUG
      const NodeState s = m_state.load();
      assert(s == NODE_NEW || s == NODE_DUMMY);
#endif
      memcpy(const_cast<char *>(vec()), v, hnsw.m_dimensions * sizeof(float));
    }

    uint64_t base_pk() const {
#ifndef NDEBUG
      const NodeState s = m_state.load();
      assert(s == NODE_COMPLETE || s == NODE_LINKING);
#endif
      return m_base_pk;
    }
    void set_base_pk(uint64_t base_pk) {
#ifndef NDEBUG
      const NodeState s = m_state.load();
      assert(s == NODE_NEW || s == NODE_DUMMY);
#endif
      m_base_pk = base_pk;
    }

    void alloc_neighbors(ArenaAllocator &allocator, const HNSW &hnsw) {
#ifndef NDEBUG
      const NodeState s = m_state.load();
      assert(s == NODE_NEW || s == NODE_DUMMY);
#endif
      const size_t neighbors_size =
          (static_cast<size_t>(m_layer) + 2) * hnsw.m_M * sizeof(Node *);
      m_neighbors = static_cast<Node **>(allocator.allocate(neighbors_size));
      if (m_neighbors == nullptr) throw std::bad_alloc();
      memset(m_neighbors, 0, neighbors_size);
    }

    Node **all_neighbors_begin(const HNSW &) const { return m_neighbors; }

    Node **all_neighbors_end(const HNSW &hnsw) const {
      return all_neighbors_begin(hnsw) + (m_layer + 2) * hnsw.m_M;
    }

    // Neighbor slot range for a layer: [neighbors_begin, neighbors_end).
    // Layout and per-layer width: see Node class comment / get_Mmax().
    Node **neighbors_begin(const HNSW &hnsw, uint8_t layer) const {
      return all_neighbors_begin(hnsw) + (m_layer - layer) * hnsw.m_M;
    }
    Node **neighbors_end(const HNSW &hnsw, uint8_t layer) const {
      return neighbors_begin(hnsw, layer) + hnsw.get_Mmax(layer);
    }

   private:
    /// Unique graph node id (0 is reserved; see insert() / NeighborIdIterator).
    const uint64_t m_id;
    /**
      Node lifecycle state; see HNSW::NodeState.
    */
    std::atomic<NodeState> m_state;

    // Layer, base_pk, vector bytes, and neighbor storage are filled while
    // NODE_NEW (insert) or NODE_DUMMY (load helpers), then become readable
    // to other threads once the node is NODE_LINKING or NODE_COMPLETE.

    /// Top layer on which the node is present
    /// (it is also present on all lower layers).
    uint8_t m_layer;
    /// Base table row pk (duplicates allowed).
    uint64_t m_base_pk;
    /**
      Outgoing neighbor slots; separate arena block (see class comment).
      Uninitialized on NODE_DUMMY stubs until load_node_neighbors() is called.
      Allocated while NODE_NEW during insert() before set_linking().

      One should not rely on the neighbor slots being correctly filled until
      node is transitioned to NODE_COMPLETE state.

      When allocated: (m_layer + 2) * M pointers, layers packed high-to-low
      (layer L at offset (m_layer - L) * M, width get_Mmax(L)); unused slots
      are nullptr. Persistor serializes the same flat id sequence via
      NeighborIdRange (0 = empty slot).
    */
    Node **m_neighbors;

    explicit Node(uint64_t id, NodeState state) : m_id(id), m_state(state) {}
  };

 public:
  /**
    Forward iterator over a node's neighbor slots as graph ids.

    Empty slots (nullptr) yield 0. Graph node id 0 is reserved for this
    sentinel so Persistor need not store a per-layer neighbor count.
    Use NeighborIdRange::size() for the slot count ((m_layer + 2) * M).
  */
  class NeighborIdIterator {
   public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = uint64_t;
    using difference_type = std::ptrdiff_t;
    using pointer = void;
    using reference = uint64_t;

    NeighborIdIterator() = default;
    explicit NeighborIdIterator(Node **it) : m_it(it) {}

    uint64_t operator*() const { return *m_it == nullptr ? 0 : (*m_it)->id(); }

    NeighborIdIterator &operator++() {
      ++m_it;
      return *this;
    }
    NeighborIdIterator operator++(int) {
      NeighborIdIterator tmp = *this;
      ++*this;
      return tmp;
    }

    bool operator==(NeighborIdIterator o) const { return m_it == o.m_it; }
    bool operator!=(NeighborIdIterator o) const { return m_it != o.m_it; }

   private:
    Node **m_it{nullptr};
  };

  /**
    [begin, end) range of neighbor graph ids for one node.

    Views the node's live neighbor slots; valid only for the duration of the
    Persistor callback that received it (see HNSW @tparam Persistor).
  */
  struct NeighborIdRange {
    NeighborIdIterator begin() const { return NeighborIdIterator(m_begin); }
    NeighborIdIterator end() const { return NeighborIdIterator(m_end); }
    size_t size() const { return static_cast<size_t>(m_end - m_begin); }

    Node **m_begin;
    Node **m_end;
  };

 private:
  NeighborIdRange neighbor_ids(const Node *node) const {
    return NeighborIdRange{node->all_neighbors_begin(*this),
                           node->all_neighbors_end(*this)};
  }

  const size_t m_dimensions;
  vec_dist_func_t *const m_dist_func;
  const size_t m_M;
  const size_t m_ef_construction;
  const double m_layer_factor;

  // Lock protecting allocator and nodes map.
  std::mutex m_global_lock;
  ArenaAllocator m_allocator;
  // Nodes are arena-allocated; pointers are non-owning.
  std::unordered_map<uint64_t, Node *> m_nodes;

  /**
    Search / insert entry point (HNSW paper).

    Invariants:
      - nullptr iff the index is empty (m_nodes.empty()).
      - When non-null: present in m_nodes, fully loaded, and sits on the
        highest layer present in the graph (its m_layer equals the global
        max). Nodes inserted later on that same layer do not replace it.
    Updated on first insert and when a new node is assigned a layer higher
    than the current max.

    @note This is an atomic pointer which is also protected by
          m_entry_point_lock. Readers can safely read the pointer
          value using atomic load operation. Writers must both
          acquire the lock and perform the atomic store operation.
          This ensures that changes of entry point in memory and
          on disk (through the Persistor) are done in-sync, and
          can't be affected by possible concurrent modifications
          of entry point.
  */
  std::atomic<Node *> m_entry_point{nullptr};
  /// Lock protecting entry point from concurrent modifications
  /// and ensuring that changes of entry point in memory and on disk
  /// are synchronized.
  std::mutex m_entry_point_lock;

  /// Layer-assignment RNG; see @tparam RandomEngine.
  RandomEngine m_rng;

  /// Stateless callback sink; see @tparam Persistor.
  Persistor m_persistor;
  /**
    Max neighbor slots on @p layer: 2*M on layer 0 (as suggested by the HNSW
    paper), M on higher layers.
  */
  size_t get_Mmax(size_t layer) const { return layer == 0 ? 2 * m_M : m_M; }

  /**
    Draw the top layer for a new node (HNSW paper level generation).

    @param current_max_layer  Current entry-point layer (0 if the index
                              is empty).

    @return Top layer index for the node being inserted.
            Value 0 is most likely to be returned, while higher values
            get exponentially sparser until reaching 255 (uint8_t max) cap.
            We additionally cap result to current_max_layer + 1 in order to
            throttle layer growth to avoid HNSW graph becoming too tall
            right from the start
  */
  uint8_t random_layer(uint8_t current_max_layer) {
    // Avoid log(0) by using minimum double value.
    const double u =
        std::max(std::uniform_real_distribution<double>(0.0, 1.0)(m_rng),
                 std::numeric_limits<double>::min());
    // Throttle layer growth and avoid UB caused by double -> uint8_t overflow.
    const uint8_t layer_cap = std::min<int>(
        current_max_layer + 1, std::numeric_limits<uint8_t>::max());
    return static_cast<uint8_t>(
        std::min<double>(-std::log(u) * m_layer_factor, layer_cap));
  }

  /**
    Max-heap of (node, distance) pairs for a single-layer HNSW search.

    Used as SEARCH-LAYER entry points, working set W, and return value.
    Iteration walks the underlying vector in heap layout (not sorted by
    distance). Move-only.
  */
  struct SearchLayerResult
      : std::priority_queue<NodeDist, std::vector<NodeDist>, NodeDistMaxCmp> {
    SearchLayerResult() = default;
    explicit SearchLayerResult(const NodeDist &ep) { this->push(ep); }

    SearchLayerResult(const SearchLayerResult &) = delete;
    SearchLayerResult &operator=(const SearchLayerResult &) = delete;
    SearchLayerResult(SearchLayerResult &&) = default;
    SearchLayerResult &operator=(SearchLayerResult &&) = default;

    std::vector<NodeDist>::const_iterator begin() const {
      return this->c.begin();
    }
    std::vector<NodeDist>::const_iterator end() const { return this->c.end(); }
  };

  double dist(const char *q, const Node *node) const {
    return m_dist_func(q, node->vec(), static_cast<uint32_t>(m_dimensions));
  }

  static constexpr size_t kNodeLockStripes = 16;  // Should be a power of 2.

  /// One mutex per cache line to avoid false sharing between stripes.
  struct alignas(64) LockStripe {
    std::mutex mutex;
  };

  // Array of mutexes which protect neighbor lists in nodes.
  LockStripe m_node_locks[kNodeLockStripes];

  static size_t node_lock_index(const Node *node) {
    return static_cast<size_t>(node->id()) & (kNodeLockStripes - 1);
  }

  void lock_node(const Node *node) {
    m_node_locks[node_lock_index(node)].mutex.lock();
  }
  void unlock_node(const Node *node) {
    m_node_locks[node_lock_index(node)].mutex.unlock();
  }

  static constexpr size_t kLoadNodeLockStripes = 4;  // Should be a power of 2.

  // Array of mutexes which protect from concurrent loads of the same node.
  LockStripe m_load_node_locks[kLoadNodeLockStripes];

  static size_t load_node_lock_index(const Node *node) {
    return static_cast<size_t>(node->id()) & (kLoadNodeLockStripes - 1);
  }

  void lock_load_node(const Node *node) {
    m_load_node_locks[load_node_lock_index(node)].mutex.lock();
  }
  void unlock_load_node(const Node *node) {
    m_load_node_locks[load_node_lock_index(node)].mutex.unlock();
  }

  /**
    Greedy one-nearest-neighbor search on a single layer (ef = 1).

    Corresponds to SEARCH-LAYER(..., ef=1) (algorithm 2) in the HNSW paper.
    Used when descending from the top layer toward the target layer during
    INSERT and K-NN-SEARCH, where only the nearest neighbor of the query is
    needed as the entry point for the next lower layer.

    Kept separate from search_layer() to avoid the overhead of a visited set
    and candidate priority queues when ef = 1.

    @param q            Query vector.
    @param entry_point  Entry element on this layer (node and distance to q).
    @param layer        Layer index to search.
    @param persistor_ctx  Persistor call context for lazy load during search.
    @param scratch_buffer  Caller-owned buffer of at least get_Mmax(layer)
                           Node* slots (callers typically allocate get_Mmax(0)
                           = 2*M). Used to lock-and-copy the current node's
                           neighbor list so distance / load work runs without
                           holding the node stripe lock. Not reentrant: must
                           not be shared across concurrent calls.

    @note Pre condition: @p entry_point node is complete.
          Post condition: the returned nearest neighbor node is complete.

    @return Nearest neighbor of q found on the given layer, with distance.
  */
  NodeDist search_layer_ef_1(const char *q, const NodeDist &entry_point,
                             uint8_t layer, PersistorContext *persistor_ctx,
                             Node **scratch_buffer) {
    Node *best_node = entry_point.node;
    double best_dist = entry_point.distance;
    const size_t Mmax = get_Mmax(layer);

    for (;;) {
      Node *cur_node = best_node;

      // The node we are currently inspecting must be complete.
      assert(cur_node->state() == NODE_COMPLETE);

      // Copy neighbors to scratch buffer to avoid expensive distance
      // calculations under the lock and complex handling of not yet
      // loaded node case.
      lock_node(cur_node);
      std::copy(cur_node->neighbors_begin(*this, layer),
                cur_node->neighbors_end(*this, layer), scratch_buffer);
      unlock_node(cur_node);

      for (size_t i = 0; i < Mmax && scratch_buffer[i] != nullptr; ++i) {
        Node *neighbor = scratch_buffer[i];
        NodeState neighbor_state = neighbor->state();

        // Nodes in the NODE_NEW state are not yet part of the graph.
        assert(neighbor_state != NODE_NEW);

        // Skip nodes which are in the process of being linked or lost.
        // The former might not have proper neighbor list on this layer
        // yet, so by following them we might end up in the graph deadend.
        // OTOH such nodes represent rows which should not be visible to
        // the current search anyway, so it is safe to skip them.
        if (neighbor_state == NODE_LINKING || neighbor_state == NODE_LOST)
          continue;

        // Load the neighbor if it is not loaded yet, skip if it is lost.
        if (neighbor_state == NODE_DUMMY && !load_node(persistor_ctx, neighbor))
          continue;

        assert(neighbor->state() == NODE_COMPLETE);

        const double neighbor_dist = dist(q, neighbor);
        if (neighbor_dist < best_dist) {
          best_node = neighbor;
          best_dist = neighbor_dist;
        }
      }

      if (best_node == cur_node) {
        return {best_node, best_dist};
      }
    }
  }

  /**
    Search for up to ef nearest neighbors of q on a single layer.

    Corresponds to SEARCH-LAYER (algorithm 2) in the HNSW paper.
    Used on the construction layers during INSERT (with ef = m_ef_construction)
    and on layer 0 during K-NN-SEARCH (with ef = ef_search).

    @param q   Query vector.
    @param entry_points  Entry points on this layer.
    @param ef  Size of the dynamic candidate / result list.
    @param layer  Layer index to search.
    @param persistor_ctx  Persistor call context for lazy load during search.
    @param scratch_buffer  Caller-owned neighbor scratch; see
                           search_layer_ef_1().

    @note Pre condition: @p entry_points nodes are complete.
          Post condition: all nodes in the result set are complete.

    @return Up to ef nearest neighbors of q on the given layer (as a
            max-heap of node/distance pairs; not sorted by distance when
            iterated).
  */
  SearchLayerResult search_layer(const char *q, SearchLayerResult entry_points,
                                 size_t ef, uint8_t layer,
                                 PersistorContext *persistor_ctx,
                                 Node **scratch_buffer) {
    assert(!entry_points.empty());
    std::unordered_set<Node *> visited;  // v in the paper
    // Guesstimate the number of unique nodes to visit in the layer.
    visited.reserve(ef * get_Mmax(layer));
    // C in the paper. Min-heap of nodes which neighbors we are going to
    // consider for inclusion in the result set.
    NodeDistMinQueue candidates;
    // W in the paper. Max-heap of nodes which are already in the tentative
    // result set (i.e. tentatively the closest ef nodes).
    SearchLayerResult result = std::move(entry_points);

    for (const NodeDist &entry : result) {
      const bool res [[maybe_unused]] = visited.insert(entry.node).second;
      assert(res == true);
      // Entry point nodes must be complete.
      assert(entry.node->state() == NODE_COMPLETE);
      candidates.push(entry);
    }

    search_layer_core(q, &result, &candidates, &visited, nullptr, ef, layer,
                      persistor_ctx, scratch_buffer);

    return result;
  }

  /**
    Core of SEARCH-LAYER() algorithm from the HNSW paper used by search_layer()
    and streaming search.

    @param q           Query vector.
    @param result      In/out result/working set max-heap (W in the paper).
                       Must be non-empty on entry.
    @param candidates  In/out min-heap of nodes whose neighbors to consider
                       (C in the paper).
    @param visited     In/out visited set (v in the paper).
    @param discarded   Optional sink for evicted W entries and neighbors
                       of C entries which were not admitted to W and C;
                       nullptr to drop them.
    @param ef          Maximum size of @p result.
    @param layer       Layer index to search.
    @param persistor_ctx  Persistor call context for lazy load during search.
    @param scratch_buffer  Caller-owned neighbor scratch; see
                           search_layer_ef_1().

    @note Pre condition: @p entry_points nodes are complete.
          Post condition: all nodes in the result set are complete.
          Post condition: all nodes in the discarded heap are complete.
  */
  void search_layer_core(const char *q, SearchLayerResult *result,
                         NodeDistMinQueue *candidates,
                         std::unordered_set<Node *> *visited,
                         NodeDistMinQueue *discarded, size_t ef, uint8_t layer,
                         PersistorContext *persistor_ctx,
                         Node **scratch_buffer) {
    assert(result != nullptr && !result->empty());
    assert(candidates != nullptr && !candidates->empty());
    // Early exit should not be possible as we populate C == W initially.
    assert(candidates->top().distance <= result->top().distance);
    assert(visited != nullptr);

    const size_t Mmax = get_Mmax(layer);

    while (!candidates->empty()) {
      const NodeDist c = candidates->top();
      candidates->pop();

      if (c.distance > result->top().distance) {
        break;
      }

      // The node we are currently inspecting must be complete.
      assert(c.node->state() == NODE_COMPLETE);

      lock_node(c.node);
      std::copy(c.node->neighbors_begin(*this, layer),
                c.node->neighbors_end(*this, layer), scratch_buffer);
      unlock_node(c.node);

      for (size_t i = 0; i < Mmax && scratch_buffer[i] != nullptr; ++i) {
        Node *e = scratch_buffer[i];
        NodeState e_state = e->state();

        // Nodes in the NODE_NEW state are not yet part of the graph.
        assert(e_state != NODE_NEW);

        // Skip nodes which are in the process of being linked or lost.
        // Let us discuss the rationale for the former in more detail.
        // There are two cases to consider:
        // 1. We are executing a search or a streaming search and
        //    are looking for nodes which are closest to the query
        //    on the layer 0. In this case neighbor nodes in the linking
        //    state do not have their neighbors on that level set yet,
        //    so they won't help us reaching other nodes. OTOH they are not
        //    interesting to us themselves since associated rows are not
        //    supposed to be visible to the search anyway.
        //    Hence it is reasonable to skip them.
        // 2. We are executing an insert and are selecting neighbors for
        //    the new node. In this case neighbor nodes in the linking
        //    state might not have proper neighbor list set on that layer
        //    as well. So they won't help us reaching other nodes.
        //    In theory, they might be interesting as candidate neighbors
        //    for our new node. But in practice, allowing that would
        //    complicate linking and back-linking process too much.
        //    Hence we skip them as well. The downside of this is that we
        //    might get slightly worse graph as result, but this situation
        //    should be rare and will be eventually remedied by the later
        //    insertions.
        if (e_state == NODE_LINKING || e_state == NODE_LOST) continue;

        // Load the neighbor if it is not loaded yet, skip if it is lost.
        if (e_state == NODE_DUMMY && !load_node(persistor_ctx, e)) continue;

        if (!visited->insert(e).second) {
          continue;
        }

        const double e_dist = dist(q, e);
        if (e_dist < result->top().distance || result->size() < ef) {
          candidates->push({e, e_dist});
          result->push({e, e_dist});
          if (result->size() > ef) {
            if (discarded != nullptr) {
              discarded->push(result->top());
            }
            result->pop();
          }
        } else {
          if (discarded != nullptr) {
            discarded->push({e, e_dist});
          }
        }
      }
    }
  }

  /**
    Select up to max_neighbors neighbors of q from candidate set.

    Corresponds to SELECT-NEIGHBORS (heuristic, algorithm 4) in the HNSW
    paper, including keepPrunedConnections but excluding extendCandidates.

    @param q    Query / base element vector (unused; distances come from
                candidates; kept for consistency with the paper).
    @param candidates     Candidate neighbors with distances to q.
    @param max_neighbors  Maximum number of neighbors to select.
    @param out  Output buffer of at least max_neighbors Node* slots;
                selected neighbors are written to out[0 .. return_value).
                Together with r_size/return value corresponds to R in the paper.

    @note Pre condition: all nodes in the candidates set are complete or in the
    linking state. Post condition: all nodes in the output buffer are complete
    or in the linking state.

    @return Number of neighbors written to out (at most max_neighbors).
  */
  template <typename NodeDistRange>
  size_t select_neighbors(const char * /* q */, const NodeDistRange &candidates,
                          size_t max_neighbors, Node **out) const {
    size_t r_size = 0;
    // W in the paper.
    std::priority_queue<NodeDist, std::vector<NodeDist>, NodeDistMinCmp>
        work_queue;
    // Discarded candidates in nearest-first order (same order as pops from W).
    std::vector<Node *> discarded;  // Wd from the paper.

    for (const NodeDist &entry : candidates) {
      // Candidates must be complete or in the linking state.
      //
      // Note it is important to do only one atomic load of the state here,
      // otherwise we might get spurious failures due to concurrent LINKING
      // -> COMPLETE transitions.
#ifndef NDEBUG
      NodeState entry_state = entry.node->state();
      assert(entry_state == NODE_COMPLETE || entry_state == NODE_LINKING);
#endif
      work_queue.push(entry);
    }

    while (!work_queue.empty() && r_size < max_neighbors) {
      const NodeDist e = work_queue.top();
      work_queue.pop();

      bool discard = false;
      for (size_t i = 0; i < r_size; ++i) {
        const double r_dist = dist(e.node->vec(), out[i]);
        /*
          A candidate e is discarded when it is closer or at least as close
          to some already-selected neighbor as it is to the query vector q.
        */
        if (e.distance >= r_dist) {
          discard = true;
          break;
        }
      }
      if (!discard) {
        out[r_size++] = e.node;
      } else {
        discarded.push_back(e.node);
      }
    }

    for (size_t i = 0; i < discarded.size() && r_size < max_neighbors; ++i) {
      out[r_size++] = discarded[i];
    }

    return r_size;
  }

  bool load_node(PersistorContext *persistor_ctx, Node *node) {
    // Lock the node to avoid concurrent loads of the same node.
    lock_load_node(node);
    // We need to re-check the state under the lock.
    switch (node->state()) {
      case NODE_DUMMY: {
        // Most likely case, indeed, the node needs to be loaded.
        //
        // Load callback must call, in order: load_set_layer(), load_set_vec(),
        // load_set_base_pk(), load_node_neighbors().
        bool loaded = m_persistor.load_node_cb(
            persistor_ctx, *this, static_cast<LoadNodeHandle>(node));
        if (loaded) {
          node->set_complete();
        } else {
          node->set_lost();
        }
        unlock_load_node(node);
        return loaded;
      }
      case NODE_COMPLETE:
        unlock_load_node(node);
        return true;
      case NODE_LOST:
        unlock_load_node(node);
        return false;
      case NODE_NEW:
      case NODE_LINKING:
      default:
        // NODE_NEW/LINKING are insert-owned, not loadable stubs.
        assert(false);
        unlock_load_node(node);
        return false;
    }
  }
};

#endif  // HNSW_H_INCLUDED
