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

#ifndef AUDIT_LOG_FILTER_TABLE_ACCESS_SERVICES_H_INCLUDED
#define AUDIT_LOG_FILTER_TABLE_ACCESS_SERVICES_H_INCLUDED

#include <mysql/components/services/table_access_service.h>
#include <mysql/plugin.h>

namespace audit_log_filter {

/**
 * @brief Wrapper class around my_h_string to make sure resources are released
 *        once string is not needed any more.
 */
class HStringContainer {
  using string_factory_srv_t = SERVICE_TYPE(mysql_string_factory);

 public:
  explicit HStringContainer(string_factory_srv_t *string_factory)
      : m_string_factory{string_factory}, m_string{nullptr} {
    m_string_factory->create(&m_string);
  }

  ~HStringContainer() { m_string_factory->destroy(m_string); }

  /**
   * @brief Get wrapped string object.
   *
   * @return Wrapped string object
   */
  my_h_string get() { return m_string; }

 private:
  string_factory_srv_t *m_string_factory;
  my_h_string m_string;
};

class TableAccessServices {
  using registry_srv_t = SERVICE_TYPE(registry);
  using mysql_charset_srv_t = SERVICE_TYPE(mysql_charset);
  using mysql_current_thread_reader_srv_t =
      SERVICE_TYPE(mysql_current_thread_reader);
  using mysql_string_factory_srv_t = SERVICE_TYPE(mysql_string_factory);
  using mysql_string_charset_converter_srv_t =
      SERVICE_TYPE(mysql_string_charset_converter);
  using table_access_factory_srv_t = SERVICE_TYPE(table_access_factory_v1);
  using table_access_srv_t = SERVICE_TYPE(table_access_v1);
  using table_access_scan_srv_t = SERVICE_TYPE(table_access_scan_v1);
  using table_access_index_srv_t = SERVICE_TYPE(table_access_index_v1);
  using table_access_update_srv_t = SERVICE_TYPE(table_access_update_v1);
  using field_varchar_access_srv_t = SERVICE_TYPE(field_varchar_access_v1);
  using field_integer_access_srv_t = SERVICE_TYPE(field_integer_access_v1);
  using field_any_access_srv_t = SERVICE_TYPE(field_any_access_v1);

 public:
  ~TableAccessServices();

  /**
   * @brief Initialize required table access services.
   *
   * @return true in case services initialized successfully,
   *         false otherwise
   */
  bool init();

  /**
   * @brief Get character set handling service.
   *
   * @return Character set handling service
   */
  mysql_charset_srv_t *get_charset_srv() { return m_charset_srv; }

  /**
   * @brief Get current thread access service.
   *
   * @return Current thread access service
   */
  mysql_current_thread_reader_srv_t *get_current_thd_srv() {
    return m_current_thd_srv;
  }

  /**
   * @brief Get string factory service.
   *
   * @return String factory service
   */
  mysql_string_factory_srv_t *get_string_factory_srv() {
    return m_string_factory_srv;
  }

  /**
   * @brief Get string character set converter service.
   *
   * @return String character set converter service
   */
  mysql_string_charset_converter_srv_t *get_string_converter_srv() {
    return m_string_converter_srv;
  }

  /**
   * @brief Get table access factory service.
   *
   * @return Table access factory service
   */
  table_access_factory_srv_t *get_ta_factory_srv() { return m_ta_factory_srv; }

  /**
   * @brief Get table access service.
   *
   * @return Table access service
   */
  table_access_srv_t *get_ta_srv() { return m_ta_srv; }

  /**
   * @brief Get table scan service.
   *
   * @return Table scan service
   */
  table_access_scan_srv_t *get_ta_scan_srv() { return m_ta_scan_srv; }

  /**
   * @brief Get table index access service.
   *
   * @return Table index access service
   */
  table_access_index_srv_t *get_ta_index_srv() { return m_ta_index_srv; }

  /**
   * @brief Get table update service.
   *
   * @return Table update service
   */
  table_access_update_srv_t *get_ta_update_srv() { return m_ta_update_srv; }

  /**
   * @brief Get varchar field access service.
   *
   * @return varchar field access service
   */
  field_varchar_access_srv_t *get_fa_varchar_srv() { return m_fa_varchar_srv; }

  /**
   * @brief Get integer field access service.
   *
   * @return integer field access service
   */
  field_integer_access_srv_t *get_fa_int_srv() { return m_fa_int_srv; }

  /**
   * @brief Get field access service for type ANY.
   *
   * @return Field access service for type ANY
   */
  field_any_access_srv_t *get_fa_any_srv() { return m_fa_any_srv; }

 private:
  /**
   * @brief De-initialize table access services.
   */
  void deinit();

 private:
  registry_srv_t *m_reg_srv = nullptr;
  mysql_charset_srv_t *m_charset_srv = nullptr;
  mysql_current_thread_reader_srv_t *m_current_thd_srv = nullptr;
  mysql_string_factory_srv_t *m_string_factory_srv = nullptr;
  mysql_string_charset_converter_srv_t *m_string_converter_srv = nullptr;
  table_access_factory_srv_t *m_ta_factory_srv = nullptr;
  table_access_srv_t *m_ta_srv = nullptr;
  table_access_scan_srv_t *m_ta_scan_srv = nullptr;
  table_access_index_srv_t *m_ta_index_srv = nullptr;
  table_access_update_srv_t *m_ta_update_srv = nullptr;
  field_varchar_access_srv_t *m_fa_varchar_srv = nullptr;
  field_integer_access_srv_t *m_fa_int_srv = nullptr;
  field_any_access_srv_t *m_fa_any_srv = nullptr;
};

}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_TABLE_ACCESS_SERVICES_H_INCLUDED
