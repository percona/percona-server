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

#include <map>
#include <string>

#include "masking_functions/sql_context.hpp"

namespace masking_functions {

class query_cache {
 public:
  query_cache();

  bool check_term_presence_in_dictionary(const std::string &dictionary_name,
                                         const std::string &term) const;

  sql_context::optional_string select_random_term_for_dictionary(
      const std::string &dictionary_name) const;

  bool delete_for_dictionary(const std::string &dictionary_name);

  bool delete_for_dictionary_and_term(const std::string &dictionary_name,
                                      const std::string &term);

  bool insert_ignore_record(const std::string &dictionary_name,
                            const std::string &term);
  bool load_cache();

 private:
  sql_context::dict_container_type m_dict_cache;
};

}  // namespace masking_functions

#endif  // MASKING_FUNCTIONS_QUERY_CACHE_HPP
