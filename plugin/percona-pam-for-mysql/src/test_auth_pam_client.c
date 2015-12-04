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

 PAM authentication for MySQL, the test version of the client-side plugin.
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>

#define STDCALL

#include <mysql/plugin_auth.h>
#include <mysql/client_plugin.h>

#include "lib_auth_pam_client.h"

const char * echo_off_reply_1 = "aaaaaaa";
const char * echo_off_reply_2 = "AAAAAAA";

const char * echo_on_reply_1 = "bbbbbbbbbb";
const char * echo_on_reply_2 = "BBBBBBBBBB";

/* Returns alternating echo_off_reply_1 and echo_off_reply_2 */
static char * test_prompt_echo_off (const char * prompt __attribute__((unused)))
{
  static unsigned call_no= 0;
  return strdup((call_no++ % 2) == 0 ? echo_off_reply_1 : echo_off_reply_2);
}

/* Returns alternating echo_on_reply_1 and echo_on_reply_2 */
static char * test_prompt_echo_on (const char * prompt __attribute__((unused)))
{
  static unsigned call_no= 0;
  return strdup((call_no++ % 2) == 0 ? echo_on_reply_1 : echo_on_reply_2);
}

/* Pretend we have shown the message to the user */
static void test_show_anything(const char * message __attribute__((unused)))
{
}

static int test_pam_auth_client (MYSQL_PLUGIN_VIO *vio, struct st_mysql *mysql)
{
  return authenticate_user_with_pam_client_common (vio, mysql,
                                                   &test_prompt_echo_off,
                                                   &test_prompt_echo_on,
                                                   &test_show_anything,
                                                   &test_show_anything);
}

mysql_declare_client_plugin(AUTHENTICATION)
  "auth_pam_test",
  "Percona, Inc.",
  "Test version of the client PAM authentication plugin. "
  "DO NOT USE IN PRODUCTION.",
  {0,1,0},
  "GPL",
  NULL,
  NULL, /* init */
  NULL, /* deinit */
  NULL, /* options */
  &test_pam_auth_client
mysql_end_client_plugin;
