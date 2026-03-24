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

#include "components/audit_log_filter/audit_rule_registry.h"
#include "components/audit_log_filter/audit_error_log.h"
#include "components/audit_log_filter/audit_rule.h"
#include "components/audit_log_filter/sys_vars.h"

#include <string>
#include <tuple>

namespace audit_log_filter {
namespace {
const std::string kDefaultUserName = "%";
const std::string kDefaultHostName = "%";

void log_load_failure(const std::string &error_msg) {
  if (error_msg.empty()) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to load filtering rules");
    return;
  }

  const std::string log_message =
      "Failed to load filtering rules: " + error_msg;
  LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, log_message.c_str());
}
}  // namespace

std::shared_ptr<AuditRule> AuditRuleRegistry::get_rule(
    const std::string &rule_name) noexcept {
  std::shared_lock lock(m_registry_mutex);

  auto it = m_audit_filter_rules.find(rule_name);
  return it == m_audit_filter_rules.end() ? nullptr : it->second;
}

std::shared_ptr<AuditRule> AuditRuleRegistry::resolve_rule_for_user(
    const std::string &user_name, const std::string &host_name,
    std::string *rule_name) noexcept {
  if (rule_name != nullptr) {
    rule_name->clear();
  }

  if (!m_is_initialised) {
    m_is_initialised = true;
    std::string error_msg;
    if (!load(error_msg)) {
      log_load_failure(error_msg);
      // fail only when there is no prior good snapshot; otherwise keep using
      // the old one
      if (load_version() == 0) {
        return nullptr;
      }
    }
  }

  std::shared_lock lock(m_registry_mutex);

  auto user_it = m_audit_users.find(std::make_pair(user_name, host_name));
  if (user_it == m_audit_users.end()) {
    user_it =
        m_audit_users.find(std::make_pair(kDefaultUserName, kDefaultHostName));
    if (user_it == m_audit_users.end()) {
      return nullptr;
    }
  }

  if (rule_name != nullptr) {
    *rule_name = user_it->second;
  }

  auto rule_it = m_audit_filter_rules.find(user_it->second);
  return rule_it == m_audit_filter_rules.end() ? nullptr : rule_it->second;
}

bool AuditRuleRegistry::lookup_rule_name(const std::string &user_name,
                                         const std::string &host_name,
                                         std::string &rule_name) noexcept {
  if (!m_is_initialised) {
    m_is_initialised = true;
    std::string error_msg;
    if (!load(error_msg)) {
      log_load_failure(error_msg);
      if (load_version() == 0) {
        return false;
      }
    }
  }

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

void AuditRuleRegistry::invalidate() noexcept { m_is_initialised = false; }

bool AuditRuleRegistry::load(std::string &error_message) noexcept {
  m_loads_in_progress.fetch_add(1, std::memory_order_release);

  audit_table::AuditLogFilter audit_log_filter{
      SysVars::get_config_database_name()};
  audit_table::AuditLogUser audit_log_user{SysVars::get_config_database_name()};

  auto tmp_users = audit_table::AuditLogUser::AuditUsersContainer{};
  auto tmp_rules = audit_table::AuditLogFilter::AuditRulesContainer{};

  const bool is_success =
      (audit_log_filter.load_filters(tmp_rules, error_message) ==
       audit_table::TableResult::Ok) &&
      (audit_log_user.load_users(tmp_users) == audit_table::TableResult::Ok);

  if (is_success) {
    std::unique_lock lock(m_registry_mutex);
    m_audit_users.swap(tmp_users);
    m_audit_filter_rules.swap(tmp_rules);
    m_load_version.fetch_add(1, std::memory_order_release);
  }

  m_loads_in_progress.fetch_sub(1, std::memory_order_release);

  return is_success;
}

uint64_t AuditRuleRegistry::load_version() const noexcept {
  return m_load_version.load(std::memory_order_acquire);
}

bool AuditRuleRegistry::is_reload_in_progress() const noexcept {
  return m_loads_in_progress.load(std::memory_order_acquire) > 0;
}

}  // namespace audit_log_filter
