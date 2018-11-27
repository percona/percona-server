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

#ifndef DD_SYSTEM_VIEWS__COMPRESSION_DICTIONARY_TABLES_INCLUDED
#define DD_SYSTEM_VIEWS__COMPRESSION_DICTIONARY_TABLES_INCLUDED

#include "sql/dd/impl/system_views/system_view_definition_impl.h"
#include "sql/dd/impl/system_views/system_view_impl.h"

namespace dd {
namespace system_views {

/*
  The class representing INFORMATION_SCHEMA.COMPRESSSION_DICTIONARY_TABLES
  system view definition
*/
class Compression_dictionary_tables
    : public System_view_impl<System_view_select_definition_impl> {
 public:
  enum enum_fields {
    FIELD_TABLE_SCHEMA,
    FIELD_TABLE_NAME,
    FIELD_COLUMN_NAME,
    FIELD_DICT_NAME
  };

  Compression_dictionary_tables();

  static const Compression_dictionary_tables &instance() noexcept;

  static const String_type &view_name() noexcept {
    static String_type s_view_name("COMPRESSION_DICTIONARY_TABLES");
    return s_view_name;
  }

  virtual const String_type &name() const noexcept override {
    return Compression_dictionary_tables::view_name();
  }
};

}  // namespace system_views
}  // namespace dd

#endif  // DD_SYSTEM_VIEWS__COMPRESSION_DICTIONARY_TABLES_INCLUDED
