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

#include "components/masking_functions/include/udf/udf_gen_rnd_uk_nin.h"
#include "components/masking_functions/include/component.h"
#include "components/masking_functions/include/udf/udf_utils.h"
#include "components/masking_functions/include/udf/udf_utils_string.h"

static bool gen_rnd_uk_nin_init(UDF_INIT *initid, UDF_ARGS *args,
                                char *message) {
  DBUG_TRACE;

  if (args->arg_count != 0) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument list: gen_rnd_uk_nin()");
    return true;
  }

  if (mysql::plugins::set_return_value_charset(initid)) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Unable to set character set service for UDF");
    return true;
  }

  initid->maybe_null = 0;
  initid->const_item =
      0;  // Non-Deterministic: same arguments will produce different values
  initid->ptr = nullptr;

  return false;
}

static void gen_rnd_uk_nin_deinit(UDF_INIT *initid) {
  DBUG_TRACE;

  if (initid->ptr) delete[] initid->ptr;
}

/**
 * Returns a random U.S. Social Security number with the first and second parts
 * each chosen from a range not used for legitimate numbers
 *
 * @return A random U.S. Social Security number.
 */
static char *gen_rnd_uk_nin(UDF_INIT *initid, UDF_ARGS *args [[maybe_unused]],
                            char *result [[maybe_unused]],
                            unsigned long *length, char *, char *) {
  DBUG_TRACE;

  std::string uk_nin = mysql::plugins::random_uk_nin();
  *length = uk_nin.size();
  initid->ptr = new char[*length + 1];
  strcpy(initid->ptr, uk_nin.c_str());

  return initid->ptr;
}

udf_descriptor udf_gen_rnd_uk_nin() {
  return {"gen_rnd_uk_nin", Item_result::STRING_RESULT,
          reinterpret_cast<Udf_func_any>(gen_rnd_uk_nin), gen_rnd_uk_nin_init,
          gen_rnd_uk_nin_deinit};
}
