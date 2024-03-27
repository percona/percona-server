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

#include "masking_functions/query_cache.hpp"

#include "masking_functions/command_service_tuple.hpp"
#include "masking_functions/primitive_singleton.hpp"
#include "masking_functions/query_builder.hpp"

#include <algorithm>

namespace masking_functions {
namespace {

using global_command_services = masking_functions::primitive_singleton<
    masking_functions::command_service_tuple>;
using global_query_builder =
    masking_functions::primitive_singleton<masking_functions::query_builder>;

}  // namespace

query_cache::query_cache() { load_cache(); }

bool query_cache::load_cache() {
  auto query = global_query_builder::instance().select_all_from_dictionary();
  auto result =
      masking_functions::sql_context{global_command_services::instance()}
          .query_list(query);

  if (result.has_value()) {
    m_dict_cache.swap(result.value());
  }

  return result.has_value();
}

bool query_cache::check_term_presence_in_dictionary(
    const std::string &dictionary_name, const std::string &term) const {
  masking_functions::sql_context sql_ctx{global_command_services::instance()};

  auto range = m_dict_cache.equal_range(dictionary_name);

  if (range.first == range.second) {
    return false;
  }

  if (term.length() == 0) {
    return true;
  }

  return std::find_if(range.first, range.second, [term](const auto &el) {
           return el.second == term;
         }) != range.second;
}

sql_context::optional_string query_cache::select_random_term_for_dictionary(
    const std::string &dictionary_name) const {
  auto range = m_dict_cache.equal_range(dictionary_name);

  if (range.first == range.second) {
    return std::nullopt;
  }

  int random_step = rand() % std::distance(range.first, range.second);
  std::advance(range.first, random_step);

  return sql_context::optional_string{std::in_place, range.first->second};
}

bool query_cache::delete_for_dictionary(const std::string &dictionary_name) {
  masking_functions::sql_context sql_ctx{global_command_services::instance()};
  auto query =
      global_query_builder::instance().delete_for_dictionary(dictionary_name);

  if (!sql_ctx.execute(query)) {
    return false;
  }

  m_dict_cache.erase(dictionary_name);
  return true;
}

bool query_cache::delete_for_dictionary_and_term(
    const std::string &dictionary_name, const std::string &term) {
  masking_functions::sql_context sql_ctx{global_command_services::instance()};
  auto query = global_query_builder::instance().delete_for_dictionary_and_term(
      dictionary_name, term);

  if (!sql_ctx.execute(query)) {
    return false;
  }

  auto range = m_dict_cache.equal_range(dictionary_name);

  if (range.first != range.second) {
    auto it = std::find_if(range.first, range.second, [term](const auto &el) {
      return el.second == term;
    });
    if (it != range.second) {
      m_dict_cache.erase(it);
    }
  }

  return true;
}

bool query_cache::insert_ignore_record(const std::string &dictionary_name,
                                       const std::string &term) {
  masking_functions::sql_context sql_ctx{global_command_services::instance()};
  auto query = global_query_builder::instance().insert_ignore_record(
      dictionary_name, term);

  if (!sql_ctx.execute(query)) {
    return false;
  }

  m_dict_cache.insert({dictionary_name, term});
  return true;
}

}  // namespace masking_functions
