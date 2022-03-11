#ifndef SQL_TIMING_ITERATOR_H_
#define SQL_TIMING_ITERATOR_H_

/* Copyright (c) 2019, 2021, Oracle and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include <inttypes.h>
#include <stdio.h>
#include <chrono>

#include "my_alloc.h"
#include "sql/row_iterator.h"
#include "sql/sql_class.h"
#include "sql/sql_lex.h"

/**
  An iterator template that wraps a RowIterator, such that all calls to Init()
  and Read() are timed (all others are passed through unchanged, and possibly
  even inlined, since all RowIterator implementations are final). This is used
  for EXPLAIN ANALYZE.

  If RealIterator has a type called keeps_own_timing (no matter what it
  is), it must contain num_rows() and num_init_calls() accessors that override
  the number of Init() calls that is counted. This is useful for
  MaterializeIterator, where most Init() calls don't actually cause a
  rematerialization and are effectively free.

  See also NewIterator, below.
 */
template <class RealIterator>
class TimingIterator final : public RowIterator {
 public:
  template <class... Args>
  TimingIterator(THD *thd, Args &&... args)
      : RowIterator(thd), m_iterator(thd, std::forward<Args>(args)...) {}

  bool Init() override;
  int Read() override;
  void SetNullRowFlag(bool is_null_row) override {
    m_iterator.SetNullRowFlag(is_null_row);
  }
  void UnlockRow() override { m_iterator.UnlockRow(); }
  void StartPSIBatchMode() override { m_iterator.StartPSIBatchMode(); }
  void EndPSIBatchModeIfStarted() override {
    m_iterator.EndPSIBatchModeIfStarted();
  }

  std::string TimingString() const override;

  RowIterator *real_iterator() override { return &m_iterator; }
  const RowIterator *real_iterator() const override { return &m_iterator; }

 private:
  // To avoid a lot of repetitive writing.
  using steady_clock = std::chrono::steady_clock;
  template <class T>
  using duration = std::chrono::duration<T>;

  steady_clock::time_point now() const {
#if defined(__linux__)
    // Work around very slow libstdc++ implementations of std::chrono
    // (those compiled with _GLIBCXX_USE_CLOCK_GETTIME_SYSCALL).
    timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return steady_clock::time_point(
        steady_clock::duration(std::chrono::seconds(tp.tv_sec) +
                               std::chrono::nanoseconds(tp.tv_nsec)));
#else
    return steady_clock::now();
#endif
  }

  // These are at the same offset for all TimingIterator specializations,
  // in the hope that the linker can manage to fold all the TimingString()
  // implementations into one.
  uint64_t m_num_rows = 0;
  uint64_t m_num_init_calls = 0;
  steady_clock::time_point::duration m_time_spent_in_first_row{0};
  steady_clock::time_point::duration m_time_spent_in_other_rows{0};
  bool m_first_row;

  RealIterator m_iterator;
};

template <class RealIterator>
bool TimingIterator<RealIterator>::Init() {
  ++m_num_init_calls;
  steady_clock::time_point start = now();
  bool err = m_iterator.Init();
  steady_clock::time_point end = now();
  m_time_spent_in_first_row += end - start;
  m_first_row = true;
  return err;
}

template <class RealIterator>
int TimingIterator<RealIterator>::Read() {
  steady_clock::time_point start = now();
  int err = m_iterator.Read();
  steady_clock::time_point end = now();
  if (m_first_row) {
    m_time_spent_in_first_row += end - start;
    m_first_row = false;
  } else {
    m_time_spent_in_other_rows += end - start;
  }
  if (err == 0) {
    ++m_num_rows;
  }
  return err;
}

// In the default implementation, just return default_num_init_calls.
template <class RealIterator, class = void>
struct GetTimingData {
  inline uint64_t num_init_calls(const RealIterator &,
                                 uint64_t default_num_init_calls) const {
    return default_num_init_calls;
  }
  inline uint64_t num_rows(const RealIterator &,
                           uint64_t default_num_rows) const {
    return default_num_rows;
  }
};

// However, if RealIterator::keeps_own_timing exists, call its
// num_init_calls(). (If it does not exist, this template is not considered
// due to SFINAE.)
template <class RealIterator>
struct GetTimingData<RealIterator, typename RealIterator::keeps_own_timing> {
  inline uint64_t num_init_calls(const RealIterator &iterator, uint64_t) const {
    return iterator.num_init_calls();
  }
  inline uint64_t num_rows(const RealIterator &iterator, uint64_t) const {
    return iterator.num_rows();
  }
};

template <class RealIterator>
std::string TimingIterator<RealIterator>::TimingString() const {
  double first_row_ms =
      duration<double>(m_time_spent_in_first_row).count() * 1e3;
  double last_row_ms =
      duration<double>(m_time_spent_in_first_row + m_time_spent_in_other_rows)
          .count() *
      1e3;
  char buf[1024];
  GetTimingData<RealIterator> timing_data;
  const uint64_t num_init_calls =
      timing_data.num_init_calls(m_iterator, m_num_init_calls);
  const uint64_t num_rows = timing_data.num_rows(m_iterator, m_num_rows);
  if (num_init_calls == 0) {
    snprintf(buf, sizeof(buf), "(never executed)");
  } else {
    snprintf(buf, sizeof(buf),
             "(actual time=%.3f..%.3f rows=%lld loops=%" PRIu64 ")",
             first_row_ms / num_init_calls, last_row_ms / num_init_calls,
             llrintf(static_cast<double>(num_rows) / num_init_calls),
             num_init_calls);
  }
  return buf;
}

// Allocates a new iterator on the given THD's MEM_ROOT. The MEM_ROOT must live
// for at least as long as the iterator does.
//
// If we are in EXPLAIN ANALYZE, the iterator is wrapped in a TimingIterator<T>,
// so that it collects timing information. For this reason, nearly all
// instantiations of iterators should go through this function.

template <class RealIterator, class... Args>
unique_ptr_destroy_only<RowIterator> NewIterator(THD *thd, Args &&... args) {
  if (thd->lex->is_explain_analyze) {
    return unique_ptr_destroy_only<RowIterator>(
        new (thd->mem_root)
            TimingIterator<RealIterator>(thd, std::forward<Args>(args)...));
  } else {
    return unique_ptr_destroy_only<RowIterator>(
        new (thd->mem_root) RealIterator(thd, std::forward<Args>(args)...));
  }
}

#endif  // SQL_TIMING_ITERATOR_H_
