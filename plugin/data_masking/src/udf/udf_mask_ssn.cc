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

#include "plugin/data_masking/include/udf/udf_mask_ssn.h"
#include "plugin/data_masking/include/plugin.h"
#include "plugin/data_masking/include/udf/udf_utils.h"
#include "plugin/data_masking/include/udf/udf_utils_string.h"

static bool mask_ssn_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
  DBUG_ENTER("mask_ssn_init");

  if (args->arg_count != 1) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument list: mask_ssn(string)");
    DBUG_RETURN(true);
  }

  if (args->arg_type[0] != STRING_RESULT) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument type: mask_ssn(string)");
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

static void mask_ssn_deinit(UDF_INIT *initid) {
  DBUG_ENTER("mask_ssn_deinit");

  if (initid->ptr) delete[] initid->ptr;

  DBUG_VOID_RETURN;
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
static const char *mask_ssn(UDF_INIT *initid, UDF_ARGS *args,
                            char *result [[maybe_unused]],
                            unsigned long *length, char *is_null, char *) {
  DBUG_ENTER("mask_ssn");

  if (args->args[0] == NULL || args->lengths[0] != SSN_LENGTH) {
    *is_null = 1;
  } else {
    const char masking_char = 'X';
    const unsigned int unmasked_chars_end = 4;

    std::string s(args->args[0]);
    s = mysql::plugins::mask_inner(args->args[0], args->lengths[0], 0,
                                   unmasked_chars_end, masking_char);
    *length = s.length();
    initid->ptr = new char[*length + 1];
    strcpy(initid->ptr, s.c_str());
    initid->ptr[3] = '-';
    initid->ptr[6] = '-';
  }

  DBUG_RETURN(initid->ptr);
}

udf_descriptor udf_mask_ssn() {
  return {"mask_ssn", Item_result::STRING_RESULT,
          reinterpret_cast<Udf_func_any>(mask_ssn), mask_ssn_init,
          mask_ssn_deinit};
}
