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

#ifndef AUDIT_LOG_FILTER_RULE_PARSER_H_INCLUDED
#define AUDIT_LOG_FILTER_RULE_PARSER_H_INCLUDED

#include "plugin/audit_log_filter/audit_rule.h"

#include "my_rapidjson_size_t.h"
#include "rapidjson/document.h"

namespace audit_log_filter {

class AuditRuleParser {
 public:
  /**
   * @brief Parse audit filter rule represented as a string.
   *
   * @param rule_str String representation of audit filtering rule
   * @param audit_rule Audit filtering rule instance to be initialized
   * @return true in case rule is parsed successfully,
   *         false otherwise
   */
  static bool parse(const char *rule_str, AuditRule &audit_rule) noexcept;

 private:
  /**
   * @brief Parse audit filter rule represented as a JSON document object.
   *
   * @param json_doc JSON document representation of audit filtering rule
   * @param audit_rule Audit filtering rule instance to be initialized
   * @return true in case rule is parsed successfully,
   *         false otherwise
   */
  static bool parse(rapidjson::Document &json_doc,
                    AuditRule &audit_rule) noexcept;

  /**
   * @brief Determine the default logging action for unmatched audit events.
   *
   * @param json_doc JSON document representation of audit filtering rule
   * @param audit_rule Audit filtering rule instance to be initialized
   * @return true in case default action parsed successfully,
   *         false otherwise
   */
  static bool parse_default_log_action_json(const rapidjson::Document &json_doc,
                                            AuditRule &audit_rule) noexcept;

  /**
   * @brief Parse audit event class defined actions in audit filtering rule.
   *
   * @param json_doc JSON document representation of audit filtering rule
   * @param audit_rule Audit filtering rule instance to be initialized
   * @return true in case event class definition parsed successfully,
   *         false otherwise
   */
  static bool parse_event_class_json(const rapidjson::Document &json_doc,
                                     AuditRule &audit_rule) noexcept;

  /**
   * @brief Parse one JSON object related to audit event class definition.
   *
   * @param event_class_json JSON object representing rule definition for
   *                         one audit event class
   * @param audit_rule Audit filtering rule instance to be initialized
   * @return true in case event class definition parsed successfully,
   *         false otherwise
   */
  static bool parse_event_class_obj_json(
      const rapidjson::Value &event_class_json, AuditRule &audit_rule) noexcept;

  /**
   * @brief Parse audit event subclass related definitions in a filtering rule
   *        represented by a JSON object.
   *
   * @param class_name Audit event class name
   * @param event_subclass_json JSON object representing audit event
   *                            subclass info
   * @param audit_rule Audit filtering rule instance to be initialized
   * @return true in case of success, false otherwise
   */
  static bool parse_event_subclass_json(
      const std::string &class_name,
      const rapidjson::Value &event_subclass_json,
      AuditRule &audit_rule) noexcept;

  /**
   * @brief Parse JSON definition for one event subclass in a filtering rule.
   *
   * @param class_name Audit event class name
   * @param event_subclass_json JSON object representing audit event
   *                            subclass info
   * @param audit_rule Audit filtering rule instance to be initialized
   * @return true in case of success, false otherwise
   */
  static bool parse_event_subclass_obj_json(
      const std::string &class_name,
      const rapidjson::Value &event_subclass_json,
      AuditRule &audit_rule) noexcept;

  /**
   * @brief Parse definition of a logical condition in audit event filter
   *        represented by a JSON string.
   *
   * @param condition_json JSON object representing condition definition
   * @param audit_rule Audit filtering rule instance to be initialized
   * @return Pointer to an instance representing parsed condition
   */
  static std::shared_ptr<EventFieldConditionBase> parse_condition(
      const rapidjson::Value &condition_json, AuditRule &audit_rule) noexcept;

  /**
   * @brief Get type of condition specified for an event field.
   *
   * @param event_field_obj JSON object representing audit event field info
   * @param audit_rule Audit filtering rule instance to be initialized
   * @return One of condition types defined by @ref EventFieldConditionType
   */
  static EventFieldConditionType get_condition_type(
      const rapidjson::Value &event_field_obj, AuditRule &audit_rule) noexcept;

  /**
   * @brief Parse audit event field condition in a filtering rule
   *        represented by a JSON string.
   *
   * @param condition_json JSON object representing audit event filter condition
   * @param cond_type Condition type, one of @ref EventFieldConditionType
   * @param audit_rule Audit filtering rule instance to be initialized
   * @return Logical condition instance
   */
  static std::shared_ptr<EventFieldConditionBase> parse_condition_json(
      const rapidjson::Value &condition_json, EventFieldConditionType cond_type,
      AuditRule &audit_rule) noexcept;

  /**
   * @brief Parse function definition in a filtering rule represented by
   *        a JSON string.
   *
   * @param function_json JSON object representing a function
   * @param expected_return_type Expected return type of the function
   * @param audit_rule Audit filtering rule instance to be initialized
   * @return Function instance
   */
  static std::unique_ptr<EventFilterFunctionBase> parse_function(
      const rapidjson::Value &function_json,
      FunctionReturnType expected_return_type, AuditRule &audit_rule) noexcept;

  /**
   * @brief Parse function arguments in a filtering rule represented
   *        by a JSON string.
   *
   * @param function_args_obj JSON object representing function arguments
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
   * @param audit_rule Audit filtering rule instance to be initialized
   * @return Action instance
   */
  [[nodiscard]] static std::shared_ptr<EventFieldActionBase> parse_action_json(
      EventActionType action_type, const rapidjson::Value &action_json,
      AuditRule &audit_rule) noexcept;

  /**
   * @brief Build replacement filtering rule.
   *
   * @param rule_json JSON definition of replacement filtering rule
   * @return Replacement filtering rule instance
   */
  [[nodiscard]] static std::shared_ptr<AuditRule> make_replacement_rule(
      const rapidjson::Value &rule_json) noexcept;
};

}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_RULE_PARSER_H_INCLUDED
