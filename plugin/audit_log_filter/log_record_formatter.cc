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

#include "plugin/audit_log_filter/log_record_formatter.h"

#include "log_record_formatter/csv.h"
#include "log_record_formatter/json.h"
#include "log_record_formatter/new.h"
#include "log_record_formatter/old.h"

namespace audit_log_filter {

template <AuditLogFormatType FormatType>
std::unique_ptr<LogRecordFormatterBase> create_helper() {
  return std::make_unique<LogRecordFormatter<FormatType>>();
}

std::unique_ptr<LogRecordFormatterBase> get_log_record_formatter(
    AuditLogFormatType format_type) {
  using CreateFunc = std::unique_ptr<LogRecordFormatterBase> (*)();
  static const CreateFunc
      funcs[static_cast<int>(AuditLogFormatType::FormatsCount)] = {
          create_helper<AuditLogFormatType::New>,
          create_helper<AuditLogFormatType::Old>,
          create_helper<AuditLogFormatType::Json>,
          create_helper<AuditLogFormatType::Csv>};
  return (*funcs[static_cast<int>(format_type)])();
}

}  // namespace audit_log_filter
