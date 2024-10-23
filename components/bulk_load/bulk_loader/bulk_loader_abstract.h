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

#pragma once

#include "mysql/service_plugin_registry.h"
#include "mysql/components/service.h"
#include "mysql/components/services/registry.h"
#include "mysql/components/services/bulk_data_service.h"
#include "mysql/components/services/bulk_load_service.h"

#include "stream_parser/parser_params.h"
#include "components/bulk_load/data_stream/data_stream_abstract.h"

#include <string>
#include <unordered_map>

namespace Bulk_load {

class Bulk_loader_impl {
 public:
  Bulk_loader_impl(THD *thd, my_thread_id connection_id, const TABLE *table,
                   const CHARSET_INFO *charset);

  Bulk_loader_impl(const Bulk_loader_impl &other) = delete;
  Bulk_loader_impl(Bulk_loader_impl &&other) = delete;
  virtual ~Bulk_loader_impl() = default;
  Bulk_loader_impl &operator=(const Bulk_loader_impl &other) = delete;
  Bulk_loader_impl &operator=(Bulk_loader_impl &&other) = delete;

  void set_string(Bulk_string type, std::string value) noexcept;
  void set_char(Bulk_char type, unsigned char value) noexcept;
  void set_size(Bulk_size type, size_t value) noexcept;
  void set_condition(Bulk_condition type, bool value) noexcept;
  void set_compression_algorithm(Bulk_compression_algorithm algorithm) noexcept;

  bool load(size_t &affected_rows) noexcept;

 protected:
  // temporary debug
  void settings_dump_to_log();

  THD *get_thd() noexcept;
  my_thread_id get_connection_id() const noexcept;
  const TABLE *get_table() noexcept;
  const CHARSET_INFO *get_charset() noexcept;

  std::string get_string(Bulk_string type) const noexcept;
  unsigned char get_char(Bulk_char type) const noexcept;
  size_t get_size(Bulk_size type) const noexcept;
  bool get_condition(Bulk_condition type) const noexcept;
  Bulk_compression_algorithm get_compression_algorithm() const noexcept;

  bool acquire_services() noexcept;
  void release_services() noexcept;
  SERVICE_TYPE(bulk_data_convert) *data_convert_service() noexcept;
  SERVICE_TYPE(bulk_data_load) *data_load_service() noexcept;

  void *start_session(size_t data_size, size_t se_memory_size, size_t num_threads) noexcept;
  void end_session(void *load_ctx) noexcept;

  ParserParamsPtr get_parser_params() const noexcept;

 private:
  virtual std::unique_ptr<DataStreamAbstract> get_data_stream() noexcept = 0;

 private:
  THD *m_thd;
  my_thread_id m_connection_id;
  const TABLE *m_table;
  const CHARSET_INFO *m_charset;

  std::unordered_map<Bulk_string, std::string> m_string_attrs;
  std::unordered_map<Bulk_char, unsigned char> m_char_attrs;
  std::unordered_map<Bulk_size, size_t> m_size_attrs;
  std::unordered_map<Bulk_condition, bool> m_condition_attrs;

  Bulk_compression_algorithm m_compression_algorithm;

  SERVICE_TYPE(registry) *m_srv_registry = nullptr;
  my_h_service m_svc_data_convert = nullptr;
  my_h_service m_svc_data_load = nullptr;
  bool m_load_session_start_done = false;
};

template <Bulk_source BulkSource>
class Bulk_loader_base;

}  // namespace Bulk_load
