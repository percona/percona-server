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

#include "plugin/data_masking/include/udf/udf_gen_rnd_ssn.h"
#include "plugin/data_masking/include/plugin.h"
#include "plugin/data_masking/include/udf/udf_utils.h"
#include "plugin/data_masking/include/udf/udf_utils_string.h"

static bool gen_rnd_ssn_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
  DBUG_ENTER("gen_rnd_ssn_init");

  if (args->arg_count != 0) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument list: gen_rnd_ssn()");
    DBUG_RETURN(true);
  }

  initid->maybe_null = 0;
  initid->const_item =
      0;  // Non-Deterministic: same arguments will produce different values
  initid->ptr = NULL;

  DBUG_RETURN(false);
}

static void gen_rnd_ssn_deinit(UDF_INIT *initid) {
  DBUG_ENTER("gen_rnd_ssn_deinit");

  if (initid->ptr) delete[] initid->ptr;

  DBUG_VOID_RETURN;
}

/**
 * Returns a random U.S. Social Security number with the first and second parts
 * each chosen from a range not used for legitimate numbers
 *
 * @return A random U.S. Social Security number.
 */
static char *gen_rnd_ssn(UDF_INIT *initid,
                         UDF_ARGS *args MY_ATTRIBUTE((unused)),
                         char *result MY_ATTRIBUTE((unused)),
                         unsigned long *length, char *, char *) {
  DBUG_ENTER("gen_rnd_ssn");

  std::string ssn = mysql::plugins::random_ssn();
  *length = ssn.size();
  initid->ptr = new char[*length + 1];
  strcpy(initid->ptr, ssn.c_str());

  DBUG_RETURN(initid->ptr);
}

udf_descriptor udf_gen_rnd_ssn() {
  return {"gen_rnd_ssn", Item_result::STRING_RESULT,
          reinterpret_cast<Udf_func_any>(gen_rnd_ssn), gen_rnd_ssn_init,
          gen_rnd_ssn_deinit};
}
