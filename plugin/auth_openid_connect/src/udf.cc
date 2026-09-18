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

#include <cstdio>

#include <mysql/udf_registration_types.h>
#include <mysql_com.h>

#include "config.h"

extern "C" {

/**
 * @brief Initialization function for the update_jwks UDF.
 *
 * This function is called by MySQL when the UDF is loaded. It validates
 * the number and types of arguments and sets up the return type.
 *
 * @param udf_init Pointer to UDF_INIT structure to be filled with metadata.
 * @param args Pointer to UDF_ARGS containing argument information.
 * @param message Output buffer for error messages (MYSQL_ERRMSG_SIZE bytes).
 * @return true if initialization fails, false on success.
 *
 * @note Arguments:
 *   - 0 arguments: Updates keys for all IDPs
 *   - 1 argument (STRING): Updates keys for specific IDP by name
 *   - More than 1 argument: Error
 */
bool update_jwks_init(UDF_INIT *udf_init, UDF_ARGS *args, char *message) {
  if (args->arg_count > 1) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "function requires 0 or 1 argument");
    return true;
  }
  if (args->arg_count == 1 && args->arg_type[0] != STRING_RESULT) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "first argument of the function must be string");
    return true;
  }

  udf_init->maybe_null = false;
  udf_init->decimals = 0;
  udf_init->max_length = 20;

  return false;
}

/**
 * @brief Deinitialization function for the update_jwks UDF.
 *
 * This function is called by MySQL when the UDF is unloaded.
 * Currently, does nothing as no resources are allocated during init.
 *
 * @param udf_init Pointer to UDF_INIT structure.
 */
void update_jwks_deinit(UDF_INIT *udf_init [[maybe_unused]]) {}

/**
 * @brief Updates JWKS keys for one or all IDPs.
 *
 * This UDF function refreshes the JSON Web Key Set from the JWKS endpoint(s).
 * Can either update all IDPs or a specific IDP by name.
 *
 * @param udf_init Pointer to UDF_INIT structure.
 * @param args Pointer to UDF_ARGS containing arguments.
 *   - args->arg_count == 0: Update all IDPs
 *   - args->arg_count == 1: Update specific IDP (args->args[0] contains name)
 * @param is_null Output parameter to mark result as NULL (set to 0 for valid
 * result).
 * @param error Output parameter for error flag (set to 1 on error).
 * @return Number of updated IDPs on success, negative on error.
 *   -  >= 0: Number of successfully updated IDPs
 *   -  -1: Configuration not found
 *   -  -2: Unexpected error during update
 *
 * @throws May throw exceptions which are caught and reported via error flag.
 */
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
