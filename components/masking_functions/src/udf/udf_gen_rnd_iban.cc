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

#include "components/masking_functions/include/udf/udf_gen_rnd_iban.h"
#include "components/masking_functions/include/component.h"
#include "components/masking_functions/include/udf/udf_utils.h"
#include "components/masking_functions/include/udf/udf_utils_string.h"

static bool gen_rnd_iban_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
  DBUG_TRACE;

  if (args->arg_count > 2) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument list: gen_rnd_iban([coutry], [size])");
    return true;
  }

  if ((args->arg_count >= 1 && args->arg_type[0] != STRING_RESULT) ||
      (args->arg_count == 2 && args->arg_type[1] != INT_RESULT)) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument type: gen_rnd_email([string, int])");
    return true;
  }

  if (args->arg_count >= 1 && args->lengths[1] != 2) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument: country code must be two characters");
    return true;
  }

  if (args->arg_count == 2) {
    long long len = *(int *)args->args[1];
    if (len < 15 || len > 34) {
      std::snprintf(
          message, MYSQL_ERRMSG_SIZE,
          "Wrong argument: length must be between 15 and 34 characters");
      return true;
    }
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

static void gen_rnd_iban_deinit(UDF_INIT *initid) {
  DBUG_TRACE;

  if (initid->ptr) delete[] initid->ptr;
}

/**
 * Returns a random U.S. Social Security number with the first and second parts
 * each chosen from a range not used for legitimate numbers
 *
 * @return A random U.S. Social Security number.
 */
static char *gen_rnd_iban(UDF_INIT *initid, UDF_ARGS *args [[maybe_unused]],
                          char *result [[maybe_unused]], unsigned long *length,
                          char *, char *) {
  DBUG_TRACE;

  std::string country("ZZ");
  if (args->arg_count >= 1) {
    country.assign(static_cast<const char *>(args->args[0]), args->lengths[0]);
  }

  long long len = 16;
  if (args->arg_count >= 2) {
    len = *(int *)args->args[1];
  }

  std::string iban = mysql::plugins::random_iban(country, len);
  *length = iban.size();
  initid->ptr = new char[*length + 1];
  strcpy(initid->ptr, iban.c_str());

  return initid->ptr;
}

udf_descriptor udf_gen_rnd_iban() {
  return {"gen_rnd_iban", Item_result::STRING_RESULT,
          reinterpret_cast<Udf_func_any>(gen_rnd_iban), gen_rnd_iban_init,
          gen_rnd_iban_deinit};
}
