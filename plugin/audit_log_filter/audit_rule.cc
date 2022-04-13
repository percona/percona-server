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

#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

namespace audit_log_filter {

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
  auto it = m_class_to_action_map.find(event_class_name);

  if (it == m_class_to_action_map.end()) {
    return AuditAction::None;
  }

  return it->second;
}

AuditAction AuditRule::get_event_subclass_action(
    const std::string_view event_subclass_name) const noexcept {
  assert(m_json_rule_parsed);
  auto it = m_subclass_to_action_map.find(event_subclass_name);

  if (it == m_subclass_to_action_map.end()) {
    return AuditAction::None;
  }

  return it->second;
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

      if (!parse_event_subclass(event_class_obj["event"])) {
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
    if (!parse_event_subclass_obj(event_subclass_obj)) {
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

      if (!parse_event_subclass_obj(*it)) {
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
    const rapidjson::Value &event_subclass_obj) noexcept {
  assert(event_subclass_obj.IsObject());

  if (!event_subclass_obj.HasMember("name")) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Wrong JSON filter '%s' format, "
                    "no name provided for event subclass",
                    m_rule_name.c_str());
    return false;
  }

  AuditAction action = AuditAction::Log;
  if (event_subclass_obj.HasMember("log")) {
    if (event_subclass_obj["log"].IsBool()) {
      if (!event_subclass_obj["log"].GetBool()) {
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

  if (event_subclass_obj["name"].IsString()) {
    m_subclass_to_action_map.insert(
        {event_subclass_obj["name"].GetString(), action});
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

      m_subclass_to_action_map.insert({it->GetString(), action});
    }
  } else {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Wrong JSON filter '%s' format, event subclass name type "
                    "must be either string or an array of strings",
                    m_rule_name.c_str());
    return false;
  }

  return true;
}

}  // namespace audit_log_filter
