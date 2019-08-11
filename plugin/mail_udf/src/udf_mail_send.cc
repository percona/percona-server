/* Copyright (c) 2019 Francisco Miguel Biete Banon. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   51 Franklin Street, Suite 500, Boston, MA 02110-1335 USA
 */

#include "plugin/mail_udf/include/udf_mail_send.h"
#include "plugin/mail_udf/include/plugin.h"

#include <ctime>
#include <iostream>
#include <sstream>
#include <string>

#include <curl/curl.h>

static const std::string udf_mail_send_ok = "Mail sent";
static const std::string udf_mail_invalid_smtp_uri =
    "SMTP URI must look like smtp://server.domain:port or "
    "smtps://server.domain:port";
static const std::string udf_mail_unsafe_header =
    "From, To, Cc, Subject cannot contain line breaks";
static const std::string format_date = "%a, %e %b %Y %H:%M:%S %z";

struct mime_message {
  bool _read;
  unsigned int _read_to;
  std::stringstream _mime_lines;
};

static bool mail_send_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
  DBUG_ENTER("mail_send_init");

  if (args->arg_count != 7) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Wrong argument list: mail_send(smtp_uri, "
                  "smtps_validate_cert, from, to, cc, "
                  "subject, body)");
    DBUG_RETURN(true);
  }

  if (args->arg_type[0] != STRING_RESULT /*smtp uri*/ ||
      args->arg_type[1] != INT_RESULT /*smtps validate certificates*/ ||
      args->arg_type[2] != STRING_RESULT /*from*/ ||
      args->arg_type[3] != STRING_RESULT /*to*/ ||
      args->arg_type[4] != STRING_RESULT /*cc*/ ||
      args->arg_type[5] != STRING_RESULT /*subject*/ ||
      args->arg_type[6] != STRING_RESULT /*body*/
  ) {
    std::snprintf(
        message, MYSQL_ERRMSG_SIZE,
        "Wrong argument type: mail_send(str, 0|1, str, str, str, str, str)");
    DBUG_RETURN(true);
  }

  if (!args->args[0] /*smtp uri*/ ||
      !args->args[1] /*smtps validate certificates*/ ||
      !((*(int *)args->args[1]) == 0 ||
        (*(int *)args->args[1]) == 1) /* 0 or 1 valid values*/
      || !args->args[2] /*from*/ || !args->args[3] /*to*/ ||
      !args->args[4] /*cc*/ || !args->args[5] /*subject*/ ||
      !args->args[6] /*body*/
  ) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE,
                  "Arguments cannot be null: mail_send(smtp_uri, "
                  "smtps_validate_cert (0|1), from, to, cc, "
                  "subject, body)");
    DBUG_RETURN(true);
  }

  initid->maybe_null = 0;
  initid->const_item =
      0;  // Non-Deterministic: same arguments will produce different values
  initid->ptr = NULL;

  DBUG_RETURN(false);
}

static void mail_send_deinit(UDF_INIT *initid MY_ATTRIBUTE((unused))) {
  DBUG_ENTER("mail_send_deinit");

  return;
}

static size_t _prepare_message(char *ptr, size_t size, size_t nitems,
                               void *userdata) {
  struct mime_message *msg = (struct mime_message *)userdata;

  if (size == 0 || nitems == 0 || (size * nitems) < 1) {
    return 0;
  }

  const char *data = msg->_mime_lines.str().c_str();
  const size_t data_len = strlen(data);
  if (!msg->_read) {
    const size_t start_reading = msg->_read_to;
    const size_t to_read = ((data_len - start_reading) > (size * nitems))
                               ? size * nitems
                               : data_len - start_reading;
    memcpy(ptr, data + start_reading, to_read);
    msg->_read_to += to_read;
    msg->_read = (msg->_read_to == data_len);

    return to_read;
  }

  return 0;
}

static std::string _mail_send(const char *smtp_uri,
                              const int smtps_validate_cert, const char *from,
                              const char *to, const char *cc,
                              const char *subject, const char *body) {
  std::string result;
  CURL *curl;

  curl = curl_easy_init();
  if (curl) {
    CURLcode res = CURLE_OK;
    curl_slist *recipients = nullptr;
    time_t rawtime;
    tm *timeinfo;
    char formatted_date[80];
    mime_message msg;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(formatted_date, 80, format_date.c_str(), timeinfo);

    msg._read = false;
    msg._read_to = 0;
    msg._mime_lines << "Date: " << formatted_date << "\r\n";
    msg._mime_lines << "From: " << from << "\r\n";
    msg._mime_lines << "To: " << to << "\r\n";
    if (cc && strlen(cc) > 0) msg._mime_lines << "Cc: " << cc << "\r\n";
    msg._mime_lines << "Subject: " << subject << "\r\n";
    msg._mime_lines << "\r\n";
    msg._mime_lines << body;

    curl_easy_setopt(curl, CURLOPT_URL, smtp_uri);
    if (strncmp(smtp_uri, "smtps", 5) == 0) {
      curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
      if (smtps_validate_cert == 0) {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
      }
    }

    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, from);
    recipients = curl_slist_append(recipients, to);
    if (cc && strlen(cc) > 0) recipients = curl_slist_append(recipients, cc);
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, _prepare_message);
    curl_easy_setopt(curl, CURLOPT_READDATA, &msg);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    res = curl_easy_perform(curl);
    if (res == CURLE_OK) {
      result = udf_mail_send_ok;
    } else {
      result =
          std::string("curl_easy_perform() failed: ") + curl_easy_strerror(res);
    }

    curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);
  } else {
    result = std::string("curl_easy_init() failed");
  }

  return result;
}

static bool valid_smtp(const char *smtp_uri) {
  return (strncmp(smtp_uri, "smtp://", 7) == 0 ||
          strncmp(smtp_uri, "smtps://", 8) == 0);
}

static bool safe_header(const char *header) {
  return std::string(header).find("\n") == std::string::npos;
}

static char *mail_send(UDF_INIT *, UDF_ARGS *args, char *result,
                       unsigned long *length,
                       char *is_null MY_ATTRIBUTE((unused)), char *is_error) {
  DBUG_ENTER("mail_send");

  if (!valid_smtp(args->args[0]) || !safe_header(args->args[0])) {
    *length = udf_mail_invalid_smtp_uri.length() > MYSQL_ERRMSG_SIZE
                  ? MYSQL_ERRMSG_SIZE
                  : udf_mail_invalid_smtp_uri.length();
    strncpy(result, udf_mail_invalid_smtp_uri.c_str(), *length);
    *is_error = 1;
    DBUG_RETURN(result);
  }

  if (!(safe_header(args->args[2]) && safe_header(args->args[3]) &&
        safe_header(args->args[4]) && safe_header(args->args[5]))) {
    *length = udf_mail_unsafe_header.length() > MYSQL_ERRMSG_SIZE
                  ? MYSQL_ERRMSG_SIZE
                  : udf_mail_unsafe_header.length();
    strncpy(result, udf_mail_unsafe_header.c_str(), *length);
    *is_error = 1;
    DBUG_RETURN(result);
  }

  std::string res =
      _mail_send(args->args[0], *(int *)args->args[1], args->args[2],
                 args->args[3], args->args[4], args->args[5], args->args[6]);
  *length = res.length() > MYSQL_ERRMSG_SIZE ? MYSQL_ERRMSG_SIZE : res.length();
  strncpy(result, res.c_str(), *length);
  *is_error = (res.compare(udf_mail_send_ok) != 0);

  DBUG_RETURN(result);
}

udf_descriptor udf_mail_send() {
  return {"mail_send", Item_result::STRING_RESULT,
          reinterpret_cast<Udf_func_any>(mail_send), mail_send_init,
          mail_send_deinit};
}
