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

  Concurrency tests for HNSW.

  Phase 1: concurrent insert / k-NN / streaming with unique new ids and a
  thread-safe RandomEngine (MutexRandomEngine), in-memory NullPersistor.

  Phase 2: same patterns against a lazy-loaded graph (RecordingPersistor),
  including concurrent load_node single-flight.
*/

#include <gtest/gtest.h>

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>
#include <unordered_set>
#include <utility>
#include <vector>

#include "unittest/gunit/hnsw_test_utils.h"
#include "vector-common/hnsw.h"

namespace hnsw_unittest {
namespace {

/**
  Reusable multi-thread rendezvous: each of @p count participants blocks in
  arrive_and_wait() until all have arrived, then all proceed together.
*/
class ThreadBarrier {
 public:
  explicit ThreadBarrier(size_t count)
      : m_threshold(count), m_count(count), m_generation(0) {}

  void arrive_and_wait() {
    std::unique_lock<std::mutex> lock(m_mutex);
    const size_t gen = m_generation;
    if (--m_count == 0) {
      ++m_generation;
      m_count = m_threshold;
      m_cv.notify_all();
    } else {
      m_cv.wait(lock, [&] { return gen != m_generation; });
    }
  }

 private:
  std::mutex m_mutex;
  std::condition_variable m_cv;
  const size_t m_threshold;
  size_t m_count;
  size_t m_generation;
};

/** Fraction of indexed points whose exact query returns their base_pk in top-k.
 */
template <typename Hnsw>
double self_match_hit_rate(
    Hnsw &index, const std::vector<std::vector<float>> &points,
    const std::vector<uint64_t> &base_pks, size_t k, size_t ef_search,
    typename Hnsw::PersistorContext *persistor_ctx = nullptr) {
  assert(points.size() == base_pks.size());
  size_t hits = 0;
  for (size_t i = 0; i < points.size(); ++i) {
    const auto [rc, result] =
        index.k_nn_search(as_bytes(points[i]), k, ef_search, persistor_ctx);
    assert(rc == Hnsw::HNSW_SUCCESS);
    for (const auto &hit : result) {
      if (hit.base_pk == base_pks[i]) {
        ++hits;
        break;
      }
    }
  }
  return static_cast<double>(hits) / static_cast<double>(points.size());
}

}  // namespace

class HnswConcurrencyTest : public ::testing::Test {
 protected:
  static constexpr size_t kDims = 2;
  static constexpr size_t kM = 4;
  static constexpr size_t kEfConstruction = 16;
  static constexpr size_t kNumThreads = 4;
};

TEST_F(HnswConcurrencyTest, ConcurrentInsertsOnly) {
  constexpr size_t kPerThread = 128;
  ConcurrentTestHnsw index(kDims, euclidean, kM, kEfConstruction);

  std::vector<std::vector<float>> points;
  std::vector<uint64_t> base_pks;
  points.reserve(kNumThreads * kPerThread);
  base_pks.reserve(kNumThreads * kPerThread);

  uint64_t state = 42;
  for (size_t t = 0; t < kNumThreads; ++t) {
    for (size_t i = 0; i < kPerThread; ++i) {
      const size_t idx = t * kPerThread + i;
      points.push_back(
          make_pseudo_random_points(/*count=*/1, kDims, &state)[0]);
      base_pks.push_back(static_cast<uint64_t>(idx));
    }
  }

  ThreadBarrier barrier(kNumThreads);
  std::vector<std::thread> threads;
  threads.reserve(kNumThreads);
  for (size_t t = 0; t < kNumThreads; ++t) {
    threads.emplace_back([&, t] {
      barrier.arrive_and_wait();
      for (size_t i = 0; i < kPerThread; ++i) {
        const size_t idx = t * kPerThread + i;
        const uint64_t id = static_cast<uint64_t>(idx) + 1;
        index.insert(id, base_pks[idx], as_bytes(points[idx]));
      }
    });
  }
  for (std::thread &th : threads) {
    th.join();
  }

#ifndef NDEBUG
  EXPECT_TRUE(index.validate());
#endif
  EXPECT_GE(self_match_hit_rate(index, points, base_pks, /*k=*/5,
                                /*ef_search=*/64),
            0.95);
}

TEST_F(HnswConcurrencyTest, EmptyIndexEntryPointRace) {
  constexpr size_t kWorkers = 8;
  ConcurrentTestHnsw index(kDims, euclidean, kM, kEfConstruction);

  std::vector<std::vector<float>> points(kWorkers);
  uint64_t state = 7;
  for (size_t i = 0; i < kWorkers; ++i) {
    points[i] = make_pseudo_random_points(/*count=*/1, kDims, &state)[0];
  }

  ThreadBarrier barrier(kWorkers);
  std::vector<std::thread> threads;
  threads.reserve(kWorkers);
  for (size_t t = 0; t < kWorkers; ++t) {
    threads.emplace_back([&, t] {
      barrier.arrive_and_wait();
      const uint64_t id = static_cast<uint64_t>(t) + 1;
      index.insert(id, /*base_pk=*/id, as_bytes(points[t]));
    });
  }
  for (std::thread &th : threads) {
    th.join();
  }

#ifndef NDEBUG
  EXPECT_TRUE(index.validate());
#endif

  std::vector<uint64_t> base_pks(kWorkers);
  for (size_t i = 0; i < kWorkers; ++i) {
    base_pks[i] = static_cast<uint64_t>(i) + 1;
  }
  EXPECT_GE(self_match_hit_rate(index, points, base_pks, /*k=*/kWorkers,
                                /*ef_search=*/32),
            1.0);
}

TEST_F(HnswConcurrencyTest, ConcurrentSearchesStableGraph) {
  constexpr size_t kNodes = 200;
  ConcurrentTestHnsw index(kDims, euclidean, kM, kEfConstruction);

  uint64_t state = 99;
  const auto points = make_pseudo_random_points(kNodes, kDims, &state);
  for (size_t i = 0; i < kNodes; ++i) {
    index.insert(i + 1, /*base_pk=*/i, as_bytes(points[i]));
  }

  const auto query = make_pseudo_random_points(/*count=*/1, kDims, &state)[0];
  const auto [rc_baseline, baseline] =
      index.k_nn_search(as_bytes(query), /*k=*/10, /*ef_search=*/50);
  ASSERT_EQ(rc_baseline, ConcurrentTestHnsw::HNSW_SUCCESS);
  ASSERT_FALSE(baseline.empty());

  ThreadBarrier barrier(kNumThreads);
  std::vector<std::vector<ConcurrentTestHnsw::SearchHit>> results(kNumThreads);
  std::vector<std::thread> threads;
  threads.reserve(kNumThreads);
  for (size_t t = 0; t < kNumThreads; ++t) {
    threads.emplace_back([&, t] {
      barrier.arrive_and_wait();
      const auto [rc, hits] =
          index.k_nn_search(as_bytes(query), /*k=*/10, /*ef_search=*/50);
      EXPECT_EQ(rc, ConcurrentTestHnsw::HNSW_SUCCESS);
      results[t] = hits;
    });
  }
  for (std::thread &th : threads) {
    th.join();
  }

  for (size_t t = 0; t < kNumThreads; ++t) {
    EXPECT_EQ(baseline, results[t]) << "thread " << t;
  }
}

TEST_F(HnswConcurrencyTest, ConcurrentInsertAndKnnSearch) {
  constexpr size_t kInsertThreads = 4;
  constexpr size_t kSearchThreads = 4;
  constexpr size_t kPerInsertThread = 100;
  constexpr size_t kSearchesPerThread = 50;

  ConcurrentTestHnsw index(kDims, euclidean, kM, kEfConstruction);

  // Seed so searchers never run on a fully empty index for long.
  const auto seed_vec = make_vec({0.0f, 0.0f});
  index.insert(/*id=*/1, /*base_pk=*/0, as_bytes(seed_vec));

  const size_t total_new = kInsertThreads * kPerInsertThread;
  std::vector<std::vector<float>> points;
  std::vector<uint64_t> base_pks;
  points.reserve(total_new);
  base_pks.reserve(total_new);
  uint64_t state = 123;
  for (size_t i = 0; i < total_new; ++i) {
    points.push_back(make_pseudo_random_points(/*count=*/1, kDims, &state)[0]);
    // base_pk 0 is the seed node; new rows use 1..total_new
    base_pks.push_back(static_cast<uint64_t>(i) + 1);
  }

  const size_t n_threads = kInsertThreads + kSearchThreads;
  ThreadBarrier barrier(n_threads);
  std::atomic<bool> inserts_done{false};
  std::vector<std::thread> threads;
  threads.reserve(n_threads);

  for (size_t t = 0; t < kInsertThreads; ++t) {
    threads.emplace_back([&, t] {
      barrier.arrive_and_wait();
      for (size_t i = 0; i < kPerInsertThread; ++i) {
        const size_t idx = t * kPerInsertThread + i;
        // ids 2..; id 1 is the seed
        const uint64_t id = static_cast<uint64_t>(idx) + 2;
        index.insert(id, base_pks[idx], as_bytes(points[idx]));
      }
    });
  }
  for (size_t t = 0; t < kSearchThreads; ++t) {
    threads.emplace_back([&, t] {
      barrier.arrive_and_wait();
      uint64_t local = static_cast<uint64_t>(t) + 1;
      for (size_t i = 0; i < kSearchesPerThread; ++i) {
        const auto q = make_pseudo_random_points(/*count=*/1, kDims, &local)[0];
        const auto [rc, result] =
            index.k_nn_search(as_bytes(q), /*k=*/5, /*ef_search=*/32);
        EXPECT_EQ(rc, ConcurrentTestHnsw::HNSW_SUCCESS);
        // May be empty only if somehow EP missing; seeded index forbids that.
        EXPECT_FALSE(result.empty());
        if (inserts_done.load(std::memory_order_relaxed)) {
          break;
        }
      }
    });
  }

  for (size_t t = 0; t < kInsertThreads; ++t) {
    threads[t].join();
  }
  inserts_done.store(true, std::memory_order_relaxed);
  for (size_t t = kInsertThreads; t < n_threads; ++t) {
    threads[t].join();
  }

#ifndef NDEBUG
  EXPECT_TRUE(index.validate());
#endif
  EXPECT_GE(self_match_hit_rate(index, points, base_pks, /*k=*/5,
                                /*ef_search=*/64),
            0.95);
}

TEST_F(HnswConcurrencyTest, ConcurrentInsertAndStreaming) {
  constexpr size_t kInsertThreads = 4;
  constexpr size_t kStreamThreads = 4;
  constexpr size_t kPerInsertThread = 80;

  ConcurrentTestHnsw index(kDims, euclidean, kM, kEfConstruction);
  const auto seed_vec = make_vec({1.0f, -1.0f});
  index.insert(/*id=*/1, /*base_pk=*/0, as_bytes(seed_vec));

  const size_t total_new = kInsertThreads * kPerInsertThread;
  std::vector<std::vector<float>> points;
  std::vector<uint64_t> base_pks;
  points.reserve(total_new);
  base_pks.reserve(total_new);
  uint64_t state = 456;
  for (size_t i = 0; i < total_new; ++i) {
    points.push_back(make_pseudo_random_points(/*count=*/1, kDims, &state)[0]);
    base_pks.push_back(static_cast<uint64_t>(i) + 1);
  }

  const size_t n_threads = kInsertThreads + kStreamThreads;
  ThreadBarrier barrier(n_threads);
  std::vector<std::thread> threads;
  threads.reserve(n_threads);

  for (size_t t = 0; t < kInsertThreads; ++t) {
    threads.emplace_back([&, t] {
      barrier.arrive_and_wait();
      for (size_t i = 0; i < kPerInsertThread; ++i) {
        const size_t idx = t * kPerInsertThread + i;
        const uint64_t id = static_cast<uint64_t>(idx) + 2;
        index.insert(id, base_pks[idx], as_bytes(points[idx]));
      }
    });
  }
  for (size_t t = 0; t < kStreamThreads; ++t) {
    threads.emplace_back([&, t] {
      barrier.arrive_and_wait();
      // Distinct context per thread (must not share NNSearchContext).
      ConcurrentTestHnsw::NNSearchContext ctx;
      uint64_t local = static_cast<uint64_t>(t) + 99;
      const auto q = make_pseudo_random_points(/*count=*/1, kDims, &local)[0];
      EXPECT_EQ(index.nn_search_start(&ctx, as_bytes(q), /*batch_size=*/8,
                                      /*ef_search=*/32),
                ConcurrentTestHnsw::HNSW_SUCCESS);
      std::unordered_set<uint64_t> seen;
      for (size_t i = 0; i < 40; ++i) {
        const auto [rc, hit] = index.nn_search_next(&ctx);
        if (rc == ConcurrentTestHnsw::HNSW_NOT_FOUND) {
          break;
        }
        EXPECT_EQ(rc, ConcurrentTestHnsw::HNSW_SUCCESS);
        EXPECT_TRUE(seen.insert(hit.id).second)
            << "duplicate node id in stream thread " << t;
      }
    });
  }

  for (std::thread &th : threads) {
    th.join();
  }

#ifndef NDEBUG
  EXPECT_TRUE(index.validate());
#endif

  const auto q = make_vec({0.0f, 0.0f});
  const auto [drain_rc, drained] =
      drain_stream(index, as_bytes(q), /*batch_size=*/16, /*ef_search=*/64,
                   /*max_results=*/total_new + 1);
  ASSERT_EQ(drain_rc, ConcurrentTestHnsw::HNSW_SUCCESS);
  EXPECT_FALSE(drained.empty());
  EXPECT_GE(self_match_hit_rate(index, points, base_pks, /*k=*/5,
                                /*ef_search=*/64),
            0.95);
}

TEST_F(HnswConcurrencyTest, EntryPointRaiseChurn) {
  // Small M makes higher layers more likely; many concurrent inserts contend
  // on entry-point updates.
  constexpr size_t kMSmall = 2;
  constexpr size_t kWorkers = 8;
  constexpr size_t kPerThread = 64;

  ConcurrentTestHnsw index(kDims, euclidean, kMSmall, kEfConstruction);

  const size_t total = kWorkers * kPerThread;
  std::vector<std::vector<float>> points;
  std::vector<uint64_t> base_pks;
  points.reserve(total);
  base_pks.reserve(total);
  uint64_t state = 777;
  for (size_t i = 0; i < total; ++i) {
    points.push_back(make_pseudo_random_points(/*count=*/1, kDims, &state)[0]);
    base_pks.push_back(static_cast<uint64_t>(i));
  }

  ThreadBarrier barrier(kWorkers);
  std::vector<std::thread> threads;
  threads.reserve(kWorkers);
  for (size_t t = 0; t < kWorkers; ++t) {
    threads.emplace_back([&, t] {
      barrier.arrive_and_wait();
      for (size_t i = 0; i < kPerThread; ++i) {
        const size_t idx = t * kPerThread + i;
        index.insert(static_cast<uint64_t>(idx) + 1, base_pks[idx],
                     as_bytes(points[idx]));
      }
    });
  }
  for (std::thread &th : threads) {
    th.join();
  }

#ifndef NDEBUG
  EXPECT_TRUE(index.validate());
#endif
  EXPECT_GE(self_match_hit_rate(index, points, base_pks, /*k=*/5,
                                /*ef_search=*/64),
            0.90);
}

TEST_F(HnswConcurrencyTest, ConcurrentSearchesColdGraph) {
  constexpr size_t kNumPoints = 400;
  constexpr uint64_t kSeed = 4242;
  RoundTripFixture fixture =
      make_random_round_trip_fixture(kDims, kNumPoints, kSeed);

  LoadTestHnsw built(kDims, euclidean, kM, kEfConstruction);
  populate_round_trip_index(built, &fixture);
  ASSERT_GT(fixture.store.entry_point, 0U);

  const auto [rc_baseline, baseline] = built.k_nn_search(
      as_bytes(fixture.query), /*k=*/10, /*ef_search=*/50, &fixture.store);
  ASSERT_EQ(rc_baseline, LoadTestHnsw::HNSW_SUCCESS);
  ASSERT_FALSE(baseline.empty());

  std::mutex store_mu;
  fixture.store.guard = &store_mu;
  fixture.store.load_counts.clear();

  ConcurrentLoadHnsw cold(kDims, euclidean, kM, kEfConstruction);
  ASSERT_EQ(
      cold.init_from_entry_point(fixture.store.entry_point, &fixture.store),
      ConcurrentLoadHnsw::HNSW_SUCCESS);
  ASSERT_EQ(1U, fixture.store.load_counts.size());
  ASSERT_EQ(1U, fixture.store.load_counts.at(fixture.store.entry_point));

  ThreadBarrier barrier(kNumThreads);
  std::vector<std::vector<ConcurrentLoadHnsw::SearchHit>> results(kNumThreads);
  std::vector<std::thread> threads;
  threads.reserve(kNumThreads);
  for (size_t t = 0; t < kNumThreads; ++t) {
    threads.emplace_back([&, t] {
      barrier.arrive_and_wait();
      const auto [rc, hits] = cold.k_nn_search(
          as_bytes(fixture.query), /*k=*/10, /*ef_search=*/50, &fixture.store);
      EXPECT_EQ(rc, ConcurrentLoadHnsw::HNSW_SUCCESS);
      results[t] = hits;
    });
  }
  for (std::thread &th : threads) {
    th.join();
  }

  for (size_t t = 0; t < kNumThreads; ++t) {
    ASSERT_FALSE(results[t].empty()) << "thread " << t;
    ASSERT_EQ(baseline.size(), results[t].size()) << "thread " << t;
    for (size_t i = 0; i < baseline.size(); ++i) {
      EXPECT_EQ(baseline[i].id, results[t][i].id)
          << "thread " << t << " i=" << i;
      EXPECT_EQ(baseline[i].base_pk, results[t][i].base_pk)
          << "thread " << t << " i=" << i;
    }
  }
  EXPECT_GT(fixture.store.load_counts.size(), 1U);
}

TEST_F(HnswConcurrencyTest, ConcurrentInsertAndSearchWhileLazyLoading) {
  constexpr size_t kPersisted = 300;
  constexpr size_t kInsertThreads = 4;
  constexpr size_t kSearchThreads = 4;
  constexpr size_t kPerInsertThread = 40;
  constexpr uint64_t kSeed = 5151;

  RoundTripFixture fixture =
      make_random_round_trip_fixture(kDims, kPersisted, kSeed);
  LoadTestHnsw built(kDims, euclidean, kM, kEfConstruction);
  populate_round_trip_index(built, &fixture);
  ASSERT_GT(fixture.store.entry_point, 0U);

  // New insert ids must not collide with persisted graph ids (1..kPersisted).
  const size_t total_new = kInsertThreads * kPerInsertThread;
  std::vector<std::vector<float>> new_points;
  std::vector<uint64_t> new_base_pks;
  new_points.reserve(total_new);
  new_base_pks.reserve(total_new);
  uint64_t state = kSeed + 1;
  for (size_t i = 0; i < total_new; ++i) {
    new_points.push_back(
        make_pseudo_random_points(/*count=*/1, kDims, &state)[0]);
    new_base_pks.push_back(100000U + static_cast<uint64_t>(i));
  }

  std::mutex store_mu;
  fixture.store.guard = &store_mu;
  fixture.store.load_counts.clear();

  ConcurrentLoadHnsw cold(kDims, euclidean, kM, kEfConstruction);
  ASSERT_EQ(
      cold.init_from_entry_point(fixture.store.entry_point, &fixture.store),
      ConcurrentLoadHnsw::HNSW_SUCCESS);

  const size_t n_threads = kInsertThreads + kSearchThreads;
  ThreadBarrier barrier(n_threads);
  std::vector<std::thread> threads;
  threads.reserve(n_threads);

  for (size_t t = 0; t < kInsertThreads; ++t) {
    threads.emplace_back([&, t] {
      barrier.arrive_and_wait();
      for (size_t i = 0; i < kPerInsertThread; ++i) {
        const size_t idx = t * kPerInsertThread + i;
        const uint64_t id =
            static_cast<uint64_t>(kPersisted) + 1 + static_cast<uint64_t>(idx);
        cold.insert(id, new_base_pks[idx], as_bytes(new_points[idx]),
                    &fixture.store);
      }
    });
  }
  for (size_t t = 0; t < kSearchThreads; ++t) {
    threads.emplace_back([&, t] {
      barrier.arrive_and_wait();
      uint64_t local = static_cast<uint64_t>(t) + 9;
      for (size_t i = 0; i < 40; ++i) {
        const auto q = make_pseudo_random_points(/*count=*/1, kDims, &local)[0];
        const auto [rc, result] =
            cold.k_nn_search(as_bytes(q), /*k=*/5,
                             /*ef_search=*/32, &fixture.store);
        EXPECT_EQ(rc, ConcurrentLoadHnsw::HNSW_SUCCESS);
        EXPECT_FALSE(result.empty());
      }
      ConcurrentLoadHnsw::NNSearchContext ctx;
      const auto sq = make_pseudo_random_points(/*count=*/1, kDims, &local)[0];
      EXPECT_EQ(cold.nn_search_start(&ctx, as_bytes(sq), /*batch_size=*/8,
                                     /*ef_search=*/32, &fixture.store),
                ConcurrentLoadHnsw::HNSW_SUCCESS);
      std::unordered_set<uint64_t> seen;
      for (size_t i = 0; i < 30; ++i) {
        const auto [rc, hit] = cold.nn_search_next(&ctx);
        if (rc == ConcurrentLoadHnsw::HNSW_NOT_FOUND) {
          break;
        }
        EXPECT_EQ(rc, ConcurrentLoadHnsw::HNSW_SUCCESS);
        EXPECT_TRUE(seen.insert(hit.id).second)
            << "duplicate node id in stream thread " << t;
      }
    });
  }

  for (std::thread &th : threads) {
    th.join();
  }

  EXPECT_GE(self_match_hit_rate(cold, new_points, new_base_pks, /*k=*/5,
                                /*ef_search=*/64, &fixture.store),
            0.90);
  EXPECT_GT(fixture.store.load_counts.size(), 1U);
}

TEST_F(HnswConcurrencyTest, LoadNodeSingleFlight) {
  constexpr size_t kNumPoints = 500;
  constexpr uint64_t kSeed = 6060;
  constexpr size_t kWorkers = 8;

  RoundTripFixture fixture =
      make_random_round_trip_fixture(kDims, kNumPoints, kSeed);
  LoadTestHnsw built(kDims, euclidean, kM, kEfConstruction);
  populate_round_trip_index(built, &fixture);
  ASSERT_GT(fixture.store.entry_point, 0U);

  std::mutex store_mu;
  fixture.store.guard = &store_mu;
  fixture.store.load_counts.clear();

  ConcurrentLoadHnsw cold(kDims, euclidean, kM, kEfConstruction);
  ASSERT_EQ(
      cold.init_from_entry_point(fixture.store.entry_point, &fixture.store),
      ConcurrentLoadHnsw::HNSW_SUCCESS);
  ASSERT_EQ(1U, fixture.store.load_counts.at(fixture.store.entry_point));

  ThreadBarrier barrier(kWorkers);
  std::vector<std::thread> threads;
  threads.reserve(kWorkers);
  for (size_t t = 0; t < kWorkers; ++t) {
    threads.emplace_back([&] {
      barrier.arrive_and_wait();
      (void)cold.k_nn_search(as_bytes(fixture.query), /*k=*/10,
                             /*ef_search=*/50, &fixture.store);
    });
  }
  for (std::thread &th : threads) {
    th.join();
  }

  // After concurrent first-touch, each loaded id should have been filled by
  // exactly one successful load_node_cb (single-flight under load stripes).
  {
    std::lock_guard<std::mutex> lock(store_mu);
    ASSERT_GT(fixture.store.load_counts.size(), 1U);
    for (const auto &kv : fixture.store.load_counts) {
      EXPECT_EQ(1U, kv.second) << "graph id " << kv.first;
    }
  }

  const auto counts_after_first = fixture.store.load_counts;
  (void)cold.k_nn_search(as_bytes(fixture.query), /*k=*/10, /*ef_search=*/50,
                         &fixture.store);
  EXPECT_EQ(counts_after_first, fixture.store.load_counts);
}

/**
  Stress concurrent inserts while a fraction of insert_cb calls fail.

  Race under test: thread A reverse-link prune may observe another insert's
  node as NODE_LINKING and push it into select_neighbors(); meanwhile that
  insert's insert_cb fails and the node becomes NODE_FAILED. select_neighbors()
  must tolerate LINKING -> FAILED (same single-load pattern as LINKING ->
  COMPLETE). Small M fills neighbor lists quickly so the prune path runs often.
*/
TEST_F(HnswConcurrencyTest, ConcurrentInsertCbFailuresSelectNeighbors) {
  constexpr size_t kMSmall = 2;
  constexpr size_t kWorkers = 8;
  constexpr size_t kPerThread = 64;
  constexpr size_t kSeedPoints = 16;
  constexpr int kFailInsertPercent = 40;

  RecordingPersistor::Context store;
  store.dims = kDims;
  std::mutex store_mu;
  store.guard = &store_mu;
  store.fail_insert_cb_percent = kFailInsertPercent;

  ConcurrentLoadHnsw index(kDims, euclidean, kMSmall, kEfConstruction);

  const size_t total = kSeedPoints + kWorkers * kPerThread;
  std::vector<std::vector<float>> points;
  std::vector<uint64_t> base_pks;
  points.reserve(total);
  base_pks.reserve(total);
  uint64_t rng_state = 991;
  for (size_t i = 0; i < total; ++i) {
    points.push_back(
        make_pseudo_random_points(/*count=*/1, kDims, &rng_state)[0]);
    base_pks.push_back(static_cast<uint64_t>(i));
  }

  // Seed without failures so the graph has full neighbor lists before churn.
  store.fail_insert_cb_percent = 0;
  for (size_t i = 0; i < kSeedPoints; ++i) {
    ASSERT_EQ(index.insert(static_cast<uint64_t>(i) + 1, base_pks[i],
                           as_bytes(points[i]), &store),
              ConcurrentLoadHnsw::HNSW_SUCCESS);
  }
  store.fail_insert_cb_percent = kFailInsertPercent;

  ThreadBarrier barrier(kWorkers);
  std::atomic<size_t> success_count{0};
  std::atomic<size_t> fail_count{0};
  std::vector<std::thread> threads;
  threads.reserve(kWorkers);
  for (size_t t = 0; t < kWorkers; ++t) {
    threads.emplace_back([&, t] {
      barrier.arrive_and_wait();
      for (size_t i = 0; i < kPerThread; ++i) {
        const size_t idx = kSeedPoints + t * kPerThread + i;
        const uint64_t id = static_cast<uint64_t>(idx) + 1;
        const HnswResult rc =
            index.insert(id, base_pks[idx], as_bytes(points[idx]), &store);
        if (rc == ConcurrentLoadHnsw::HNSW_SUCCESS) {
          success_count.fetch_add(1, std::memory_order_relaxed);
        } else {
          EXPECT_EQ(rc, ConcurrentLoadHnsw::HNSW_ERROR_CB);
          fail_count.fetch_add(1, std::memory_order_relaxed);
        }
      }
    });
  }
  for (std::thread &th : threads) {
    th.join();
  }

  EXPECT_GT(success_count.load(), 0U);
  EXPECT_GT(fail_count.load(), 0U);

#ifndef NDEBUG
  EXPECT_TRUE(index.validate(/*possibly_failed_cbs=*/true));
#endif

  // Successful inserts remain searchable; FAILED nodes must not appear.
  std::vector<std::vector<float>> ok_points;
  std::vector<uint64_t> ok_base_pks;
  {
    std::lock_guard<std::mutex> lock(store_mu);
    for (size_t i = 0; i < total; ++i) {
      const uint64_t id = static_cast<uint64_t>(i) + 1;
      if (store.nodes.count(id) != 0) {
        ok_points.push_back(points[i]);
        ok_base_pks.push_back(base_pks[i]);
      }
    }
  }
  ASSERT_FALSE(ok_points.empty());
  EXPECT_GE(self_match_hit_rate(index, ok_points, ok_base_pks, /*k=*/5,
                                /*ef_search=*/64, &store),
            0.85);
}

}  // namespace hnsw_unittest
