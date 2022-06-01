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
namespace {

const std::string CONDITION_TAG_LOG{"log"};
const std::string CONDITION_TAG_ABORT{"abort"};

}  // namespace

AuditRule::AuditRule(const char *rule_name, const char *rule_str)
    : AuditRule{0, rule_name, rule_str} {}

AuditRule::AuditRule(uint64_t filter_id, const char *rule_name,
                     const char *rule_str)
    : m_filter_id{filter_id},
      m_rule_name{rule_name},
      m_json_rule_parsed{false},
      m_json_rule_format_valid{false},
      m_global_action{AuditAction::Skip} {
  m_json_rule_doc.Parse(rule_str);
}

std::string AuditRule::to_string() noexcept {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  m_json_rule_doc.Accept(writer);

  return buffer.GetString();
}

uint64_t AuditRule::get_filter_id() const noexcept { return m_filter_id; }

std::string AuditRule::get_rule_name() const noexcept { return m_rule_name; }

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

AuditAction AuditRule::get_global_action() const noexcept {
  assert(m_json_rule_parsed);
  return m_global_action;
}

AuditAction AuditRule::get_event_class_action(
    const std::string_view event_class_name) const noexcept {
  assert(m_json_rule_parsed);
  auto it = m_class_to_action_map.find(event_class_name.data());

  if (it == m_class_to_action_map.end()) {
    return AuditAction::None;
  }

  return it->second;
}

AuditAction AuditRule::get_event_subclass_action(
    const std::string_view event_class_name,
    const std::string_view event_subclass_name,
    const AuditRecordFieldsList &fields) const noexcept {
  assert(m_json_rule_parsed);

  std::stringstream subclass_key;
  subclass_key << event_class_name << "." << event_subclass_name;

  const auto cond = m_subclass_to_condition_map.find(subclass_key.str());

  if (cond != m_subclass_to_condition_map.cend()) {
    return cond->second->check_applies(fields);
  }

  return AuditAction::None;
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
  m_global_action = AuditAction::Log;

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

    if (!m_json_rule_doc["filter"]["log"].GetBool()) {
      m_global_action = AuditAction::Skip;
    }
  } else if (m_json_rule_doc["filter"].HasMember("class")) {
    m_global_action = AuditAction::Skip;
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
    if (!parse_event_class_obj(ev_class)) {
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

      if (!parse_event_class_obj(*it)) {
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

bool AuditRule::parse_event_class_obj(
    const rapidjson::Value &event_class_obj) noexcept {
  assert(event_class_obj.IsObject());

  if (!event_class_obj.HasMember("name")) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Wrong JSON filter '%s' format, "
                    "no name provided for event class",
                    m_rule_name.c_str());
    return false;
  }

  if (event_class_obj.HasMember("abort")) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Wrong JSON filter '%s' format, "
                    "'abort' condition should be set for subclass only",
                    m_rule_name.c_str());
    return false;
  }

  AuditAction action = AuditAction::Log;
  if (event_class_obj.HasMember("log")) {
    if (event_class_obj["log"].IsBool()) {
      if (!event_class_obj["log"].GetBool()) {
        action = AuditAction::Skip;
      }
    } else {
      LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                      "Wrong JSON filter '%s' format, "
                      "'log' must be of bool type",
                      m_rule_name.c_str());
      return false;
    }
  }

  if (event_class_obj["name"].IsString()) {
    if (event_class_obj.HasMember("event")) {
      // There is a subclass specific definition, it will provide actual action
      action = AuditAction::Skip;

      if (!parse_event_subclass(event_class_obj["name"].GetString(),
                                event_class_obj["event"])) {
        return false;
      }
    }

    m_class_to_action_map.insert({event_class_obj["name"].GetString(), action});
  } else if (event_class_obj["name"].IsArray()) {
    // There may be no event subclass specified in case event class name is
    // defined as an array { "name": [ "class_name_1", "class_name_2" ] }
    if (event_class_obj.HasMember("event")) {
      LogPluginErrMsg(
          ERROR_LEVEL, ER_LOG_PRINTF_MSG,
          "Wrong JSON filter '%s' format, there must be no 'event' in "
          "case class names provided as an array of strings",
          m_rule_name.c_str());
      return false;
    }

    for (auto it = event_class_obj["name"].Begin();
         it != event_class_obj["name"].End(); ++it) {
      if (!it->IsString()) {
        LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                        "Wrong JSON filter '%s' format, event class name "
                        "within an array should be of a string type",
                        m_rule_name.c_str());
        return false;
      }

      m_class_to_action_map.insert({it->GetString(), action});
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
    const rapidjson::Value &event_subclass_obj) noexcept {
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
  if (event_subclass_obj.IsObject()) {
    if (!parse_event_subclass_obj(class_name, event_subclass_obj)) {
      return false;
    }
  } else if (event_subclass_obj.IsArray()) {
    for (auto it = event_subclass_obj.Begin(); it != event_subclass_obj.End();
         ++it) {
      if (!it->IsObject()) {
        LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                        "Wrong JSON filter '%s' format, "
                        "'event' array element must be of object type",
                        m_rule_name.c_str());
        return false;
      }

      if (!parse_event_subclass_obj(class_name, *it)) {
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

bool AuditRule::parse_event_subclass_obj(
    const std::string &class_name,
    const rapidjson::Value &event_subclass_obj) noexcept {
  assert(event_subclass_obj.IsObject());

  if (!event_subclass_obj.HasMember("name")) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Wrong JSON filter '%s' format, "
                    "no name provided for event subclass",
                    m_rule_name.c_str());
    return false;
  }

  std::vector<std::string> cond_keys;

  if (event_subclass_obj["name"].IsString()) {
    std::stringstream cond_key;
    cond_key << class_name << "." << event_subclass_obj["name"].GetString();
    cond_keys.push_back(cond_key.str());
  } else if (event_subclass_obj["name"].IsArray()) {
    for (auto it = event_subclass_obj["name"].Begin();
         it != event_subclass_obj["name"].End(); ++it) {
      if (!it->IsString()) {
        LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                        "Wrong JSON filter '%s' format, event subclass name "
                        "within an array should be of a string type",
                        m_rule_name.c_str());
        return false;
      }

      std::stringstream cond_key;
      cond_key << class_name << "." << it->GetString();
      cond_keys.push_back(cond_key.str());
    }
  } else {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Wrong JSON filter '%s' format, event subclass name type "
                    "must be either string or an array of strings",
                    m_rule_name.c_str());
    return false;
  }

  const auto has_log = event_subclass_obj.HasMember("log");
  const auto has_abort = event_subclass_obj.HasMember("abort");

  if (has_log && has_abort) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Wrong JSON filter '%s' format, there must be only one "
                    "condition provided, 'log' or 'abort'",
                    m_rule_name.c_str());
    return false;
  }

  AuditAction action = AuditAction::Log;
  std::string condition_tag = CONDITION_TAG_LOG;

  if (has_abort) {
    action = AuditAction::Block;
    condition_tag = CONDITION_TAG_ABORT;
  }

  if (event_subclass_obj.HasMember(condition_tag.c_str())) {
    const auto &condition_obj = event_subclass_obj[condition_tag.c_str()];

    if (condition_obj.IsBool()) {
      if (!condition_obj.GetBool()) {
        action = AuditAction::Skip;
      }
    } else if (condition_obj.IsObject()) {
      return parse_event_field(cond_keys, action, condition_obj);
    } else {
      LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                      "Wrong JSON filter '%s' format, "
                      "'log' must be of bool or object type",
                      m_rule_name.c_str());
      return false;
    }
  }

  for (const auto &cond_key : cond_keys) {
    m_subclass_to_condition_map.insert(
        {cond_key, std::make_unique<EventFieldConditionBool>(action)});
  }

  return true;
}

bool AuditRule::parse_event_field(
    const std::vector<std::string> &cond_keys, const AuditAction cond_action,
    const rapidjson::Value &event_field_obj) noexcept {
  /*
   * Parse event field, must be an object containing field name and value
   *
   * "log": {
   *   "field": { "name": "general_command.str", "value": "Query" }
   * }
   */
  assert(event_field_obj.IsObject());

  auto cond_type = get_condition_type(event_field_obj);

  if (cond_type == EventFieldConditionType::Unknown) {
    return false;
  }

  auto cond = parse_condition_obj(event_field_obj, cond_type, cond_action);

  if (cond == nullptr) {
    return false;
  }

  for (const auto &cond_key : cond_keys) {
    m_subclass_to_condition_map.insert({cond_key, cond});
  }

  return true;
}

EventFieldConditionType AuditRule::get_condition_type(
    const rapidjson::Value &json) noexcept {
  /*
   * There may be one of the following conditions provided:
   *
   * "log": { "field": { } }
   * "log": { "and": [ ] }
   * "log": { "or": [ ] }
   * "log": { "not": { } }
   * "log": { "variable": { } }
   * "log": { "function": { } }
   */
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

std::shared_ptr<EventFieldConditionBase> AuditRule::parse_condition_obj(
    const rapidjson::Value &event_field_obj,
    const EventFieldConditionType cond_type,
    const AuditAction cond_action) noexcept {
  switch (cond_type) {
    case EventFieldConditionType::Field: {
      /*
       * Parse 'field', must be an object containing field name and value
       *
       * "log": {
       *   "field": { "name": "general_command.str", "value": "Query" }
       * }
       */
      if (!event_field_obj["field"].IsObject()) {
        LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                        "Wrong JSON filter '%s' format, "
                        "condition definition 'field' must be of object type",
                        m_rule_name.c_str());
        return nullptr;
      }

      if (!event_field_obj["field"].HasMember("name") ||
          !event_field_obj["field"].HasMember("value") ||
          !event_field_obj["field"]["name"].IsString() ||
          !event_field_obj["field"]["value"].IsString()) {
        LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                        "Wrong JSON filter '%s' format, "
                        "event field definition 'field' must have field 'name' "
                        "and 'value' provided as strings",
                        m_rule_name.c_str());
        return nullptr;
      }

      std::string field_name{event_field_obj["field"]["name"].GetString()};
      std::string field_value{event_field_obj["field"]["value"].GetString()};

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

      return std::make_shared<EventFieldConditionField>(
          std::move(field_name), std::move(field_value), cond_action);
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
      if (!event_field_obj["and"].IsArray()) {
        LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                        "Wrong JSON filter '%s' format, "
                        "condition definition 'and' must be of array type",
                        m_rule_name.c_str());
        return nullptr;
      }

      std::vector<std::shared_ptr<EventFieldConditionBase>> sub_conditions;

      for (auto it = event_field_obj["and"].Begin();
           it != event_field_obj["and"].End(); ++it) {
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

        auto condition = parse_condition_obj(*it, sub_cond_type, cond_action);

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

      return std::make_shared<EventFieldConditionAnd>(std::move(sub_conditions),
                                                      cond_action);
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
      if (!event_field_obj["or"].IsArray()) {
        LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                        "Wrong JSON filter '%s' format, "
                        "condition definition 'or' must be of array type",
                        m_rule_name.c_str());
        return nullptr;
      }

      std::vector<std::shared_ptr<EventFieldConditionBase>> sub_conditions;

      for (auto it = event_field_obj["or"].Begin();
           it != event_field_obj["or"].End(); ++it) {
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

        auto condition = parse_condition_obj(*it, sub_cond_type, cond_action);

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

      return std::make_shared<EventFieldConditionOr>(std::move(sub_conditions),
                                                     cond_action);
    }
    case EventFieldConditionType::Not: {
      /*
       * Parse 'not' condition, must be an object containing another condition
       *
       * "log": {
       *   "not": { ... }
       * }
       */
      if (!event_field_obj["not"].IsObject()) {
        LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                        "Wrong JSON filter '%s' format, "
                        "condition definition 'not' must be of object type",
                        m_rule_name.c_str());
        return nullptr;
      }

      auto sub_cond_type = get_condition_type(event_field_obj["not"]);

      if (sub_cond_type == EventFieldConditionType::Unknown) {
        return nullptr;
      }

      auto condition = parse_condition_obj(event_field_obj["not"],
                                           sub_cond_type, cond_action);

      if (condition == nullptr) {
        return nullptr;
      }

      return std::make_shared<EventFieldConditionNot>(std::move(condition),
                                                      cond_action);
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
      if (!event_field_obj["variable"].IsObject()) {
        LogPluginErrMsg(
            ERROR_LEVEL, ER_LOG_PRINTF_MSG,
            "Wrong JSON filter '%s' format, "
            "condition definition 'variable' must be of object type",
            m_rule_name.c_str());
        return nullptr;
      }

      if (!event_field_obj["variable"].HasMember("name") ||
          !event_field_obj["variable"].HasMember("value") ||
          !event_field_obj["variable"]["name"].IsString() ||
          !event_field_obj["variable"]["value"].IsString()) {
        LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                        "Wrong JSON filter '%s' format, "
                        "event field definition 'variable' must have field "
                        "'name' and 'value' provided as strings",
                        m_rule_name.c_str());
        return nullptr;
      }

      return std::make_shared<EventFieldConditionVariable>(
          event_field_obj["variable"]["name"].GetString(),
          event_field_obj["variable"]["value"].GetString(), cond_action);
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
      if (!event_field_obj["function"].IsObject()) {
        LogPluginErrMsg(
            ERROR_LEVEL, ER_LOG_PRINTF_MSG,
            "Wrong JSON filter '%s' format, "
            "condition definition 'function' must be of object type",
            m_rule_name.c_str());
        return nullptr;
      }

      if (!event_field_obj["function"].HasMember("name") ||
          !event_field_obj["function"]["name"].IsString()) {
        LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                        "Wrong JSON filter '%s' format, "
                        "missing 'function' name or not a string",
                        m_rule_name.c_str());
        return nullptr;
      }

      const std::string function_name{
          event_field_obj["function"]["name"].GetString()};

      if (!EventFieldConditionFunction::check_function_name(function_name)) {
        LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                        "Wrong JSON filter '%s' format, "
                        "unknown function name '%s'",
                        m_rule_name.c_str(), function_name.c_str());
        return nullptr;
      }

      FunctionArgsList args;

      if (event_field_obj["function"].HasMember("args") &&
          !parse_function_args_obj(event_field_obj["function"]["args"], args)) {
        LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                        "Wrong JSON filter '%s' format, "
                        "wrong function args format provided",
                        m_rule_name.c_str());
        return nullptr;
      }

      if (!EventFieldConditionFunction::validate_args(function_name, args)) {
        return nullptr;
      }

      return std::make_shared<EventFieldConditionFunction>(function_name, args,
                                                           cond_action);
    }
    default:
      assert(false);
  }

  return nullptr;
}

bool AuditRule::parse_function_args_obj(
    const rapidjson::Value &function_args_obj,
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
  if (!function_args_obj.IsArray()) {
    return false;
  }

  for (auto it = function_args_obj.Begin(); it != function_args_obj.End();
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

    const auto arg_type =
        EventFieldConditionFunction::get_function_arg_type(arg_type_name);
    const auto arg_source_type =
        EventFieldConditionFunction::get_function_arg_source_type(
            arg_source_name);

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

}  // namespace audit_log_filter
