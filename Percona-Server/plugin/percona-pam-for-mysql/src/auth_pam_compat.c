/*
(C) 2012, 2013 Percona Percona LLC and/or its affiliates

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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

#include <string.h>
#include "auth_pam_common.h"

int auth_pam_client_talk_init(void **talk_data)
{
  int *num_talks= calloc(1, sizeof(int));
  *talk_data= (void*)num_talks;
  return (num_talks != NULL) ? PAM_SUCCESS : PAM_BUF_ERR;
}

void auth_pam_client_talk_finalize(void *talk_data)
{
  free(talk_data);
}

int auth_pam_talk_perform(const struct pam_message *msg,
                          struct pam_response *resp,
                          struct pam_conv_data *data,
                          void *talk_data)
{
  int pkt_len;
  unsigned char *pkt;
  int *num_talks= (int*)talk_data;

  if (msg->msg_style == PAM_PROMPT_ECHO_OFF
      || msg->msg_style == PAM_PROMPT_ECHO_ON)
  {
    /* mysql_clear_password plugin has support for only single phrase */
    if (*num_talks > 1)
      return PAM_CONV_ERR;

    /* Read the answer */
    if ((pkt_len= data->vio->read_packet(data->vio, &pkt))
        < 0)
      return PAM_CONV_ERR;

    resp->resp= malloc(pkt_len + 1);
    if (resp->resp == NULL)
      return PAM_BUF_ERR;

    strncpy(resp->resp, (char *)pkt, pkt_len);
    resp->resp[pkt_len]= '\0';

    /* we could only guess whether password was used or not */
    data->info->password_used= PASSWORD_USED_NO_MENTION;
    ++(*num_talks);
  }

  return PAM_SUCCESS;
}

static struct st_mysql_auth pam_auth_handler=
{
  MYSQL_AUTHENTICATION_INTERFACE_VERSION,
  "mysql_clear_password",
  &authenticate_user_with_pam_server
};

mysql_declare_plugin(auth_pam)
{
  MYSQL_AUTHENTICATION_PLUGIN,
  &pam_auth_handler,
  "auth_pam_compat",
  "Percona, Inc.",
  "PAM authentication plugin",
  PLUGIN_LICENSE_GPL,
  NULL,
  NULL,
  0x0001,
  NULL,
  NULL,
  NULL
#if MYSQL_PLUGIN_INTERFACE_VERSION >= 0x103
  ,
  0
#endif
}
mysql_declare_plugin_end;
