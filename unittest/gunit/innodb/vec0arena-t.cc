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
TEST(Vec0ArenaTest, SmallBlocksShareSlabs) {
  Vec_arena arena;
  for (int i = 0; i < 500; i++) ASSERT_NE(nullptr, arena.allocate(137));
  /* 500 * 137 = 68 500 bytes of payload: two slabs, not 500 heap blocks.
  Without slabs the heap's own 8000-byte blocks would need a dozen. */
  EXPECT_LE(arena.bytes_allocated(), 2u * 64 * 1024 + 8192);
}

/* An oversized request is carved to
size, so it must not become the slab that later small requests are
measured against. If it does, every small allocation after an oversized
one buys a fresh slab, and a graph of maximum-dimension vectors costs
about twice what it should. Sizes here mimic a VECTOR(16383) node and its
neighbour list. */
TEST(Vec0ArenaTest, OversizedSlabDoesNotStrandTheCurrentOne) {
  Vec_arena arena;
  const size_t big = 66 * 1024;
  const size_t small = 256;
  for (int i = 0; i < 8; i++) {
    ASSERT_NE(nullptr, arena.allocate(big));
    ASSERT_NE(nullptr, arena.allocate(small));
  }
  /* Eight oversized slabs, and ONE shared slab for all eight neighbour
  lists. Promoting the oversized slab each time would add eight more. */
  EXPECT_LT(arena.bytes_allocated(), 8u * big + 2u * 64 * 1024);
}

/* A VECTOR(n) large enough to exceed a whole slab still has to work -
the arena gives such a request a slab of its own rather than failing or
rounding every slab up to the worst case. */
TEST(Vec0ArenaTest, OversizedRequestGetsItsOwnSlab) {
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

/* Destroying the arena frees the heap and returns every byte to the
server-wide counter; run under ASAN this is the test that would catch a
leaked or double-freed slab. */
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
    ASSERT_EQ(HNSW_SUCCESS,
              hnsw.insert(static_cast<uint64_t>(i), static_cast<uint64_t>(i),
                          hnsw_unittest::as_bytes(points.back()),
                          /*persistor_ctx=*/nullptr));
  }

  /* Nearest neighbour of a point that IS in the graph must be itself,
  asked the way InnoDB asks it: a streaming scan. k_nn_search would do
  here too, but nothing in the engine calls it - the handler opens a
  scan and pulls hits one at a time - so testing it would be testing an
  API no caller uses. */
  std::vector<float> probe(kDims, 42.0f);
  ArenaHnsw::NNSearchContext scan;
  ASSERT_EQ(HNSW_SUCCESS,
            hnsw.nn_search_start(&scan, hnsw_unittest::as_bytes(probe),
                                 /*batch_size=*/1, /*ef_search=*/32,
                                 /*persistor_ctx=*/nullptr));
  const auto hit = hnsw.nn_search_next(&scan);
  ASSERT_EQ(HNSW_SUCCESS, hit.first);
  /* A hit is SearchHit{id, base_pk}, not a bare base_pk. This insert
  used the same value for both, so both must read 42. */
  EXPECT_EQ(42u, hit.second.base_pk);
  EXPECT_EQ(42u, hit.second.id);
}

/* Vec_persistor's four callbacks are member templates whose signatures
the class checks by name resolution at instantiation. Nothing enforces
them until something instantiates HNSW<Vec_arena, Vec_persistor>, and a
mismatch there is a compile error rather than a runtime surprise - so
this instantiation IS the test. It is why the persistor needs no
registration step: the using-declaration is the registration.

Constructing one also proves the arena and the persistor are both
default-constructible and held by value, which the class requires. */
TEST(Vec0ArenaTest, PersistorSatisfiesHnswContract) {
  Vec_hnsw graph(/*dimensions=*/8, &vector_distance_euclidean_squared,
                 /*M=*/8, /*ef_construction=*/32);
  /* Empty graph: a scan must be well-formed and yield nothing, without
  ever reaching a callback (there is no entry point to load from). */
  std::vector<float> probe(8, 1.0f);
  Vec_ctx ctx;
  Vec_hnsw::NNSearchContext scan;
  EXPECT_EQ(HNSW_SUCCESS,
            graph.nn_search_start(&scan, hnsw_unittest::as_bytes(probe),
                                  /*batch_size=*/1, /*ef_search=*/8, &ctx));
  /* End of scan, not a failure. */
  EXPECT_EQ(HNSW_NOT_FOUND, graph.nn_search_next(&scan).first);
  EXPECT_EQ(DB_SUCCESS, ctx.err);
}

/* What InnoDB does with each result the HNSW class can hand back.

Enumerated rather than reached through SQL because the paths that produce
some of these - an arena that could not grow, a node map insert that threw
- have no way in from a statement, and because getting one of them wrong
is not a subtle bug: DB_OUT_OF_MEMORY is not listed in
row_mysql_handle_errors, so returning it from anything on the INSERT path
is a dead server rather than a failed statement. That mistake has been
made twice in this file's history.

HNSW_ERROR_CB with no reason in the context is deliberately not exercised
here: the mapping asserts on it, because every callback sets ctx->err
before returning that result. */
TEST(Vec0HnswErrorTest, EveryResultMapsToItsOwnError) {
  Vec_ctx ctx;
  ASSERT_EQ(DB_SUCCESS, ctx.err);

  EXPECT_EQ(DB_SUCCESS, vec_hnsw_dberr(HNSW_SUCCESS, &ctx));

  /* The graph names a node its table does not have: the two disagree and
  no retry changes that. Index-scoped, not DB_CORRUPTION, which would
  report the base table as crashed. */
  EXPECT_EQ(DB_INDEX_CORRUPT, vec_hnsw_dberr(HNSW_NOT_FOUND, &ctx));

  /* Both OOM results are transient, and neither may be DB_OUT_OF_MEMORY. */
  EXPECT_EQ(DB_VEC_OUT_OF_MEMORY, vec_hnsw_dberr(HNSW_OOM_GRAPH, &ctx));
  EXPECT_EQ(DB_VEC_OUT_OF_MEMORY, vec_hnsw_dberr(HNSW_OOM_CONTEXT, &ctx));
  EXPECT_NE(DB_OUT_OF_MEMORY, vec_hnsw_dberr(HNSW_OOM_GRAPH, &ctx));
  EXPECT_NE(DB_OUT_OF_MEMORY, vec_hnsw_dberr(HNSW_OOM_CONTEXT, &ctx));

  /* A callback failure carries no information the context does not
  already have, so the reason the callback recorded is what comes out -
  whatever it was. */
  for (const dberr_t reported :
       {DB_LOCK_WAIT_TIMEOUT, DB_INDEX_CORRUPT, DB_OUT_OF_FILE_SPACE,
        DB_INTERRUPTED, DB_VEC_OUT_OF_MEMORY}) {
    Vec_ctx failed;
    failed.err = reported;
    EXPECT_EQ(reported, vec_hnsw_dberr(HNSW_ERROR_CB, &failed));
  }

  /* A context carrying a reason does not change what the other results
  mean: only HNSW_ERROR_CB defers to it. */
  Vec_ctx noisy;
  noisy.err = DB_LOCK_WAIT_TIMEOUT;
  EXPECT_EQ(DB_INDEX_CORRUPT, vec_hnsw_dberr(HNSW_NOT_FOUND, &noisy));
  EXPECT_EQ(DB_VEC_OUT_OF_MEMORY, vec_hnsw_dberr(HNSW_OOM_GRAPH, &noisy));

  /* An index build has no context to report through - its persistor
  cannot fail - and every result it can see maps without one. */
  EXPECT_EQ(DB_SUCCESS, vec_hnsw_dberr(HNSW_SUCCESS, nullptr));
  EXPECT_EQ(DB_VEC_OUT_OF_MEMORY, vec_hnsw_dberr(HNSW_OOM_GRAPH, nullptr));
  EXPECT_EQ(DB_VEC_OUT_OF_MEMORY, vec_hnsw_dberr(HNSW_OOM_CONTEXT, nullptr));
}

/* Nothing this returns may be a code row_mysql_handle_errors leaves to its
ib::fatal arm, because the INSERT path goes through that function. The list
below is what that switch handles as statement-level, for the memory case
that matters here. */
TEST(Vec0HnswErrorTest, NoResultMapsToAFatalError) {
  Vec_ctx ctx;
  for (const HnswResult rc :
       {HNSW_NOT_FOUND, HNSW_OOM_GRAPH, HNSW_OOM_CONTEXT}) {
    const dberr_t err = vec_hnsw_dberr(rc, &ctx);
    EXPECT_NE(DB_OUT_OF_MEMORY, err) << "result " << rc;
    EXPECT_NE(DB_ERROR, err) << "result " << rc;
  }
}

}  // namespace innodb_vec0arena_unittest
