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

#include <string>
#include <tuple>

namespace audit_log_filter {
namespace {
const std::string kDefaultUserName = "%";
const std::string kDefaultHostName = "%";
}  // namespace

AuditRuleRegistry::AuditRuleRegistry(comp_registry_srv_t *comp_registry_srv)
    : m_comp_registry_srv{comp_registry_srv} {}

AuditRule *AuditRuleRegistry::get_rule(const std::string &rule_name) noexcept {
  if (m_audit_filter_rules.count(rule_name) == 0) {
    return nullptr;
  }

  auto it = m_audit_filter_rules.find(rule_name);
  return &it->second;
}

bool AuditRuleRegistry::lookup_rule_name(const std::string &user_name,
                                         const std::string &host_name,
                                         std::string &rule_name) noexcept {
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
  audit_table::AuditLogFilter audit_log_filter{m_comp_registry_srv};
  audit_table::AuditLogUser audit_log_user{m_comp_registry_srv};

  auto users_result = audit_log_user.load_users(m_audit_users);
  auto filter_result = audit_log_filter.load_filters(m_audit_filter_rules);

  if (users_result == audit_table::TableResult::MissingTable &&
      filter_result == audit_table::TableResult::MissingTable) {
    return init_audit_tables();
  }

  if (users_result != audit_table::TableResult::Ok ||
      filter_result != audit_table::TableResult::Ok) {
    m_audit_users.clear();
    m_audit_filter_rules.clear();
  }

  return users_result == audit_table::TableResult::Ok &&
         filter_result == audit_table::TableResult::Ok;
}

// TODO: implement
bool AuditRuleRegistry::init_audit_tables() noexcept { return true; }

}  // namespace audit_log_filter
