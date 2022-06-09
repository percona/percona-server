/*
(C) 2012, 2015 Percona LLC and/or its affiliates

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/

/**
 @file

 PAM authentication for MySQL, server-side plugin for the
 production use.

 Oracle MySQL-compatible plugin.  Acts as a mediator
 between the MySQL server, the MySQL client, and the PAM backend.

 The server plugin requests authentication from the PAM backend, and reads one
 phrase from client plugin. mysql_clear_password plugin used as client plugin.

 This plugin does not encrypt the communication channel in any way.  If this is
 required, a SSL connection should be used.

 To install this plugin, copy the .so file to the plugin directory and do

 INSTALL PLUGIN auth_pam SONAME 'auth_pam_compat.so';

 To use this plugin for one particular user, specify it at user's creation time
 (TODO: tested with localhost only):

 CREATE USER 'username'@'hostname' IDENTIFIED WITH auth_pam_compat;

 Alternatively UPDATE the mysql.user table to set the plugin value for an
 existing user.

 Also it is possible to use this plugin to authenticate anonymous users:

 CREATE USER ''@'hostname' IDENTIFIED WITH auth_pam_compat;

*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include "auth_pam_common.h"
#include "my_sys.h"

MYSQL_PLUGIN auth_pam_plugin_info;

int auth_pam_client_talk_init(void **talk_data) {
  int *num_talks = static_cast<int *>(
      my_malloc(PSI_NOT_INSTRUMENTED, sizeof(int), MY_ZEROFILL));
  *talk_data = (void *)num_talks;
  return (num_talks != nullptr) ? PAM_SUCCESS : PAM_BUF_ERR;
}

void auth_pam_client_talk_finalize(void *talk_data) { my_free(talk_data); }

int auth_pam_talk_perform(const struct pam_message *msg,
                          struct pam_response *resp, struct pam_conv_data *data,
                          void *talk_data) {
  if (msg->msg_style == PAM_PROMPT_ECHO_OFF ||
      msg->msg_style == PAM_PROMPT_ECHO_ON) {
    /* mysql_clear_password plugin has support for only single phrase */
    int *num_talks = (int *)talk_data;
    if (*num_talks > 1) return PAM_CONV_ERR;

    /* Read the answer */
    unsigned char *pkt;
    int pkt_len = data->vio->read_packet(data->vio, &pkt);
    if (pkt_len < 0) return PAM_CONV_ERR;

    resp->resp = static_cast<char *>(malloc(pkt_len + 1));
    if (resp->resp == nullptr) return PAM_BUF_ERR;

    strncpy(resp->resp, (char *)pkt, pkt_len);
    resp->resp[pkt_len] = '\0';

    /**
      we could only guess whether password was used or not
      normally we would set PASSWORD_USED_NO_MENTION but
      because of http://bugs.mysql.com/bug.php?id=72536
      we set PASSWORD_USED_YES.
    */
    data->info->password_used = PASSWORD_USED_YES;
    ++(*num_talks);
  }

  return PAM_SUCCESS;
}

static int auth_pam_compat_init(MYSQL_PLUGIN plugin_info) {
  auth_pam_common_init("auth_pam_compat");
  auth_pam_plugin_info = plugin_info;
  return 0;
}

static struct st_mysql_auth pam_auth_handler = {
    MYSQL_AUTHENTICATION_INTERFACE_VERSION,
    "mysql_clear_password",
    &authenticate_user_with_pam_server,
    &auth_pam_generate_auth_string_hash,
    &auth_pam_validate_auth_string_hash,
    &auth_pam_set_salt,
    0UL,
    nullptr};

mysql_declare_plugin(auth_pam) {
  MYSQL_AUTHENTICATION_PLUGIN, &pam_auth_handler, "auth_pam_compat",
      "Percona, Inc.", "PAM authentication plugin", PLUGIN_LICENSE_GPL,
      auth_pam_compat_init, nullptr, nullptr, 0x0001, nullptr, nullptr, nullptr
#if MYSQL_PLUGIN_INTERFACE_VERSION >= 0x103
      ,
      0
#endif
}
mysql_declare_plugin_end;
