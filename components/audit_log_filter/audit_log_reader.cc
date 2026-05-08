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

#include "components/audit_log_filter/audit_log_reader.h"

#include "components/audit_log_filter/audit_error_log.h"
#include "components/audit_log_filter/audit_keyring.h"
#include "components/audit_log_filter/audit_psi_info.h"

#include "rapidjson/document.h"
#include "rapidjson/reader.h"

#include <mysql/components/my_service.h>
#include <mysql/components/services/mysql_current_thread_reader.h>

#include "mysql/components/library_mysys/my_memory.h"

#include <scope_guard.h>
#include <algorithm>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace {

/**
  SAX parser handler for audit JSON logs.
  Saves "timestamp" field from the first encountered event, and stops parsing.
*/
class AuditJsonFirstTimestampHandler
    : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>,
                                          AuditJsonFirstTimestampHandler> {
 public:
  bool Default() {
    m_expect_timestamp = false;

    // We want to make sure the first parsed token is "open-array".
    // If we encounter something else first, we stop parsing.
    return m_array_open;
  }

  bool String(const char *str, rapidjson::SizeType length,
              [[maybe_unused]] bool copy) {
    if (m_expect_timestamp) {
      timestamp.assign(str, length);
      return false;
    }
    return Default();
  }

  bool StartObject() {
    ++m_depth;
    return Default();
  }

  bool Key(const char *str, rapidjson::SizeType length,
           [[maybe_unused]] bool copy) {
    // Depth 2 means we're in "[ { ...".
    m_expect_timestamp =
        m_depth == 2 && std::string_view(str, length) == "timestamp";
    return true;
  }

  bool EndObject([[maybe_unused]] rapidjson::SizeType memberCount) {
    --m_depth;
    return Default();
  }

  bool StartArray() {
    m_array_open = true;
    ++m_depth;
    return Default();
  }

  bool EndArray([[maybe_unused]] rapidjson::SizeType elementCount) {
    --m_depth;
    return Default();
  }

  std::string timestamp;

 private:
  int m_depth = 0;
  bool m_expect_timestamp = false;
  bool m_array_open = false;
};

}  // namespace

namespace audit_log_filter {

void AuditLogReader::set_files_to_read_list(
    AuditLogReaderContext *const reader_context) noexcept {
  if (reader_context == nullptr) {
    return;
  }

  for (auto item = m_timestamp_to_file_map.cbegin();
       item != m_timestamp_to_file_map.cend(); ++item) {
    auto next_item = std::next(item);
    bool is_last_item = next_item == m_timestamp_to_file_map.cend();

    if (is_last_item || reader_context->next_event_bookmark.timestamp <=
                            next_item->second->first_timestamp) {
      auto *file_info = item->second.get();

      if (file_info->is_encrypted && file_info->encryption_options == nullptr) {
        continue;
      }

      reader_context->files_to_read.push_back(file_info);
    }
  }
}

void AuditLogReader::reset() noexcept { m_reload_requested = true; }

bool AuditLogReader::init() noexcept {
  if (SysVars::get_format_type() != AuditLogFormatType::Json &&
      SysVars::get_format_type() != AuditLogFormatType::Jsonl) {
    // Not supported for other log formats
    return true;
  }

  std::unique_lock lock(m_reader_mutex);

  if (!m_reload_requested) {
    return true;
  }

  my_service<SERVICE_TYPE(mysql_current_thread_reader)> thd_reader_srv(
      "mysql_current_thread_reader", SysVars::get_comp_registry_srv());

  MYSQL_THD thd = nullptr;

  if (thd_reader_srv->get(&thd) == 1 || thd == nullptr) {
    return false;
  }

  auto json_reader_stream =
      std::make_unique<json_reader::AuditJsonReadStream>();

  if (!json_reader_stream->init()) {
    return false;
  }

  const auto log_current_file_name = SysVars::get_file_name();
  auto log_base_file_name = std::filesystem::path{log_current_file_name};

  while (log_base_file_name.has_extension()) {
    log_base_file_name.replace_extension();
  }

  std::vector<std::string> all_files;
  bool have_complete_file_snapshot = true;

  std::error_code ec;
  auto it = std::filesystem::directory_iterator{SysVars::get_file_dir(), ec};

  if (ec) {
    return false;
  }

  for (; it != std::filesystem::directory_iterator{}; it.increment(ec)) {
    const auto &entry = *it;
    auto log_name = entry.path().filename().string();

    std::error_code entry_ec;
    if (entry.is_regular_file(entry_ec) && !entry_ec &&
        log_name.find(log_base_file_name) != std::string::npos) {
      all_files.push_back(std::move(log_name));
    } else if (entry_ec) {
      have_complete_file_snapshot = false;
    }
  }

  if (ec) {
    have_complete_file_snapshot = false;
  }

  std::vector<std::string> new_files;
  std::vector<std::string> removed_files;

  std::copy_if(std::cbegin(all_files), std::cend(all_files),
               std::back_inserter(new_files), [this](const auto &name) {
                 return !std::any_of(std::cbegin(m_timestamp_to_file_map),
                                     std::cend(m_timestamp_to_file_map),
                                     [&name](const auto &entry) {
                                       return entry.second->name == name;
                                     });
               });

  if (have_complete_file_snapshot) {
    std::for_each(
        std::cbegin(m_timestamp_to_file_map),
        std::cend(m_timestamp_to_file_map),
        [&all_files, &removed_files](const auto &pair) {
          if (!std::any_of(std::cbegin(all_files), std::cend(all_files),
                           [&pair](const auto &name) {
                             return name == pair.second->name;
                           })) {
            removed_files.push_back(pair.second->name);
          }
        });
  }

  for (const auto &log_name : new_files) {
    bool is_compressed = log_name.find(".gz") != std::string::npos;
    bool is_encrypted = log_name.find(".enc") != std::string::npos;

    auto encryption_options_id =
        audit_keyring::get_options_id_for_file_name(log_name);

    assert(!is_encrypted || !encryption_options_id.empty());

    auto file_info = std::make_unique<FileInfo>(log_name, encryption_options_id,
                                                is_compressed, is_encrypted);

    if (file_info->is_encrypted && !file_info->encryption_options_id.empty()) {
      file_info->encryption_options = audit_keyring::get_encryption_options(
          file_info->encryption_options_id);

      if (file_info->encryption_options == nullptr) {
        continue;
      }
    }

    if (!json_reader_stream->open(file_info.get())) {
      continue;
    }

    auto json_reader_guard =
        create_scope_guard([&] { json_reader_stream->close(); });

    rapidjson::Reader json_reader;
    AuditJsonFirstTimestampHandler first_timestamp_handler;
    json_reader.Parse(*json_reader_stream, first_timestamp_handler);

    if (first_timestamp_handler.timestamp.empty()) {
      continue;
    }

    file_info->first_timestamp = first_timestamp_handler.timestamp;

    try {
      auto ts = LogFileTimestamp(log_name);
      m_timestamp_to_file_map.emplace(std::move(ts), std::move(file_info));
    } catch (const std::exception &e) {
      LogComponentErr(WARNING_LEVEL, ER_AUDIT_LOG_FILE_NAME_PARSE_FAILURE,
                      log_name.c_str(), e.what());
    } catch (...) {
      LogComponentErr(WARNING_LEVEL,
                      ER_AUDIT_LOG_FILE_NAME_UNKNOWN_PARSE_FAILURE,
                      log_name.c_str());
    }
  }

  for (const auto &log_name : removed_files) {
    auto it = std::find_if(
        std::cbegin(m_timestamp_to_file_map),
        std::cend(m_timestamp_to_file_map),
        [log_name](const auto &pair) { return pair.second->name == log_name; });

    if (it != std::cend(m_timestamp_to_file_map)) {
      m_timestamp_to_file_map.erase(it);
    }
  }

  if (!have_complete_file_snapshot) {
    return false;
  }

  m_reload_requested = false;
  return true;
}

bool AuditLogReader::read(AuditLogReaderContext *reader_context) noexcept {
  std::shared_lock lock(m_reader_mutex);

  if (m_reload_requested) {
    return false;
  }

  reader_context->is_batch_end = false;
  reader_context->audit_json_handler->iterative_parse_init();

  while (!reader_context->is_batch_end) {
    if (reader_context->current_file == nullptr) {
      if (reader_context->files_to_read.empty()) {
        reader_context->is_session_end = true;
        reader_context->audit_json_handler->iterative_parse_close(true);
        return true;
      }

      reader_context->current_file = reader_context->files_to_read.front();
      reader_context->files_to_read.pop_front();

      if (!reader_context->audit_json_read_stream->open(
              reader_context->current_file)) {
        return false;
      }

      if (reader_context->reader == nullptr) {
        reader_context->reader = std::make_unique<rapidjson::Reader>();
      }

      reader_context->reader->IterativeParseInit();
    } else if (reader_context->reader == nullptr) {
      /*
        If the previous call to read() failed to open the file (returned false),
        current_file would be set but the reader not initialized.
      */
      reader_context->reader = std::make_unique<rapidjson::Reader>();
      reader_context->reader->IterativeParseInit();
    }

    while (!reader_context->reader->IterativeParseComplete()) {
      reader_context->reader->IterativeParseNext<rapidjson::kParseDefaultFlags>(
          *reader_context->audit_json_read_stream,
          *reader_context->audit_json_handler);

      // We don't want reading to fail because of EOF during parsing.
      // This would break reading of currently open log file, as its
      // top-level array isn't closed yet.
      if (reader_context->audit_json_read_stream->check_eof_reached()) {
        break;
      }

      if (reader_context->reader->HasParseError()) {
        return false;
      }

      if (reader_context->is_batch_end) {
        break;
      }
    }

    if (reader_context->audit_json_read_stream->check_eof_reached()) {
      reader_context->audit_json_read_stream->close();
      reader_context->current_file = nullptr;
    }
  }

  reader_context->audit_json_handler->iterative_parse_close(false);

  return true;
}

AuditLogReaderContext *AuditLogReader::init_reader_session(
    MYSQL_THD thd, const AuditLogReaderArgs *reader_args) noexcept {
  if (m_reload_requested && !init()) {
    return nullptr;
  }

  std::shared_lock lock(m_reader_mutex);
  auto reader_context = std::make_unique<AuditLogReaderContext>();

  if (reader_context == nullptr) {
    return nullptr;
  }

  reader_context->next_event_bookmark.timestamp = reader_args->timestamp;
  reader_context->next_event_bookmark.id = reader_args->id;
  set_files_to_read_list(reader_context.get());

  auto read_buff_size = SysVars::get_read_buffer_size(thd);
  auto read_buff = std::unique_ptr<char, std::function<void(char *)>>(
      static_cast<char *>(my_malloc(key_memory_audit_log_filter_read_buffer,
                                    read_buff_size, MY_ZEROFILL)),
      [](char *buff) { my_free(buff); });

  if (read_buff == nullptr) {
    return nullptr;
  }

  auto json_handler = std::make_unique<json_reader::AuditJsonHandler>(
      reader_context.get(), std::move(read_buff), read_buff_size);
  auto json_reader_stream =
      std::make_unique<json_reader::AuditJsonReadStream>();

  if (!json_reader_stream->init()) {
    return nullptr;
  }

  reader_context->audit_json_handler.swap(json_handler);
  reader_context->audit_json_read_stream.swap(json_reader_stream);

  return reader_context.release();
}

void AuditLogReader::close_reader_session(AuditLogReaderContext *reader_context
                                          [[maybe_unused]]) noexcept {}

}  // namespace audit_log_filter
