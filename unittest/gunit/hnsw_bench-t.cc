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

/**
  @file

  Recall / memory / throughput benchmark for the in-memory HNSW index.

  This target is defined but NOT built by default and is never run by ctest;
  it builds a large index and is meant to be run deliberately. Build and run
  it with:

    ninja hnsw_bench-t          # or: make hnsw_bench-t
    ./runtime_output_directory/hnsw_bench-t

  Timing benchmarks only (they are skipped in debug builds):

    ./runtime_output_directory/hnsw_bench-t --gtest_filter='Microbenchmarks*'

  The data set is sized by environment variables, so the whole curve can be
  re-measured at a different scale without rebuilding:

    HNSW_BENCH_POINTS  number of indexed vectors
    HNSW_BENCH_DIMS    dimensions per vector
    HNSW_BENCH_QUERIES number of query vectors
    HNSW_BENCH_M       max out-degree above layer 0
    HNSW_BENCH_EFC     ef_construction
    HNSW_BENCH_K       k for recall@k

  Unlike the correctness tests in hnsw-t.cc, this file asserts almost nothing.
  Recall depends on the data distribution, so absolute thresholds are a
  flakiness generator; the assertions that do exist are either structural
  (no duplicate rows) or scale-free (recall must improve with ef_search).
  The numbers are printed for a human to read.
*/

#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <unordered_set>
#include <utility>
#include <vector>

#include "unittest/gunit/benchmark.h"
#include "unittest/gunit/hnsw_test_utils.h"
#include "vector-common/vector_distance.h"

namespace hnsw_bench {

using hnsw_unittest::ArenaHandover;
using hnsw_unittest::as_bytes;
using hnsw_unittest::BorrowedHnsw;
using hnsw_unittest::drain_stream;
using hnsw_unittest::make_clustered_points;
using hnsw_unittest::make_pseudo_random_points;

// Keeps the optimizer from discarding search results.
static volatile uint64_t bench_sink = 0;

#ifdef NDEBUG
constexpr size_t kDefaultPoints = 20000;
constexpr size_t kDefaultDims = 64;
constexpr size_t kDefaultQueries = 50;
constexpr size_t kDefaultM = 16;
constexpr size_t kDefaultEfConstruction = 200;
#else
// A 20000 x 64 build at -O0 with the hot-path asserts live runs into minutes,
// so debug builds measure a small index just to keep the code exercised.
constexpr size_t kDefaultPoints = 2000;
constexpr size_t kDefaultDims = 16;
constexpr size_t kDefaultQueries = 10;
constexpr size_t kDefaultM = 8;
constexpr size_t kDefaultEfConstruction = 32;
#endif

constexpr size_t kDefaultK = 10;

/** ef_search values swept by RecallVsEfSearch. */
constexpr size_t kEfSearchGrid[] = {10, 16, 32, 64, 128, 256};

/**
  Results fetched per streaming batch.

  Must stay well below k: with batch_size >= k the first batch already
  satisfies the request, the stream never refills, and it would return exactly
  what k_nn_search returns -- making the comparison between the two paths
  vacuous.
*/
constexpr size_t kStreamBatchSize = 3;

/**
  Clusters in the synthetic data set, and how tight they are.

  The spread is tuned so recall is graded across the whole ef_search grid
  rather than saturating early: at this setting clusters overlap substantially
  but structure remains, and recall runs from roughly 0.79 at ef_search=10 to
  roughly 0.998 at 256. Tighter clusters (spread <= 1.5) put every true
  neighbor in an easily-found pocket and pin recall at 1.0 from ef_search=32
  up, which measures nothing.
*/
constexpr size_t kNumClusters = 100;
constexpr float kClusterSpread = 3.00f;

/**
  MEM_ROOT block size for the index arena.

  Deliberately far larger than the 4096 used by the correctness tests: a
  64-dimension node is ~536 bytes, so at 4096 only seven nodes fit per block
  and the reported footprint would be dominated by abandoned block tails
  rather than by the index itself.
*/
constexpr size_t kArenaBlockSize = 1024 * 1024;

static size_t read_size_env(const char *name, size_t default_value) {
  const char *value = std::getenv(name);
  if (value == nullptr || *value == '\0') {
    return default_value;
  }
  char *end = nullptr;
  const unsigned long long parsed = std::strtoull(value, &end, 10);
  if (end == value || *end != '\0' || parsed == 0) {
    std::cerr << "hnsw_bench: ignoring invalid " << name << "='" << value
              << "', using " << default_value << "\n";
    return default_value;
  }
  return static_cast<size_t>(parsed);
}

static double seconds_since(
    const std::chrono::steady_clock::time_point &start) {
  return std::chrono::duration<double>(std::chrono::steady_clock::now() - start)
      .count();
}

/**
  Data set, index and brute-force ground truth, built once for the whole
  binary.

  Building is by far the most expensive thing here, and the microbenchmark
  harness calls each benchmark body twice (a calibration pass and the timed
  run), so sharing this is what keeps the run to seconds rather than minutes.
*/
class BenchFixture {
 public:
  BenchFixture()
      : m_num_points(read_size_env("HNSW_BENCH_POINTS", kDefaultPoints)),
        m_dims(read_size_env("HNSW_BENCH_DIMS", kDefaultDims)),
        m_num_queries(read_size_env("HNSW_BENCH_QUERIES", kDefaultQueries)),
        m_M(read_size_env("HNSW_BENCH_M", kDefaultM)),
        m_ef_construction(
            read_size_env("HNSW_BENCH_EFC", kDefaultEfConstruction)),
        m_k(read_size_env("HNSW_BENCH_K", kDefaultK)),
        m_arena(kArenaBlockSize) {
    // The index and the ground truth must rank with the same function, and
    // this is the SIMD-dispatched kernel the server itself uses. Squaring
    // does not change the ordering, so recall is the same as it would be for
    // true L2.
    init_vector_distance_functions();

    uint64_t state = 42;
    const std::vector<std::vector<float>> centers =
        make_pseudo_random_points(kNumClusters, m_dims, &state);
    m_points =
        make_clustered_points(centers, m_num_points, kClusterSpread, &state);
    // Queries are drawn from the same centers -- otherwise they land in empty
    // space between clusters and the run measures out-of-distribution
    // behavior -- but from fresh draws, so no query is an exact copy of an
    // indexed point (a self-match would take one of the k slots for free).
    m_queries =
        make_clustered_points(centers, m_num_queries, kClusterSpread, &state);

    const auto build_start = std::chrono::steady_clock::now();
    {
      ArenaHandover handover(&m_arena);
      m_index.emplace(m_dims, vector_distance_euclidean_squared, m_M,
                      m_ef_construction);
    }
    for (size_t i = 0; i < m_num_points; ++i) {
      m_index->insert(i + 1, /*base_pk=*/i, as_bytes(m_points[i]));
    }
    m_build_seconds = seconds_since(build_start);

    const auto truth_start = std::chrono::steady_clock::now();
    m_ground_truth.resize(m_num_queries);
    std::vector<std::pair<double, uint64_t>> scratch;
    scratch.reserve(m_num_points);
    const size_t truth_k = std::min(m_k, m_num_points);
    for (size_t q = 0; q < m_num_queries; ++q) {
      scratch.clear();
      for (size_t i = 0; i < m_num_points; ++i) {
        scratch.emplace_back(vector_distance_euclidean_squared(
                                 as_bytes(m_queries[q]), as_bytes(m_points[i]),
                                 static_cast<uint32_t>(m_dims)),
                             i);
      }
      std::partial_sort(scratch.begin(), scratch.begin() + truth_k,
                        scratch.end());
      for (size_t i = 0; i < truth_k; ++i) {
        m_ground_truth[q].insert(scratch[i].second);
      }
    }
    m_ground_truth_seconds = seconds_since(truth_start);
  }

  BenchFixture(const BenchFixture &) = delete;
  BenchFixture &operator=(const BenchFixture &) = delete;

  size_t num_points() const { return m_num_points; }
  size_t dims() const { return m_dims; }
  size_t num_queries() const { return m_num_queries; }
  size_t M() const { return m_M; }
  size_t ef_construction() const { return m_ef_construction; }
  size_t k() const { return m_k; }
  double build_seconds() const { return m_build_seconds; }
  double ground_truth_seconds() const { return m_ground_truth_seconds; }
  size_t arena_allocated_bytes() const {
    return m_arena.mem_root.allocated_size();
  }
  size_t arena_requested_bytes() const { return m_arena.bytes_requested; }
  size_t arena_requests() const { return m_arena.requests_number; }

  BorrowedHnsw &index() { return *m_index; }
  const std::vector<float> &query(size_t q) const { return m_queries[q]; }
  const std::unordered_set<uint64_t> &ground_truth(size_t q) const {
    return m_ground_truth[q];
  }

  /** Fraction of the true top-k present in @p found (duplicates ignored). */
  double recall_of(const std::vector<uint64_t> &found, size_t q) const {
    const std::unordered_set<uint64_t> distinct(found.begin(), found.end());
    size_t hits = 0;
    for (uint64_t base_pk : distinct) {
      hits += m_ground_truth[q].count(base_pk);
    }
    return static_cast<double>(hits) /
           static_cast<double>(m_ground_truth[q].size());
  }

  void print_config() const {
    std::printf(
        "\nHNSW benchmark: %zu points x %zu dims, %zu queries, M=%zu, "
        "ef_construction=%zu, recall@%zu\n"
        "  data: %zu clusters, spread %.2f    build: %.2fs    "
        "ground truth: %.2fs\n\n",
        m_num_points, m_dims, m_num_queries, m_M, m_ef_construction, m_k,
        kNumClusters, static_cast<double>(kClusterSpread), m_build_seconds,
        m_ground_truth_seconds);
  }

 private:
  const size_t m_num_points;
  const size_t m_dims;
  const size_t m_num_queries;
  const size_t m_M;
  const size_t m_ef_construction;
  const size_t m_k;

  std::vector<std::vector<float>> m_points;
  std::vector<std::vector<float>> m_queries;
  std::vector<std::unordered_set<uint64_t>> m_ground_truth;

  // Declared before the index: nodes point into this arena, so it has to
  // outlive them. Owned here rather than inside the index so that its size is
  // observable from outside.
  hnsw_unittest::ArenaStats m_arena;
  std::optional<BorrowedHnsw> m_index;

  double m_build_seconds = 0.0;
  double m_ground_truth_seconds = 0.0;
};

/** Built on first use, so a --gtest_filter run only pays for what it selects.
 */
static BenchFixture &fixture() {
  static BenchFixture instance;
  return instance;
}

/** Recall of k_nn_search() at @p ef_search, averaged over all queries. */
struct RecallStats {
  double average = 0.0;
  double worst = 1.0;
};

static RecallStats measure_knn_recall(BenchFixture &f, size_t ef_search) {
  RecallStats stats;
  for (size_t q = 0; q < f.num_queries(); ++q) {
    const std::vector<uint64_t> found =
        f.index().k_nn_search(as_bytes(f.query(q)), f.k(), ef_search);
    const double recall = f.recall_of(found, q);
    stats.average += recall;
    stats.worst = std::min(stats.worst, recall);
  }
  stats.average /= static_cast<double>(f.num_queries());
  return stats;
}

static RecallStats measure_stream_recall(BenchFixture &f, size_t ef_search) {
  RecallStats stats;
  for (size_t q = 0; q < f.num_queries(); ++q) {
    const std::vector<uint64_t> found =
        drain_stream(f.index(), as_bytes(f.query(q)),
                     std::min(kStreamBatchSize, f.k()), ef_search,
                     /*max_results=*/f.k());
    const double recall = f.recall_of(found, q);
    stats.average += recall;
    stats.worst = std::min(stats.worst, recall);
  }
  stats.average /= static_cast<double>(f.num_queries());
  return stats;
}

TEST(HnswBenchmark, RecallVsEfSearch) {
  BenchFixture &f = fixture();
  f.print_config();

  std::printf("  %9s | %9s %9s | %9s %9s | %8s\n", "ef_search", "knn_avg",
              "knn_worst", "str_avg", "str_worst", "delta");
  std::printf("  %9s-+-%9s-%9s-+-%9s-%9s-+-%8s\n", "---------", "---------",
              "---------", "---------", "---------", "--------");

  RecallStats first_knn;
  RecallStats first_stream;
  RecallStats last_knn;
  RecallStats last_stream;
  bool first = true;

  for (size_t ef_search : kEfSearchGrid) {
    const RecallStats knn = measure_knn_recall(f, ef_search);
    const RecallStats stream = measure_stream_recall(f, ef_search);

    std::printf("  %9zu | %9.4f %9.4f | %9.4f %9.4f | %+8.4f\n", ef_search,
                knn.average, knn.worst, stream.average, stream.worst,
                stream.average - knn.average);

    if (first) {
      first_knn = knn;
      first_stream = stream;
      first = false;
    }
    last_knn = knn;
    last_stream = stream;
  }
  std::printf("\n");

  // Scale-free and distribution-independent: whatever the data looks like,
  // a wider search must not find fewer of the true neighbors. This is the
  // assertion that actually catches a regression.
  EXPECT_GE(last_knn.average, first_knn.average)
      << "k_nn_search recall did not improve with ef_search";
  EXPECT_GE(last_stream.average, first_stream.average)
      << "streaming recall did not improve with ef_search";

  // Deliberately wide floors, calibrated from a real run rather than guessed:
  // both paths measure ~0.998 at the widest ef_search on this data set, so
  // these leave room for compiler, libm and data-layout drift while still
  // catching a collapse. They are separate because the streaming path drops
  // any row closer than the last one it yielded (hnsw.h, m_seen_distance) and
  // so is not guaranteed to track k_nn_search.
  EXPECT_GE(last_knn.average, 0.85) << "k_nn_search recall collapsed";
  EXPECT_GE(last_stream.average, 0.80) << "streaming recall collapsed";
}

TEST(HnswBenchmark, StreamFullDrainQuality) {
  BenchFixture &f = fixture();

  // A full drain costs O(N * Mmax) distance evaluations, so sample a few
  // queries rather than the whole set.
  const size_t num_drains = std::min<size_t>(3, f.num_queries());
  constexpr size_t kDrainEfSearch = 64;

  std::printf("  full drain (ef_search=%zu, batch=%zu), %zu queries:\n",
              kDrainEfSearch, kStreamBatchSize, num_drains);

  for (size_t q = 0; q < num_drains; ++q) {
    // max_results has to exceed the graph size, or the drain is truncated and
    // the completeness ratio below would be meaningless.
    const std::vector<uint64_t> streamed = drain_stream(
        f.index(), as_bytes(f.query(q)), kStreamBatchSize, kDrainEfSearch,
        /*max_results=*/f.num_points() + 1);

    const std::unordered_set<uint64_t> distinct(streamed.begin(),
                                                streamed.end());
    const size_t duplicates = streamed.size() - distinct.size();
    const double completeness = static_cast<double>(distinct.size()) /
                                static_cast<double>(f.num_points());

    std::printf(
        "    q=%zu  rows %6zu  distinct %6zu  duplicates %4zu  "
        "completeness %6.2f%%\n",
        q, streamed.size(), distinct.size(), duplicates, 100.0 * completeness);

    // Structural invariant, and the one the first version of the streaming
    // code violated. This test builds a 1:1 id <-> base_pk mapping, so a
    // repeated base_pk really is the same graph node twice; HNSW itself
    // permits distinct nodes to share a base_pk.
    EXPECT_EQ(0U, duplicates) << "q=" << q << ": stream yielded the same row "
                              << duplicates << " time(s) over";
  }

  // Completeness is reported, never asserted to be 100%: rows are legitimately
  // dropped by the documented m_seen_distance skip in nn_search_next(), and
  // some nodes may be unreachable through the layer-0 graph.
  std::printf("\n");
}

TEST(HnswBenchmark, IndexMemoryAndBuildCost) {
  const BenchFixture &f = fixture();

  const size_t nodes = f.num_points();
  const size_t requested = f.arena_requested_bytes();
  const size_t allocated = f.arena_allocated_bytes();
  const size_t raw_vectors = nodes * f.dims() * sizeof(float);
  const double bytes_per_node =
      static_cast<double>(requested) / static_cast<double>(nodes);

  // m_nodes is a std::unordered_map allocated from the global allocator, not
  // from the arena, so it is invisible to allocated_size(). Estimate it:
  // one hash node (next pointer + key + value) plus one bucket slot per entry.
  const size_t map_estimate = nodes * (sizeof(void *) + sizeof(uint64_t) +
                                       sizeof(void *) + sizeof(void *));

  std::printf("  index memory and build cost:\n");
  std::printf("    nodes                 %12zu\n", nodes);
  std::printf("    build time            %12.2f s   (%.1f us/insert)\n",
              f.build_seconds(),
              1e6 * f.build_seconds() / static_cast<double>(nodes));
  std::printf("    node bytes requested  %12zu bytes  (%.2f MB, %zu allocs)\n",
              requested, requested / (1024.0 * 1024.0), f.arena_requests());
  std::printf("    bytes per node        %12.1f\n", bytes_per_node);
  std::printf("    raw vector bytes      %12zu bytes  (%.2f MB)\n", raw_vectors,
              raw_vectors / (1024.0 * 1024.0));
  std::printf(
      "    graph / raw vectors   %12.2fx\n",
      static_cast<double>(requested) / static_cast<double>(raw_vectors));
  std::printf(
      "    arena resident        %12zu bytes  (%.2f MB, %.1f%% slack)\n",
      allocated, allocated / (1024.0 * 1024.0),
      100.0 *
          (static_cast<double>(allocated) - static_cast<double>(requested)) /
          static_cast<double>(allocated));
  std::printf(
      "    m_nodes map (est.)    %12zu bytes  (%.2f MB, outside arena)\n",
      map_estimate, map_estimate / (1024.0 * 1024.0));
  std::printf("\n");

  // A node stores its vector plus at least 2*M neighbor slots on layer 0, so
  // anything at or below the raw vector size means the layout changed under us.
  EXPECT_GT(bytes_per_node, static_cast<double>(f.dims() * sizeof(float)))
      << "a node cannot be smaller than the vector it stores";
  const double slots_per_node =
      static_cast<double>(2 * f.M() + 2) * sizeof(void *);
  EXPECT_LT(bytes_per_node,
            2.0 * (f.dims() * sizeof(float) + slots_per_node) + 512.0)
      << "per-node footprint far above the expected layout";
}

static void BM_HnswKnnSearch(size_t num_iterations) {
#ifndef NDEBUG
  // Timings from a -O0 build with hot-path asserts live are not informative.
  GTEST_SKIP() << "Benchmarks skipped in debug builds "
                  "(configure with -DWITH_DEBUG=OFF for meaningful results)";
#endif
  StopBenchmarkTiming();
  BenchFixture &f = fixture();
  constexpr size_t kEfSearch = 64;

  StartBenchmarkTiming();
  for (size_t i = 0; i < num_iterations; ++i) {
    // Rotate the query: repeating one query would measure a fully warmed path.
    const std::vector<uint64_t> found = f.index().k_nn_search(
        as_bytes(f.query(i % f.num_queries())), f.k(), kEfSearch);
    bench_sink = bench_sink + (found.empty() ? 0 : found[0]);
  }
  StopBenchmarkTiming();
}
BENCHMARK(BM_HnswKnnSearch)

static void BM_HnswStreamSearch(size_t num_iterations) {
#ifndef NDEBUG
  GTEST_SKIP() << "Benchmarks skipped in debug builds "
                  "(configure with -DWITH_DEBUG=OFF for meaningful results)";
#endif
  StopBenchmarkTiming();
  BenchFixture &f = fixture();
  constexpr size_t kEfSearch = 64;

  StartBenchmarkTiming();
  for (size_t i = 0; i < num_iterations; ++i) {
    // Same k as BM_HnswKnnSearch so the two are directly comparable, and
    // bounded because the harness calibrates with 100 unconditional
    // iterations -- a full drain here would cost minutes before it noticed.
    const std::vector<uint64_t> found = drain_stream(
        f.index(), as_bytes(f.query(i % f.num_queries())),
        std::min(kStreamBatchSize, f.k()), kEfSearch, /*max_results=*/f.k());
    bench_sink = bench_sink + (found.empty() ? 0 : found[0]);
  }
  StopBenchmarkTiming();
}
BENCHMARK(BM_HnswStreamSearch)

}  // namespace hnsw_bench
