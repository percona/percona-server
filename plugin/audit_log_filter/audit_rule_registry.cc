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

#include "plugin/audit_log_filter/audit_rule_registry.h"
#include "plugin/audit_log_filter/audit_error_log.h"
#include "plugin/audit_log_filter/audit_rule.h"

#include <mutex>
#include <string>
#include <tuple>

namespace audit_log_filter {
namespace {
const std::string kDefaultUserName = "%";
const std::string kDefaultHostName = "%";
}  // namespace

AuditRule *AuditRuleRegistry::get_rule(const std::string &rule_name) noexcept {
  std::shared_lock lock(m_registry_mutex);

  if (m_audit_filter_rules.count(rule_name) == 0) {
    return nullptr;
  }

  auto it = m_audit_filter_rules.find(rule_name);
  return &it->second;
}

bool AuditRuleRegistry::lookup_rule_name(const std::string &user_name,
                                         const std::string &host_name,
                                         std::string &rule_name) noexcept {
  std::shared_lock lock(m_registry_mutex);

  if (m_audit_users.count(std::make_pair(user_name, host_name)) != 0) {
    rule_name = m_audit_users[std::make_pair(user_name, host_name)];
    return true;
  }

  if (m_audit_users.count(std::make_pair(kDefaultUserName, kDefaultHostName)) !=
      0) {
    rule_name =
        m_audit_users[std::make_pair(kDefaultUserName, kDefaultHostName)];
    return true;
  }

  return false;
}

bool AuditRuleRegistry::load() noexcept {
  audit_table::AuditLogFilter audit_log_filter;
  audit_table::AuditLogUser audit_log_user;

  auto tmp_users = audit_table::AuditLogUser::AuditUsersContainer{};
  auto tmp_rules = audit_table::AuditLogFilter::AuditRulesContainer{};

  const bool is_success =
      (audit_log_filter.load_filters(tmp_rules) ==
       audit_table::TableResult::Ok) &&
      (audit_log_user.load_users(tmp_users) == audit_table::TableResult::Ok);

  if (is_success) {
    std::unique_lock lock(m_registry_mutex);
    m_audit_users.swap(tmp_users);
    m_audit_filter_rules.swap(tmp_rules);
  }

  return is_success;
}

}  // namespace audit_log_filter
