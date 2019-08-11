/* Copyright (c) 2019 Francisco Miguel Biete Banon. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   51 Franklin Street, Suite 500, Boston, MA 02110-1335 USA
*/

#include "plugin/mail_udf/include/plugin.h"
#include "plugin/mail_udf/include/udf_registration.h"

static int mail_plugin_init(void *) {
  DBUG_ENTER("mail_plugin_init");

  CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
  if (res != CURLE_OK) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                 "Mail Plugin: ERROR initilizing CURL");
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, curl_easy_strerror(res));
    DBUG_RETURN(1);
  }

  register_udfs();

  DBUG_RETURN(0);
}

static int mail_plugin_deinit(void *) {
  DBUG_ENTER("mail_plugin_deinit");

  curl_global_cleanup();

  unregister_udfs();

  DBUG_RETURN(0);
}

struct st_mysql_daemon mail_plugin = {MYSQL_DAEMON_INTERFACE_VERSION};

mysql_declare_plugin(mail){
    MYSQL_DAEMON_PLUGIN,                     // Type
    &mail_plugin,                            // info struct for plugin type
    "mail",                                  // Short name
    "Oracle. Francisco Miguel Biete Banon",  // Author
    "Mail plugin",                           // Long name
    PLUGIN_LICENSE_GPL,                      // License
    mail_plugin_init,                        // Init function
    NULL,                                    // Uninstall function
    mail_plugin_deinit,                      // Deinit function
    0x0100,                                  // version
    NULL,                                    // status variables
    NULL,                                    // system variables
    NULL,                                    // config options
    0,                                       // flags
} mysql_declare_plugin_end;
