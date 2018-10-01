/*****************************************************************************

Copyright (c) 2018, Percona Inc. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Suite 500, Boston, MA 02110-1335 USA

*****************************************************************************/

#include "sql/dd/impl/system_views/compression_dictionary_tables.h"

namespace dd {
namespace system_views {

const Compression_dictionary_tables &
Compression_dictionary_tables::instance() noexcept {
  static Compression_dictionary_tables *s_instance =
      new Compression_dictionary_tables();
  return *s_instance;
}

Compression_dictionary_tables::Compression_dictionary_tables() {
  m_target_def.set_view_name(view_name());

  m_target_def.add_field(FIELD_TABLE_SCHEMA, "TABLE_SCHEMA", "sch.name");

  m_target_def.add_field(FIELD_TABLE_NAME, "TABLE_NAME", "tbl.name");

  m_target_def.add_field(FIELD_COLUMN_NAME, "COLUMN_NAME", "col.name");

  m_target_def.add_field(FIELD_DICT_NAME, "DICT_NAME", "dict.name");

  m_target_def.add_from("mysql.compression_dictionary_cols cdc");
  m_target_def.add_from("JOIN mysql.tables tbl ON cdc.table_id=tbl.id");
  m_target_def.add_from(
      "JOIN mysql.columns col ON col.table_id = cdc.table_id AND col.id = "
      "cdc.column_id");
  m_target_def.add_from("JOIN mysql.schemata sch ON tbl.schema_id=sch.id");
  m_target_def.add_from(
      "JOIN mysql.compression_dictionary dict ON dict.id = cdc.dict_id");
}

}  // namespace system_views
}  // namespace dd
