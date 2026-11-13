/* Copyright (c) 2026 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; version 2 of
   the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "my_base.h"
#include "sql/handler.h"
#include "sql/partitioning/partition_base.h"
#include "sql/table.h"
#include "unittest/gunit/base_mock_handler.h"
#include "unittest/gunit/fake_table.h"
#include "unittest/gunit/test_utils.h"

namespace native_part {

using my_testing::Server_initializer;
using ::testing::NiceMock;

/**
  The handler of a partition. An index read finds no row and records
  whether the handler had an end range when it was called.
*/
class Mock_partition : public Base_mock_HANDLER {
 public:
  Mock_partition(handlerton *hton, TABLE_SHARE *share)
      : Base_mock_HANDLER(hton, share) {}

  int index_first(uchar *) override { return read(); }
  int index_last(uchar *) override { return read(); }
  int index_read_map(uchar *, const uchar *, key_part_map,
                     enum ha_rkey_function) override {
    return read();
  }
  int index_read_last_map(uchar *, const uchar *, key_part_map) override {
    return read();
  }

  bool has_end_range() const { return end_range != nullptr; }

  /// The handler had an end range in the last index read.
  bool m_read_with_end_range{false};

 private:
  int read() {
    m_read_with_end_range = has_end_range();
    return HA_ERR_END_OF_FILE;
  }
};

/**
  A partitioned table whose partitions are handlers of the test.
*/
class Test_partition_base : public Partition_base {
 public:
  Test_partition_base(TABLE_SHARE *share, handler **partitions,
                      uint num_partitions)
      : Partition_base(nullptr, share) {
    m_file = partitions;
    m_tot_parts = num_partitions;
  }

  ~Test_partition_base() override {
    // The test owns the partitions, ~Partition_base() must not destroy them.
    m_file = nullptr;
    m_tot_parts = 0;
  }

  handler *clone(const char *, MEM_ROOT *) override { return nullptr; }
  bool rpl_lookup_rows() override { return false; }
  enum row_type get_partition_row_type(const dd::Table *, uint) override {
    return ROW_TYPE_NOT_USED;
  }

 private:
  handler *get_file_handler(TABLE_SHARE *, MEM_ROOT *) const override {
    return nullptr;
  }
};

/**
  The SQL layer sets and clears the end range of a scan only in the handler
  of the partitioned table. A range read of a partition copies it to the
  handler of the partition, where it stays until the index is closed, so
  Partition_base has to clear it before every other index read in that
  partition (PS-11793). Only index_read_map_in_part() can be reached from SQL
  after a range read at present, by skip scan and loose index scan, see
  rocksdb.skip_scan_partition_wrong_result. This test covers the other index
  reads too.

  The fixture is a friend of Partition_base and calls its private functions
  for the tests.
*/
class PartitionBaseEndRangeTest : public ::testing::Test {
 protected:
  void SetUp() override {
    m_initializer.SetUp();

    m_table.reset(new Fake_TABLE(1, false));
    m_table->create_index(m_table->field[0]);
    // The handler of a temporary table does not have to be locked.
    m_table->s->tmp_table = INTERNAL_TMP_TABLE;

    m_partition.reset(new NiceMock<Mock_partition>(nullptr, m_table->s));
    m_partition->change_table_ptr(m_table.get(), m_table->s);
    m_partitions[0] = m_partition.get();
    m_partitions[1] = nullptr;
    m_base.reset(new Test_partition_base(m_table->s, m_partitions, 1));

    ASSERT_EQ(0, m_partition->ha_index_init(0, false));
    ASSERT_FALSE(m_partition->has_end_range());
  }

  void TearDown() override {
    EXPECT_EQ(0, m_partition->ha_index_end());
    m_base.reset();
    m_partition.reset();
    m_table.reset();
    m_initializer.TearDown();
  }

  /// Leave an end range in the partition, as a range read does.
  void read_range() {
    key_range end_key;
    end_key.key = m_key;
    end_key.length = 0;
    end_key.keypart_map = 0;
    end_key.flag = HA_READ_AFTER_KEY;
    EXPECT_EQ(HA_ERR_END_OF_FILE, m_base->read_range_first_in_part(
                                      0, nullptr, nullptr, &end_key, false));
    EXPECT_TRUE(m_partition->m_read_with_end_range);
    EXPECT_TRUE(m_partition->has_end_range());
  }

  /// The index read after the range read did not see its end range.
  void expect_no_end_range() const {
    EXPECT_FALSE(m_partition->m_read_with_end_range);
    EXPECT_FALSE(m_partition->has_end_range());
  }

  int index_read_map_in_part() {
    return m_base->index_read_map_in_part(0, m_table->record[0], m_key, 1,
                                          HA_READ_KEY_OR_NEXT);
  }
  int index_first_in_part() {
    return m_base->index_first_in_part(0, m_table->record[0]);
  }
  int index_last_in_part() {
    return m_base->index_last_in_part(0, m_table->record[0]);
  }
  int index_read_last_map_in_part() {
    return m_base->index_read_last_map_in_part(0, m_table->record[0], m_key, 1);
  }

  Server_initializer m_initializer;
  std::unique_ptr<Fake_TABLE> m_table;
  std::unique_ptr<NiceMock<Mock_partition>> m_partition;
  handler *m_partitions[2];
  std::unique_ptr<Test_partition_base> m_base;
  uchar m_key[8]{};
};

TEST_F(PartitionBaseEndRangeTest, IndexReadMap) {
  read_range();
  EXPECT_EQ(HA_ERR_END_OF_FILE, index_read_map_in_part());
  expect_no_end_range();
}

TEST_F(PartitionBaseEndRangeTest, IndexFirst) {
  read_range();
  EXPECT_EQ(HA_ERR_END_OF_FILE, index_first_in_part());
  expect_no_end_range();
}

TEST_F(PartitionBaseEndRangeTest, IndexLast) {
  read_range();
  EXPECT_EQ(HA_ERR_END_OF_FILE, index_last_in_part());
  expect_no_end_range();
}

TEST_F(PartitionBaseEndRangeTest, IndexReadLastMap) {
  read_range();
  EXPECT_EQ(HA_ERR_END_OF_FILE, index_read_last_map_in_part());
  expect_no_end_range();
}

}  // namespace native_part
