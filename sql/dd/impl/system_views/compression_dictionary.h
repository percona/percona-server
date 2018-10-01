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

#ifndef DD_SYSTEM_VIEWS__COMPRESSION_DICTIONARY_INCLUDED
#define DD_SYSTEM_VIEWS__COMPRESSION_DICTIONARY_INCLUDED

#include "sql/dd/impl/system_views/system_view_definition_impl.h"
#include "sql/dd/impl/system_views/system_view_impl.h"

namespace dd {
namespace system_views {

/*
  The class representing INFORMATION_SCHEMA.INNODB_COMPRESSION_DICTIONARY system
  view definition
*/
class Compression_dictionary
    : public System_view_impl<System_view_select_definition_impl> {
 public:
  enum enum_fields { FIELD_DICT_VERSION, FIELD_DICT_NAME, FIELD_DICT_DATA };

  Compression_dictionary();

  static const Compression_dictionary &instance() noexcept;

  static const String_type &view_name() noexcept {
    static String_type s_view_name("COMPRESSION_DICTIONARY");
    return s_view_name;
  }

  virtual const String_type &name() const noexcept override {
    return Compression_dictionary::view_name();
  }
};

}  // namespace system_views
}  // namespace dd

#endif  // DD_SYSTEM_VIEWS__COMPRESSION_DICTIONARY_INCLUDED
