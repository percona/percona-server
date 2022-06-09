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

#include <stdlib.h>
#include <string.h>
#include "auth_pam_common.h"
#include "my_sys.h"
#include "mysql/psi/mysql_memory.h"

MYSQL_PLUGIN auth_pam_plugin_info;

/** The maximum length of buffered PAM messages, i.e. any messages up to the
    next PAM reply-requiring message. 10K should be more than enough by order
    of magnitude. */
static const constexpr auto max_pam_buffered_msg_len = 10240;

static PSI_memory_key key_memory_pam_msg_buf;

static PSI_memory_info pam_auth_memory[] = {
    {&key_memory_pam_msg_buf, "auth_pam_msg_buf", PSI_FLAG_ONLY_GLOBAL_STAT,
     PSI_VOLATILITY_UNKNOWN, PSI_DOCUMENT_ME},
};

struct pam_msg_buf {
  unsigned char buf[max_pam_buffered_msg_len];
  unsigned char *ptr;
};

static char pam_msg_style_to_char(int pam_msg_style) {
  /* Magic byte for the dialog plugin, '\2' is defined as ORDINARY_QUESTION
     and '\4' as PASSWORD_QUESTION there. */
  return (pam_msg_style == PAM_PROMPT_ECHO_ON) ? '\2' : '\4';
}

int auth_pam_client_talk_init(void **talk_data) {
  struct pam_msg_buf *msg_buf = static_cast<pam_msg_buf *>(my_malloc(
      key_memory_pam_msg_buf, sizeof(struct pam_msg_buf), MY_ZEROFILL));
  *talk_data = (void *)msg_buf;
  if (msg_buf != nullptr) {
    msg_buf->ptr = msg_buf->buf + 1;
    return PAM_SUCCESS;
  }
  return PAM_BUF_ERR;
}

void auth_pam_client_talk_finalize(void *talk_data) { my_free(talk_data); }

int auth_pam_talk_perform(const struct pam_message *msg,
                          struct pam_response *resp, struct pam_conv_data *data,
                          void *talk_data) {
  struct pam_msg_buf *msg_buf = (struct pam_msg_buf *)talk_data;

  /* Append the PAM message or prompt to the unsent message buffer */
  if (msg->msg) {
    unsigned char *last_buf_pos = msg_buf->buf + max_pam_buffered_msg_len - 1;
    if (msg_buf->ptr + strlen(msg->msg) >= last_buf_pos) {
      /* Cannot happen: the PAM message buffer too small. */
      MY_ASSERT_UNREACHABLE();
      return PAM_CONV_ERR;
    }
    memcpy(msg_buf->ptr, msg->msg, strlen(msg->msg));
    msg_buf->ptr += strlen(msg->msg);
    *(msg_buf->ptr)++ = '\n';
  }

  if (msg->msg_style == PAM_PROMPT_ECHO_OFF ||
      msg->msg_style == PAM_PROMPT_ECHO_ON) {
    int pkt_len;
    unsigned char *pkt;

    msg_buf->buf[0] = pam_msg_style_to_char(msg->msg_style);

    /* Write the message.  */
    if (data->vio->write_packet(data->vio, msg_buf->buf,
                                msg_buf->ptr - msg_buf->buf - 1))
      return PAM_CONV_ERR;

    /* Read the answer */
    if ((pkt_len = data->vio->read_packet(data->vio, &pkt)) < 0)
      return PAM_CONV_ERR;

    resp->resp = static_cast<char *>(malloc(pkt_len + 1));
    if (resp->resp == nullptr) return PAM_BUF_ERR;

    strncpy(resp->resp, (char *)pkt, pkt_len);
    resp->resp[pkt_len] = '\0';

    if (msg->msg_style == PAM_PROMPT_ECHO_OFF)
      data->info->password_used = PASSWORD_USED_YES;

    msg_buf->ptr = msg_buf->buf + 1;
  }

  return PAM_SUCCESS;
}

static int auth_pam_init(MYSQL_PLUGIN plugin_info) {
  int count;
  auth_pam_common_init("auth_pam");
  count = array_elements(pam_auth_memory);
  mysql_memory_register("auth_pam", pam_auth_memory, count);
  auth_pam_plugin_info = plugin_info;
  return 0;
}

static struct st_mysql_auth pam_auth_handler = {
    MYSQL_AUTHENTICATION_INTERFACE_VERSION,
    "dialog",
    &authenticate_user_with_pam_server,
    &auth_pam_generate_auth_string_hash,
    &auth_pam_validate_auth_string_hash,
    &auth_pam_set_salt,
    0UL,
    nullptr};

mysql_declare_plugin(auth_pam) {
  MYSQL_AUTHENTICATION_PLUGIN, &pam_auth_handler, "auth_pam", "Percona, Inc.",
      "PAM authentication plugin", PLUGIN_LICENSE_GPL, auth_pam_init, nullptr,
      nullptr, 0x0001, nullptr, nullptr, nullptr
#if MYSQL_PLUGIN_INTERFACE_VERSION >= 0x103
      ,
      0
#endif
}
mysql_declare_plugin_end;
