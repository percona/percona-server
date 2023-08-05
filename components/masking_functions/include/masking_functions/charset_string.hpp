/* Copyright (c) 2023 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   51 Franklin Street, Suite 500, Boston, MA 02110-1335 USA */

#ifndef MASKING_FUNCTIONS_CHARSET_STRING_HPP
#define MASKING_FUNCTIONS_CHARSET_STRING_HPP

#include <cassert>
#include <cstdint>
#include <memory>
#include <string_view>
#include <utility>

#include "masking_functions/charset_string_fwd.hpp"

#include "masking_functions/string_service_tuple_fwd.hpp"

namespace masking_functions {

// A wrapper class that uses MySQL string services under the hood and
// simplifies operations on strings of different character sets / collations.
// It supports value semantics and provides a minimal 'std::string'-like
// interface.
// 'string_service_tuple' here can be considered as an analogue of
// 'char_traits' / 'allocator' combination in 'std::string'.
class charset_string {
 public:
  using collation_type = void *;

  static constexpr const char *const ascii_collation_name = "ascii_general_ci";
  static constexpr const char *const utf8mb4_collation_name =
      "utf8mb4_0900_ai_ci";
  static constexpr const char *const default_collation_name =
      utf8mb4_collation_name;

  static collation_type get_collation_by_name(
      const string_service_tuple &services,
      const char *collation_name) noexcept;
  static collation_type get_ascii_collation(
      const string_service_tuple &services) noexcept {
    return get_collation_by_name(services, ascii_collation_name);
  }
  static collation_type get_utf8mb4_collation(
      const string_service_tuple &services) noexcept {
    return get_collation_by_name(services, utf8mb4_collation_name);
  }
  static collation_type get_default_collation(
      const string_service_tuple &services) noexcept {
    return get_collation_by_name(services, default_collation_name);
  }

  charset_string() noexcept = default;

  charset_string(const string_service_tuple &services, std::string_view buffer,
                 collation_type collation);
  charset_string(const string_service_tuple &services, std::string_view buffer,
                 const char *collation_name)
      : charset_string{services, buffer,
                       get_collation_by_name(services, collation_name)} {}

  charset_string(const charset_string &another)
      : impl_{nullptr, deleter{nullptr}} {
    if (!another.impl_) return;
    auto another_data = another.get_internal();
    charset_string local{another.get_services(), another_data.first,
                         another_data.second};
    swap(local);
  }
  charset_string &operator=(const charset_string &another) {
    charset_string local{another};
    swap(local);
    return *this;
  }

  charset_string(charset_string &&another) noexcept = default;

  charset_string &operator=(charset_string &&another) noexcept = default;

  ~charset_string() = default;

  const string_service_tuple &get_services() const noexcept {
    return *impl_.get_deleter().services;
  }

  std::size_t get_size_in_characters() const noexcept;

  std::size_t get_size_in_bytes() const noexcept;

  std::string_view get_buffer() const noexcept {
    assert(impl_);
    return get_internal().first;
  }
  collation_type get_collation() const noexcept {
    assert(impl_);
    return get_internal().second;
  }

  void clear() noexcept;

  std::uint32_t operator[](std::size_t index) const noexcept;
  std::uint32_t at(std::size_t index) const;

  charset_string &append(const charset_string &another);
  charset_string &operator+=(const charset_string &another) {
    return append(another);
  }

  void swap(charset_string &another) noexcept { impl_.swap(another.impl_); }

  charset_string substr(std::size_t offset, std::size_t count) const;

  charset_string convert_to_collation_copy(collation_type collation) const;
  charset_string convert_to_collation_copy(const char *collation_name) const {
    return convert_to_collation_copy(
        get_collation_by_name(get_services(), collation_name));
  }

  int compare(const charset_string &another) const;

 private:
  struct deleter {
    void operator()(void *ptr) const noexcept;
    const string_service_tuple *services;
  };
  using impl_type = std::unique_ptr<void, deleter>;
  impl_type impl_;

  using internal_data = std::pair<std::string_view, collation_type>;
  internal_data get_internal() const noexcept;
};

inline charset_string operator+(const charset_string &lhs,
                                const charset_string &rhs) {
  charset_string result{lhs};
  result += rhs;
  return result;
}

inline bool operator==(const charset_string &lhs, const charset_string &rhs) {
  return lhs.compare(rhs) == 0;
}
inline bool operator!=(const charset_string &lhs, const charset_string &rhs) {
  return lhs.compare(rhs) != 0;
}
inline bool operator<(const charset_string &lhs, const charset_string &rhs) {
  return lhs.compare(rhs) < 0;
}
inline bool operator<=(const charset_string &lhs, const charset_string &rhs) {
  return lhs.compare(rhs) <= 0;
}
inline bool operator>(const charset_string &lhs, const charset_string &rhs) {
  return lhs.compare(rhs) > 0;
}
inline bool operator>=(const charset_string &lhs, const charset_string &rhs) {
  return lhs.compare(rhs) >= 0;
}

const charset_string &smart_convert_to_collation(
    const charset_string &cs, charset_string::collation_type collation,
    charset_string &buffer);

inline const charset_string &smart_convert_to_collation(
    const charset_string &cs, const char *collation_name,
    charset_string &buffer) {
  return smart_convert_to_collation(
      cs,
      charset_string::get_collation_by_name(cs.get_services(), collation_name),
      buffer);
}

}  // namespace masking_functions

#endif  // MASKING_FUNCTIONS_CHARSET_STRING_HPP
