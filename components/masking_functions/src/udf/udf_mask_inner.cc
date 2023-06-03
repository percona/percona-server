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

#include "components/masking_functions/include/udf/udf_mask_inner.h"
#include "components/masking_functions/include/component.h"
#include "components/masking_functions/include/udf/udf_utils.h"
#include "components/masking_functions/include/udf/udf_utils_string.h"

#include <mysql/components/services/mysql_string.h>

extern REQUIRES_SERVICE_PLACEHOLDER(mysql_string_converter);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_string_character_access);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_string_factory);

static bool mask_inner_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
  DBUG_TRACE;

  if (args->arg_count < 3 || args->arg_count > 4) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument list: MASK_INNER(string, marging left, "
                  "margin right, [masking character])");
    return true;
  }

  if (args->arg_type[0] != STRING_RESULT || args->arg_type[1] != INT_RESULT ||
      args->arg_type[2] != INT_RESULT ||
      (args->arg_count == 4 &&
       (args->arg_type[3] !=
        STRING_RESULT /*|| mysql::plugins::str_len(args, 3) != 1*/))) {
    // TODO: we can't really implement a good length check
    // The currently existing service functions fail to handle charsets such as
    // utf16/utf32
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument type: MASK_INNER(string, int, int, [char])");
    return true;
  }

  const int a2 = *(int *)args->args[1];
  const int a3 = *(int *)args->args[2];

  if (a2 < 0) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument: margin1 can't be negative");
    return true;
  }

  if (a3 < 0) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument: margin1 can't be negative");
    return true;
  }

  if (mysql::plugins::set_return_value_charset_to_match_arg(initid, args, 0)) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Unable to set return type of function");
    return true;
  }

  initid->maybe_null = 1;
  initid->ptr = nullptr;

  return false;
}

static void mask_inner_deinit(UDF_INIT *initid) {
  DBUG_TRACE;

  if (initid->ptr) delete[] initid->ptr;
}

/**
 * Masks the interior part of a string, leaving the ends untouched, and returns
 * the result. An optional masking character can be specified.
 * @param str: The string to mask.
 * @param str_length: The length of the string to mask
 * @param margin1: A nonnegative integer that specifies the number of characters
 * on the left end of the string to remain unmasked. If the value is 0, no left
 * end characters remain unmasked.
 * @param margin2: A nonnegative integer that specifies the number of characters
 * on the right end of the string to remain unmasked. If the value is 0, no
 * right end characters remain unmasked.
 * @param mask_char: (Optional) The single character to use for masking. The
 * default is 'X' if mask_char is not given.
 *
 * @returns The masked string, or NULL if either margin is negative. If the sum
 * of the margin values is larger than the argument length, no masking occurs
 * and the argument is returned unchanged.
 */
static const char *mask_inner(UDF_INIT *initid, UDF_ARGS *args,
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
      mysql::plugins::decide_masking_char(args, 3, original_charset);

  // Character set can be variable width: we use service functions to figure out
  // lengths
  // TODO: these service functions are bugged, they can't handle utf16/32
  const uint a2 = *(int *)args->args[1];
  const uint a3 = *(int *)args->args[2];

  const std::string sresult = mysql::plugins::mask_inner(
      args->args[0], args->lengths[0], a2, a3, original_charset, masking_char);

  if ((*length = sresult.size()) > 0) {
    initid->ptr = new char[sresult.size() + 1];
    memcpy(initid->ptr, sresult.c_str(), sresult.size() + 1);
  }

  return initid->ptr;
}

udf_descriptor udf_mask_inner() {
  return {"mask_inner", Item_result::STRING_RESULT,
          reinterpret_cast<Udf_func_any>(mask_inner), mask_inner_init,
          mask_inner_deinit};
}
