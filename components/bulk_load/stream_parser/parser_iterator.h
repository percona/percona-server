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

#include "parser_result.h"
#include "parser_row.h"

#include <coroutine>
#include <iterator>

namespace Bulk_load {

struct ParserGenerator {

struct promise_type {
  void unhandled_exception() noexcept {}
  ParserGenerator get_return_object();
  std::suspend_always initial_suspend() noexcept { return {}; }
  std::suspend_always yield_value(ParserResult result) noexcept;
  void return_value(ParserResult result);
  std::suspend_always final_suspend() noexcept { return {}; }

  ParserResult m_parser_result;
};

class iterator {
 public:
//  iterator() = default;
  explicit iterator(ParserGenerator *generator);

  bool operator==(std::default_sentinel_t) const;
  iterator &operator++();
  iterator& operator++(int);
  const ParserResult *operator*() const;

 private:
  ParserGenerator *m_generator;
};

explicit ParserGenerator(promise_type &promise);
ParserGenerator(ParserGenerator &&iter) noexcept;
ParserGenerator &operator=(ParserGenerator &&iter) noexcept;
ParserGenerator(const ParserGenerator &iter) = delete;
ParserGenerator &operator=(const ParserGenerator &iter) = delete;
~ParserGenerator() noexcept;

iterator begin();
std::default_sentinel_t end();

std::coroutine_handle<promise_type> m_handle{};
};

}  // namespace Bulk_load
