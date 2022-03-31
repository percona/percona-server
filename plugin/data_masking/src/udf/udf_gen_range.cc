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

#include <iostream>

extern "C" {
  my_bool gen_range_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
  void gen_range_deinit(UDF_INIT *initid);
  long long gen_range(UDF_INIT *initid, UDF_ARGS *args, char *is_null,
                      char *is_error);
}

my_bool gen_range_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
  DBUG_ENTER("gen_range_init");

  if (!data_masking_is_inited(message, MYSQL_ERRMSG_SIZE)) {
    DBUG_RETURN(true);
  }

  if (args->arg_count != 2) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument list: gen_range(lower, upper)");
    DBUG_RETURN(true);
  }

  if (args->arg_type[0] != INT_RESULT || args->arg_type[1] != INT_RESULT) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument type: gen_range(long, long)");
    DBUG_RETURN(true);
  }

  initid->maybe_null = 1;
  initid->const_item =
      0;  // Non-Deterministic: same arguments will produce different values
  initid->ptr = NULL;

  DBUG_RETURN(false);
}

void gen_range_deinit(UDF_INIT *initid) {
  DBUG_ENTER("gen_range_deinit");

  if (initid->ptr) free(initid->ptr);

  DBUG_VOID_RETURN;
}

/**
 * Generates a random number chosen from a specified range.
 *
 * @param lower: An integer that specifies the lower boundary of the range.
 * @param upper: An integer that specifies the upper boundary of the range,
 * which must not be less than the lower boundary.
 *
 * @return A random integer in the range from lower to upper, inclusive, or NULL
 * if the upper argument is less than lower.
 */
long long gen_range(UDF_INIT *initid MY_ATTRIBUTE((unused)), UDF_ARGS *args,
                    char *is_null, char *is_error) {
  DBUG_ENTER("gen_range");

  long lower = *(long long *)args->args[0];
  long upper = *(long long *)args->args[1];
  long long value = 0;

  if (upper < lower) {
    *is_null = 1;
  } else {
    value = mysql::plugins::random_number(lower, upper);
  }
  *is_error = 0;

  DBUG_RETURN(value);
}
