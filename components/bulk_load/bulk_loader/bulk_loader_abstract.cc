/* Copyright (c) 2024, Percona and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation. The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA */

#include "bulk_loader_abstract.h"

#include "components/bulk_load/stream_parser/stream_parser.h"

#include <mysql/components/services/log_builtins.h>
#include <mysqld_error.h>

#include "scope_guard.h"

#include <iostream>
#include <utility>

namespace Bulk_load {
namespace {

constexpr size_t default_loader_memory_bytes = 1024 * 1024 * 1024;
constexpr size_t default_concurrency = 1;  // TODO: 16

}  // namespace

Bulk_loader_impl::Bulk_loader_impl(THD *thd, my_thread_id connection_id,
                                   const TABLE *table,
                                   const CHARSET_INFO *charset)
    : m_thd{thd}
    , m_connection_id{connection_id}
    , m_table{table}
    , m_charset{charset}
    , m_compression_algorithm{Bulk_compression_algorithm::NONE} {}

THD *Bulk_loader_impl::get_thd() noexcept {
  return m_thd;
}

my_thread_id Bulk_loader_impl::get_connection_id() const noexcept {
  return m_connection_id;
}

const TABLE *Bulk_loader_impl::get_table() noexcept {
  return m_table;
}

const CHARSET_INFO *Bulk_loader_impl::get_charset() noexcept {
  return m_charset;
}

void Bulk_loader_impl::set_string(
    Bulk_string type, std::string value) noexcept {
  m_string_attrs[type] = std::move(value);
}

void Bulk_loader_impl::set_char(Bulk_char type, unsigned char value) noexcept {
  m_char_attrs[type] = value;
}

void Bulk_loader_impl::set_size(Bulk_size type, size_t value) noexcept {
  m_size_attrs[type] = value;
}

void Bulk_loader_impl::set_condition(Bulk_condition type, bool value) noexcept {
  m_condition_attrs[type] = value;
}

std::string Bulk_loader_impl::get_string(Bulk_string type) const noexcept {
   const auto it = m_string_attrs.find(type);
   return it != m_string_attrs.cend() ? it->second : "";
}

unsigned char Bulk_loader_impl::get_char(Bulk_char type) const noexcept {
  const auto it = m_char_attrs.find(type);
  return it != m_char_attrs.cend() ? it->second : '\0';
}

size_t Bulk_loader_impl::get_size(Bulk_size type) const noexcept {
  const auto it = m_size_attrs.find(type);
  return it != m_size_attrs.cend() ? it->second : 0;
}

bool Bulk_loader_impl::get_condition(Bulk_condition type) const noexcept {
  const auto it = m_condition_attrs.find(type);
  return it != m_condition_attrs.cend() ? it->second : false;
}

void Bulk_loader_impl::set_compression_algorithm(
    Bulk_compression_algorithm algorithm) noexcept {
  m_compression_algorithm = algorithm;
}

Bulk_compression_algorithm
Bulk_loader_impl::get_compression_algorithm() const noexcept {
  return m_compression_algorithm;
}

bool Bulk_loader_impl::acquire_services() noexcept {
  m_srv_registry = mysql_plugin_registry_acquire();

  if (m_srv_registry == nullptr) {
    LogComponentErr(ERROR_LEVEL, ER_BULK_LOADER_COMPONENT_ERROR,
                    "Cannot acquire registry");
    return false;
  }

  if (m_srv_registry->acquire("bulk_data_convert", &m_svc_data_convert) != 0) {
    LogComponentErr(ERROR_LEVEL, ER_BULK_LOADER_COMPONENT_ERROR,
                    "Cannot acquire data service");
    return false;
  }

  if (m_srv_registry->acquire("bulk_data_load", &m_svc_data_load) != 0) {
    LogComponentErr(ERROR_LEVEL, ER_BULK_LOADER_COMPONENT_ERROR,
                    "Cannot acquire load service");
    return false;
  }

  return true;
}

void Bulk_loader_impl::release_services() noexcept {
  if (m_srv_registry != nullptr) {
    if (m_svc_data_convert != nullptr &&
        m_srv_registry->release(m_svc_data_convert) != 0) {
      LogComponentErr(ERROR_LEVEL, ER_BULK_LOADER_COMPONENT_ERROR,
                      "Cannot release data service");
    }

    if (m_svc_data_load != nullptr &&
        m_srv_registry->release(m_svc_data_load) != 0) {
      LogComponentErr(ERROR_LEVEL, ER_BULK_LOADER_COMPONENT_ERROR,
                      "Cannot release load service");
    }

    if (mysql_plugin_registry_release(m_srv_registry) != 0) {
      LogComponentErr(ERROR_LEVEL, ER_BULK_LOADER_COMPONENT_ERROR,
                      "Cannot release registry");
    }
  }
}

SERVICE_TYPE(bulk_data_convert)
*Bulk_loader_impl::data_convert_service() noexcept {
  return reinterpret_cast<SERVICE_TYPE(bulk_data_convert) *>(
      m_svc_data_convert);
}

SERVICE_TYPE(bulk_data_load) *Bulk_loader_impl::data_load_service() noexcept {
  return reinterpret_cast<SERVICE_TYPE(bulk_data_load) *>(m_svc_data_load);
}

void *Bulk_loader_impl::start_session(
    size_t data_size, size_t se_memory_size, size_t num_threads) noexcept {
  m_load_session_start_done = true;
  auto *load_ctx = data_load_service()->begin(
      m_thd, m_table, data_size, se_memory_size, num_threads);

  if (load_ctx == nullptr) {
    LogComponentErr(ERROR_LEVEL, ER_BULK_LOADER_COMPONENT_ERROR,
                    "Failed to start session");
  }

  return load_ctx;
}

void Bulk_loader_impl::end_session(void *load_ctx) noexcept {
  if (!m_load_session_start_done) {
    return;
  }

  if (!data_load_service()->end(m_thd, load_ctx,
                                m_table, load_ctx == nullptr)) {
    LogComponentErr(ERROR_LEVEL, ER_BULK_LOADER_COMPONENT_ERROR,
                    "Failed to end session");
  }

  m_load_session_start_done = false;
}

ParserParamsPtr Bulk_loader_impl::get_parser_params() const noexcept {
  return std::make_unique<ParserParams>(
      get_string(Bulk_string::COLUMN_TERM),
      get_string(Bulk_string::ROW_TERM),
      get_char(Bulk_char::ESCAPE_CHAR),
      get_char(Bulk_char::ENCLOSE_CHAR),
      get_size(Bulk_size::COUNT_ROW_SKIP)
  );
}

void Bulk_loader_impl::settings_dump_to_log() {
  std::cout << "Bulk_loader_impl settings =====================================" << std::endl;
  std::cout << "schema_name: " << get_string(Bulk_string::SCHEMA_NAME) << std::endl;
  std::cout << "table_name: " << get_string(Bulk_string::TABLE_NAME) << std::endl;
  std::cout << "file_prefix: " << get_string(Bulk_string::FILE_PREFIX) << std::endl;
  std::cout << "file_suffix: " << get_string(Bulk_string::FILE_SUFFIX) << std::endl;
  std::cout << "column_term: " << get_string(Bulk_string::COLUMN_TERM) << std::endl;
  std::cout << "row_term: " << get_string(Bulk_string::ROW_TERM) << std::endl;
  std::cout << "append_to_last_prefix: " << get_string(Bulk_string::APPENDTOLASTPREFIX) << std::endl;
  std::cout << "escape_char: " << get_char(Bulk_char::ESCAPE_CHAR) << std::endl;
  std::cout << "enclose_char: " << get_char(Bulk_char::ENCLOSE_CHAR) << std::endl;
  std::cout << "count_files: " << get_size(Bulk_size::COUNT_FILES) << std::endl;
  std::cout << "count_row_skip: " << get_size(Bulk_size::COUNT_ROW_SKIP) << std::endl;
  std::cout << "count_columns: " << get_size(Bulk_size::COUNT_COLUMNS) << std::endl;
  std::cout << "concurrency: " << get_size(Bulk_size::CONCURRENCY) << std::endl;
  std::cout << "memory: " << get_size(Bulk_size::MEMORY) << std::endl;
  std::cout << "start_index: " << get_size(Bulk_size::START_INDEX) << std::endl;
  std::cout << "ordered_data: " << get_condition(Bulk_condition::ORDERED_DATA) << std::endl;
  std::cout << "optional_enclose: " << get_condition(Bulk_condition::OPTIONAL_ENCLOSE) << std::endl;
  std::cout << "dryrun: " << get_condition(Bulk_condition::DRYRUN) << std::endl;
  std::cout << "compression_algorithm: " << (m_compression_algorithm == Bulk_compression_algorithm::NONE ? "NONE" : "ZSTD") << std::endl;
  std::cout << "end settings ==================================================" << std::endl;
}

bool Bulk_loader_impl::load(size_t &affected_rows [[maybe_unused]]) noexcept {
  settings_dump_to_log();

  void *load_ctx = nullptr;

  auto cleanup_guard = create_scope_guard([&]() {
    end_session(load_ctx);
    release_services();
  });

  if (!acquire_services()) {
    return false;
  }

  // init stream reader
  auto data_stream = get_data_stream();
  if (!data_stream->open()) {
    return false;
  }

  const auto *srv_data_convert = data_convert_service();
  const auto *srv_data_load = data_load_service();

  const auto data_size = data_stream->get_data_size();
  const auto se_memory_size = srv_data_load->get_se_memory_size(m_thd, m_table);
  const auto loader_total_memory_size = (get_size(Bulk_size::MEMORY) == 0)
                                            ? default_loader_memory_bytes
                                            : get_size(Bulk_size::MEMORY);
  const auto concurrency = (get_size(Bulk_size::CONCURRENCY) == 0)
                               ? default_concurrency
                               : get_size(Bulk_size::CONCURRENCY);
  const auto is_data_ordered = get_condition(Bulk_condition::ORDERED_DATA);

  const size_t parser_buffer_size = loader_total_memory_size / 3;
  const size_t text_rows_buffer_size = parser_buffer_size;
  size_t converted_rows_buffer_size = parser_buffer_size;

  load_ctx = start_session(data_size, se_memory_size, concurrency);

  if (load_ctx == nullptr) {
    return false;
  }

  Row_meta row_metadata;

  if (!srv_data_convert->get_row_metadata(
          m_thd, m_table, is_data_ordered, row_metadata)) {
    LogComponentErr(ERROR_LEVEL, ER_BULK_LOADER_COMPONENT_ERROR,
                    "Failed to init row metadata");
    return false;
  }

  StreamParser stream_parser{parser_buffer_size,
                             row_metadata.m_num_columns,
                             get_parser_params(),
                             data_stream.get()};

  // init buffers
  const size_t max_text_rows_in_chunk =
      text_rows_buffer_size / (sizeof(Rows_text) * row_metadata.m_num_columns);
  Rows_text text_rows{row_metadata.m_num_columns};
  text_rows.set_num_rows(max_text_rows_in_chunk);
  size_t text_rows_count = 0;
  size_t next_unprocessed_text_row_index = 0;

  Bulk_load_error_location_details error_details;
  Rows_mysql sql_rows{row_metadata.m_num_columns};
  std::unique_ptr<char[]> sql_rows_buffer(
      new (std::nothrow) char[converted_rows_buffer_size]);

  if (sql_rows_buffer == nullptr) {
    LogComponentErr(ERROR_LEVEL, ER_BULK_LOADER_COMPONENT_ERROR,
                    "Failed to allocate rows buffer");
    return false;
  }

  Bulk_load::Stat_callbacks wait_cbks{
      []() {},
      []() {}
  };

  // process data
  auto row_iterator = stream_parser.row_iterator();

  for (const auto *result : row_iterator) {
    if (result->m_is_error) {
      std::stringstream error;
      error << "Error parsing row " << result->m_parser_row->m_row_idx;
      LogComponentErr(ERROR_LEVEL, ER_BULK_LOADER_COMPONENT_ERROR,
                      error.str().c_str());
      return false;
    }

    if (!result->m_parser_row->process_columns(text_rows)) {
      LogComponentErr(ERROR_LEVEL, ER_BULK_LOADER_COMPONENT_ERROR,
                      "Failed to process parsed row");
      return false;
    }

    ++text_rows_count;

    if (text_rows_count == max_text_rows_in_chunk ||
        result->m_parser_row->m_is_last_row) {
      text_rows.set_num_rows(text_rows_count);
      next_unprocessed_text_row_index = 0;
      sql_rows.set_num_rows(0);

      auto ret = srv_data_convert->mysql_format(
          m_thd, m_table, text_rows, next_unprocessed_text_row_index,
          sql_rows_buffer.get(), converted_rows_buffer_size, m_charset,
          row_metadata, sql_rows, error_details);

      if (ret != 0) {
        std::stringstream error;
        error << "Failed to format data, filename: " << error_details.filename
              << ", row_number: " << error_details.row_number
              << ", column_name: " << error_details.column_name
              << ", column_type: " << error_details.column_type
              << ", column_input_data: " << error_details.column_input_data;
        LogComponentErr(ERROR_LEVEL, ER_BULK_LOADER_COMPONENT_ERROR,
                        error.str().c_str());
        return false;
      }

      if (!srv_data_load->load(m_thd, load_ctx, m_table, sql_rows, 0,
                               wait_cbks)) {
        LogComponentErr(ERROR_LEVEL, ER_BULK_LOADER_COMPONENT_ERROR,
                        "Failed to load data");
        return false;
      }
    }
  }

  return true;
}

}  // namespace Bulk_load
