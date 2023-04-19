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

#include "plugin/data_masking/include/udf/udf_gen_rnd_us_phone.h"
#include "plugin/data_masking/include/plugin.h"
#include "plugin/data_masking/include/udf/udf_utils.h"
#include "plugin/data_masking/include/udf/udf_utils_string.h"

#include <iostream>

static bool gen_rnd_us_phone_init(UDF_INIT *initid, UDF_ARGS *args,
                                  char *message) {
  DBUG_ENTER("gen_rnd_us_phone_init");

  if (args->arg_count != 0) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument list: gen_rnd_us_phone()");
    DBUG_RETURN(true);
  }

  if (mysql::plugins::Charset_service::set_return_value_charset(initid)) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Unable to set character set service for UDF");
    DBUG_RETURN(true);
  }

  initid->maybe_null = 0;
  initid->const_item =
      0;  // Non-Deterministic: same arguments will produce different values
  initid->ptr = NULL;

  DBUG_RETURN(false);
}

static void gen_rnd_us_phone_deinit(UDF_INIT *initid) {
  DBUG_ENTER("gen_rnd_us_phone_deinit");

  if (initid->ptr) delete[] initid->ptr;

  DBUG_VOID_RETURN;
}

/**
 * Returns a random U.S. phone number in the 555 area code not used for
 * legitimate numbers.
 *
 * @return A random U.S. phone number.
 */
static char *gen_rnd_us_phone(UDF_INIT *initid,
                              UDF_ARGS *args [[maybe_unused]],
                              char *result [[maybe_unused]],
                              unsigned long *length, char *is_null,
                              char *is_error) {
  DBUG_ENTER("gen_rnd_us_phone");

  std::string phone = mysql::plugins::random_us_phone();
  *length = phone.size();
  initid->ptr = new char[*length + 1];
  strcpy(initid->ptr, phone.c_str());
  *is_error = 0;
  *is_null = 0;

  DBUG_RETURN(initid->ptr);
}

udf_descriptor udf_gen_rnd_us_phone() {
  return {"gen_rnd_us_phone", Item_result::STRING_RESULT,
          reinterpret_cast<Udf_func_any>(gen_rnd_us_phone),
          gen_rnd_us_phone_init, gen_rnd_us_phone_deinit};
}
