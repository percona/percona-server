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
#include <string>

#include <mysql/components/services/psi_thread.h>

#include "masking_functions/bookshelf_fwd.hpp"
#include "masking_functions/dictionary_fwd.hpp"
#include "masking_functions/query_builder_fwd.hpp"

namespace masking_functions {

class query_cache {
 public:
  query_cache(query_builder_ptr query_builder,
              std::uint64_t flusher_interval_seconds);
  query_cache(const query_cache &other) = delete;
  query_cache(query_cache &&other) = delete;
  query_cache &operator=(const query_cache &other) = delete;
  query_cache &operator=(query_cache &&other) = delete;
  ~query_cache();

  bool contains(const std::string &dictionary_name,
                const std::string &term) const;
  optional_string get_random(const std::string &dictionary_name) const;
  bool remove(const std::string &dictionary_name);
  bool remove(const std::string &dictionary_name, const std::string &term);
  bool insert(const std::string &dictionary_name, const std::string &term);

  void reload_cache();

 private:
  query_builder_ptr m_query_builder;

  // TODO: in c++20 change this to std::atomic<bookshelf_ptr> and
  //       remove deprecated atomic_load() / atomic_store()
  mutable bookshelf_ptr m_dict_cache;

  std::uint64_t m_flusher_interval_seconds;
  std::atomic<bool> m_is_flusher_stopped;
  std::mutex m_flusher_mutex;
  std::condition_variable m_flusher_condition_var;

  PSI_thread_key m_psi_flusher_thread_key;
  my_thread_handle m_flusher_thread;
  my_thread_attr_t m_flusher_thread_attr;
  std::unique_ptr<THD> m_flusher_thd;

  void init_thd() noexcept;
  void release_thd() noexcept;
  void dict_flusher() noexcept;

  static void *run_dict_flusher(void *arg);

  bookshelf_ptr create_dict_cache_internal() const;
  // returning deliberately by value to increase reference counter
  bookshelf_ptr get_pinned_dict_cache_internal() const;
};

}  // namespace masking_functions

#endif  // MASKING_FUNCTIONS_QUERY_CACHE_HPP
