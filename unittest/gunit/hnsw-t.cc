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
#include <random>
#include <unordered_set>
#include <utility>
#include <vector>

#include "my_alloc.h"
#include "vector-common/hnsw.h"

namespace hnsw_unittest {

/**
  Arena allocator backed by MEM_ROOT for HNSW unit tests.
  All blocks are freed when the allocator (and its MEM_ROOT) is destroyed.
*/
class ArenaAllocator {
 public:
  ArenaAllocator() : m_mem_root(PSI_NOT_INSTRUMENTED, 4096) {}

  void *allocate(size_t size) { return m_mem_root.Alloc(size); }

 private:
  MEM_ROOT m_mem_root;
};

using TestHnsw = HNSW<ArenaAllocator>;

static double euclidean(const char *a_raw, const char *b_raw, uint32_t dims) {
  const float *a = reinterpret_cast<const float *>(a_raw);
  const float *b = reinterpret_cast<const float *>(b_raw);
  double sum = 0.0;
  for (uint32_t i = 0; i < dims; ++i) {
    const double d = static_cast<double>(a[i]) - static_cast<double>(b[i]);
    sum += d * d;
  }
  return std::sqrt(sum);
}

static std::vector<float> make_vec(std::initializer_list<float> values) {
  return std::vector<float>(values);
}

static const char *as_bytes(const std::vector<float> &v) {
  return reinterpret_cast<const char *>(v.data());
}

class HnswTest : public ::testing::Test {
 protected:
  static constexpr size_t kDims = 2;
  static constexpr size_t kM = 4;
  static constexpr size_t kEfConstruction = 16;
};

TEST_F(HnswTest, SearchEmptyIndex) {
  TestHnsw index(kDims, euclidean, kM, kEfConstruction);
  const auto query = make_vec({0.0f, 0.0f});
  const auto result = index.k_nn_search(as_bytes(query), 3, 16);
  EXPECT_TRUE(result.empty());
}

TEST_F(HnswTest, InsertSingleAndSearchExact) {
  TestHnsw index(kDims, euclidean, kM, kEfConstruction);
  const auto v0 = make_vec({1.0f, 2.0f});
  index.insert(42, 1001, as_bytes(v0));

  const auto result = index.k_nn_search(as_bytes(v0), 1, 8);
  ASSERT_EQ(1U, result.size());
  EXPECT_EQ(1001U, result[0]);
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
  const auto result = index.k_nn_search(as_bytes(query), 1, 16);
  ASSERT_EQ(1U, result.size());
  EXPECT_EQ(2003U, result[0]);
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
  const auto result = index.k_nn_search(as_bytes(query), 3, 16);
  ASSERT_EQ(3U, result.size());

  std::unordered_set<uint64_t> base_pks(result.begin(), result.end());
  // Closest three to (0.1, 0) should be base_pks 3010, 3011, 3012.
  EXPECT_EQ(1U, base_pks.count(3010));
  EXPECT_EQ(1U, base_pks.count(3011));
  EXPECT_EQ(1U, base_pks.count(3012));
}

TEST_F(HnswTest, SearchKLargerThanIndexSize) {
  TestHnsw index(kDims, euclidean, kM, kEfConstruction);

  const auto v0 = make_vec({0.0f, 0.0f});
  const auto v1 = make_vec({1.0f, 0.0f});
  index.insert(1, 4001, as_bytes(v0));
  index.insert(2, 4002, as_bytes(v1));

  const auto query = make_vec({0.0f, 0.0f});
  const auto result = index.k_nn_search(as_bytes(query), 10, 16);
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
  const auto result = index.k_nn_search(as_bytes(query), 3, 16);
  ASSERT_EQ(3U, result.size());
  EXPECT_EQ(5020U, result[0]);
  EXPECT_EQ(5022U, result[1]);
  EXPECT_EQ(5021U, result[2]);
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
    const auto result = index.k_nn_search(as_bytes(points[i]), 1, 32);
    ASSERT_EQ(1U, result.size()) << "i=" << i;
    EXPECT_EQ(5000 + i, result[0]) << "i=" << i;
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
  constexpr double kMinAvgRecall = 0.9;

  TestHnsw index(kRecallDims, euclidean, kM, kEfConstruction);

  std::mt19937 rng(42);
  std::uniform_real_distribution<float> coord(-1.0f, 1.0f);

  std::vector<std::vector<float>> points(kNumPoints);
  for (size_t i = 0; i < kNumPoints; ++i) {
    points[i].resize(kRecallDims);
    for (size_t d = 0; d < kRecallDims; ++d) {
      points[i][d] = coord(rng);
    }
    index.insert(i, /*base_pk=*/i, as_bytes(points[i]));
  }

  double recall_sum = 0.0;
  for (size_t q = 0; q < kNumQueries; ++q) {
    std::vector<float> query(kRecallDims);
    for (size_t d = 0; d < kRecallDims; ++d) {
      query[d] = coord(rng);
    }

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

    const auto approx = index.k_nn_search(as_bytes(query), kK, kEfSearch);
    ASSERT_EQ(kK, approx.size()) << "q=" << q;

    size_t hits = 0;
    for (uint64_t pk : approx) {
      hits += exact_ids.count(pk);
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
  const auto result =
      index.k_nn_search(as_bytes(query), /*k=*/1, /*ef_search=*/0);
  ASSERT_EQ(1U, result.size());
  EXPECT_EQ(100U, result[0]);
}

TEST_F(HnswTest, DuplicateBasePkAllowed) {
  // Same base_pk, different graph ids (e.g. vector update).
  TestHnsw index(kDims, euclidean, kM, kEfConstruction);
  index.insert(1, /*base_pk=*/42, as_bytes(make_vec({0.0f, 0.0f})));
  index.insert(2, /*base_pk=*/42, as_bytes(make_vec({10.0f, 0.0f})));

  const auto result =
      index.k_nn_search(as_bytes(make_vec({0.0f, 0.0f})), 2, 16);
  ASSERT_EQ(2U, result.size());
  EXPECT_EQ(42U, result[0]);
  EXPECT_EQ(42U, result[1]);
}

#ifndef NDEBUG
TEST_F(HnswTest, GraphInvariants) {
  TestHnsw index(kDims, euclidean, kM, kEfConstruction);
  EXPECT_TRUE(index.validate());

  for (uint64_t i = 0; i < 50; ++i) {
    const auto v = make_vec({static_cast<float>(i), static_cast<float>(i % 7)});
    index.insert(i, 1000 + i, as_bytes(v));
    EXPECT_TRUE(index.validate()) << "after insert i=" << i;
  }
}

TEST(HnswDeathTest, SearchKZeroAsserts) {
  TestHnsw index(2, euclidean, 4, 16);
  const auto v = make_vec({0.0f, 0.0f});
  index.insert(1, 100, as_bytes(v));
  EXPECT_DEATH_IF_SUPPORTED(index.k_nn_search(as_bytes(v), /*k=*/0, 16), "");
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
