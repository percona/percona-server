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

#ifndef OPENSSLPP_CORE_ALLOCATED_BUFFER
#define OPENSSLPP_CORE_ALLOCATED_BUFFER

#include <cstdint>
#include <memory>

namespace opensslpp {
class core_allocated_buffer final {
 private:
  static constexpr std::size_t nlength = ~static_cast<std::size_t>(0);

 public:
  explicit core_allocated_buffer(void *ptr = nullptr,
                                 std::size_t length = nlength) noexcept
      : impl_{ptr}, length_{ptr == nullptr ? 0 : length} {}
  ~core_allocated_buffer() noexcept = default;

  core_allocated_buffer(const core_allocated_buffer &obj) noexcept = delete;
  core_allocated_buffer(core_allocated_buffer &&obj) noexcept = default;

  core_allocated_buffer &operator=(const core_allocated_buffer &obj) noexcept =
      delete;
  core_allocated_buffer &operator=(core_allocated_buffer &&obj) noexcept =
      default;

  void *get_raw_ptr() noexcept { return impl_.get(); }
  const void *get_raw_ptr() const noexcept { return impl_.get(); }

  template <typename T>
  T *get_typed_ptr() noexcept {
    return static_cast<T *>(get_raw_ptr());
  }
  template <typename T>
  const T *get_typed_ptr() const noexcept {
    return static_cast<const T *>(get_raw_ptr());
  }

  bool is_empty() const noexcept { return !impl_; }

  bool has_length() const noexcept { return length_ != nlength; }
  std::size_t get_length() const noexcept { return length_; }

 private:
  // should not be declared final as this prevents optimization for empty
  // deleter in std::unique_ptr
  struct core_deleter {
    void operator()(void *ptr) const noexcept;
  };

  using impl_ptr = std::unique_ptr<void, core_deleter>;
  impl_ptr impl_;
  std::size_t length_;
};

}  // namespace opensslpp

#endif
