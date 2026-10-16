/* Copyright (c) 2026 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; version 2 of
   the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

/**
  @file storage/perfschema/pfs_latch_source.cc
  Percona: wait and hold statistics of mutexes and rwlocks aggregated per
  acquire source line (implementation).

  Rows are preallocated at startup and indexed by an LF_HASH, like the
  statement digests (pfs_digest.cc).

  The acquire source is only known when a wait starts, while the wait time is
  only known when it ends, so each thread remembers its pending waits, keyed
  by the locker state, in a small thread local array. Hold time is measured
  from the end of a successful wait to the matching unlock, using a bounded
  thread local stack of held locks. Both thread local structures remember the
  generation, which changes on TRUNCATE TABLE and whenever collection is
  turned on or off, so entries recorded before are discarded.
*/

#include "storage/perfschema/pfs_latch_source.h"

#include <string.h>
#include <sys/types.h>
#include <algorithm>
#include <atomic>
#include <cassert>

#include "lf.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_murmur3.h"
#include "my_sys.h"
#include "storage/perfschema/pfs_builtin_memory.h"
#include "storage/perfschema/pfs_global.h"
#include "storage/perfschema/pfs_instr.h"
#include "storage/perfschema/pfs_lock.h"
#include "storage/perfschema/pfs_server.h"
#include "storage/perfschema/pfs_timer.h"

ulong latch_source_summary = LATCH_SOURCE_SUMMARY_OFF;
bool flag_latch_source_summary = false;
size_t latch_source_max = 0;
std::atomic<ulonglong> latch_source_lost{0};
std::atomic<ulonglong> latch_source_holds_lost{0};
PFS_latch_source_stat *latch_source_stat_array = nullptr;

namespace {

/** Waits a thread can have started and not yet ended. */
constexpr uint LATCH_SOURCE_PENDING_SIZE = 8;
/** Locks a thread can hold with hold time measured. */
constexpr uint LATCH_SOURCE_HELD_SIZE = 32;
/** Retries after a concurrent insert of the same key. */
constexpr uint LATCH_SOURCE_INSERT_RETRY_MAX = 3;

LF_HASH latch_source_hash;
bool latch_source_hash_inited = false;

/** Next row to try when inserting a new key. */
std::atomic<size_t> latch_source_monotonic_index{0};
/** True when every row was found taken, until TRUNCATE TABLE. */
std::atomic<bool> latch_source_full{false};

/**
  Changed by TRUNCATE TABLE and when collection is turned on or off.
  Thread local entries of another generation are discarded.
*/
std::atomic<uint64> latch_source_generation{0};

struct Pending_wait {
  const void *m_state;
  PFS_latch_source_stat *m_row;
  uint64 m_generation;
};

struct Held_latch {
  const void *m_identity;
  PFS_latch_source_stat *m_row;
  /** Start of the current hold interval, 0 while in a condition wait. */
  ulonglong m_start;
  uint64 m_generation;
  PFS_latch_mode m_mode;
};

thread_local Pending_wait t_pending[LATCH_SOURCE_PENDING_SIZE];
thread_local uint t_pending_count = 0;
thread_local Held_latch t_held[LATCH_SOURCE_HELD_SIZE];
thread_local uint t_held_count = 0;
/**
  True while this thread searches or inserts into the LF_HASH. An insert can
  allocate memory, which can take an instrumented lock; that nested lookup is
  skipped so the pins are never used re-entrantly.
*/
thread_local bool t_in_hash = false;

uint64 current_generation() {
  return latch_source_generation.load(std::memory_order_relaxed);
}

const uchar *latch_source_hash_get_key(const uchar *entry, size_t *length) {
  const auto *row =
      *reinterpret_cast<const PFS_latch_source_stat *const *>(entry);
  assert(row != nullptr);
  *length = sizeof(PFS_latch_source_key);
  return reinterpret_cast<const uchar *>(&row->m_key);
}

uint latch_source_hash_func(const LF_HASH *, const uchar *key,
                            size_t key_len [[maybe_unused]]) {
  assert(key_len == sizeof(PFS_latch_source_key));
  return murmur3_32(key, sizeof(PFS_latch_source_key), 0);
}

int latch_source_hash_cmp_func(const uchar *key1,
                               size_t key_len1 [[maybe_unused]],
                               const uchar *key2,
                               size_t key_len2 [[maybe_unused]]) {
  assert(key_len1 == sizeof(PFS_latch_source_key));
  assert(key_len2 == sizeof(PFS_latch_source_key));
  return memcmp(key1, key2, sizeof(PFS_latch_source_key));
}

/** @return the pins of a thread, allocated on first use. */
LF_PINS *get_latch_source_hash_pins(PFS_thread *thread) {
  if (unlikely(thread->m_latch_source_hash_pins == nullptr)) {
    thread->m_latch_source_hash_pins = lf_hash_get_pins(&latch_source_hash);
  }
  return thread->m_latch_source_hash_pins;
}

/** @return the row for the key, inserted when missing, or nullptr. */
PFS_latch_source_stat *find_or_create(LF_PINS *pins,
                                      const PFS_latch_source_key &key) {
  uint retry_count = 0;

search:
  auto **entry = reinterpret_cast<PFS_latch_source_stat **>(
      lf_hash_search(&latch_source_hash, pins, &key, sizeof(key)));

  if (entry != nullptr && entry != MY_LF_ERRPTR) {
    PFS_latch_source_stat *row = *entry;
    lf_hash_search_unpin(pins);
    return row;
  }

  lf_hash_search_unpin(pins);

  if (latch_source_full.load(std::memory_order_relaxed)) {
    latch_source_lost.fetch_add(1, std::memory_order_relaxed);
    return nullptr;
  }

  for (size_t attempts = 0; attempts < latch_source_max; attempts++) {
    const size_t index =
        latch_source_monotonic_index.fetch_add(1, std::memory_order_relaxed) %
        latch_source_max;
    PFS_latch_source_stat *row = &latch_source_stat_array[index];
    pfs_dirty_state dirty_state;

    if (!row->m_lock.is_free() || !row->m_lock.free_to_dirty(&dirty_state)) {
      continue;
    }

    row->m_key = key;
    row->m_wait.reset();
    row->m_hold.reset();

    const int res = lf_hash_insert(&latch_source_hash, pins, &row);
    if (likely(res == 0)) {
      row->m_lock.dirty_to_allocated(&dirty_state);
      return row;
    }

    row->m_lock.dirty_to_free(&dirty_state);

    if (res > 0 && ++retry_count <= LATCH_SOURCE_INSERT_RETRY_MAX) {
      /* Another thread inserted the same key. */
      goto search;
    }

    /* Out of memory in lf_hash_insert, or too many retries. */
    latch_source_lost.fetch_add(1, std::memory_order_relaxed);
    return nullptr;
  }

  latch_source_full.store(true, std::memory_order_relaxed);
  latch_source_lost.fetch_add(1, std::memory_order_relaxed);
  return nullptr;
}

ulonglong hold_end(const Held_latch &held) {
  return (held.m_start == 0) ? 0 : get_wait_timer();
}

void aggregate_hold(const Held_latch &held, ulonglong end) {
  if (held.m_start != 0 && end >= held.m_start &&
      held.m_generation == current_generation()) {
    held.m_row->m_hold.aggregate_value(end - held.m_start);
  }
}

bool mode_matches(PFS_latch_mode held, PFS_latch_mode released) {
  return released == PFS_latch_mode::ANY || held == PFS_latch_mode::ANY ||
         held == released;
}

/** @return index of the newest held entry for the lock, or -1. */
int find_held(const void *identity, PFS_latch_mode mode) {
  for (int i = static_cast<int>(t_held_count) - 1; i >= 0; i--) {
    if (t_held[i].m_identity == identity &&
        mode_matches(t_held[i].m_mode, mode)) {
      return i;
    }
  }
  return -1;
}

void remove_held(int index) {
  memmove(&t_held[index], &t_held[index + 1],
          (t_held_count - index - 1) * sizeof(Held_latch));
  t_held_count--;
}

/** Make room in a full held stack. */
void evict_held() {
  /* Entries of an older generation are discarded first, without counting. */
  const uint64 generation = current_generation();
  uint kept = 0;
  for (uint i = 0; i < t_held_count; i++) {
    if (t_held[i].m_generation == generation) {
      t_held[kept++] = t_held[i];
    }
  }
  t_held_count = kept;

  if (t_held_count == LATCH_SOURCE_HELD_SIZE) {
    /* Oldest entry is most likely a lock released by another thread. */
    remove_held(0);
    latch_source_holds_lost.fetch_add(1, std::memory_order_relaxed);
  }
}

}  // namespace

int init_latch_source(const PFS_global_param *param) {
  latch_source_lost.store(0);
  latch_source_holds_lost.store(0);
  latch_source_full.store(false);
  latch_source_monotonic_index.store(0);
  latch_source_stat_array = nullptr;
  latch_source_max = (param->m_latch_source_sizing > 0)
                         ? static_cast<size_t>(param->m_latch_source_sizing)
                         : 0;

  if (latch_source_max == 0) {
    return 0;
  }

  latch_source_stat_array = PFS_MALLOC_ARRAY(
      &builtin_memory_latch_source, latch_source_max,
      sizeof(PFS_latch_source_stat), PFS_latch_source_stat, MYF(MY_ZEROFILL));

  if (unlikely(latch_source_stat_array == nullptr)) {
    latch_source_max = 0;
    return 1;
  }

  for (size_t index = 0; index < latch_source_max; index++) {
    latch_source_stat_array[index].m_wait.reset();
    latch_source_stat_array[index].m_hold.reset();
  }

  return 0;
}

void cleanup_latch_source() {
  PFS_FREE_ARRAY(&builtin_memory_latch_source, latch_source_max,
                 sizeof(PFS_latch_source_stat), latch_source_stat_array);
  latch_source_stat_array = nullptr;
  latch_source_max = 0;
}

int init_latch_source_hash(const PFS_global_param *param) {
  if (!latch_source_hash_inited && param->m_latch_source_sizing > 0) {
    lf_hash_init3(&latch_source_hash, sizeof(PFS_latch_source_stat *),
                  LF_HASH_UNIQUE, latch_source_hash_get_key,
                  latch_source_hash_func, latch_source_hash_cmp_func,
                  nullptr /* ctor */, nullptr /* dtor */, nullptr /* init */);
    latch_source_hash_inited = true;
  }
  return 0;
}

void cleanup_latch_source_hash() {
  if (latch_source_hash_inited) {
    lf_hash_destroy(&latch_source_hash);
    latch_source_hash_inited = false;
  }
}

void cleanup_latch_source_pins(PFS_thread *thread) {
  if (thread->m_latch_source_hash_pins != nullptr) {
    lf_hash_put_pins(thread->m_latch_source_hash_pins);
    thread->m_latch_source_hash_pins = nullptr;
  }
}

void reset_latch_source() {
  latch_source_generation.fetch_add(1, std::memory_order_relaxed);

  if (latch_source_stat_array != nullptr && latch_source_hash_inited) {
    /* Same re-entrancy rule as latch_source_start_wait(). */
    t_in_hash = true;
    PFS_thread *thread = PFS_thread::get_current_thread();
    LF_PINS *pins = (thread != nullptr) ? get_latch_source_hash_pins(thread)
                                        : lf_hash_get_pins(&latch_source_hash);
    if (pins != nullptr) {
      /* Rows being inserted (dirty) are left to their inserting thread. */
      for (size_t index = 0; index < latch_source_max; index++) {
        PFS_latch_source_stat *row = &latch_source_stat_array[index];
        if (row->m_lock.is_populated()) {
          const int res =
              DBUG_EVALUATE_IF("latch_source_hash_delete_oom", -1,
                               lf_hash_delete(&latch_source_hash, pins,
                                              &row->m_key, sizeof(row->m_key)));
          /* Out of memory: the hash may still point to the row, so the row
          keeps its key and stays allocated. */
          if (res != -1) {
            row->m_lock.allocated_to_free();
          }
        }
      }
      if (thread == nullptr) {
        lf_hash_put_pins(pins);
      }
    }
    t_in_hash = false;
  }

  latch_source_monotonic_index.store(0, std::memory_order_relaxed);
  latch_source_full.store(false, std::memory_order_relaxed);
  latch_source_lost.store(0, std::memory_order_relaxed);
  latch_source_holds_lost.store(0, std::memory_order_relaxed);
}

void update_latch_source_summary_flag() {
  const bool collect = (latch_source_summary == LATCH_SOURCE_SUMMARY_SOURCE);

  if (collect != flag_latch_source_summary) {
    latch_source_generation.fetch_add(1, std::memory_order_relaxed);
    flag_latch_source_summary = collect;
  }
}

void latch_source_start_wait(const void *state, PFS_instr_class *klass,
                             uint operation, const char *src_file,
                             uint src_line) {
  if (latch_source_stat_array == nullptr || t_in_hash) {
    return;
  }

  /*
    Waits of threads without a PFS_thread are not collected. With consumer
    thread_instrumentation disabled they are instrumented, also while the
    thread ends: my_thread_end() deletes the PFS_thread, then frees the
    thread's mysys state and takes THR_LOCK_threads. Allocating memory for
    LF_HASH pins or nodes is not safe at that point.
  */
  PFS_thread *thread = PFS_thread::get_current_thread();
  if (unlikely(thread == nullptr)) {
    return;
  }

  PFS_latch_source_key key;
  memset(&key, 0, sizeof(key));
  key.m_class = klass;
  key.m_operation = operation;
  key.m_line = src_line;
  if (src_file != nullptr) {
    const char *base = base_name(src_file);
    memcpy(key.m_file, base,
           std::min<size_t>(strlen(base), PFS_LATCH_SOURCE_FILE_LENGTH));
  }

  t_in_hash = true;
  PFS_latch_source_stat *row = nullptr;
  LF_PINS *pins = get_latch_source_hash_pins(thread);
  if (likely(pins != nullptr)) {
    row = find_or_create(pins, key);
  } else {
    latch_source_lost.fetch_add(1, std::memory_order_relaxed);
  }
  t_in_hash = false;

  if (row == nullptr) {
    return;
  }

  if (t_pending_count == LATCH_SOURCE_PENDING_SIZE) {
    /* Waits that never ended (collection turned off meanwhile): drop. */
    memmove(&t_pending[0], &t_pending[1],
            (LATCH_SOURCE_PENDING_SIZE - 1) * sizeof(Pending_wait));
    t_pending_count--;
  }

  t_pending[t_pending_count++] = {state, row, current_generation()};
}

void latch_source_end_wait(const void *state, const void *identity,
                           PFS_latch_mode mode, int rc, ulonglong timer_start,
                           ulonglong timer_end) {
  int found = -1;
  for (int i = static_cast<int>(t_pending_count) - 1; i >= 0; i--) {
    if (t_pending[i].m_state == state) {
      found = i;
      break;
    }
  }

  if (found < 0) {
    return;
  }

  const Pending_wait pending = t_pending[found];
  /* Entries above it belong to waits that never ended, drop them too. */
  t_pending_count = static_cast<uint>(found);

  if (pending.m_generation != current_generation()) {
    return;
  }

  pending.m_row->m_wait.aggregate_value(timer_end - timer_start);

  if (rc != 0) {
    return;
  }

  if (t_held_count == LATCH_SOURCE_HELD_SIZE) {
    evict_held();
  }

  t_held[t_held_count++] = {identity, pending.m_row, timer_end,
                            pending.m_generation, mode};
}

void latch_source_unlock(const void *identity, PFS_latch_mode mode) {
  const int found = find_held(identity, mode);

  /* No entry: lock taken before collection started, or by another thread. */
  if (found < 0) {
    return;
  }

  const Held_latch held = t_held[found];
  remove_held(found);
  aggregate_hold(held, hold_end(held));
}

void latch_source_cond_wait_start(const void *mutex_identity) {
  const int found = find_held(mutex_identity, PFS_latch_mode::ANY);
  if (found < 0) {
    return;
  }

  Held_latch &held = t_held[found];
  aggregate_hold(held, hold_end(held));
  held.m_start = 0;
}

void latch_source_cond_wait_end(const void *mutex_identity) {
  for (int i = static_cast<int>(t_held_count) - 1; i >= 0; i--) {
    if (t_held[i].m_identity == mutex_identity && t_held[i].m_start == 0) {
      t_held[i].m_start = get_wait_timer();
      return;
    }
  }
}
