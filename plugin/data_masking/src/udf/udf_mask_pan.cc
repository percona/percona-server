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

#include <my_global.h>
#include "../../include/plugin.h"
#include "../../include/udf/udf_utils.h"
#include "../../include/udf/udf_utils_string.h"

extern "C" {
  my_bool mask_pan_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
  void mask_pan_deinit(UDF_INIT *initid);
  const char *mask_pan(UDF_INIT *initid, UDF_ARGS *args, char *result,
                       unsigned long *length, char *is_null, char *);
}

my_bool mask_pan_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
  DBUG_ENTER("mask_pan_init");

  if (!data_masking_is_inited(message, MYSQL_ERRMSG_SIZE)) {
    DBUG_RETURN(true);
  }

  if (args->arg_count != 1) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument list: mask_pan(string)");
    DBUG_RETURN(true);
  }

  if (args->arg_type[0] != STRING_RESULT) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument type: mask_pan(string)");
    DBUG_RETURN(true);
  }

  initid->maybe_null = 1;
  initid->ptr = NULL;

  DBUG_RETURN(false);
}

void mask_pan_deinit(UDF_INIT *initid) {
  DBUG_ENTER("mask_pan_deinit");

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
const char *mask_pan(UDF_INIT *initid, UDF_ARGS *args,
                     char *result   MY_ATTRIBUTE((unused)),
                     unsigned long *length, char *is_null, char *) {
  DBUG_ENTER("mask_pan");

  if (args->args[0] == NULL) {
    *is_null = 1;
  } else {
    const char masking_char = 'X';
    const unsigned int unmasked_chars_end = 4;

    std::string s(args->args[0]);
    if (args->lengths[0] == PAN_LENGTH_1 || args->lengths[0] == PAN_LENGTH_2)
      s = mysql::plugins::mask_inner(args->args[0], args->lengths[0], 0,
                                     unmasked_chars_end, masking_char);
    if ((*length = s.length()) > 0) {
      initid->ptr = new char[*length + 1];
      strcpy(initid->ptr, s.c_str());
    }
  }

  DBUG_RETURN(initid->ptr);
}
