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

#include "plugin/audit_log_filter/log_writer/file.h"

#include "plugin/audit_log_filter/audit_error_log.h"
#include "plugin/audit_log_filter/log_record_formatter/base.h"
#include "plugin/audit_log_filter/sys_vars.h"

#include "sql/mysqld.h"

#include <filesystem>

namespace audit_log_filter::log_writer {

LogWriter<AuditLogHandlerType::File>::LogWriter(
    std::shared_ptr<SysVars> config,
    std::unique_ptr<log_record_formatter::LogRecordFormatterBase> formatter)
    : LogWriterBase{std::move(config), std::move(formatter)},
      m_is_rotating{false},
      m_strategy{
          get_log_writer_strategy(get_config()->get_file_strategy_type())} {}

LogWriter<AuditLogHandlerType::File>::~LogWriter() { do_close_file(); }

bool LogWriterFile::open() noexcept { return do_open_file(); }

bool LogWriterFile::close() noexcept { return do_close_file(); }

bool LogWriterFile::do_open_file() noexcept {
  auto file_path = std::filesystem::path{mysql_data_home} /
                   std::filesystem::path{get_config()->get_file_name()};
  bool is_new_file = !std::filesystem::exists(file_path);

  if (!is_new_file) {
    m_file_handle.remove_file_footer(file_path,
                                     get_formatter()->get_file_footer());
  }

  if (!m_strategy->do_open_file(&m_file_handle, file_path,
                                get_config()->get_buffer_size())) {
    return false;
  }

  init_formatter();

  if (is_new_file) {
    write(get_formatter()->get_file_header());
  }

  return true;
}

bool LogWriterFile::do_close_file() noexcept {
  write(get_formatter()->get_file_footer());
  return m_strategy->do_close_file(&m_file_handle);
}

void LogWriterFile::write(const std::string &record) noexcept {
  m_strategy->do_write(&m_file_handle, record);

  const auto file_size_limit = get_config()->get_rotate_on_size();

  if (file_size_limit > 0 && !m_is_rotating &&
      file_size_limit < get_log_size()) {
    rotate();
  }
}

uint64_t LogWriterFile::get_log_size() const noexcept {
  return m_file_handle.get_file_size();
}

void LogWriterFile::rotate() noexcept {
  m_is_rotating = true;
  do_close_file();

  auto ec = FileHandle::rotate(mysql_data_home, get_config()->get_file_name());

  if (ec.value() != 0) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to rotate audit filter log: %i, %s", ec.value(),
                    ec.message().c_str());
  }

  do_open_file();
  m_is_rotating = false;
}

void LogWriterFile::flush() noexcept {
  do_close_file();
  do_open_file();
}

}  // namespace audit_log_filter::log_writer
