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

#ifndef AUDIT_LOG_FILTER_FILTER_H_INCLUDED
#define AUDIT_LOG_FILTER_FILTER_H_INCLUDED

#include "components/audit_log_filter/audit_action.h"
#include "components/audit_log_filter/audit_record.h"

#include <functional>

namespace audit_log_filter {

class AuditRule;

/**
 * Implements Audit Rule application logic
 */
class AuditEventFilter {
 public:
  /**
   * @brief Apply audit filtering rule.
   *
   * An event which satisfies the 'abort' condition of the rule is not always
   * aborted. AuditAction::Block is returned only for an event which is
   * aborted, its record is not written and is left as it is. The record of
   * an event which is not aborted is prepared and AuditAction::Log is
   * returned for it, whatever the 'log' condition of the rule is.
   *
   * @param rule Filtering rule
   * @param audit_record Audit record
   * @param is_aborted Tells whether the event is aborted when it satisfies
   *                   the 'abort' condition. Called for such an event only.
   * @return Action which should be applied to a record by audit filter,
   *         one of actions defined by @ref AuditAction
   */
  [[nodiscard]] static AuditAction apply(
      AuditRule *rule, AuditRecordVariant &audit_record,
      const std::function<bool()> &is_aborted) noexcept;
};

}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_FILTER_H_INCLUDED
