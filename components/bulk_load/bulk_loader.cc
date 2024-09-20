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

#include "bulk_loader.h"

#include "components/bulk_load/data_source/data_source.h"
#include "components/bulk_load/rows_reader/text_rows_reader.h"
#include "scope_guard.h"

#include <mysql/components/services/log_builtins.h>
#include <mysqld_error.h>

namespace Bulk_load {
namespace {

constexpr size_t default_loader_memory_bytes = 1024 * 1024 * 1024;
constexpr size_t default_concurrency = 1;  // TODO: 16

}  // namespace

Bulk_loader_impl::Bulk_loader_impl(THD *thd,
                                   const my_thread_id connection_id,
                                   const TABLE *table,
                                   const Bulk_source bulk_source,
                                   const CHARSET_INFO *charset)
  : m_thd{thd},
    m_connection_id{connection_id},
    m_table{table},
    m_bulk_source{bulk_source},
    m_charset{charset},
    m_compression_algorithm{Bulk_compression_algorithm::NONE} {}

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

void Bulk_loader_impl::set_compression_algorithm(
    Bulk_compression_algorithm algorithm) noexcept {
  m_compression_algorithm = algorithm;
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

bool Bulk_loader_impl::load(size_t &affected_rows [[maybe_unused]]) noexcept {
  void *load_ctx = nullptr;

  auto cleanup_guard = create_scope_guard([&]() {
    end_session(load_ctx);
    release_services();
  });

  if (!acquire_services()) {
    return false;
  }

  // init data reader
  auto data_source = Data_source::create(m_bulk_source,
                                         get_string(Bulk_string::FILE_PREFIX));
  if (!data_source->open()) {
    return false;
  }

  const auto *srv_data_convert = data_convert_service();
  const auto *srv_data_load = data_load_service();

  const auto data_size = data_source->get_data_size();
  const auto se_memory_size = srv_data_load->get_se_memory_size(m_thd, m_table);
  const auto loader_total_memory_size = (get_size(Bulk_size::MEMORY) == 0)
                                            ? default_loader_memory_bytes
                                            : get_size(Bulk_size::MEMORY);
  const auto concurrency = (get_size(Bulk_size::CONCURRENCY) == 0)
                               ? default_concurrency
                               : get_size(Bulk_size::CONCURRENCY);
  const auto is_data_ordered = get_condition(Bulk_condition::ORDERED_DATA);

  const size_t reader_buffer_size = loader_total_memory_size / 3;
  const size_t text_rows_buffer_size = reader_buffer_size;
  size_t converted_rows_buffer_size = reader_buffer_size;

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

  TextRowsReader text_rows_reader{reader_buffer_size,
                                  row_metadata.m_num_columns,
                                  get_parser_params(),
                                  data_source.get()};

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

  for (const auto *result : text_rows_reader.row_iterator()) {
    if (result->m_is_error) {
      std::stringstream error;
      error << "Error parsing row " << result->m_text_row->m_row_idx;
      LogComponentErr(ERROR_LEVEL, ER_BULK_LOADER_COMPONENT_ERROR,
                      error.str().c_str());
      return false;
    }

    if (!result->m_text_row->process_columns(text_rows)) {
      LogComponentErr(ERROR_LEVEL, ER_BULK_LOADER_COMPONENT_ERROR,
                      "Failed to process parsed row");
      return false;
    }

    ++text_rows_count;

    if (text_rows_count == max_text_rows_in_chunk ||
        result->m_text_row->m_is_last_row) {
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

//THD *Bulk_loader_impl::get_thd() noexcept {
//  return m_thd;
//}
//
//my_thread_id Bulk_loader_impl::get_connection_id() const noexcept {
//  return m_connection_id;
//}
//
//const TABLE *Bulk_loader_impl::get_table() noexcept {
//  return m_table;
//}
//
//const CHARSET_INFO *Bulk_loader_impl::get_charset() noexcept {
//  return m_charset;
//}
//
//Bulk_compression_algorithm
//Bulk_loader_impl::get_compression_algorithm() const noexcept {
//  return m_compression_algorithm;
//}

}  // namespace Bulk_load
