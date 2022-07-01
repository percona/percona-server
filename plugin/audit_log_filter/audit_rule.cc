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

#include "plugin/audit_log_filter/audit_rule.h"
#include "plugin/audit_log_filter/audit_error_log.h"

#include "plugin/audit_log_filter/event_field_action/block.h"
#include "plugin/audit_log_filter/event_field_action/log.h"
#include "plugin/audit_log_filter/event_field_action/replace_filter.h"
#include "plugin/audit_log_filter/event_field_condition/and.h"
#include "plugin/audit_log_filter/event_field_condition/bool.h"
#include "plugin/audit_log_filter/event_field_condition/field.h"
#include "plugin/audit_log_filter/event_field_condition/function.h"
#include "plugin/audit_log_filter/event_field_condition/not.h"
#include "plugin/audit_log_filter/event_field_condition/or.h"
#include "plugin/audit_log_filter/event_field_condition/variable.h"

#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include <string>

namespace audit_log_filter {

AuditRule::AuditRule(const char *rule_name, const char *rule_str)
    : AuditRule{0, rule_name, rule_str} {}

AuditRule::AuditRule(uint64_t filter_id, const char *rule_name,
                     const char *rule_str)
    : m_filter_id{filter_id},
      m_rule_name{rule_name},
      m_json_rule_parsed{false},
      m_json_rule_format_valid{false},
      m_should_log_unmatched{false},
      m_replacement_rule{nullptr} {
  m_json_rule_doc.Parse(rule_str);
}

AuditRule::AuditRule(rapidjson::Document json_doc)
    : m_filter_id{0},
      m_rule_name{},
      m_json_rule_doc{std::move(json_doc)},
      m_json_rule_parsed{false},
      m_json_rule_format_valid{false},
      m_should_log_unmatched{false},
      m_replacement_rule{nullptr} {}

std::string AuditRule::to_string() noexcept {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  m_json_rule_doc.Accept(writer);

  return buffer.GetString();
}

uint64_t AuditRule::get_filter_id() const noexcept { return m_filter_id; }

std::string AuditRule::get_rule_name() const noexcept { return m_rule_name; }

void AuditRule::set_replacement_rule(AuditRule *rule) noexcept {
  m_replacement_rule = rule;
}

void AuditRule::clear_replacement_rule() noexcept {
  m_replacement_rule = nullptr;
}

bool AuditRule::check_valid() noexcept {
  if (m_json_rule_doc.HasParseError()) {
    return false;
  }

  // The root of the JSON rule must be an object
  if (!m_json_rule_doc.IsObject()) {
    return false;
  }

  // The basic JSON rule format must be the following: '{"filter": {}}'
  if (!m_json_rule_doc.HasMember("filter") ||
      !m_json_rule_doc["filter"].IsObject()) {
    return false;
  }

  return true;
}

bool AuditRule::check_parse_state() noexcept {
  if (!m_json_rule_parsed) {
    m_json_rule_format_valid = parse_json_rule();
  }

  return m_json_rule_format_valid;
}

bool AuditRule::should_log_unmatched() const noexcept {
  assert(m_json_rule_parsed);
  return m_should_log_unmatched;
}

void AuditRule::add_action_for_event(
    const std::shared_ptr<EventFieldActionBase> &action,
    const std::string &event_class_name,
    const std::string &event_subclass_name) noexcept {
  std::stringstream event_key;
  event_key << event_class_name;

  if (!event_subclass_name.empty()) {
    event_key << "." << event_subclass_name;
  }

  auto actions_list = m_matched_event_to_action_map.find(event_key.str());

  if (actions_list == m_matched_event_to_action_map.end()) {
    m_matched_event_to_action_map.insert({event_key.str(), {action}});
  } else {
    actions_list->second.push_back(action);
  }
}

bool AuditRule::has_actions_for(
    std::string_view event_class_name,
    std::string_view event_subclass_name) const noexcept {
  if (m_replacement_rule == nullptr) {
    std::stringstream subclass_key;
    subclass_key << event_class_name << "." << event_subclass_name;

    if (m_matched_event_to_action_map.count(subclass_key.str()) > 0) {
      return true;
    }

    return m_matched_event_to_action_map.count(event_class_name.data()) > 0;
  }

  return m_replacement_rule->has_actions_for(event_class_name,
                                             event_subclass_name);
}

const EventFieldActionBase *AuditRule::get_action(
    const EventActionType action_type, const std::string_view event_class_name,
    const std::string_view event_subclass_name) const noexcept {
  if (m_replacement_rule == nullptr) {
    assert(m_json_rule_parsed);

    std::stringstream subclass_key;
    subclass_key << event_class_name << "." << event_subclass_name;

    auto it = m_matched_event_to_action_map.find(subclass_key.str());

    if (it == m_matched_event_to_action_map.cend()) {
      it = m_matched_event_to_action_map.find(event_class_name.data());
    }

    if (it == m_matched_event_to_action_map.cend()) {
      return nullptr;
    }

    const auto action =
        std::find_if(it->second.cbegin(), it->second.cend(),
                     [&action_type](const auto &act) {
                       return act->get_action_type() == action_type;
                     });

    if (action == std::cend(it->second)) {
      return nullptr;
    }

    return action->get();
  }

  return m_replacement_rule->get_action(action_type, event_class_name,
                                        event_subclass_name);
}

bool AuditRule::parse_json_rule() noexcept {
  assert(!m_json_rule_parsed);
  /*
   * Parsing default 'log' action.
   *
   * The top level 'log' action is the one defined in a rule
   * like '{"filter": { "log": true }}'. This is same as '{"filter": {}}'.
   * The behavior depends on whether 'class' or 'event' items are specified
   * for the rule:
   * - With 'log' explicitly specified, its given value is used.
   * - Without 'log' specified, logging is true if no 'class' or 'event' item
   *   is specified, and false otherwise (in which case, class or event can
   *   include their own log item).
   */
  m_json_rule_parsed = true;
  m_should_log_unmatched = true;

  if (m_json_rule_doc["filter"].ObjectEmpty()) {
    return true;
  }

  if (m_json_rule_doc["filter"].HasMember("log")) {
    if (!m_json_rule_doc["filter"]["log"].IsBool()) {
      LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                      "Wrong JSON filter '%s' format, "
                      "the 'log' member must be of type bool",
                      m_rule_name.c_str());
      return false;
    }

    m_should_log_unmatched = m_json_rule_doc["filter"]["log"].GetBool();
  } else if (m_json_rule_doc["filter"].HasMember("class")) {
    m_should_log_unmatched = false;
  }

  /*
   * Parsing event class name
   *
   * Possible formats of event class name definition within filtering rule:
   *
   *   "filter": {
   *     "class": { "log": true, "name": "class_name" }
   *   }
   *
   *   "filter": {
   *     "class": [
   *       { "name": [ "class_name_1", "class_name_2" ] }
   *     ]
   *   }
   *
   *   "filter": {
   *     "class": [
   *       { "name": "class_name_1" },
   *       { "name": "class_name_2" }
   *     ]
   *   }
   */
  if (!m_json_rule_doc["filter"].HasMember("class")) {
    return true;
  }

  const auto &ev_class = m_json_rule_doc["filter"]["class"];

  if (ev_class.IsObject()) {
    if (!parse_event_class_json(ev_class)) {
      return false;
    }
  } else if (ev_class.IsArray()) {
    for (auto it = ev_class.Begin(); it != ev_class.End(); ++it) {
      if (!it->IsObject()) {
        LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                        "Wrong JSON filter '%s' format, "
                        "'class' array element must be of object type",
                        m_rule_name.c_str());
        return false;
      }

      if (!parse_event_class_json(*it)) {
        return false;
      }
    }
  } else {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Wrong JSON filter '%s' format, "
                    "'class' must be an object or an array",
                    m_rule_name.c_str());
    return false;
  }

  return true;
}

bool AuditRule::parse_event_class_json(
    const rapidjson::Value &event_class_json) noexcept {
  assert(event_class_json.IsObject());

  if (!event_class_json.HasMember("name")) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Wrong JSON filter '%s' format, "
                    "no name provided for event class",
                    m_rule_name.c_str());
    return false;
  }

  if (event_class_json.HasMember("abort")) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Wrong JSON filter '%s' format, "
                    "'abort' condition should be set for subclass only",
                    m_rule_name.c_str());
    return false;
  }

  bool should_log = true;
  if (event_class_json.HasMember("log")) {
    if (event_class_json["log"].IsBool()) {
      should_log = event_class_json["log"].GetBool();
    } else {
      LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                      "Wrong JSON filter '%s' format, "
                      "'log' must be of bool type",
                      m_rule_name.c_str());
      return false;
    }
  }

  std::shared_ptr<EventFieldActionBase> replace_field;

  if (event_class_json.HasMember("print")) {
    replace_field =
        parse_action_json(EventActionType::ReplaceField, event_class_json);

    if (replace_field == nullptr) {
      LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                      "Wrong JSON filter '%s' format, "
                      "failed to parse 'print' replacement rule",
                      m_rule_name.c_str());
      return false;
    }
  }

  if (event_class_json["name"].IsString()) {
    const std::string event_class_name = event_class_json["name"].GetString();

    if (event_class_json.HasMember("event")) {
      if (replace_field != nullptr) {
        LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                        "Wrong JSON filter '%s' format, "
                        "replacement rule not expected for event class",
                        m_rule_name.c_str());
        return false;
      }

      // There is a subclass specific definition, it will provide actual action
      should_log = false;

      if (!parse_event_subclass(event_class_name, event_class_json["event"])) {
        return false;
      }
    }

    add_action_for_event(
        std::make_shared<EventFieldActionLog>(
            std::make_unique<EventFieldConditionBool>(should_log)),
        event_class_name);

    if (replace_field != nullptr) {
      add_action_for_event(replace_field, event_class_name);
    }
  } else if (event_class_json["name"].IsArray()) {
    // There may be no event subclass specified in case event class name is
    // defined as an array { "name": [ "class_name_1", "class_name_2" ] }
    if (event_class_json.HasMember("event")) {
      LogPluginErrMsg(
          ERROR_LEVEL, ER_LOG_PRINTF_MSG,
          "Wrong JSON filter '%s' format, there must be no 'event' in "
          "case class names provided as an array of strings",
          m_rule_name.c_str());
      return false;
    }

    std::shared_ptr<EventFieldActionBase> log_action =
        std::make_shared<EventFieldActionLog>(
            std::make_unique<EventFieldConditionBool>(should_log));

    for (auto it = event_class_json["name"].Begin();
         it != event_class_json["name"].End(); ++it) {
      if (!it->IsString()) {
        LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                        "Wrong JSON filter '%s' format, event class name "
                        "within an array should be of a string type",
                        m_rule_name.c_str());
        return false;
      }

      const std::string event_class_name = it->GetString();
      add_action_for_event(log_action, event_class_name);

      if (replace_field != nullptr) {
        add_action_for_event(replace_field, event_class_name);
      }
    }
  } else {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Wrong JSON filter '%s' format, event class name type "
                    "must be either string or an array of strings",
                    m_rule_name.c_str());
    return false;
  }

  return true;
}

bool AuditRule::parse_event_subclass(
    const std::string &class_name,
    const rapidjson::Value &event_subclass_json) noexcept {
  /*
   * Parse event subclass, 'event' may be an object or an array of objects
   *
   * "event": { "name": "read", "log": false }
   *
   * or
   *
   * "event": [
   *   { "name": "read", "log": false },
   *   { "name": "insert", "log": true }
   * ]
   *
   * or
   *
   * "event": { "name": [ "read", "insert" ] }
   */
  if (event_subclass_json.IsObject()) {
    if (!parse_event_subclass_json(class_name, event_subclass_json)) {
      return false;
    }
  } else if (event_subclass_json.IsArray()) {
    for (auto it = event_subclass_json.Begin(); it != event_subclass_json.End();
         ++it) {
      if (!it->IsObject()) {
        LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                        "Wrong JSON filter '%s' format, "
                        "'event' array element must be of object type",
                        m_rule_name.c_str());
        return false;
      }

      if (!parse_event_subclass_json(class_name, *it)) {
        return false;
      }
    }
  } else {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Wrong JSON filter '%s' format, type of 'event' "
                    "must be either an object or an array of objects",
                    m_rule_name.c_str());
    return false;
  }

  return true;
}

bool AuditRule::parse_event_subclass_json(
    const std::string &class_name,
    const rapidjson::Value &event_subclass_json) noexcept {
  assert(event_subclass_json.IsObject());

  if (!event_subclass_json.HasMember("name")) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Wrong JSON filter '%s' format, "
                    "no name provided for event subclass",
                    m_rule_name.c_str());
    return false;
  }

  std::vector<std::string> subclass_names;

  if (event_subclass_json["name"].IsString()) {
    subclass_names.emplace_back(event_subclass_json["name"].GetString());
  } else if (event_subclass_json["name"].IsArray()) {
    for (auto it = event_subclass_json["name"].Begin();
         it != event_subclass_json["name"].End(); ++it) {
      if (!it->IsString()) {
        LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                        "Wrong JSON filter '%s' format, event subclass name "
                        "within an array should be of a string type",
                        m_rule_name.c_str());
        return false;
      }

      subclass_names.emplace_back(it->GetString());
    }
  } else {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Wrong JSON filter '%s' format, event subclass name type "
                    "must be either string or an array of strings",
                    m_rule_name.c_str());
    return false;
  }

  const auto has_log_tag = event_subclass_json.HasMember("log");
  const auto has_abort_tag = event_subclass_json.HasMember("abort");

  if (has_log_tag && has_abort_tag) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Wrong JSON filter '%s' format, there must be only one "
                    "condition provided, 'log' or 'abort'",
                    m_rule_name.c_str());
    return false;
  }

  const EventActionType action_type =
      has_abort_tag ? EventActionType::Block : EventActionType::Log;

  std::shared_ptr<EventFieldActionBase> action =
      parse_action_json(action_type, event_subclass_json);

  if (action == nullptr) {
    return false;
  }

  std::shared_ptr<EventFieldActionBase> replace_field;

  if (event_subclass_json.HasMember("print")) {
    replace_field =
        parse_action_json(EventActionType::ReplaceField, event_subclass_json);

    if (replace_field == nullptr) {
      LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                      "Wrong JSON filter '%s' format, "
                      "failed to parse 'print' replacement rule",
                      m_rule_name.c_str());
      return false;
    }
  }

  std::shared_ptr<EventFieldActionBase> replace_filter;

  if (event_subclass_json.HasMember("filter")) {
    replace_filter =
        parse_action_json(EventActionType::ReplaceFilter, event_subclass_json);

    if (replace_filter == nullptr) {
      LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                      "Wrong JSON filter '%s' format, "
                      "failed to parse 'filter' replacement rule",
                      m_rule_name.c_str());
      return false;
    }
  }

  for (const auto &subclass_name : subclass_names) {
    add_action_for_event(action, class_name, subclass_name);

    if (replace_field != nullptr) {
      add_action_for_event(replace_field, class_name, subclass_name);
    }
    if (replace_filter != nullptr) {
      add_action_for_event(replace_filter, class_name, subclass_name);
    }
  }

  return true;
}

std::shared_ptr<EventFieldConditionBase> AuditRule::parse_condition(
    const rapidjson::Value &condition_json) noexcept {
  auto cond_type = get_condition_type(condition_json);

  if (cond_type == EventFieldConditionType::Unknown) {
    return nullptr;
  }

  return parse_condition_json(condition_json, cond_type);
}

EventFieldConditionType AuditRule::get_condition_type(
    const rapidjson::Value &json) noexcept {
  /*
   * There may be either bool or one of the condition objects provided:
   *
   * "log": true|false
   * "log": { "field": { } }
   * "log": { "and": [ ] }
   * "log": { "or": [ ] }
   * "log": { "not": { } }
   * "log": { "variable": { } }
   * "log": { "function": { } }
   */
  assert(json.IsBool() || json.IsObject());

  if (json.IsBool()) {
    return EventFieldConditionType::Bool;
  }

  if (json.MemberCount() != 1) {
    LogPluginErrMsg(
        ERROR_LEVEL, ER_LOG_PRINTF_MSG,
        "Wrong JSON filter '%s' format, "
        "there must be only one condition specified for 'log' field",
        m_rule_name.c_str());
    return EventFieldConditionType::Unknown;
  }

  const auto &condition = json.MemberBegin();

  if (!condition->name.IsString()) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Wrong JSON filter '%s' format, "
                    "the 'log' condition name must be of string type",
                    m_rule_name.c_str());
    return EventFieldConditionType::Unknown;
  }

  const std::string condition_name{condition->name.GetString()};

  if (condition_name == "field") {
    return EventFieldConditionType::Field;
  } else if (condition_name == "and") {
    return EventFieldConditionType::And;
  } else if (condition_name == "or") {
    return EventFieldConditionType::Or;
  } else if (condition_name == "not") {
    return EventFieldConditionType::Not;
  } else if (condition_name == "variable") {
    return EventFieldConditionType::Variable;
  } else if (condition_name == "function") {
    return EventFieldConditionType::Function;
  }

  LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                  "Wrong JSON filter '%s' format, "
                  "unknown 'log' condition name '%s'",
                  m_rule_name.c_str(), condition_name.c_str());

  return EventFieldConditionType::Unknown;
}

std::shared_ptr<EventFieldConditionBase> AuditRule::parse_condition_json(
    const rapidjson::Value &condition_json,
    const EventFieldConditionType cond_type) noexcept {
  assert(condition_json.IsBool() || condition_json.IsObject());

  switch (cond_type) {
    case EventFieldConditionType::Bool: {
      /*
       * Simple boolean true|false value.
       */
      return std::make_shared<EventFieldConditionBool>(
          condition_json.GetBool());
    }
    case EventFieldConditionType::Field: {
      /*
       * Parse 'field', must be an object containing field name and value
       *
       * "log": {
       *   "field": { "name": "general_command.str", "value": "Query" }
       * }
       */
      if (!condition_json["field"].IsObject()) {
        LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                        "Wrong JSON filter '%s' format, "
                        "condition definition 'field' must be of object type",
                        m_rule_name.c_str());
        return nullptr;
      }

      if (!condition_json["field"].HasMember("name") ||
          !condition_json["field"].HasMember("value") ||
          !condition_json["field"]["name"].IsString() ||
          !condition_json["field"]["value"].IsString()) {
        LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                        "Wrong JSON filter '%s' format, "
                        "event field definition 'field' must have field 'name' "
                        "and 'value' provided as strings",
                        m_rule_name.c_str());
        return nullptr;
      }

      std::string field_name{condition_json["field"]["name"].GetString()};
      std::string field_value{condition_json["field"]["value"].GetString()};

      if (field_name == CONNECTION_TYPE_FIELD_NAME) {
        /*
         * Handle special case for 'connection_type' field. It may contain
         * symbolic pseudo-constants that may be given instead of the literal
         * numeric values for connection type:
         *   0 or "::undefined": Undefined
         *   1 or "::tcp/ip": TCP/IP
         *   2 or "::socket": Socket
         *   3 or "::named_pipe": Named pipe
         *   4 or "::ssl": TCP/IP with encryption
         *   5 or "::shared_memory": Shared memory
         */
        update_connection_type_pseudo_to_numeric(field_value);
      }

      return std::make_shared<EventFieldConditionField>(std::move(field_name),
                                                        std::move(field_value));
    }
    case EventFieldConditionType::And: {
      /*
       * Parse 'and' condition, must be an array containing a list of other
       * condition objects
       *
       * "log": {
       *   "and": [
       *     { ... },
       *     { ... }
       *   ]
       * }
       */
      if (!condition_json["and"].IsArray()) {
        LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                        "Wrong JSON filter '%s' format, "
                        "condition definition 'and' must be of array type",
                        m_rule_name.c_str());
        return nullptr;
      }

      std::vector<std::shared_ptr<EventFieldConditionBase>> sub_conditions;

      for (auto it = condition_json["and"].Begin();
           it != condition_json["and"].End(); ++it) {
        if (!it->IsObject()) {
          LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                          "Wrong JSON filter '%s' format, "
                          "a member of 'and' condition must be of object type",
                          m_rule_name.c_str());
          return nullptr;
        }

        auto sub_cond_type = get_condition_type(*it);

        if (sub_cond_type == EventFieldConditionType::Unknown) {
          return nullptr;
        }

        auto condition = parse_condition_json(*it, sub_cond_type);

        if (condition == nullptr) {
          return nullptr;
        }

        sub_conditions.push_back(std::move(condition));
      }

      if (sub_conditions.size() < 2) {
        LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                        "Wrong JSON filter '%s' format, "
                        "there should be at least two fields provided for "
                        "'and' condition",
                        m_rule_name.c_str());
        return nullptr;
      }

      return std::make_shared<EventFieldConditionAnd>(
          std::move(sub_conditions));
    }
    case EventFieldConditionType::Or: {
      /*
       * Parse 'or' condition, must be an array containing a list of other
       * condition objects
       *
       * "log": {
       *   "or": [
       *     { ... },
       *     { ... }
       *   ]
       * }
       */
      if (!condition_json["or"].IsArray()) {
        LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                        "Wrong JSON filter '%s' format, "
                        "condition definition 'or' must be of array type",
                        m_rule_name.c_str());
        return nullptr;
      }

      std::vector<std::shared_ptr<EventFieldConditionBase>> sub_conditions;

      for (auto it = condition_json["or"].Begin();
           it != condition_json["or"].End(); ++it) {
        if (!it->IsObject()) {
          LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                          "Wrong JSON filter '%s' format, "
                          "a member of 'or' condition must be of object type",
                          m_rule_name.c_str());
          return nullptr;
        }

        auto sub_cond_type = get_condition_type(*it);

        if (sub_cond_type == EventFieldConditionType::Unknown) {
          return nullptr;
        }

        auto condition = parse_condition_json(*it, sub_cond_type);

        if (condition == nullptr) {
          return nullptr;
        }

        sub_conditions.push_back(std::move(condition));
      }

      if (sub_conditions.size() < 2) {
        LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                        "Wrong JSON filter '%s' format, "
                        "there should be at least two fields provided for "
                        "'or' condition",
                        m_rule_name.c_str());
        return nullptr;
      }

      return std::make_shared<EventFieldConditionOr>(std::move(sub_conditions));
    }
    case EventFieldConditionType::Not: {
      /*
       * Parse 'not' condition, must be an object containing another condition
       *
       * "log": {
       *   "not": { ... }
       * }
       */
      if (!condition_json["not"].IsObject()) {
        LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                        "Wrong JSON filter '%s' format, "
                        "condition definition 'not' must be of object type",
                        m_rule_name.c_str());
        return nullptr;
      }

      auto sub_cond_type = get_condition_type(condition_json["not"]);

      if (sub_cond_type == EventFieldConditionType::Unknown) {
        return nullptr;
      }

      auto condition =
          parse_condition_json(condition_json["not"], sub_cond_type);

      if (condition == nullptr) {
        return nullptr;
      }

      return std::make_shared<EventFieldConditionNot>(std::move(condition));
    }
    case EventFieldConditionType::Variable: {
      /*
       * Parse 'variable' condition, must be an object containing variable name
       * and expected result as strings
       *
       * "log": {
       *   "variable": {
       *     "name": "variable_name",
       *     "value": comparison_value
       *   }
       * }
       */
      if (!condition_json["variable"].IsObject()) {
        LogPluginErrMsg(
            ERROR_LEVEL, ER_LOG_PRINTF_MSG,
            "Wrong JSON filter '%s' format, "
            "condition definition 'variable' must be of object type",
            m_rule_name.c_str());
        return nullptr;
      }

      if (!condition_json["variable"].HasMember("name") ||
          !condition_json["variable"].HasMember("value") ||
          !condition_json["variable"]["name"].IsString() ||
          !condition_json["variable"]["value"].IsString()) {
        LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                        "Wrong JSON filter '%s' format, "
                        "event field definition 'variable' must have field "
                        "'name' and 'value' provided as strings",
                        m_rule_name.c_str());
        return nullptr;
      }

      return std::make_shared<EventFieldConditionVariable>(
          condition_json["variable"]["name"].GetString(),
          condition_json["variable"]["value"].GetString());
    }
    case EventFieldConditionType::Function: {
      /*
       * Parse 'function' condition, must be an object containing function name
       * and passed arguments as strings
       *
       * "log": {
       *   "function": {
       *     "name": "function_name",
       *     "args": arguments
       *   }
       * }
       */
      auto func =
          parse_function(condition_json["function"], FunctionReturnType::Bool);

      if (func == nullptr) {
        return nullptr;
      }

      return std::make_shared<EventFieldConditionFunction>(std::move(func));
    }
    default:
      assert(false);
  }

  return nullptr;
}

std::unique_ptr<EventFilterFunctionBase> AuditRule::parse_function(
    const rapidjson::Value &function_json,
    const FunctionReturnType expected_return_type) noexcept {
  if (!function_json.IsObject()) {
    LogPluginErrMsg(
        ERROR_LEVEL, ER_LOG_PRINTF_MSG,
        "Wrong JSON filter '%s' format, 'function' must be of object type",
        m_rule_name.c_str());
    return nullptr;
  }

  if (!function_json.HasMember("name") || !function_json["name"].IsString()) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Wrong JSON filter '%s' format, "
                    "missing 'function' name or not a string",
                    m_rule_name.c_str());
    return nullptr;
  }

  const std::string func_name = function_json["name"].GetString();
  const auto func_type = get_filter_function_type(func_name);

  if (func_type == EventFilterFunctionType::Unknown) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Wrong JSON filter '%s' format, "
                    "unknown function name '%s'",
                    m_rule_name.c_str(), func_name.c_str());
    return nullptr;
  }

  FunctionArgsList args;

  if (function_json.HasMember("args") &&
      !parse_function_args_json(function_json["args"], args)) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Wrong JSON filter '%s' format, "
                    "wrong function args format provided",
                    m_rule_name.c_str());
    return nullptr;
  }

  if (!validate_filter_function_args(func_type, args, expected_return_type)) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Wrong JSON filter '%s' format, "
                    "invalid arguments for '%s' function",
                    m_rule_name.c_str(), func_name.c_str());
    return nullptr;
  }

  return get_event_filter_function(func_type, args);
}

bool AuditRule::parse_function_args_json(
    const rapidjson::Value &function_args_json,
    FunctionArgsList &args) noexcept {
  /*
   * Parse 'function' arguments list, must be an array of objects with each
   * object containing argument type and its value along with the value source
   *
   * "function": {
   *   "name": "string_find",
   *   "args": [
   *     { "string": { "field": "table_name.str" } },
   *     { "string": { "string": "some str" } }
   *   ]
   * }
   */
  if (!function_args_json.IsArray()) {
    return false;
  }

  for (auto it = function_args_json.Begin(); it != function_args_json.End();
       ++it) {
    if (!it->IsObject() || it->MemberCount() != 1) {
      return false;
    }

    const auto arg_json = it->MemberBegin();

    if (!arg_json->name.IsString() || !arg_json->value.IsObject() ||
        arg_json->value.MemberCount() != 1) {
      return false;
    }

    const auto arg_value_json = arg_json->value.MemberBegin();

    if (!arg_value_json->name.IsString()) {
      return false;
    }

    const std::string arg_type_name = arg_json->name.GetString();
    const std::string arg_source_name = arg_value_json->name.GetString();

    const auto arg_type = get_filter_function_arg_type(arg_type_name);
    const auto arg_source_type =
        get_filter_function_arg_source_type(arg_source_name);

    if (arg_type == FunctionArgType::None ||
        arg_source_type == FunctionArgSourceType::None) {
      return false;
    }

    if (arg_type == FunctionArgType::String &&
        !arg_value_json->value.IsString()) {
      return false;
    }

    args.push_back(
        {arg_type, arg_source_type, arg_value_json->value.GetString()});
  }

  return true;
}

std::shared_ptr<EventFieldActionBase> AuditRule::parse_action_json(
    const EventActionType action_type,
    const rapidjson::Value &action_json) noexcept {
  assert(action_json.IsObject());

  switch (action_type) {
    case EventActionType::Log: {
      std::shared_ptr<EventFieldConditionBase> log_cond;

      if (action_json.HasMember("log")) {
        log_cond = parse_condition(action_json["log"]);

        if (log_cond == nullptr) {
          return nullptr;
        }
      } else {
        // default to enabled
        log_cond = std::make_shared<EventFieldConditionBool>(true);
      }

      return std::make_shared<EventFieldActionLog>(log_cond);
    }
    case EventActionType::Block: {
      auto block_cond = parse_condition(action_json["abort"]);

      if (block_cond == nullptr) {
        LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                        "Wrong JSON filter '%s' format, "
                        "'abort' must be of bool or object type",
                        m_rule_name.c_str());
        return nullptr;
      }

      return std::make_shared<EventFieldActionBlock>(block_cond);
    }
    case EventActionType::ReplaceField: {
      /*
       * Parse event field action
       *
       * "print": {
       *   "field": {
       *     "name": "field_name",
       *     "print": condition,
       *     "replace": replacement_value
       *   }
       * }
       */
      if (!action_json["print"].IsObject() ||
          action_json["print"].MemberCount() != 1 ||
          !action_json["print"].HasMember("field") ||
          !action_json["print"]["field"].IsObject()) {
        return nullptr;
      }

      const auto &field_json = action_json["print"]["field"];

      // Required fields
      if (!field_json.HasMember("name") || !field_json.HasMember("print") ||
          !field_json.HasMember("replace")) {
        return nullptr;
      }

      if (!field_json["name"].IsString()) {
        return nullptr;
      }

      std::string replaced_field_name = field_json["name"].GetString();

      if (!EventFieldActionReplaceField::validate_field_name(
              replaced_field_name)) {
        LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                        "Event field '%s' cannot be replaced",
                        replaced_field_name.c_str());
        return nullptr;
      }

      auto print_cond = parse_condition(field_json["print"]);

      if (print_cond == nullptr) {
        return nullptr;
      }

      if (!field_json["replace"].IsObject() ||
          !field_json["replace"].HasMember("function")) {
        return nullptr;
      }

      auto replacement_func = parse_function(field_json["replace"]["function"],
                                             FunctionReturnType::String);

      if (replacement_func == nullptr) {
        return nullptr;
      }

      return std::make_shared<EventFieldActionReplaceField>(
          replaced_field_name, print_cond, std::move(replacement_func));
    }
    case EventActionType::ReplaceFilter: {
      /*
       * "filter": {
       *   "class": {
       *     "name": "general",
       *     "event" : { "name": "status",
       *                 "filter": { "ref": "main" } }
       *   },
       *   "activate": {
       *     "or": [
       *       { "field": { "name": "table_name.str", "value": "temp_1" } },
       *       { "field": { "name": "table_name.str", "value": "temp_2" } }
       *     ]
       *   }
       * }
       */
      if (!action_json["filter"].IsObject()) {
        return nullptr;
      }

      if (action_json["filter"].MemberCount() == 1 &&
          action_json["filter"].HasMember("ref") &&
          action_json["filter"]["ref"].IsString()) {
        // This is a filter defined within temporary replacement rule,
        // here "ref" points to original rule making it effective again.
        return std::make_shared<EventFieldActionReplaceFilter>(
            action_json["filter"]["ref"].GetString());
      }

      if (action_json["filter"].MemberCount() != 2 ||
          !action_json["filter"].HasMember("class") ||
          !action_json["filter"].HasMember("activate") ||
          !action_json["filter"]["class"].IsObject() ||
          !action_json["filter"]["activate"].IsObject()) {
        return nullptr;
      }

      auto activation_cond = parse_condition(action_json["filter"]["activate"]);

      if (activation_cond == nullptr) {
        return nullptr;
      }

      auto replacement_rule =
          make_replacement_rule(action_json["filter"]["class"]);

      if (!replacement_rule->check_parse_state()) {
        return nullptr;
      }

      return std::make_shared<EventFieldActionReplaceFilter>(
          std::move(activation_cond), std::move(replacement_rule));
    }
    default:
      assert(false);
  }

  return nullptr;
}

std::shared_ptr<AuditRule> AuditRule::make_replacement_rule(
    const rapidjson::Value &rule_json) noexcept {
  rapidjson::Document d;
  d.SetObject();
  d.AddMember("filter", rapidjson::Value{rapidjson::kObjectType},
              d.GetAllocator());
  d["filter"].AddMember("class", rapidjson::Value{rapidjson::kObjectType},
                        d.GetAllocator());
  d["filter"]["class"].CopyFrom(rule_json, d.GetAllocator());

  return std::shared_ptr<AuditRule>(new AuditRule{std::move(d)});
}

}  // namespace audit_log_filter
