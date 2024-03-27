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
#include "masking_functions/sql_context.hpp"

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
    std::unique_lock dict_write_lock{m_dict_mut};
    m_dict_cache = std::move(result.value());
  }

  return result.has_value();
}

bool query_cache::contains(const std::string &dictionary_name,
                           const std::string &term) const {
  std::shared_lock dict_read_lock{m_dict_mut};
  return m_dict_cache.contains(dictionary_name, term);
}

optional_string query_cache::get(const std::string &dictionary_name) const {
  std::shared_lock dict_read_lock{m_dict_mut};
  return m_dict_cache.get(dictionary_name);
}

bool query_cache::remove(const std::string &dictionary_name) {
  std::unique_lock dict_write_lock{m_dict_mut};

  masking_functions::sql_context sql_ctx{global_command_services::instance()};
  auto query =
      global_query_builder::instance().delete_for_dictionary(dictionary_name);

  if (!sql_ctx.execute(query)) {
    return false;
  }

  return m_dict_cache.remove(dictionary_name);
}

bool query_cache::remove(const std::string &dictionary_name,
                         const std::string &term) {
  std::shared_lock dict_read_lock{m_dict_mut};

  masking_functions::sql_context sql_ctx{global_command_services::instance()};
  auto query = global_query_builder::instance().delete_for_dictionary_and_term(
      dictionary_name, term);

  if (!sql_ctx.execute(query)) {
    return false;
  }

  return m_dict_cache.remove(dictionary_name, term);
}

bool query_cache::insert(const std::string &dictionary_name,
                         const std::string &term) {
  std::unique_lock dict_write_lock{m_dict_mut};

  masking_functions::sql_context sql_ctx{global_command_services::instance()};
  auto query = global_query_builder::instance().insert_ignore_record(
      dictionary_name, term);

  if (!sql_ctx.execute(query)) {
    return false;
  }

  return m_dict_cache.insert(dictionary_name, term);
}

}  // namespace masking_functions
