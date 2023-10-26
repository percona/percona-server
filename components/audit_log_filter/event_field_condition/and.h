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

#ifndef AUDIT_LOG_FILTER_EVENT_FIELD_CONDITION_AND_H_INCLUDED
#define AUDIT_LOG_FILTER_EVENT_FIELD_CONDITION_AND_H_INCLUDED

#include "base.h"

#include <memory>
#include <vector>

namespace audit_log_filter::event_field_condition {

class EventFieldConditionAnd : public EventFieldConditionBase {
 public:
  explicit EventFieldConditionAnd(
      std::vector<std::shared_ptr<EventFieldConditionBase>> conditions);

  /**
   * @brief Check if logical condition applies to provided event fields.
   *
   * @param fields Event fields list
   * @return true in case condition applies to an audit event, false otherwise
   */
  [[nodiscard]] bool check_applies(
      const AuditRecordFieldsList &fields) const noexcept override;

 private:
  std::vector<std::shared_ptr<EventFieldConditionBase>> m_conditions;
};

}  // namespace audit_log_filter::event_field_condition

#endif  // AUDIT_LOG_FILTER_EVENT_FIELD_CONDITION_AND_H_INCLUDED
