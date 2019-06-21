/*
  Copyright (c) 2018, Percona and/or its affiliates. All rights reserved.
  Copyright (c) 2010, 2015, MariaDB

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02111-1301 USA
*/

#include <gtest/gtest.h>
#include "my_config.h"

#include "m_string.h"
#include "my_sys.h"

namespace mf_iocache_unittest {

static const constexpr size_t CACHE_SIZE = 16384;

#define INFO_TAIL                                             \
  ", pos_in_file = " << info.pos_in_file << ", pos_in_mem = " \
                     << (*info.current_pos - info.request_pos)

#ifdef GTEST_HAS_PARAM_TEST

class IOCacheTest : public ::testing::TestWithParam<bool> {
 protected:
  static const constexpr char FILL = 0x5A;

  static bool data_bad(const uchar *buf, size_t len) noexcept {
    const uchar *end = buf + len;
    while (buf < end)
      if (*buf++ != FILL) return true;
    return false;
  }
};

INSTANTIATE_TEST_CASE_P(IOCacheTestGroup, IOCacheTest, ::testing::Bool());

TEST_P(IOCacheTest, ReadWrite) {
  IO_CACHE info;

  int res;
  uchar buf[CACHE_SIZE + 200];
  memset(buf, FILL, sizeof(buf));

  bool encryption_enabled = GetParam();
  init_io_cache_encryption(encryption_enabled);

  res = open_cached_file(&info, 0, 0, CACHE_SIZE, 0);
  EXPECT_EQ(0, res) << "open_cached_file" << INFO_TAIL;

  res = my_b_write(&info, buf, 100);
  EXPECT_TRUE(res == 0 && info.pos_in_file == 0) << "small write" << INFO_TAIL;

  res = my_b_write(&info, buf, sizeof(buf));
  EXPECT_TRUE(res == 0 && info.pos_in_file == CACHE_SIZE)
      << "large write" << INFO_TAIL;

  res = reinit_io_cache(&info, WRITE_CACHE, 250, 0, 0);
  EXPECT_EQ(0, res) << "reinit with rewind" << INFO_TAIL;

  res = my_b_write(&info, buf, sizeof(buf));
  EXPECT_EQ(0, res) << "large write" << INFO_TAIL;

  res = my_b_flush_io_cache(&info, 1);
  EXPECT_EQ(0, res) << "flush" << INFO_TAIL;

  res = reinit_io_cache(&info, READ_CACHE, 0, 0, 0);
  EXPECT_EQ(0, res) << "reinit READ_CACHE" << INFO_TAIL;

  res = my_pread(info.file, buf, 50, 50, MYF(MY_NABP));
  EXPECT_TRUE(res == 0 && data_bad(buf, 50) == encryption_enabled)
      << "file must be " << (encryption_enabled ? "un" : "") << "readable";

  res = my_b_read(&info, buf, 50) || data_bad(buf, 50);
  EXPECT_TRUE(res == 0 && info.pos_in_file == 0) << "small read" << INFO_TAIL;

  res = my_b_read(&info, buf, sizeof(buf)) || data_bad(buf, sizeof(buf));
  EXPECT_TRUE(res == 0 && info.pos_in_file == CACHE_SIZE)
      << "large read" << INFO_TAIL;

  close_cached_file(&info);
}

TEST_P(IOCacheTest, MDEV9044) {
  int res;
  IO_CACHE info;
  uchar buf[CACHE_SIZE + 200];

  bool encryption_enabled = GetParam();
  init_io_cache_encryption(encryption_enabled);

  res = open_cached_file(&info, 0, 0, CACHE_SIZE, 0);
  EXPECT_EQ(0, res) << "open_cached_file" << INFO_TAIL;

  res = my_b_write(
      &info, STRING_WITH_LEN(reinterpret_cast<const uchar *>("first write\0")));
  EXPECT_EQ(0, res) << "first write" << INFO_TAIL;

  res = my_b_flush_io_cache(&info, 1);
  EXPECT_EQ(0, res) << "flush" << INFO_TAIL;

  res = reinit_io_cache(&info, WRITE_CACHE, 0, 0, 0);
  EXPECT_EQ(0, res) << "reinit WRITE_CACHE" << INFO_TAIL;

  res = my_b_write(
      &info,
      STRING_WITH_LEN(reinterpret_cast<const uchar *>("second write\0")));
  EXPECT_EQ(0, res) << "second write" << INFO_TAIL;

  res = reinit_io_cache(&info, READ_CACHE, 0, 0, 0);
  EXPECT_EQ(0, res) << "reinit READ_CACHE" << INFO_TAIL;

  if (encryption_enabled) {
    info.read_pos = info.read_end;
  }
  res = my_b_fill(&info);
  EXPECT_EQ(0, res) << "fill" << INFO_TAIL;

  res = reinit_io_cache(&info, READ_CACHE, 0, 0, 0);
  EXPECT_EQ(0, res) << "reinit READ_CACHE" << INFO_TAIL;

  res = my_b_read(&info, buf, sizeof(buf));
  EXPECT_TRUE(res == 1 && strcmp((char *)buf, "second write") == 0)
      << "read '" << buf << "'";

  close_cached_file(&info);
}

#endif

}  // namespace mf_iocache_unittest
