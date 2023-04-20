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

#include "plugin/audit_log_filter/log_writer/base.h"
#include "plugin/audit_log_filter/audit_error_log.h"
#include "plugin/audit_log_filter/log_record_formatter.h"
#include "plugin/audit_log_filter/sys_vars.h"

#include <string>
#include <variant>

namespace audit_log_filter::log_writer {

LogWriterBase::LogWriterBase(
    std::unique_ptr<log_record_formatter::LogRecordFormatterBase> formatter)
    : m_formatter{std::move(formatter)} {}

void LogWriterBase::init_formatter() noexcept { SysVars::init_record_id(0); }

void LogWriterBase::write(AuditRecordVariant record) noexcept {
  // Format event data according to audit_log_filter_format settings
  std::string record_str = std::visit(
      [this](const auto &rec) -> std::string {
        return m_formatter->apply(rec);
      },
      record);

  DBUG_EXECUTE_IF("audit_log_filter_add_record_debug_info", {
    const std::string_view event_class_name = std::visit(
        [](const auto &rec) { return rec.event_class_name; }, record);
    const std::string_view event_subclass_name = std::visit(
        [](const auto &rec) { return rec.event_subclass_name; }, record);

    m_formatter->apply_debug_info(event_class_name, event_subclass_name,
                                  record_str);
  });

  {
    std::lock_guard<std::mutex> write_guaard{m_write_mutex};
    write(record_str, true);
  }
}

}  // namespace audit_log_filter::log_writer
