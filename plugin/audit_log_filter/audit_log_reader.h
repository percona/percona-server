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

#ifndef AUDIT_LOG_FILTER_AUDIT_LOG_READER_H_INCLUDED
#define AUDIT_LOG_FILTER_AUDIT_LOG_READER_H_INCLUDED

#include "plugin/audit_log_filter/audit_encryption.h"
#include "plugin/audit_log_filter/json_reader/audit_json_handler.h"
#include "plugin/audit_log_filter/json_reader/audit_json_read_stream.h"
#include "plugin/audit_log_filter/sys_vars.h"

#include <atomic>
#include <deque>
#include <map>
#include <memory>
#include <shared_mutex>
#include <string>
#include <utility>
#include <vector>

namespace audit_log_filter {

struct AuditLogReaderArgs {
  std::string timestamp{};
  uint64_t id{0};
  uint64_t max_array_length{0};
  bool close_read_sequence{false};
};

struct AuditLogReaderContext {
  LogBookmark next_event_bookmark;
  std::unique_ptr<AuditLogReaderArgs> batch_reader_args;
  std::unique_ptr<json_reader::AuditJsonHandler> audit_json_handler;
  std::unique_ptr<json_reader::AuditJsonReadStream> audit_json_read_stream;
  std::deque<FileInfo *> files_to_read;
  FileInfo *current_file;
  bool is_session_end{false};
  bool is_batch_end{false};
};

struct FileInfo {
  FileInfo(std::string name_, std::string encryption_options_id_,
           bool is_compressed_, bool is_encrypted_)
      : name{std::move(name_)},
        encryption_options_id{std::move(encryption_options_id_)},
        is_compressed{is_compressed_},
        is_encrypted{is_encrypted_},
        encryption_options{nullptr} {}
  std::string name;
  std::string encryption_options_id;
  bool is_compressed;
  bool is_encrypted;
  std::unique_ptr<encryption::EncryptionOptions> encryption_options;
};

class AuditLogReader {
 public:
  AuditLogReader() = default;

  void reset() noexcept;

  bool init() noexcept;

  bool read(AuditLogReaderContext *reader_context) noexcept;

  AuditLogReaderContext *init_reader_session(
      MYSQL_THD thd, const AuditLogReaderArgs *reader_args) noexcept;

  void close_reader_session(AuditLogReaderContext *reader_context) noexcept;

 private:
  void set_files_to_read_list(AuditLogReaderContext *reader_context) noexcept;

 private:
  std::map<std::string, std::unique_ptr<FileInfo>>
      m_first_timestamp_to_file_map;
  std::shared_mutex m_reader_mutex;
  std::atomic<bool> m_reload_requested;
};

}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_AUDIT_LOG_READER_H_INCLUDED
