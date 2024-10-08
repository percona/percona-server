/* Copyright (c) 2024 Percona LLC and/or its affiliates. All rights reserved.

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

#ifndef MASKING_FUNCTIONS_QUERY_CACHE_HPP
#define MASKING_FUNCTIONS_QUERY_CACHE_HPP

#include "masking_functions/query_cache_fwd.hpp"

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>

#include <mysql/components/services/psi_thread.h>

#include "masking_functions/bookshelf_fwd.hpp"
#include "masking_functions/dictionary_fwd.hpp"
#include "masking_functions/query_builder_fwd.hpp"

namespace masking_functions {

class query_cache {
 public:
  // passing unique_ptr by value to transfer ownership
  query_cache(query_builder_ptr query_builder,
              std::uint64_t flusher_interval_seconds);
  query_cache(const query_cache &other) = delete;
  query_cache(query_cache &&other) = delete;
  query_cache &operator=(const query_cache &other) = delete;
  query_cache &operator=(query_cache &&other) = delete;
  ~query_cache();

  bool contains(const std::string &dictionary_name,
                const std::string &term) const;
  // returns a copy of the string to avoid race conditions
  // an empty string is returned if the dictionary does not exist
  std::string get_random(const std::string &dictionary_name) const;
  bool remove(const std::string &dictionary_name);
  bool remove(const std::string &dictionary_name, const std::string &term);
  bool insert(const std::string &dictionary_name, const std::string &term);

  void reload_cache();

 private:
  query_builder_ptr dict_query_builder_;

  mutable bookshelf_ptr dict_cache_;
  mutable std::shared_mutex dict_cache_mutex_;

  std::uint64_t flusher_interval_seconds_;
  std::atomic<bool> is_flusher_stopped_;
  std::mutex flusher_mutex_;
  std::condition_variable flusher_condition_var_;

  PSI_thread_key psi_flusher_thread_key_;
  my_thread_handle flusher_thread_;
  my_thread_attr_t flusher_thread_attr_;
  std::unique_ptr<THD> flusher_thd_;

  void init_thd() noexcept;
  void release_thd() noexcept;
  void dict_flusher() noexcept;

  static void *run_dict_flusher(void *arg);

  bookshelf_ptr create_dict_cache_internal() const;
  using shared_lock_type = std::shared_lock<std::shared_mutex>;
  using unique_lock_type = std::unique_lock<std::shared_mutex>;
  const bookshelf &acquire_dict_cache_shared(
      shared_lock_type &read_lock, unique_lock_type &write_lock) const;
  bookshelf &acquire_dict_cache_unique(unique_lock_type &write_lock) const;
};

}  // namespace masking_functions

#endif  // MASKING_FUNCTIONS_QUERY_CACHE_HPP
