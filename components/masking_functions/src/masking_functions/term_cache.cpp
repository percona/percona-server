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

#include "masking_functions/term_cache.hpp"

#include "masking_functions/abstract_sql_context_builder_fwd.hpp"
#include "masking_functions/charset_string.hpp"
#include "masking_functions/term_cache_core.hpp"

namespace masking_functions {

term_cache::term_cache(const term_cache_core_ptr &core,
                       const abstract_sql_context_builder_ptr &sql_ctx_builder)
    : core_{core}, sql_ctx_builder_{sql_ctx_builder} {}

term_cache::~term_cache() = default;

bool term_cache::contains(const charset_string &dictionary_name,
                          const charset_string &term) const {
  return core_->contains(*sql_ctx_builder_, dictionary_name, term);
}

charset_string term_cache::get_random(
    const charset_string &dictionary_name) const {
  return core_->get_random(*sql_ctx_builder_, dictionary_name);
}

bool term_cache::remove(const charset_string &dictionary_name) {
  return core_->remove(*sql_ctx_builder_, dictionary_name);
}

bool term_cache::remove(const charset_string &dictionary_name,
                        const charset_string &term) {
  return core_->remove(*sql_ctx_builder_, dictionary_name, term);
}

bool term_cache::insert(const charset_string &dictionary_name,
                        const charset_string &term) {
  return core_->insert(*sql_ctx_builder_, dictionary_name, term);
}

void term_cache::reload_cache() { core_->reload_cache(*sql_ctx_builder_); }

}  // namespace masking_functions
