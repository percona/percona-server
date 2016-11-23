/*
   Portions Copyright (c) 2015-Present, Facebook, Inc.
   Portions Copyright (c) 2012,2013 Monty Program Ab

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
#pragma once

/* C++ standard header files */
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <string>

/* MySQL header files */
#include "./handler.h"
#include <my_global.h>
#include "./atomic_stat.h"

/* my_io_perf_* functionality snatched from upstream FacebookSQL server core */

/* Struct used for IO performance counters within a single thread */
struct my_io_perf_struct {
  ulonglong bytes;
  ulonglong requests;
  ulonglong svc_time; /*!< time to do read or write operation */
  ulonglong svc_time_max;
  ulonglong wait_time; /*!< total time in the request array */
  ulonglong wait_time_max;
  ulonglong slow_ios; /*!< requests that take too long */

  /* Initialize an my_io_perf_t struct. */
  inline void init() {
    memset(this, 0, sizeof(*this));
  }

  /* Accumulates io perf values */
  inline void sum(const my_io_perf_struct& perf) {
    bytes += perf.bytes;
    requests += perf.requests;
    svc_time += perf.svc_time;
    svc_time_max = std::max(svc_time_max, perf.svc_time_max);
    wait_time += perf.wait_time;
    wait_time_max = std::max(wait_time_max, perf.wait_time_max);
    slow_ios += perf.slow_ios;
  }
};
typedef struct my_io_perf_struct my_io_perf_t;

/* Struct used for IO performance counters, shared among multiple threads */
struct my_io_perf_atomic_struct {
  atomic_stat<ulonglong> bytes;
  atomic_stat<ulonglong> requests;
  atomic_stat<ulonglong> svc_time; /*!< time to do read or write operation */
  atomic_stat<ulonglong> svc_time_max;
  atomic_stat<ulonglong> wait_time; /*!< total time in the request array */
  atomic_stat<ulonglong> wait_time_max;
  atomic_stat<ulonglong> slow_ios; /*!< requests that take too long */

  /* Initialize an my_io_perf_atomic_t struct. */
  inline void init() {
    bytes.clear();
    requests.clear();
    svc_time.clear();
    svc_time_max.clear();
    wait_time.clear();
    wait_time_max.clear();
    slow_ios.clear();
  }

  /* Accumulates io perf values using atomic operations */
  inline void sum(const my_io_perf_t& perf) {
    bytes.inc(perf.bytes);
    requests.inc(perf.requests);

    svc_time.inc(perf.svc_time);
    wait_time.inc(perf.wait_time);

    /*
       In the unlikely case that two threads attempt to update the max
       value at the same time, only the first will succeed.  It's possible
       that the second thread would have set a larger max value, but we
       would rather error on the side of simplicity and avoid looping the
       compare-and-swap.
    */
    svc_time_max.set_max_maybe(perf.svc_time);
    wait_time_max.set_max_maybe(perf.wait_time);

    slow_ios.inc(perf.slow_ios);
  }
};
typedef struct my_io_perf_atomic_struct my_io_perf_atomic_t;

namespace myrocks {

enum {
  PC_USER_KEY_COMPARISON_COUNT = 0,
  PC_BLOCK_CACHE_HIT_COUNT,
  PC_BLOCK_READ_COUNT,
  PC_BLOCK_READ_BYTE,
  PC_BLOCK_READ_TIME,
  PC_BLOCK_CHECKSUM_TIME,
  PC_BLOCK_DECOMPRESS_TIME,
  PC_KEY_SKIPPED,
  PC_DELETE_SKIPPED,
  PC_GET_SNAPSHOT_TIME,
  PC_GET_FROM_MEMTABLE_TIME,
  PC_GET_FROM_MEMTABLE_COUNT,
  PC_GET_POST_PROCESS_TIME,
  PC_GET_FROM_OUTPUT_FILES_TIME,
  PC_SEEK_ON_MEMTABLE_TIME,
  PC_SEEK_ON_MEMTABLE_COUNT,
  PC_SEEK_CHILD_SEEK_TIME,
  PC_SEEK_CHILD_SEEK_COUNT,
  PC_SEEK_MIN_HEAP_TIME,
  PC_SEEK_INTERNAL_SEEK_TIME,
  PC_FIND_NEXT_USER_ENTRY_TIME,
  PC_WRITE_WAL_TIME,
  PC_WRITE_MEMTABLE_TIME,
  PC_WRITE_DELAY_TIME,
  PC_WRITE_PRE_AND_POST_PROCESSS_TIME,
  PC_DB_MUTEX_LOCK_NANOS,
  PC_DB_CONDITION_WAIT_NANOS,
  PC_MERGE_OPERATOR_TIME_NANOS,
  PC_READ_INDEX_BLOCK_NANOS,
  PC_READ_FILTER_BLOCK_NANOS,
  PC_NEW_TABLE_BLOCK_ITER_NANOS,
  PC_NEW_TABLE_ITERATOR_NANOS,
  PC_BLOCK_SEEK_NANOS,
  PC_FIND_TABLE_NANOS,
  PC_IO_THREAD_POOL_ID,
  PC_IO_BYTES_WRITTEN,
  PC_IO_BYTES_READ,
  PC_IO_OPEN_NANOS,
  PC_IO_ALLOCATE_NANOS,
  PC_IO_WRITE_NANOS,
  PC_IO_READ_NANOS,
  PC_IO_RANGE_SYNC_NANOS,
  PC_IO_LOGGER_NANOS,
  PC_MAX_IDX
};

class Rdb_perf_counters;

/*
  A collection of performance counters that can be safely incremented by
  multiple threads since it stores atomic datapoints.
*/
struct Rdb_atomic_perf_counters
{
  std::atomic_ullong m_value[PC_MAX_IDX];
};

/*
  A collection of performance counters that is meant to be incremented by
  a single thread.
*/
class Rdb_perf_counters
{
 public:
  uint64_t m_value[PC_MAX_IDX];

  void load(const Rdb_atomic_perf_counters &atomic_counters);
};

extern std::string rdb_pc_stat_types[PC_MAX_IDX];

/*
  Perf timers for data reads
 */
class Rdb_io_perf
{
  // Context management
  Rdb_atomic_perf_counters *m_atomic_counters= nullptr;
  my_io_perf_atomic_t *m_shared_io_perf_read= nullptr;
  ha_statistics *m_stats= nullptr;

 public:
  void init(Rdb_atomic_perf_counters *atomic_counters,
            my_io_perf_atomic_t *shared_io_perf_read,
            ha_statistics *stats)
  {
    DBUG_ASSERT(atomic_counters != nullptr);
    DBUG_ASSERT(shared_io_perf_read != nullptr);
    DBUG_ASSERT(stats != nullptr);

    m_atomic_counters= atomic_counters;
    m_shared_io_perf_read= shared_io_perf_read;
    m_stats= stats;
  }

  bool start(uint32_t perf_context_level);
  void end_and_record(uint32_t perf_context_level);

  explicit Rdb_io_perf() : m_atomic_counters(nullptr),
                           m_shared_io_perf_read(nullptr),
                           m_stats(nullptr) {}
};

}  // namespace myrocks
