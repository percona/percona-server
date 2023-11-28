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

#ifndef AUDIT_LOG_FILTER_AUDIT_LOG_JSON_HANDLER_H_INCLUDED
#define AUDIT_LOG_FILTER_AUDIT_LOG_JSON_HANDLER_H_INCLUDED

#include "components/audit_log_filter/sys_vars.h"

#include "my_rapidjson_size_t.h"
#include "rapidjson/document.h"
#include "rapidjson/reader.h"

#include <sstream>

namespace audit_log_filter {

struct AuditLogReaderContext;

namespace json_reader {

class AuditJsonHandler
    : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, AuditJsonHandler> {
 public:
  AuditJsonHandler(AuditLogReaderContext *reader_context,
                   std::unique_ptr<char, std::function<void(char *)>> out_buff,
                   ulong out_buff_size);

  char *get_result_buffer_ptr() noexcept;

  /**
   * @brief Prepare handler to process next chunk of records
   *        (reset buffer and printed out records counter).
   */
  void iterative_parse_init() noexcept;

  /**
   * @brief Finish processing chunk of data which is ready to be printed out
   *        (adds JSON array closing tag to the buffer end)
   *
   * @param with_null_tag Indicates if terminating 'null' element should be
   *                      added to JSON array.
   */
  void iterative_parse_close(bool with_null_tag) noexcept;

  bool Null();
  bool Bool(bool value);
  bool Int(int value);
  bool Uint(unsigned value);
  bool Int64(int64_t value);
  bool Uint64(uint64_t value);
  bool Double(double value);
  bool String(const char *value, rapidjson::SizeType length, bool copy);
  bool StartObject();
  bool Key(const char *str, rapidjson::SizeType length, bool copy);
  bool EndObject(rapidjson::SizeType memberCount);
  bool StartArray();
  bool EndArray(rapidjson::SizeType elementCount);

 private:
  void clear_current_event();
  bool check_reading_start_reached();
  void update_bookmark(uint64_t id);
  void update_bookmark(const std::string &timestamp);
  void write_out_buff(const char *str, std::size_t str_length);

 private:
  AuditLogReaderContext *m_reader_context;

  rapidjson::Document m_json_value;
  std::string m_current_key_name;
  std::stringstream m_event_str;
  int m_obj_level;
  int m_arr_level;

  std::unique_ptr<char, std::function<void(char *)>> m_out_buff;
  char *m_current_buff;
  ulong m_out_buff_size;
  ulong m_used_buff_size;
  ulong m_printed_events_count;
  bool m_reading_start_reached;

  LogBookmark m_current_event_bookmark;
};

}  // namespace json_reader
}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_AUDIT_LOG_JSON_HANDLER_H_INCLUDED
