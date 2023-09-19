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

#ifndef AUDIT_LOG_FILTER_RULE_H_INCLUDED
#define AUDIT_LOG_FILTER_RULE_H_INCLUDED

#include "components/audit_log_filter/event_field_action/replace_field.h"
#include "components/audit_log_filter/event_field_condition/function.h"
#include "components/audit_log_filter/event_filter_function.h"

#include <memory>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace audit_log_filter {

using namespace event_field_condition;
using namespace event_field_action;

class AuditRule {
 public:
  AuditRule();
  explicit AuditRule(const char *rule_name);
  AuditRule(uint64_t filter_id, const char *rule_name);
  AuditRule(AuditRule &&other) noexcept;

 public:
  /**
   * @brief Get filtering rule numeric ID.
   *
   * @return Numeric ID of a filtering rule
   */
  [[nodiscard]] uint64_t get_filter_id() const noexcept;

  /**
   * @brief Get filtering rule name.
   *
   * @return Filtering rule name
   */
  [[nodiscard]] std::string get_rule_name() const noexcept;

  /**
   * @brief Set temporary replacement filtering rule.
   *
   * @param rule Pointer to replacement filtering rule
   */
  void set_replacement_rule(AuditRule *rule) noexcept;

  /**
   * @brief Clear temporary replacement filtering rule.
   */
  void clear_replacement_rule() noexcept;

  /**
   * @brief Check if event not matching the rule should be written to log.
   *
   * Default 'log' action is specified on rule global level like
   * '{"filter": { "log": true }}'. This action may later be overridden
   * for a specific event class or subclass.
   *
   * @return true in case event not matching the rule should be logged,
   *         false otherwise
   */
  [[nodiscard]] bool should_log_unmatched() const noexcept;

  /**
   * @brief Set default logging action for unmatched events.
   *
   * @param should_log true in case unmatched event should be written to
   *                   audit log, false otherwise
   */
  void set_should_log_unmatched(bool should_log) noexcept;

  /**
   * @brief Check if rule defines any actions for an audit event.
   *
   * @param event_class_name Audit event class name
   * @param event_subclass_name Audit event subclass name
   * @return true in case there are actions defined for an audit event,
   *         false otherwise
   */
  [[nodiscard]] bool has_actions_for(
      std::string_view event_class_name,
      std::string_view event_subclass_name) const noexcept;

  /**
   * @brief Get action defined for an audit event.
   *
   * @param action_type Action type
   * @param event_class_name Audit event class name
   * @param event_subclass_name Audit event subclass name
   * @return Pointer to an action instance
   */
  [[nodiscard]] const EventFieldActionBase *get_action(
      EventActionType action_type, std::string_view event_class_name,
      std::string_view event_subclass_name) const noexcept;

  void add_action_for_event(
      const std::shared_ptr<EventFieldActionBase> &action,
      const std::string &event_class_name,
      const std::string &event_subclass_name = "") noexcept;

 private:
  uint64_t m_filter_id;
  std::string m_rule_name;

  bool m_should_log_unmatched;
  std::unordered_map<std::string,
                     std::vector<std::shared_ptr<EventFieldActionBase>>>
      m_matched_event_to_action_map;

  AuditRule *m_replacement_rule;
};

}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_RULE_H_INCLUDED
