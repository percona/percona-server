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

#include "plugin/audit_log_filter/log_writer.h"

#include "log_record_formatter/base.h"
#include "log_writer/file.h"
#include "log_writer/syslog.h"
#include "sys_vars.h"

namespace audit_log_filter {

template <AuditLogHandlerType HandlerType>
std::unique_ptr<LogWriterBase> create_helper(
    std::unique_ptr<log_record_formatter::LogRecordFormatterBase> formatter) {
  return std::make_unique<LogWriter<HandlerType>>(std::move(formatter));
}

std::unique_ptr<LogWriterBase> get_log_writer(
    std::unique_ptr<log_record_formatter::LogRecordFormatterBase> formatter) {
  using CreateFunc = std::unique_ptr<LogWriterBase> (*)(
      std::unique_ptr<log_record_formatter::LogRecordFormatterBase>);

  const auto handler_type = SysVars::get_handler_type();
  static const CreateFunc
      funcs[static_cast<int>(AuditLogHandlerType::TypesCount)] = {
          create_helper<AuditLogHandlerType::File>,
          create_helper<AuditLogHandlerType::Syslog>};

  return (*funcs[static_cast<int>(handler_type)])(std::move(formatter));
}

}  // namespace audit_log_filter
