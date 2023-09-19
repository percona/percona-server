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

#include "components/audit_log_filter/event_field_action/replace_filter.h"

#include "components/audit_log_filter/audit_rule.h"
#include "components/audit_log_filter/event_field_condition/base.h"

#include <memory>

namespace audit_log_filter::event_field_action {

EventFieldActionReplaceFilter::EventFieldActionReplaceFilter(
    std::string replacement_filter_ref)
    : m_activation_cond{nullptr},
      m_replacement_rule{nullptr},
      m_replacement_filter_ref{std::move(replacement_filter_ref)} {}

EventFieldActionReplaceFilter::EventFieldActionReplaceFilter(
    std::shared_ptr<event_field_condition::EventFieldConditionBase>
        activation_cond,
    std::shared_ptr<AuditRule> replacement_rule)
    : m_activation_cond{std::move(activation_cond)},
      m_replacement_rule(std::move(replacement_rule)) {}

EventActionType EventFieldActionReplaceFilter::get_action_type()
    const noexcept {
  return EventActionType::ReplaceFilter;
}

bool EventFieldActionReplaceFilter::apply(
    const AuditRecordFieldsList &fields,
    AuditRecordVariant &audit_record [[maybe_unused]],
    AuditRule *audit_rule) const noexcept {
  if (!m_replacement_filter_ref.empty()) {
    // This is a temporary filter. Switch filtering rule back to original one
    // pointed by the ref.
    audit_rule->clear_replacement_rule();
    return true;
  }

  if (m_activation_cond->check_applies(fields)) {
    audit_rule->set_replacement_rule(m_replacement_rule.get());
  }

  return true;
}

}  // namespace audit_log_filter::event_field_action
