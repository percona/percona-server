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

/*
#include "extern.h"
#include "gen_dictionary_drop.h"
#include "global.h"
#include "utils.h"
#include "utils_string.h"
*/

#include <my_global.h>
#include "../../include/plugin.h"
#include "../../include/udf/udf_utils.h"
#include "../../include/udf/udf_utils_string.h"

extern "C" {
  my_bool  gen_dictionary_drop_init(UDF_INIT *initid, UDF_ARGS *args,
                                 char *message);
  void  gen_dictionary_drop_deinit(UDF_INIT *initid);
  char *gen_dictionary_drop(UDF_INIT *, UDF_ARGS *args, char *result,
                            unsigned long *length, char *, char *);
}

my_bool gen_dictionary_drop_init(UDF_INIT *initid, UDF_ARGS *args,
                                     char *message) {
  DBUG_ENTER("gen_dictionary_drop_init");

  if (!data_masking_is_inited(message, MYSQL_ERRMSG_SIZE)) {
    DBUG_RETURN(true);
  }

  if (args->arg_count != 1) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument list: gen_dictionary_drop(dictionary name)");
    DBUG_RETURN(true);
  }

  if (args->arg_type[0] != STRING_RESULT) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument type: gen_dictionary_drop(string)");
    DBUG_RETURN(true);
  }

  initid->maybe_null = 0;
  initid->const_item =
      0;  // Non-Deterministic: same arguments will produce different values
  initid->ptr = NULL;

  DBUG_RETURN(false);
}

void gen_dictionary_drop_deinit(UDF_INIT *initid) {
  DBUG_ENTER("gen_dictionary_drop_deinit");

  if (initid->ptr) free(initid->ptr);

  DBUG_VOID_RETURN;
}

/**
 * Removes a dictionary from the dictionary registry.
 *
 * @param dictionary_name: A string that names the dictionary to remove from the
 * dictionary registry.
 *
 * @return A string that indicates whether the drop operation succeeded.
 *    "Dictionary removed" indicates success.
 *    "Dictionary removal error" indicates failure.
 */
static std::string _gen_dictionary_drop(const char *dictionary_name) {
  std::string res = "Dictionary removal error: unknown";
  std::string s_dictname(dictionary_name);
  mysql::plugins::tolower(s_dictname);

  mysql_rwlock_wrlock(&g_data_masking_dict_rwlock);
  // Check if dictionary exists in global list
  if (g_data_masking_dict->count(s_dictname) == 1) {
    if (g_data_masking_dict->erase(s_dictname) == 1) {
      res = "Dictionary removed";
    } else {
      res = "Dictionary removal error: erase failed";
    }
  } else {
    res = "Dictionary removal error: dictionary not present in global list";
  }
  mysql_rwlock_unlock(&g_data_masking_dict_rwlock);

  return res;
}

char *gen_dictionary_drop(UDF_INIT *, UDF_ARGS *args, char *result,
                          unsigned long *length, char *, char *) {
  DBUG_ENTER("gen_dictionary_drop");

  std::string res = _gen_dictionary_drop(args->args[0]);
  assert(res.size() < *length);

  *length = std::min<unsigned long>(res.size(), *length - 1);
  strncpy(result, res.c_str(), *length);
  result[*length]= '\0';

  DBUG_RETURN(result);
}
