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

#include "plugin/audit_log_filter/audit_filter.h"
#include "plugin/audit_log_filter/audit_error_log.h"

namespace audit_log_filter {

AuditAction AuditEventFilter::apply(
    AuditRule *rule, const AuditRecordVariant &audit_record) noexcept {
  if (!rule->check_parse_state()) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Filtering rule '%s' has wrong format, "
                    "skipping audit event",
                    rule->get_rule_name().c_str());
    return AuditAction::Skip;
  }

  AuditAction action = rule->get_global_action();

  auto event_class_name = std::visit(
      [](const auto &rec) { return rec.event_class_name; }, audit_record);

  auto class_action = rule->get_event_class_action(event_class_name);

  if (class_action == AuditAction::None) {
    // No match for event class, use globally defined action
    return action;
  }

  auto event_subclass_name = std::visit(
      [](const auto &rec) { return rec.event_subclass_name; }, audit_record);

  auto subclass_action = rule->get_event_subclass_action(event_subclass_name);

  if (subclass_action == AuditAction::None) {
    return class_action;
  }

  return subclass_action;
}

}  // namespace audit_log_filter
