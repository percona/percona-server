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

#include "plugin/audit_log_filter/log_writer/syslog.h"
#include "plugin/audit_log_filter/log_record_formatter/base.h"

namespace audit_log_filter::log_writer {

LogWriterSyslog::LogWriter(
    std::unique_ptr<log_record_formatter::LogRecordFormatterBase> formatter,
    const LogWriterConfig &conf)
    : LogWriterBase{std::move(formatter)},
      m_syslog_ident{conf.syslog_ident},
      m_syslog_facility{conf.syslog_facility},
      m_syslog_priority{conf.syslog_priority} {}

}  // namespace audit_log_filter::log_writer
