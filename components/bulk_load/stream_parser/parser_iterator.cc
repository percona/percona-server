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

#include "parser_iterator.h"

#include <utility>

namespace Bulk_load {

ParserGenerator::ParserGenerator(promise_type &promise)
    : m_handle{std::coroutine_handle<promise_type>::from_promise(promise)} {}

ParserGenerator::ParserGenerator(ParserGenerator &&iter) noexcept
    : m_handle{std::exchange(iter.m_handle, nullptr)} {}

ParserGenerator &ParserGenerator::operator=(ParserGenerator &&iter) noexcept {
  m_handle = std::exchange(iter.m_handle, nullptr);
  return *this;
}

ParserGenerator::~ParserGenerator() noexcept {
  if (m_handle) {
    m_handle.destroy();
  }
}

ParserGenerator::iterator ParserGenerator::begin() {
  auto it = iterator{this};
  return ++it;
}

std::default_sentinel_t ParserGenerator::end() { return {}; }

ParserGenerator::iterator::iterator(ParserGenerator *generator)
    : m_generator{generator} {}

bool ParserGenerator::iterator::operator==(std::default_sentinel_t) const {
  return m_generator->m_handle.done();
}

ParserGenerator::iterator &ParserGenerator::iterator::operator++() {
  if (m_generator !=nullptr && m_generator->m_handle) {
    m_generator->m_handle.resume();
  }
  return *this;
}

ParserGenerator::iterator &ParserGenerator::iterator::operator++(int) {
  return ++(*this);
}

const ParserResult *ParserGenerator::iterator::operator*() const {
  return &m_generator->m_handle.promise().m_parser_result;
}

ParserGenerator ParserGenerator::promise_type::get_return_object() {
  return ParserGenerator{*this};
}

std::suspend_always
ParserGenerator::promise_type::yield_value(ParserResult result) noexcept {
  m_parser_result = result;
  return {};
}

void ParserGenerator::promise_type::return_value(ParserResult result) {
  m_parser_result = result;
}

}  // namespace Bulk_load
