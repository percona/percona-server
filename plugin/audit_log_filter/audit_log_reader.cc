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

#include "plugin/audit_log_filter/audit_log_reader.h"

#include "plugin/audit_log_filter/audit_keyring.h"
#include "plugin/audit_log_filter/audit_psi_info.h"

#include "rapidjson/document.h"
#include "rapidjson/reader.h"

#include <mysql/components/my_service.h>
#include <mysql/components/services/mysql_current_thread_reader.h>

#include <scope_guard.h>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace audit_log_filter {

void AuditLogReader::set_files_to_read_list(
    AuditLogReaderContext *const reader_context) noexcept {
  if (reader_context == nullptr) {
    return;
  }

  std::vector<std::string> tp_list;

  for (const auto &item : m_first_timestamp_to_file_map) {
    if (item.first >= reader_context->next_event_bookmark.timestamp) {
      auto *file_info = item.second.get();

      if (file_info->is_encrypted && file_info->encryption_options == nullptr) {
        continue;
      }

      reader_context->files_to_read.push_back(file_info);
    }
  }
}

bool AuditLogReader::init() noexcept {
  if (SysVars::get_format_type() != AuditLogFormatType::Json) {
    // Not supported for other log formats
    return true;
  }

  my_service<SERVICE_TYPE(mysql_current_thread_reader)> thd_reader_srv(
      "mysql_current_thread_reader", SysVars::get_comp_regystry_srv());

  MYSQL_THD thd;

  if (thd_reader_srv->get(&thd)) {
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

  m_first_timestamp_to_file_map.clear();

  for (const auto &entry :
       std::filesystem::directory_iterator{SysVars::get_file_dir()}) {
    auto log_name = entry.path().filename().string();

    if (entry.is_regular_file() &&
        log_name.find(log_base_file_name) != std::string::npos) {
      if (std::any_of(m_first_timestamp_to_file_map.cbegin(),
                      m_first_timestamp_to_file_map.cend(),
                      [&log_name](const auto &entry) {
                        return entry.second->name == log_name;
                      })) {
        continue;
      }

      bool is_compressed = log_name.find(".gz") != std::string::npos;
      bool is_encrypted = log_name.find(".enc") != std::string::npos;
      auto encryption_options_id =
          audit_keyring::get_options_id_for_file_name(log_name);

      assert(!is_encrypted || !encryption_options_id.empty());

      auto file_info = std::make_unique<FileInfo>(
          log_name, encryption_options_id, is_compressed, is_encrypted);

      if (file_info->is_encrypted &&
          !file_info->encryption_options_id.empty()) {
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

      rapidjson::Document json_doc;
      json_doc.ParseStream(*json_reader_stream);

      if (json_doc.HasParseError() || json_doc.Empty() || !json_doc.IsArray() ||
          json_doc.GetArray().Empty()) {
        continue;
      }

      auto first_event = json_doc.GetArray().Begin();

      if (!first_event->IsObject() || !first_event->HasMember("timestamp") ||
          !first_event->GetObject()["timestamp"].IsString()) {
        continue;
      }

      m_first_timestamp_to_file_map.emplace(
          first_event->GetObject()["timestamp"].GetString(),
          std::move(file_info));
    }
  }

  return true;
}

bool AuditLogReader::read(AuditLogReaderContext *reader_context) noexcept {
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
    }

    rapidjson::Reader reader;
    reader.IterativeParseInit();

    while (!reader.IterativeParseComplete()) {
      reader.IterativeParseNext<rapidjson::kParseDefaultFlags>(
          *reader_context->audit_json_read_stream,
          *reader_context->audit_json_handler);
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
