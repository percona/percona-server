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

#ifndef PFS_LATCH_SOURCE_H
#define PFS_LATCH_SOURCE_H

/**
  @file storage/perfschema/pfs_latch_source.h
  Percona: wait and hold statistics of mutexes and rwlocks aggregated per
  acquire source line, exposed in table
  PERFORMANCE_SCHEMA.EVENTS_WAITS_SUMMARY_BY_SOURCE.
*/

#include <sys/types.h>
#include <atomic>
#include <climits>
#include <cstddef>

#include "my_compiler.h"
#include "my_inttypes.h"
#include "storage/perfschema/pfs_global.h"
#include "storage/perfschema/pfs_lock.h"
#include "storage/perfschema/pfs_stat.h"

struct PFS_global_param;
struct PFS_instr_class;
struct PFS_thread;

/** Maximum length of the source file base name kept in a row. */
constexpr uint PFS_LATCH_SOURCE_FILE_LENGTH = 64;

/** Lock mode remembered for a held latch, used to match unlocks. */
enum class PFS_latch_mode : uint8 { ANY, SHARED, SHARED_EXCLUSIVE, EXCLUSIVE };

/**
  Key of a row: instrument class, operation and acquire source.
  Fixed size and zero padded, so it is hashed and compared as raw bytes.
*/
struct PFS_latch_source_key {
  PFS_instr_class *m_class;
  /** enum_operation_type of the acquire operation. */
  uint32 m_operation;
  uint32 m_line;
  /** Base name of the source file, zero padded, not zero terminated. */
  char m_file[PFS_LATCH_SOURCE_FILE_LENGTH];
};

/** Single statistic updated concurrently by many threads. */
struct PFS_latch_source_atomic_stat {
  std::atomic<ulonglong> m_count{0};
  std::atomic<ulonglong> m_sum{0};
  std::atomic<ulonglong> m_min{ULLONG_MAX};
  std::atomic<ulonglong> m_max{0};

  void aggregate_value(ulonglong value) {
    m_count.fetch_add(1, std::memory_order_relaxed);
    m_sum.fetch_add(value, std::memory_order_relaxed);

    /* A row is shared by all instances of a class, so concurrent updates are
    common: a plain load and store could overwrite a better extreme. */
    ulonglong min = m_min.load(std::memory_order_relaxed);
    while (
        unlikely(value < min) &&
        !m_min.compare_exchange_weak(min, value, std::memory_order_relaxed)) {
    }
    ulonglong max = m_max.load(std::memory_order_relaxed);
    while (
        unlikely(value > max) &&
        !m_max.compare_exchange_weak(max, value, std::memory_order_relaxed)) {
    }
  }

  void reset() {
    m_count.store(0, std::memory_order_relaxed);
    m_sum.store(0, std::memory_order_relaxed);
    m_min.store(ULLONG_MAX, std::memory_order_relaxed);
    m_max.store(0, std::memory_order_relaxed);
  }

  void copy_to(PFS_single_stat *stat) const {
    stat->m_count = m_count.load(std::memory_order_relaxed);
    stat->m_sum = m_sum.load(std::memory_order_relaxed);
    stat->m_min = m_min.load(std::memory_order_relaxed);
    stat->m_max = m_max.load(std::memory_order_relaxed);
  }
};

/** A row of EVENTS_WAITS_SUMMARY_BY_SOURCE. */
struct PFS_ALIGNED PFS_latch_source_stat {
  /** Row state: free, dirty while inserted, allocated. */
  pfs_lock m_lock;
  /** Key, also the LF_HASH key. */
  PFS_latch_source_key m_key;
  PFS_latch_source_atomic_stat m_wait;
  PFS_latch_source_atomic_stat m_hold;
};

/** Values of system variable PERFORMANCE_SCHEMA_LATCH_SOURCE_SUMMARY. */
enum enum_latch_source_summary : ulong {
  LATCH_SOURCE_SUMMARY_OFF = 0,
  /** Aggregate per acquire source line. */
  LATCH_SOURCE_SUMMARY_SOURCE = 1
};

/** System variable PERFORMANCE_SCHEMA_LATCH_SOURCE_SUMMARY. */
extern ulong latch_source_summary;
/**
  True when per source data is collected. The only flag tested by the
  instrumentation.
  @sa update_latch_source_summary_flag()
*/
extern bool flag_latch_source_summary;
/** Number of rows in the table. */
extern size_t latch_source_max;
/** Waits not counted because no row was available for their key. */
extern std::atomic<ulonglong> latch_source_lost;
/** Held locks dropped from a full per-thread stack, hold time not measured. */
extern std::atomic<ulonglong> latch_source_holds_lost;
extern PFS_latch_source_stat *latch_source_stat_array;

int init_latch_source(const PFS_global_param *param);
void cleanup_latch_source();
int init_latch_source_hash(const PFS_global_param *param);
void cleanup_latch_source_hash();
/** TRUNCATE TABLE EVENTS_WAITS_SUMMARY_BY_SOURCE. */
void reset_latch_source();

/**
  Recompute flag_latch_source_summary from latch_source_summary, at startup
  and after the variable changed. Thread local state recorded before a change
  of the flag is discarded.
*/
void update_latch_source_summary_flag();

/** Release the LF_HASH pins of a thread. */
void cleanup_latch_source_pins(PFS_thread *thread);

/**
  Called when a timed mutex or rwlock wait starts.
  @param state         locker state, identifies the wait until it ends
  @param klass         instrument class
  @param operation     enum_operation_type of the acquire
  @param src_file      source file of the acquire
  @param src_line      source line of the acquire
*/
void latch_source_start_wait(const void *state, PFS_instr_class *klass,
                             uint operation, const char *src_file,
                             uint src_line);

/**
  Called when a timed mutex or rwlock wait ends.
  @param state         locker state passed to latch_source_start_wait()
  @param identity      lock instance identity
  @param mode          mode of the lock taken
  @param rc            0 when the lock was taken
  @param timer_start   wait start, in wait timer units
  @param timer_end     wait end, in wait timer units
*/
void latch_source_end_wait(const void *state, const void *identity,
                           PFS_latch_mode mode, int rc, ulonglong timer_start,
                           ulonglong timer_end);

/**
  Called when a mutex or rwlock is released.
  @param identity      lock instance identity
  @param mode          mode released, ANY when unknown
*/
void latch_source_unlock(const void *identity, PFS_latch_mode mode);

/** Called when a condition wait releases its mutex. */
void latch_source_cond_wait_start(const void *mutex_identity);

/** Called when a condition wait re-acquired its mutex. */
void latch_source_cond_wait_end(const void *mutex_identity);

#endif
