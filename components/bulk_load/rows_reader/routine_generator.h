/* Copyright (c) 2024, Percona and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation. The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA */

#pragma once

#include "generator_promise.h"
#include "generator_iterator.h"

#include <coroutine>
#include <utility>

namespace Bulk_load {

template <class DataType>
struct RoutineGenerator {
  using promise_type = GeneratorPromise<RoutineGenerator, DataType>;
  using iterator_type = GeneratorIterator<RoutineGenerator, DataType>;

  explicit RoutineGenerator(promise_type &promise)
      : m_handle{std::coroutine_handle<promise_type>::from_promise(promise)} {}
  RoutineGenerator(RoutineGenerator &&iter) noexcept
      : m_handle{std::exchange(iter.m_handle, nullptr)} {}
  RoutineGenerator &operator=(RoutineGenerator &&iter) noexcept {
    m_handle = std::exchange(iter.m_handle, nullptr);
    return *this;
  }
  RoutineGenerator(const RoutineGenerator &iter) = delete;
  RoutineGenerator &operator=(const RoutineGenerator &iter) = delete;
  ~RoutineGenerator() noexcept {
    if (m_handle) {
      m_handle.destroy();
    }
  }

  void resume() { if (m_handle) m_handle.resume(); }
  bool check_done() const { return m_handle.done(); }
  const DataType *data() const { return m_handle.promise().data(); }

  iterator_type begin() {
    auto it = iterator_type{this};
    return ++it;
  }
  std::default_sentinel_t end() { return {}; }

  std::coroutine_handle<promise_type> m_handle{};
};

}  // namespace Bulk_load
