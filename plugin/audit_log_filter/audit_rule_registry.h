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

#ifndef AUDIT_LOG_FILTER_RULE_REGISTRY_H_INCLUDED
#define AUDIT_LOG_FILTER_RULE_REGISTRY_H_INCLUDED

#include "plugin/audit_log_filter/audit_table/audit_log_filter.h"
#include "plugin/audit_log_filter/audit_table/audit_log_user.h"

#include "mysql/plugin.h"

#include <map>
#include <string>

namespace audit_log_filter {

class AuditRule;

class AuditRuleRegistry {
 public:
  AuditRuleRegistry() = default;

  /**
   * @brief Load filtering rules from DB.
   *
   * @return true in case filtering rules are loaded successfully,
   *         false otherwise
   */
  bool load() noexcept;

  /**
   * @brief Get filtering rule by name.
   *
   * @param [in] filter_name Filtering rule name
   * @return Filtering rule
   */
  [[nodiscard]] AuditRule *get_rule(const std::string &filter_name) noexcept;

  /**
   * @brief Lookup filtering rule by user name and user host.
   *
   * @param [in] user_name User name
   * @param [in] host_name User host name
   * @param [out] rule_name Filtering rule name
   * @return true in case filtering rule assigned to a user was found,
   *         false otherwise
   */
  bool lookup_rule_name(const std::string &user_name,
                        const std::string &host_name,
                        std::string &rule_name) noexcept;

 private:
  /**
   * @brief Create plugin configuration tables.
   *
   * @return true in case of success, false otherwise
   */
  bool init_audit_tables() noexcept;

 private:
  audit_table::AuditLogUser::AuditUsersContainer m_audit_users;
  audit_table::AuditLogFilter::AuditRulesContainer m_audit_filter_rules;
};

}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_RULE_REGISTRY_H_INCLUDED
