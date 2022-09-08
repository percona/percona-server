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

#include "plugin/audit_log_filter/audit_log_json_handler.h"
#include "plugin/audit_log_filter/audit_psi_info.h"
#include "plugin/audit_log_filter/sys_vars.h"

#include "sql/mysqld.h"

#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"

#include <mysql/components/my_service.h>
#include <mysql/components/services/mysql_current_thread_reader.h>

#include <cstdio>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace audit_log_filter {

AuditLogReader::AuditLogReader(comp_registry_srv_t *comp_registry_srv)
    : m_comp_registry_srv{comp_registry_srv} {}

auto AuditLogReader::get_log_file_handle(
    AuditLogReaderContext *reader_context) const noexcept {
  std::vector<std::string> tp_list;

  for (const auto &item : m_first_timestamp_to_file_map) {
    if (reader_context->next_event_bookmark.timestamp >= item.first) {
      tp_list.push_back(item.first);
    }
  }

  std::string log_file_name;

  if (tp_list.empty()) {
    log_file_name = SysVars::get_file_name();
  } else {
    auto log_timepoint = std::max_element(tp_list.cbegin(), tp_list.cend());
    log_file_name = m_first_timestamp_to_file_map.find(*log_timepoint)->second;
  }

  return std::unique_ptr<std::FILE, std::function<void(std::FILE *)>>(
      fopen(log_file_name.c_str(), "r"), [](std::FILE *fh) { fclose(fh); });
}

bool AuditLogReader::init() noexcept {
  if (SysVars::get_format_type() != AuditLogFormatType::Json) {
    // Not supported for other log formats
    return true;
  }

  my_service<SERVICE_TYPE(mysql_current_thread_reader)> thd_reader_srv(
      "mysql_current_thread_reader", m_comp_registry_srv);

  MYSQL_THD thd;

  if (thd_reader_srv->get(&thd)) {
    return false;
  }

  auto read_buf_size = SysVars::get_read_buffer_size(thd);
  char *read_buf = static_cast<char *>(my_malloc(
      key_memory_audit_log_filter_read_buffer, read_buf_size, MY_ZEROFILL));

  if (read_buf == nullptr) {
    return false;
  }

  auto log_dir_name = mysql_data_home;
  const auto log_current_file_name = SysVars::get_file_name();
  auto log_base_file_name = std::filesystem::path{log_current_file_name};
  log_base_file_name.replace_extension();

  for (const auto &entry : std::filesystem::directory_iterator{log_dir_name}) {
    const auto log_name = entry.path().filename().string();

    if (entry.is_regular_file() &&
        log_name.find(log_base_file_name) != std::string::npos) {
      if (std::any_of(m_first_timestamp_to_file_map.cbegin(),
                      m_first_timestamp_to_file_map.cend(),
                      [&log_name](const auto &entry) {
                        return entry.second == log_name;
                      })) {
        continue;
      }

      FILE *fp = fopen(log_name.c_str(), "r");

      if (fp == nullptr) {
        continue;
      }

      rapidjson::FileReadStream json_stream(fp, read_buf, read_buf_size);
      rapidjson::Document json_doc;
      json_doc.ParseStream(json_stream);

      if (json_doc.HasParseError() || !json_doc.IsArray() || json_doc.Empty()) {
        fclose(fp);
        continue;
      }

      auto first_event = json_doc.GetArray().Begin();

      if (!first_event->IsObject() || !first_event->HasMember("timestamp") ||
          !first_event->GetObject()["timestamp"].IsString()) {
        fclose(fp);
        continue;
      }

      m_first_timestamp_to_file_map.emplace(
          first_event->GetObject()["timestamp"].GetString(), log_name.c_str());

      fclose(fp);
    }
  }

  my_free(read_buf);

  return true;
}

bool AuditLogReader::read(const AuditLogReaderArgs &reader_args,
                          AuditLogReaderContext *reader_context, char *out_buff,
                          ulong out_buff_size) noexcept {
  auto log_file_handle = get_log_file_handle(reader_context);

  if (log_file_handle == nullptr) {
    return false;
  }

  auto read_buff = std::unique_ptr<char, std::function<void(char *)>>(
      static_cast<char *>(my_malloc(key_memory_audit_log_filter_read_buffer,
                                    out_buff_size, MY_ZEROFILL)),
      [](char *buff) { my_free(buff); });

  if (read_buff == nullptr) {
    return false;
  }

  AuditJsonHandler handler{reader_args, reader_context, out_buff,
                           out_buff_size};
  rapidjson::FileReadStream json_stream{log_file_handle.get(), read_buff.get(),
                                        out_buff_size};
  rapidjson::Reader reader;
  reader.Parse(json_stream, handler);

  return true;
}

AuditLogReaderContext *AuditLogReader::init_reader_session(
    const AuditLogReaderArgs &reader_args) noexcept {
  auto *reader_context = new AuditLogReaderContext{};
  reader_context->next_event_bookmark.timestamp = reader_args.timestamp;
  reader_context->next_event_bookmark.id = reader_args.id;

  return reader_context;
}

void AuditLogReader::close_reader_session(AuditLogReaderContext *reader_context
                                          [[maybe_unused]]) noexcept {}

}  // namespace audit_log_filter
