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

#include "plugin/audit_log_filter/log_writer/file_writer.h"
#include "plugin/audit_log_filter/log_writer/file_writer_buffering.h"
#include "plugin/audit_log_filter/log_writer/file_writer_compressing.h"
#include "plugin/audit_log_filter/log_writer/file_writer_encrypting.h"

#include "plugin/audit_log_filter/audit_error_log.h"
#include "plugin/audit_log_filter/audit_keyring.h"
#include "plugin/audit_log_filter/audit_log_filter.h"
#include "plugin/audit_log_filter/log_record_formatter/base.h"
#include "plugin/audit_log_filter/sys_vars.h"

#include "sql/mysqld.h"

#include <filesystem>
#include <queue>

namespace audit_log_filter::log_writer {

namespace {

FileWriterPtr get_file_writer(FileHandle &file_handle) {
  /*
   * ASYNCHRONOUS - (default) log using memory buffer, do not drop messages
   *                if buffer is full
   * PERFORMANCE - log using memory buffer, drop messages if buffer is full
   * SEMISYNCHRONOUS - log directly to file, do not flush and sync every event
   * SYNCHRONOUS - log directly to file, flush and sync every event.
   */
  auto strategy_type = SysVars::get_file_strategy_type();
  std::unique_ptr<FileWriterBase> writer = std::make_unique<FileWriter>(
      file_handle, strategy_type == AuditLogStrategyType::Synchronous);

  if (SysVars::get_log_encryption_enabled()) {
    writer = std::make_unique<FileWriterEncrypting>(std::move(writer));
  }

  if (SysVars::get_compression_type() == AuditLogCompressionType::Gzip) {
    writer = std::make_unique<FileWriterCompressing>(std::move(writer));
  }

  if (strategy_type == AuditLogStrategyType::Asynchronous ||
      strategy_type == AuditLogStrategyType::Performance) {
    writer = std::make_unique<FileWriterBuffering>(
        std::move(writer), SysVars::get_buffer_size(),
        strategy_type == AuditLogStrategyType::Performance);

    if (!writer->init()) {
      return nullptr;
    }
  }

  return writer;
}

}  // namespace

LogWriter<AuditLogHandlerType::File>::LogWriter(
    std::unique_ptr<log_record_formatter::LogRecordFormatterBase> formatter)
    : LogWriterBase{std::move(formatter)},
      m_is_rotating{false},
      m_is_log_empty{true},
      m_is_opened{false},
      m_file_writer{nullptr} {}

LogWriter<AuditLogHandlerType::File>::~LogWriter() { do_close_file(); }

bool LogWriterFile::init() noexcept {
  m_file_writer = get_file_writer(m_file_handle);
  return m_file_writer != nullptr;
}

bool LogWriterFile::open() noexcept {
  assert(m_file_writer != nullptr);

  const auto current_log_path = FileHandle::get_not_rotated_file_path(
      mysql_data_home, SysVars::get_file_name());
  auto ec = FileHandle::rotate(current_log_path);

  if (ec.value() != 0) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to rotate audit filter log: %i, %s", ec.value(),
                    ec.message().c_str());
    return false;
  }

  return do_open_file();
}

bool LogWriterFile::close() noexcept { return do_close_file(); }

bool LogWriterFile::do_open_file() noexcept {
  auto file_path = std::filesystem::path{mysql_data_home} /
                   std::filesystem::path{SysVars::get_file_name()};
  if (SysVars::get_compression_type() != AuditLogCompressionType::None) {
    file_path += ".gz";
  }

  if (SysVars::get_log_encryption_enabled()) {
    std::stringstream suffix;
    suffix << "."
           << audit_keyring::get_password_id_timestamp(
                  SysVars::get_encryption_password_id())
                  .c_str()
           << ".enc";
    file_path += suffix.str();
  }

  bool is_new_file = !std::filesystem::exists(file_path);

  if (!is_new_file) {
    FileHandle::remove_file_footer(file_path,
                                   get_formatter()->get_file_footer());
  }

  if (!m_file_handle.open_file(file_path)) {
    return false;
  }

  if (!m_file_writer->open()) {
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

  m_is_opened = true;

  return true;
}

bool LogWriterFile::do_close_file() noexcept {
  if (!m_is_opened) {
    return true;
  }

  write(get_formatter()->get_file_footer(), false);
  m_file_writer->close();
  m_is_opened = false;

  return m_file_handle.close_file();
}

void LogWriterFile::write(const std::string &record,
                          const bool print_separator) noexcept {
  std::lock_guard<std::recursive_mutex> write_guard{m_write_lock};

  if (print_separator && !m_is_log_empty) {
    const auto separator = get_formatter()->get_record_separator();
    m_file_writer->write(separator.c_str(), separator.length());
  }

  m_file_writer->write(record.c_str(), record.length());

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
  std::lock_guard<std::recursive_mutex> write_guard{m_write_lock};

  m_is_rotating = true;
  const auto current_log_path = m_file_handle.get_file_path();

  do_close_file();

  auto ec = FileHandle::rotate(current_log_path);

  if (ec.value() != 0) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to rotate audit filter log: %i, %s", ec.value(),
                    ec.message().c_str());
  }

  do_open_file();
  m_is_rotating = false;

  get_audit_log_filter_instance()->on_audit_log_rotated();
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
