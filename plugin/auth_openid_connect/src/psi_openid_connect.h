/*
(C) 2026 Percona LLC and/or its affiliates

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
*/

#ifndef PSI_OIDC_H
#define PSI_OIDC_H

#include <my_sys.h>
#include <mysql/psi/mysql_memory.h>
#include <mysql/service_mysql_alloc.h>
#include <stdexcept>

namespace Psi_openid_connect {

void init();

extern PSI_memory_key config_memory_key;

/**
 * @brief STL-compatible allocator that routes allocations through my_malloc /
 *        my_free for MySQL PSI memory instrumentation.
 *
 * @tparam T          Value type of the container element.
 * @tparam Key        Reference to the PSI_memory_key to use for accounting.
 */
template <typename T, PSI_memory_key &Key>
class Allocator {
 public:
  using value_type = T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using pointer = T *;
  using const_pointer = const T *;
  using reference = T &;
  using const_reference = const T &;

  Allocator() noexcept = default;
  ~Allocator() = default;
  Allocator(const Allocator &) noexcept = default;
  Allocator(Allocator &&) noexcept = default;
  Allocator &operator=(const Allocator &) = default;
  Allocator &operator=(Allocator &&) = default;
  bool operator==(const Allocator &) const noexcept = default;

  template <typename U>
  explicit Allocator(const Allocator<U, Key> &) noexcept {}

  T *allocate(std::size_t size) {
    (void)this;  // fake use to silence clang-tidy "could be static"
    void *ptr = my_malloc(Key, size * sizeof(T), MYF(MY_WME));
    if (ptr == nullptr) throw std::bad_alloc();
    return static_cast<T *>(ptr);
  }

  void deallocate(T *ptr, std::size_t size [[maybe_unused]]) noexcept {
    (void)this;  // fake use to silence clang-tidy "could be static"
    my_free(ptr);
  }

  template <typename U>
  struct rebind {
    using other = Allocator<U, Key>;
  };
};

template <typename T>
using Config_allocator = Allocator<T, config_memory_key>;

}  // namespace Psi_openid_connect
#endif  // PSI_OIDC_H
