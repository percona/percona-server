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

#ifndef AUDIT_LOG_FILTER_LOG_RECORD_FORMATTER_H_INCLUDED
#define AUDIT_LOG_FILTER_LOG_RECORD_FORMATTER_H_INCLUDED

#include "log_record_formatter/base.h"

#include <memory>

namespace audit_log_filter {

using namespace log_record_formatter;

/**
 * @brief Get an instance of log record formatter of specified type.
 *
 * @param [in] format_type Formatter type,
 *                         @see log_record_formatter::AuditLogFormatType
 * @return An instance of log record formatter
 */
std::unique_ptr<LogRecordFormatterBase> get_log_record_formatter(
    AuditLogFormatType format_type);

}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_LOG_RECORD_FORMATTER_H_INCLUDED
