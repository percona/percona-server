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

#include "plugin/audit_log_filter/table_access_services.h"
#include "plugin/audit_log_filter/audit_error_log.h"

SERVICE_TYPE(log_builtins) *log_bi = nullptr;
SERVICE_TYPE(log_builtins_string) *log_bs = nullptr;

namespace audit_log_filter {

TableAccessServices::~TableAccessServices() { deinit(); }

bool TableAccessServices::init() {
  my_h_service charset_service = nullptr;
  my_h_service current_thread_reader_service = nullptr;
  my_h_service string_factory_service = nullptr;
  my_h_service string_converter_service = nullptr;
  my_h_service ta_factory_service = nullptr;
  my_h_service ta_service = nullptr;
  my_h_service ta_scan_service = nullptr;
  my_h_service ta_index_service = nullptr;
  my_h_service ta_update_service = nullptr;
  my_h_service fa_varchar_service = nullptr;
  my_h_service fa_int_service = nullptr;
  my_h_service fa_any_service = nullptr;

  m_reg_srv = mysql_plugin_registry_acquire();

  if (init_logging_service_for_plugin(&m_reg_srv, &log_bi, &log_bs)) {
    return false;
  }

  if (!m_reg_srv->acquire("mysql_charset.mysql_server", &charset_service) &&
      !m_reg_srv->acquire("mysql_current_thread_reader.mysql_server",
                          &current_thread_reader_service) &&
      !m_reg_srv->acquire("mysql_string_factory.mysql_server",
                          &string_factory_service) &&
      !m_reg_srv->acquire("mysql_string_charset_converter.mysql_server",
                          &string_converter_service) &&
      !m_reg_srv->acquire("table_access_factory_v1.mysql_server",
                          &ta_factory_service) &&
      !m_reg_srv->acquire("table_access_v1.mysql_server", &ta_service) &&
      !m_reg_srv->acquire("table_access_scan_v1.mysql_server",
                          &ta_scan_service) &&
      !m_reg_srv->acquire("table_access_index_v1.mysql_server",
                          &ta_index_service) &&
      !m_reg_srv->acquire("table_access_update_v1.mysql_server",
                          &ta_update_service) &&
      !m_reg_srv->acquire("field_varchar_access_v1.mysql_server",
                          &fa_varchar_service) &&
      !m_reg_srv->acquire("field_integer_access_v1.mysql_server",
                          &fa_int_service) &&
      !m_reg_srv->acquire("field_any_access_v1.mysql_server",
                          &fa_any_service)) {
    m_charset_srv = reinterpret_cast<mysql_charset_srv_t *>(charset_service);
    m_current_thd_srv = reinterpret_cast<mysql_current_thread_reader_srv_t *>(
        current_thread_reader_service);
    m_string_factory_srv =
        reinterpret_cast<mysql_string_factory_srv_t *>(string_factory_service);
    m_string_converter_srv =
        reinterpret_cast<mysql_string_charset_converter_srv_t *>(
            string_converter_service);
    m_ta_factory_srv =
        reinterpret_cast<table_access_factory_srv_t *>(ta_factory_service);
    m_ta_srv = reinterpret_cast<table_access_srv_t *>(ta_service);
    m_ta_scan_srv =
        reinterpret_cast<table_access_scan_srv_t *>(ta_scan_service);
    m_ta_index_srv =
        reinterpret_cast<table_access_index_srv_t *>(ta_index_service);
    m_ta_update_srv =
        reinterpret_cast<table_access_update_srv_t *>(ta_update_service);
    m_fa_varchar_srv =
        reinterpret_cast<field_varchar_access_srv_t *>(fa_varchar_service);
    m_fa_int_srv =
        reinterpret_cast<field_integer_access_srv_t *>(fa_int_service);
    m_fa_any_srv = reinterpret_cast<field_any_access_srv_t *>(fa_any_service);

    return true;
  }

  return false;
}

void TableAccessServices::deinit() {
  using charset_t = SERVICE_TYPE_NO_CONST(mysql_charset);
  using current_thread_reader_t =
      SERVICE_TYPE_NO_CONST(mysql_current_thread_reader);
  using string_factory_t = SERVICE_TYPE_NO_CONST(mysql_string_factory);
  using string_converter_t =
      SERVICE_TYPE_NO_CONST(mysql_string_charset_converter);
  using table_access_factory_t = SERVICE_TYPE_NO_CONST(table_access_factory_v1);
  using table_access_t = SERVICE_TYPE_NO_CONST(table_access_v1);
  using table_access_scan_t = SERVICE_TYPE_NO_CONST(table_access_scan_v1);
  using table_access_index_t = SERVICE_TYPE_NO_CONST(table_access_index_v1);
  using table_access_update_t = SERVICE_TYPE_NO_CONST(table_access_update_v1);
  using field_varchar_access_t = SERVICE_TYPE_NO_CONST(field_varchar_access_v1);
  using field_int_access_t = SERVICE_TYPE_NO_CONST(field_integer_access_v1);
  using field_any_access_t = SERVICE_TYPE_NO_CONST(field_any_access_v1);

  if (m_reg_srv == nullptr) {
    return;
  }

  if (m_charset_srv != nullptr) {
    m_reg_srv->release(
        reinterpret_cast<my_h_service>(const_cast<charset_t *>(m_charset_srv)));
  }
  if (m_current_thd_srv != nullptr) {
    m_reg_srv->release(reinterpret_cast<my_h_service>(
        const_cast<current_thread_reader_t *>(m_current_thd_srv)));
  }
  if (m_string_factory_srv != nullptr) {
    m_reg_srv->release(reinterpret_cast<my_h_service>(
        const_cast<string_factory_t *>(m_string_factory_srv)));
  }
  if (m_string_converter_srv != nullptr) {
    m_reg_srv->release(reinterpret_cast<my_h_service>(
        const_cast<string_converter_t *>(m_string_converter_srv)));
  }
  if (m_ta_factory_srv != nullptr) {
    m_reg_srv->release(reinterpret_cast<my_h_service>(
        const_cast<table_access_factory_t *>(m_ta_factory_srv)));
  }
  if (m_ta_srv != nullptr) {
    m_reg_srv->release(
        reinterpret_cast<my_h_service>(const_cast<table_access_t *>(m_ta_srv)));
  }
  if (m_ta_scan_srv != nullptr) {
    m_reg_srv->release(reinterpret_cast<my_h_service>(
        const_cast<table_access_scan_t *>(m_ta_scan_srv)));
  }
  if (m_ta_index_srv != nullptr) {
    m_reg_srv->release(reinterpret_cast<my_h_service>(
        const_cast<table_access_index_t *>(m_ta_index_srv)));
  }
  if (m_ta_update_srv != nullptr) {
    m_reg_srv->release(reinterpret_cast<my_h_service>(
        const_cast<table_access_update_t *>(m_ta_update_srv)));
  }
  if (m_fa_varchar_srv != nullptr) {
    m_reg_srv->release(reinterpret_cast<my_h_service>(
        const_cast<field_varchar_access_t *>(m_fa_varchar_srv)));
  }
  if (m_fa_int_srv != nullptr) {
    m_reg_srv->release(reinterpret_cast<my_h_service>(
        const_cast<field_int_access_t *>(m_fa_int_srv)));
  }
  if (m_fa_any_srv != nullptr) {
    m_reg_srv->release(reinterpret_cast<my_h_service>(
        const_cast<field_any_access_t *>(m_fa_any_srv)));
  }

  deinit_logging_service_for_plugin(&m_reg_srv, &log_bi, &log_bs);

  mysql_plugin_registry_release(m_reg_srv);

  m_charset_srv = nullptr;
  m_current_thd_srv = nullptr;
  m_string_factory_srv = nullptr;
  m_string_converter_srv = nullptr;
  m_ta_factory_srv = nullptr;
  m_ta_srv = nullptr;
  m_ta_scan_srv = nullptr;
  m_ta_index_srv = nullptr;
  m_ta_update_srv = nullptr;
  m_fa_varchar_srv = nullptr;
  m_fa_int_srv = nullptr;
  m_fa_any_srv = nullptr;
  m_reg_srv = nullptr;
}

}  // namespace audit_log_filter
