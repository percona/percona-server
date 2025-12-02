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

#include "components/audit_log_filter/json_reader/audit_json_handler.h"
#include <cassert>
#include <cstring>  // std::strcpy
#include "components/audit_log_filter/audit_log_reader.h"

namespace audit_log_filter::json_reader {

namespace {

const std::string kJsonArrayOpenTag = "[\n";
const std::string kJsonArrayCloseTag = "\n]\n";
const std::string kJsonArrayCloseWithNullTag = "null\n]\n";
const auto kBufferReservedSize =
    kJsonArrayCloseTag.length() + kJsonArrayCloseWithNullTag.length();

}  // namespace

AuditJsonHandler::AuditJsonHandler(
    AuditLogReaderContext *reader_context,
    std::unique_ptr<char, std::function<void(char *)>> out_buff,
    ulong out_buff_size)
    : m_reader_context{reader_context},
      m_obj_level{0},
      m_arr_level{0},
      m_out_buff{std::move(out_buff)},
      m_current_buff{m_out_buff.get()},
      m_out_buff_size{out_buff_size},
      m_used_buff_size{0},
      m_printed_events_count{0},
      m_reading_start_reached{false},
      m_is_first_field{false} {}

char *AuditJsonHandler::get_result_buffer_ptr() noexcept {
  return m_out_buff.get();
}

void AuditJsonHandler::iterative_parse_init() noexcept {
  m_current_buff = m_out_buff.get();
  m_used_buff_size = 0;
  m_printed_events_count = 0;

  write_out_buff(kJsonArrayOpenTag.c_str(), kJsonArrayOpenTag.length());

  if (!m_event_str.str().empty()) {
    // Print leftovers from previous batch if any
    write_out_buff(m_event_str.str().c_str(), m_event_str.str().length());
    ++m_printed_events_count;
    clear_current_event();
  }
}

void AuditJsonHandler::iterative_parse_close(bool with_null_tag) noexcept {
  // Remove the trailing ",\n" from the last event, if present,
  // to ensure valid JSON array closing.
  if (m_used_buff_size >= 2 && (m_current_buff - 2)[0] == ',' &&
      (m_current_buff - 1)[0] == '\n' && !with_null_tag) {
    m_current_buff -= 2;
    m_used_buff_size -= 2;
  }

  const auto &closing_tag =
      with_null_tag ? kJsonArrayCloseWithNullTag : kJsonArrayCloseTag;
  write_out_buff(closing_tag.c_str(), closing_tag.length());
}

// --- Value Handlers ---

bool AuditJsonHandler::Null() { return true; }

bool AuditJsonHandler::Bool(bool value) {
  m_event_str << (value ? "true" : "false");
  return true;
}

bool AuditJsonHandler::Int(int value) {
  update_bookmark(static_cast<uint64_t>(value));
  m_event_str << value;
  return true;
}

bool AuditJsonHandler::Uint(unsigned value) {
  update_bookmark(static_cast<uint64_t>(value));
  m_event_str << value;
  return true;
}

bool AuditJsonHandler::Int64(int64_t value) {
  update_bookmark(static_cast<uint64_t>(value));
  m_event_str << value;
  return true;
}

bool AuditJsonHandler::Uint64(uint64_t value) {
  update_bookmark(value);
  m_event_str << value;
  return true;
}

bool AuditJsonHandler::Double(double value) {
  m_event_str << value;
  return true;
}

bool AuditJsonHandler::String(const char *value, rapidjson::SizeType length,
                              bool copy [[maybe_unused]]) {
  std::string s_value(value, length);
  update_bookmark(s_value);
  m_event_str << "\"" << s_value << "\"";
  return true;
}

// --- Structure Handlers ---

bool AuditJsonHandler::StartObject() {
  ++m_obj_level;
  m_event_str << "{";
  // Reset flag: the first key encountered will not have a comma separator
  m_is_first_field = true;
  return true;
}

bool AuditJsonHandler::Key(const char *str, rapidjson::SizeType length,
                           bool copy [[maybe_unused]]) {
  m_current_key_name.assign(str, length);

  // Manage comma separation and state before appending the key.
  if (m_is_first_field) {
    m_is_first_field = false;  // This is the first field, so unset the flag
  } else {
    m_event_str << ", ";  // Prepend separator if a previous field exists
  }

  // Append key and colon
  m_event_str << "\"" << str << "\": ";
  return true;
}

bool AuditJsonHandler::EndObject(rapidjson::SizeType memberCount
                                 [[maybe_unused]]) {
  if (m_obj_level > 0) {
    --m_obj_level;
  }

  // Append the closing brace.
  m_event_str << "}";

  // If closing an inner object (m_obj_level is now > 0), or the top-level
  // object, ensure the m_is_first_field is false, so the next Key() call
  // (if any, in the parent) correctly prepends a comma.
  m_is_first_field = false;

  // Handle the completed top-level event object.
  if (m_obj_level == 0) {
    if (!check_reading_start_reached()) {
      clear_current_event();
      return true;
    }

    // Add the inter-event separator (comma and newline).
    m_event_str << ",\n";

    const auto event_length = m_event_str.str().length();
    const auto max_array_length =
        m_reader_context->batch_reader_args->max_array_length;

    if ((m_used_buff_size + event_length >=
         m_out_buff_size - kBufferReservedSize) ||
        (max_array_length != 0 && m_printed_events_count == max_array_length)) {
      m_reader_context->next_event_bookmark = m_current_event_bookmark;
      m_reader_context->is_batch_end = true;
      return true;
    }

    write_out_buff(m_event_str.str().c_str(), event_length);
    ++m_printed_events_count;
    clear_current_event();
  }

  return true;
}

bool AuditJsonHandler::StartArray() {
  // If we ever to support such arrays (i.e. non top-level) we need to rethink
  // and change comma separation logic between arrays and objects elements.
  assert(m_arr_level == 0);
  ++m_arr_level;
  return true;
}

bool AuditJsonHandler::EndArray(rapidjson::SizeType elementCount
                                [[maybe_unused]]) {
  if (m_arr_level > 0) {
    --m_arr_level;
  }
  return true;
}

void AuditJsonHandler::clear_current_event() { m_event_str.str(std::string()); }

bool AuditJsonHandler::check_reading_start_reached() {
  if (!m_reading_start_reached) {
    m_reading_start_reached = m_reader_context->next_event_bookmark.timestamp ==
                                  m_current_event_bookmark.timestamp &&
                              (m_reader_context->next_event_bookmark.id == 0 ||
                               m_reader_context->next_event_bookmark.id ==
                                   m_current_event_bookmark.id);
  }

  return m_reading_start_reached;
}

void AuditJsonHandler::update_bookmark(uint64_t id) {
  if (!m_current_key_name.empty() && m_current_key_name == "id") {
    m_current_event_bookmark.id = id;
  }
}

void AuditJsonHandler::update_bookmark(const std::string &timestamp) {
  if (!m_current_key_name.empty() && m_current_key_name == "timestamp") {
    m_current_event_bookmark.timestamp = timestamp;
  }
}

void AuditJsonHandler::write_out_buff(const char *str, std::size_t str_length) {
  std::strcpy(m_current_buff, str);
  m_current_buff += str_length;
  m_used_buff_size += str_length;
}

}  // namespace audit_log_filter::json_reader
