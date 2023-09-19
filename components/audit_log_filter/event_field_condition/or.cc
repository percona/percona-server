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

#include "components/audit_log_filter/event_field_condition/or.h"

#include <algorithm>

namespace audit_log_filter::event_field_condition {

EventFieldConditionOr::EventFieldConditionOr(
    std::vector<std::shared_ptr<EventFieldConditionBase>> conditions)
    : m_conditions{std::move(conditions)} {}

bool EventFieldConditionOr::check_applies(
    const AuditRecordFieldsList &fields) const noexcept {
  return std::any_of(
      m_conditions.cbegin(), m_conditions.cend(),
      [&fields](const auto &cond) { return cond->check_applies(fields); });
}

}  // namespace audit_log_filter::event_field_condition
