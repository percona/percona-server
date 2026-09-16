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

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <unordered_set>
#include <utility>
#include <vector>

#include "unittest/gunit/hnsw_test_utils.h"
#include "vector-common/hnsw.h"

namespace hnsw_unittest {

class HnswTest : public ::testing::Test {
 protected:
  static constexpr size_t kDims = 2;
  static constexpr size_t kM = 4;
  static constexpr size_t kEfConstruction = 16;
};

TEST_F(HnswTest, SearchEmptyIndex) {
  TestHnsw index(kDims, euclidean, kM, kEfConstruction);
  const auto query = make_vec({0.0f, 0.0f});
  const auto [rc, result] = index.k_nn_search(as_bytes(query), 3, 16);
  EXPECT_EQ(rc, TestHnsw::HNSW_SUCCESS);
  EXPECT_TRUE(result.empty());
}

TEST_F(HnswTest, InsertSingleAndSearchExact) {
  TestHnsw index(kDims, euclidean, kM, kEfConstruction);
  const auto v0 = make_vec({1.0f, 2.0f});
  index.insert(42, 1001, as_bytes(v0));

  const auto [rc, result] = index.k_nn_search(as_bytes(v0), 1, 8);
  ASSERT_EQ(rc, TestHnsw::HNSW_SUCCESS);
  ASSERT_EQ(1U, result.size());
  EXPECT_EQ(42U, result[0].id);
  EXPECT_EQ(1001U, result[0].base_pk);
}

TEST_F(HnswTest, InsertMultipleSearchNearest) {
  TestHnsw index(kDims, euclidean, kM, kEfConstruction);

  const auto v0 = make_vec({0.0f, 0.0f});
  const auto v1 = make_vec({10.0f, 0.0f});
  const auto v2 = make_vec({0.0f, 10.0f});
  const auto v3 = make_vec({10.0f, 10.0f});
  index.insert(100, 2000, as_bytes(v0));
  index.insert(101, 2001, as_bytes(v1));
  index.insert(102, 2002, as_bytes(v2));
  index.insert(103, 2003, as_bytes(v3));

  const auto query = make_vec({9.5f, 9.5f});
  const auto [rc, result] = index.k_nn_search(as_bytes(query), 1, 16);
  ASSERT_EQ(rc, TestHnsw::HNSW_SUCCESS);
  ASSERT_EQ(1U, result.size());
  EXPECT_EQ(103U, result[0].id);
  EXPECT_EQ(2003U, result[0].base_pk);
}

TEST_F(HnswTest, SearchReturnsKResults) {
  TestHnsw index(kDims, euclidean, kM, kEfConstruction);

  const auto v0 = make_vec({0.0f, 0.0f});
  const auto v1 = make_vec({1.0f, 0.0f});
  const auto v2 = make_vec({2.0f, 0.0f});
  const auto v3 = make_vec({3.0f, 0.0f});
  const auto v4 = make_vec({4.0f, 0.0f});
  index.insert(10, 3010, as_bytes(v0));
  index.insert(11, 3011, as_bytes(v1));
  index.insert(12, 3012, as_bytes(v2));
  index.insert(13, 3013, as_bytes(v3));
  index.insert(14, 3014, as_bytes(v4));

  const auto query = make_vec({0.1f, 0.0f});
  const auto [rc, result] = index.k_nn_search(as_bytes(query), 3, 16);
  ASSERT_EQ(rc, TestHnsw::HNSW_SUCCESS);
  ASSERT_EQ(3U, result.size());

  const auto base_pks = base_pks_of(result);
  std::unordered_set<uint64_t> base_pk_set(base_pks.begin(), base_pks.end());
  // Closest three to (0.1, 0) should be base_pks 3010, 3011, 3012.
  EXPECT_EQ(1U, base_pk_set.count(3010));
  EXPECT_EQ(1U, base_pk_set.count(3011));
  EXPECT_EQ(1U, base_pk_set.count(3012));
}

TEST_F(HnswTest, SearchKLargerThanIndexSize) {
  TestHnsw index(kDims, euclidean, kM, kEfConstruction);

  const auto v0 = make_vec({0.0f, 0.0f});
  const auto v1 = make_vec({1.0f, 0.0f});
  index.insert(1, 4001, as_bytes(v0));
  index.insert(2, 4002, as_bytes(v1));

  const auto query = make_vec({0.0f, 0.0f});
  const auto [rc, result] = index.k_nn_search(as_bytes(query), 10, 16);
  EXPECT_EQ(rc, TestHnsw::HNSW_SUCCESS);
  EXPECT_EQ(2U, result.size());
}

TEST_F(HnswTest, SearchResultsOrderedByDistance) {
  TestHnsw index(kDims, euclidean, kM, kEfConstruction);

  const auto v0 = make_vec({0.0f, 0.0f});
  const auto v1 = make_vec({5.0f, 0.0f});
  const auto v2 = make_vec({1.0f, 0.0f});
  index.insert(20, 5020, as_bytes(v0));
  index.insert(21, 5021, as_bytes(v1));
  index.insert(22, 5022, as_bytes(v2));

  const auto query = make_vec({0.0f, 0.0f});
  const auto [rc, result] = index.k_nn_search(as_bytes(query), 3, 16);
  ASSERT_EQ(rc, TestHnsw::HNSW_SUCCESS);
  ASSERT_EQ(3U, result.size());
  EXPECT_EQ(5020U, result[0].base_pk);
  EXPECT_EQ(5022U, result[1].base_pk);
  EXPECT_EQ(5021U, result[2].base_pk);
}

TEST_F(HnswTest, ExactMatchIsFirstResult) {
  TestHnsw index(kDims, euclidean, kM, kEfConstruction);

  std::vector<std::vector<float>> points = {
      make_vec({0.0f, 0.0f}),  make_vec({1.0f, 1.0f}), make_vec({2.0f, -1.0f}),
      make_vec({-3.0f, 4.0f}), make_vec({5.0f, 5.0f}), make_vec({-1.0f, -1.0f}),
      make_vec({8.0f, 0.0f}),  make_vec({0.0f, 8.0f}),
  };
  for (size_t i = 0; i < points.size(); ++i) {
    index.insert(1000 + i, 5000 + i, as_bytes(points[i]));
  }

  for (size_t i = 0; i < points.size(); ++i) {
    const auto [rc, result] = index.k_nn_search(as_bytes(points[i]), 1, 32);
    ASSERT_EQ(rc, TestHnsw::HNSW_SUCCESS) << "i=" << i;
    ASSERT_EQ(1U, result.size()) << "i=" << i;
    EXPECT_EQ(1000 + i, result[0].id) << "i=" << i;
    EXPECT_EQ(5000 + i, result[0].base_pk) << "i=" << i;
  }
}

TEST_F(HnswTest, BruteForceRecall) {
  constexpr size_t kRecallDims = 16;
  constexpr size_t kM = 5;
  constexpr size_t kNumPoints = 2000;
  constexpr size_t kNumQueries = 20;
  constexpr size_t kK = 10;
  constexpr size_t kEfSearch = 50;
  // HNSW is approximate; require strong average recall@k on this seeded set.
  constexpr double kMinAvgRecall = 0.95;

  TestHnsw index(kRecallDims, euclidean, kM, kEfConstruction);

  uint64_t state = 42;
  const auto points =
      make_pseudo_random_points(kNumPoints, kRecallDims, &state);
  for (size_t i = 0; i < kNumPoints; ++i) {
    index.insert(i + 1, /*base_pk=*/i, as_bytes(points[i]));
  }

  const auto queries =
      make_pseudo_random_points(kNumQueries, kRecallDims, &state);

  double recall_sum = 0.0;
  for (size_t q = 0; q < kNumQueries; ++q) {
    const std::vector<float> &query = queries[q];

    std::vector<std::pair<double, uint64_t>> exact;
    exact.reserve(kNumPoints);
    for (size_t i = 0; i < kNumPoints; ++i) {
      exact.emplace_back(euclidean(as_bytes(query), as_bytes(points[i]),
                                   static_cast<uint32_t>(kRecallDims)),
                         i);
    }
    std::partial_sort(exact.begin(), exact.begin() + kK, exact.end());
    std::unordered_set<uint64_t> exact_ids;
    for (size_t i = 0; i < kK; ++i) {
      exact_ids.insert(exact[i].second);
    }

    const auto [rc, approx] = index.k_nn_search(as_bytes(query), kK, kEfSearch);
    ASSERT_EQ(rc, TestHnsw::HNSW_SUCCESS) << "q=" << q;
    ASSERT_EQ(kK, approx.size()) << "q=" << q;

    size_t hits = 0;
    for (const auto &hit : approx) {
      hits += exact_ids.count(hit.base_pk);
    }
    recall_sum += static_cast<double>(hits) / static_cast<double>(kK);
  }

  const double avg_recall = recall_sum / static_cast<double>(kNumQueries);
  EXPECT_GE(avg_recall, kMinAvgRecall) << "avg recall@10=" << avg_recall;
}

TEST_F(HnswTest, SearchEfZeroStillReturnsK) {
  // ef is clamped with max(ef_search, k), so ef=0 must still work.
  TestHnsw index(kDims, euclidean, kM, kEfConstruction);
  index.insert(1, 100, as_bytes(make_vec({0.0f, 0.0f})));
  index.insert(2, 101, as_bytes(make_vec({1.0f, 0.0f})));

  const auto query = make_vec({0.0f, 0.0f});
  const auto [rc, result] =
      index.k_nn_search(as_bytes(query), /*k=*/1, /*ef_search=*/0);
  ASSERT_EQ(rc, TestHnsw::HNSW_SUCCESS);
  ASSERT_EQ(1U, result.size());
  EXPECT_EQ(1U, result[0].id);
  EXPECT_EQ(100U, result[0].base_pk);
}

TEST_F(HnswTest, DuplicateBasePkAllowed) {
  // Same base_pk, different graph ids (e.g. vector update).
  TestHnsw index(kDims, euclidean, kM, kEfConstruction);
  index.insert(1, /*base_pk=*/42, as_bytes(make_vec({0.0f, 0.0f})));
  index.insert(2, /*base_pk=*/42, as_bytes(make_vec({10.0f, 0.0f})));

  const auto [rc, result] =
      index.k_nn_search(as_bytes(make_vec({0.0f, 0.0f})), 2, 16);
  ASSERT_EQ(rc, TestHnsw::HNSW_SUCCESS);
  ASSERT_EQ(2U, result.size());
  EXPECT_EQ(1U, result[0].id);
  EXPECT_EQ(42U, result[0].base_pk);
  EXPECT_EQ(2U, result[1].id);
  EXPECT_EQ(42U, result[1].base_pk);
}

TEST_F(HnswTest, StreamEmptyIndex) {
  TestHnsw index(kDims, euclidean, kM, kEfConstruction);
  TestHnsw::NNSearchContext ctx;
  const auto query = make_vec({0.0f, 0.0f});
  EXPECT_EQ(index.nn_search_start(&ctx, as_bytes(query), /*batch_size=*/8,
                                  /*ef_search=*/16),
            TestHnsw::HNSW_SUCCESS);
  EXPECT_EQ(index.nn_search_next(&ctx).first, TestHnsw::HNSW_NOT_FOUND);
}

TEST_F(HnswTest, StreamMatchesKnnFirstBatch) {
  TestHnsw index(kDims, euclidean, kM, kEfConstruction);
  index.insert(10, 3010, as_bytes(make_vec({0.0f, 0.0f})));
  index.insert(11, 3011, as_bytes(make_vec({1.0f, 0.0f})));
  index.insert(12, 3012, as_bytes(make_vec({2.0f, 0.0f})));
  index.insert(13, 3013, as_bytes(make_vec({3.0f, 0.0f})));
  index.insert(14, 3014, as_bytes(make_vec({4.0f, 0.0f})));

  const auto query = make_vec({0.1f, 0.0f});
  constexpr size_t kEf = 3;
  const auto [rc, knn] = index.k_nn_search(as_bytes(query), kEf, kEf);
  ASSERT_EQ(rc, TestHnsw::HNSW_SUCCESS);
  const auto streamed = drain_stream(index, as_bytes(query), /*batch_size=*/kEf,
                                     /*ef_search=*/kEf);

  ASSERT_GE(streamed.size(), knn.size());
  for (size_t i = 0; i < knn.size(); ++i) {
    EXPECT_EQ(knn[i], streamed[i]) << "i=" << i;
  }
}

TEST_F(HnswTest, StreamMultipleBatchesNoDuplicates) {
  TestHnsw index(kDims, euclidean, kM, kEfConstruction);
  for (uint64_t i = 0; i < 30; ++i) {
    const auto v = make_vec({static_cast<float>(i), 0.0f});
    index.insert(i + 1, 1000 + i, as_bytes(v));
  }
  const auto query = make_vec({0.0f, 0.0f});
  // Small batch/ef forces continuation refills across batches.
  const auto streamed = drain_stream(index, as_bytes(query), /*batch_size=*/3,
                                     /*ef_search=*/8);

  EXPECT_GE(streamed.size(), 3U);
  std::unordered_set<uint64_t> seen;
  for (const auto &hit : streamed) {
    EXPECT_TRUE(seen.insert(hit.id).second);
  }
}

TEST_F(HnswTest, StreamDistancesNonDecreasingAcrossBatches) {
  TestHnsw index(kDims, euclidean, kM, kEfConstruction);

  std::vector<std::vector<float>> points;
  for (uint64_t i = 0; i < 40; ++i) {
    points.push_back(make_vec({static_cast<float>(i), 0.0f}));
    index.insert(i + 1, /*base_pk=*/i, as_bytes(points.back()));
  }

  const auto query = make_vec({0.0f, 0.0f});
  // batch_size < ef_search forces refills while still returning many hits.
  const auto streamed = drain_stream(index, as_bytes(query), /*batch_size=*/3,
                                     /*ef_search=*/10, /*max_results=*/30);

  ASSERT_GT(streamed.size(), 3U);  // more than one batch worth

  double prev_dist = -std::numeric_limits<double>::infinity();
  for (size_t i = 0; i < streamed.size(); ++i) {
    const uint64_t pk = streamed[i].base_pk;
    ASSERT_LT(pk, points.size());
    const double d = euclidean(as_bytes(query), as_bytes(points[pk]),
                               static_cast<uint32_t>(kDims));
    EXPECT_GE(d, prev_dist) << "i=" << i << " pk=" << pk;
    prev_dist = d;
  }
}

TEST_F(HnswTest, StreamDrainsEntireGraph) {
  constexpr size_t kNumPoints = 25;
  TestHnsw index(kDims, euclidean, kM, kEfConstruction);

  for (uint64_t i = 0; i < kNumPoints; ++i) {
    const auto v = make_vec({static_cast<float>(i), 0.0f});
    index.insert(i + 1, /*base_pk=*/1000 + i, as_bytes(v));
  }

  const auto query = make_vec({0.0f, 0.0f});
  // Small batches, wide search; max_results above graph size so we can finish.
  const auto streamed =
      drain_stream(index, as_bytes(query), /*batch_size=*/4,
                   /*ef_search=*/32, /*max_results=*/kNumPoints + 10);

  ASSERT_EQ(kNumPoints, streamed.size());

  std::unordered_set<uint64_t> seen_pks = [&] {
    std::unordered_set<uint64_t> s;
    for (const auto &hit : streamed) {
      s.insert(hit.base_pk);
    }
    return s;
  }();
  EXPECT_EQ(kNumPoints, seen_pks.size());
  for (uint64_t i = 0; i < kNumPoints; ++i) {
    EXPECT_EQ(1U, seen_pks.count(1000 + i)) << "missing base_pk=" << (1000 + i);
  }
}

TEST_F(HnswTest, StreamDrainsEntireGraphWithEfSmallerThanGraph) {
  constexpr size_t kNumPoints = 100;
  TestHnsw index(kDims, euclidean, kM, kEfConstruction);

  for (uint64_t i = 0; i < kNumPoints; ++i) {
    const auto v = make_vec({static_cast<float>(i), 0.0f});
    index.insert(i + 1, /*base_pk=*/1000 + i, as_bytes(v));
  }

  const auto query = make_vec({0.0f, 0.0f});
  const auto streamed =
      drain_stream(index, as_bytes(query), /*batch_size=*/4,
                   /*ef_search=*/8, /*max_results=*/kNumPoints + 10);

  const std::unordered_set<uint64_t> seen = [&] {
    std::unordered_set<uint64_t> s;
    for (const auto &hit : streamed) {
      s.insert(hit.id);
    }
    return s;
  }();
  EXPECT_EQ(streamed.size(), seen.size()) << "stream returned duplicate rows";
  EXPECT_EQ(kNumPoints, seen.size()) << "stream ended after " << seen.size()
                                     << " of " << kNumPoints << " rows";
}

TEST_F(HnswTest, StreamNoDuplicatesInMultiDimensionalGraph) {
  constexpr size_t kStreamDims = 8;
  constexpr size_t kStreamM = 4;
  constexpr size_t kNumPoints = 100;
  constexpr size_t kNumQueries = 4;
  constexpr size_t kBatchSize = 4;
  constexpr size_t kEfSearch = 8;

  TestHnsw index(kStreamDims, euclidean, kStreamM, kEfConstruction);

  uint64_t state = 12345;
  const auto points =
      make_pseudo_random_points(kNumPoints, kStreamDims, &state);
  for (size_t i = 0; i < kNumPoints; ++i) {
    index.insert(i + 1, /*base_pk=*/i, as_bytes(points[i]));
  }

  const auto queries =
      make_pseudo_random_points(kNumQueries, kStreamDims, &state);
  for (size_t q = 0; q < kNumQueries; ++q) {
    const auto streamed =
        drain_stream(index, as_bytes(queries[q]), kBatchSize, kEfSearch,
                     /*max_results=*/kNumPoints * 2);
    const std::unordered_set<uint64_t> seen = [&] {
      std::unordered_set<uint64_t> s;
      for (const auto &hit : streamed) {
        s.insert(hit.id);
      }
      return s;
    }();
    EXPECT_EQ(streamed.size(), seen.size())
        << "q=" << q << ": stream yielded " << (streamed.size() - seen.size())
        << " duplicate rows out of " << streamed.size();
  }
}

TEST_F(HnswTest, StreamYieldsEachNodeAtMostOnce) {
  constexpr size_t kStreamM = 2;
  constexpr size_t kStreamEfConstruction = 2;
  constexpr uint32_t kStreamSeed = 157;
  constexpr size_t kBatchSize = 1;
  constexpr size_t kEfSearch = 4;

  TestHnsw index(kDims, euclidean, kStreamM, kStreamEfConstruction,
                 kStreamSeed);

  for (uint64_t i = 0; i < 5; ++i) {
    index.insert(
        i + 1, i + 1,
        as_bytes(make_vec({100.0f + 0.5f * static_cast<float>(i), 0.0f})));
  }
  for (uint64_t i = 5; i < 10; ++i) {
    index.insert(
        i + 1, i + 1,
        as_bytes(make_vec({100.0f + 0.5f * static_cast<float>(i - 5), 1.0f})));
  }
  for (uint64_t i = 10; i < 15; ++i) {
    index.insert(
        i + 1, i + 1,
        as_bytes(make_vec({100.0f + 0.5f * static_cast<float>(i - 10), 2.0f})));
  }
  for (uint64_t i = 100; i < 105; ++i) {
    index.insert(
        i + 1, i + 1,
        as_bytes(make_vec({0.5f * static_cast<float>(i - 100), 0.0f})));
  }
  for (uint64_t i = 105; i < 110; ++i) {
    index.insert(
        i + 1, i + 1,
        as_bytes(make_vec({0.5f * static_cast<float>(i - 105), 1.0f})));
  }
  for (uint64_t i = 110; i < 115; ++i) {
    index.insert(
        i + 1, i + 1,
        as_bytes(make_vec({0.5f * static_cast<float>(i - 110), 2.0f})));
  }

  const auto query = make_vec({0.0f, 0.0f});
  const auto streamed = drain_stream(index, as_bytes(query), kBatchSize,
                                     kEfSearch, /*max_results=*/30);
  std::unordered_set<uint64_t> seen;
  for (const auto &hit : streamed) {
    EXPECT_TRUE(seen.insert(hit.id).second);
  }
  EXPECT_EQ(seen.size(), streamed.size())
      << "stream must not yield the same node id twice";
}

TEST_F(HnswTest, StreamExhaustThenNextStaysDone) {
  TestHnsw index(kDims, euclidean, kM, kEfConstruction);
  index.insert(1, 100, as_bytes(make_vec({0.0f, 0.0f})));
  TestHnsw::NNSearchContext ctx;
  ASSERT_EQ(index.nn_search_start(&ctx, as_bytes(make_vec({0.0f, 0.0f})),
                                  /*batch_size=*/8, /*ef_search=*/16),
            TestHnsw::HNSW_SUCCESS);
  ASSERT_EQ(index.nn_search_next(&ctx).first, TestHnsw::HNSW_SUCCESS);
  EXPECT_EQ(index.nn_search_next(&ctx).first, TestHnsw::HNSW_NOT_FOUND);
  EXPECT_EQ(index.nn_search_next(&ctx).first, TestHnsw::HNSW_NOT_FOUND);
}

TEST_F(HnswTest, StreamRestartContext) {
  TestHnsw index(kDims, euclidean, kM, kEfConstruction);
  index.insert(1, 100, as_bytes(make_vec({0.0f, 0.0f})));
  index.insert(2, 200, as_bytes(make_vec({5.0f, 0.0f})));

  TestHnsw::NNSearchContext ctx;
  const auto q = make_vec({0.0f, 0.0f});
  ASSERT_EQ(index.nn_search_start(&ctx, as_bytes(q), /*batch_size=*/8,
                                  /*ef_search=*/16),
            TestHnsw::HNSW_SUCCESS);
  ASSERT_EQ(100U, index.nn_search_next(&ctx).second.base_pk);

  // Re-start on same context must work after reset.
  ctx.reset();

  ASSERT_EQ(index.nn_search_start(&ctx, as_bytes(q), /*batch_size=*/8,
                                  /*ef_search=*/16),
            TestHnsw::HNSW_SUCCESS);
  const auto [rc, hit] = index.nn_search_next(&ctx);
  ASSERT_EQ(rc, TestHnsw::HNSW_SUCCESS);
  EXPECT_EQ(1U, hit.id);
  EXPECT_EQ(100U, hit.base_pk);
}

TEST_F(HnswTest, StreamBruteForceRecall) {
  constexpr size_t kRecallDims = 16;
  constexpr size_t kM = 5;
  constexpr size_t kNumPoints = 2000;
  constexpr size_t kNumQueries = 20;
  constexpr size_t kK = 125;
  // Batch size smaller than kK, so we have multiple batches.
  constexpr size_t kBatchSize = 10;
  // Ef_search is big enough to get good recall, but still smaller than kK.
  // So we have more than one candidate set refills.
  constexpr size_t kEfSearch = 50;
  // Multi-batch streaming is more approximate, hence worse recall.
  constexpr double kMinAvgRecall = 0.9;

  TestHnsw index(kRecallDims, euclidean, kM, kEfConstruction);

  uint64_t state = 42;
  const auto points =
      make_pseudo_random_points(kNumPoints, kRecallDims, &state);
  for (size_t i = 0; i < kNumPoints; ++i) {
    index.insert(i + 1, /*base_pk=*/i, as_bytes(points[i]));
  }

  const auto queries =
      make_pseudo_random_points(kNumQueries, kRecallDims, &state);

  double recall_sum = 0.0;
  for (size_t q = 0; q < kNumQueries; ++q) {
    const std::vector<float> &query = queries[q];

    std::vector<std::pair<double, uint64_t>> exact;
    exact.reserve(kNumPoints);
    for (size_t i = 0; i < kNumPoints; ++i) {
      exact.emplace_back(euclidean(as_bytes(query), as_bytes(points[i]),
                                   static_cast<uint32_t>(kRecallDims)),
                         i);
    }
    std::partial_sort(exact.begin(), exact.begin() + kK, exact.end());
    std::unordered_set<uint64_t> exact_ids;
    for (size_t i = 0; i < kK; ++i) {
      exact_ids.insert(exact[i].second);
    }

    const auto streamed =
        drain_stream(index, as_bytes(query), kBatchSize, kEfSearch,
                     /*max_results=*/kK);
    ASSERT_EQ(kK, streamed.size()) << "q=" << q;

    size_t hits = 0;
    for (const auto &hit : streamed) {
      hits += exact_ids.count(hit.base_pk);
    }
    recall_sum += static_cast<double>(hits) / static_cast<double>(kK);
  }

  const double avg_recall = recall_sum / static_cast<double>(kNumQueries);
  EXPECT_GE(avg_recall, kMinAvgRecall)
      << "avg stream recall@125=" << avg_recall;
}

namespace {

void assert_round_trip_knn(RoundTripFixture *fixture, size_t dims, size_t M,
                           size_t ef_construction, size_t k, size_t ef_search,
                           bool validate_built [[maybe_unused]]) {
  LoadTestHnsw built(dims, euclidean, M, ef_construction);
  populate_round_trip_index(built, fixture);
  ASSERT_GT(fixture->store.entry_point, 0U);
#ifndef NDEBUG
  if (validate_built) {
    EXPECT_TRUE(built.validate());
  }
#endif

  LoadTestHnsw reloaded(dims, euclidean, M, ef_construction);
  ASSERT_EQ(reloaded.init_from_entry_point(fixture->store.entry_point,
                                           &fixture->store),
            LoadTestHnsw::HNSW_SUCCESS);

  const auto [rc_built, from_built] = built.k_nn_search(
      as_bytes(fixture->query), k, ef_search, &fixture->store);
  const auto [rc_reloaded, from_reloaded] = reloaded.k_nn_search(
      as_bytes(fixture->query), k, ef_search, &fixture->store);

  ASSERT_EQ(rc_built, LoadTestHnsw::HNSW_SUCCESS);
  ASSERT_EQ(rc_reloaded, LoadTestHnsw::HNSW_SUCCESS);
  ASSERT_EQ(from_built.size(), from_reloaded.size());
  EXPECT_EQ(from_built, from_reloaded);
}

void assert_round_trip_stream(RoundTripFixture *fixture, size_t dims, size_t M,
                              size_t ef_construction, size_t batch_size,
                              size_t ef_search, size_t max_results) {
  LoadTestHnsw built(dims, euclidean, M, ef_construction);
  populate_round_trip_index(built, fixture);
  ASSERT_GT(fixture->store.entry_point, 0U);

  LoadTestHnsw reloaded(dims, euclidean, M, ef_construction);
  ASSERT_EQ(reloaded.init_from_entry_point(fixture->store.entry_point,
                                           &fixture->store),
            LoadTestHnsw::HNSW_SUCCESS);

  const auto stream_built = drain_stream(built, as_bytes(fixture->query),
                                         batch_size, ef_search, max_results);
  const auto stream_reloaded =
      drain_stream(reloaded, as_bytes(fixture->query), batch_size, ef_search,
                   max_results, &fixture->store);

  ASSERT_EQ(stream_built.size(), stream_reloaded.size());
  EXPECT_EQ(stream_built, stream_reloaded);
}

}  // namespace

TEST_F(HnswTest, RoundTripSmallGraph) {
  RoundTripFixture fixture = make_fixed_round_trip_fixture(kDims);
  assert_round_trip_knn(&fixture, kDims, kM, kEfConstruction,
                        /*k=*/3, /*ef_search=*/16, /*validate_built=*/true);
}

TEST_F(HnswTest, RoundTripRandomGraph) {
  constexpr size_t kNumPoints = 1000;
  constexpr uint64_t kSeed = 4242;
  RoundTripFixture fixture =
      make_random_round_trip_fixture(kDims, kNumPoints, kSeed);
  assert_round_trip_knn(&fixture, kDims, kM, kEfConstruction,
                        /*k=*/10, /*ef_search=*/50, /*validate_built=*/false);
}

TEST_F(HnswTest, RoundTripStreamSearch) {
  RoundTripFixture fixture = make_fixed_round_trip_fixture(kDims);
  assert_round_trip_stream(&fixture, kDims, kM, kEfConstruction,
                           /*batch_size=*/2, /*ef_search=*/16,
                           /*max_results=*/3);
}

TEST_F(HnswTest, RoundTripStreamRandomGraph) {
  constexpr size_t kNumPoints = 1000;
  constexpr uint64_t kSeed = 5150;
  RoundTripFixture fixture =
      make_random_round_trip_fixture(kDims, kNumPoints, kSeed);
  assert_round_trip_stream(&fixture, kDims, kM, kEfConstruction,
                           /*batch_size=*/5, /*ef_search=*/50,
                           /*max_results=*/25);
}

TEST_F(HnswTest, InitFromEntryPointLoadsEP) {
  RoundTripFixture fixture = make_fixed_round_trip_fixture(kDims);
  LoadTestHnsw built(kDims, euclidean, kM, kEfConstruction);
  populate_round_trip_index(built, &fixture);
  ASSERT_GT(fixture.store.entry_point, 0U);

  LoadTestHnsw reloaded(kDims, euclidean, kM, kEfConstruction);
  ASSERT_EQ(
      reloaded.init_from_entry_point(fixture.store.entry_point, &fixture.store),
      LoadTestHnsw::HNSW_SUCCESS);

  EXPECT_EQ(1U, fixture.store.load_counts.size());
  EXPECT_EQ(1U, fixture.store.load_counts.at(fixture.store.entry_point));
  for (uint64_t id : fixture.graph_ids) {
    if (id == fixture.store.entry_point) {
      continue;
    }
    EXPECT_EQ(0U, fixture.store.load_counts.count(id));
  }
}

TEST_F(HnswTest, SearchLoadsNeighborsOnDemand) {
  constexpr size_t kNumPoints = 1000;
  constexpr uint64_t kSeed = 7777;
  RoundTripFixture fixture =
      make_random_round_trip_fixture(kDims, kNumPoints, kSeed);
  LoadTestHnsw built(kDims, euclidean, kM, kEfConstruction);
  populate_round_trip_index(built, &fixture);
  ASSERT_GT(fixture.store.entry_point, 0U);

  LoadTestHnsw reloaded(kDims, euclidean, kM, kEfConstruction);
  ASSERT_EQ(
      reloaded.init_from_entry_point(fixture.store.entry_point, &fixture.store),
      LoadTestHnsw::HNSW_SUCCESS);
  ASSERT_EQ(1U, fixture.store.load_counts.size());

  const auto [rc, hits] =
      reloaded.k_nn_search(as_bytes(fixture.query), 10, 50, &fixture.store);
  ASSERT_EQ(rc, LoadTestHnsw::HNSW_SUCCESS);
  ASSERT_GE(hits.size(), 1U);
  EXPECT_GT(fixture.store.load_counts.size(), 1U);
  EXPECT_LT(fixture.store.load_counts.size(), fixture.graph_ids.size());
}

TEST_F(HnswTest, LoadNodeIdempotent) {
  constexpr size_t kNumPoints = 1000;
  constexpr uint64_t kSeed = 8888;
  RoundTripFixture fixture =
      make_random_round_trip_fixture(kDims, kNumPoints, kSeed);
  LoadTestHnsw built(kDims, euclidean, kM, kEfConstruction);
  populate_round_trip_index(built, &fixture);
  ASSERT_GT(fixture.store.entry_point, 0U);

  LoadTestHnsw reloaded(kDims, euclidean, kM, kEfConstruction);
  ASSERT_EQ(
      reloaded.init_from_entry_point(fixture.store.entry_point, &fixture.store),
      LoadTestHnsw::HNSW_SUCCESS);

  const auto query = as_bytes(fixture.query);
  (void)reloaded.k_nn_search(query, 10, 50, &fixture.store);
  const auto counts_after_first = fixture.store.load_counts;

  (void)reloaded.k_nn_search(query, 10, 50, &fixture.store);
  EXPECT_EQ(counts_after_first, fixture.store.load_counts);
}

TEST_F(HnswTest, AdjacentPruneShortListZeroFillsTail) {
  // Reverse-link prune on a full adjacent list can see fewer than Mmax usable
  // candidates when some neighbors are NODE_LOST / fail to lazy-load. The
  // write-back must pack the selected prefix and nullptr-fill the tail.
  constexpr size_t kDimsLocal = 2;
  constexpr size_t kMLocal = 2;  // layer-0 Mmax = 4
  constexpr size_t kEfConstructionLocal = 16;
  constexpr size_t kNumPoints = 80;
  constexpr uint64_t kSeed = 424242;

  RoundTripFixture fixture =
      make_random_round_trip_fixture(kDimsLocal, kNumPoints, kSeed);
  LoadTestHnsw built(kDimsLocal, euclidean, kMLocal, kEfConstructionLocal);
  populate_round_trip_index(built, &fixture);

  const uint64_t hub_id = find_full_layer0_hub(fixture.store, kMLocal);
  ASSERT_NE(0U, hub_id)
      << "expected at least one node with a full layer-0 list";

  const StoredNode &hub_built = fixture.store.nodes.at(hub_id);
  const size_t l0_begin = stored_layer0_begin(hub_built.layer, kMLocal);
  const size_t l0_end = stored_layer0_end(hub_built.layer, kMLocal);
  ASSERT_EQ(l0_end - l0_begin, 2 * kMLocal);

  // Fail lazy-load for two of hub's layer-0 neighbors so prune candidates
  // drop below Mmax (Mmax - 2 + new_node).
  fixture.store.fail_load_ids.insert(hub_built.neighbor_ids[l0_begin]);
  fixture.store.fail_load_ids.insert(hub_built.neighbor_ids[l0_begin + 1]);
  ASSERT_NE(hub_built.neighbor_ids[l0_begin],
            hub_built.neighbor_ids[l0_begin + 1]);

  const std::vector<float> hub_vec = hub_built.vec;

  fixture.store.load_counts.clear();
  LoadTestHnsw cold(kDimsLocal, euclidean, kMLocal, kEfConstructionLocal);
  ASSERT_EQ(
      cold.init_from_entry_point(fixture.store.entry_point, &fixture.store),
      LoadTestHnsw::HNSW_SUCCESS);

  // Touch the graph so hub becomes COMPLETE with DUMMY/LOST neighbor stubs.
  (void)cold.k_nn_search(as_bytes(hub_vec), /*k=*/4, /*ef_search=*/16,
                         &fixture.store);
  ASSERT_GE(fixture.store.load_counts.count(hub_id), 1U);

  const uint64_t new_id = kNumPoints + 10;
  const uint64_t new_pk = 900000;
  cold.insert(new_id, new_pk, as_bytes(hub_vec), &fixture.store);

  const StoredNode &hub_after = fixture.store.nodes.at(hub_id);
  ASSERT_EQ(hub_after.neighbor_ids.size(), hub_built.neighbor_ids.size());

  size_t nonzero = 0;
  bool seen_null = false;
  for (size_t i = l0_begin; i < l0_end; ++i) {
    const uint64_t id = hub_after.neighbor_ids[i];
    if (id == 0) {
      seen_null = true;
      continue;
    }
    EXPECT_FALSE(seen_null) << "nullptr hole in layer-0 list at slot " << i;
    ++nonzero;
  }

  // Skipped at least two lost neighbors → selected < Mmax → trailing zeros.
  EXPECT_LT(nonzero, 2 * kMLocal);
  EXPECT_GT(nonzero, 0U);
  EXPECT_TRUE(seen_null);

  // New insert should still be searchable by exact vector.
  const auto [rc_hits, hits] =
      cold.k_nn_search(as_bytes(hub_vec), /*k=*/5,
                       /*ef_search=*/32, &fixture.store);
  ASSERT_EQ(rc_hits, LoadTestHnsw::HNSW_SUCCESS);
  bool found_new = false;
  for (const auto &hit : hits) {
    if (hit.id == new_id) {
      found_new = true;
      EXPECT_EQ(new_pk, hit.base_pk);
      break;
    }
  }
  EXPECT_TRUE(found_new);

#ifndef NDEBUG
  // Graph may retain DUMMY/LOST stubs from failed lazy loads.
  EXPECT_TRUE(cold.validate());
#endif
}

TEST_F(HnswTest, FirstNodeInsertCbFailure) {
  RecordingPersistor::Context store;
  store.dims = kDims;
  store.fail_next_insert_cb = true;

  LoadTestHnsw index(kDims, euclidean, kM, kEfConstruction);
  const auto v1 = make_vec({1.0f, 2.0f});

  EXPECT_EQ(index.insert(1, 100, as_bytes(v1), &store),
            LoadTestHnsw::HNSW_ERROR_CB);
  EXPECT_TRUE(store.nodes.empty());
  EXPECT_EQ(0U, store.entry_point);
  {
    const auto [rc, hits] =
        index.k_nn_search(as_bytes(v1), /*k=*/1, /*ef_search=*/8, &store);
    EXPECT_EQ(rc, LoadTestHnsw::HNSW_SUCCESS);
    EXPECT_TRUE(hits.empty());
  }

#ifndef NDEBUG
  // Only NODE_LOST remains in-memory; no COMPLETE entry point.
  EXPECT_TRUE(index.validate(/*possibly_failed_cbs=*/true));
  EXPECT_FALSE(index.validate());
#endif

  // A later insert still sees a null EP and becomes the first published node.
  const auto v2 = make_vec({3.0f, 4.0f});
  EXPECT_EQ(index.insert(2, 200, as_bytes(v2), &store),
            LoadTestHnsw::HNSW_SUCCESS);
  EXPECT_EQ(1U, store.nodes.count(2));
  EXPECT_EQ(0U, store.nodes.count(1));
  EXPECT_EQ(2U, store.entry_point);

  const auto [rc, hits] =
      index.k_nn_search(as_bytes(v2), /*k=*/1, /*ef_search=*/8, &store);
  ASSERT_EQ(rc, LoadTestHnsw::HNSW_SUCCESS);
  ASSERT_EQ(1U, hits.size());
  EXPECT_EQ(2U, hits[0].id);
  EXPECT_EQ(200U, hits[0].base_pk);

#ifndef NDEBUG
  EXPECT_TRUE(index.validate());
#endif
}

TEST_F(HnswTest, FirstNodeUpdateEntryPointCbFailure) {
  RecordingPersistor::Context store;
  store.dims = kDims;
  store.fail_next_update_entry_point_cb = true;

  LoadTestHnsw index(kDims, euclidean, kM, kEfConstruction);
  const auto v1 = make_vec({1.0f, 2.0f});

  EXPECT_EQ(index.insert(1, 100, as_bytes(v1), &store),
            LoadTestHnsw::HNSW_ERROR_CB);
  // insert_cb already persisted the row; EP update did not.
  ASSERT_EQ(1U, store.nodes.count(1));
  EXPECT_EQ(100U, store.nodes.at(1).base_pk);
  EXPECT_EQ(0U, store.entry_point);
  {
    const auto [rc, hits] =
        index.k_nn_search(as_bytes(v1), /*k=*/1, /*ef_search=*/8, &store);
    EXPECT_EQ(rc, LoadTestHnsw::HNSW_SUCCESS);
    EXPECT_TRUE(hits.empty());
  }

#ifndef NDEBUG
  EXPECT_TRUE(index.validate(/*possibly_failed_cbs=*/true));
  EXPECT_FALSE(index.validate());
#endif

  // Second insert publishes a new EP. Id 1 stays as an orphaned persisted row
  // and an in-memory NODE_LOST stub; search must not return it.
  const auto v2 = make_vec({3.0f, 4.0f});
  EXPECT_EQ(index.insert(2, 200, as_bytes(v2), &store),
            LoadTestHnsw::HNSW_SUCCESS);
  EXPECT_EQ(1U, store.nodes.count(1));
  EXPECT_EQ(1U, store.nodes.count(2));
  EXPECT_EQ(2U, store.entry_point);

  const auto [rc, hits] =
      index.k_nn_search(as_bytes(v2), /*k=*/2, /*ef_search=*/8, &store);
  ASSERT_EQ(rc, LoadTestHnsw::HNSW_SUCCESS);
  ASSERT_FALSE(hits.empty());
  EXPECT_EQ(2U, hits[0].id);
  EXPECT_EQ(200U, hits[0].base_pk);
  for (const auto &hit : hits) {
    EXPECT_NE(1U, hit.id);
  }

#ifndef NDEBUG
  // LOST id 1 may remain in m_nodes; EP is COMPLETE on the max COMPLETE layer.
  EXPECT_TRUE(index.validate());
#endif
}

TEST_F(HnswTest, MidGraphInsertCbFailure) {
  RecordingPersistor::Context store;
  store.dims = kDims;

  LoadTestHnsw index(kDims, euclidean, kM, kEfConstruction);
  const auto v0 = make_vec({0.0f, 0.0f});
  const auto v1 = make_vec({1.0f, 0.0f});
  const auto v2 = make_vec({0.0f, 1.0f});
  ASSERT_EQ(index.insert(1, 100, as_bytes(v0), &store),
            LoadTestHnsw::HNSW_SUCCESS);
  ASSERT_EQ(index.insert(2, 200, as_bytes(v1), &store),
            LoadTestHnsw::HNSW_SUCCESS);
  ASSERT_EQ(index.insert(3, 300, as_bytes(v2), &store),
            LoadTestHnsw::HNSW_SUCCESS);
  const uint64_t ep_before = store.entry_point;
  ASSERT_NE(0U, ep_before);

  // Fail insert_cb after reverse-linking: node becomes NODE_LOST, not
  // persisted.
  store.fail_next_insert_cb = true;
  const auto v_fail = make_vec({0.5f, 0.5f});
  constexpr uint64_t kFailId = 4;
  EXPECT_EQ(index.insert(kFailId, 400, as_bytes(v_fail), &store),
            LoadTestHnsw::HNSW_ERROR_CB);
  EXPECT_EQ(0U, store.nodes.count(kFailId));
  EXPECT_EQ(ep_before, store.entry_point);

  const auto [rc, hits] =
      index.k_nn_search(as_bytes(v_fail), /*k=*/3, /*ef_search=*/16, &store);
  ASSERT_EQ(rc, LoadTestHnsw::HNSW_SUCCESS);
  ASSERT_FALSE(hits.empty());
  for (const auto &hit : hits) {
    EXPECT_NE(kFailId, hit.id);
  }

#ifndef NDEBUG
  // Healthy EP/max-layer invariants still hold; LOST may sit on reverse edges.
  EXPECT_TRUE(index.validate(/*possibly_failed_cbs=*/true));
  EXPECT_TRUE(index.validate());
#endif
}

TEST_F(HnswTest, MidGraphUpdateNeighborsCbFailure) {
  RecordingPersistor::Context store;
  store.dims = kDims;

  LoadTestHnsw index(kDims, euclidean, kM, kEfConstruction);
  const auto v0 = make_vec({0.0f, 0.0f});
  const auto v1 = make_vec({1.0f, 0.0f});
  const auto v2 = make_vec({0.0f, 1.0f});
  ASSERT_EQ(index.insert(1, 100, as_bytes(v0), &store),
            LoadTestHnsw::HNSW_SUCCESS);
  ASSERT_EQ(index.insert(2, 200, as_bytes(v1), &store),
            LoadTestHnsw::HNSW_SUCCESS);
  ASSERT_EQ(index.insert(3, 300, as_bytes(v2), &store),
            LoadTestHnsw::HNSW_SUCCESS);
  const uint64_t ep_before = store.entry_point;
  const auto neighbors_before_1 = store.nodes.at(1).neighbor_ids;
  const auto neighbors_before_2 = store.nodes.at(2).neighbor_ids;
  const auto neighbors_before_3 = store.nodes.at(3).neighbor_ids;

  // insert_cb succeeds (node COMPLETE); first update_neighbors_cb fails.
  store.fail_next_update_neighbors_cb = true;
  const auto v_new = make_vec({0.5f, 0.5f});
  constexpr uint64_t kNewId = 4;
  EXPECT_EQ(index.insert(kNewId, 400, as_bytes(v_new), &store),
            LoadTestHnsw::HNSW_ERROR_CB);
  ASSERT_EQ(1U, store.nodes.count(kNewId));
  EXPECT_EQ(400U, store.nodes.at(kNewId).base_pk);
  EXPECT_EQ(ep_before, store.entry_point);
  // First failing update_neighbors_cb does not write; earlier neighbors
  // unchanged.
  EXPECT_EQ(neighbors_before_1, store.nodes.at(1).neighbor_ids);
  EXPECT_EQ(neighbors_before_2, store.nodes.at(2).neighbor_ids);
  EXPECT_EQ(neighbors_before_3, store.nodes.at(3).neighbor_ids);

  // In-memory node is COMPLETE and searchable despite the persist error.
  const auto [rc, hits] =
      index.k_nn_search(as_bytes(v_new), /*k=*/3, /*ef_search=*/16, &store);
  ASSERT_EQ(rc, LoadTestHnsw::HNSW_SUCCESS);
  bool found_new = false;
  for (const auto &hit : hits) {
    if (hit.id == kNewId) {
      found_new = true;
      EXPECT_EQ(400U, hit.base_pk);
      break;
    }
  }
  EXPECT_TRUE(found_new);

#ifndef NDEBUG
  EXPECT_TRUE(index.validate());
#endif
}

TEST_F(HnswTest, EntryPointRaiseUpdateEntryPointCbFailure) {
  RecordingPersistor::Context store;
  store.dims = kDims;

  LoadTestHnsw index(kDims, euclidean, kM, kEfConstruction);
  const auto v0 = make_vec({0.0f, 0.0f});
  ASSERT_EQ(index.insert(1, 100, as_bytes(v0), &store),
            LoadTestHnsw::HNSW_SUCCESS);
  const uint64_t ep_before = store.entry_point;
  ASSERT_EQ(1U, ep_before);

  // Arm after the first node: only EP-raising inserts call
  // update_entry_point_cb.
  store.fail_next_update_entry_point_cb = true;

  uint64_t failed_id = 0;
  std::vector<float> failed_vec;
  for (uint64_t id = 2; id <= 64; ++id) {
    auto v = make_vec({static_cast<float>(id), 0.0f});
    const auto rc = index.insert(id, 1000 + id, as_bytes(v), &store);
    if (rc == LoadTestHnsw::HNSW_ERROR_CB) {
      failed_id = id;
      failed_vec = std::move(v);
      break;
    }
    ASSERT_EQ(LoadTestHnsw::HNSW_SUCCESS, rc) << "id=" << id;
  }
  ASSERT_NE(0U, failed_id) << "expected an EP-raising insert within 63 tries";

  // Node was persisted and linked; in-memory EP was not published.
  ASSERT_EQ(1U, store.nodes.count(failed_id));
  EXPECT_EQ(ep_before, store.entry_point);
  EXPECT_NE(failed_id, store.entry_point);

  const auto [rc, hits] = index.k_nn_search(as_bytes(failed_vec), /*k=*/3,
                                            /*ef_search=*/16, &store);
  ASSERT_EQ(rc, LoadTestHnsw::HNSW_SUCCESS);
  bool found = false;
  for (const auto &hit : hits) {
    if (hit.id == failed_id) {
      found = true;
      EXPECT_EQ(1000 + failed_id, hit.base_pk);
      break;
    }
  }
  EXPECT_TRUE(found);

#ifndef NDEBUG
  // COMPLETE node above published EP is allowed only with possibly_failed_cbs.
  EXPECT_TRUE(index.validate(/*possibly_failed_cbs=*/true));
  EXPECT_FALSE(index.validate());
#endif
}

TEST_F(HnswTest, InitFromEntryPointLoadErrorCb) {
  RoundTripFixture fixture = make_fixed_round_trip_fixture(kDims);
  LoadTestHnsw built(kDims, euclidean, kM, kEfConstruction);
  populate_round_trip_index(built, &fixture);
  ASSERT_GT(fixture.store.entry_point, 0U);

  fixture.store.fail_load_error_ids.insert(fixture.store.entry_point);
  LoadTestHnsw cold(kDims, euclidean, kM, kEfConstruction);
  EXPECT_EQ(
      cold.init_from_entry_point(fixture.store.entry_point, &fixture.store),
      LoadTestHnsw::HNSW_ERROR_CB);
  EXPECT_EQ(1U, fixture.store.load_counts.at(fixture.store.entry_point));
}

TEST_F(HnswTest, SearchLoadErrorCbPropagatesAndRetries) {
  RoundTripFixture fixture = make_fixed_round_trip_fixture(kDims);
  LoadTestHnsw built(kDims, euclidean, kM, kEfConstruction);
  populate_round_trip_index(built, &fixture);
  ASSERT_GT(fixture.store.entry_point, 0U);

  const StoredNode &ep_row = fixture.store.nodes.at(fixture.store.entry_point);
  uint64_t bad_id = 0;
  for (uint64_t id : ep_row.neighbor_ids) {
    if (id != 0 && id != fixture.store.entry_point) {
      bad_id = id;
      break;
    }
  }
  ASSERT_NE(0U, bad_id) << "entry point must have at least one neighbor";

  fixture.store.load_counts.clear();
  LoadTestHnsw cold(kDims, euclidean, kM, kEfConstruction);
  ASSERT_EQ(
      cold.init_from_entry_point(fixture.store.entry_point, &fixture.store),
      LoadTestHnsw::HNSW_SUCCESS);

  fixture.store.fail_load_error_ids.insert(bad_id);
  const auto [rc_fail, hits_fail] = cold.k_nn_search(
      as_bytes(fixture.query), /*k=*/3, /*ef_search=*/16, &fixture.store);
  EXPECT_EQ(rc_fail, LoadTestHnsw::HNSW_ERROR_CB);
  EXPECT_TRUE(hits_fail.empty());
  ASSERT_GE(fixture.store.load_counts.count(bad_id), 1U);
  const size_t loads_after_fail = fixture.store.load_counts.at(bad_id);

  // ERROR_CB leaves the stub NODE_DUMMY: clearing the injection lets retry
  // complete the load and search successfully.
  fixture.store.fail_load_error_ids.clear();
  const auto [rc_ok, hits_ok] = cold.k_nn_search(
      as_bytes(fixture.query), /*k=*/3, /*ef_search=*/16, &fixture.store);
  EXPECT_EQ(rc_ok, LoadTestHnsw::HNSW_SUCCESS);
  EXPECT_FALSE(hits_ok.empty());
  EXPECT_GT(fixture.store.load_counts.at(bad_id), loads_after_fail);

#ifndef NDEBUG
  EXPECT_TRUE(cold.validate());
#endif
}

TEST_F(HnswTest, InsertLoadErrorCbPropagates) {
  RoundTripFixture fixture = make_fixed_round_trip_fixture(kDims);
  LoadTestHnsw built(kDims, euclidean, kM, kEfConstruction);
  populate_round_trip_index(built, &fixture);
  ASSERT_GT(fixture.store.entry_point, 0U);

  const StoredNode &ep_row = fixture.store.nodes.at(fixture.store.entry_point);
  uint64_t bad_id = 0;
  for (uint64_t id : ep_row.neighbor_ids) {
    if (id != 0 && id != fixture.store.entry_point) {
      bad_id = id;
      break;
    }
  }
  ASSERT_NE(0U, bad_id);

  LoadTestHnsw cold(kDims, euclidean, kM, kEfConstruction);
  ASSERT_EQ(
      cold.init_from_entry_point(fixture.store.entry_point, &fixture.store),
      LoadTestHnsw::HNSW_SUCCESS);

  fixture.store.fail_load_error_ids.insert(bad_id);
  constexpr uint64_t kFailId = 999;
  EXPECT_EQ(cold.insert(kFailId, 9999, as_bytes(fixture.query), &fixture.store),
            LoadTestHnsw::HNSW_ERROR_CB);
  EXPECT_EQ(0U, fixture.store.nodes.count(kFailId));

  // Id was consumed in-memory (LOST); a later insert with a fresh id succeeds
  // once the load injection is cleared (neighbor stub stayed DUMMY).
  fixture.store.fail_load_error_ids.clear();
  constexpr uint64_t kOkId = 1000;
  EXPECT_EQ(cold.insert(kOkId, 10000, as_bytes(fixture.query), &fixture.store),
            LoadTestHnsw::HNSW_SUCCESS);
  EXPECT_EQ(1U, fixture.store.nodes.count(kOkId));
  EXPECT_EQ(0U, fixture.store.nodes.count(kFailId));

#ifndef NDEBUG
  EXPECT_TRUE(cold.validate());
#endif
}

TEST_F(HnswTest, StreamLoadErrorCbPropagates) {
  RoundTripFixture fixture = make_fixed_round_trip_fixture(kDims);
  LoadTestHnsw built(kDims, euclidean, kM, kEfConstruction);
  populate_round_trip_index(built, &fixture);
  ASSERT_GT(fixture.store.entry_point, 0U);

  const StoredNode &ep_row = fixture.store.nodes.at(fixture.store.entry_point);
  uint64_t bad_id = 0;
  for (uint64_t id : ep_row.neighbor_ids) {
    if (id != 0 && id != fixture.store.entry_point) {
      bad_id = id;
      break;
    }
  }
  ASSERT_NE(0U, bad_id);

  LoadTestHnsw cold(kDims, euclidean, kM, kEfConstruction);
  ASSERT_EQ(
      cold.init_from_entry_point(fixture.store.entry_point, &fixture.store),
      LoadTestHnsw::HNSW_SUCCESS);

  fixture.store.fail_load_error_ids.insert(bad_id);
  LoadTestHnsw::NNSearchContext ctx;
  EXPECT_EQ(
      cold.nn_search_start(&ctx, as_bytes(fixture.query), /*batch_size=*/2,
                           /*ef_search=*/8, &fixture.store),
      LoadTestHnsw::HNSW_ERROR_CB);
}

TEST_F(HnswTest, StreamNextLoadErrorCbPropagates) {
  // Build a 1-D chain so layer-0 links stay local and start(ef=1) leaves
  // further-along neighbors as NODE_DUMMY stubs for a later refill.
  constexpr size_t kNumPoints = 40;
  RecordingPersistor::Context store;
  store.dims = kDims;
  LoadTestHnsw built(kDims, euclidean, kM, kEfConstruction);
  for (size_t i = 0; i < kNumPoints; ++i) {
    const auto v = make_vec({static_cast<float>(i) * 10.0f, 0.0f});
    ASSERT_EQ(built.insert(i + 1, /*base_pk=*/i, as_bytes(v), &store),
              LoadTestHnsw::HNSW_SUCCESS);
  }
  ASSERT_GT(store.entry_point, 0U);

  LoadTestHnsw cold(kDims, euclidean, kM, kEfConstruction);
  ASSERT_EQ(cold.init_from_entry_point(store.entry_point, &store),
            LoadTestHnsw::HNSW_SUCCESS);

  const auto query = make_vec({0.0f, 0.0f});
  LoadTestHnsw::NNSearchContext ctx;
  ASSERT_EQ(cold.nn_search_start(&ctx, as_bytes(query), /*batch_size=*/1,
                                 /*ef_search=*/1, &store),
            LoadTestHnsw::HNSW_SUCCESS);

  // Persistently fail lazy-load for stubs adjacent to the already-loaded set
  // but not yet load-attempted (unexpanded discarded frontier).
  for (const auto &kv : store.load_counts) {
    const StoredNode &row = store.nodes.at(kv.first);
    for (uint64_t nid : row.neighbor_ids) {
      if (nid != 0 && store.load_counts.count(nid) == 0) {
        store.fail_load_error_ids.insert(nid);
      }
    }
  }
  ASSERT_FALSE(store.fail_load_error_ids.empty())
      << "start left no unloaded neighbor stubs for nn_search_next to hit";

  bool saw_error = false;
  for (size_t i = 0; i < kNumPoints + 10; ++i) {
    const auto [rc, hit] = cold.nn_search_next(&ctx);
    if (rc == LoadTestHnsw::HNSW_ERROR_CB) {
      saw_error = true;
      EXPECT_EQ(0U, hit.id);
      EXPECT_EQ(0U, hit.base_pk);
      break;
    }
    if (rc == LoadTestHnsw::HNSW_NOT_FOUND) {
      break;
    }
    ASSERT_EQ(rc, LoadTestHnsw::HNSW_SUCCESS) << "i=" << i;
  }
  EXPECT_TRUE(saw_error)
      << "stream ended without nn_search_next observing HNSW_ERROR_CB";
}

#ifndef NDEBUG
TEST_F(HnswTest, GraphInvariants) {
  TestHnsw index(kDims, euclidean, kM, kEfConstruction);
  EXPECT_TRUE(index.validate());

  for (uint64_t i = 0; i < 50; ++i) {
    const auto v = make_vec({static_cast<float>(i), static_cast<float>(i % 7)});
    index.insert(i + 1, 1000 + i, as_bytes(v));
    EXPECT_TRUE(index.validate()) << "after insert i=" << i;
  }
}

TEST(HnswDeathTest, SearchKZeroAsserts) {
  TestHnsw index(2, euclidean, 4, 16);
  const auto v = make_vec({0.0f, 0.0f});
  index.insert(1, 100, as_bytes(v));
  EXPECT_DEATH_IF_SUPPORTED(index.k_nn_search(as_bytes(v), /*k=*/0, 16), "");
}

TEST(HnswDeathTest, ZeroIdAsserts) {
  TestHnsw index(2, euclidean, 4, 16);
  const auto v = make_vec({1.0f, 2.0f});
  EXPECT_DEATH_IF_SUPPORTED(index.insert(0, 100, as_bytes(v)), "");
}

TEST(HnswDeathTest, DuplicateIdAsserts) {
  TestHnsw index(2, euclidean, 4, 16);
  const auto v = make_vec({1.0f, 2.0f});
  index.insert(7, 100, as_bytes(v));
  EXPECT_DEATH_IF_SUPPORTED(index.insert(7, 101, as_bytes(v)), "");
}

TEST(HnswDeathTest, ZeroDimensionsAsserts) {
  EXPECT_DEATH_IF_SUPPORTED(TestHnsw(0, euclidean, 4, 16), "");
}

TEST(HnswDeathTest, MTooSmallAsserts) {
  EXPECT_DEATH_IF_SUPPORTED(TestHnsw(2, euclidean, 1, 16), "");
}
#endif  // NDEBUG

}  // namespace hnsw_unittest
