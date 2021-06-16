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

extern "C" {
  my_bool gen_dictionary_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
  void gen_dictionary_deinit(UDF_INIT *initid);
  char *gen_dictionary(UDF_INIT *initid, UDF_ARGS *args, char *,
                       unsigned long *length, char *is_null, char *);
}

my_bool gen_dictionary_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
  DBUG_ENTER("gen_dictionary_init");

  if (!data_masking_is_inited(message, MYSQL_ERRMSG_SIZE)) {
    DBUG_RETURN(true);
  }

  if (args->arg_count != 1) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument list: gen_dictionary(dictionary name)");
    DBUG_RETURN(true);
  }

  if (args->arg_type[0] != STRING_RESULT) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument type: gen_dictionary(string)");
    DBUG_RETURN(true);
  }

  initid->maybe_null = 1;
  initid->const_item =
      0;  // Non-Deterministic: same arguments will produce different values
  initid->ptr = NULL;

  DBUG_RETURN(false);
}

void gen_dictionary_deinit(UDF_INIT *initid) {
  DBUG_ENTER("gen_dictionary_deinit");

  if (initid->ptr) delete[] initid->ptr;

  DBUG_VOID_RETURN;
}

/**
 * Returns a random term from a dictionary.
 *
 * @param dictionary_name: A string that names the dictionary from which to
 * choose the term.
 *
 * @return A random term from the dictionary as a string, or NULL if the
 * dictionary name is not in the dictionary registry.
 */
static std::string _gen_dictionary(const char *dictionary_name) {
  std::string res = "";
  std::string s_dictname(dictionary_name);
  mysql::plugins::tolower(s_dictname);

  mysql_rwlock_rdlock(&g_data_masking_dict_rwlock);
  if (g_data_masking_dict->count(s_dictname) == 1) {
    std::vector<std::string> *a = &(g_data_masking_dict->at(s_dictname));
    res = (*a)[mysql::plugins::random_number(0, a->size() - 1)];
  }
  mysql_rwlock_unlock(&g_data_masking_dict_rwlock);

  return res;
}

char *gen_dictionary(UDF_INIT *initid, UDF_ARGS *args, char *,
                     unsigned long *length, char *is_null, char *) {
  DBUG_ENTER("gen_dictionary");

  std::string res = _gen_dictionary(args->args[0]);
  *length = res.size();
  if (!(*is_null = (*length == 0))) {
    initid->ptr = new char[*length + 1];
    strcpy(initid->ptr, res.c_str());
  }

  DBUG_RETURN(initid->ptr);
}
