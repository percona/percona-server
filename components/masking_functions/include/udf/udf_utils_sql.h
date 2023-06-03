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

#ifndef _UTILS_SQL_H
#define _UTILS_SQL_H

#include <mysql/components/services/mysql_command_services.h>
#include <optional>
#include <stdexcept>
#include <string>

namespace mysql::components {

class sql_context_exception : public std::runtime_error {
  using runtime_error::runtime_error;
};

class sql_context {
 public:
  sql_context();
  ~sql_context();

  // Executes a query where we either expect a single result (one row one
  // column), or nothing
  std::optional<std::string> query_single_value(std::string const &query);

  bool execute(std::string const &query);

 private:
  MYSQL_H mysql_h = nullptr;
};

void escape_string_into(std::string &into, std::string const &str);
bool have_masking_admin_privilege();

}  // namespace mysql::components

#endif
