/* Copyright (c) 2019, 2024, Oracle and/or its affiliates.

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

#ifndef MEM_ROOT_DEQUE_H
#define MEM_ROOT_DEQUE_H

#include <algorithm>
#include <cassert>
#include <list>

#include "sql/mem_root_allocator.h"

/**
 * We use list instead of deque for memory efficiency reasons,
 * primarily to optimize operations like bulk insert.
 */
template <class T>
class mem_root_deque {
 private:
  using deque_container = std::list<T, Mem_root_allocator<T>>;
  deque_container list_;

 public:
  using iterator = typename deque_container::iterator;
  using const_iterator = typename deque_container::const_iterator;
  using reverse_iterator = typename deque_container::reverse_iterator;
  using const_reverse_iterator =
      typename deque_container::const_reverse_iterator;
  using value_type = T;

  explicit mem_root_deque(MEM_ROOT *mem_root)
      : list_(Mem_root_allocator<T>(mem_root)) {}

  mem_root_deque(typename deque_container::size_type count, const T &value,
                 MEM_ROOT *mem_root)
      : list_(count, value, Mem_root_allocator<T>(mem_root)) {}

  mem_root_deque(typename deque_container::size_type count, MEM_ROOT *mem_root)
      : list_(count, Mem_root_allocator<T>(mem_root)) {}

  template <class InputIt>
  mem_root_deque(InputIt first, InputIt last, MEM_ROOT *mem_root)
      : list_(first, last, Mem_root_allocator<T>(mem_root)) {}

  mem_root_deque(const mem_root_deque &other) : list_(other.list_) {}

  mem_root_deque(const mem_root_deque &other, MEM_ROOT *mem_root)
      : list_(other.list_, Mem_root_allocator<T>(mem_root)) {}

  mem_root_deque(mem_root_deque &&other) noexcept
      : list_(std::move(other.list_)) {
    other.list_.clear();
  }

  mem_root_deque(mem_root_deque &&other, MEM_ROOT *mem_root)
      : list_(std::move(other.list_), Mem_root_allocator<T>(mem_root)) {}

  mem_root_deque(std::initializer_list<T> init, MEM_ROOT *mem_root)
      : list_(std::move(init), Mem_root_allocator<T>(mem_root)) {}

  const_iterator begin() const { return list_.begin(); }

  const_iterator end() const { return list_.end(); }

  iterator begin() { return list_.begin(); }

  iterator end() { return list_.end(); }

  const_iterator cbegin() const { return list_.cbegin(); }

  const_iterator cend() const { return list_.cend(); }

  reverse_iterator rbegin() { return list_.rbegin(); }

  reverse_iterator rend() { return list_.rend(); }

  const_reverse_iterator rbegin() const { return list_.rbegin(); }

  const_reverse_iterator rend() const { return list_.rend(); }

  T &front() { return list_.front(); }

  const T &front() const { return list_.front(); }

  T &back() { return list_.back(); }

  const T &back() const { return list_.back(); }

  bool empty() const { return list_.empty(); }

  size_t size() const { return list_.size(); }

  bool push_back(const T &value) {
    list_.push_back(value);
    return false;
  }

  bool push_front(const T &value) {
    list_.push_front(value);
    return false;
  }

  void pop_back() {
    if (!list_.empty()) {
      list_.pop_back();
    }
  }

  void pop_front() {
    if (!list_.empty()) {
      list_.pop_front();
    }
  }

  void clear() { list_.clear(); }

  typename deque_container::iterator erase(
      typename deque_container::iterator position) {
    return list_.erase(position);
  }

  typename deque_container::iterator erase(
      typename deque_container::iterator first,
      typename deque_container::iterator last) {
    return list_.erase(first, last);
  }

  typename deque_container::iterator insert(
      typename deque_container::iterator position, const T &value) {
    auto it = list_.insert(position, value);
    return it;
  }

  typename deque_container::iterator insert(
      typename deque_container::iterator position, size_t count,
      const T &value) {
    auto it = list_.insert(position, count, value);
    return it;
  }

  template <class InputIt>
  typename deque_container::iterator insert(
      typename deque_container::const_iterator pos, InputIt first,
      InputIt last) {
    auto it = list_.insert(pos, first, last);
    return it;
  }

  typename deque_container::iterator find(const T &value) {
    return std::find(list_.begin(), list_.end(), value);
  }

  bool contains(const T &value) const {
    return std::find(list_.begin(), list_.end(), value) != list_.end();
  }

  void sort() { list_.sort(); }

  void reverse() { list_.reverse(); }

  void merge(mem_root_deque &other) { list_.merge(other.list_); }

  void splice(typename deque_container::iterator pos, mem_root_deque &other) {
    list_.splice(pos, other.list_);
  }

  void splice(typename deque_container::iterator pos, mem_root_deque &other,
              typename deque_container::iterator it) {
    list_.splice(pos, other.list_, it);
  }

  void unique() { list_.unique(); }

  void remove(const T &value) { list_.remove(value); }

  T &operator[](size_t index) {
    auto it = list_.begin();
    std::advance(it, index);  // Advance the iterator by index
    return *it;               // Return the element at the given index
  }

  const T &operator[](size_t index) const {
    auto it = list_.cbegin();
    std::advance(it, index);  // Advance the const iterator by index
    return *it;               // Return the element at the given index
  }

  mem_root_deque &operator=(mem_root_deque &&other) noexcept {
    if (this != &other) {
      list_ = std::move(other.list_);
      other.list_.clear();  // Clear the list in the source object
    }
    return *this;
  }

  mem_root_deque &operator=(const mem_root_deque &arg) = default;
};

#endif  // MEM_ROOT_DEQUE_H
