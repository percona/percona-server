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

#ifndef AUDIT_LOG_FILTER_EVENT_FIELD_ACTION_PRINT_SERVICE_COMP_H_INCLUDED
#define AUDIT_LOG_FILTER_EVENT_FIELD_ACTION_PRINT_SERVICE_COMP_H_INCLUDED

#include "plugin/audit_log_filter/event_field_action/base.h"

#include <string>
#include <string_view>
#include <vector>

namespace audit_log_filter::event_field_action {

enum class ServiceCompElementType { Double, Longlong, Unknown };

using ServiceCompElementName = std::string_view;
using PrintServiceElementsList =
    std::vector<std::pair<ServiceCompElementType, ServiceCompElementName>>;

class EventFieldActionPrintServiceComp : public EventFieldActionBase {
 public:
  EventFieldActionPrintServiceComp(std::string tag_name,
                                   PrintServiceElementsList elements_list);

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

  /**
   * @brief Get element type from its string representation.
   *
   * @param type_name Element type name
   * @return Element type
   */
  static ServiceCompElementType string_to_element_type(
      const std::string &type_name) noexcept;

  /**
   * @brief Get element name from its string representation.
   *
   * @param element_name Element name
   * @return Element name
   */
  static ServiceCompElementName string_to_element_name(
      const std::string &element_name) noexcept;

 private:
  std::string m_tag_name;
  PrintServiceElementsList m_elements_list;
};

}  // namespace audit_log_filter::event_field_action

#endif  // AUDIT_LOG_FILTER_EVENT_FIELD_ACTION_PRINT_SERVICE_COMP_H_INCLUDED
