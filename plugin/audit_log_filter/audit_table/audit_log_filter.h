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

#ifndef AUDIT_LOG_FILTER_AUDIT_TABLE_AUDIT_LOG_FILTER_H_INCLUDED
#define AUDIT_LOG_FILTER_AUDIT_TABLE_AUDIT_LOG_FILTER_H_INCLUDED

#include "base.h"
#include "plugin/audit_log_filter/audit_rule.h"

#include <map>
#include <string>

namespace audit_log_filter::audit_table {

class AuditLogFilter : public AuditTableBase {
 public:
  using AuditRulesContainer = std::map<std::string, AuditRule>;

  explicit AuditLogFilter(comp_registry_srv_t *comp_registry_srv);

  /**
   * @brief Load filtering rules list.
   *
   * @param container Container to store filtering rules list into
   * @return Table access result, @ref TableResult
   */
  TableResult load_filters(AuditRulesContainer &container) noexcept;

  /**
   * @brief Check if filtering rule with provided name exists.
   *
   * @param rule_name Filtering rule name
   * @return Table access result, @ref TableResult
   */
  TableResult check_name_exists(const std::string &rule_name) noexcept;

  /**
   * @brief Insert filtering rule.
   *
   * @param rule_name Filtering rule name
   * @param rule_definition Filtering rule definition
   * @return Table access result, @ref TableResult
   */
  TableResult insert_filter(const std::string &rule_name,
                            const std::string &rule_definition) noexcept;

  /**
   * @brief Delete filtering rule.
   *
   * @param rule_name Filtering rule name
   * @return Table access result, @ref TableResult
   */
  TableResult delete_filter(const std::string &rule_name) noexcept;

 private:
  /**
   * @brief Locate a record in audit_log_filter table by filtering rule name.
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
   * @brief Destroy index access session for a table.
   *
   * @param ta_context Table access context
   * @param key Table access key
   */
  void index_scan_end(TableAccessContext *ta_context, TA_key key) noexcept;

  /**
   * @brief Get next free filtering rule ID.
   *
   * @param ta_context Table access context
   * @param next_pk Returned filtering rule ID
   * @return Table access result, @ref TableResult
   */
  TableResult get_next_pk_value(TableAccessContext *ta_context,
                                long long &next_pk) noexcept;

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

#endif  // AUDIT_LOG_FILTER_AUDIT_TABLE_AUDIT_LOG_FILTER_H_INCLUDED
