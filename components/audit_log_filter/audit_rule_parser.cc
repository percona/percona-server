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

#include "components/audit_log_filter/audit_rule_parser.h"
#include "components/audit_log_filter/audit_error_log.h"
#include "components/audit_log_filter/sys_vars.h"

#include "components/audit_log_filter/event_field_action/block.h"
#include "components/audit_log_filter/event_field_action/log.h"
#include "components/audit_log_filter/event_field_action/print_query_attrs.h"
#include "components/audit_log_filter/event_field_action/print_service_comp.h"
#include "components/audit_log_filter/event_field_action/replace_filter.h"
#include "components/audit_log_filter/event_field_condition/and.h"
#include "components/audit_log_filter/event_field_condition/bool.h"
#include "components/audit_log_filter/event_field_condition/field.h"
#include "components/audit_log_filter/event_field_condition/function.h"
#include "components/audit_log_filter/event_field_condition/not.h"
#include "components/audit_log_filter/event_field_condition/or.h"
#include "components/audit_log_filter/event_field_condition/variable.h"

#include <limits>
#include <memory>
#include <set>
#include <string_view>

namespace audit_log_filter {
namespace {

std::string find_unknown_key(const rapidjson::Value &json_obj,
                             const std::set<std::string_view> &allowed_keys) {
  for (auto it = json_obj.MemberBegin(); it != json_obj.MemberEnd(); ++it) {
    if (it->name.IsString()) {
      std::string_view key{it->name.GetString(), it->name.GetStringLength()};
      if (allowed_keys.count(key) == 0) {
        return std::string{key};
      }
    }
  }
  return {};
}

}  // namespace

bool AuditRuleParser::parse(const char *rule_str, AuditRule *audit_rule,
                            bool skip_disabled_events) noexcept {
  rapidjson::Document json_doc;
  json_doc.Parse(rule_str);

  return parse(json_doc, audit_rule, skip_disabled_events);
}

bool AuditRuleParser::parse(rapidjson::Document &json_doc,
                            AuditRule *audit_rule,
                            bool skip_disabled_events) noexcept {
  // Do basic check of rule structure
  if (json_doc.HasParseError()) {
    audit_rule->set_parse_error("JSON parse error");
    return false;
  }

  // The root of the JSON rule must be an object
  if (!json_doc.IsObject()) {
    audit_rule->set_parse_error("root element must be a JSON object");
    return false;
  }

  // The basic JSON rule format must be like the following: '{"filter": {}}'
  if (!json_doc.HasMember("filter") || !json_doc["filter"].IsObject()) {
    audit_rule->set_parse_error("missing or invalid 'filter' object");
    return false;
  }

  if (!json_doc["filter"].ObjectEmpty()) {
    static const std::set<std::string_view> allowed_filter_keys{"id", "log",
                                                                "class"};
    auto unknown = find_unknown_key(json_doc["filter"], allowed_filter_keys);
    if (!unknown.empty()) {
      LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_UNEXPECTED_KEY,
                      audit_rule->get_rule_name().c_str(), unknown.c_str());
      audit_rule->set_parse_error("unexpected key '" + unknown +
                                  "' in 'filter' definition");
      return false;
    }
  }

  if (!parse_default_log_action_json(json_doc, audit_rule)) {
    return false;
  }

  if (!parse_event_class_json(json_doc, audit_rule, skip_disabled_events)) {
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

  if (json_doc["filter"].HasMember("log")) {
    if (!json_doc["filter"]["log"].IsBool()) {
      LogComponentErr(ERROR_LEVEL,
                      ER_AUDIT_PARSE_DEFAULT_LOG_ACTION_BAD_LOG_TYPE,
                      audit_rule->get_rule_name().c_str());
      audit_rule->set_parse_error("the 'log' member must be of type bool");
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
    const rapidjson::Document &json_doc, AuditRule *audit_rule,
    bool skip_disabled_events) noexcept {
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
    return parse_event_class_obj_json(ev_class_json, audit_rule,
                                      skip_disabled_events);
  } else if (ev_class_json.IsArray()) {
    if (ev_class_json.Empty()) {
      LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_EMPTY_ARRAY,
                      audit_rule->get_rule_name().c_str());
      audit_rule->set_parse_error("'class' must not be an empty array");
      return false;
    }

    for (const auto *it = ev_class_json.Begin(); it != ev_class_json.End();
         ++it) {
      if (!it->IsObject()) {
        LogComponentErr(ERROR_LEVEL,
                        ER_AUDIT_PARSE_EVENT_CLASS_BAD_CLASS_LIST_TYPE,
                        audit_rule->get_rule_name().c_str());
        audit_rule->set_parse_error(
            "'class' array element must be of object type");
        return false;
      }

      if (!parse_event_class_obj_json(*it, audit_rule, skip_disabled_events)) {
        return false;
      }
    }
  } else {
    LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_EVENT_CLASS_BAD_CLASS_TYPE,
                    audit_rule->get_rule_name().c_str());
    audit_rule->set_parse_error("'class' must be an object or an array");
    return false;
  }

  return true;
}

bool AuditRuleParser::parse_event_class_obj_json(
    const rapidjson::Value &event_class_json, AuditRule *audit_rule,
    bool skip_disabled_events) noexcept {
  assert(event_class_json.IsObject());

  if (!event_class_json.HasMember("name")) {
    LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_EVENT_CLASS_NO_CLASS_NAME,
                    audit_rule->get_rule_name().c_str());
    audit_rule->set_parse_error("no name provided for event class");
    return false;
  }

  {
    static const std::set<std::string_view> allowed_class_keys{
        "name", "log", "event", "print", "abort"};
    auto unknown = find_unknown_key(event_class_json, allowed_class_keys);
    if (!unknown.empty()) {
      LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_UNEXPECTED_KEY,
                      audit_rule->get_rule_name().c_str(), unknown.c_str());
      audit_rule->set_parse_error("unexpected key '" + unknown +
                                  "' in 'class' definition");
      return false;
    }
  }

  if (event_class_json.HasMember("abort")) {
    LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_EVENT_CLASS_BAD_ABORT_DEF,
                    audit_rule->get_rule_name().c_str());
    audit_rule->set_parse_error(
        "'abort' condition should be set for subclass only");
    return false;
  }

  bool should_log = true;
  if (event_class_json.HasMember("log")) {
    if (event_class_json["log"].IsBool()) {
      should_log = event_class_json["log"].GetBool();
    } else {
      LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_EVENT_CLASS_BAD_LOG_TYPE,
                      audit_rule->get_rule_name().c_str());
      audit_rule->set_parse_error("'log' must be of bool type");
      return false;
    }
  }

  std::shared_ptr<EventFieldActionBase> replace_field;
  const bool has_print = event_class_json.HasMember("print");

  if (event_class_json["name"].IsString()) {
    const std::string event_class_name = event_class_json["name"].GetString();

    if (!is_valid_event_class_name(event_class_name)) {
      LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_INVALID_EVENT_CLASS_NAME,
                      audit_rule->get_rule_name().c_str(),
                      event_class_name.c_str());
      audit_rule->set_parse_error("unknown event class name '" +
                                  event_class_name + "'");
      return false;
    }

    if (SysVars::get_event_mode_type() == AuditLogEventModeType::Reduced &&
        !is_event_class_allowed_in_reduced_mode(event_class_name)) {
      if (skip_disabled_events) {
        LogComponentErr(WARNING_LEVEL, ER_AUDIT_PARSE_SKIP_DISABLED_EVENT_CLASS,
                        event_class_name.c_str(),
                        audit_rule->get_rule_name().c_str());
        return true;
      }
      audit_rule->set_parse_error(
          "event class '" + event_class_name +
          "' is disabled in audit_log_filter.event_mode=REDUCED");
      return false;
    }

    if (has_print) {
      replace_field =
          parse_action_json(EventActionType::ReplaceField, event_class_json,
                            event_class_name, audit_rule);

      if (replace_field == nullptr) {
        LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_EVENT_CLASS_BAD_PRINT_DEF,
                        audit_rule->get_rule_name().c_str());
        audit_rule->set_parse_error("failed to parse 'print' replacement rule");
        return false;
      }
    }

    if (event_class_json.HasMember("event")) {
      if (replace_field != nullptr) {
        LogComponentErr(ERROR_LEVEL,
                        ER_AUDIT_PARSE_EVENT_CLASS_UNEXPECTED_PRINT_DEF,
                        audit_rule->get_rule_name().c_str());
        audit_rule->set_parse_error(
            "replacement rule not expected for event class");
        return false;
      }

      should_log = false;

      if (!parse_event_subclass_json(event_class_name,
                                     event_class_json["event"], audit_rule,
                                     skip_disabled_events)) {
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
    if (event_class_json["name"].Empty()) {
      LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_EMPTY_ARRAY,
                      audit_rule->get_rule_name().c_str());
      audit_rule->set_parse_error("class 'name' must not be an empty array");
      return false;
    }

    // There may be no event subclass specified in case event class name is
    // defined as an array { "name": [ "class_name_1", "class_name_2" ] }
    if (event_class_json.HasMember("event")) {
      LogComponentErr(ERROR_LEVEL,
                      ER_AUDIT_PARSE_EVENT_CLASS_UNEXPECTED_EVENT_DEF,
                      audit_rule->get_rule_name().c_str());
      audit_rule->set_parse_error(
          "there must be no 'event' in case class names provided as an array "
          "of strings");
      return false;
    }

    std::shared_ptr<EventFieldActionBase> log_action =
        std::make_shared<EventFieldActionLog>(
            std::make_unique<EventFieldConditionBool>(should_log));

    for (const auto *it = event_class_json["name"].Begin();
         it != event_class_json["name"].End(); ++it) {
      if (!it->IsString()) {
        LogComponentErr(ERROR_LEVEL,
                        ER_AUDIT_PARSE_EVENT_CLASS_BAD_EVENT_NAME_FOR_LIST,
                        audit_rule->get_rule_name().c_str());
        audit_rule->set_parse_error(
            "event class name within an array should be of a string type");
        return false;
      }

      const std::string event_class_name = it->GetString();

      if (!is_valid_event_class_name(event_class_name)) {
        LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_INVALID_EVENT_CLASS_NAME,
                        audit_rule->get_rule_name().c_str(),
                        event_class_name.c_str());
        audit_rule->set_parse_error("unknown event class name '" +
                                    event_class_name + "'");
        return false;
      }

      if (SysVars::get_event_mode_type() == AuditLogEventModeType::Reduced &&
          !is_event_class_allowed_in_reduced_mode(event_class_name)) {
        if (skip_disabled_events) {
          LogComponentErr(
              WARNING_LEVEL, ER_AUDIT_PARSE_SKIP_DISABLED_EVENT_CLASS,
              event_class_name.c_str(), audit_rule->get_rule_name().c_str());
          continue;
        }
        audit_rule->set_parse_error(
            "event class '" + event_class_name +
            "' is disabled in audit_log_filter.event_mode=REDUCED");
        return false;
      }

      audit_rule->add_action_for_event(log_action, event_class_name);

      if (has_print) {
        auto replace_field_for_class =
            parse_action_json(EventActionType::ReplaceField, event_class_json,
                              event_class_name, audit_rule);

        if (replace_field_for_class == nullptr) {
          LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_EVENT_CLASS_BAD_PRINT_DEF,
                          audit_rule->get_rule_name().c_str());
          audit_rule->set_parse_error(
              "failed to parse 'print' replacement rule");
          return false;
        }

        audit_rule->add_action_for_event(replace_field_for_class,
                                         event_class_name);
      }
    }
  } else {
    LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_EVENT_CLASS_BAD_EVENT_NAME_TYPE,
                    audit_rule->get_rule_name().c_str());
    audit_rule->set_parse_error(
        "event class name type must be either string or an array of strings");
    return false;
  }

  return true;
}

bool AuditRuleParser::parse_event_subclass_json(
    const std::string &class_name, const rapidjson::Value &event_subclass_json,
    AuditRule *audit_rule, bool skip_disabled_events) noexcept {
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
                                       audit_rule, skip_disabled_events)) {
      return false;
    }
  } else if (event_subclass_json.IsArray()) {
    if (event_subclass_json.Empty()) {
      LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_EMPTY_ARRAY,
                      audit_rule->get_rule_name().c_str());
      audit_rule->set_parse_error("'event' must not be an empty array");
      return false;
    }

    for (const auto *it = event_subclass_json.Begin();
         it != event_subclass_json.End(); ++it) {
      if (!it->IsObject()) {
        LogComponentErr(ERROR_LEVEL,
                        ER_AUDIT_PARSE_EVENT_SUBCLASS_BAD_SUBCLASS_LIST_TYPE,
                        audit_rule->get_rule_name().c_str());
        audit_rule->set_parse_error(
            "'event' array element must be of object type");
        return false;
      }

      if (!parse_event_subclass_obj_json(class_name, *it, audit_rule,
                                         skip_disabled_events)) {
        return false;
      }
    }
  } else {
    LogComponentErr(ERROR_LEVEL,
                    ER_AUDIT_PARSE_EVENT_SUBCLASS_BAD_SUBCLASS_TYPE,
                    audit_rule->get_rule_name().c_str());
    audit_rule->set_parse_error(
        "type of 'event' must be either an object or an array of objects");
    return false;
  }

  return true;
}

bool AuditRuleParser::parse_event_subclass_obj_json(
    const std::string &class_name, const rapidjson::Value &event_subclass_json,
    AuditRule *audit_rule, bool skip_disabled_events) noexcept {
  assert(event_subclass_json.IsObject());

  if (!event_subclass_json.HasMember("name")) {
    LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_NO_SUBCLASS_NAME,
                    audit_rule->get_rule_name().c_str());
    audit_rule->set_parse_error("no name provided for event subclass");
    return false;
  }

  {
    static const std::set<std::string_view> allowed_event_keys{
        "name", "log", "abort", "print", "filter"};
    auto unknown = find_unknown_key(event_subclass_json, allowed_event_keys);
    if (!unknown.empty()) {
      LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_UNEXPECTED_KEY,
                      audit_rule->get_rule_name().c_str(), unknown.c_str());
      audit_rule->set_parse_error("unexpected key '" + unknown +
                                  "' in 'event' definition");
      return false;
    }
  }

  std::vector<std::string> subclass_names;

  if (event_subclass_json["name"].IsString()) {
    subclass_names.emplace_back(event_subclass_json["name"].GetString());
  } else if (event_subclass_json["name"].IsArray()) {
    if (event_subclass_json["name"].Empty()) {
      LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_EMPTY_ARRAY,
                      audit_rule->get_rule_name().c_str());
      audit_rule->set_parse_error(
          "event subclass 'name' must not be an empty array");
      return false;
    }

    for (const auto *it = event_subclass_json["name"].Begin();
         it != event_subclass_json["name"].End(); ++it) {
      if (!it->IsString()) {
        LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_BAD_SUBCLASS_NAME_TYPE,
                        audit_rule->get_rule_name().c_str());
        audit_rule->set_parse_error(
            "event subclass name within an array should be of a string type");
        return false;
      }

      subclass_names.emplace_back(it->GetString());
    }
  } else {
    LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_BAD_SUBCLASS_NAME_LIST_TYPE,
                    audit_rule->get_rule_name().c_str());
    audit_rule->set_parse_error(
        "event subclass name type must be either string or an array of "
        "strings");
    return false;
  }

  for (auto it = subclass_names.begin(); it != subclass_names.end();) {
    const auto &name = *it;
    if (!is_valid_event_subclass_name(class_name, name)) {
      LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_INVALID_EVENT_SUBCLASS_NAME,
                      audit_rule->get_rule_name().c_str(), name.c_str(),
                      class_name.c_str());
      audit_rule->set_parse_error("unknown event subclass name '" + name +
                                  "' for class '" + class_name + "'");
      return false;
    }

    if (SysVars::get_event_mode_type() == AuditLogEventModeType::Reduced &&
        !is_event_subclass_allowed_in_reduced_mode(class_name, name)) {
      if (skip_disabled_events) {
        LogComponentErr(WARNING_LEVEL,
                        ER_AUDIT_PARSE_SKIP_DISABLED_EVENT_SUBCLASS,
                        class_name.c_str(), name.c_str(),
                        audit_rule->get_rule_name().c_str());
        it = subclass_names.erase(it);
        continue;
      }
      audit_rule->set_parse_error(
          "event '" + class_name + "/" + name +
          "' is disabled in audit_log_filter.event_mode=REDUCED");
      return false;
    }

    ++it;
  }

  const auto has_log_tag = event_subclass_json.HasMember("log");
  const auto has_abort_tag = event_subclass_json.HasMember("abort");

  if (has_log_tag && has_abort_tag) {
    LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_CONDITION_DUPLICATED,
                    audit_rule->get_rule_name().c_str());
    audit_rule->set_parse_error(
        "there must be only one condition provided, 'log' or 'abort'");
    return false;
  }

  const EventActionType log_action_type =
      has_abort_tag ? EventActionType::Block : EventActionType::Log;

  std::shared_ptr<EventFieldActionBase> log_action = parse_action_json(
      log_action_type, event_subclass_json, class_name, audit_rule);

  if (log_action == nullptr) {
    return false;
  }

  std::vector<std::shared_ptr<EventFieldActionBase>> actions_list;
  actions_list.push_back(log_action);

  if (event_subclass_json.HasMember("print")) {
    const auto &print_json = event_subclass_json["print"];
    if (!print_json.IsObject()) {
      LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_EVENT_CLASS_BAD_PRINT_DEF,
                      audit_rule->get_rule_name().c_str());
      audit_rule->set_parse_error("failed to parse 'print' replacement rule");
      return false;
    }
    // There may be a few actions modifying record content defined within
    // "print" tag
    for (auto it = print_json.MemberBegin(); it != print_json.MemberEnd();
         ++it) {
      const auto action_type =
          event_field_action::get_event_action_type(it->name.GetString());

      if (action_type == EventActionType::Unknown) {
        LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_UNKNOWN_TAG,
                        audit_rule->get_rule_name().c_str(),
                        it->name.GetString());
        audit_rule->set_parse_error(std::string("unknown tag '") +
                                    it->name.GetString() + "'");
        return false;
      }

      auto action = parse_action_json(action_type, event_subclass_json,
                                      class_name, audit_rule);

      if (action == nullptr) {
        LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_BAD_ACTION_FORMAT,
                        audit_rule->get_rule_name().c_str(),
                        it->name.GetString());
        audit_rule->set_parse_error(std::string("bad format for '") +
                                    it->name.GetString() + "' action");
        return false;
      }

      actions_list.push_back(action);
    }
  }

  std::shared_ptr<EventFieldActionBase> replace_filter_action;

  if (event_subclass_json.HasMember("filter")) {
    replace_filter_action =
        parse_action_json(EventActionType::ReplaceFilter, event_subclass_json,
                          class_name, audit_rule);

    if (replace_filter_action == nullptr) {
      LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_BAD_REPLACEMENT_RULE,
                      audit_rule->get_rule_name().c_str());
      audit_rule->set_parse_error("failed to parse 'filter' replacement rule");
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
    const rapidjson::Value &condition_json, const std::string &class_name,
    AuditRule *audit_rule) noexcept {
  auto cond_type = get_condition_type(condition_json, audit_rule);

  if (cond_type == EventFieldConditionType::Unknown) {
    return nullptr;
  }

  return parse_condition_json(condition_json, cond_type, class_name,
                              audit_rule);
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
  if (json.IsBool()) {
    return EventFieldConditionType::Bool;
  }

  if (!json.IsObject()) {
    LogComponentErr(ERROR_LEVEL,
                    ER_AUDIT_PARSE_CONDITION_TYPE_UNEXPECTED_COND_TYPE,
                    audit_rule->get_rule_name().c_str());
    audit_rule->set_parse_error(
        "the 'log' field must be of bool or object type");
    return EventFieldConditionType::Unknown;
  }

  if (json.MemberCount() != 1) {
    LogComponentErr(ERROR_LEVEL,
                    ER_AUDIT_PARSE_CONDITION_TYPE_UNEXPECTED_COND_FORMAT,
                    audit_rule->get_rule_name().c_str());
    audit_rule->set_parse_error(
        "there must be only one condition specified for 'log' field");
    return EventFieldConditionType::Unknown;
  }

  const auto &condition = json.MemberBegin();

  if (!condition->name.IsString()) {
    LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_CONDITION_TYPE_BAD_COND_TYPE,
                    audit_rule->get_rule_name().c_str());
    audit_rule->set_parse_error(
        "the 'log' condition name must be of string type");
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

  LogComponentErr(ERROR_LEVEL,
                  ER_AUDIT_PARSE_CONDITION_TYPE_UNEXPECTED_COND_NAME,
                  audit_rule->get_rule_name().c_str(), condition_name.c_str());
  audit_rule->set_parse_error("unknown 'log' condition name '" +
                              condition_name + "'");

  return EventFieldConditionType::Unknown;
}

std::shared_ptr<EventFieldConditionBase> AuditRuleParser::parse_condition_json(
    const rapidjson::Value &condition_json,
    const EventFieldConditionType cond_type, const std::string &class_name,
    AuditRule *audit_rule) noexcept {
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
        LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_CONDITION_BAD_FIELD_TYPE,
                        audit_rule->get_rule_name().c_str());
        audit_rule->set_parse_error(
            "condition definition 'field' must be of object type");
        return nullptr;
      }

      if (!condition_json["field"].HasMember("name") ||
          !condition_json["field"].HasMember("value") ||
          !condition_json["field"]["name"].IsString()) {
        LogComponentErr(ERROR_LEVEL,
                        ER_AUDIT_PARSE_CONDITION_BAD_FIELD_NAME_AND_VALUE,
                        audit_rule->get_rule_name().c_str());
        audit_rule->set_parse_error(
            "event field definition 'field' must have field 'name' "
            "provided as a string and 'value' as a string or integer");
        return nullptr;
      }

      const auto &value_json = condition_json["field"]["value"];
      std::string field_name{condition_json["field"]["name"].GetString()};
      std::string field_value;

      if (value_json.IsString()) {
        field_value = value_json.GetString();
      } else if (value_json.IsInt64()) {
        field_value = std::to_string(value_json.GetInt64());
      } else if (value_json.IsUint64()) {
        field_value = std::to_string(value_json.GetUint64());
      } else {
        LogComponentErr(ERROR_LEVEL,
                        ER_AUDIT_PARSE_CONDITION_BAD_FIELD_NAME_AND_VALUE,
                        audit_rule->get_rule_name().c_str());
        audit_rule->set_parse_error(
            "event field definition 'field' 'value' must be "
            "a string or integer");
        return nullptr;
      }

      if (!class_name.empty() &&
          !is_valid_event_field_name(class_name, field_name)) {
        LogComponentErr(ERROR_LEVEL,
                        ER_AUDIT_PARSE_CONDITION_FIELD_NAME_NOT_FOUND,
                        audit_rule->get_rule_name().c_str(), field_name.c_str(),
                        class_name.c_str());
        audit_rule->set_parse_error("field name '" + field_name +
                                    "' is not valid for event class '" +
                                    class_name + "'");
        return nullptr;
      }

      if (!value_json.IsString() && !class_name.empty()) {
        auto field_type = get_event_field_value_type(class_name, field_name);

        if (field_type == EventFieldValueType::String) {
          LogComponentErr(ERROR_LEVEL,
                          ER_AUDIT_PARSE_CONDITION_BAD_FIELD_NAME_AND_VALUE,
                          audit_rule->get_rule_name().c_str());
          audit_rule->set_parse_error(
              "field '" + field_name +
              "' is not an integer field; 'value' must be a string");
          return nullptr;
        }

        if (field_type == EventFieldValueType::UnsignedInteger &&
            value_json.IsInt64() && value_json.GetInt64() < 0) {
          LogComponentErr(ERROR_LEVEL,
                          ER_AUDIT_PARSE_CONDITION_BAD_FIELD_NAME_AND_VALUE,
                          audit_rule->get_rule_name().c_str());
          audit_rule->set_parse_error(
              "field '" + field_name +
              "' is an unsigned integer field; 'value' must not be negative");
          return nullptr;
        }

        if (field_type == EventFieldValueType::SignedInteger &&
            value_json.IsUint64() &&
            value_json.GetUint64() >
                static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
          LogComponentErr(ERROR_LEVEL,
                          ER_AUDIT_PARSE_CONDITION_BAD_FIELD_NAME_AND_VALUE,
                          audit_rule->get_rule_name().c_str());
          audit_rule->set_parse_error(
              "field '" + field_name +
              "' is a signed integer field; 'value' must not exceed " +
              std::to_string(std::numeric_limits<int64_t>::max()));
          return nullptr;
        }
      }

      if (value_json.IsString() && !class_name.empty() &&
          field_name != CONNECTION_TYPE_FIELD_NAME) {
        auto field_type = get_event_field_value_type(class_name, field_name);

        if (field_type != EventFieldValueType::String) {
          bool valid = false;
          try {
            size_t pos = 0;
            if (field_type == EventFieldValueType::UnsignedInteger) {
              if (!field_value.empty() && field_value[0] == '-') {
                valid = false;
              } else {
                (void)std::stoull(field_value, &pos);
                valid = (pos == field_value.size());
              }
            } else if (field_type == EventFieldValueType::SignedInteger) {
              (void)std::stoll(field_value, &pos);
              valid = (pos == field_value.size());
            } else {
              assert(false);
            }
          } catch (...) {
            valid = false;
          }

          if (!valid) {
            LogComponentErr(ERROR_LEVEL,
                            ER_AUDIT_PARSE_CONDITION_BAD_FIELD_NAME_AND_VALUE,
                            audit_rule->get_rule_name().c_str());
            audit_rule->set_parse_error(
                "field '" + field_name +
                "' is an integer field; string value '" + field_value +
                "' is not a valid number");
            return nullptr;
          }
        }
      }

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

        if (!is_valid_connection_type_value(field_value)) {
          LogComponentErr(ERROR_LEVEL,
                          ER_AUDIT_PARSE_CONDITION_BAD_FIELD_NAME_AND_VALUE,
                          audit_rule->get_rule_name().c_str());
          audit_rule->set_parse_error(
              "invalid 'connection_type' value '" + field_value +
              "'; expected integer 0..5 or one of "
              "\"::undefined\", \"::tcp/ip\", \"::socket\", "
              "\"::named_pipe\", \"::ssl\", \"::shared_memory\"");
          return nullptr;
        }
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
        LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_CONDITION_BAD_AND_COND_TYPE,
                        audit_rule->get_rule_name().c_str());
        audit_rule->set_parse_error(
            "condition definition 'and' must be of array type");
        return nullptr;
      }

      std::vector<std::shared_ptr<EventFieldConditionBase>> sub_conditions;

      for (const auto *it = condition_json["and"].Begin();
           it != condition_json["and"].End(); ++it) {
        if (!it->IsObject()) {
          LogComponentErr(ERROR_LEVEL,
                          ER_AUDIT_PARSE_CONDITION_BAD_AND_COND_FORMAT,
                          audit_rule->get_rule_name().c_str());
          audit_rule->set_parse_error(
              "a member of 'and' condition must be of object type");
          return nullptr;
        }

        auto sub_cond_type = get_condition_type(*it, audit_rule);

        if (sub_cond_type == EventFieldConditionType::Unknown) {
          return nullptr;
        }

        auto condition =
            parse_condition_json(*it, sub_cond_type, class_name, audit_rule);

        if (condition == nullptr) {
          return nullptr;
        }

        sub_conditions.push_back(std::move(condition));
      }

      if (sub_conditions.size() < 2) {
        LogComponentErr(ERROR_LEVEL,
                        ER_AUDIT_PARSE_CONDITION_BAD_AND_COND_OPERANDS,
                        audit_rule->get_rule_name().c_str());
        audit_rule->set_parse_error(
            "there should be at least two fields provided for 'and' "
            "condition");
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
        LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_CONDITION_BAD_OR_COND_TYPE,
                        audit_rule->get_rule_name().c_str());
        audit_rule->set_parse_error(
            "condition definition 'or' must be of array type");
        return nullptr;
      }

      std::vector<std::shared_ptr<EventFieldConditionBase>> sub_conditions;

      for (auto it = condition_json["or"].Begin();
           it != condition_json["or"].End(); ++it) {
        if (!it->IsObject()) {
          LogComponentErr(ERROR_LEVEL,
                          ER_AUDIT_PARSE_CONDITION_BAD_OR_COND_FORMAT,
                          audit_rule->get_rule_name().c_str());
          audit_rule->set_parse_error(
              "a member of 'or' condition must be of object type");
          return nullptr;
        }

        auto sub_cond_type = get_condition_type(*it, audit_rule);

        if (sub_cond_type == EventFieldConditionType::Unknown) {
          return nullptr;
        }

        auto condition =
            parse_condition_json(*it, sub_cond_type, class_name, audit_rule);

        if (condition == nullptr) {
          return nullptr;
        }

        sub_conditions.push_back(std::move(condition));
      }

      if (sub_conditions.size() < 2) {
        LogComponentErr(ERROR_LEVEL,
                        ER_AUDIT_PARSE_CONDITION_BAD_OR_COND_OPERANDS,
                        audit_rule->get_rule_name().c_str());
        audit_rule->set_parse_error(
            "there should be at least two fields provided for 'or' "
            "condition");
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
        LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_CONDITION_BAD_NOT_COND_TYPE,
                        audit_rule->get_rule_name().c_str());
        audit_rule->set_parse_error(
            "condition definition 'not' must be of object type");
        return nullptr;
      }

      auto sub_cond_type =
          get_condition_type(condition_json["not"], audit_rule);

      if (sub_cond_type == EventFieldConditionType::Unknown) {
        return nullptr;
      }

      auto condition = parse_condition_json(
          condition_json["not"], sub_cond_type, class_name, audit_rule);

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
        LogComponentErr(ERROR_LEVEL,
                        ER_AUDIT_PARSE_CONDITION_BAD_VARIABLE_COND_TYPE,
                        audit_rule->get_rule_name().c_str());
        audit_rule->set_parse_error(
            "condition definition 'variable' must be of object type");
        return nullptr;
      }

      if (!condition_json["variable"].HasMember("name") ||
          !condition_json["variable"].HasMember("value") ||
          !condition_json["variable"]["name"].IsString() ||
          !condition_json["variable"]["value"].IsString()) {
        LogComponentErr(ERROR_LEVEL,
                        ER_AUDIT_PARSE_CONDITION_BAD_VARIABLE_NAME_AND_VALUE,
                        audit_rule->get_rule_name().c_str());
        audit_rule->set_parse_error(
            "event field definition 'variable' must have field 'name' and "
            "'value' provided as strings");
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
          parse_function(condition_json["function"], FunctionReturnType::Bool,
                         class_name, audit_rule);

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
    const std::string &class_name, AuditRule *audit_rule) noexcept {
  if (!function_json.IsObject()) {
    LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_FUNCTION_NOT_OBJECT,
                    audit_rule->get_rule_name().c_str());
    audit_rule->set_parse_error("'function' must be of object type");
    return nullptr;
  }

  if (!function_json.HasMember("name") || !function_json["name"].IsString()) {
    LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_FUNCTION_NO_FUNCTION_NAME,
                    audit_rule->get_rule_name().c_str());
    audit_rule->set_parse_error("missing 'function' name or not a string");
    return nullptr;
  }

  const std::string func_name = function_json["name"].GetString();
  const auto func_type = get_filter_function_type(func_name);

  if (func_type == EventFilterFunctionType::Unknown) {
    LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_FUNCTION_UNKNOWN_FUNCTION_NAME,
                    audit_rule->get_rule_name().c_str(), func_name.c_str());
    audit_rule->set_parse_error("unknown function name '" + func_name + "'");
    return nullptr;
  }

  FunctionArgsList args;

  if (function_json.HasMember("args") &&
      !parse_function_args_json(function_json["args"], args, class_name,
                                audit_rule)) {
    LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_FUNCTION_BAD_ARGS_FORMAT,
                    audit_rule->get_rule_name().c_str());
    audit_rule->set_parse_error("wrong function args format provided");
    return nullptr;
  }

  if (!validate_filter_function_args(func_type, args, expected_return_type)) {
    LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_FUNCTION_BAD_ARGS,
                    audit_rule->get_rule_name().c_str(), func_name.c_str());
    audit_rule->set_parse_error("invalid arguments for '" + func_name +
                                "' function");
    return nullptr;
  }

  return get_event_filter_function(func_type, args);
}

bool AuditRuleParser::parse_function_args_json(
    const rapidjson::Value &function_args_json, FunctionArgsList &args,
    const std::string &class_name, AuditRule *audit_rule) noexcept {
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

  for (const auto *it = function_args_json.Begin();
       it != function_args_json.End(); ++it) {
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

    const std::string arg_value = arg_value_json->value.GetString();

    if (arg_source_type == FunctionArgSourceType::Field &&
        !class_name.empty() &&
        !is_valid_event_field_name(class_name, arg_value)) {
      LogComponentErr(ERROR_LEVEL,
                      ER_AUDIT_PARSE_CONDITION_FIELD_NAME_NOT_FOUND,
                      audit_rule->get_rule_name().c_str(), arg_value.c_str(),
                      class_name.c_str());
      audit_rule->set_parse_error("field name '" + arg_value +
                                  "' is not valid for event class '" +
                                  class_name + "'");
      return false;
    }

    args.push_back({arg_type, arg_source_type, arg_value});
  }

  return true;
}

std::shared_ptr<EventFieldActionBase> AuditRuleParser::parse_action_json(
    const EventActionType action_type, const rapidjson::Value &action_json,
    const std::string &class_name, AuditRule *audit_rule) noexcept {
  assert(action_json.IsObject());

  switch (action_type) {
    case EventActionType::Log: {
      std::shared_ptr<EventFieldConditionBase> log_cond;

      if (action_json.HasMember("log")) {
        log_cond = parse_condition(action_json["log"], class_name, audit_rule);

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
      auto block_cond =
          parse_condition(action_json["abort"], class_name, audit_rule);

      if (block_cond == nullptr) {
        LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_ACTION_BAD_ABORT_TYPE,
                        audit_rule->get_rule_name().c_str());
        audit_rule->set_parse_error("'abort' must be of bool or object type");
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
        LogComponentErr(ERROR_LEVEL, ER_AUDIT_PARSE_ACTION_BAD_REPLACE,
                        replaced_field_name.c_str());
        audit_rule->set_parse_error("event field '" + replaced_field_name +
                                    "' cannot be replaced");
        return nullptr;
      }

      auto print_cond =
          parse_condition(field_json["print"], class_name, audit_rule);

      if (print_cond == nullptr) {
        return nullptr;
      }

      if (!field_json["replace"].IsObject() ||
          !field_json["replace"].HasMember("function")) {
        return nullptr;
      }

      auto replacement_func =
          parse_function(field_json["replace"]["function"],
                         FunctionReturnType::String, class_name, audit_rule);

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

      auto activation_cond = parse_condition(action_json["filter"]["activate"],
                                             class_name, audit_rule);

      if (activation_cond == nullptr) {
        return nullptr;
      }

      auto replacement_rule =
          make_replacement_rule(action_json["filter"]["class"], audit_rule);

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

      for (const auto *it = query_attrs_json["element"].Begin();
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

      for (const auto *it = service_json["element"].Begin();
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
    const rapidjson::Value &rule_json, AuditRule *audit_rule) noexcept {
  rapidjson::Document d;
  d.SetObject();
  d.AddMember("filter", rapidjson::Value{rapidjson::kObjectType},
              d.GetAllocator());
  d["filter"].AddMember("class", rapidjson::Value{rapidjson::kObjectType},
                        d.GetAllocator());
  d["filter"]["class"].CopyFrom(rule_json, d.GetAllocator());

  const auto rule_name =
      audit_rule != nullptr ? audit_rule->get_rule_name() : std::string{};
  auto rule = std::make_shared<AuditRule>(rule_name.c_str());

  if (AuditRuleParser::parse(d, rule.get(), false)) {
    return rule;
  }

  if (audit_rule != nullptr && !rule->get_parse_error().empty()) {
    audit_rule->set_parse_error(rule->get_parse_error());
  }

  return nullptr;
}

}  // namespace audit_log_filter
