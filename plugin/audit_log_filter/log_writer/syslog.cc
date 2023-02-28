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
#include "plugin/audit_log_filter/sys_vars.h"

#include <syslog.h>

namespace audit_log_filter::log_writer {

LogWriterSyslog::LogWriter(
    std::unique_ptr<log_record_formatter::LogRecordFormatterBase> formatter)
    : LogWriterBase{std::move(formatter)} {}

bool LogWriterSyslog::init() noexcept {
  return true;  // nothing to do
}

bool LogWriterSyslog::open() noexcept {
  openlog(SysVars::get_syslog_ident(), 0, SysVars::get_syslog_facility());
  return true;
}

bool LogWriterSyslog::close() noexcept {
  closelog();
  return true;
}

void LogWriterSyslog::write(const std::string &record,
                            bool print_separator [[maybe_unused]]) noexcept {
  syslog(SysVars::get_syslog_priority(), "%s", record.c_str());
}

}  // namespace audit_log_filter::log_writer
