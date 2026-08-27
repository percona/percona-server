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
@file include/vec0arena.h
Arena allocator for HNSW graph nodes.
*/

#pragma once

#include <cstddef>
#include <cstdint>

/** Arena the HNSW graph allocates its nodes from.

Satisfies the class's ArenaAllocator contract: a default constructor, and
`void *allocate(size_t)`. There is deliberately no free for an individual
block — the graph never destroys a node, so the arena only has to outlive
the graph and release everything at once. HNSW holds one of these by value
and is neither copyable nor movable, so destroying the graph destroys the
arena, which is exactly the lifetime the contract asks for.

Blocks are carved from larger chunks so that a graph with a million small
nodes does not become a million malloc calls. A request too large for a
fresh chunk gets a chunk of its own, which keeps the allocator correct for
a big VECTOR(n) without oversizing every chunk to suit the worst case.

`allocate()` returns nullptr when the underlying allocation fails; the
class asserts on that today (hnsw.h has two "revisit once we add memory
limits" TODOs). Byte accounting for innodb_hnsw_max_memory belongs here
later — this is the single point every graph byte passes through — but
the refusal itself has to happen before insert() starts mutating, not
inside allocate(), because there is no per-block free to unwind with. */
class Vec_arena {
 public:
  Vec_arena() = default;
  ~Vec_arena();

  Vec_arena(const Vec_arena &) = delete;
  Vec_arena &operator=(const Vec_arena &) = delete;
  Vec_arena(Vec_arena &&) = delete;
  Vec_arena &operator=(Vec_arena &&) = delete;

  /** Allocate `size` bytes, aligned for any scalar type.
  @param[in]  size  bytes wanted
  @return the block, or nullptr if the allocation failed */
  void *allocate(size_t size);

  /** Total bytes handed to the underlying allocator, chunk headers
  included. What a memory budget would charge. */
  size_t bytes_allocated() const { return m_bytes_allocated; }

 private:
  /** Default chunk size. Large enough that ordinary nodes amortise well,
  small enough that an index with few rows is not charged megabytes. */
  static constexpr size_t CHUNK_SIZE = 64 * 1024;

  struct Chunk {
    Chunk *m_next;
    size_t m_size; /*!< usable bytes after this header */
    size_t m_used;
  };

  Chunk *m_head{nullptr};
  size_t m_bytes_allocated{0};
};
