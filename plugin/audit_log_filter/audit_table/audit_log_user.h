/* Copyright (c) 2022 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#ifndef AUDIT_LOG_FILTER_AUDIT_TABLE_AUDIT_LOG_USER_H_INCLUDED
#define AUDIT_LOG_FILTER_AUDIT_TABLE_AUDIT_LOG_USER_H_INCLUDED

#include "base.h"

#include <map>
#include <string>

namespace audit_log_filter::audit_table {

class AuditLogUser : public AuditTableBase {
 public:
  using AuditUsersContainer =
      std::map<std::pair<std::string, std::string>, std::string>;

  explicit AuditLogUser(comp_registry_srv_t *comp_registry_srv);

  /**
   * @brief Load user list.
   *
   * @param container Container to store user list into
   * @return Table access result, @ref TableResult
   */
  TableResult load_users(AuditUsersContainer &container) noexcept;

  /**
   * @brief Delete filtering rule assignment by filtering rule name.
   *
   * @param rule_name Filtering rule name
   * @return Table access result, @ref TableResult
   */
  TableResult delete_user_by_filter(const std::string &rule_name) noexcept;

  /**
   * @brief Delete filtering rule assignment by user name and host name.
   *
   * @param user_name User name
   * @param user_host Host name
   * @return Table access result, @ref TableResult
   */
  TableResult delete_user_by_name_host(const std::string &user_name,
                                       const std::string &user_host) noexcept;

  /**
   * @brief Set or update filtering rule assignment for a user.
   *
   * @param user_name User name
   * @param user_host Host name
   * @param filter_name Filtering rule name
   * @return Table access result, @ref TableResult
   */
  TableResult set_update_filter(const std::string &user_name,
                                const std::string &user_host,
                                const std::string &filter_name) noexcept;

 private:
  /**
   * @brief Locate a record in audit_log_user table by filtering rule name.
   *
   * @param ta_context Table access context
   * @param key Table access key
   * @param rule_name Filtering rule name
   * @return Table access result, @ref TableResult
   */
  TableResult index_scan_locate_record_by_rule_name(
      TableAccessContext *ta_context, TA_key *key,
      const std::string &rule_name) noexcept;

  /**
   * @brief Locate a record in audit_log_user table by user name and host name.
   *
   * @param ta_context Table access context
   * @param key Table access key
   * @param user_name User name
   * @param user_host User host
   * @return Table access result, @ref TableResult
   */
  TableResult index_scan_locate_record_by_user_name_host(
      TableAccessContext *ta_context, TA_key *key, const std::string &user_name,
      const std::string &user_host) noexcept;

  /**
   * @brief Destroy index access session for a table.
   *
   * @param ta_context Table access context
   * @param key Table access key
   */
  void index_scan_end(TableAccessContext *ta_context, TA_key key) noexcept;

  /**
   * @brief Get database name.
   *
   * @return Database name
   */
  [[nodiscard]] const char *get_table_db_name() noexcept override;

  /**
   * @brief Get table name.
   *
   * @return Table name
   */
  [[nodiscard]] const char *get_table_name() noexcept override;

  /**
   * @brief Get table fields definition.
   *
   * @return Table fields definition
   */
  [[nodiscard]] const TA_table_field_def *get_table_field_def() noexcept
      override;

  /**
   * @brief Get number of table fields.
   *
   * @return Number of table fields
   */
  [[nodiscard]] size_t get_table_field_count() noexcept override;
};

}  // namespace audit_log_filter::audit_table

#endif  // AUDIT_LOG_FILTER_AUDIT_TABLE_AUDIT_LOG_USER_H_INCLUDED
