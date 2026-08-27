/* Copyright (c) 2026, Percona Inc.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License, version 2.0, as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
   Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include <gtest/gtest.h>

#include <cstdint>
#include <cstring>
#include <set>
#include <vector>

#include "storage/innobase/include/vec0arena.h"
#include "storage/innobase/include/vec0hnsw.h"
#include "unittest/gunit/hnsw_test_utils.h"
#include "vector-common/hnsw.h"
#include "vector-common/vector_distance.h"

namespace innodb_vec0arena_unittest {

TEST(Vec0ArenaTest, ZeroSizeYieldsNullptr) {
  Vec_arena arena;
  EXPECT_EQ(nullptr, arena.allocate(0));
  EXPECT_EQ(0u, arena.bytes_allocated());
}

TEST(Vec0ArenaTest, BlocksAreAligned) {
  Vec_arena arena;
  for (size_t size : {1u, 3u, 7u, 17u, 64u, 129u}) {
    void *p = arena.allocate(size);
    ASSERT_NE(nullptr, p);
    EXPECT_EQ(0u, reinterpret_cast<uintptr_t>(p) % alignof(std::max_align_t))
        << "size " << size;
  }
}

/* The graph writes a node and its trailing vector into one block, so
overlapping blocks would corrupt neighbouring nodes rather than fail
loudly. Fill each block and check nothing bled into another. */
TEST(Vec0ArenaTest, BlocksDoNotOverlap) {
  Vec_arena arena;
  constexpr int kBlocks = 500;
  constexpr size_t kSize = 137;
  std::vector<char *> blocks;

  for (int i = 0; i < kBlocks; i++) {
    char *p = static_cast<char *>(arena.allocate(kSize));
    ASSERT_NE(nullptr, p);
    memset(p, i & 0xff, kSize);
    blocks.push_back(p);
  }
  for (int i = 0; i < kBlocks; i++) {
    for (size_t b = 0; b < kSize; b++) {
      ASSERT_EQ(static_cast<char>(i & 0xff), blocks[i][b])
          << "block " << i << " byte " << b;
    }
  }
  std::set<char *> distinct(blocks.begin(), blocks.end());
  EXPECT_EQ(blocks.size(), distinct.size());
}

/* Many small blocks must not become many allocations: that is the whole
point of chunking. 500 * 137 bytes is well under two 64K chunks. */
TEST(Vec0ArenaTest, SmallBlocksShareChunks) {
  Vec_arena arena;
  for (int i = 0; i < 500; i++) ASSERT_NE(nullptr, arena.allocate(137));
  EXPECT_LE(arena.bytes_allocated(), 2u * 64 * 1024 + 4096);
}

/* A VECTOR(n) large enough to exceed a whole chunk still has to work —
the arena gives such a request a chunk of its own rather than failing or
rounding every chunk up to the worst case. */
TEST(Vec0ArenaTest, OversizedRequestGetsItsOwnChunk) {
  Vec_arena arena;
  const size_t big = 300 * 1024;
  char *p = static_cast<char *>(arena.allocate(big));
  ASSERT_NE(nullptr, p);
  memset(p, 0xab, big);
  EXPECT_GE(arena.bytes_allocated(), big);

  /* and the arena keeps working afterwards */
  char *q = static_cast<char *>(arena.allocate(64));
  ASSERT_NE(nullptr, q);
  EXPECT_EQ(static_cast<char>(0xab), p[big - 1]);
}

/* Destroying the arena frees every chunk; run under ASAN this is the
test that would catch a leaked or double-freed chunk. */
TEST(Vec0ArenaTest, DestructorReleasesEverything) {
  for (int round = 0; round < 3; round++) {
    Vec_arena arena;
    for (int i = 0; i < 100; i++) ASSERT_NE(nullptr, arena.allocate(1024));
    EXPECT_GT(arena.bytes_allocated(), 0u);
  }
}

/* The point of Vec_arena is to satisfy the class's ArenaAllocator
contract. Asserting the contract in the abstract is worth less than
building a real graph on it, so do that: insert enough points to force
several chunks and confirm the graph still answers queries. */
TEST(Vec0ArenaTest, SatisfiesHnswArenaContract) {
  using ArenaHnsw = HNSW<Vec_arena, hnsw_unittest::NullPersistor>;

  constexpr size_t kDims = 8;
  ArenaHnsw hnsw(kDims, &vector_distance_euclidean_squared, /*M=*/8,
                 /*ef_construction=*/32);

  std::vector<std::vector<float>> points;
  for (int i = 1; i <= 400; i++) {
    std::vector<float> v(kDims, static_cast<float>(i));
    points.push_back(v);
    hnsw.insert(static_cast<uint64_t>(i), static_cast<uint64_t>(i),
                hnsw_unittest::as_bytes(points.back()),
                /*persistor_ctx=*/nullptr);
  }

  /* Nearest neighbour of a point that IS in the graph must be itself. */
  std::vector<float> probe(kDims, 42.0f);
  auto results = hnsw.k_nn_search(hnsw_unittest::as_bytes(probe), /*k=*/1,
                                  /*ef_search=*/32,
                                  /*persistor_ctx=*/nullptr);
  ASSERT_FALSE(results.empty());
  /* k_nn_search returns SearchHit{id, base_pk}, not a bare base_pk. This
  insert used the same value for both, so both must read 42. */
  EXPECT_EQ(42u, results[0].base_pk);
  EXPECT_EQ(42u, results[0].id);
}

/* Vec_persistor's four callbacks are member templates whose signatures
the class checks by name resolution at instantiation. Nothing enforces
them until something instantiates HNSW<Vec_arena, Vec_persistor>, and a
mismatch there is a compile error rather than a runtime surprise — so
this instantiation IS the test. It is why the persistor needs no
registration step: the using-declaration is the registration.

Constructing one also proves the arena and the persistor are both
default-constructible and held by value, which the class requires. */
TEST(Vec0ArenaTest, PersistorSatisfiesHnswContract) {
  Vec_hnsw graph(/*dimensions=*/8, &vector_distance_euclidean_squared,
                 /*M=*/8, /*ef_construction=*/32);
  /* Empty graph: a search must be well-formed and find nothing, without
  ever reaching a callback (there is no entry point to load from). */
  std::vector<float> probe(8, 1.0f);
  Vec_ctx ctx;
  auto results = graph.k_nn_search(hnsw_unittest::as_bytes(probe), /*k=*/1,
                                   /*ef_search=*/8, &ctx);
  EXPECT_TRUE(results.empty());
  EXPECT_EQ(DB_SUCCESS, ctx.err);
}

}  // namespace innodb_vec0arena_unittest
