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

#ifndef AUDIT_LOG_FILTER_EVENT_FIELD_ACTION_REPLACE_FILTER_H_INCLUDED
#define AUDIT_LOG_FILTER_EVENT_FIELD_ACTION_REPLACE_FILTER_H_INCLUDED

#include "plugin/audit_log_filter/event_field_action/base.h"

namespace audit_log_filter {

class AuditRule;

namespace event_field_condition {
class EventFieldConditionBase;
}

namespace event_field_action {

class EventFieldActionReplaceFilter : public EventFieldActionBase {
 public:
  explicit EventFieldActionReplaceFilter(std::string replacement_filter_ref);
  EventFieldActionReplaceFilter(
      std::shared_ptr<event_field_condition::EventFieldConditionBase>
          activation_cond,
      std::shared_ptr<AuditRule> replacement_rule);

  /**
   * @brief Get action type.
   *
   * @return Action type, one of @ref EventActionType
   */
  [[nodiscard]] EventActionType get_action_type() const noexcept override;

  /**
   * @brief Apply action to audit event record.
   *
   * @param fields Audit event field list
   * @param audit_record Audit record
   * @param audit_rule Effective audit rule
   * @return true in case action applied successfully, false otherwise
   */
  [[nodiscard]] bool apply(const AuditRecordFieldsList &fields,
                           AuditRecordVariant &audit_record,
                           AuditRule *audit_rule) const noexcept override;

 private:
  std::shared_ptr<event_field_condition::EventFieldConditionBase>
      m_activation_cond;
  std::shared_ptr<AuditRule> m_replacement_rule;
  std::string m_replacement_filter_ref;
};

}  // namespace event_field_action
}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_EVENT_FIELD_ACTION_REPLACE_FILTER_H_INCLUDED
