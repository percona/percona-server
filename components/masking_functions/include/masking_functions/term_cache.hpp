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

#ifndef MASKING_FUNCTIONS_TERM_CACHE_HPP
#define MASKING_FUNCTIONS_TERM_CACHE_HPP

#include "masking_functions/term_cache_fwd.hpp"  // IWYU pragma: export

#include "masking_functions/abstract_sql_context_builder_fwd.hpp"
#include "masking_functions/charset_string_fwd.hpp"
#include "masking_functions/term_cache_core_fwd.hpp"

namespace masking_functions {

// 'term_cache' class allows to have a convenient wrapper over a combination
// of 'term_cache_core' and 'abstract_sql_context_builder'.
// Its 'contains()' / 'get_random()' / 'remove()' / 'insert()' methods in
// contrast to 'term_cache_core' do not have additional 'sql_ctx_builder'
// parameter of type 'abstract_sql_context_builder'.
// Basically, for the needs of data_masking component we can have a single
// instance of 'term_cache_core' and two instances of 'term_cache':
// 1. One for the dictionary-related UDFs (constructed with
//    'default_sql_context_builder' that established new connection every
//    time we need to access persistent storage - 'masking_dictionaries'
//    table).
// 2. One for the background reloading thread (constructed with
//    'static_sql_context_builder' that always reuses the same connection it
//    established in the constructor).
class term_cache {
 public:
  term_cache(const term_cache_core_ptr &core,
             const abstract_sql_context_builder_ptr &sql_ctx_builder);

  term_cache(const term_cache &other) = delete;
  term_cache(term_cache &&other) = delete;
  term_cache &operator=(const term_cache &other) = delete;
  term_cache &operator=(term_cache &&other) = delete;
  ~term_cache();

  bool contains(const charset_string &dictionary_name,
                const charset_string &term) const;
  // returns a copy of the string to avoid race conditions
  // an empty string is returned if the dictionary does not exist
  charset_string get_random(const charset_string &dictionary_name) const;
  // returns true if there was at least one term in the 'dictionary_name'
  // dictionary
  // returns false if there was not a single term that belongs to the
  // 'dictionary_name' dictionary
  bool remove(const charset_string &dictionary_name);
  // returns true if the term has been successfully removed from the
  // 'dictionary_name' dictionary
  // returns false if the term was not present in the 'dictionary_name'
  // dictionary
  bool remove(const charset_string &dictionary_name,
              const charset_string &term);
  // returns true if the term has been successfully inserted into the
  // 'dictionary_name' dictionary
  // returns false if the term is alreaddy in the 'dictionary_name'
  // dictionary
  bool insert(const charset_string &dictionary_name,
              const charset_string &term);

  void reload_cache();

 private:
  term_cache_core_ptr core_;
  abstract_sql_context_builder_ptr sql_ctx_builder_;
};

}  // namespace masking_functions

#endif  // MASKING_FUNCTIONS_TERM_CACHE_HPP
