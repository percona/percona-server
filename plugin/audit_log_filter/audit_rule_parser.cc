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

#include "plugin/audit_log_filter/audit_rule_parser.h"
#include "plugin/audit_log_filter/audit_error_log.h"

#include "plugin/audit_log_filter/event_field_action/block.h"
#include "plugin/audit_log_filter/event_field_action/log.h"
#include "plugin/audit_log_filter/event_field_action/print_query_attrs.h"
#include "plugin/audit_log_filter/event_field_action/print_service_comp.h"
#include "plugin/audit_log_filter/event_field_action/replace_filter.h"
#include "plugin/audit_log_filter/event_field_condition/and.h"
#include "plugin/audit_log_filter/event_field_condition/bool.h"
#include "plugin/audit_log_filter/event_field_condition/field.h"
#include "plugin/audit_log_filter/event_field_condition/function.h"
#include "plugin/audit_log_filter/event_field_condition/not.h"
#include "plugin/audit_log_filter/event_field_condition/or.h"
#include "plugin/audit_log_filter/event_field_condition/variable.h"

#include <memory>

namespace audit_log_filter {
namespace {}  // namespace

bool AuditRuleParser::parse(const char *rule_str,
                            AuditRule *audit_rule) noexcept {
  rapidjson::Document json_doc;
  json_doc.Parse(rule_str);

  return parse(json_doc, audit_rule);
}

bool AuditRuleParser::parse(rapidjson::Document &json_doc,
                            AuditRule *audit_rule) noexcept {
  // Do basic check of rule structure
  if (json_doc.HasParseError()) {
    return false;
  }

  // The root of the JSON rule must be an object
  if (!json_doc.IsObject()) {
    return false;
  }

  // The basic JSON rule format must be like the following: '{"filter": {}}'
  if (!json_doc.HasMember("filter") || !json_doc["filter"].IsObject()) {
    return false;
  }

  if (!parse_default_log_action_json(json_doc, audit_rule)) {
    return false;
  }

  if (!parse_event_class_json(json_doc, audit_rule)) {
    return false;
  }

  return true;
}

bool AuditRuleParser::parse_default_log_action_json(
    const rapidjson::Document &json_doc, AuditRule *audit_rule) noexcept {
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
  bool should_log_unmatched = true;

  if (json_doc["filter"].ObjectEmpty()) {
    return true;
  }

  if (json_doc["filter"].HasMember("log")) {
    if (!json_doc["filter"]["log"].IsBool()) {
      LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                      "Wrong JSON filter '%s' format, "
                      "the 'log' member must be of type bool",
                      audit_rule->get_rule_name().c_str());
      return false;
    }

    should_log_unmatched = json_doc["filter"]["log"].GetBool();
  } else if (json_doc["filter"].HasMember("class")) {
    should_log_unmatched = false;
  }

  audit_rule->set_should_log_unmatched(should_log_unmatched);

  return true;
}

bool AuditRuleParser::parse_event_class_json(
    const rapidjson::Document &json_doc, AuditRule *audit_rule) noexcept {
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
  if (!json_doc["filter"].HasMember("class")) {
    return true;
  }

  const auto &ev_class_json = json_doc["filter"]["class"];

  if (ev_class_json.IsObject()) {
    return parse_event_class_obj_json(ev_class_json, audit_rule);
  } else if (ev_class_json.IsArray()) {
    for (auto it = ev_class_json.Begin(); it != ev_class_json.End(); ++it) {
      if (!it->IsObject()) {
        LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                        "Wrong JSON filter '%s' format, "
                        "'class' array element must be of object type",
                        audit_rule->get_rule_name().c_str());
        return false;
      }

      if (!parse_event_class_obj_json(*it, audit_rule)) {
        return false;
      }
    }
  } else {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Wrong JSON filter '%s' format, "
                    "'class' must be an object or an array",
                    audit_rule->get_rule_name().c_str());
    return false;
  }

  return true;
}

bool AuditRuleParser::parse_event_class_obj_json(
    const rapidjson::Value &event_class_json, AuditRule *audit_rule) noexcept {
  assert(event_class_json.IsObject());

  if (!event_class_json.HasMember("name")) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Wrong JSON filter '%s' format, "
                    "no name provided for event class",
                    audit_rule->get_rule_name().c_str());
    return false;
  }

  if (event_class_json.HasMember("abort")) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Wrong JSON filter '%s' format, "
                    "'abort' condition should be set for subclass only",
                    audit_rule->get_rule_name().c_str());
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
                      audit_rule->get_rule_name().c_str());
      return false;
    }
  }

  std::shared_ptr<EventFieldActionBase> replace_field;

  if (event_class_json.HasMember("print")) {
    replace_field = parse_action_json(EventActionType::ReplaceField,
                                      event_class_json, audit_rule);

    if (replace_field == nullptr) {
      LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                      "Wrong JSON filter '%s' format, "
                      "failed to parse 'print' replacement rule",
                      audit_rule->get_rule_name().c_str());
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
                        audit_rule->get_rule_name().c_str());
        return false;
      }

      // There is a subclass specific definition, it will provide actual action
      should_log = false;

      if (!parse_event_subclass_json(event_class_name,
                                     event_class_json["event"], audit_rule)) {
        return false;
      }
    }

    audit_rule->add_action_for_event(
        std::make_shared<EventFieldActionLog>(
            std::make_unique<EventFieldConditionBool>(should_log)),
        event_class_name);

    if (replace_field != nullptr) {
      audit_rule->add_action_for_event(replace_field, event_class_name);
    }
  } else if (event_class_json["name"].IsArray()) {
    // There may be no event subclass specified in case event class name is
    // defined as an array { "name": [ "class_name_1", "class_name_2" ] }
    if (event_class_json.HasMember("event")) {
      LogPluginErrMsg(
          ERROR_LEVEL, ER_LOG_PRINTF_MSG,
          "Wrong JSON filter '%s' format, there must be no 'event' in "
          "case class names provided as an array of strings",
          audit_rule->get_rule_name().c_str());
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
                        audit_rule->get_rule_name().c_str());
        return false;
      }

      const std::string event_class_name = it->GetString();
      audit_rule->add_action_for_event(log_action, event_class_name);

      if (replace_field != nullptr) {
        audit_rule->add_action_for_event(replace_field, event_class_name);
      }
    }
  } else {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Wrong JSON filter '%s' format, event class name type "
                    "must be either string or an array of strings",
                    audit_rule->get_rule_name().c_str());
    return false;
  }

  return true;
}

bool AuditRuleParser::parse_event_subclass_json(
    const std::string &class_name, const rapidjson::Value &event_subclass_json,
    AuditRule *audit_rule) noexcept {
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
    if (!parse_event_subclass_obj_json(class_name, event_subclass_json,
                                       audit_rule)) {
      return false;
    }
  } else if (event_subclass_json.IsArray()) {
    for (auto it = event_subclass_json.Begin(); it != event_subclass_json.End();
         ++it) {
      if (!it->IsObject()) {
        LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                        "Wrong JSON filter '%s' format, "
                        "'event' array element must be of object type",
                        audit_rule->get_rule_name().c_str());
        return false;
      }

      if (!parse_event_subclass_obj_json(class_name, *it, audit_rule)) {
        return false;
      }
    }
  } else {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Wrong JSON filter '%s' format, type of 'event' "
                    "must be either an object or an array of objects",
                    audit_rule->get_rule_name().c_str());
    return false;
  }

  return true;
}

bool AuditRuleParser::parse_event_subclass_obj_json(
    const std::string &class_name, const rapidjson::Value &event_subclass_json,
    AuditRule *audit_rule) noexcept {
  assert(event_subclass_json.IsObject());

  if (!event_subclass_json.HasMember("name")) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Wrong JSON filter '%s' format, "
                    "no name provided for event subclass",
                    audit_rule->get_rule_name().c_str());
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
                        audit_rule->get_rule_name().c_str());
        return false;
      }

      subclass_names.emplace_back(it->GetString());
    }
  } else {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Wrong JSON filter '%s' format, event subclass name type "
                    "must be either string or an array of strings",
                    audit_rule->get_rule_name().c_str());
    return false;
  }

  const auto has_log_tag = event_subclass_json.HasMember("log");
  const auto has_abort_tag = event_subclass_json.HasMember("abort");

  if (has_log_tag && has_abort_tag) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Wrong JSON filter '%s' format, there must be only one "
                    "condition provided, 'log' or 'abort'",
                    audit_rule->get_rule_name().c_str());
    return false;
  }

  const EventActionType log_action_type =
      has_abort_tag ? EventActionType::Block : EventActionType::Log;

  std::shared_ptr<EventFieldActionBase> log_action =
      parse_action_json(log_action_type, event_subclass_json, audit_rule);

  if (log_action == nullptr) {
    return false;
  }

  std::vector<std::shared_ptr<EventFieldActionBase>> actions_list;
  actions_list.push_back(log_action);

  if (event_subclass_json.HasMember("print")) {
    // There may be a few actions modifying record content defined within
    // "print" tag
    for (auto it = event_subclass_json["print"].MemberBegin();
         it != event_subclass_json["print"].MemberEnd(); ++it) {
      const auto action_type =
          event_field_action::get_event_action_type(it->name.GetString());

      if (action_type == EventActionType::Unknown) {
        LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                        "Wrong JSON filter '%s' format, "
                        "unknown tag '%s'",
                        audit_rule->get_rule_name().c_str(),
                        it->name.GetString());
        return false;
      }

      auto action =
          parse_action_json(action_type, event_subclass_json, audit_rule);

      if (action == nullptr) {
        LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                        "Wrong JSON filter '%s' format, "
                        "bad format for '%s' action",
                        audit_rule->get_rule_name().c_str(),
                        it->name.GetString());
        return false;
      }

      actions_list.push_back(action);
    }
  }

  std::shared_ptr<EventFieldActionBase> replace_filter_action;

  if (event_subclass_json.HasMember("filter")) {
    replace_filter_action = parse_action_json(EventActionType::ReplaceFilter,
                                              event_subclass_json, audit_rule);

    if (replace_filter_action == nullptr) {
      LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                      "Wrong JSON filter '%s' format, "
                      "failed to parse 'filter' replacement rule",
                      audit_rule->get_rule_name().c_str());
      return false;
    }

    actions_list.push_back(replace_filter_action);
  }

  for (const auto &subclass_name : subclass_names) {
    for (const auto &action_ptr : actions_list) {
      audit_rule->add_action_for_event(action_ptr, class_name, subclass_name);
    }
  }

  return true;
}

std::shared_ptr<EventFieldConditionBase> AuditRuleParser::parse_condition(
    const rapidjson::Value &condition_json, AuditRule *audit_rule) noexcept {
  auto cond_type = get_condition_type(condition_json, audit_rule);

  if (cond_type == EventFieldConditionType::Unknown) {
    return nullptr;
  }

  return parse_condition_json(condition_json, cond_type, audit_rule);
}

EventFieldConditionType AuditRuleParser::get_condition_type(
    const rapidjson::Value &json, AuditRule *audit_rule) noexcept {
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
        audit_rule->get_rule_name().c_str());
    return EventFieldConditionType::Unknown;
  }

  const auto &condition = json.MemberBegin();

  if (!condition->name.IsString()) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Wrong JSON filter '%s' format, "
                    "the 'log' condition name must be of string type",
                    audit_rule->get_rule_name().c_str());
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
                  audit_rule->get_rule_name().c_str(), condition_name.c_str());

  return EventFieldConditionType::Unknown;
}

std::shared_ptr<EventFieldConditionBase> AuditRuleParser::parse_condition_json(
    const rapidjson::Value &condition_json,
    const EventFieldConditionType cond_type, AuditRule *audit_rule) noexcept {
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
                        audit_rule->get_rule_name().c_str());
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
                        audit_rule->get_rule_name().c_str());
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
                        audit_rule->get_rule_name().c_str());
        return nullptr;
      }

      std::vector<std::shared_ptr<EventFieldConditionBase>> sub_conditions;

      for (auto it = condition_json["and"].Begin();
           it != condition_json["and"].End(); ++it) {
        if (!it->IsObject()) {
          LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                          "Wrong JSON filter '%s' format, "
                          "a member of 'and' condition must be of object type",
                          audit_rule->get_rule_name().c_str());
          return nullptr;
        }

        auto sub_cond_type = get_condition_type(*it, audit_rule);

        if (sub_cond_type == EventFieldConditionType::Unknown) {
          return nullptr;
        }

        auto condition = parse_condition_json(*it, sub_cond_type, audit_rule);

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
                        audit_rule->get_rule_name().c_str());
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
                        audit_rule->get_rule_name().c_str());
        return nullptr;
      }

      std::vector<std::shared_ptr<EventFieldConditionBase>> sub_conditions;

      for (auto it = condition_json["or"].Begin();
           it != condition_json["or"].End(); ++it) {
        if (!it->IsObject()) {
          LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                          "Wrong JSON filter '%s' format, "
                          "a member of 'or' condition must be of object type",
                          audit_rule->get_rule_name().c_str());
          return nullptr;
        }

        auto sub_cond_type = get_condition_type(*it, audit_rule);

        if (sub_cond_type == EventFieldConditionType::Unknown) {
          return nullptr;
        }

        auto condition = parse_condition_json(*it, sub_cond_type, audit_rule);

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
                        audit_rule->get_rule_name().c_str());
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
                        audit_rule->get_rule_name().c_str());
        return nullptr;
      }

      auto sub_cond_type =
          get_condition_type(condition_json["not"], audit_rule);

      if (sub_cond_type == EventFieldConditionType::Unknown) {
        return nullptr;
      }

      auto condition = parse_condition_json(condition_json["not"],
                                            sub_cond_type, audit_rule);

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
            audit_rule->get_rule_name().c_str());
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
                        audit_rule->get_rule_name().c_str());
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
      auto func = parse_function(condition_json["function"],
                                 FunctionReturnType::Bool, audit_rule);

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

std::unique_ptr<EventFilterFunctionBase> AuditRuleParser::parse_function(
    const rapidjson::Value &function_json,
    const FunctionReturnType expected_return_type,
    AuditRule *audit_rule) noexcept {
  if (!function_json.IsObject()) {
    LogPluginErrMsg(
        ERROR_LEVEL, ER_LOG_PRINTF_MSG,
        "Wrong JSON filter '%s' format, 'function' must be of object type",
        audit_rule->get_rule_name().c_str());
    return nullptr;
  }

  if (!function_json.HasMember("name") || !function_json["name"].IsString()) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Wrong JSON filter '%s' format, "
                    "missing 'function' name or not a string",
                    audit_rule->get_rule_name().c_str());
    return nullptr;
  }

  const std::string func_name = function_json["name"].GetString();
  const auto func_type = get_filter_function_type(func_name);

  if (func_type == EventFilterFunctionType::Unknown) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Wrong JSON filter '%s' format, "
                    "unknown function name '%s'",
                    audit_rule->get_rule_name().c_str(), func_name.c_str());
    return nullptr;
  }

  FunctionArgsList args;

  if (function_json.HasMember("args") &&
      !parse_function_args_json(function_json["args"], args)) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Wrong JSON filter '%s' format, "
                    "wrong function args format provided",
                    audit_rule->get_rule_name().c_str());
    return nullptr;
  }

  if (!validate_filter_function_args(func_type, args, expected_return_type)) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Wrong JSON filter '%s' format, "
                    "invalid arguments for '%s' function",
                    audit_rule->get_rule_name().c_str(), func_name.c_str());
    return nullptr;
  }

  return get_event_filter_function(func_type, args);
}

bool AuditRuleParser::parse_function_args_json(
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

std::shared_ptr<EventFieldActionBase> AuditRuleParser::parse_action_json(
    const EventActionType action_type, const rapidjson::Value &action_json,
    AuditRule *audit_rule) noexcept {
  assert(action_json.IsObject());

  switch (action_type) {
    case EventActionType::Log: {
      std::shared_ptr<EventFieldConditionBase> log_cond;

      if (action_json.HasMember("log")) {
        log_cond = parse_condition(action_json["log"], audit_rule);

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
      auto block_cond = parse_condition(action_json["abort"], audit_rule);

      if (block_cond == nullptr) {
        LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                        "Wrong JSON filter '%s' format, "
                        "'abort' must be of bool or object type",
                        audit_rule->get_rule_name().c_str());
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

      auto print_cond = parse_condition(field_json["print"], audit_rule);

      if (print_cond == nullptr) {
        return nullptr;
      }

      if (!field_json["replace"].IsObject() ||
          !field_json["replace"].HasMember("function")) {
        return nullptr;
      }

      auto replacement_func =
          parse_function(field_json["replace"]["function"],
                         FunctionReturnType::String, audit_rule);

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

      auto activation_cond =
          parse_condition(action_json["filter"]["activate"], audit_rule);

      if (activation_cond == nullptr) {
        return nullptr;
      }

      auto replacement_rule =
          make_replacement_rule(action_json["filter"]["class"]);

      if (replacement_rule == nullptr) {
        return nullptr;
      }

      return std::make_shared<EventFieldActionReplaceFilter>(
          std::move(activation_cond), std::move(replacement_rule));
    }
    case EventActionType::PrintQueryAttrs: {
      /*
       * "print" : {
       *   "query_attributes": {
       *     "tag": "query_attributes",
       *     "element": [
       *       { "name": "attr1" },
       *       { "name": "attr2" },
       *       { "name": "attr3" }
       *     ]
       *   }
       * }
       */
      if (!action_json["print"].IsObject() ||
          !action_json["print"].HasMember("query_attributes") ||
          !action_json["print"]["query_attributes"].IsObject()) {
        return nullptr;
      }

      const auto &query_attrs_json = action_json["print"]["query_attributes"];

      // Check required fields and their type
      if (!query_attrs_json.HasMember("tag") ||
          !query_attrs_json.HasMember("element") ||
          !query_attrs_json["tag"].IsString() ||
          !query_attrs_json["element"].IsArray()) {
        return nullptr;
      }

      std::string tag_name = query_attrs_json["tag"].GetString();

      if (tag_name.empty()) {
        return nullptr;
      }

      event_field_action::QueryAttrsList attrs_list;

      for (auto it = query_attrs_json["element"].Begin();
           it != query_attrs_json["element"].End(); ++it) {
        if (!it->IsObject()) {
          return nullptr;
        }

        auto attr_info = it->GetObject();

        if (!attr_info.HasMember("name") || !attr_info["name"].IsString()) {
          return nullptr;
        }

        attrs_list.push_back(attr_info["name"].GetString());
      }

      if (attrs_list.empty()) {
        return nullptr;
      }

      return std::make_shared<EventFieldActionPrintQueryAttrs>(
          std::move(tag_name), std::move(attrs_list));
    }
    case EventActionType::PrintServiceComp: {
      /*
       * "print" : {
       *   "service": {
       *     "tag": "query_statistics",
       *     "element": [
       *       { "name": "query_time",     "type": "double" },
       *       { "name": "bytes_sent",     "type": "longlong" },
       *       { "name": "bytes_received", "type": "longlong" },
       *       { "name": "rows_sent",      "type": "longlong" },
       *       { "name": "rows_examined",  "type": "longlong" }
       *     ]
       *   }
       * }
       */
      if (!action_json["print"].IsObject() ||
          !action_json["print"].HasMember("service") ||
          !action_json["print"]["service"].IsObject()) {
        return nullptr;
      }

      const auto &service_json = action_json["print"]["service"];

      if (!service_json.HasMember("tag") ||
          !service_json.HasMember("element") ||
          !service_json["tag"].IsString() ||
          !service_json["element"].IsArray()) {
        return nullptr;
      }

      std::string tag_name = service_json["tag"].GetString();

      if (tag_name.empty() || service_json["element"].Empty()) {
        return nullptr;
      }

      PrintServiceElementsList elements;

      for (auto it = service_json["element"].Begin();
           it != service_json["element"].End(); ++it) {
        if (!it->IsObject()) {
          return nullptr;
        }

        auto element_info = it->GetObject();

        if (!element_info.HasMember("name") ||
            !element_info["name"].IsString() ||
            !element_info.HasMember("type") ||
            !element_info["type"].IsString()) {
          return nullptr;
        }

        auto element_type =
            EventFieldActionPrintServiceComp::string_to_element_type(
                element_info["type"].GetString());
        auto element_name =
            EventFieldActionPrintServiceComp::string_to_element_name(
                element_info["name"].GetString());

        if (element_type == ServiceCompElementType::Unknown ||
            element_name.empty()) {
          return nullptr;
        }

        elements.emplace_back(element_type, element_name);
      }

      return std::make_shared<EventFieldActionPrintServiceComp>(
          std::move(tag_name), std::move(elements));
    }
    default:
      assert(false);
  }

  return nullptr;
}

std::shared_ptr<AuditRule> AuditRuleParser::make_replacement_rule(
    const rapidjson::Value &rule_json) noexcept {
  rapidjson::Document d;
  d.SetObject();
  d.AddMember("filter", rapidjson::Value{rapidjson::kObjectType},
              d.GetAllocator());
  d["filter"].AddMember("class", rapidjson::Value{rapidjson::kObjectType},
                        d.GetAllocator());
  d["filter"]["class"].CopyFrom(rule_json, d.GetAllocator());

  auto rule = std::make_shared<AuditRule>();

  if (AuditRuleParser::parse(d, rule.get())) {
    return rule;
  }

  return nullptr;
}

}  // namespace audit_log_filter
