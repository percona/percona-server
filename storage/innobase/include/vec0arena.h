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

#include "mem0mem.h"

/** Arena the HNSW graph allocates its nodes from.

Satisfies the class's ArenaAllocator contract: a default constructor, and
`void *allocate(size_t)`. There is deliberately no free for an individual
block - the graph never destroys a node, so the arena only has to outlive
the graph and release everything at once. HNSW holds one of these by value
and is neither copyable nor movable, so destroying the graph destroys the
arena, which is exactly the lifetime the contract asks for.

The memory comes from an InnoDB `mem_heap_t`, which is where everything
else with a dict_table_t's lifetime lives, so teardown is one
mem_heap_free() and there is no chunk list of ours to walk. What this
class adds on top is the granularity: a dynamic heap caps its own blocks
at MEM_BLOCK_STANDARD_SIZE (8000 bytes today), which would mean one block
per two ordinary nodes and half a million malloc calls for a
million-node graph. So we take SLAB_SIZE-sized buffers from the heap -
sized above the cap, which the heap honours verbatim (mem_heap_add_block:
`if (new_size < n) new_size = n`) - and sub-allocate nodes inside them.

A request too large for a fresh slab gets a slab of its own, so a
VECTOR(16383) node is handled without oversizing every slab to suit the
worst case. The current slab is whichever has the most room left, never
simply the newest: an exactly-consumed oversized slab must not displace a
slab that still has room, or every small allocation after it buys another
slab. That was a real ~2x regression before it was fixed.

`allocate()` returns nullptr only if the heap could not hand out a slab;
in practice InnoDB's heap is fatal on allocation failure, and the class
asserts on nullptr anyway (hnsw.h has two "revisit once we add memory
limits" TODOs). Refusal against innodb_hnsw_max_memory has to happen
before insert() starts mutating rather than inside allocate(), because
there is no per-block free to unwind with. */
/** Bytes held by every Vec_arena in the server, heap block headers
included. What innodb_hnsw_max_memory is measured against. */
uint64_t vec_arena_global_bytes();

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

  /** Total bytes the underlying heap holds, block headers included.
  What a memory budget charges. */
  size_t bytes_allocated() const { return m_bytes_allocated; }

 private:
  /** Size of the buffers we take from the heap and sub-allocate inside.
  Above MEM_BLOCK_STANDARD_SIZE on purpose, so the heap gives us exactly
  this rather than its own 8000-byte default: large enough that ordinary
  nodes amortise well, small enough that an index with few rows is not
  charged megabytes. */
  static constexpr size_t SLAB_SIZE = 64 * 1024;

  /** Charge the difference between the heap's size and what we last
  counted to the server-wide total. Called after every slab. */
  void recount();

  /** The heap every slab comes from. Created on the first allocate() so
  an arena that is never used costs nothing. */
  mem_heap_t *m_heap{nullptr};
  /** Start of the slab currently being sub-allocated, and its extent.
  m_cur is null until the first slab. */
  char *m_cur{nullptr};
  size_t m_cur_size{0};
  size_t m_cur_used{0};
  /** Bytes the heap holds, as last charged to the global total. */
  size_t m_bytes_allocated{0};
};
