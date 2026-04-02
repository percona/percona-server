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

#define ALLOW_COMPONENT_INCLUDE // for plugin.h
#include "components/audit_log_filter/log_writer/file.h"

#include "components/audit_log_filter/log_writer/file_writer.h"
#include "components/audit_log_filter/log_writer/file_writer_buffering.h"
#include "components/audit_log_filter/log_writer/file_writer_compressing.h"
#include "components/audit_log_filter/log_writer/file_writer_encrypting.h"

#include "components/audit_log_filter/audit_error_log.h"
#include "components/audit_log_filter/audit_keyring.h"
#include "components/audit_log_filter/audit_log_filter.h"
#include "components/audit_log_filter/log_record_formatter/base.h"
#include "components/audit_log_filter/sys_vars.h"

#include <exception>
#include <filesystem>
#include <memory>
#include <numeric>
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
  try {
    auto strategy_type = SysVars::get_file_strategy_type();
    std::unique_ptr<FileWriterBase> writer =
        std::make_unique<FileWriter>(file_handle);

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
    }

    return writer;
  } catch (std::exception &e) {
    LogComponentErr(ERROR_LEVEL, ER_AUDIT_LOG_FILE_WRITER_CREATE_FAILURE,
                    e.what());
  } catch (...) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to create Audit Log Filter file writer");
  }
  return nullptr;
}

}  // namespace

LogWriter<AuditLogHandlerType::File>::LogWriter(
    std::unique_ptr<log_record_formatter::LogRecordFormatterBase> formatter)
    : LogWriterBase{std::move(formatter)},
      m_is_rotating{false},
      m_is_log_empty{true},
      m_is_opened{false},
      m_sync_on_write{false},
      m_file_writer{nullptr} {}

LogWriter<AuditLogHandlerType::File>::~LogWriter() {
  do_close_file();

  std::filesystem::path current_log_path;
  if (!FileHandle::get_not_rotated_file_path(SysVars::get_file_dir(),
                                             SysVars::get_file_name(),
                                             current_log_path)) {
    LogComponentErr(ERROR_LEVEL, ER_AUDIT_LOG_DIR_LIST_FAILURE,
                    SysVars::get_file_dir().c_str());
    return;
  }
  auto rotation_result = std::make_unique<log_writer::FileRotationResult>();
  FileHandle::rotate(current_log_path, rotation_result.get());

  if (rotation_result->error_code != 0) {
    LogComponentErr(ERROR_LEVEL, ER_AUDIT_LOG_ROTATE_INTERNAL_FAILURE,
                    rotation_result->error_code,
                    rotation_result->status_string.c_str());
  }
}

bool LogWriterFile::init() noexcept {
  m_sync_on_write =
      SysVars::get_file_strategy_type() == AuditLogStrategyType::Synchronous;
  m_file_writer = get_file_writer(m_file_handle);
  return m_file_writer != nullptr;
}

bool LogWriterFile::open() noexcept {
  assert(m_file_writer != nullptr);

  std::filesystem::path current_log_path;
  if (!FileHandle::get_not_rotated_file_path(SysVars::get_file_dir(),
                                             SysVars::get_file_name(),
                                             current_log_path)) {
    LogComponentErr(ERROR_LEVEL, ER_AUDIT_LOG_DIR_LIST_FAILURE,
                    SysVars::get_file_dir().c_str());
    return false;
  }
  auto rotation_result = std::make_unique<log_writer::FileRotationResult>();
  FileHandle::rotate(current_log_path, rotation_result.get());

  if (rotation_result->error_code != 0) {
    LogComponentErr(ERROR_LEVEL, ER_AUDIT_LOG_ROTATE_INTERNAL_FAILURE,
                    rotation_result->error_code,
                    rotation_result->status_string.c_str());
    return false;
  }

  return do_open_file();
}

bool LogWriterFile::close() noexcept {
  std::lock_guard<std::mutex> write_guard{m_write_mutex};
  return do_close_file();
}

bool LogWriterFile::do_open_file() noexcept {
  auto file_path = std::filesystem::path{SysVars::get_file_dir()} /
                   std::filesystem::path{SysVars::get_file_name()};
  if (SysVars::get_compression_type() != AuditLogCompressionType::None) {
    file_path += ".gz";
  }

  if (SysVars::get_log_encryption_enabled()) {
    std::stringstream suffix;
    suffix << "."
           << audit_keyring::get_options_id_timestamp(
                  SysVars::get_encryption_options_id())
                  .c_str()
           << ".enc";
    file_path += suffix.str();
  }

  std::error_code ec;
  const bool file_exists = std::filesystem::exists(file_path, ec);
  if (ec) {
    const auto file_path_str = file_path.string();
    LogComponentErr(ERROR_LEVEL, ER_AUDIT_LOG_FILE_INSPECT_FAILURE,
                    file_path_str.c_str(), ec.message().c_str());
    return false;
  }
  bool is_new_file = !file_exists;

  if (!is_new_file) {
    if (!FileHandle::remove_file_footer(file_path,
                                        get_formatter()->get_file_footer())) {
      const auto file_path_str = file_path.string();
      LogComponentErr(ERROR_LEVEL, ER_AUDIT_LOG_FILE_PREPARE_FAILURE,
                      file_path_str.c_str());
      return false;
    }
  }

  if (!m_file_handle.open_file(file_path, SysVars::get_direct_io(),
                               SysVars::get_file_strategy_type() ==
                                   AuditLogStrategyType::Semisynchronous)) {
    return false;
  }

  if (!m_file_writer->open()) {
    return false;
  }

  uint64_t total_size = 0;
  if (FileHandle::get_total_log_size(SysVars::get_file_dir(),
                                     SysVars::get_file_name(), total_size)) {
    SysVars::set_total_log_size(total_size);
  }
  SysVars::set_current_log_size(do_get_log_size());

  init_formatter();

  if (is_new_file) {
    do_write(get_formatter()->get_file_header(), false);
    m_is_log_empty = true;
  }

  m_is_opened = true;

  return true;
}

bool LogWriterFile::do_close_file() noexcept {
  if (!m_is_opened) {
    return true;
  }

  do_write(get_formatter()->get_file_footer(), false);
  m_file_writer->close();
  m_is_opened = false;

  return m_file_handle.close_file();
}

void LogWriterFile::write(const std::string &record,
                          const bool print_separator) noexcept {
  do_write(record, print_separator);
}

void LogWriterFile::do_write(const std::string &record,
                             bool print_separator) noexcept {
  size_t written_size = 0;
  std::string payload;
  if (print_separator && !m_is_log_empty) {
    const auto separator = get_formatter()->get_record_separator();
    payload.reserve(separator.length() + record.length());
    payload.append(separator);
  } else {
    payload.reserve(record.length());
  }
  payload.append(record);

  m_file_writer->write(payload.c_str(), payload.length());
  written_size += payload.length();

  written_size += write_padding();

  SysVars::update_current_log_size(written_size);
  SysVars::update_total_log_size(written_size);

  if (m_is_log_empty) {
    m_is_log_empty = false;
  }

  if (m_sync_on_write) {
    m_file_writer->sync();
  }

  const auto file_size_limit = SysVars::get_rotate_on_size();

  if (file_size_limit > 0 && !m_is_rotating &&
      file_size_limit < do_get_log_size()) {
    do_rotate(nullptr);
    do_prune();
  }
}

size_t LogWriterFile::write_padding() {
  // This function writes whitespace padding after each logged event when
  // log file encryption is enabled.
  // This is necessary in order to make newly logged events be immediately
  // available for reading by AuditLogReader. Padding pushes internal buffer of
  // encryption context over the threshold after which EVP_EncryptUpdate is
  // guaranteed to produce complete encrypted log event.

  size_t written_size = 0;

  // We add padding only for formats supported by audit log reader.
  // Currently, it's only JSON and JSONL.
  if (SysVars::get_format_type() != AuditLogFormatType::Json &&
      SysVars::get_format_type() != AuditLogFormatType::Jsonl) {
    return written_size;
  }

  // Padding is only needed for logs encrypted with certain block ciphers.
  if (SysVars::get_encryption_type() != AuditLogEncryptionType::Aes) {
    return written_size;
  }

  auto write_spaces = [&](size_t count) {
    static constexpr char padding[] = "                                ";
    m_file_writer->write(padding, count);
    written_size += count;
  };

  switch (SysVars::get_compression_type()) {
    case AuditLogCompressionType::None:
      // Write two AES 256 CBC blocks worth of spaces.
      write_spaces(2 * 16);
      break;
    case AuditLogCompressionType::Gzip:
      // Writes need to be done in separate chunks, each producing its own
      // gzip block. Otherwise, all spaces would get compressed.
      // Five such chunks are enough to produce needed padding.
      for (int chunk = 0; chunk < 5; ++chunk) {
        write_spaces(2);
      }
      break;
    default:
      assert(false);
  }

  return written_size;
}

uint64_t LogWriterFile::get_log_size() const noexcept {
  std::lock_guard<std::mutex> write_guard{m_write_mutex};
  return do_get_log_size();
}

uint64_t LogWriterFile::do_get_log_size() const noexcept {
  return m_file_handle.get_file_size();
}

void LogWriterFile::rotate(FileRotationResult *result) noexcept {
  std::lock_guard<std::mutex> write_guard{m_write_mutex};
  do_rotate(result);
}

void LogWriterFile::do_rotate(FileRotationResult *result) noexcept {
  m_is_rotating = true;
  const auto current_log_path = m_file_handle.get_file_path();

  do_close_file();

  std::unique_ptr<FileRotationResult> local_result;
  if (result == nullptr) {
    local_result = std::make_unique<FileRotationResult>();
    result = local_result.get();
  }

  FileHandle::rotate(current_log_path, result);

  if (result->error_code != 0) {
    LogComponentErr(ERROR_LEVEL, ER_AUDIT_LOG_ROTATE_INTERNAL_FAILURE,
                    result->error_code, result->status_string.c_str());
  }

  do_open_file();
  m_is_rotating = false;

  get_audit_log_filter_instance()->on_audit_log_rotated();
}

void LogWriterFile::prune() noexcept {
  std::lock_guard<std::mutex> write_guard{m_write_mutex};
  do_prune();
}

void LogWriterFile::do_prune() noexcept {
  const auto log_max_size = SysVars::get_log_max_size();
  const auto prune_seconds = SysVars::get_log_prune_seconds();

  if (log_max_size > 0) {
    PruneFilesList log_file_list;
    if (!FileHandle::get_prune_files(SysVars::get_file_dir(),
                                     SysVars::get_file_name(), log_file_list)) {
      return;
    }

    ulonglong current_logs_size = std::accumulate(
        log_file_list.begin(), log_file_list.end(), ulonglong{0},
        [](const ulonglong &a, const PruneFileInfo &b) { return a + b.size; });
    current_logs_size += do_get_log_size();

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
    PruneFilesList log_file_list;
    if (!FileHandle::get_prune_files(SysVars::get_file_dir(),
                                     SysVars::get_file_name(), log_file_list)) {
      return;
    }

    for (const auto &entry : log_file_list) {
      if (entry.age > std::chrono::seconds(prune_seconds)) {
        FileHandle::remove_file(entry.path);
      }
    }
  }

  uint64_t total_size = 0;
  if (FileHandle::get_total_log_size(SysVars::get_file_dir(),
                                     SysVars::get_file_name(), total_size)) {
    SysVars::set_total_log_size(total_size);
  }
}

}  // namespace audit_log_filter::log_writer
