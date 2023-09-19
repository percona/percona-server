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

#include "components/audit_log_filter/event_field_condition/variable.h"

namespace audit_log_filter::event_field_condition {

EventFieldConditionVariable::EventFieldConditionVariable(
    std::string name, std::string expected_value)
    : m_name{std::move(name)}, m_expected_value{std::move(expected_value)} {}

bool EventFieldConditionVariable::check_applies(
    const AuditRecordFieldsList &fields [[maybe_unused]]) const noexcept {
  // TODO: not implemented
  return false;
}

}  // namespace audit_log_filter::event_field_condition
