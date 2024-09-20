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

#include <coroutine>

namespace Bulk_load {

template <class GeneratorType, class DataType>
struct GeneratorPromise {
  void unhandled_exception() noexcept {}
  GeneratorType get_return_object() { return GeneratorType{*this}; }
  std::suspend_always initial_suspend() noexcept { return {}; }
  std::suspend_always yield_value(DataType result) noexcept {
    m_data = result;
    return {};
  }
  void return_value(DataType result) { m_data = result; }
  std::suspend_always final_suspend() noexcept { return {}; }
  const DataType *data() const { return &m_data; }

  DataType m_data;
};

}  // namespace Bulk_load
