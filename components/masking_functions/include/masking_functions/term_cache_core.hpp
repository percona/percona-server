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

#ifndef MASKING_FUNCTIONS_TERM_CACHE_CORE_HPP
#define MASKING_FUNCTIONS_TERM_CACHE_CORE_HPP

#include "masking_functions/term_cache_core_fwd.hpp"  // IWYU pragma: export

#include <mutex>
#include <shared_mutex>
#include <string>
#include <string_view>

#include "masking_functions/abstract_sql_context_builder_fwd.hpp"
#include "masking_functions/bookshelf_fwd.hpp"
#include "masking_functions/charset_string_fwd.hpp"
#include "masking_functions/sql_context_fwd.hpp"

namespace masking_functions {
// 'term_cache_core' is a thread-safe class that incapsulates 'bookshelf'
// as an in-memory data structure that is used for caching dictionary terms.
// It is also aware of how to populate itself from a persisted storage
// ('masking_dictionaries' table) and how to reflect non-readonly operations
// in this persistent storage.
// Internally it uses 'abstract_sql_context_builder' as a way of establishing
// internal server session connections and executing SQL querues (constructed
// by query_builder interface).
// This class is used as a low-level abstraction ("core") of the higher-level
// user-facing class 'term_cache.
// All the methods of this class have additional 'sql_ctx_builder' parameter
// of type 'abstract_sql_context_builder' which comes handy when it is
// necessary to create different instances of the 'term_cache' class with the
// same 'term_cache_core' but with different 'abstract_sql_context_builder'.
class term_cache_core {
 public:
  term_cache_core();
  term_cache_core(const term_cache_core &other) = delete;
  term_cache_core(term_cache_core &&other) = delete;
  term_cache_core &operator=(const term_cache_core &other) = delete;
  term_cache_core &operator=(term_cache_core &&other) = delete;
  ~term_cache_core();

  bool contains(const abstract_sql_context_builder &sql_ctx_builder,
                const charset_string &dictionary_name,
                const charset_string &term) const;
  // returns a copy of the string to avoid race conditions
  // an empty string is returned if the dictionary does not exist
  charset_string get_random(const abstract_sql_context_builder &sql_ctx_builder,
                            const charset_string &dictionary_name) const;
  // returns true if there was at least one term in the 'dictionary_name'
  // dictionary
  // returns false if there was not a single term that belongs to the
  // 'dictionary_name' dictionary
  bool remove(const abstract_sql_context_builder &sql_ctx_builder,
              const charset_string &dictionary_name);
  // returns true if the term has been successfully removed from the
  // 'dictionary_name' dictionary
  // returns false if the term was not present in the 'dictionary_name'
  // dictionary
  bool remove(const abstract_sql_context_builder &sql_ctx_builder,
              const charset_string &dictionary_name,
              const charset_string &term);
  // returns true if the term has been successfully inserted into the
  // 'dictionary_name' dictionary
  // returns false if the term is alreaddy in the 'dictionary_name'
  // dictionary
  bool insert(const abstract_sql_context_builder &sql_ctx_builder,
              const charset_string &dictionary_name,
              const charset_string &term);

  void reload_cache(const abstract_sql_context_builder &sql_ctx_builder);

 private:
  mutable bookshelf_ptr dict_cache_;
  mutable std::shared_mutex dict_cache_mutex_;

  static bookshelf_ptr create_dict_cache_internal(sql_context &sql_ctx,
                                                  std::string &error_message);

  using shared_lock_type = std::shared_lock<std::shared_mutex>;
  using unique_lock_type = std::unique_lock<std::shared_mutex>;
  const bookshelf &acquire_dict_cache_shared(
      const abstract_sql_context_builder &sql_ctx_builder,
      sql_context_ptr &sql_ctx, shared_lock_type &read_lock,
      unique_lock_type &write_lock) const;
  bookshelf &acquire_dict_cache_unique(
      const abstract_sql_context_builder &sql_ctx_builder,
      sql_context_ptr &sql_ctx, unique_lock_type &write_lock) const;

  static const charset_string &to_utf8mb4(const charset_string &str,
                                          charset_string &buffer);
};

}  // namespace masking_functions

#endif  // MASKING_FUNCTIONS_TERM_CACHE_CORE_HPP
