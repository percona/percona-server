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

#include "plugin/audit_log_filter/audit_action.h"
#include "plugin/audit_log_filter/audit_record.h"
#include "plugin/audit_log_filter/event_field_action/replace_field.h"
#include "plugin/audit_log_filter/event_field_condition/function.h"
#include "plugin/audit_log_filter/event_filter_function.h"

#include "mysql/plugin_audit.h"

#include "my_rapidjson_size_t.h"
#include "rapidjson/document.h"

#include <memory>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace audit_log_filter {

using namespace event_field_condition;
using namespace event_field_action;

class AuditRule {
 public:
  AuditRule() = delete;
  AuditRule(const char *rule_name, const char *rule_str);
  AuditRule(uint64_t filter_id, const char *rule_name, const char *rule_str);

 private:
  explicit AuditRule(rapidjson::Document json_doc);

 public:
  /**
   * @brief Get string representation of a filtering rule.
   *
   * @return Filtering rule string representation
   */
  std::string to_string() noexcept;

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
   * @brief Do basic rule format validation.
   *
   * @return true in case rule format is valid, false otherwise
   */
  [[nodiscard]] bool check_valid() noexcept;

  /**
   * @brief Parse filtering rule from its JSON representation.
   *
   * @return true in case filtering rule parsed successfully, false otherwise
   */
  [[nodiscard]] bool check_parse_state() noexcept;

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

 private:
  void add_action_for_event(
      const std::shared_ptr<EventFieldActionBase> &action,
      const std::string &event_class_name,
      const std::string &event_subclass_name = "") noexcept;

  /**
   * @brief Parse filtering rule represented by a JSON string.
   *
   * @return true in case of success, false otherwise
   */
  bool parse_json_rule() noexcept;

  /**
   * @brief Parse audit event class related definitions in a filtering rule
   *        represented by a JSON string.
   *
   * @param event_class_json JSON object representing audit event class info
   * @return true in case of success, false otherwise
   */
  bool parse_event_class_json(
      const rapidjson::Value &event_class_json) noexcept;

  /**
   * @brief Parse audit event subclass related definitions in a filtering rule
   *        represented by a JSON string.
   *
   * @param class_name Audit event class name
   * @param event_subclass_json JSON object representing audit event
   *                            subclass info
   * @return true in case of success, false otherwise
   */
  bool parse_event_subclass(
      const std::string &class_name,
      const rapidjson::Value &event_subclass_json) noexcept;

  /**
   * @brief Parse JSON definition for one event subclass in a filtering rule.
   *
   * @param class_name Audit event class name
   * @param event_subclass_json JSON object representing audit event
   *                            subclass info
   * @return true in case of success, false otherwise
   */
  bool parse_event_subclass_json(
      const std::string &class_name,
      const rapidjson::Value &event_subclass_json) noexcept;

  /**
   * @brief Parse definition of a logical condition in audit event filter
   *        represented by a JSON string.
   *
   * @param condition_json JSON object representing condition definition
   * @return Pointer to an instance representing parsed condition
   */
  std::shared_ptr<EventFieldConditionBase> parse_condition(
      const rapidjson::Value &condition_json) noexcept;

  /**
   * @brief Get type of condition specified for an event field.
   *
   * @param event_field_obj JSON object representing audit event field info
   * @return One of condition types defined by @ref EventFieldConditionType
   */
  EventFieldConditionType get_condition_type(
      const rapidjson::Value &event_field_obj) noexcept;

  /**
   * @brief Parse audit event field condition in a filtering rule
   *        represented by a JSON string.
   *
   * @param condition_json JSON object representing audit event filter condition
   * @param cond_type Condition type, one of @ref EventFieldConditionType
   * @return Logical condition instance
   */
  std::shared_ptr<EventFieldConditionBase> parse_condition_json(
      const rapidjson::Value &condition_json,
      EventFieldConditionType cond_type) noexcept;

  /**
   * @brief Parse function definition in a filtering rule represented by
   *        a JSON string.
   *
   * @param function_json JSON object representing a function
   * @param expected_return_type Expected return type of the function
   * @return Function instance
   */
  std::unique_ptr<EventFilterFunctionBase> parse_function(
      const rapidjson::Value &function_json,
      FunctionReturnType expected_return_type) noexcept;

  /**
   * @brief Parse function arguments in a filtering rule represented
   *        by a JSON string.
   *
   * @param function_args_json JSON object representing function arguments
   * @param args Container to store parsed arguments in
   * @return true in case of success, false otherwise
   */
  static bool parse_function_args_json(
      const rapidjson::Value &function_args_obj,
      FunctionArgsList &args) noexcept;

  /**
   * @brief Parse action definition in a filtering rule represented by
   *        a JSON string.
   *
   * @param action_type Action type
   * @param action_json JSON object representing action
   * @return Action instance
   */
  [[nodiscard]] std::shared_ptr<EventFieldActionBase> parse_action_json(
      EventActionType action_type,
      const rapidjson::Value &action_json) noexcept;

  /**
   * @brief Build replacement filtering rule.
   *
   * @param rule_json JSON definition of replacement filtering rule
   * @return Replacement filtering rule instance
   */
  [[nodiscard]] static std::shared_ptr<AuditRule> make_replacement_rule(
      const rapidjson::Value &rule_json) noexcept;

 private:
  uint64_t m_filter_id;
  std::string m_rule_name;
  rapidjson::Document m_json_rule_doc;

  bool m_json_rule_parsed;
  bool m_json_rule_format_valid;

  bool m_should_log_unmatched;
  std::unordered_map<std::string,
                     std::vector<std::shared_ptr<EventFieldActionBase>>>
      m_matched_event_to_action_map;

  AuditRule *m_replacement_rule;
};

}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_RULE_H_INCLUDED
