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
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <new>
#include <queue>
#include <random>
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

  @tparam ArenaAllocator
    Arena allocator for graph nodes (and trailing vector / neighbor storage).
    Must provide void *allocate(size_t size); there is no per-block free.
    Returned memory must stay valid for this object's lifetime; destroying
    the allocator frees the arena. HNSW does not destroy Nodes and must not
    outlive the allocator. allocate() may return nullptr (asserted).
    Not assumed thread-safe.

  @todo (not in order of priority)
        1) Thread safety (intertwined with persistence callbacks).
        2) Support for deletes (might be unnecessary).
        3) Persistence callback API.
        4) Support for arbitrary PKs and additional data (e.g. versioning).
        5) Memory limits/expulsion strategy.
        6) Performance optimizations (visited set?).
*/
template <typename ArenaAllocator>
class HNSW {
 public:
  /**
    Construct an empty in-memory HNSW index.

    @param dimensions      Number of float elements per vector; must be > 0.
    @param dist_func       Distance function over two vectors of @p dimensions.
    @param M               Max out-degree on layers > 0 (layer 0 uses 2*M);
                           must be >= 2.
    @param ef_construction Size of the dynamic candidate list during INSERT;
                           clamped to at least M.
    @param seed            RNG seed for random layer assignment (default 42).
  */
  HNSW(size_t dimensions, vec_dist_func_t *dist_func, size_t M,
       size_t ef_construction, uint32_t seed = 42)
      : m_dimensions(dimensions),
        m_dist_func(dist_func),
        m_M(M),
        m_ef_construction(std::max(ef_construction, M)),
        m_layer_factor(1 / std::log(M)),
        m_neighbors_offset(calc_neighbors_offset(dimensions)),
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

    @param id       Unique graph node id (must not already exist in the index).
    @param base_pk  Primary key of the corresponding base-table row. Duplicates
                    are allowed (e.g. after updating an indexed vector, a new
                    graph node may share the same base_pk).
    @param q        Vector to insert.
  */
  void insert(uint64_t id, uint64_t base_pk, const char *q) {
    const uint8_t max_layer =
        m_entry_point == nullptr ? 0 : m_entry_point->m_layer;
    const uint8_t target_layer = random_layer(max_layer);

    Node *new_node =
        Node::create(m_allocator, *this, id, base_pk, q, target_layer);

    auto inserted [[maybe_unused]] = m_nodes.emplace(id, new_node);
    assert(inserted.second);

    if (m_entry_point == nullptr) {
      m_entry_point = new_node;
      return;
    }

    NodeDist nearest_entry = {m_entry_point, dist(q, m_entry_point)};
    for (int l = max_layer; l > target_layer; --l) {
      nearest_entry = search_layer_ef_1(q, nearest_entry, l);
      assert(nearest_entry.node != nullptr);
    }

    SearchLayerResult nearest{nearest_entry};

    for (int l = std::min(max_layer, target_layer); l >= 0; --l) {
      const size_t Mmax = get_Mmax(l);
      nearest = search_layer(q, std::move(nearest), m_ef_construction, l);
      // Paper and reference implementation both use M when selecting neighbors
      // for layer 0 for newly inserted node, but it seems to be a typo.
      // Both MariaDB and pgVector use 2 * M for layer 0, so we do too.
      Node **new_node_neighbors = new_node->neighbors_begin(*this, l);
      const size_t n [[maybe_unused]] =
          select_neighbors(q, nearest, Mmax, new_node_neighbors);
      assert(n <= Mmax);

      for (Node *const *it = new_node_neighbors;
           it != new_node->neighbors_end(*this, l) && *it != nullptr; ++it) {
        Node *neighbor = *it;
        Node **it2 = neighbor->neighbors_begin(*this, l);
        while (it2 != neighbor->neighbors_end(*this, l) && *it2 != nullptr) {
          ++it2;
        }
        if (it2 != neighbor->neighbors_end(*this, l)) {
          *it2 = new_node;
        } else {
          std::vector<NodeDist> candidate_neighbors;
          candidate_neighbors.reserve(Mmax + 1);
          for (Node *const *nb = neighbor->neighbors_begin(*this, l);
               nb != neighbor->neighbors_end(*this, l); ++nb) {
            candidate_neighbors.push_back(
                {*nb, dist(neighbor->get_vec(), *nb)});
          }
          candidate_neighbors.push_back(
              {new_node, dist(neighbor->get_vec(), new_node)});

          const size_t selected [[maybe_unused]] =
              select_neighbors(neighbor->get_vec(), candidate_neighbors, Mmax,
                               neighbor->neighbors_begin(*this, l));
          assert(selected == Mmax);
        }
      }
    }

    if (target_layer > max_layer) {
      m_entry_point = new_node;
    }
  }

  /**
    Approximate k-nearest-neighbor search.

    Corresponds to K-NN-SEARCH (algorithm 5) in the HNSW paper.

    @param q          Query vector.
    @param k          Number of nearest neighbors to return; must be > 0.
    @param ef_search  Size of the dynamic candidate list on layer 0 (clamped
                      to at least k).

    @return Up to k base_pk values for q's nearest neighbors ordered by
            increasing distance.
  */
  std::vector<uint64_t> k_nn_search(const char *q, size_t k,
                                    size_t ef_search) const {
    assert(k > 0);

    if (m_entry_point == nullptr) {
      return {};
    }
    const uint8_t max_layer = m_entry_point->m_layer;
    NodeDist nearest_entry = {m_entry_point, dist(q, m_entry_point)};
    for (int l = max_layer; l > 0; --l) {
      nearest_entry = search_layer_ef_1(q, nearest_entry, l);
    }
    // Asking for more rows than the configured search width silently
    // widens the search.
    SearchLayerResult nearest = search_layer(
        q, SearchLayerResult{nearest_entry}, std::max(ef_search, k), 0);
    // 'nearest' is a max-heap, so we need to throw away
    // the extra elements to get the closest k elements.
    while (nearest.size() > k) {
      nearest.pop();
    }
    // Use trick to return closest k elements in correct order.
    std::vector<uint64_t> result(nearest.size());
    for (size_t i = nearest.size(); i > 0; --i) {
      result[i - 1] = nearest.top().node->m_base_pk;
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

    The HNSW index must outlive an in-progress search on this context.
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

      m_results_batch.clear();
      m_current_batch_pos = 0;
      m_seen_distance = -std::numeric_limits<double>::infinity();
    }

   private:
    void init(const HNSW &hnsw, const char *q, size_t batch_size,
              size_t ef_search) {
      char *query_vec = (char *)malloc(hnsw.m_dimensions * sizeof(float));
      // TODO: revisit once we add memory limits.
      if (query_vec == nullptr) throw std::bad_alloc();
      memcpy(query_vec, q, hnsw.m_dimensions * sizeof(float));
      assert(m_query_vec == nullptr);
      m_query_vec = query_vec;
      m_batch_size = batch_size;
      m_ef_search = ef_search;
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

    // Results
    std::vector<NodeDist> m_results_batch;
    size_t m_current_batch_pos{0};
    double m_seen_distance{-std::numeric_limits<double>::infinity()};

    friend class HNSW;
  };

  /**
    Start a streaming approximate nearest-neighbor search on @p ctx.

    @param ctx         Fresh or reset() context owned by the caller.
    @param q           Query vector (copied into @p ctx).
    @param batch_size  Results fetched per internal batch; must be > 0.
    @param ef_search   Search width (clamped to at least batch_size).

    @note This API is not intended to scan the entire or large part of the
          index as it is approximate and likely to omit some nodes.
          Optimizer should avoid using this API for queries which are likely
          to do so.
  */
  void nn_search_start(NNSearchContext *ctx, const char *q, size_t batch_size,
                       size_t ef_search) const {
    assert(batch_size > 0);
    size_t ef = std::max(batch_size, ef_search);
    ctx->init(*this, q, batch_size, ef);

    if (m_entry_point == nullptr) {
      return;
    }
    const uint8_t max_layer = m_entry_point->m_layer;
    NodeDist nearest_entry = {m_entry_point, dist(q, m_entry_point)};
    for (int l = max_layer; l > 0; --l) {
      nearest_entry = search_layer_ef_1(q, nearest_entry, l);
    }

    SearchLayerResult nearest{nearest_entry};
    NodeDistMinQueue candidates;
    candidates.push(nearest_entry);

    assert(ctx->m_visited.empty());
    ctx->m_visited.insert(nearest_entry.node);

    assert(ctx->m_discarded.empty());

    search_layer_core(q, &nearest, &candidates, &ctx->m_visited,
                      &ctx->m_discarded, ef, 0);

    // We can safely ignore nodes left in candidates heap. There can
    // be only nodes in it which were part of the tentative result
    // set in the past but were removed from it at some point.
    // They will be present in the discarded heap as well.

    // Intuitively, using the whole nearest/result set should negatively impact
    // recall. Quick tests were not conclusive. TODO: Thoroughly investigate
    // this.
    while (nearest.size() > batch_size) {
      ctx->m_discarded.push(nearest.top());
      nearest.pop();
    }

    ctx->m_results_batch.resize(nearest.size());
    for (size_t i = nearest.size(); i > 0; --i) {
      ctx->m_results_batch[i - 1] = nearest.top();
      nearest.pop();
    }
    assert(ctx->m_current_batch_pos == 0);
  }

  /**
    Return the next neighbor from a streaming search, or {false, 0} when done.

    Yields base_pk values in non-decreasing distance order.

    @note Since underlying algorithm is not exact, it might sometimes produce
          results out of order (then they belong to different batches).
          We solve this problem by omitting offending nodes from the result.

          Due to this and due to HNSW inherent properties this API is not
          intended for scanning the entire index or large part of it.
          Optimizer should avoid using this API for such cases.

    @param ctx  Context previously passed to nn_search_start().
  */
  std::pair<bool, uint64_t> nn_search_next(NNSearchContext *ctx) const {
    while (true) {
      if (ctx->m_current_batch_pos >= ctx->m_results_batch.size()) {
        if (ctx->m_current_batch_pos < ctx->m_batch_size) {
          // The last batch was too short. This means that we have reached the
          // end of the graph. There should be no leftover discarded nodes.
          assert(ctx->m_discarded.empty());
          return {false, 0};
        } else {
          // If there are no discarded nodes left, we have reached the end of
          // the graph and there will be no more results.
          if (ctx->m_discarded.empty()) {
            return {false, 0};
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
            ctx->m_discarded.pop();
          }

          search_layer_core(ctx->m_query_vec, &nearest, &candidates,
                            &ctx->m_visited, &ctx->m_discarded,
                            ctx->m_ef_search, 0);

          // Similarly to nn_search_start(), we can safely ignore nodes left
          // in candidates heap. They will be present in the discarded heap.

          // Again, the idea is that using the whole nearest/result set here
          // should negatively impact recall. Quick tests were not conclusive.
          // TODO: Thoroughly investigate this.
          while (nearest.size() > ctx->m_batch_size) {
            ctx->m_discarded.push(nearest.top());
            nearest.pop();
          }

          // Since search_layer_core() got some candidates, there must be at
          // least one result.
          assert(nearest.size() > 0);

          ctx->m_results_batch.resize(nearest.size());
          for (size_t i = nearest.size(); i > 0; --i) {
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
      return {true, result.node->m_base_pk};
    }
  }

#ifndef NDEBUG
  /**
    Check internal graph consistency (debug builds only).
  */
  bool validate() const {
    // Empty index <=> no entry point.
    if (m_nodes.empty()) {
      return m_entry_point == nullptr;
    }
    // Non-empty index must have an entry point.
    if (m_entry_point == nullptr) {
      return false;
    }

    uint8_t max_layer = 0;
    bool found_ep = false;
    for (const auto &kv : m_nodes) {
      const Node *node = kv.second;
      // Map key must match node id; pointer must be non-null.
      if (node == nullptr || node->m_id != kv.first) {
        return false;
      }
      max_layer = std::max(max_layer, node->m_layer);
      if (node == m_entry_point) {
        found_ep = true;
      }
    }
    // Entry point must be in the map and sit on the highest layer.
    if (!found_ep || m_entry_point->m_layer != max_layer) {
      return false;
    }

    for (const auto &kv : m_nodes) {
      const Node *node = kv.second;
      for (uint8_t lc = 0; lc <= node->m_layer; ++lc) {
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
          const auto nit = m_nodes.find(nb->m_id);
          if (nit == m_nodes.end() || nit->second != nb) {
            return false;
          }
          // Neighbor must exist on this layer (its top layer >= lc).
          if (nb->m_layer < lc) {
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
    Graph node allocated as one arena block:

      [ Node | float vector | neighbor slots ]

    Each region is ALIGN_SIZE-aligned. The vector holds m_dimensions floats.
    Neighbor storage is (m_layer + 2) * M Node* slots (layer 0 needs 2*M,
    layers 1..m_layer need M each). Within that array, layers are packed
    high-to-low: layer L starts at offset (m_layer - L) * M and spans
    get_Mmax(L) slots; unused slots are nullptr. Neighbor array starts at
    byte offset HNSW::m_neighbors_offset from Node. Arena-owned: do not delete.
  */
  class Node {
   public:
    /**
      Allocate and initialize a node in @p allocator (see class layout).
    */
    static Node *create(ArenaAllocator &allocator, const HNSW &hnsw,
                        uint64_t id, uint64_t base_pk, const char *vec,
                        uint8_t layer) {
      // TODO: Needs to be adjusted to support quantization.
      const size_t vec_size = hnsw.m_dimensions * sizeof(float);
      const size_t neighbors_size =
          (static_cast<size_t>(layer) + 2) * hnsw.m_M * sizeof(Node *);
      void *raw_mem =
          allocator.allocate(ALIGN_SIZE(sizeof(Node)) + ALIGN_SIZE(vec_size) +
                             ALIGN_SIZE(neighbors_size));

      // TODO: revisit once we add memory limits.
      if (raw_mem == nullptr) throw std::bad_alloc();

      assert(ALIGN_SIZE(sizeof(Node)) + ALIGN_SIZE(vec_size) ==
             hnsw.m_neighbors_offset);
      Node *node = new (raw_mem) Node(id, base_pk, layer);
      memcpy(node->get_vec(), vec, vec_size);
      memset(node->get_neighbors(hnsw), 0, neighbors_size);
      return node;
    }

    Node(const Node &) = delete;
    Node &operator=(const Node &) = delete;
    Node(Node &&) = delete;
    Node &operator=(Node &&) = delete;

    char *get_vec() const {
      return reinterpret_cast<char *>(const_cast<Node *>(this)) +
             ALIGN_SIZE(sizeof(Node));
    }
    Node **get_neighbors(const HNSW &hnsw) const {
      return reinterpret_cast<Node **>(
          reinterpret_cast<char *>(const_cast<Node *>(this)) +
          hnsw.m_neighbors_offset);
    }

    // Neighbor slot range for a layer: [neighbors_begin, neighbors_end).
    // Layout and per-layer width: see Node class comment / get_Mmax().
    Node **neighbors_begin(const HNSW &hnsw, uint8_t layer) const {
      return get_neighbors(hnsw) + (m_layer - layer) * hnsw.m_M;
    }
    Node **neighbors_end(const HNSW &hnsw, uint8_t layer) const {
      return neighbors_begin(hnsw, layer) + hnsw.get_Mmax(layer);
    }

    /// Unique graph node id.
    const uint64_t m_id;
    /// Base table row pk (duplicates allowed).
    const uint64_t m_base_pk;
    /// Top layer on which the node is present
    /// (it is also present on all lower layers).
    const uint8_t m_layer;

   private:
    Node(uint64_t id, uint64_t base_pk, uint8_t layer)
        : m_id(id), m_base_pk(base_pk), m_layer(layer) {}
  };

  /** Byte offset from Node* to neighbor slots. */
  static size_t calc_neighbors_offset(size_t dimensions) {
    return ALIGN_SIZE(sizeof(Node)) + ALIGN_SIZE(dimensions * sizeof(float));
  }

  ArenaAllocator m_allocator;
  const size_t m_dimensions;
  vec_dist_func_t *const m_dist_func;
  const size_t m_M;
  const size_t m_ef_construction;
  const double m_layer_factor;
  const size_t m_neighbors_offset;

  // Nodes are arena-allocated; pointers are non-owning.
  std::unordered_map<uint64_t, Node *> m_nodes;
  Node *m_entry_point{nullptr};
  std::default_random_engine m_rng;

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
    return m_dist_func(q, node->get_vec(), static_cast<uint32_t>(m_dimensions));
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

    @return Nearest neighbor of q found on the given layer, with distance.
  */
  NodeDist search_layer_ef_1(const char *q, const NodeDist &entry_point,
                             uint8_t layer) const {
    Node *best_node = entry_point.node;
    double best_dist = entry_point.distance;

    for (;;) {
      Node *cur_node = best_node;

      for (Node *const *it = cur_node->neighbors_begin(*this, layer);
           it != cur_node->neighbors_end(*this, layer) && *it != nullptr;
           ++it) {
        Node *neighbor = *it;
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

    @return Up to ef nearest neighbors of q on the given layer (as a
            max-heap of node/distance pairs; not sorted by distance when
            iterated).
  */
  SearchLayerResult search_layer(const char *q, SearchLayerResult entry_points,
                                 size_t ef, uint8_t layer) const {
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
      candidates.push(entry);
    }

    search_layer_core(q, &result, &candidates, &visited, nullptr, ef, layer);

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
*/
  void search_layer_core(const char *q, SearchLayerResult *result,
                         NodeDistMinQueue *candidates,
                         std::unordered_set<Node *> *visited,
                         NodeDistMinQueue *discarded, size_t ef,
                         uint8_t layer) const {
    assert(result != nullptr && !result->empty());
    assert(candidates != nullptr && !candidates->empty());
    // Early exit should not be possible as we populate C == W initially.
    assert(candidates->top().distance <= result->top().distance);
    assert(visited != nullptr);

    while (!candidates->empty()) {
      const NodeDist c = candidates->top();
      candidates->pop();

      if (c.distance > result->top().distance) {
        break;
      }

      for (Node *const *it = c.node->neighbors_begin(*this, layer);
           it != c.node->neighbors_end(*this, layer) && *it != nullptr; ++it) {
        Node *e = *it;
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
      work_queue.push(entry);
    }

    while (!work_queue.empty() && r_size < max_neighbors) {
      const NodeDist e = work_queue.top();
      work_queue.pop();

      bool discard = false;
      for (size_t i = 0; i < r_size; ++i) {
        const double r_dist = dist(e.node->get_vec(), out[i]);
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
};

#endif  // HNSW_H_INCLUDED
