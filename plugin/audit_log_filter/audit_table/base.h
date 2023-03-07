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

#ifndef AUDIT_LOG_FILTER_AUDIT_TABLE_BASE_H_INCLUDED
#define AUDIT_LOG_FILTER_AUDIT_TABLE_BASE_H_INCLUDED

#include <mysql/components/services/table_access_service.h>

#include <cstddef>
#include <memory>

namespace audit_log_filter::audit_table {

inline constexpr const size_t kAuditFieldLengthUsername = 32;
inline constexpr const size_t kAuditFieldLengthUserhost = 255;
inline constexpr const size_t kAuditFieldLengthFiltername = 255;
inline constexpr const size_t kAuditFieldLengthFilter = 1024;

enum class TableResult { Ok, Fail, Found, NotFound };

/**
 * @brief Wrapper class holding table access session details.
 */
struct TableAccessContext {
  MYSQL_THD thd = nullptr;
  Table_access ta_session = nullptr;
  size_t table_ticket = 0;
  TA_table ta_table = nullptr;

  TableAccessContext() = default;
  ~TableAccessContext();
};

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

class AuditTableBase {
 public:
  AuditTableBase() = default;
  virtual ~AuditTableBase() = default;

 protected:
  /**
   * @brief Open table.
   *
   * @return @ref TableAccessContext instance for an opened table.
   */
  std::unique_ptr<TableAccessContext> open_table() noexcept;

 private:
  /**
   * @brief Get database name.
   *
   * @return Database name
   */
  [[nodiscard]] virtual const char *get_table_db_name() noexcept = 0;

  /**
   * @brief Get table name.
   *
   * @return Table name
   */
  [[nodiscard]] virtual const char *get_table_name() noexcept = 0;

  /**
   * @brief Get table fields definition.
   *
   * @return Table fields definition
   */
  [[nodiscard]] virtual const TA_table_field_def *
  get_table_field_def() noexcept = 0;

  /**
   * @brief Get number of table fields.
   *
   * @return Number of table fields
   */
  [[nodiscard]] virtual size_t get_table_field_count() noexcept = 0;
};

}  // namespace audit_log_filter::audit_table

#endif  // AUDIT_LOG_FILTER_AUDIT_TABLE_BASE_H_INCLUDED
