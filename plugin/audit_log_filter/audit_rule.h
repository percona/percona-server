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
#include "plugin/audit_log_filter/event_field_condition/base.h"

#include "mysql/plugin_audit.h"

#include "my_rapidjson_size_t.h"
#include "rapidjson/document.h"

#include <memory>
#include <string_view>
#include <unordered_map>

namespace audit_log_filter {

class AuditRule {
 public:
  AuditRule() = delete;
  AuditRule(const char *rule_name, const char *rule_str);
  AuditRule(uint64_t filter_id, const char *rule_name, const char *rule_str);

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
   * @brief Get action defined by a filtering rule on global level.
   *
   * Default 'log' action is specified on rule global level like
   * '{"filter": { "log": true }}'. This action may later be overridden
   * for a specific event class or subclass.
   *
   * @return Default log action specified in a filtering rule
   */
  [[nodiscard]] AuditAction get_global_action() const noexcept;

  /**
   * @brief Get log action specified for a class name by a filtering rule.
   *
   * @param event_class_name Audit event class name
   * @return Log action specified for a class name
   */
  [[nodiscard]] AuditAction get_event_class_action(
      std::string_view event_class_name) const noexcept;

  /**
   * @brief Get log action specified for a subclass name by a filtering rule.
   *
   * @param event_class_name Audit event class name
   * @param event_subclass_name Audit event subclass name
   * @param fields List event fields and their values
   * @return Log action specified for a subclass name
   */
  [[nodiscard]] AuditAction get_event_subclass_action(
      std::string_view event_class_name, std::string_view event_subclass_name,
      const AuditRecordFieldsList &fields) const noexcept;

 private:
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
   * @param event_class_obj JSON object representing audit event class info
   * @return true in case of success, false otherwise
   */
  bool parse_event_class_obj(const rapidjson::Value &event_class_obj) noexcept;

  /**
   * @brief Parse audit event subclass related definitions in a filtering rule
   *        represented by a JSON string.
   *
   * @param class_name Audit event class name
   * @param event_subclass_obj JSON object representing audit event
   *                           subclass info
   * @return true in case of success, false otherwise
   */
  bool parse_event_subclass(
      const std::string &class_name,
      const rapidjson::Value &event_subclass_obj) noexcept;

  /**
   * @brief Parse JSON definition for one event subclass in a filtering rule.
   *
   * @param class_name Audit event class name
   * @param event_subclass_obj JSON object representing audit event
   *                           subclass info
   * @return true in case of success, false otherwise
   */
  bool parse_event_subclass_obj(
      const std::string &class_name,
      const rapidjson::Value &event_subclass_obj) noexcept;

  /**
   * @brief Parse audit event field related definitions in a filtering rule
   *        represented by a JSON string.
   *
   * @param cond_keys List of keys used to find event field related condition,
   *                  made of event class and subclass names concatenated
   *                  with a dot
   * @param cond_action Action applied to an event in case condition matches
   * @param event_field_obj JSON object representing audit event field info
   * @return true in case of success, false otherwise
   */
  bool parse_event_field(const std::vector<std::string> &cond_keys,
                         AuditAction cond_action,
                         const rapidjson::Value &event_field_obj) noexcept;

  /**
   * @brief Get type of condition specified for an event field.
   *
   * @param event_field_obj JSON object representing audit event field info
   * @return One of condition types defined by @ref EventFieldConditionType
   */
  event_field_condition::EventFieldConditionType get_condition_type(
      const rapidjson::Value &event_field_obj) noexcept;

  /**
   * @brief Parse audit event field condition in a filtering rule
   *        represented by a JSON string.
   *
   * @param event_field_obj JSON object representing audit event field condition
   * @param cond_type Condition type, one of @ref EventFieldConditionType
   * @param cond_action Action applied to an event in case condition matches
   * @return Logical condition instance
   */
  std::shared_ptr<event_field_condition::EventFieldConditionBase>
  parse_condition_obj(const rapidjson::Value &event_field_obj,
                      event_field_condition::EventFieldConditionType cond_type,
                      AuditAction cond_action) noexcept;

 private:
  uint64_t m_filter_id;
  std::string m_rule_name;
  rapidjson::Document m_json_rule_doc;

  bool m_json_rule_parsed;
  bool m_json_rule_format_valid;
  AuditAction m_global_action;
  std::unordered_map<std::string, AuditAction> m_class_to_action_map;
  std::unordered_map<
      std::string,
      std::shared_ptr<event_field_condition::EventFieldConditionBase>>
      m_subclass_to_condition_map;
};

}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_RULE_H_INCLUDED
