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

#include "components/masking_functions/include/udf/udf_mask_canada_sin.h"
#include "components/masking_functions/include/component.h"
#include "components/masking_functions/include/udf/udf_utils.h"
#include "components/masking_functions/include/udf/udf_utils_string.h"

#include <mysql/components/services/mysql_string.h>
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_string_converter);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_string_character_access);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_string_factory);

static bool mask_canada_sin_init(UDF_INIT *initid, UDF_ARGS *args,
                                 char *message) {
  DBUG_TRACE;

  if (args->arg_count > 2) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument list: mask_canada_sin(string, [mask_char])");
    return true;
  }

  if (args->arg_type[0] != STRING_RESULT ||
      (args->arg_count >= 2 && args->arg_type[1] != STRING_RESULT)) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument type: mask_canada_sin(string, [mask_char])");
    return true;
  }

  if (args->lengths[0] != 9 && args->lengths[0] != 11) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument: SIN must be specified either as nine "
                  "digits, or in the format xxx-xxx-xxxx");
    return true;
  }

  if (mysql::plugins::set_return_value_charset_to_match_arg(initid, args, 0)) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Unable to set character set service for UDF");
    return true;
  }

  initid->maybe_null = 1;
  initid->ptr = NULL;

  return false;
}

static void mask_canada_sin_deinit(UDF_INIT *initid) {
  DBUG_TRACE;

  if (initid->ptr) delete[] initid->ptr;
}

/**
 * Masks a U.S. Social Security number and returns the number with all but the
 * last four digits replaced by 'X' characters.
 *
 * @param str: The string to mask. The string must be a suitable length for the
 * Primary Account Number, but is not otherwise checked.
 * @param str_length: The length of the string to mask.
 *
 * @return The masked Social Security number as a string, or NULL if the
 * argument is not the correct length..
 */
static const char *mask_canada_sin(UDF_INIT *initid, UDF_ARGS *args,
                                   char *result [[maybe_unused]],
                                   unsigned long *length, char *is_null,
                                   char *) {
  DBUG_TRACE;

  if (args->args[0] == nullptr) {
    *is_null = 1;
    return nullptr;
  }

  std::string original_charset;
  mysql::plugins::get_arg_character_set(args, 0, original_charset);

  const std::string masking_char =
      mysql::plugins::decide_masking_char(args, 1, original_charset);

  std::string sresult;

  if (args->lengths[0] == 11) {
    sresult = mysql::plugins::mask_inner(args->args[0], args->lengths[0], 4, 4,
                                         original_charset, masking_char);
    sresult = mysql::plugins::mask_inner(sresult.c_str(), sresult.size(), 0, 8,
                                         original_charset, masking_char);
    sresult = mysql::plugins::mask_inner(sresult.c_str(), sresult.size(), 8, 0,
                                         original_charset, masking_char);
  } else {
    sresult = mysql::plugins::mask_inner(args->args[0], args->lengths[0], 0, 0,
                                         original_charset, masking_char);
  }

  if ((*length = sresult.size()) > 0) {
    initid->ptr = new char[sresult.size() + 1];
    memcpy(initid->ptr, sresult.c_str(), sresult.size() + 1);
  }

  return initid->ptr;
}

udf_descriptor udf_mask_canada_sin() {
  return {"mask_canada_sin", Item_result::STRING_RESULT,
          reinterpret_cast<Udf_func_any>(mask_canada_sin), mask_canada_sin_init,
          mask_canada_sin_deinit};
}
