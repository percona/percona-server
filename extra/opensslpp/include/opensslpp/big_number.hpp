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

#ifndef OPENSSLPP_BIG_NUMBER_HPP
#define OPENSSLPP_BIG_NUMBER_HPP

#include <cstdint>
#include <memory>
#include <ostream>

#include <opensslpp/big_number_fwd.hpp>

#include <opensslpp/accessor_fwd.hpp>

namespace opensslpp {

class big_number final {
  friend class accessor<big_number>;

 public:
  static const big_number zero;
  static const big_number one;

 public:
  big_number() noexcept = default;
  ~big_number() noexcept = default;

  big_number(std::uintmax_t value);
  big_number(const big_number &obj);
  big_number(big_number &&obj) noexcept = default;

  big_number &operator=(std::uintmax_t value);
  big_number &operator=(const big_number &obj);
  big_number &operator=(big_number &&obj) noexcept = default;

  void swap(big_number &obj) noexcept;

  bool is_empty() const noexcept { return !impl_; }

  std::uintmax_t get_primitive_value() const;
  void set_primitive_value(std::uintmax_t value);

  big_number &operator++();
  big_number operator++(int) {
    auto res = *this;
    ++*this;
    return res;
  }
  big_number &operator--();
  big_number operator--(int) {
    auto res = *this;
    --*this;
    return res;
  }

  friend std::ostream &operator<<(std::ostream &os, const big_number &obj);

 private:
  // should not be declared final as this prevents optimization for empty
  // deleter in std::unique_ptr
  struct bignum_deleter {
    void operator()(void *bn) const noexcept;
  };

  using impl_ptr = std::unique_ptr<void, bignum_deleter>;
  impl_ptr impl_;

  void set_primitive_value_internal(std::uintmax_t value);
};

}  // namespace opensslpp

#endif
