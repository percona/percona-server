/*****************************************************************************

Copyright (c) 2020, 2022, Oracle and/or its affiliates.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is also distributed with certain software (including but not
limited to OpenSSL) that is licensed under separate terms, as designated in a
particular file or component or in included license documentation. The authors
of MySQL hereby grant you an additional permission to link the program and
your derivative works with the separately licensed software that they have
included with MySQL.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/
#include <gtest/gtest.h>
#include <chrono>

#include "os0file.h"

#include "srv0shutdown.h"
#include "unittest/gunit/thread_utils.h"

using thread::Notification;
using thread::Thread;

extern bool srv_use_fdatasync;

class os0file_t : public ::testing::Test {
 protected:
  void SetUp() override {
    bool success;
    os_file_delete_if_exists_func(TEST_FILE_NAME, nullptr);
    test_file =
        os_file_create_func(TEST_FILE_NAME, OS_FILE_CREATE, OS_FILE_NORMAL,
                            OS_BUFFERED_FILE, false, &success);
    EXPECT_TRUE(success);
  }

  void TearDown() override {
    os_file_close_func(test_file.m_file);
    os_file_delete_func(TEST_FILE_NAME);
  }

  dberr_t write_test_data(const void *data, size_t len,
                          std::chrono::nanoseconds &duration) {
    IORequest request(IORequest::WRITE);

    auto name = TEST_FILE_NAME;
    auto begin = std::chrono::high_resolution_clock::now();
    auto db_err =
        os_file_write_func(request, name, test_file.m_file, data, 0, len);
    auto end = std::chrono::high_resolution_clock::now();

    duration = end - begin;
    return db_err;
  }

  dberr_t read_test_data(void *data, size_t len,
                         std::chrono::nanoseconds &duration) {
    IORequest read_request(IORequest::READ);
    read_request.disable_compression();
    read_request.clear_encrypted();

    auto name = TEST_FILE_NAME;
    auto begin = std::chrono::high_resolution_clock::now();
    auto db_err = os_file_read_func(read_request, name, test_file.m_file, data,
                                    0, len, nullptr);
    auto end = std::chrono::high_resolution_clock::now();

    duration = end - begin;
    return db_err;
  }

  bool flush_test_data(std::chrono::nanoseconds &duration) {
    auto begin = std::chrono::high_resolution_clock::now();
    auto success = os_file_flush_func(test_file.m_file);
    auto end = std::chrono::high_resolution_clock::now();

    duration = end - begin;
    return success;
  }

  void write_read_flush(const void *data, void *buffer, const size_t len,
                        std::chrono::nanoseconds &write_duration_total,
                        std::chrono::nanoseconds &read_duration_total,
                        std::chrono::nanoseconds &flush_duration_total) {
    using namespace std::chrono;
    nanoseconds write_duration(0);
    auto write_err = write_test_data(data, len, write_duration);
    EXPECT_EQ(write_err, DB_SUCCESS);
    write_duration_total += write_duration;

    nanoseconds read_duration(0);
    auto read_err = read_test_data(buffer, len, read_duration);
    EXPECT_EQ(read_err, DB_SUCCESS);
    EXPECT_FALSE(memcmp(buffer, data, len));
    read_duration_total += read_duration;

    nanoseconds flush_duration(0);
    auto success = flush_test_data(flush_duration);
    EXPECT_TRUE(success);
    flush_duration_total += flush_duration;
  }

  void write_read_flush_loop(const void *data, void *buffer, const size_t len,
                             const int loops) {
    using namespace std::chrono;
    nanoseconds flushes(0);
    nanoseconds writes(0);
    nanoseconds reads(0);

    for (int i = 0; i < loops; ++i) {
      write_read_flush(data, buffer, len, writes, reads, flushes);
    }
    auto writes_ms = duration_cast<milliseconds>(writes).count();
    auto reads_ms = duration_cast<milliseconds>(reads).count();
    auto flushes_ms = duration_cast<milliseconds>(flushes).count();
    std::cout << "Write duration total: " << writes_ms << " ms" << std::endl;
    std::cout << "Read duration total: " << reads_ms << " ms" << std::endl;
    std::cout << "Flush duration total: " << flushes_ms << " ms" << std::endl;
  }

  pfs_os_file_t test_file;
  static constexpr char TEST_FILE_NAME[] = "os0file-t-temp.txt";
};

class AIO_worker_thread : public Thread {
 public:
  void set_segment(ulint segment) { m_segment = segment; }

  void run() override;

 private:
  ulint m_segment;
};

void AIO_worker_thread::run() {
  while (srv_shutdown_state.load() != SRV_SHUTDOWN_EXIT_THREADS ||
         !os_aio_all_slots_free()) {
    fil_node_t *m1;
    void *m2;
    IORequest type;
    auto db_err = os_aio_handler(m_segment, &m1, &m2, &type);
    EXPECT_EQ(db_err, DB_SUCCESS);
    if (m2 != nullptr) {
      Notification *done = reinterpret_cast<Notification*>(m2);
      done->notify();
    }
  }
}

extern bool srv_use_native_aio;
extern bool srv_use_native_uring;

class os0file_aio_t : public os0file_t {
 protected:
  void SetUp() override {
    srv_shutdown_state.store(SRV_SHUTDOWN_NONE);

    os_event_global_init();
    sync_check_init(10);
    os_create_block_cache();

    srv_use_native_aio = false;
    srv_use_native_uring = true;

    bool init_success = os_aio_init(READER_THREADS, WRITER_THREADS);
    EXPECT_TRUE(init_success);

    os0file_t::SetUp();

    for (ulint t = 0; t < 2 + READER_THREADS + WRITER_THREADS; ++t) {
      m_thread[t].set_segment(t);
      m_thread[t].start();
    }
  }

  void TearDown() override {
    srv_shutdown_state.store(SRV_SHUTDOWN_EXIT_THREADS);

    for (ulint t = 0; t < 2 + READER_THREADS + WRITER_THREADS; ++t) {
      m_thread[t].join();
    }

    os0file_t::TearDown();

    os_aio_free();
    sync_check_close();
    os_event_global_destroy();
  }

protected:
  AIO_worker_thread m_thread[10];

  static constexpr ulint READER_THREADS = 4;
  static constexpr ulint WRITER_THREADS = 4;
  static constexpr ulint PENDING_IOS_PER_THREAD = 256;
};

TEST_F(os0file_aio_t, basic_write_read) {
  static constexpr char TEST_DATA[] = "testdata42";
  static constexpr size_t TEST_DATA_LEN = sizeof(TEST_DATA);
  IORequest write_request(IORequest::WRITE);
  char write_buffer[OS_FILE_LOG_BLOCK_SIZE];
  Notification write_done;

  memset(write_buffer, 0, sizeof(write_buffer));
  memcpy(write_buffer, TEST_DATA, TEST_DATA_LEN);

  auto write_err =
        os_aio_func(write_request, AIO_mode::NORMAL, TEST_FILE_NAME,  test_file, write_buffer, 0, sizeof(write_buffer),
                    false, nullptr, &write_done, 0, nullptr, false);
  EXPECT_EQ(write_err, DB_SUCCESS);

  write_done.wait_for_notification();

  IORequest read_request(IORequest::READ);
  char read_buffer[OS_FILE_LOG_BLOCK_SIZE];
  Notification read_done;

  auto read_err =
        os_aio_func(read_request, AIO_mode::NORMAL, TEST_FILE_NAME,  test_file, read_buffer, 0, sizeof(read_buffer),
                    false, nullptr, &read_done, 0, nullptr, false);
  EXPECT_EQ(read_err, DB_SUCCESS);
  read_done.wait_for_notification();

  EXPECT_FALSE(memcmp(read_buffer, read_buffer, TEST_DATA_LEN));
}

TEST_F(os0file_aio_t, many_writes_reads) {
  static constexpr int REQUESTS_NUMBER = WRITER_THREADS * PENDING_IOS_PER_THREAD + 10;

  char *write_buffer[REQUESTS_NUMBER];
  Notification write_done[REQUESTS_NUMBER];

  for (int i = 0; i < REQUESTS_NUMBER; ++i) {
    write_buffer[i] = new char [OS_FILE_LOG_BLOCK_SIZE];
    memset(write_buffer[i], i & 0xFF, OS_FILE_LOG_BLOCK_SIZE);
  }

  auto begin = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < REQUESTS_NUMBER; ++i) {
    IORequest write_request(IORequest::WRITE);
    auto write_err =
          os_aio_func(write_request, AIO_mode::NORMAL, TEST_FILE_NAME,  test_file,
                      write_buffer[i], i * OS_FILE_LOG_BLOCK_SIZE, OS_FILE_LOG_BLOCK_SIZE,
                      false, nullptr, &(write_done[i]), 0, nullptr, false);
    EXPECT_EQ(write_err, DB_SUCCESS);
  }

  for (int i = 0; i < REQUESTS_NUMBER; ++i)
    write_done[i].wait_for_notification();

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::nanoseconds duration = end - begin;
  std::cout << "Write duration total: " << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << " ms" << std::endl;


  char *read_buffer[REQUESTS_NUMBER];
  Notification read_done[REQUESTS_NUMBER];

  for (int i = 0; i < REQUESTS_NUMBER; ++i)
    read_buffer[i] = new char [OS_FILE_LOG_BLOCK_SIZE];

  for (int i = 0; i < REQUESTS_NUMBER; ++i) {
    IORequest read_request(IORequest::READ);
    auto read_err =
          os_aio_func(read_request, AIO_mode::NORMAL, TEST_FILE_NAME,  test_file,
                      read_buffer[i], i * OS_FILE_LOG_BLOCK_SIZE, OS_FILE_LOG_BLOCK_SIZE,
                      false, nullptr, &(read_done[i]), 0, nullptr, false);
    EXPECT_EQ(read_err, DB_SUCCESS);
  }

  for (int i = 0; i < REQUESTS_NUMBER; ++i)
    read_done[i].wait_for_notification();

  for (int i = 0; i < REQUESTS_NUMBER; ++i) {
    char cmp_buffer[OS_FILE_LOG_BLOCK_SIZE];
    memset(cmp_buffer, i & 0xFF, OS_FILE_LOG_BLOCK_SIZE);
    read_done[i].wait_for_notification();
    EXPECT_FALSE(memcmp(read_buffer[i], cmp_buffer, OS_FILE_LOG_BLOCK_SIZE));
  }

  for (int i = 0; i < REQUESTS_NUMBER; ++i) {
    delete [] write_buffer[i];
    delete [] read_buffer[i];
  }
}

TEST_F(os0file_aio_t, buffered_reads) {
  static constexpr int REQUESTS_NUMBER = PENDING_IOS_PER_THREAD + 10;

  for (int i = 0; i < REQUESTS_NUMBER; ++i) {
    IORequest write_request(IORequest::WRITE);
    char write_buffer[OS_FILE_LOG_BLOCK_SIZE];
    memset(write_buffer, i & 0xFF, OS_FILE_LOG_BLOCK_SIZE);

    auto write_err =
        os_file_write_func(write_request, TEST_FILE_NAME, test_file.m_file, write_buffer,
                           i * OS_FILE_LOG_BLOCK_SIZE, OS_FILE_LOG_BLOCK_SIZE);
    EXPECT_EQ(write_err, DB_SUCCESS);
  }

  char *read_buffer[REQUESTS_NUMBER];
  Notification read_done[REQUESTS_NUMBER];

  for (int i = 0; i < REQUESTS_NUMBER; ++i)
    read_buffer[i] = new char [OS_FILE_LOG_BLOCK_SIZE];

  for (int i = 0; i < REQUESTS_NUMBER; ++i) {
    IORequest read_request(IORequest::READ);
    auto read_err =
          os_aio_func(read_request, AIO_mode::NORMAL, TEST_FILE_NAME,  test_file,
                      read_buffer[i], i * OS_FILE_LOG_BLOCK_SIZE, OS_FILE_LOG_BLOCK_SIZE,
                      false, nullptr, &(read_done[i]), 0, nullptr, true);
    EXPECT_EQ(read_err, DB_SUCCESS);
  }

  os_aio_dispatch_read_array_submit();

  for (int i = 0; i < REQUESTS_NUMBER; ++i)
    read_done[i].wait_for_notification();

  for (int i = 0; i < REQUESTS_NUMBER; ++i) {
    char cmp_buffer[OS_FILE_LOG_BLOCK_SIZE];
    memset(cmp_buffer, i & 0xFF, OS_FILE_LOG_BLOCK_SIZE);
    read_done[i].wait_for_notification();
    EXPECT_FALSE(memcmp(read_buffer[i], cmp_buffer, OS_FILE_LOG_BLOCK_SIZE));
  }

  for (int i = 0; i < REQUESTS_NUMBER; ++i)
    delete [] read_buffer[i];
}

TEST_F(os0file_t, hundred_10_byte_writes_reads_flushes_with_fsync) {
  srv_use_fdatasync = false;
  static constexpr char TEST_DATA[] = "testdata42";
  static constexpr size_t LEN = sizeof(TEST_DATA);
  char buffer[LEN];
  write_read_flush_loop(TEST_DATA, buffer, LEN, 100);
}

TEST_F(os0file_t, hundred_10_byte_writes_reads_flushes_with_fdatasync) {
  srv_use_fdatasync = true;
  static constexpr char TEST_DATA[] = "testdata42";
  static constexpr size_t LEN = sizeof(TEST_DATA);
  char buffer[LEN];
  write_read_flush_loop(TEST_DATA, buffer, LEN, 100);
}

/* The tests below were used to measure execution times in various scenarios.
They perform loops of large writes and many fsyncs/fdatasyncs so they last a
while. Disabled prefix makes it so that they don't execute with the
merge_innodb_tests-t suite, but we can manually run them by providing the:
--gtest_also_run_disabled_tests flag during execution */
TEST_F(os0file_t,
       DISABLED_ten_thousand_1_byte_writes_reads_flushes_with_fsync) {
  srv_use_fdatasync = false;
  static constexpr char TEST_DATA[] = "!";
  static constexpr size_t LEN = sizeof(TEST_DATA);
  char buffer[LEN];
  write_read_flush_loop(TEST_DATA, buffer, LEN, 10000);
}

TEST_F(os0file_t,
       DISABLED_ten_thousand_1_byte_writes_reads_flushes_with_fdatasync) {
  srv_use_fdatasync = true;
  static constexpr char TEST_DATA[] = "testdata42";
  static constexpr size_t LEN = sizeof(TEST_DATA);
  char buffer[LEN];
  write_read_flush_loop(TEST_DATA, buffer, LEN, 10000);
}

TEST_F(os0file_t, DISABLED_thousand_10_byte_writes_reads_flushes_with_fsync) {
  srv_use_fdatasync = false;
  static constexpr char TEST_DATA[] = "testdata42";
  static constexpr size_t LEN = sizeof(TEST_DATA);
  char buffer[LEN];
  write_read_flush_loop(TEST_DATA, buffer, LEN, 1000);
}

TEST_F(os0file_t,
       DISABLED_thousand_10_byte_writes_reads_flushes_with_fdatasync) {
  srv_use_fdatasync = true;
  static constexpr char TEST_DATA[] = "testdata42";
  static constexpr size_t LEN = sizeof(TEST_DATA);
  char buffer[LEN];
  write_read_flush_loop(TEST_DATA, buffer, LEN, 1000);
}

TEST_F(os0file_t, DISABLED_thousand_1000_byte_writes_reads_flushes_with_fsync) {
  srv_use_fdatasync = false;

  constexpr int LEN = 1000;
  char data[LEN];
  char buffer[LEN];
  for (int i = 0; i < LEN; ++i) {
    data[i] = 'a' + i % ('z' - 'a' + 1);
  }

  write_read_flush_loop(data, buffer, LEN, 1000);
}

TEST_F(os0file_t,
       DISABLED_thousand_1000_byte_writes_reads_flushes_with_fdatasync) {
  srv_use_fdatasync = true;

  constexpr int LEN = 1000;
  char data[LEN];
  char buffer[LEN];
  for (int i = 0; i < LEN; ++i) {
    data[i] = 'a' + i % ('z' - 'a' + 1);
  }

  write_read_flush_loop(data, buffer, LEN, 1000);
}

TEST_F(os0file_t, DISABLED_thousand_1M_byte_writes_reads_flushes_with_fsync) {
  srv_use_fdatasync = false;

  constexpr int LEN = 1000000;
  char data[LEN];
  char buffer[LEN];
  for (int i = 0; i < LEN; ++i) {
    data[i] = 'a' + i % ('z' - 'a' + 1);
  }

  write_read_flush_loop(data, buffer, LEN, 1000);
}

TEST_F(os0file_t,
       DISABLED_thousand_1M_byte_writes_reads_flushes_with_fdatasync) {
  srv_use_fdatasync = true;

  constexpr int LEN = 1000000;
  char data[LEN];
  char buffer[LEN];
  for (int i = 0; i < LEN; ++i) {
    data[i] = 'a' + i % ('z' - 'a' + 1);
  }

  write_read_flush_loop(data, buffer, LEN, 1000);
}
