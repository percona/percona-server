/* Copyright (c) 2025 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is designed to work with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have either included with
   the program or referenced in the documentation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include <atomic>
#include <memory>
#include <version>

#if defined(__cpp_lib_atomic_shared_ptr)
template <typename T>
inline void atomic_store_shared(std::atomic<std::shared_ptr<T>> &target,
                                std::shared_ptr<T> value) {
  target.store(std::move(value));
}
#else
template <typename T>
inline void atomic_store_shared(std::shared_ptr<T> &target,
                                std::shared_ptr<T> value) {
  std::atomic_store(&target, std::move(value));
}
#endif

#if defined(__cpp_lib_atomic_shared_ptr)
template <typename T>
inline std::shared_ptr<T> atomic_load_shared(
    const std::atomic<std::shared_ptr<T>> &target) {
  return target.load();
}
#else
template <typename T>
inline std::shared_ptr<T> atomic_load_shared(const std::shared_ptr<T> &target) {
  return std::atomic_load(&target);
}
#endif
