/* Copyright (c) 2026, Percona LLC and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is designed to work with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have either included with
   the program or referenced in the documentation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/* See http://code.google.com/p/googletest/wiki/Primer */

#include <gtest/gtest.h>
#include <cstdlib>
#include <new>
#include <vector>

#include "os0event.h"
#include "srv0srv.h"
#include "sync0arr_impl.h"
#include "sync0debug.h"
#include "sync0rw.h"

namespace innodb_sync0arr_unittest {

/** RAII wait-array cell so TearDown can close sync arrays even on ASSERT. */
struct reserved_cell {
  sync_array_t *arr{nullptr};
  sync_cell_t *cell{nullptr};

  ~reserved_cell() {
    if (arr != nullptr && cell != nullptr) {
      sync_array_free_cell(arr, cell);
    }
  }
};

struct rw_lock_holder {
  rw_lock_t *lock;

  rw_lock_holder() {
    lock = static_cast<rw_lock_t *>(std::malloc(sizeof(rw_lock_t)));
    if (lock == nullptr) {
      throw std::bad_alloc();
    }
    rw_lock_create(PSI_NOT_INSTRUMENTED, lock, LATCH_ID_BUF_BLOCK_LOCK);
  }

  ~rw_lock_holder() {
    rw_lock_free(lock);
    std::free(lock);
  }
};

struct counting_indexer {
  static inline size_t index{0};
  static inline size_t calls{0};

  static size_t get_rnd_index() {
    ++calls;
    return index;
  }
};

class sync0arr : public ::testing::TestWithParam<size_t> {
 protected:
  void SetUp() override {
    saved_sync_array_size = srv_sync_array_size;
    srv_sync_array_size = GetParam();
    counting_indexer::calls = 0;
    counting_indexer::index = 0;
    os_event_global_init();
    /* One cell per instance. */
    sync_check_init(GetParam());
    ASSERT_EQ(sync_array_size, GetParam());
    ASSERT_EQ(sync_wait_array[0]->n_cells, 1UL);
  }

  void TearDown() override {
    sync_check_close();
    os_event_global_destroy();
    srv_sync_array_size = saved_sync_array_size;
  }

  ulong saved_sync_array_size{1};
};

TEST_P(sync0arr, reserve_succeeds_when_one_instance_has_space) {
  /* Destroy cells before lock: declare lock first. */
  rw_lock_holder lock;
  std::vector<reserved_cell> filled(GetParam() - 1);

  for (size_t i = 0; i < filled.size(); ++i) {
    filled[i].arr = sync_wait_array[i + 1];
    filled[i].cell = sync_array_reserve_cell(filled[i].arr, lock.lock,
                                             RW_LOCK_S, UT_LOCATION_HERE);
    ASSERT_NE(filled[i].cell, nullptr) << "array " << i + 1;
  }

  /* Start just after the free instance, forcing a full walk and wraparound.
  Repeated sampling of this index cannot find the free cell. Use an index
  larger than the array count to exercise the initial modulo as well. */
  counting_indexer::index = GetParam() + 1;
  for (size_t trial = 0; trial < 2; ++trial) {
    reserved_cell extra;
    extra.arr = sync_array_get_and_reserve_cell<counting_indexer>(
        lock.lock, RW_LOCK_S, UT_LOCATION_HERE, &extra.cell);
    ASSERT_NE(extra.cell, nullptr) << "trial " << trial;
    EXPECT_EQ(extra.arr, sync_wait_array[0]);
    EXPECT_EQ(counting_indexer::calls, GetParam() > 1 ? trial + 1 : 0);
  }

  /* Also exercise the default indexer through the production entry point. */
  reserved_cell extra;
  extra.arr = sync_array_get_and_reserve_cell(lock.lock, RW_LOCK_S,
                                              UT_LOCATION_HERE, &extra.cell);
  ASSERT_NE(extra.cell, nullptr);
  EXPECT_EQ(extra.arr, sync_wait_array[0]);
}

TEST_P(sync0arr, reserve_uses_starting_instance_when_it_has_space) {
  rw_lock_holder lock;
  reserved_cell extra;
  counting_indexer::index = 2 * GetParam() - 1;
  extra.arr = sync_array_get_and_reserve_cell<counting_indexer>(
      lock.lock, RW_LOCK_S, UT_LOCATION_HERE, &extra.cell);
  ASSERT_NE(extra.cell, nullptr);
  EXPECT_EQ(extra.arr, sync_wait_array[GetParam() - 1]);
  EXPECT_EQ(counting_indexer::calls, GetParam() > 1 ? 1U : 0U);
}

INSTANTIATE_TEST_SUITE_P(array_sizes, sync0arr,
                         ::testing::Values(1, 2, 3, 1024));

}  // namespace innodb_sync0arr_unittest
