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

#include "components/masking_functions/include/udf/udf_mask_iban.h"
#include "components/masking_functions/include/component.h"
#include "components/masking_functions/include/udf/udf_utils.h"
#include "components/masking_functions/include/udf/udf_utils_string.h"

#include <mysql/components/services/mysql_string.h>
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_string_converter);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_string_character_access);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_string_factory);

static bool mask_iban_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
  DBUG_TRACE;

  if (args->arg_count > 2) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument list: mask_iban(string, [mask_char])");
    return true;
  }

  if (args->arg_type[0] != STRING_RESULT ||
      (args->arg_count >= 2 && args->arg_type[1] != STRING_RESULT)) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument type: mask_iban(string, [mask_char])");
    return true;
  }

  if (args->lengths[0] < 13 || args->lengths[0] > 34) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument: IBAN must be between 13 and 34 characters");
    return true;
  }

  if (mysql::plugins::set_return_value_charset_to_match_arg(initid, args, 0)) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Unable to set character set service for UDF");
    return true;
  }

  initid->maybe_null = 1;
  initid->ptr = nullptr;

  return false;
}

static void mask_iban_deinit(UDF_INIT *initid) {
  DBUG_ENTER("mask_iban_deinit");

  if (initid->ptr) delete[] initid->ptr;

  DBUG_VOID_RETURN;
}

/**
 * Masks a payment card Primary Account Number and returns the number with all
 * but the last four digits replaced by 'X' characters.
 *
 * @param str: The string to mask. The string must be a suitable length for the
 * Primary Account Number, but is not otherwise checked.
 * @param str_length: The length of the string to mask.
 *
 * @return The masked payment number as a string. If the argument is shorter
 * than required, it is returned unchanged.
 */
static const char *mask_iban(UDF_INIT *initid, UDF_ARGS *args,
                             char *result [[maybe_unused]],
                             unsigned long *length, char *is_null, char *) {
  DBUG_TRACE;

  if (args->args[0] == nullptr) {
    *is_null = 1;
    return nullptr;
  }

  std::string original_charset;
  mysql::plugins::get_arg_character_set(args, 0, original_charset);

  const std::string masking_char =
      mysql::plugins::decide_masking_char(args, 1, original_charset);

  const std::string sresult = mysql::plugins::mask_inner(
      args->args[0], args->lengths[0], 2, 0, original_charset, masking_char);

  if ((*length = sresult.size()) > 0) {
    initid->ptr = new char[sresult.size() + 1];
    memcpy(initid->ptr, sresult.c_str(), sresult.size() + 1);
  }

  return initid->ptr;
}

udf_descriptor udf_mask_iban() {
  return {"mask_iban", Item_result::STRING_RESULT,
          reinterpret_cast<Udf_func_any>(mask_iban), mask_iban_init,
          mask_iban_deinit};
}
