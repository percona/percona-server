/* Copyright (c) 2023 Percona LLC and/or its affiliates. All rights reserved.

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

#include <sstream>

#include "masking_functions/query_builder.hpp"

#include "masking_functions/sql_escape_functions.hpp"

namespace masking_functions {

std::string query_builder::insert_ignore_record(
    const charset_string &dictionary_name, const charset_string &term) const {
  std::ostringstream oss;
  oss << "INSERT IGNORE INTO " << get_database_name() << '.' << get_table_name()
      << " (" << get_dictionary_field_name() << ", " << get_term_field_name()
      << ')' << " VALUES('" << escape_string(dictionary_name) << "', '"
      << escape_string(term) << "')";
  return oss.str();
}

std::string query_builder::select_term_for_dictionary_internal(
    const charset_string &dictionary_name,
    const charset_string *opt_term) const {
  std::ostringstream oss;
  // In our implementation there is no requirement that the `Term` field in
  // the `mysql.masking_dictionaries` table must be in `utf8mb4`. So, by
  // adding CONVERT(Term USING utf8mb4) we support other character sets in
  // the underlying table as well.
  oss << "SELECT "
      << "CONVERT(" << get_term_field_name() << " USING "
      << default_result_character_set << ") FROM " << get_database_name() << '.'
      << get_table_name() << " WHERE " << get_dictionary_field_name() << " = '"
      << escape_string(dictionary_name) << '\'';
  if (opt_term != nullptr) {
    oss << " AND " << get_term_field_name() << " = '"
        << escape_string(*opt_term) << '\'';
  } else {
    oss << " ORDER BY RAND() LIMIT 1";
  }
  return oss.str();
}

std::string query_builder::delete_for_dictionary_and_opt_term_internal(
    const charset_string &dictionary_name,
    const charset_string *opt_term) const {
  std::ostringstream oss;
  oss << "DELETE FROM " << get_database_name() << '.' << get_table_name()
      << " WHERE " << get_dictionary_field_name() << " = '"
      << escape_string(dictionary_name) << '\'';
  if (opt_term != nullptr) {
    oss << " AND " << get_term_field_name() << " = '"
        << escape_string(*opt_term) << '\'';
  }
  return oss.str();
}

}  // namespace masking_functions
