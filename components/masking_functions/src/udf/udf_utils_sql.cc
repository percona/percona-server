/* Copyright (c) 2023 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   51 Franklin Street, Suite 500, Boston, MA 02110-1335 USA */

#include "components/masking_functions/include/udf/udf_utils_sql.h"

#include <mysql/components/services/mysql_current_thread_reader.h>
#include <mysql/components/services/security_context.h>
#include "components/masking_functions/include/component.h"
#include "components/masking_functions/include/udf/udf_utils.h"

extern REQUIRES_SERVICE_PLACEHOLDER_AS(mysql_command_query, cmd_query_srv);
extern REQUIRES_SERVICE_PLACEHOLDER_AS(mysql_command_error_info,
                                       cmd_error_info_srv);
extern REQUIRES_SERVICE_PLACEHOLDER_AS(mysql_command_query_result,
                                       cmd_query_result_srv);
extern REQUIRES_SERVICE_PLACEHOLDER_AS(mysql_command_options, cmd_options_srv);
extern REQUIRES_SERVICE_PLACEHOLDER_AS(mysql_command_factory, cmd_factory_srv);
extern REQUIRES_SERVICE_PLACEHOLDER_AS(mysql_thd_security_context,
                                       thd_security_ctx);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_current_thread_reader);
extern REQUIRES_SERVICE_PLACEHOLDER_AS(mysql_security_context_options,
                                       security_ctx_options);

extern REQUIRES_SERVICE_PLACEHOLDER(global_grants_check);

constexpr std::string_view privilege_name = "MASKING_DICTIONARIES_ADMIN";

namespace mysql::components {

sql_context::sql_context() {
  if (cmd_factory_srv->init(&mysql_h)) {
    throw sql_context_exception("Couldn't initialize server handle");
  }

  THD *thd;
  if (mysql_service_mysql_current_thread_reader->get(&thd)) {
    throw sql_context_exception("Couldn't query current thd");
  }

  Security_context_handle sctx;
  if (thd_security_ctx->get(thd, &sctx)) {
    throw sql_context_exception("Couldn't query security context");
  }

  if (cmd_options_srv->set(mysql_h, MYSQL_COMMAND_USER_NAME, "root")) {
    throw sql_context_exception("Couldn't set username");
  }
  if (cmd_options_srv->set(mysql_h, MYSQL_COMMAND_HOST_NAME, "localhost")) {
    throw sql_context_exception("Couldn't set hostname");
  }
  if (cmd_factory_srv->connect(mysql_h)) {
    throw sql_context_exception("Couldn't establish server connection");
  }
}

sql_context::~sql_context() { cmd_factory_srv->close(mysql_h); }

std::optional<std::string> sql_context::query_single_value(
    std::string const &query) {
  MYSQL_RES_H mysql_res = nullptr;
  MYSQL_ROW_H row = nullptr;
  std::string sresult;

  if (cmd_query_srv->query(mysql_h, query.data(), query.length())) {
    char *errormsg;
    cmd_error_info_srv->sql_error(mysql_h, &errormsg);
    std::string error = "Error while executing SQL query: ";
    if (errormsg != nullptr) error += errormsg;
    throw sql_context_exception(error);
  }

  cmd_query_result_srv->store_result(mysql_h, &mysql_res);
  if (!mysql_res) {
    throw sql_context_exception("Couldn't store MySQL result");
  }

  uint64_t row_count = 0;
  if (cmd_query_srv->affected_rows(mysql_h, &row_count)) {
    cmd_query_result_srv->free_result(mysql_res);
    throw sql_context_exception("Couldn't query row count");
  }

  if (row_count == 0) {
    cmd_query_result_srv->free_result(mysql_res);
    return std::nullopt;
  }

  if (row_count > 1) {
    cmd_query_result_srv->free_result(mysql_res);
    throw sql_context_exception("Query returned more than 1 row");
  }

  if (cmd_query_result_srv->fetch_row(mysql_res, &row)) {
    cmd_query_result_srv->free_result(mysql_res);
    throw sql_context_exception("Couldn't fetch row");
  }

  ulong *length = nullptr;
  cmd_query_result_srv->fetch_lengths(mysql_res, &length);
  sresult = row[0];

  cmd_query_result_srv->free_result(mysql_res);

  return sresult;
}

bool sql_context::execute(std::string const &query) {
  uint64_t row_count = 0;
  if (cmd_query_srv->query(mysql_h, query.data(), query.length())) {
    return 0;
  }
  cmd_query_srv->affected_rows(mysql_h, &row_count);
  return row_count;
}

void escape_string_into(std::string &into, std::string const &str) {
  size_t str_pos = into.size();
  // resize the buffer to fit the original size + worst case length of s
  into.resize(str_pos + 2 * str.size() + 1);

  CHARSET_INFO *cs = get_charset_by_csname("utf8mb4", MY_CS_PRIMARY, MYF(0));
  size_t r = escape_string_for_mysql(cs, &into[str_pos], 2 * str.size() + 1,
                                     str.c_str(), str.size());
  into.resize(str_pos + r);
}

bool have_masking_admin_privilege() {
  THD *thd;
  if (mysql_service_mysql_current_thread_reader->get(&thd)) {
    throw sql_context_exception("Couldn't query current thd");
  }

  Security_context_handle sctx;
  if (thd_security_ctx->get(thd, &sctx)) {
    throw sql_context_exception("Couldn't query security context");
  }

  if (mysql_service_global_grants_check->has_global_grant(
          sctx, privilege_name.data(), privilege_name.size()))
    return true;

  return false;
}

}  // namespace mysql::components
