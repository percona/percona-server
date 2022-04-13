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
#include "plugin/audit_log_filter/audit_log_filter.h"
#include "plugin/audit_log_filter/log_record_formatter/base.h"
#include "plugin/audit_log_filter/sys_vars.h"

#include "sql/mysqld.h"

#include <filesystem>
#include <queue>

namespace audit_log_filter::log_writer {

LogWriter<AuditLogHandlerType::File>::LogWriter(
    std::unique_ptr<log_record_formatter::LogRecordFormatterBase> formatter)
    : LogWriterBase{std::move(formatter)},
      m_is_rotating{false},
      m_is_log_empty{true},
      m_strategy{get_log_writer_strategy(SysVars::get_file_strategy_type())} {}

LogWriter<AuditLogHandlerType::File>::~LogWriter() { do_close_file(); }

bool LogWriterFile::open() noexcept { return do_open_file(); }

bool LogWriterFile::close() noexcept { return do_close_file(); }

bool LogWriterFile::do_open_file() noexcept {
  auto file_path = std::filesystem::path{mysql_data_home} /
                   std::filesystem::path{SysVars::get_file_name()};
  bool is_new_file = !std::filesystem::exists(file_path);

  if (!is_new_file) {
    m_file_handle.remove_file_footer(file_path,
                                     get_formatter()->get_file_footer());
  }

  if (!m_strategy->do_open_file(&m_file_handle, file_path,
                                SysVars::get_buffer_size())) {
    return false;
  }

  SysVars::set_total_log_size(FileHandle::get_total_log_size(
      mysql_data_home, SysVars::get_file_name()));
  SysVars::set_current_log_size(get_log_size());

  init_formatter();

  if (is_new_file) {
    write(get_formatter()->get_file_header(), false);
    m_is_log_empty = true;
  }

  return true;
}

bool LogWriterFile::do_close_file() noexcept {
  write(get_formatter()->get_file_footer(), false);
  return m_strategy->do_close_file(&m_file_handle);
}

void LogWriterFile::write(const std::string &record,
                          const bool print_separator) noexcept {
  if (print_separator && !m_is_log_empty) {
    m_strategy->do_write(&m_file_handle,
                         get_formatter()->get_record_separator());
  }

  m_strategy->do_write(&m_file_handle, record);

  auto record_size = record.size();
  SysVars::update_current_log_size(record_size);
  SysVars::update_total_log_size(record_size);

  if (m_is_log_empty) {
    m_is_log_empty = false;
  }

  const auto file_size_limit = SysVars::get_rotate_on_size();

  if (file_size_limit > 0 && !m_is_rotating &&
      file_size_limit < get_log_size()) {
    rotate();
    prune();
  }
}

uint64_t LogWriterFile::get_log_size() const noexcept {
  return m_file_handle.get_file_size();
}

void LogWriterFile::rotate() noexcept {
  m_is_rotating = true;
  do_close_file();

  auto ec = FileHandle::rotate(mysql_data_home, SysVars::get_file_name());

  if (ec.value() != 0) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to rotate audit filter log: %i, %s", ec.value(),
                    ec.message().c_str());
  }

  do_open_file();
  m_is_rotating = false;

  get_audit_log_filter_instance()->on_audit_log_rotated();
}

void LogWriterFile::flush() noexcept {
  do_close_file();
  do_open_file();
}

void LogWriterFile::prune() noexcept {
  if (SysVars::get_rotate_on_size() == 0) {
    return;
  }

  const auto log_max_size = SysVars::get_log_max_size();
  const auto prune_seconds = SysVars::get_log_prune_seconds();

  if (log_max_size > 0) {
    auto log_file_list =
        FileHandle::get_prune_files(mysql_data_home, SysVars::get_file_name());

    ulonglong current_logs_size = std::accumulate(
        log_file_list.begin(), log_file_list.end(), 0,
        [](const ulonglong &a, const PruneFileInfo &b) { return a + b.size; });
    current_logs_size += get_log_size();

    if (current_logs_size < log_max_size) {
      return;
    }

    auto comparator = [](const PruneFileInfo &a, const PruneFileInfo &b) {
      return a.age < b.age;
    };

    std::priority_queue<PruneFileInfo, PruneFilesList, decltype(comparator)>
        file_queue{comparator, log_file_list};

    while (current_logs_size > log_max_size && !file_queue.empty()) {
      const auto &entry = file_queue.top();

      if (!FileHandle::remove_file(entry.path)) {
        return;
      }

      current_logs_size =
          (entry.size < current_logs_size) ? current_logs_size - entry.size : 0;
      file_queue.pop();
    }
  } else if (prune_seconds > 0) {
    auto log_file_list =
        FileHandle::get_prune_files(mysql_data_home, SysVars::get_file_name());

    for (const auto &entry : log_file_list) {
      if (entry.age > prune_seconds) {
        FileHandle::remove_file(entry.path);
      }
    }
  }

  SysVars::set_total_log_size(FileHandle::get_total_log_size(
      mysql_data_home, SysVars::get_file_name()));
}

}  // namespace audit_log_filter::log_writer
