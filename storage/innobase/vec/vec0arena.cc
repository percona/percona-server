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
@file vec/vec0arena.cc
Arena allocator for HNSW graph nodes.
*/

#include "univ.i"

#include "vec0arena.h"

#include <atomic>

#include <new>

#include "mem0mem.h"
#include "ut0new.h"

namespace {
/** Round up to an alignment that suits any scalar the graph stores. The
heap only promises UNIV_MEM_ALIGNMENT (8), so slabs are aligned here. */
constexpr size_t vec_arena_align(size_t n) {
  constexpr size_t A = alignof(std::max_align_t);
  return (n + A - 1) & ~(A - 1);
}
}  // namespace

/** Bytes held by every Vec_arena in the server, heap block headers
included.

The budget innodb_hnsw_max_memory promises is server-wide - across all
tables and all indexes - and every graph byte passes through
Vec_arena::allocate(), so one counter here covers exactly that scope. */
static std::atomic<uint64_t> vec_arena_bytes{0};

uint64_t vec_arena_global_bytes() {
  return vec_arena_bytes.load(std::memory_order_relaxed);
}

Vec_arena::~Vec_arena() {
  if (m_heap == nullptr) return;

  vec_arena_bytes.fetch_sub(m_bytes_allocated, std::memory_order_relaxed);
  m_bytes_allocated = 0;
  m_cur = nullptr;
  m_cur_size = 0;
  m_cur_used = 0;

  /* One call releases every slab: the heap owns them, which is the point
  of taking the memory from a heap rather than keeping a chunk list. */
  mem_heap_free(m_heap);
  m_heap = nullptr;
}

void Vec_arena::recount() {
  const size_t now = mem_heap_get_size(m_heap);
  if (now == m_bytes_allocated) return;

  /* The heap only grows while an arena is alive - nothing here frees a
  block - so the delta is always positive. */
  vec_arena_bytes.fetch_add(now - m_bytes_allocated, std::memory_order_relaxed);
  m_bytes_allocated = now;
}

void *Vec_arena::allocate(size_t size) {
  if (size == 0) return nullptr;

  const size_t want = vec_arena_align(size);

  if (m_cur != nullptr && m_cur_size - m_cur_used >= want) {
    void *p = m_cur + m_cur_used;
    m_cur_used += want;
    return p;
  }

  if (m_heap == nullptr) {
    /* Deliberately not sized to SLAB_SIZE: the first block would then be
    a slab we never sub-allocate from, since every slab below comes from
    its own mem_heap_alloc. Let the heap pick its default. */
    m_heap = mem_heap_create(0, UT_LOCATION_HERE);
    if (m_heap == nullptr) return nullptr;
    recount();
  }

  /* A slab larger than the heap's own block cap, so mem_heap_add_block
  hands us a block of exactly this size rather than its 8000-byte
  standard one. Over-allocate by the alignment we promise, because the
  heap only guarantees UNIV_MEM_ALIGNMENT. */
  const size_t slab = (want > SLAB_SIZE ? want : SLAB_SIZE);
  constexpr size_t A = alignof(std::max_align_t);
  char *raw = static_cast<char *>(mem_heap_alloc(m_heap, slab + A - 1));
  if (raw == nullptr) return nullptr;
  recount();

  char *base = reinterpret_cast<char *>(
      vec_arena_align(reinterpret_cast<uintptr_t>(raw)));

  /* Keep as the current slab whichever has more room left. An oversized
  request consumes its slab exactly, so it never displaces a slab that
  can still serve the small allocations that follow it - which is what
  otherwise costs one extra slab per node. */
  const size_t new_free = slab - want;
  /* An oversized request consumes its slab exactly, so new_free is 0 and
  it can never displace a slab with room left - which is the property the
  most-room-wins rule exists to protect. */
  ut_ad(want <= SLAB_SIZE || new_free == 0);

  if (m_cur == nullptr || new_free > m_cur_size - m_cur_used) {
    m_cur = base;
    m_cur_size = slab;
    m_cur_used = want;
  }

  return base;
}
