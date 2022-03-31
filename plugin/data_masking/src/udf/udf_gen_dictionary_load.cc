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
#include "gen_dictionary_load.h"
#include "global.h"
#include "utils.h"
#include "utils_string.h"
*/

#include <my_global.h>
#include "../../include/plugin.h"
#include "../../include/udf/udf_utils.h"
#include "../../include/udf/udf_utils_string.h"

#include <fstream>
#include <iostream>

extern "C" {
  my_bool gen_dictionary_load_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
  void gen_dictionary_load_deinit(UDF_INIT *initid);
  char *gen_dictionary_load(UDF_INIT *, UDF_ARGS *args, char *result,
                            unsigned long *length, char *, char *);
}

my_bool gen_dictionary_load_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
  DBUG_ENTER("gen_dictionary_load_init");

  if (!data_masking_is_inited(message, MYSQL_ERRMSG_SIZE)) {
    DBUG_RETURN(true);
  }

  if (args->arg_count != 2) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument list: gen_dictionary_load(dictionary_path, "
                  "dictionary name)");
    DBUG_RETURN(true);
  }

  if (args->arg_type[0] != STRING_RESULT ||
      args->arg_type[1] != STRING_RESULT) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument type: gen_dictionary_load(string, string)");
    DBUG_RETURN(true);
  }

  initid->maybe_null = 0;
  initid->const_item =
      0;  // Non-Deterministic: same arguments will produce different values
  initid->ptr = NULL;

  DBUG_RETURN(false);
}

void gen_dictionary_load_deinit(UDF_INIT *initid) {
  DBUG_ENTER("gen_dictionary_load_deinit");

  if (initid->ptr) free(initid->ptr);

  DBUG_VOID_RETURN;
}

/**
 * Loads a file into the dictionary registry and assigns the dictionary a name
 * to be used with other functions that require a dictionary name argument.
 *
 * Important: Dictionaries are not persistent. Any dictionary used by
 * applications must be loaded for each server startup. Once loaded into the
 * registry, a dictionary is used as is, even if the underlying
 * dictionary file changes. To reload a dictionary, first drop it with
 * gen_dictionary_load(), then load it again with
 * gen_dictionary_load().
 *
 * @param dictionary_path: A string that specifies the path name of the
 * dictionary file.
 * @param dictionary_name: A string that provides a name for the dictionary.
 *
 * @return A string that indicates whether the load operation succeeded.
 *            "Dictionary load success" indicates success.
 *            "Dictionary load error" indicates failure.
 *            Dictionary load failure can occur for several reasons, including:
 *               - A dictionary with the given name is already loaded.
 *               - The dictionary file is not found.
 *               - The dictionary file contains no terms.
 *               - The secure_file_priv system variable is set and the
 * dictionary file is not located in the directory named by the variable.
 */
static std::string _gen_dictionary_load(const char *dictionary_path,
                                        const char *dictionary_name) {
  DBUG_ENTER("_gen_dictionary_load");
  std::string res = "Dictionary load error: unknown";
  std::string s_dictname(dictionary_name);
  mysql::plugins::tolower(s_dictname);

  std::ifstream file(dictionary_path);
  // Check if the file exist in disk
  if (file) {
    mysql_rwlock_wrlock(&g_data_masking_dict_rwlock);
    // Check if dictionary already exists in global list
    if (g_data_masking_dict->count(s_dictname) == 1) {
      res = "Dictionary load error: a dictionary with that name already exists";
    } else {
      // Load file content
      std::vector<std::string> list;
      std::string s;
      while (std::getline(file, s)) {
        mysql::plugins::trim(s);
        if (s.length() > 0) list.push_back(s);
      }
      if (list.size() > 0) {
        std::sort(list.begin(), list.end());
        (*g_data_masking_dict)[s_dictname] = list;
        res = "Dictionary load success";
      } else {
        res = "Dictionary load error: dictionary file contains no records";
      }
    }
    mysql_rwlock_unlock(&g_data_masking_dict_rwlock);
  } else {
    res = "Dictionary load error: dictionary file not readable";
  }

  DBUG_RETURN(res);
}

char *gen_dictionary_load(UDF_INIT *, UDF_ARGS *args, char *result,
                          unsigned long *length, char *, char *) {
  DBUG_ENTER("gen_dictionary_load");

  std::string res = _gen_dictionary_load(args->args[0], args->args[1]);
  assert(res.size() < *length);

  *length = std::min<unsigned long>(res.size(), *length - 1);
  strncpy(result, res.c_str(), *length);
  result[*length]= '\0';

  DBUG_RETURN(result);
}
