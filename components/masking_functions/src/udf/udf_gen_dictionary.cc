/* Copyright (c) 2018, 2019 Francisco Miguel Biete Banon. All rights reserved.
   Copyright (c) 2023 Percona LLC and/or its affiliates. All rights reserved.

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

#include "components/masking_functions/include/udf/udf_gen_dictionary.h"
#include <mysql/components/services/mysql_command_services.h>
#include <mysql/components/services/mysql_current_thread_reader.h>
#include <mysql/components/services/security_context.h>
#include "components/masking_functions/include/component.h"
#include "components/masking_functions/include/udf/udf_utils.h"
#include "components/masking_functions/include/udf/udf_utils_sql.h"
#include "components/masking_functions/include/udf/udf_utils_string.h"

extern REQUIRES_SERVICE_PLACEHOLDER_AS(mysql_command_options, cmd_options_srv);
extern REQUIRES_SERVICE_PLACEHOLDER_AS(mysql_command_factory, cmd_factory_srv);
extern REQUIRES_SERVICE_PLACEHOLDER_AS(mysql_command_query, cmd_query_srv);
extern REQUIRES_SERVICE_PLACEHOLDER_AS(mysql_command_error_info,
                                       cmd_error_info_srv);
extern REQUIRES_SERVICE_PLACEHOLDER_AS(mysql_command_query_result,
                                       cmd_query_result_srv);
extern REQUIRES_SERVICE_PLACEHOLDER_AS(mysql_thd_security_context,
                                       thd_security_ctx);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_current_thread_reader);
extern REQUIRES_SERVICE_PLACEHOLDER_AS(mysql_security_context_options,
                                       security_ctx_options);

static bool gen_dictionary_init(UDF_INIT *initid, UDF_ARGS *args,
                                char *message) {
  DBUG_TRACE;

  if (args->arg_count != 1) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument list: gen_dictionary(dictionary name)");
    return true;
  }

  if (args->arg_type[0] != STRING_RESULT) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument type: gen_dictionary(string)");
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

static void gen_dictionary_deinit(UDF_INIT *initid) {
  DBUG_TRACE;

  if (initid->ptr) delete[] initid->ptr;
}

static char *gen_dictionary(UDF_INIT *initid, UDF_ARGS *args, char *,
                            unsigned long *length, char *is_null, char *) {
  DBUG_TRACE;

  std::string original_charset;
  mysql::plugins::get_arg_character_set(args, 0, original_charset);
  const std::string dictionary = mysql::plugins::convert(
      {args->args[0], args->lengths[0]}, original_charset, mysql::plugins::default_charset);

  mysql::components::sql_context sqlc;

  std::string query(
      "SELECT Term FROM mysql.masking_dictionaries WHERE Dictionary =\"");

  mysql::components::escape_string_into(query, dictionary);

  query += "\" ORDER BY RAND() LIMIT 1";

  auto sresult = sqlc.query_single_value(query);

  if (sresult && (*length = sresult->size()) > 0) {
    initid->ptr = new char[sresult->size() + 1];
    memcpy(initid->ptr, sresult->c_str(), sresult->size() + 1);
  } else {
    *is_null = 1;
    return nullptr;
  }

  return initid->ptr;
}

udf_descriptor udf_gen_dictionary() {
  return {"gen_dictionary", Item_result::STRING_RESULT,
          reinterpret_cast<Udf_func_any>(gen_dictionary), gen_dictionary_init,
          gen_dictionary_deinit};
}
