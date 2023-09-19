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

#include "components/audit_log_filter/audit_rule.h"

#include <algorithm>
#include <sstream>
#include <string>

namespace audit_log_filter {

AuditRule::AuditRule()
    : m_filter_id{0},
      m_rule_name{},
      m_should_log_unmatched{false},
      m_replacement_rule{nullptr} {}

AuditRule::AuditRule(const char *rule_name) : AuditRule{0, rule_name} {}

AuditRule::AuditRule(uint64_t filter_id, const char *rule_name)
    : m_filter_id{filter_id},
      m_rule_name{rule_name},
      m_should_log_unmatched{false},
      m_replacement_rule{nullptr} {}

AuditRule::AuditRule(AuditRule &&other) noexcept
    : m_filter_id{other.m_filter_id},
      m_rule_name{std::move(other.m_rule_name)},
      m_should_log_unmatched{other.m_should_log_unmatched},
      m_matched_event_to_action_map{
          std::move(other.m_matched_event_to_action_map)},
      m_replacement_rule{other.m_replacement_rule} {
  other.m_replacement_rule = nullptr;
}

uint64_t AuditRule::get_filter_id() const noexcept { return m_filter_id; }

std::string AuditRule::get_rule_name() const noexcept { return m_rule_name; }

void AuditRule::set_replacement_rule(AuditRule *rule) noexcept {
  m_replacement_rule = rule;
}

void AuditRule::clear_replacement_rule() noexcept {
  m_replacement_rule = nullptr;
}

bool AuditRule::should_log_unmatched() const noexcept {
  return m_should_log_unmatched;
}

void AuditRule::set_should_log_unmatched(const bool should_log) noexcept {
  m_should_log_unmatched = should_log;
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

}  // namespace audit_log_filter
