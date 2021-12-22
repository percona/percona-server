/* Copyright (c) 2018, 2019 Francisco Miguel Biete Banon. All rights reserved.

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

#include "plugin/data_masking/include/udf/udf_mask_outer.h"
#include "plugin/data_masking/include/plugin.h"
#include "plugin/data_masking/include/udf/udf_utils.h"
#include "plugin/data_masking/include/udf/udf_utils_string.h"

static bool mask_outer_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
  DBUG_ENTER("mask_outer_init");

  if (args->arg_count < 3 || args->arg_count > 4) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument list: MASK_OUTER(string, marging left, "
                  "margin right, [masking character])");
    DBUG_RETURN(true);
  }

  if (args->arg_type[0] != STRING_RESULT || args->arg_type[1] != INT_RESULT ||
      args->arg_type[2] != INT_RESULT ||
      (args->arg_count == 4 &&
       (args->arg_type[3] != STRING_RESULT || args->lengths[3] != 1))) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument type: MASK_OUTER(string, int, int, [char])");
    DBUG_RETURN(true);
  }

  if (mysql::plugins::Charset_service::set_return_value_charset(initid) ||
      mysql::plugins::Charset_service::set_args_charset(args)) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Unable to set character set service for UDF");
    DBUG_RETURN(true);
  }

  initid->maybe_null = 1;
  initid->ptr = NULL;

  DBUG_RETURN(false);
}

static void mask_outer_deinit(UDF_INIT *initid) {
  DBUG_ENTER("mask_outer_deinit");

  if (initid->ptr) delete[] initid->ptr;

  DBUG_VOID_RETURN;
}

/**
 * Masks the left and right ends of a string, leaving the interior unmasked, and
 * returns the result. An optional masking character can be specified.
 *
 * @param str: The string to mask.
 * @param str_length: The length of the string to mask.
 * @param margin1: A nonnegative integer that specifies the number of characters
 * on the left end of the string to mask. If the value is 0, no left end
 * characters are masked.
 * @param margin2: A nonnegative integer that specifies the number of characters
 * on the right end of the string to mask. If the value is 0, no right end
 * characters are masked.
 * @param mask_char: (Optional) The single character to use for masking. The
 * default is 'X' if mask_char is not given.
 *
 * @return The masked string, or NULL if either margin is negative.
 *         If the sum of the margin values is larger than the argument length,
 * the entire argument is masked.
 */
static const char *mask_outer(UDF_INIT *initid, UDF_ARGS *args,
                              char *result [[maybe_unused]],
                              unsigned long *length, char *is_null, char *) {
  DBUG_ENTER("mask_outer");

  if (args->args[0] == NULL) {
    *is_null = 1;
  } else {
    char masking_char = 'X';
    if (args->arg_count == 4) {
      masking_char = args->args[3][0];
    }

    std::string s = mysql::plugins::mask_outer(
        args->args[0], args->lengths[0], *(int *)args->args[1],
        *(int *)args->args[2], masking_char);
    if ((*length = s.length()) > 0) {
      initid->ptr = new char[*length + 1];
      strcpy(initid->ptr, s.c_str());
    }
  }

  DBUG_RETURN(initid->ptr);
}

udf_descriptor udf_mask_outer() {
  return {"mask_outer", Item_result::STRING_RESULT,
          reinterpret_cast<Udf_func_any>(mask_outer), mask_outer_init,
          mask_outer_deinit};
}
