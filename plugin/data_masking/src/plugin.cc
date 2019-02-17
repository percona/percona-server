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

#include "plugin/data_masking/include/plugin.h"
#include "plugin/data_masking/include/udf/udf_registration.h"

static int data_masking_plugin_init(void *p) {
  DBUG_ENTER("data_masking_plugin_init");

  sql_print_information(
      "DataMasking Plugin: Initializing data masking dictionary memory "
      "structures");

  init_data_masking_memory();
  if (!g_data_masking_dict) {
    sql_print_error("DataMasking Plugin: ERROR reserving memory");
    DBUG_RETURN(1);
  }

  struct st_plugin_int *plugin = (struct st_plugin_int *)p;
  plugin->data = (void *)g_data_masking_dict;

  register_udfs();

  DBUG_RETURN(0);
}

static int data_masking_plugin_deinit(void *p) {
  DBUG_ENTER("data_masking_plugin_deinit");
  sql_print_information(
      "DataMasking Plugin: Deinitializing plugin memory structures");

  deinit_data_masking_memory();

  struct st_plugin_int *plugin = (struct st_plugin_int *)p;
  plugin->data = (void *)NULL;

  unregister_udfs();

  DBUG_RETURN(0);
}

struct st_mysql_daemon data_masking_plugin = {MYSQL_DAEMON_INTERFACE_VERSION};

//#define mysql_declare_plugin_end   ,{0,0,0,0,0,0,0,0,0,0,0,0,0,0}} <- 14 v8.0
//#define mysql_declare_plugin_end   ,{0,0,0,0,0,0,0,0,0,0,0,0,0}}   <- 13 v5.7

mysql_declare_plugin(data_masking){
    MYSQL_DAEMON_PLUGIN,             // Type
    &data_masking_plugin,            // info struct for plugin type
    "data_masking",                  // Short name
    "Francisco Miguel Biete Banon",  // Author
    "Data Masking plugin",           // Long name
    PLUGIN_LICENSE_GPL,              // License
    data_masking_plugin_init,        // Init function
    NULL,                            // Uninstall function
    data_masking_plugin_deinit,      // Deinit function
    0x0100,                          // version
    NULL,                            // status variables
    NULL,                            // system variables
    NULL,                            // config options
    0,                               // flags
} mysql_declare_plugin_end;
