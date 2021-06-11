/* Copyright (c) 2012, 2021, Oracle and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#ifndef MY_DEFAULT_INCLUDED
#define MY_DEFAULT_INCLUDED


#include "my_global.h"

/**
  First mysql version supporting compressed columns.
*/
#define FIRST_SUPPORTED_COMPRESSED_COLUMNS_VERSION 50633

C_MODE_START

extern const char *my_defaults_extra_file;
extern const char *my_defaults_group_suffix;
extern const char *my_defaults_file;
extern my_bool my_getopt_use_args_separator;
extern my_bool my_defaults_read_login_file;

/* Define the type of function to be passed to process_default_option_files */
typedef int (*Process_option_func)(void *ctx, const char *group_name,
                                   const char *option);

my_bool my_getopt_is_args_separator(const char* arg);
int get_defaults_options(int argc, char **argv,
                         char **defaults, char **extra_defaults,
                         char **group_suffix, char **login_path,
                         my_bool found_no_defaults);
int my_load_defaults(const char *conf_file, const char **groups,
                     int *argc, char ***argv, const char ***);
int check_file_permissions(const char *file_name, my_bool is_login_file);
int load_defaults(const char *conf_file, const char **groups,
                  int *argc, char ***argv);
int my_search_option_files(const char *conf_file, int *argc,
                           char ***argv, uint *args_used,
                           Process_option_func func, void *func_ctx,
                           const char **default_directories,
                           my_bool is_login_file, my_bool found_no_defaults);
void free_defaults(char **argv);
void my_print_default_files(const char *conf_file);
void print_defaults(const char *conf_file, const char **groups);

C_MODE_END

#endif  // MY_DEFAULT_INCLUDED
