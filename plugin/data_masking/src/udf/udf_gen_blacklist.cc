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

#include <algorithm>

extern "C" {
  my_bool gen_blacklist_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
  void gen_blacklist_deinit(UDF_INIT *initid);
  char *gen_blacklist(UDF_INIT *initid, UDF_ARGS *args, char *,
                      unsigned long *length, char *is_null, char *);
}

my_bool gen_blacklist_init(UDF_INIT *initid, UDF_ARGS *args,
                        char *message) {
  DBUG_ENTER("gen_blacklist_init");

  if (!data_masking_is_inited(message, MYSQL_ERRMSG_SIZE)) {
    DBUG_RETURN(true);
  }

  if (args->arg_count != 3) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument list: gen_blacklist(str, dictionary_name, "
                  "replacement_dictionary_name)");
    DBUG_RETURN(true);
  }

  if (args->arg_type[0] != STRING_RESULT ||
      args->arg_type[1] != STRING_RESULT ||
      args->arg_type[2] != STRING_RESULT) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument type: gen_blacklist(string, string, string)");
    DBUG_RETURN(true);
  }

  initid->maybe_null = 1;
  initid->const_item =
      0;  // Non-Deterministic: same arguments will produce different values
  initid->ptr = NULL;

  DBUG_RETURN(false);
}

void gen_blacklist_deinit(UDF_INIT *initid) {
  DBUG_ENTER("gen_blacklist_deinit");

  if (initid->ptr) delete[] (initid->ptr);

  DBUG_VOID_RETURN;
}

/**
 * Replaces a term present in one dictionary with a term from a second
 * dictionary and returns the replacement term. This masks the original term by
 * substitution.
 *
 * @param str: A string that indicates the term to replace.
 * @param dictionary_name: A string that names the dictionary containing the
 * term to replace.
 * @param replacement_dictionary_name: A string that names the dictionary from
 * which to choose the replacement term.
 *
 * @return A string randomly chosen from replacement_dictionary_name as a
 * replacement for str, or str if it does not appear in dictionary_name, or NULL
 * if either dictionary name is not in the dictionary registry. If the term to
 * replace appears in both dictionaries, it is possible for the return
 * value to be the same term.
 */
static std::string _gen_blacklist(const char *str, const char *dictionary_name,
                                  const char *replacement_dictionary_name) {
  std::string res(str);
  std::string s_dictname_a(dictionary_name);
  mysql::plugins::tolower(s_dictname_a);
  std::string s_dictname_b(replacement_dictionary_name);
  mysql::plugins::tolower(s_dictname_b);

  mysql_rwlock_rdlock(&g_data_masking_dict_rwlock);
  if (g_data_masking_dict->count(s_dictname_a) == 1 &&
      g_data_masking_dict->count(s_dictname_b) == 1) {
    std::vector<std::string> *a = &(g_data_masking_dict->at(s_dictname_a));
    std::vector<std::string> *b = &(g_data_masking_dict->at(s_dictname_b));
    if (std::binary_search(a->begin(), a->end(), res)) {
      // found, we choose a value from b
      res = (*b)[mysql::plugins::random_number(0, b->size() - 1)];
    }
  } else {
    // dictionary not found
    res = "";
  }
  mysql_rwlock_unlock(&g_data_masking_dict_rwlock);

  return res;
}

char *gen_blacklist(UDF_INIT *initid, UDF_ARGS *args, char *,
                    unsigned long *length, char *is_null, char *) {
  DBUG_ENTER("gen_blacklist");

  std::string res = _gen_blacklist(args->args[0], args->args[1], args->args[2]);
  *length = res.size();
  if (!(*is_null = (*length == 0))) {
    initid->ptr = new char[*length + 1];
    strcpy(initid->ptr, res.c_str());
  }

  DBUG_RETURN(initid->ptr);
}
