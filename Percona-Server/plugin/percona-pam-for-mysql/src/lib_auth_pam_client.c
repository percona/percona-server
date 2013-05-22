/*
  (C) 2011 Percona Inc.

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

 PAM authentication for MySQL, common code for client-side plugins.

 For the general description, see the top comment in auth_pam.c.
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "lib_auth_pam_client.h"

#include <assert.h>
#include <string.h>

#define MY_ASSERT_UNREACHABLE() assert(0)

int authenticate_user_with_pam_client_common (MYSQL_PLUGIN_VIO *vio,
                                              struct st_mysql *mysql __attribute__((unused)),
                                              prompt_fn echoless_prompt_fn,
                                              prompt_fn echo_prompt_fn,
                                              info_fn show_error_fn,
                                              info_fn show_info_fn)
{
  do {
    char *buf;
    int pkt_len;

    if ((pkt_len= vio->read_packet(vio, (unsigned char **)&buf)) < 0)
      return CR_ERROR;

    /* The first byte is the message type, followed by the message itself.  */

    if (buf[0] == '\2' || buf[0] == '\3')
    {
      /* '\2' - PAM_PROMPT_ECHO_OFF, '\3' - PAM_PROMPT_ECHO_ON */
      char *reply = (buf[0] == '\2')
        ? echoless_prompt_fn(&buf[1]) : echo_prompt_fn(&buf[1]);
      if (!reply)
        return CR_ERROR;
      if (vio->write_packet(vio, (unsigned char *)reply, strlen(reply) + 1))
      {
        free(reply);
        return CR_ERROR;
      }
      free(reply);
    }
    else if (buf[0] == '\4') /* PAM_ERROR_MSG */
      show_error_fn(&buf[1]);
    else if (buf[0] == '\5') /* PAM_TEXT_INFO */
      show_info_fn(&buf[1]);
    else if (buf[0] == '\0') /* end-of-authorization */
      return CR_OK;
    else
      return CR_ERROR; /* Unknown! */
  }
  while (1);

  /* Should not come here */
  MY_ASSERT_UNREACHABLE();
  return CR_ERROR;
}
