/* Copyright (c) 2022 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; version 2 of
   the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#include <cassert>

#include <openssl/bn.h>

#include <opensslpp/big_number.hpp>

#include <opensslpp/core_error.hpp>

#include "opensslpp/big_number_accessor.hpp"

namespace opensslpp {

void big_number::bignum_deleter::operator()(void *bn) const noexcept {
  if (bn != nullptr) BN_free(static_cast<BIGNUM *>(bn));
}

/*static*/ const big_number big_number::zero{0};
/*static*/ const big_number big_number::one{1};

big_number::big_number(std::uintmax_t value) : impl_{BN_new()} {
  if (is_empty()) throw core_error{"cannot create big number"};

  set_primitive_value_internal(value);
}

big_number::big_number(const big_number &obj)
    : impl_{obj.is_empty() ? nullptr
                           : BN_dup(big_number_accessor::get_impl(obj))} {
  if (!obj.is_empty() && is_empty())
    throw core_error{"cannot duplicate big number"};
}

big_number &big_number::operator=(std::uintmax_t value) {
  set_primitive_value(value);
  return *this;
}

big_number &big_number::operator=(const big_number &obj) {
  auto tmp = big_number{obj};
  swap(tmp);
  return *this;
}

void big_number::swap(big_number &obj) noexcept { impl_.swap(obj.impl_); }

std::uintmax_t big_number::get_primitive_value() const {
  assert(!is_empty());

  std::uintmax_t res = BN_get_word(big_number_accessor::get_impl(*this));
  if (res == ~static_cast<std::uintmax_t>(0))
    throw core_error{"big number is too big for a primitive type"};
  return res;
}

void big_number::set_primitive_value(std::uintmax_t value) {
  if (is_empty()) {
    auto tmp = big_number{value};
    swap(tmp);
  } else {
    set_primitive_value_internal(value);
  }
}
big_number &big_number::operator++() {
  assert(!is_empty());
  if (BN_add_word(big_number_accessor::get_impl(*this), 1) == 0)
    throw core_error{"cannot increment big number value"};
  return *this;
}

big_number &big_number::operator--() {
  assert(!is_empty());
  if (BN_sub_word(big_number_accessor::get_impl(*this), 1) == 0)
    throw core_error{"cannot decrement big number value"};
  return *this;
}

struct openssl_core_deleter {
  void operator()(char *ptr) const noexcept {
    if (ptr != nullptr) OPENSSL_free(ptr);
  }
};
using openssl_core_buffer_ptr = std::unique_ptr<char, openssl_core_deleter>;

std::ostream &operator<<(std::ostream &os, const big_number &obj) {
  assert(!obj.is_empty());
  openssl_core_buffer_ptr buffer{BN_bn2dec(big_number_accessor::get_impl(obj))};
  if (!buffer) throw core_error{"cannot convert big number to decimal string"};
  os << buffer.get();
  return os;
}

void big_number::set_primitive_value_internal(std::uintmax_t value) {
  if (BN_set_word(big_number_accessor::get_impl(*this), value) == 0)
    throw core_error{"cannot set big number value"};
}

}  // namespace opensslpp
