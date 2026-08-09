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

#ifndef HNSW_TEST_UTILS_INCLUDED
#define HNSW_TEST_UTILS_INCLUDED

/**
  @file

  Helpers shared by the HNSW unit tests (hnsw-t.cc) and the HNSW benchmark
  (hnsw_bench-t.cc). Everything here is inline rather than static: hnsw-t.cc is
  compiled into merge_small_tests-t, and a static helper that one including
  translation unit happens not to use is a -Wunused-function error under
  maintainer mode.
*/

#include <cassert>
#include <cmath>
#include <cstdint>
#include <initializer_list>
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

/**
  Caller-owned arena plus request counters, for measuring an index's memory.

  MEM_ROOT::allocated_size() reports what was taken from the OS, which includes
  the unused tail of the last (exponentially grown) block; bytes_requested is
  what the index actually asked for. Reporting both separates the index's own
  footprint from the allocator's slack.
*/
struct ArenaStats {
  explicit ArenaStats(size_t block_size)
      : mem_root(PSI_NOT_INSTRUMENTED, block_size) {}

  MEM_ROOT mem_root;
  size_t bytes_requested = 0;
  size_t requests_number = 0;
};

/**
  Arena handed to the next HNSW constructed on this thread.

  HNSW holds its allocator by value as a private member and default-constructs
  it, so there is no way to pass an arena in. This slot lets a caller keep
  ownership of the arena anyway, which is what makes the index's memory
  footprint observable from outside. Set it with ArenaHandover, never directly.
*/
inline ArenaStats *g_pending_arena = nullptr;

/**
  Allocator that borrows a caller-owned arena instead of owning one.

  The ArenaStats must outlive the index, since nodes point into its MEM_ROOT.
*/
class BorrowedArenaAllocator {
 public:
  BorrowedArenaAllocator() : m_arena(g_pending_arena) {
    assert(m_arena != nullptr);  // no ArenaHandover in scope
    g_pending_arena = nullptr;
  }

  void *allocate(size_t size) {
    m_arena->bytes_requested += size;
    ++m_arena->requests_number;
    return m_arena->mem_root.Alloc(size);
  }

 private:
  ArenaStats *m_arena;
};

using BorrowedHnsw = HNSW<BorrowedArenaAllocator>;

/** Scoped arm of g_pending_arena; construct the index inside its scope. */
class ArenaHandover {
 public:
  explicit ArenaHandover(ArenaStats *arena) { g_pending_arena = arena; }
  ~ArenaHandover() { g_pending_arena = nullptr; }

  ArenaHandover(const ArenaHandover &) = delete;
  ArenaHandover &operator=(const ArenaHandover &) = delete;
};

inline double euclidean(const char *a_raw, const char *b_raw, uint32_t dims) {
  const float *a = reinterpret_cast<const float *>(a_raw);
  const float *b = reinterpret_cast<const float *>(b_raw);
  double sum = 0.0;
  for (uint32_t i = 0; i < dims; ++i) {
    const double d = static_cast<double>(a[i]) - static_cast<double>(b[i]);
    sum += d * d;
  }
  return std::sqrt(sum);
}

inline std::vector<float> make_vec(std::initializer_list<float> values) {
  return std::vector<float>(values);
}

inline const char *as_bytes(const std::vector<float> &v) {
  return reinterpret_cast<const char *>(v.data());
}

/**
  Deterministic pseudo-random coordinate in [-1, 1).

  Used instead of std::mt19937 / std::uniform_real_distribution so point
  clouds are identical across standard libraries.
*/
inline float pseudo_coord(uint64_t *state) {
  *state = *state * 6364136223846793005ULL + 1442695040888963407ULL;
  return static_cast<float>((*state >> 40) & 0xFFFF) / 32768.0f - 1.0f;
}

inline std::vector<std::vector<float>> make_pseudo_random_points(
    size_t count, size_t dims, uint64_t *state) {
  std::vector<std::vector<float>> points(count);
  for (size_t i = 0; i < count; ++i) {
    points[i].resize(dims);
    for (size_t d = 0; d < dims; ++d) {
      points[i][d] = pseudo_coord(state);
    }
  }
  return points;
}

/**
  Deterministic point cloud drawn from the given cluster centers.

  Uniform points in a high-dimensional cube are close to the worst case for any
  approximate nearest-neighbor index: distances concentrate, so the "nearest"
  neighbors are barely nearer than the rest and recall is low even for a
  correct implementation. Real embeddings are clustered, so benchmark numbers
  taken on clustered data are both more representative and more stable.

  Members sit within @p spread of their center (sum of four draws, so roughly
  bell-shaped). Queries must be drawn from the same centers as the indexed
  points, or they land in empty space and the measurement says more about
  out-of-distribution behavior than about the index.
*/
inline std::vector<std::vector<float>> make_clustered_points(
    const std::vector<std::vector<float>> &centers, size_t count, float spread,
    uint64_t *state) {
  assert(!centers.empty());
  const size_t dims = centers[0].size();

  std::vector<std::vector<float>> points(count);
  for (size_t i = 0; i < count; ++i) {
    const std::vector<float> &center = centers[i % centers.size()];
    points[i].resize(dims);
    for (size_t d = 0; d < dims; ++d) {
      float offset = 0.0f;
      for (int j = 0; j < 4; ++j) {
        offset += pseudo_coord(state);
      }
      points[i][d] = center[d] + spread * offset * 0.25f;
    }
  }
  return points;
}

/**
  Run a streaming search to completion (or @p max_results rows) and return the
  base_pk values in the order the stream yielded them.
*/
template <typename Hnsw>
inline std::vector<uint64_t> drain_stream(const Hnsw &index, const char *query,
                                          size_t batch_size, size_t ef_search,
                                          size_t max_results = 1000) {
  typename Hnsw::NNSearchContext ctx;
  index.nn_search_start(&ctx, query, batch_size, ef_search);
  std::vector<uint64_t> out;
  for (size_t i = 0; i < max_results; ++i) {
    const std::pair<bool, uint64_t> step = index.nn_search_next(&ctx);
    if (!step.first) {
      break;
    }
    out.push_back(step.second);
  }
  return out;
}

}  // namespace hnsw_unittest

#endif  // HNSW_TEST_UTILS_INCLUDED
