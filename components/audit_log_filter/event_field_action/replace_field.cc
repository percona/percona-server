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

#include "components/audit_log_filter/event_field_action/replace_field.h"

#include "components/audit_log_filter/event_field_condition/base.h"

#include <mysql/components/services/log_builtins.h>
#include <mysqld_error.h>

#include <utility>
#include <variant>

namespace audit_log_filter::event_field_action {

EventFieldActionReplaceField::EventFieldActionReplaceField(
    std::string field_name,
    std::shared_ptr<event_field_condition::EventFieldConditionBase> print_cond,
    std::unique_ptr<event_filter_function::EventFilterFunctionBase>
        replacement_func)
    : m_field_name{std::move(field_name)},
      m_print_cond{std::move(print_cond)},
      m_replacement_func{std::move(replacement_func)} {}

bool EventFieldActionReplaceField::validate_field_name(
    const std::string &field_name) noexcept {
  /*
   * Applicable to the following fields only:
   *    query -> query.str
   *    parse -> query.str
   */
  return field_name == "query.str";
}

EventActionType EventFieldActionReplaceField::get_action_type() const noexcept {
  return EventActionType::ReplaceField;
}

bool EventFieldActionReplaceField::apply(const AuditRecordFieldsList &fields,
                                         AuditRecordVariant &audit_record,
                                         AuditRule *audit_rule
                                         [[maybe_unused]]) const noexcept {
  if (m_print_cond->check_applies(fields)) {
    // print unchanged
    return false;
  }

  std::string new_value;

  if (!m_replacement_func->exec(fields, new_value)) {
    return false;
  }

  if (new_value.empty()) {
    new_value = "...";
  }

  if (std::holds_alternative<AuditRecordQuery>(audit_record)) {
    auto *rec = std::get_if<AuditRecordQuery>(&audit_record);
    if (rec != nullptr) {
      rec->extended_info.digest = std::move(new_value);
    }
  } else if (std::holds_alternative<AuditRecordParse>(audit_record)) {
    auto *rec = std::get_if<AuditRecordParse>(&audit_record);
    if (rec != nullptr) {
      rec->extended_info.digest = std::move(new_value);
    }
  } else {
    LogComponentErr(ERROR_LEVEL, ER_AUDIT_REPLACE_FIELD_UNEXPECTED_EVENT_TYPE);
  }

  return true;
}

}  // namespace audit_log_filter::event_field_action
