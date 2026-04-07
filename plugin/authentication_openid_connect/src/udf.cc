/*
(C) 2026 Percona LLC and/or its affiliates

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
*/

#include <mysql/udf_registration_types.h>

#include <string.h>

#include "config.h"

extern "C" {

bool update_jwks_init(UDF_INIT *udf_init, UDF_ARGS *args, char *message) {
  if (args->arg_count > 1) {
    std::strncpy(message, "function requires 0 or 1 argument",
                 MYSQL_ERRMSG_SIZE);
    return true;
  }
  if (args->arg_count == 1 && args->arg_type[0] != STRING_RESULT) {
    std::strncpy(message, "first argument of the function must be string",
                 MYSQL_ERRMSG_SIZE);
    return true;
  }

  udf_init->maybe_null = false;
  udf_init->decimals = 0;
  udf_init->max_length = 20;

  return false;
}

void update_jwks_deinit(UDF_INIT *udf_init [[maybe_unused]]) {}

long long update_jwks(UDF_INIT *udf_init [[maybe_unused]], UDF_ARGS *args,
                      char *is_null, char *error) {
  *is_null = 0;
  const long long ret = (args->arg_count == 0)
                            ? Idp_configs::update_keys()
                            : Idp_configs::update_keys(args->args[0]);
  *error = (ret >= 0) ? 0 : 1;
  return ret;
}
}