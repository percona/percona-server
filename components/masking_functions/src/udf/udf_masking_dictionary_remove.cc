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

#include "components/masking_functions/include/udf/udf_masking_dictionary_remove.h"
#include <mysql/components/services/mysql_command_services.h>
#include <mysql/components/services/mysql_current_thread_reader.h>
#include <mysql/components/services/security_context.h>
#include "components/masking_functions/include/component.h"
#include "components/masking_functions/include/udf/udf_utils.h"
#include "components/masking_functions/include/udf/udf_utils_sql.h"
#include "components/masking_functions/include/udf/udf_utils_string.h"

static bool masking_dictionary_remove_init(UDF_INIT *initid, UDF_ARGS *args,
                                           char *message) {
  DBUG_TRACE;

  if (!mysql::components::have_masking_admin_privilege()) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "masking_dictionary_remove requires the "
                  "MASKING_DICTIONARIES_ADMIN privilege");
    return true;
  }

  if (args->arg_count != 1) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument list: masking_dictionary_remove(dictionary "
                  "name)");
    return true;
  }

  if (args->arg_type[0] != STRING_RESULT) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument type: masking_dictionary_remove(string)");
    return true;
  }

  if (mysql::plugins::set_return_value_charset_to_match_arg(initid, args, 0)) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Unable to set character set service for UDF");
    return true;
  }

  initid->maybe_null = 1;
  initid->const_item =
      0;  // Non-Deterministic: same arguments will produce different values
  initid->ptr = nullptr;

  return false;
}

static void masking_dictionary_remove_deinit(UDF_INIT *initid) {
  DBUG_TRACE;

  if (initid->ptr) delete[] initid->ptr;
}

static char *masking_dictionary_remove(UDF_INIT *initid, UDF_ARGS *args, char *,
                                       unsigned long *length, char *is_null,
                                       char *) {
  DBUG_TRACE;

  std::string original_charset;
  mysql::plugins::get_arg_character_set(args, 0, original_charset);
  const std::string dictionary = mysql::plugins::convert(
      {args->args[0], args->lengths[0]}, original_charset, mysql::plugins::default_charset);

  mysql::components::sql_context sqlc;

  std::string query(
      "DELETE FROM mysql.masking_dictionaries WHERE Dictionary=\"");

  mysql::components::escape_string_into(query, dictionary);
  query += "\"";

  if (sqlc.execute(query) == 0) {
    *is_null = 1;
    return nullptr;
  }

  initid->ptr = new char[2];
  memcpy(initid->ptr, "1", 2);
  *length = 1;

  return initid->ptr;
}

udf_descriptor udf_masking_dictionary_remove() {
  return {"masking_dictionary_remove", Item_result::STRING_RESULT,
          reinterpret_cast<Udf_func_any>(masking_dictionary_remove),
          masking_dictionary_remove_init, masking_dictionary_remove_deinit};
}
