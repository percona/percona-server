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

#include "mysql/plugin_audit.h"

#include "my_rapidjson_size_t.h"
#include "rapidjson/document.h"

#include <string_view>
#include <unordered_map>

namespace audit_log_filter {

enum class AuditAction {
  None,  // action not defined
  Log,   // write event to audit log
  Skip,  // don't write event to audit log
  Block  // event blocked by a rule, server should reject it
};

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
   * @param event_subclass_name Audit event subclass name
   * @return Log action specified for a subclass name
   */
  [[nodiscard]] AuditAction get_event_subclass_action(
      std::string_view event_subclass_name) const noexcept;

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
   * @param event_subclass_obj JSON object representing audit event
   *                           subclass info
   * @return true in case of success, false otherwise
   */
  bool parse_event_subclass(
      const rapidjson::Value &event_subclass_obj) noexcept;

  /**
   * @brief Parse JSON definition for one event subclass in a filtering rule.
   *
   * @param event_subclass_obj JSON object representing audit event
   *                           subclass info
   * @return true in case of success, false otherwise
   */
  bool parse_event_subclass_obj(
      const rapidjson::Value &event_subclass_obj) noexcept;

 private:
  uint64_t m_filter_id;
  std::string m_rule_name;
  rapidjson::Document m_json_rule_doc;

  bool m_json_rule_parsed;
  bool m_json_rule_format_valid;
  AuditAction m_global_action;
  std::unordered_map<std::string_view, AuditAction> m_class_to_action_map;
  std::unordered_map<std::string_view, AuditAction> m_subclass_to_action_map;
};

}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_RULE_H_INCLUDED
