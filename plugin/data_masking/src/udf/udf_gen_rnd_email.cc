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

#include "plugin/data_masking/include/udf/udf_gen_rnd_email.h"
#include "plugin/data_masking/include/plugin.h"
#include "plugin/data_masking/include/udf/udf_utils.h"
#include "plugin/data_masking/include/udf/udf_utils_string.h"

static bool gen_rnd_email_init(UDF_INIT *initid, UDF_ARGS *args,
                               char *message) {
  DBUG_ENTER("gen_rnd_email_init");

  if (args->arg_count > 2) {
    std::snprintf(
        message, MYSQL_ERRMSG_SIZE,
        "Wrong argument list: gen_rnd_email([length=20], [email domain])");
    DBUG_RETURN(true);
  }

  if ((args->arg_count >= 1 && args->arg_type[0] != INT_RESULT) ||
      (args->arg_count == 2 && args->arg_type[1] != STRING_RESULT)) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument type: gen_rnd_email([int, string])");
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

static void gen_rnd_email_deinit(UDF_INIT *initid) {
  DBUG_ENTER("gen_rnd_email_deinit");

  if (initid->ptr) delete[] initid->ptr;

  DBUG_VOID_RETURN;
}

/**
 * Returns a random email address in the example.com domain:
 *
 * @param length: [optional] Total length of the email address.
 * @param domain: [optional] Domain for the email address.
 *
 * @return A random email address.
 */
static char *gen_rnd_email(UDF_INIT *initid, UDF_ARGS *args,
                           char *result [[maybe_unused]],
                           unsigned long *length, char *is_null,
                           char *is_error) {
  DBUG_ENTER("gen_rnd_email");

  unsigned int email_length = 20;
  if (args->arg_count >= 1) {
    email_length = *(int *)args->args[0];
  }
  std::string email_domain("example.com");
  if (args->arg_count >= 2) {
    email_domain.assign(static_cast<const char *>(args->args[1]));
  }
  unsigned int user_length = email_length - (email_domain.length() + 1);

  std::string email = mysql::plugins::random_string(user_length, true)
                          .append("@")
                          .append(email_domain);
  *length = email.size();
  initid->ptr = new char[*length + 1];
  strcpy(initid->ptr, email.c_str());
  *is_error = 0;
  *is_null = 0;

  DBUG_RETURN(initid->ptr);
}

udf_descriptor udf_gen_rnd_email() {
  return {"gen_rnd_email", Item_result::STRING_RESULT,
          reinterpret_cast<Udf_func_any>(gen_rnd_email), gen_rnd_email_init,
          gen_rnd_email_deinit};
}
