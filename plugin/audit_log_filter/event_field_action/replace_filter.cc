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

#include "plugin/audit_log_filter/event_field_action/replace_filter.h"

#include "plugin/audit_log_filter/event_field_condition/base.h"

namespace audit_log_filter::event_field_action {

EventActionType EventFieldActionReplaceFilter::get_action_type()
    const noexcept {
  return EventActionType::ReplaceFilter;
}

bool EventFieldActionReplaceFilter::apply(const AuditRecordFieldsList &fields
                                          [[maybe_unused]],
                                          AuditRecordVariant &audit_record
                                          [[maybe_unused]]) const noexcept {
  return true;
}

}  // namespace audit_log_filter::event_field_action
