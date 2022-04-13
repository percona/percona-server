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

#include "plugin/audit_log_filter/event_field_condition/bool.h"

namespace audit_log_filter::event_field_condition {

EventFieldConditionBool::EventFieldConditionBool(const bool value)
    : m_value{value} {}

bool EventFieldConditionBool::check_applies(const AuditRecordFieldsList &fields
                                            [[maybe_unused]]) const noexcept {
  return m_value;
}

}  // namespace audit_log_filter::event_field_condition
