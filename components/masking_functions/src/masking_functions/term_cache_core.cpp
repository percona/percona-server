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

#include "masking_functions/term_cache_core.hpp"

#include <exception>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include "masking_functions/abstract_sql_context_builder.hpp"
#include "masking_functions/bookshelf.hpp"
#include "masking_functions/charset_string.hpp"
#include "masking_functions/primitive_singleton.hpp"
#include "masking_functions/query_builder.hpp"
#include "masking_functions/sql_context.hpp"
#include "masking_functions/string_service_tuple.hpp"

namespace masking_functions {

using global_query_builder = masking_functions::primitive_singleton<
    masking_functions::query_builder_ptr>;

term_cache_core::term_cache_core() = default;

term_cache_core::~term_cache_core() = default;

bool term_cache_core::contains(
    const abstract_sql_context_builder &sql_ctx_builder,
    const charset_string &dictionary_name, const charset_string &term) const {
  sql_context_ptr sql_ctx;
  shared_lock_type read_lock{};
  unique_lock_type write_lock{};
  const auto &acquired_dict_cache{acquire_dict_cache_shared(
      sql_ctx_builder, sql_ctx, read_lock, write_lock)};

  charset_string dictionary_name_buffer;
  const auto &utf8mb4_dictionary_name{
      to_utf8mb4(dictionary_name, dictionary_name_buffer)};
  charset_string term_buffer;
  const auto &utf8mb4_term{to_utf8mb4(term, term_buffer)};
  return acquired_dict_cache.contains(utf8mb4_dictionary_name, utf8mb4_term);
}

charset_string term_cache_core::get_random(
    const abstract_sql_context_builder &sql_ctx_builder,
    const charset_string &dictionary_name) const {
  sql_context_ptr sql_ctx;
  shared_lock_type read_lock{};
  unique_lock_type write_lock{};
  const auto &acquired_dict_cache{acquire_dict_cache_shared(
      sql_ctx_builder, sql_ctx, read_lock, write_lock)};

  charset_string dictionary_name_buffer;
  const auto &utf8mb4_dictionary_name{
      to_utf8mb4(dictionary_name, dictionary_name_buffer)};
  return acquired_dict_cache.get_random(utf8mb4_dictionary_name);
}

bool term_cache_core::remove(
    const abstract_sql_context_builder &sql_ctx_builder,
    const charset_string &dictionary_name) {
  sql_context_ptr sql_ctx;
  auto query{
      global_query_builder::instance()->delete_for_dictionary(dictionary_name)};

  unique_lock_type write_lock{};
  auto &acquired_dict_cache{
      acquire_dict_cache_unique(sql_ctx_builder, sql_ctx, write_lock)};

  if (!sql_ctx) {
    sql_ctx = sql_ctx_builder.build();
  }

  // there is a chance that a user can delete the dictionary from the
  // dictionary table directly (not via UDF function) and execute_dml()
  // will return false here, whereas cache operation will return true -
  // this is why we rely only on the result of the cache operation
  sql_ctx->execute_dml(query);
  charset_string dictionary_name_buffer;
  const auto &utf8mb4_dictionary_name{
      to_utf8mb4(dictionary_name, dictionary_name_buffer)};
  return acquired_dict_cache.remove(utf8mb4_dictionary_name);
}

bool term_cache_core::remove(
    const abstract_sql_context_builder &sql_ctx_builder,
    const charset_string &dictionary_name, const charset_string &term) {
  sql_context_ptr sql_ctx;
  auto query{global_query_builder::instance()->delete_for_dictionary_and_term(
      dictionary_name, term)};

  unique_lock_type write_lock{};
  auto &acquired_dict_cache{
      acquire_dict_cache_unique(sql_ctx_builder, sql_ctx, write_lock)};

  if (!sql_ctx) {
    sql_ctx = sql_ctx_builder.build();
  }

  // similarly to another remove() method, we ignore the result of the
  // sql operation and rely only on the result of the cache modification
  sql_ctx->execute_dml(query);
  charset_string dictionary_name_buffer;
  const auto &utf8mb4_dictionary_name{
      to_utf8mb4(dictionary_name, dictionary_name_buffer)};
  charset_string term_buffer;
  const auto &utf8mb4_term{to_utf8mb4(term, term_buffer)};
  return acquired_dict_cache.remove(utf8mb4_dictionary_name, utf8mb4_term);
}

bool term_cache_core::insert(
    const abstract_sql_context_builder &sql_ctx_builder,
    const charset_string &dictionary_name, const charset_string &term) {
  sql_context_ptr sql_ctx;
  auto query{global_query_builder::instance()->insert_ignore_record(
      dictionary_name, term)};

  unique_lock_type write_lock{};
  auto &acquired_dict_cache{
      acquire_dict_cache_unique(sql_ctx_builder, sql_ctx, write_lock)};

  if (!sql_ctx) {
    sql_ctx = sql_ctx_builder.build();
  }

  // here, as cache insert may throw, we start the 2-phase operation
  // with this cache insert because it can be easily reversed without throwing
  charset_string dictionary_name_buffer;
  const auto &utf8mb4_dictionary_name{
      to_utf8mb4(dictionary_name, dictionary_name_buffer)};
  charset_string term_buffer;
  const auto &utf8mb4_term{to_utf8mb4(term, term_buffer)};
  const auto result{
      acquired_dict_cache.insert(utf8mb4_dictionary_name, utf8mb4_term)};
  try {
    sql_ctx->execute_dml(query);
  } catch (...) {
    dict_cache_->remove(utf8mb4_dictionary_name, utf8mb4_term);
    throw;
  }

  return result;
}

void term_cache_core::reload_cache(
    const abstract_sql_context_builder &sql_ctx_builder) {
  unique_lock_type dict_cache_write_lock{dict_cache_mutex_};

  std::string error_message;
  auto sql_ctx{sql_ctx_builder.build()};
  auto local_dict_cache{create_dict_cache_internal(*sql_ctx, error_message)};
  if (!local_dict_cache) {
    throw std::runtime_error{error_message};
  }

  dict_cache_ = std::move(local_dict_cache);
}

bookshelf_ptr term_cache_core::create_dict_cache_internal(
    sql_context &sql_ctx, std::string &error_message) {
  bookshelf_ptr result;
  error_message.clear();
  try {
    auto query{global_query_builder::instance()->select_all_from_dictionary()};
    auto local_dict_cache{std::make_unique<bookshelf>()};
    sql_context::row_callback<2> result_inserter{[&terms = *local_dict_cache](
                                                     const auto &field_values) {
      const auto &string_services{
          primitive_singleton<string_service_tuple>::instance()};
      charset_string dictionary_name{string_services, field_values[0],
                                     charset_string::utf8mb4_collation_name};
      charset_string term{string_services, field_values[1],
                          charset_string::utf8mb4_collation_name};
      terms.insert(dictionary_name, term);
    }};
    sql_ctx.execute_select(query, result_inserter);
    result = std::move(local_dict_cache);
  } catch (const std::exception &e) {
    error_message = e.what();
  } catch (...) {
    error_message =
        "unexpected exception caught while loading dictionary cache";
  }

  return result;
}

const bookshelf &term_cache_core::acquire_dict_cache_shared(
    const abstract_sql_context_builder &sql_ctx_builder,
    sql_context_ptr &sql_ctx, shared_lock_type &read_lock,
    unique_lock_type &write_lock) const {
  read_lock = shared_lock_type{dict_cache_mutex_};
  if (!dict_cache_) {
    // upgrading to a unique_lock
    read_lock.unlock();
    acquire_dict_cache_unique(sql_ctx_builder, sql_ctx, write_lock);
  }
  return *dict_cache_;
}

bookshelf &term_cache_core::acquire_dict_cache_unique(
    const abstract_sql_context_builder &sql_ctx_builder,
    sql_context_ptr &sql_ctx, unique_lock_type &write_lock) const {
  write_lock = unique_lock_type{dict_cache_mutex_};
  if (!dict_cache_) {
    std::string error_message;
    sql_ctx = sql_ctx_builder.build();
    auto local_dict_cache{create_dict_cache_internal(*sql_ctx, error_message)};
    if (!local_dict_cache) {
      throw std::runtime_error{error_message};
    }
    dict_cache_ = std::move(local_dict_cache);
  }
  return *dict_cache_;
}

const charset_string &term_cache_core::to_utf8mb4(const charset_string &str,
                                                  charset_string &buffer) {
  return smart_convert_to_collation(str, charset_string::utf8mb4_collation_name,
                                    buffer);
}

}  // namespace masking_functions
