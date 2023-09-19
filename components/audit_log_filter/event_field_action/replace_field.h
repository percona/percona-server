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

#ifndef AUDIT_LOG_FILTER_EVENT_FIELD_ACTION_REPLACE_FIELD_H_INCLUDED
#define AUDIT_LOG_FILTER_EVENT_FIELD_ACTION_REPLACE_FIELD_H_INCLUDED

#include "components/audit_log_filter/event_field_action/base.h"
#include "components/audit_log_filter/event_filter_function/base.h"

#include <memory>
#include <string>

namespace audit_log_filter {

namespace event_field_condition {
class EventFieldConditionBase;
}

namespace event_field_action {

class EventFieldActionReplaceField : public EventFieldActionBase {
 public:
  EventFieldActionReplaceField(
      std::string field_name,
      std::shared_ptr<event_field_condition::EventFieldConditionBase>
          print_cond,
      std::unique_ptr<event_filter_function::EventFilterFunctionBase>
          replacement_func);

  static bool validate_field_name(const std::string &field_name) noexcept;

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
  bool apply(const AuditRecordFieldsList &fields,
             AuditRecordVariant &audit_record,
             AuditRule *audit_rule) const noexcept override;

 private:
  std::string m_field_name;
  std::shared_ptr<event_field_condition::EventFieldConditionBase> m_print_cond;
  std::unique_ptr<event_filter_function::EventFilterFunctionBase>
      m_replacement_func;
};

}  // namespace event_field_action
}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_EVENT_FIELD_ACTION_REPLACE_FIELD_H_INCLUDED
