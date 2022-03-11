
/*
Copyright (c) 2021, Oracle and/or its affiliates.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2.0,
as published by the Free Software Foundation.

This program is also distributed with certain software (including
but not limited to OpenSSL) that is licensed under separate terms,
as designated in a particular file or component or in included license
documentation.  The authors of MySQL hereby grant you an additional
permission to link the program and your derivative works with the
separately licensed software that they have included with MySQL.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License, version 2.0, for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include <sstream>
#include <vector>

#include "user_registration.h"

#include "my_hostname.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysql/service_mysql_alloc.h"  // my_malloc
#include "mysqld_error.h"

/**
  This helper method parses --fido-register-factor option values, and
  inserts the parsed values in list.

  @param[in] what_factor      comma separated string containing what all factors
                              are to be registered
  @param[out] list            container holding individual factors

  @return true failed
  @return false success
*/
static bool parse_register_option(char *what_factor,
                                  std::vector<unsigned int> &list) {
  std::string token;
  std::stringstream str(what_factor);
  while (getline(str, token, ',')) {
    unsigned int nth_factor = 0;
    try {
      nth_factor = std::stoul(token);
    } catch (std::invalid_argument &) {
      return true;
    } catch (std::out_of_range &) {
      return true;
    }
    /* nth_factor can be either 2 or 3 */
    if (nth_factor < 2 || nth_factor > 3) return true;
    list.push_back(nth_factor);
  }
  return false;
}

/**
  This helper method is used to perform device registration against a user
  account. Below are the messages exchanged between client, server and
  authenticator (fido device) during registration process.

  == Initiate registration ==

  client -> server : connect
  server -> client : keep connection in registration mode
  client -> server : ALTER USER account name nth FACTOR INITIATE REGISTRATION
  server -> client : random challenge, user id, relying party ID

  == Finish registration ==

  client -> server : ALTER USER account name nth FACTOR FINISH REGISTRATION
  client -> authenticator : random challenge, user id, relying party ID
  authenticator -> client : public key, credential ID (X.509 certificate,
  signature) client -> server : public key, credential ID server -> client : Ok
  packet upon successful verification of signature

  @param mysql              mysql connection handle
  @param register_option    Comma separated list of values, which specifies
  which factor requires registration. Valid values are "2", "3", "2,3" or "3,2"
  @param errmsg             Buffer tol hold error message in case of error.

  @return true failed
  @return false success
*/
bool user_device_registration(MYSQL *mysql, char *register_option,
                              char *errmsg) {
  char query[1024] = {0};
  MYSQL_RES *result;
  MYSQL_ROW row;
  ulong *lengths;
  uchar *server_challenge = nullptr;
  uchar *server_challenge_response = nullptr;
  MYSQL_STMT *finish_reg_stmt;
  MYSQL_BIND rs_bind;
  uchar *pos = nullptr;

  if (!mysql) {
    sprintf(errmsg, "MySQL internal error. ");
    return true;
  }

  std::vector<unsigned int> list;
  if (parse_register_option(register_option, list)) {
    sprintf(errmsg,
            "Incorrect value specified for --fido-register-factor option. "
            "Correct values can be '2', '3', '2,3' or '3,2'.");
    return true;
  }

  for (auto f : list) {
    sprintf(query, "ALTER USER USER() %d FACTOR INITIATE REGISTRATION", f);
    if (mysql_real_query(mysql, query, (ulong)strlen(query))) {
      sprintf(errmsg, "Initiate registration failed with error: %s. ",
              mysql_error(mysql));
      return true;
    }
    if (!(result = mysql_store_result(mysql))) {
      sprintf(errmsg, "Initiate registration failed with error: %s. ",
              mysql_error(mysql));
      return true;
    }
    if (mysql_num_rows(result) > 1) {
      sprintf(errmsg, "Initiate registration failed with error: %s. ",
              mysql_error(mysql));
      mysql_free_result(result);
      return true;
    }
    row = mysql_fetch_row(result);
    lengths = mysql_fetch_lengths(result);
    /*
      max length of challenge can be 32 (random challenge) +
      255 (relying party ID) + 255 (host name) + 32 (user name) + 4 byte for
      length encodings
    */
    if (lengths[0] > (CHALLENGE_LENGTH + RELYING_PARTY_ID_LENGTH +
                      HOSTNAME_LENGTH + USERNAME_LENGTH + 4)) {
      sprintf(errmsg, "Received server challenge is corrupt. Please retry.\n");
      mysql_free_result(result);
      return true;
    }
    server_challenge = static_cast<uchar *>(my_malloc(
        PSI_NOT_INSTRUMENTED, lengths[0] + 1, MYF(MY_WME | MY_ZEROFILL)));
    memcpy(server_challenge, row[0], lengths[0]);
    mysql_free_result(result);

    /* load fido client authentication plugin if required */
    struct st_mysql_client_plugin *p =
        mysql_client_find_plugin(mysql, "authentication_fido_client",
                                 MYSQL_CLIENT_AUTHENTICATION_PLUGIN);
    if (!p) {
      my_free(server_challenge);
      sprintf(
          errmsg,
          "Loading authentication_fido_client plugin failed with error: %s. ",
          mysql_error(mysql));
      return true;
    }
    /* set server challenge in plugin */
    if (mysql_plugin_options(p, "registration_challenge", server_challenge)) {
      my_free(server_challenge);
      sprintf(errmsg,
              "Failed to set plugin options \"registration_challenge\".\n");
      return true;
    }
    my_free(server_challenge);
    server_challenge = nullptr;
    /* get challenge response from plugin */
    if (mysql_plugin_get_option(p, "registration_response",
                                &server_challenge_response)) {
      sprintf(errmsg,
              "Failed to get plugin options \"registration_response\".\n");
      return true;
    }
    pos = server_challenge_response;

    finish_reg_stmt = mysql_stmt_init(mysql);
    /* execute FINISH REGISTRATION sql */
    sprintf(query,
            "ALTER USER USER() %d FACTOR FINISH REGISTRATION SET "
            "CHALLENGE_RESPONSE AS '%s'",
            f, server_challenge_response);
    if (mysql_stmt_prepare(finish_reg_stmt, query, (ulong)strlen(query))) {
      goto error;
    }
    /* Bind input buffers */
    memset(&rs_bind, 0, sizeof(rs_bind));
    rs_bind.buffer_type = MYSQL_TYPE_STRING;
    rs_bind.buffer = (char *)(pos);
    rs_bind.buffer_length =
        (ulong)strlen(reinterpret_cast<char *>(server_challenge_response));
    rs_bind.is_null = nullptr;

    if (mysql_stmt_bind_param(finish_reg_stmt, &rs_bind)) {
      goto error;
    }
    if (mysql_stmt_execute(finish_reg_stmt)) {
      goto error;
    }
    if (mysql_stmt_close(finish_reg_stmt)) {
      goto error;
    }
  }
  return false;
error:
  sprintf(errmsg, "Finish registration failed with error: %s.\n",
          mysql_stmt_error(finish_reg_stmt));
  return true;
}