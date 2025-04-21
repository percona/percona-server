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

#include <list>
#include <algorithm>
#include <assert.h>

#include "sql/mem_root_allocator.h"

/**
 * We use list instead of deque for memory efficiency reasons,
 * primarily to optimize operations like bulk insert.
 */
template <class T>
class mem_root_deque {
private:
    std::list<T, Mem_root_allocator<T>> list_;
    size_t size_;

public:
    using iterator = typename std::list<T, Mem_root_allocator<T>>::iterator;
    using const_iterator = typename std::list<T, Mem_root_allocator<T>>::const_iterator;
    using reverse_iterator = typename std::list<T, Mem_root_allocator<T>>::reverse_iterator;
    using const_reverse_iterator = typename std::list<T, Mem_root_allocator<T>>::const_reverse_iterator;


    explicit mem_root_deque(MEM_ROOT *mem_root)
        : list_(Mem_root_allocator<T>(mem_root)), size_(0) {}

    mem_root_deque(typename std::list<T, Mem_root_allocator<T>>::size_type count, const T &value, MEM_ROOT *mem_root)
        : list_(count, value, Mem_root_allocator<T>(mem_root)), size_(count) {}

    mem_root_deque(typename std::list<T, Mem_root_allocator<T>>::size_type count, MEM_ROOT *mem_root)
        : list_(count, Mem_root_allocator<T>(mem_root)), size_(count) {}

    template <class InputIt>
    mem_root_deque(InputIt first, InputIt last, MEM_ROOT *mem_root)
        : list_(first, last, Mem_root_allocator<T>(mem_root)), size_(std::distance(first, last)) {}

    mem_root_deque(const mem_root_deque &other)
        : list_(other.list_), size_(other.size_) {}

    mem_root_deque(const mem_root_deque &other, MEM_ROOT *mem_root)
        : list_(other.list_, Mem_root_allocator<T>(mem_root)), size_(other.size_) {}

    mem_root_deque(mem_root_deque &&other)
        : list_(std::move(other.list_)), size_(other.size_) {}

    mem_root_deque(mem_root_deque &&other, MEM_ROOT *mem_root)
        : list_(std::move(other.list_), Mem_root_allocator<T>(mem_root)), size_(other.size_) {}

    mem_root_deque(std::initializer_list<T> init, MEM_ROOT *mem_root)
      : list_(std::move(init), Mem_root_allocator<T>(mem_root)), size_(init.size()) {}

    inline const_iterator begin() const {
      return list_.begin();
    }

    inline const_iterator end() const {
      return list_.end();
    }

    inline iterator begin() {
      return list_.begin();
    }

    inline iterator end() {
      return list_.end();
    }

    inline const_iterator cbegin() const {
        return list_.cbegin();
    }

    inline const_iterator cend() const {
        return list_.cend();
    }

    inline reverse_iterator rbegin() {
        return list_.rbegin();
    }

    inline reverse_iterator rend() {
        return list_.rend();
    }

    inline const_reverse_iterator rbegin() const {
        return list_.rbegin();
    }

    inline const_reverse_iterator rend() const {
        return list_.rend();
    }

    inline T& front() {
      return list_.front();
    }

    inline const T& front() const {
      return list_.front();
    }

    inline T& back() {
      return list_.back();
    }

    inline const T& back() const {
      return list_.back();
    }

    inline bool empty() const {
      return size_ == 0;
    }

    inline size_t size() const {
      assert(list_.size() == size_);
      return size_;
    }

    inline void push_back(const T &value) {
        list_.push_back(value);
        ++size_;
    }

    inline void push_front(const T &value) {
        list_.push_front(value);
        ++size_;
    }

    inline void pop_back() {
        if (!list_.empty()) {
            list_.pop_back();
            --size_;
        }
    }

    inline void pop_front() {
        if (!list_.empty()) {
            list_.pop_front();
            --size_;
        }
    }

    inline void clear() {
        list_.clear();
        size_ = 0;
    }

    inline typename std::list<T, Mem_root_allocator<T>>::iterator
      erase(typename std::list<T, Mem_root_allocator<T>>::iterator position) {
        --size_;
        return list_.erase(position);
    }

    inline typename std::list<T, Mem_root_allocator<T>>::iterator
      erase(typename std::list<T, Mem_root_allocator<T>>::iterator first,
        typename std::list<T, Mem_root_allocator<T>>::iterator last) {
        size_ -= std::distance(first, last);
        return list_.erase(first, last);
    }

    inline typename std::list<T, Mem_root_allocator<T>>::iterator
      insert(typename std::list<T, Mem_root_allocator<T>>::iterator position, const T &value) {
        auto it = list_.insert(position, value);
        ++size_;
        return it;
    }

    inline typename std::list<T, Mem_root_allocator<T>>::iterator
      insert(typename std::list<T, Mem_root_allocator<T>>::iterator position, size_t count, const T &value) {
        auto it = list_.insert(position, count, value);
        size_ += count;
        return it;
    }

    template <class InputIt>
    inline typename std::list<T, Mem_root_allocator<T>>::iterator
    insert(typename std::list<T, Mem_root_allocator<T>>::const_iterator pos, InputIt first, InputIt last) {
        auto it = list_.insert(pos, first, last);
        size_ += std::distance(first, last);
        return it;
    }

    inline typename std::list<T, Mem_root_allocator<T>>::iterator find(const T &value) {
        return std::find(list_.begin(), list_.end(), value);
    }

    inline bool contains(const T &value) const {
        return std::find(list_.begin(), list_.end(), value) != list_.end();
    }

    inline void sort() {
        list_.sort();
    }

    inline void reverse() {
        list_.reverse();
    }

    inline void merge(mem_root_deque &other) {
        list_.merge(other.list_);
        size_ += other.size_;
        other.size_ = 0;
    }

    inline void splice(typename std::list<T, Mem_root_allocator<T>>::iterator pos, mem_root_deque &other) {
        list_.splice(pos, other.list_);
        size_ += other.size_;
        other.size_ = 0;
    }

    inline void splice(typename std::list<T, Mem_root_allocator<T>>::iterator pos,
        mem_root_deque &other, typename std::list<T, Mem_root_allocator<T>>::iterator it) {
        list_.splice(pos, other.list_, it);
        --other.size_;
        ++size_;
    }

    inline void unique() {
        list_.unique();
    }

    inline void remove(const T &value) {
        size_ -= std::count(list_.begin(), list_.end(), value);
        list_.remove(value);
    }

    inline T& operator[](size_t index) {
      auto it = list_.begin();
      std::advance(it, index);  // Advance the iterator by index
      return *it;  // Return the element at the given index
    }

    inline const T& operator[](size_t index) const {
      auto it = list_.cbegin();
      std::advance(it, index);  // Advance the const iterator by index
      return *it;  // Return the element at the given index
    }

    inline mem_root_deque &operator=(const mem_root_deque &arg) = default;
};

#endif  // MEM_ROOT_DEQUE_H

