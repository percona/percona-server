/*
(C) 2011-2015 Percona LLC and/or its affiliates

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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include "auth_mapping.h"
#include "auth_pam_common.h"
#include "groups.h"
#include "my_sys.h"
#include "mysql/psi/mysql_memory.h"

/* The server plugin */

PSI_memory_key key_memory_pam_mapping_iter;
PSI_memory_key key_memory_pam_group_iter;

static PSI_memory_info common_pam_memory[] = {
    {&key_memory_pam_mapping_iter, "auth_pam_mapping_iterator",
     PSI_FLAG_ONLY_GLOBAL_STAT, PSI_VOLATILITY_UNKNOWN, PSI_DOCUMENT_ME},
    {&key_memory_pam_group_iter, "auth_pam_group_iterator",
     PSI_FLAG_ONLY_GLOBAL_STAT, PSI_VOLATILITY_UNKNOWN, PSI_DOCUMENT_ME},
};

/** The MySQL service name for PAM configuration */
static const char *service_name_default = "mysqld";

void auth_pam_common_init(const char *psi_category) {
  int count = array_elements(common_pam_memory);
  mysql_memory_register(psi_category, common_pam_memory, count);
}

static bool valid_pam_msg_style(int pam_msg_style) {
  switch (pam_msg_style) {
    case PAM_PROMPT_ECHO_OFF:
    case PAM_PROMPT_ECHO_ON:
    case PAM_ERROR_MSG:
    case PAM_TEXT_INFO:
      return true;
    default:
      return false;
  }
}

/** The maximum length of service name. It shouldn't be too long as it's
    filename in pam.d directory also */
static const constexpr auto max_pam_service_name_len = 64;

static void free_pam_response(struct pam_response **resp, int n) {
  for (int i = 0; i < n; i++) {
    free((*resp)[i].resp);
  }
  free(*resp);
  *resp = nullptr;
}

static int vio_server_conv(int num_msg, const struct pam_message **msg,
                           struct pam_response **resp, void *appdata_ptr) {
  struct pam_conv_data *data = (struct pam_conv_data *)appdata_ptr;

  if (data == nullptr) {
    MY_ASSERT_UNREACHABLE();
    return PAM_CONV_ERR;
  }

  *resp = (struct pam_response *)calloc(num_msg, sizeof(struct pam_response));
  if (*resp == nullptr) return PAM_BUF_ERR;

  void *talk_data;
  int error = auth_pam_client_talk_init(&talk_data);
  if (error != PAM_SUCCESS) {
    free_pam_response(resp, 0);
    return error;
  }

  for (int i = 0; i < num_msg; i++) {
    if (!valid_pam_msg_style(msg[i]->msg_style)) {
      auth_pam_client_talk_finalize(talk_data);
      free_pam_response(resp, i);
      return PAM_CONV_ERR;
    }

    error = auth_pam_talk_perform(msg[i], &(*resp)[i], data, talk_data);
    if (error != PAM_SUCCESS) {
      auth_pam_client_talk_finalize(talk_data);
      free_pam_response(resp, i);
      return error;
    }
  }
  auth_pam_client_talk_finalize(talk_data);
  return PAM_SUCCESS;
}

int authenticate_user_with_pam_server(MYSQL_PLUGIN_VIO *vio,
                                      MYSQL_SERVER_AUTH_INFO *info) {
  /* Set service name as specified in auth_string. If no auth_string
  provided or parsing error occurs, then keep default value */
  char service_name[max_pam_service_name_len];
  strcpy(service_name, service_name_default);
  if (info->auth_string)
    mapping_get_service_name(service_name, sizeof(service_name),
                             info->auth_string);

  info->password_used = PASSWORD_USED_NO_MENTION;

  pam_handle_t *pam_handle;
  struct pam_conv_data data = {vio, info};
  struct pam_conv conv_func_info = {&vio_server_conv, &data};
  int error =
      pam_start(service_name, info->user_name, &conv_func_info, &pam_handle);
  if (error != PAM_SUCCESS) return CR_ERROR;

  error = pam_set_item(pam_handle, PAM_RUSER, info->user_name);
  if (error != PAM_SUCCESS) {
    pam_end(pam_handle, error);
    return CR_ERROR;
  }

  error = pam_set_item(pam_handle, PAM_RHOST, info->host_or_ip);
  if (error != PAM_SUCCESS) {
    pam_end(pam_handle, error);
    return CR_ERROR;
  }

  error = pam_authenticate(pam_handle, 0);
  if (error != PAM_SUCCESS) {
    pam_end(pam_handle, error);
    return CR_ERROR;
  }

  error = pam_acct_mgmt(pam_handle, 0);
  if (error != PAM_SUCCESS) {
    pam_end(pam_handle, error);
    return CR_ERROR;
  }

  /* Get the authenticated user name from PAM */
  char *pam_mapped_user_name;
  error = pam_get_item(pam_handle, PAM_USER,
                       const_cast<const void **>(
                           reinterpret_cast<void **>(&pam_mapped_user_name)));
  if (error != PAM_SUCCESS) {
    pam_end(pam_handle, error);
    return CR_ERROR;
  }

  /* Check if user name from PAM is the same as provided for MySQL.  If
  different, use the new user name for MySQL authorization and as
  CURRENT_USER() value.  */
  if (strcmp(info->user_name, pam_mapped_user_name)) {
    strncpy(info->authenticated_as, pam_mapped_user_name,
            MYSQL_USERNAME_LENGTH);
    info->authenticated_as[MYSQL_USERNAME_LENGTH] = '\0';
  }

  if (info->auth_string) {
    mapping_lookup_user(pam_mapped_user_name, info->authenticated_as,
                        MYSQL_USERNAME_LENGTH, info->auth_string);
  }

  error = pam_end(pam_handle, error);
  if (error != PAM_SUCCESS) return CR_ERROR;

  return CR_OK;
}

int auth_pam_generate_auth_string_hash(char *outbuf, unsigned int *buflen,
                                       const char *inbuf,
                                       unsigned int inbuflen) {
  /*
    fail if buffer specified by server cannot be copied to output buffer
  */
  if (*buflen < inbuflen) return 1; /* error */
  strncpy(outbuf, inbuf, inbuflen);
  *buflen = strlen(inbuf);
  return 0; /* success */
}

int auth_pam_validate_auth_string_hash(char *const buf __attribute__((unused)),
                                       unsigned int len
                                       __attribute__((unused))) {
  return 0; /* success */
}

int auth_pam_set_salt(const char *password __attribute__((unused)),
                      unsigned int password_len __attribute__((unused)),
                      unsigned char *salt __attribute__((unused)),
                      unsigned char *salt_len) {
  *salt_len = 0;
  return 0; /* success */
}
