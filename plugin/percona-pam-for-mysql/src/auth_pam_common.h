#ifndef AUTH_PAM_COMMON_INCLUDED
#define AUTH_PAM_COMMON_INCLUDED
/*
  (C) 2012 Percona Inc.

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

 PAM authentication for MySQL, common definitions for side plugins.

 For the general description, see the top comment in auth_pam_common.c.
*/

/* Define these macros ourselves, so we don't have to include my_global.h and
can compile against unconfigured MySQL source tree.  */
#define STDCALL

#include <security/pam_appl.h>
#include <security/pam_modules.h>
#if HAVE_SECURITY_PAM_MISC_H
#include <security/pam_misc.h>
#elif HAVE_SECURITY_OPENPAM_H
#include <security/openpam.h>
#endif

#include <mysql/plugin.h>
#include <mysql/plugin_auth.h>
#include <mysql/client_plugin.h>

#include <assert.h>

#define MY_ASSERT_UNREACHABLE() assert(0)

#ifdef __cplusplus
extern "C" {
#endif

struct pam_conv_data {
    MYSQL_PLUGIN_VIO *vio;
    MYSQL_SERVER_AUTH_INFO *info;
};

/** Define following three functions for your specific client plugin */

int auth_pam_client_talk_init(void **talk_data);

int auth_pam_talk_perform(const struct pam_message *msg,
                          struct pam_response *resp,
                          struct pam_conv_data *data,
                          void *talk_data);

void auth_pam_client_talk_finalize(void *talk_data);

int authenticate_user_with_pam_server (MYSQL_PLUGIN_VIO *vio,
                                       MYSQL_SERVER_AUTH_INFO *info);

#ifdef __cplusplus
}
#endif

#endif
