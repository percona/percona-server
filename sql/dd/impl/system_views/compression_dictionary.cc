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

#include "sql/dd/impl/system_views/compression_dictionary.h"

namespace dd {
namespace system_views {

const Compression_dictionary &Compression_dictionary::instance() noexcept {
  static Compression_dictionary *s_instance = new Compression_dictionary();
  return *s_instance;
}

Compression_dictionary::Compression_dictionary() {
  m_target_def.set_view_name(view_name());

  m_target_def.add_field(FIELD_DICT_VERSION, "DICT_VERSION", "dict.version");
  m_target_def.add_field(FIELD_DICT_NAME, "DICT_NAME", "dict.name");
  m_target_def.add_field(FIELD_DICT_DATA, "DICT_DATA", "dict.data");

  m_target_def.add_from("mysql.compression_dictionary dict");
}

}  // namespace system_views
}  // namespace dd
