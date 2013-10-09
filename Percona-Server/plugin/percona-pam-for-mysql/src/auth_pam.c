/*
(C) 2012, 2013 Percona LLC and/or its affiliates

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

 A general-purpose PAM authentication plugin for MySQL.  Acts as a mediator
 between the MySQL server, the MySQL client, and the PAM backend. Dialog plugin
 used as client plugin.

 The server plugin requests authentication from the PAM backend, forwards any
 requests and messages from the PAM backend over the wire to the client (in
 cleartext) and reads back any replies for the backend.

 This plugin does not encrypt the communication channel in any way.  If this is
 required, a SSL connection should be used.

 To install this plugin, copy the .so file to the plugin directory and do

 INSTALL PLUGIN auth_pam SONAME 'auth_pam.so';

 To use this plugin for one particular user, specify it at user's creation time
 (TODO: tested with localhost only):

 CREATE USER 'username'@'hostname' IDENTIFIED WITH auth_pam;

 Alternatively UPDATE the mysql.user table to set the plugin value for an
 existing user.

 Also it is possible to use this plugin to authenticate anonymous users:

 CREATE USER ''@'hostname' IDENTIFIED WITH auth_pam;

*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include "auth_pam_common.h"

/** The maximum length of buffered PAM messages, i.e. any messages up to the
    next PAM reply-requiring message. 10K should be more than enough by order
    of magnitude. */
enum { max_pam_buffered_msg_len = 10240 };

struct pam_msg_buf {
    unsigned char buf[max_pam_buffered_msg_len];
    unsigned char* ptr;
};

static char pam_msg_style_to_char (int pam_msg_style)
{
  /* Magic byte for the dialog plugin, '\2' is defined as ORDINARY_QUESTION
     and '\4' as PASSWORD_QUESTION there. */
  return (pam_msg_style == PAM_PROMPT_ECHO_ON) ? '\2' : '\4';
}

int auth_pam_client_talk_init(void **talk_data)
{
  struct pam_msg_buf *msg_buf= calloc(1, sizeof(struct pam_msg_buf));
  *talk_data= (void*)msg_buf;
  if (msg_buf != NULL)
  {
    msg_buf->ptr= msg_buf->buf + 1;
    return PAM_SUCCESS;
  }
  return PAM_BUF_ERR;
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
  struct pam_msg_buf *msg_buf= (struct pam_msg_buf*)talk_data;

  /* Append the PAM message or prompt to the unsent message buffer */
  if (msg->msg)
  {
    unsigned char *last_buf_pos = msg_buf->buf + max_pam_buffered_msg_len - 1;
    if (msg_buf->ptr + strlen(msg->msg) >= last_buf_pos)
    {
      /* Cannot happen: the PAM message buffer too small. */
      MY_ASSERT_UNREACHABLE();
      return PAM_CONV_ERR;
    }
    memcpy(msg_buf->ptr, msg->msg, strlen(msg->msg));
    msg_buf->ptr+= strlen(msg->msg);
    *(msg_buf->ptr)++= '\n';
  }

  if (msg->msg_style == PAM_PROMPT_ECHO_OFF
      || msg->msg_style == PAM_PROMPT_ECHO_ON)
  {
    int pkt_len;
    unsigned char *pkt;

    msg_buf->buf[0]= pam_msg_style_to_char(msg->msg_style);

    /* Write the message.  */
    if (data->vio->write_packet(data->vio, msg_buf->buf,
                                msg_buf->ptr - msg_buf->buf - 1))
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

    if (msg->msg_style == PAM_PROMPT_ECHO_OFF)
      data->info->password_used= PASSWORD_USED_YES;

    msg_buf->ptr= msg_buf->buf + 1;
  }

  return PAM_SUCCESS;
}

static struct st_mysql_auth pam_auth_handler=
{
  MYSQL_AUTHENTICATION_INTERFACE_VERSION,
  "dialog",
  &authenticate_user_with_pam_server
};

mysql_declare_plugin(auth_pam)
{
  MYSQL_AUTHENTICATION_PLUGIN,
  &pam_auth_handler,
  "auth_pam",
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
