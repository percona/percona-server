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

#include "components/audit_log_filter/audit_table/audit_log_filter.h"
#include "components/audit_log_filter/audit_table/audit_log_user.h"

#include <atomic>
#include <map>
#include <shared_mutex>
#include <string>

namespace audit_log_filter {

class AuditRule;

class AuditRuleRegistry {
 public:
  AuditRuleRegistry() = default;

  /**
   * @brief Load filtering rules from DB.
   *
   * @param error_message Out-parameter for a human-readable error
   * @return true in case filtering rules are loaded successfully,
   *         false otherwise
   */
  bool load(std::string &error_message) noexcept;

  /**
   * @brief Get filtering rule by name.
   *
   * @param [in] filter_name Filtering rule name
   * @return Filtering rule
   */
  [[nodiscard]] std::shared_ptr<AuditRule> get_rule(
      const std::string &filter_name) noexcept;

  /**
   * @brief Resolve the filtering rule assigned to a user under one shared-lock
   *        acquisition.
   *
   * Performs both the user-to-rule lookup and the rule lookup against one
   * consistent registry snapshot, avoiding an intermediate reload between the
   * two reads.
   *
   * @param [in] user_name User name
   * @param [in] host_name User host name
   * @param [out] rule_name Resolved filtering rule name, empty if no mapping
   *                        was found
   * @return Filtering rule, or nullptr if the user has no assigned rule or if
   *         the mapping refers to a missing rule
   */
  [[nodiscard]] std::shared_ptr<AuditRule> resolve_rule_for_user(
      const std::string &user_name, const std::string &host_name,
      std::string *rule_name = nullptr) noexcept;

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

  /**
   * @brief Mark the registry as needing a reload on the next rule resolution.
   *
   * Safe to call from any context (lock-free atomic store). The actual
   * reload happens inside the next rule resolution call, which runs
   * in a THD context where table access is permitted.
   */
  void invalidate() noexcept;

  /**
   * @brief Registry version, incremented under the exclusive lock every
   *        time load() successfully swaps in new data.
   *
   * Used by CONNECT/CHANGE_USER events to detect a concurrent reload
   * that may have changed user-to-filter mappings between the pre-resolve
   * snapshot and the post-resolve check.
   */
  [[nodiscard]] uint64_t load_version() const noexcept;

  /**
   * @brief True while at least one load() call is in progress.
   *
   * The counter is incremented before the DB read and decremented after
   * the version bump.  CONNECT/CHANGE_USER paths use this to suppress
   * caching when a reload is underway — the resolved rule is used for
   * the current event only, and the next event re-resolves.
   */
  [[nodiscard]] bool is_reload_in_progress() const noexcept;

 private:
  std::atomic<bool> m_is_initialised{false};
  std::shared_mutex m_registry_mutex;
  std::atomic<uint64_t> m_load_version{0};
  std::atomic<uint32_t> m_loads_in_progress{0};
  audit_table::AuditLogUser::AuditUsersContainer m_audit_users;
  audit_table::AuditLogFilter::AuditRulesContainer m_audit_filter_rules;
};

}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_RULE_REGISTRY_H_INCLUDED
