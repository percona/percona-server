#ifndef LIB_AUTH_PAM_CLIENT_INCLUDED
#define LIB_AUTH_PAM_CLIENT_INCLUDED
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

 PAM authentication for MySQL, common definitions for client-side plugins.

 For the general description, see the top comment in auth_pam.c.
*/

#define STDCALL

#include <mysql/client_plugin.h>

/**
 Callback type for functions that prompt the user for (echoed or silent) input
 and return it.  Should returns a pointer to malloc-allocated string, the
 caller is responsible for freeing it.  Should return NULL in the case of a
 memory allocation or I/O error. */
typedef char* (*prompt_fn)(const char *);

/**
 Callback type for functions that show user some info (error or notification).
*/
typedef void (*info_fn)(const char *);

struct st_mysql;

#ifdef __cplusplus
extern "C" {
#endif

/**
 Client-side PAM auth plugin implementation.

 Communicates with the server-side plugin and does user interaction using the
 provided callbacks.

 @param vio TODO
 @param mysql TODO
 @param echoless_prompt_fn callback to use to prompt the user for non-echoed
                           input (e.g. password)
 @param echo_prompt_fn callback to use to prompt the user for echoed input
                       (e.g. user name)
 @param show_error_fn callback to use to show the user an error message
 @param show_info_fn callback to use to show the user an informational message

 @return Authentication conversation status
   @retval CR_OK the authentication dialog is completed successfully
   @retval CR_ERROR the authentication dialog is aborted due to error
*/
int authenticate_user_with_pam_client_common (MYSQL_PLUGIN_VIO *vio,
                                              struct st_mysql *mysql,
                                              prompt_fn echoless_prompt_fn,
                                              prompt_fn echo_prompt_fn,
                                              info_fn show_error_fn,
                                              info_fn show_info_fn);

#ifdef __cplusplus
}
#endif

#endif
