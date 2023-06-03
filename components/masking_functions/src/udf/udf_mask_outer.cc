/* Copyright (c) 2018, 2019 Francisco Miguel Biete Banon. All rights reserved.
   Copyright (c) 2023 Percona LLC and/or its affiliates. All rights reserved.

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

#include "components/masking_functions/include/udf/udf_mask_outer.h"
#include "components/masking_functions/include/component.h"
#include "components/masking_functions/include/udf/udf_utils.h"
#include "components/masking_functions/include/udf/udf_utils_string.h"

#include <mysql/components/services/mysql_string.h>
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_string_converter);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_string_character_access);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_string_factory);

static bool mask_outer_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
  DBUG_TRACE;

  if (args->arg_count < 3 || args->arg_count > 4) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument list: MASK_OUTER(string, marging left, "
                  "margin right, [masking character])");
    return true;
  }

  if (args->arg_type[0] != STRING_RESULT || args->arg_type[1] != INT_RESULT ||
      args->arg_type[2] != INT_RESULT ||
      (args->arg_count == 4 &&
       (args->arg_type[3] != STRING_RESULT /*|| args->lengths[3] != 1*/))) {
    // TODO: we can't really implement a good length check
    // The currently existing service functions fail to handle charsets such as
    // utf16/utf32
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument type: MASK_OUTER(string, int, int, [char])");
    return true;
  }

  const int a2 = *(int *)args->args[1];
  const int a3 = *(int *)args->args[2];

  if (a2 < 0) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument: margin1 can't be negative");
    return true;
  }

  if (a3 < 0) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument: margin1 can't be negative");
    return true;
  }

  if (mysql::plugins::set_return_value_charset_to_match_arg(initid, args, 0)) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Unable to set character set service for UDF");
    return true;
  }

  initid->maybe_null = 1;
  initid->ptr = nullptr;

  return false;
}

static void mask_outer_deinit(UDF_INIT *initid) {
  DBUG_TRACE;

  if (initid->ptr) delete[] initid->ptr;
}

/**
 * Masks the left and right ends of a string, leaving the interior unmasked, and
 * returns the result. An optional masking character can be specified.
 *
 * @param str: The string to mask.
 * @param str_length: The length of the string to mask.
 * @param margin1: A nonnegative integer that specifies the number of characters
 * on the left end of the string to mask. If the value is 0, no left end
 * characters are masked.
 * @param margin2: A nonnegative integer that specifies the number of characters
 * on the right end of the string to mask. If the value is 0, no right end
 * characters are masked.
 * @param mask_char: (Optional) The single character to use for masking. The
 * default is 'X' if mask_char is not given.
 *
 * @return The masked string, or NULL if either margin is negative.
 *         If the sum of the margin values is larger than the argument length,
 * the entire argument is masked.
 */
static const char *mask_outer(UDF_INIT *initid, UDF_ARGS *args,
                              char *result [[maybe_unused]],
                              unsigned long *length, char *is_null, char *) {
  DBUG_TRACE;

  if (args->args[0] == nullptr) {
    *is_null = 1;
    return nullptr;
  }

  std::string original_charset;
  mysql::plugins::get_arg_character_set(args, 0, original_charset);

  const std::string masking_char =
      mysql::plugins::decide_masking_char(args, 3, original_charset);

  // Character set can be variable width: we use service functions to figure out
  // lengths
  // TODO: these service functions are bugged, they can't handle utf16/32
  const uint a2 = *(int *)args->args[1];
  const uint a3 = *(int *)args->args[2];
  ulong c2, c3;
  uint mlen = 0;
  {
    my_h_string mstr;
    mysql_service_mysql_string_converter->convert_from_buffer(
        &mstr, args->args[0], args->lengths[0], original_charset.c_str());
    mysql_service_mysql_string_character_access->get_char_length(mstr, &mlen);
    mysql_service_mysql_string_character_access->get_char_offset(mstr, a2, &c2);
    mysql_service_mysql_string_character_access->get_char_offset(
        mstr, mlen - a3, &c3);
    mysql_service_mysql_string_factory->destroy(mstr);
  }

  if (a2 + a3 >= mlen) {
    // too long => all masked
    std::string sresult;

    for (uint i = 0; i < mlen; ++i) sresult.append(masking_char);

    initid->ptr = new char[sresult.size() + 1];
    memcpy(initid->ptr, sresult.c_str(), sresult.size() + 1);
    *length = sresult.size();
    return initid->ptr;
  }

  const int chars_to_keep = mlen - a2 - a3;

  std::string sresult;

  for (uint i = 0; i < a2; ++i) sresult.append(masking_char);

  sresult.append(args->args[0] + a2, chars_to_keep);

  for (uint i = 0; i < a3; ++i) sresult.append(masking_char);

  if ((*length = sresult.size()) > 0) {
    initid->ptr = new char[sresult.size() + 1];
    memcpy(initid->ptr, sresult.c_str(), sresult.size() + 1);
  }

  return initid->ptr;
}

udf_descriptor udf_mask_outer() {
  return {"mask_outer", Item_result::STRING_RESULT,
          reinterpret_cast<Udf_func_any>(mask_outer), mask_outer_init,
          mask_outer_deinit};
}
