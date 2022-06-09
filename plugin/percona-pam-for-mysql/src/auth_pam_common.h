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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/

/**
 @file

 PAM authentication for MySQL, common definitions for side plugins.

 For the general description, see the top comment in auth_pam_common.c.
*/

#include <security/pam_appl.h>
#include <security/pam_modules.h>
#ifdef HAVE_SECURITY_PAM_MISC_H
#include <security/pam_misc.h>
#elif defined(HAVE_SECURITY_OPENPAM_H)
#include <security/openpam.h>
#endif

#include "mysql/client_plugin.h"
#include "mysql/plugin.h"
#include "mysql/plugin_auth.h"

#ifdef __cplusplus
extern "C" {
#endif

struct pam_conv_data {
  MYSQL_PLUGIN_VIO *vio;
  MYSQL_SERVER_AUTH_INFO *info;
};

extern MYSQL_PLUGIN auth_pam_plugin_info;

extern PSI_memory_key key_memory_pam_mapping_iter;
extern PSI_memory_key key_memory_pam_group_iter;

void auth_pam_common_init(const char *psi_category);

/** Define following three functions for your specific client plugin */

int auth_pam_client_talk_init(void **talk_data);

int auth_pam_talk_perform(const struct pam_message *msg,
                          struct pam_response *resp, struct pam_conv_data *data,
                          void *talk_data);

void auth_pam_client_talk_finalize(void *talk_data);

int authenticate_user_with_pam_server(MYSQL_PLUGIN_VIO *vio,
                                      MYSQL_SERVER_AUTH_INFO *info);

int auth_pam_generate_auth_string_hash(char *outbuf, unsigned int *buflen,
                                       const char *inbuf,
                                       unsigned int inbuflen);

int auth_pam_validate_auth_string_hash(char *const buf, unsigned int len);

int auth_pam_set_salt(const char *password, unsigned int password_len,
                      unsigned char *salt, unsigned char *salt_len);

#ifdef __cplusplus
}
#endif

#endif
