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

#include "ut0new.h"

namespace {
/** Round up to an alignment that suits any scalar the graph stores. */
constexpr size_t vec_arena_align(size_t n) {
  constexpr size_t A = alignof(std::max_align_t);
  return (n + A - 1) & ~(A - 1);
}
}  // namespace

/** Bytes held by every Vec_arena in the server, chunk headers included.

The budget innodb_hnsw_max_memory promises is server-wide — across all
tables and all indexes — and every graph byte passes through
Vec_arena::allocate(), so one counter here covers exactly that scope. */
static std::atomic<uint64_t> vec_arena_bytes{0};

uint64_t vec_arena_global_bytes() {
  return vec_arena_bytes.load(std::memory_order_relaxed);
}

Vec_arena::~Vec_arena() {
  Chunk *chunk = m_head;
  while (chunk != nullptr) {
    Chunk *next = chunk->m_next;
    ut::free(chunk);
    chunk = next;
  }
  m_head = nullptr;
  vec_arena_bytes.fetch_sub(m_bytes_allocated, std::memory_order_relaxed);
  m_bytes_allocated = 0;
}

void *Vec_arena::allocate(size_t size) {
  if (size == 0) return nullptr;

  const size_t want = vec_arena_align(size);
  const size_t header = vec_arena_align(sizeof(Chunk));

  if (m_head != nullptr && m_head->m_size - m_head->m_used >= want) {
    void *p = reinterpret_cast<char *>(m_head) + header + m_head->m_used;
    m_head->m_used += want;
    return p;
  }

  /* A request larger than a fresh chunk gets a chunk sized to it. The
  current chunk keeps whatever it has left rather than being abandoned:
  it stays at the head only if it still has more room than the new one
  would, which for an oversized request it will not. */
  const size_t usable = want > CHUNK_SIZE ? want : CHUNK_SIZE;

  void *raw = ut::malloc_withkey(UT_NEW_THIS_FILE_PSI_KEY, header + usable);
  if (raw == nullptr) return nullptr;

  Chunk *chunk = static_cast<Chunk *>(raw);
  chunk->m_next = m_head;
  chunk->m_size = usable;
  chunk->m_used = want;
  m_head = chunk;
  m_bytes_allocated += header + usable;
  vec_arena_bytes.fetch_add(header + usable, std::memory_order_relaxed);

  return reinterpret_cast<char *>(chunk) + header;
}
