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
#include <algorithm>
#include <cassert>
#include <cstring>
#include <limits>
#include <new>
#include <stdexcept>
#include "components/audit_log_filter/audit_log_reader.h"
#include "components/audit_log_filter/audit_psi_info.h"
#include "my_dbug.h"
#include "my_sys.h"
#ifdef MYSQL_COMPONENT
#include "mysql/components/library_mysys/my_memory.h"
#else
#include "mysql/service_mysql_alloc.h"
#endif

namespace audit_log_filter::json_reader {

namespace {

const std::string kJsonArrayOpenTag = "[\n";
const std::string kJsonArrayCloseTag = "\n]\n";
const std::string kJsonArrayCloseWithNullTag = "null\n]\n";
const auto kBufferReservedSize =
    kJsonArrayCloseTag.length() + kJsonArrayCloseWithNullTag.length();

// SAX values are already unescaped. Re-encode them when constructing the UDF's
// JSON result, including embedded NULs and non-ASCII query text.
//
// The output is the same as rapidjson::Writer<StringBuffer>::String(): quote
// and backslash are escaped, \b \f \n \r \t use their short forms, any other
// byte below 0x20 becomes \u00XX, and every other byte (UTF-8 included) is
// copied. Using the Writer here makes GCC 16 report a false
// -Wstringop-overflow inside rapidjson's StringBuffer in optimized builds.
std::string json_string(const char *value, size_t length) {
  static constexpr char kHexDigits[] = "0123456789ABCDEF";

  std::string out;
  out.reserve(length + 2);
  out.push_back('"');

  for (size_t i = 0; i < length; ++i) {
    const auto c = static_cast<unsigned char>(value[i]);
    switch (c) {
      case '"':
        out.append("\\\"");
        break;
      case '\\':
        out.append("\\\\");
        break;
      case '\b':
        out.append("\\b");
        break;
      case '\f':
        out.append("\\f");
        break;
      case '\n':
        out.append("\\n");
        break;
      case '\r':
        out.append("\\r");
        break;
      case '\t':
        out.append("\\t");
        break;
      default:
        if (c < 0x20) {
          const char escaped[] = {
              '\\', 'u', '0', '0', kHexDigits[c >> 4], kHexDigits[c & 0x0F]};
          out.append(escaped, sizeof(escaped));
        } else {
          out.push_back(static_cast<char>(c));
        }
    }
  }

  out.push_back('"');
  return out;
}

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
      m_batch_size{out_buff_size},
      m_out_buff_size{out_buff_size},
      m_used_buff_size{0},
      m_printed_events_count{0},
      m_reading_start_reached{false} {}

char *AuditJsonHandler::get_result_buffer_ptr() noexcept {
  return m_out_buff.get();
}

void AuditJsonHandler::iterative_parse_init() {
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

void AuditJsonHandler::iterative_parse_close(bool with_null_tag) {
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

bool AuditJsonHandler::Null() {
  before_value();
  m_event_str << "null";
  return true;
}

bool AuditJsonHandler::Bool(bool value) {
  before_value();
  m_event_str << (value ? "true" : "false");
  return true;
}

bool AuditJsonHandler::Int(int value) {
  update_bookmark(static_cast<uint64_t>(value));
  before_value();
  m_event_str << value;
  return true;
}

bool AuditJsonHandler::Uint(unsigned value) {
  update_bookmark(static_cast<uint64_t>(value));
  before_value();
  m_event_str << value;
  return true;
}

bool AuditJsonHandler::Int64(int64_t value) {
  update_bookmark(static_cast<uint64_t>(value));
  before_value();
  m_event_str << value;
  return true;
}

bool AuditJsonHandler::Uint64(uint64_t value) {
  update_bookmark(value);
  before_value();
  m_event_str << value;
  return true;
}

bool AuditJsonHandler::Double(double value) {
  before_value();
  m_event_str << value;
  return true;
}

bool AuditJsonHandler::String(const char *value, rapidjson::SizeType length,
                              bool copy [[maybe_unused]]) {
  std::string s_value(value, length);
  update_bookmark(s_value);
  before_value();
  m_event_str << json_string(value, length);
  return true;
}

// --- Structure Handlers ---

bool AuditJsonHandler::StartObject() {
  before_value();
  ++m_obj_level;
  m_event_str << "{";
  m_context_stack.push_back({ContainerType::Object, true});
  return true;
}

bool AuditJsonHandler::Key(const char *str, rapidjson::SizeType length,
                           bool copy [[maybe_unused]]) {
  m_current_key_name.assign(str, length);
  assert(!m_context_stack.empty());
  assert(m_context_stack.back().type == ContainerType::Object);

  auto &context = m_context_stack.back();
  if (context.is_first_element) {
    context.is_first_element = false;
  } else {
    m_event_str << ", ";
  }

  m_event_str << json_string(str, length) << ": ";
  return true;
}

bool AuditJsonHandler::EndObject(rapidjson::SizeType memberCount
                                 [[maybe_unused]]) {
  if (m_obj_level > 0) {
    --m_obj_level;
  }

  m_event_str << "}";
  assert(!m_context_stack.empty());
  assert(m_context_stack.back().type == ContainerType::Object);
  m_context_stack.pop_back();

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

    // A single event may exceed the configured batch size after conversion or
    // JSON escaping. Return it whole, growing the output buffer if necessary,
    // rather than producing an empty batch or overflowing on the next call.
    // Keep subsequent batches bounded by the original configured size.
    const bool exceeds_batch =
        event_length >= m_batch_size ||
        m_used_buff_size >= m_batch_size - event_length ||
        kBufferReservedSize >= m_batch_size - event_length - m_used_buff_size;
    if ((m_printed_events_count != 0 && exceeds_batch) ||
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
  const bool is_top_level_array = (m_arr_level == 0 && m_obj_level == 0);

  if (!is_top_level_array) {
    before_value();
    m_event_str << "[";
    m_context_stack.push_back({ContainerType::Array, true});
  }

  ++m_arr_level;
  return true;
}

bool AuditJsonHandler::EndArray(rapidjson::SizeType elementCount
                                [[maybe_unused]]) {
  if (m_arr_level > 0) {
    if (!(m_arr_level == 1 && m_obj_level == 0)) {
      m_event_str << "]";
      assert(!m_context_stack.empty());
      assert(m_context_stack.back().type == ContainerType::Array);
      m_context_stack.pop_back();
    }

    --m_arr_level;
  }
  return true;
}

void AuditJsonHandler::before_value() {
  if (m_context_stack.empty()) {
    return;
  }

  auto &context = m_context_stack.back();
  if (context.type != ContainerType::Array) {
    return;
  }

  if (context.is_first_element) {
    context.is_first_element = false;
  } else {
    m_event_str << ", ";
  }
}

void AuditJsonHandler::clear_current_event() {
  m_event_str.str(std::string());
  m_event_str.clear();
  m_current_key_name.clear();
  m_current_event_bookmark = {};
}

bool AuditJsonHandler::check_reading_start_reached() {
  if (!m_reading_start_reached) {
    switch (m_reader_context->batch_reader_args->command) {
      case AuditLogReaderArgs::Command::ReadFromBookmark:
        m_reading_start_reached =
            m_reader_context->next_event_bookmark == m_current_event_bookmark;
        break;
      case AuditLogReaderArgs::Command::ReadFromTimestamp:
        m_reading_start_reached =
            m_reader_context->next_event_bookmark.timestamp <=
            m_current_event_bookmark.timestamp;
        break;
      default:
        assert(false);
    }
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
  if (str_length >= std::numeric_limits<size_t>::max() - m_used_buff_size) {
    throw std::length_error("Audit reader output is too large");
  }
  const size_t required = m_used_buff_size + str_length + 1;
  if (required > m_out_buff_size) {
    const size_t capacity =
        m_out_buff_size <= std::numeric_limits<size_t>::max() / 2
            ? std::max(required, m_out_buff_size * 2)
            : required;
    std::unique_ptr<char, std::function<void(char *)>> buffer(
        nullptr, [](char *p) { my_free(p); });
    DBUG_EXECUTE_IF("audit_log_filter_reader_grow_bad_alloc",
                    throw std::bad_alloc(););
    buffer.reset(static_cast<char *>(
        my_malloc(key_memory_audit_log_filter_read_buffer, capacity, MYF(0))));
    if (!buffer) throw std::bad_alloc();
    std::memcpy(buffer.get(), m_out_buff.get(), m_used_buff_size);
    m_out_buff.swap(buffer);
    m_out_buff_size = capacity;
    m_current_buff = m_out_buff.get() + m_used_buff_size;
  }
  std::memcpy(m_current_buff, str, str_length);
  m_current_buff += str_length;
  m_used_buff_size += str_length;
  *m_current_buff = '\0';
}

}  // namespace audit_log_filter::json_reader
