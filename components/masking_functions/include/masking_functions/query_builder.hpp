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

#ifndef MASKING_FUNCTIONS_QUERY_BUILDER_HPP
#define MASKING_FUNCTIONS_QUERY_BUILDER_HPP

#include "masking_functions/query_builder_fwd.hpp"

#include <string>
#include <string_view>

namespace masking_functions {

// A helper class which allows to easily construct SQL-statements necessary
// for data-masking dictionary manipulation.
class query_builder {
 public:
  static constexpr std::string_view default_result_character_set = "utf8mb4";

  static constexpr std::string_view default_table_name = "masking_dictionaries";
  static constexpr std::string_view default_dictionary_field_name =
      "Dictionary";
  static constexpr std::string_view default_term_field_name = "Term";

  explicit query_builder(
      std::string_view database_name,
      std::string_view table_name = default_table_name,
      std::string_view dictionary_field_name = default_dictionary_field_name,
      std::string_view term_field_name = default_term_field_name)
      : database_name_{database_name},
        table_name_{table_name},
        dictionary_field_name_{dictionary_field_name},
        term_field_name_{term_field_name} {}

  const std::string &get_database_name() const noexcept {
    return database_name_;
  }
  const std::string &get_table_name() const noexcept { return table_name_; }
  const std::string &get_dictionary_field_name() const noexcept {
    return dictionary_field_name_;
  }
  const std::string &get_term_field_name() const noexcept {
    return term_field_name_;
  }

  std::string select_all_from_dictionary() const;

  std::string insert_ignore_record(const std::string &dictionary_name,
                                   const std::string &term) const;

  std::string delete_for_dictionary(const std::string &dictionary_name) const {
    return delete_for_dictionary_and_opt_term_internal(dictionary_name,
                                                       nullptr);
  }

  std::string delete_for_dictionary_and_term(const std::string &dictionary_name,
                                             const std::string &term) const {
    return delete_for_dictionary_and_opt_term_internal(dictionary_name, &term);
  }

 private:
  std::string database_name_;
  std::string table_name_;
  std::string dictionary_field_name_;
  std::string term_field_name_;

  std::string delete_for_dictionary_and_opt_term_internal(
      const std::string &dictionary_name, const std::string *opt_term) const;
};

}  // namespace masking_functions

#endif  // MASKING_FUNCTIONS_QUERY_BUILDER_HPP
