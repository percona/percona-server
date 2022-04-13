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

#include "plugin/audit_log_filter/table_access_services.h"

#include <cstddef>
#include <memory>

namespace audit_log_filter::audit_table {

inline constexpr const size_t kAuditFieldLengthUsername = 32;
inline constexpr const size_t kAuditFieldLengthUserhost = 255;
inline constexpr const size_t kAuditFieldLengthFiltername = 255;
inline constexpr const size_t kAuditFieldLengthFilter = 1024;

enum class TableResult { Ok, Fail, MissingTable, Found, NotFound };

/**
 * @brief Wrapper class holding table access session details.
 */
struct TableAccessContext {
  MYSQL_THD thd = nullptr;
  Table_access ta_session = nullptr;
  size_t table_ticket = 0;
  TA_table ta_table = nullptr;

  TableAccessServices *ta_services;

  TableAccessContext() = delete;
  explicit TableAccessContext(TableAccessServices *ta_services_)
      : ta_services{ta_services_} {}

  ~TableAccessContext() {
    ta_table = nullptr;
    table_ticket = 0;

    if (ta_session != nullptr) {
      ta_services->get_ta_factory_srv()->destroy(ta_session);
      ta_session = nullptr;
    }

    thd = nullptr;
  }
};

class AuditTableBase {
 public:
  explicit AuditTableBase(TableAccessServices *table_access_services);
  virtual ~AuditTableBase() = default;

  /**
   * @brief Get table access service instance.
   *
   * @return Table access service
   */
  [[nodiscard]] TableAccessServices *get_ta_srv() noexcept {
    return m_table_access_services;
  }

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

 private:
  TableAccessServices *m_table_access_services = nullptr;
};

}  // namespace audit_log_filter::audit_table

#endif  // AUDIT_LOG_FILTER_AUDIT_TABLE_BASE_H_INCLUDED
