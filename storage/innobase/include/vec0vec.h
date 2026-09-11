/*****************************************************************************

Copyright (c) 2026, Percona Inc.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is designed to work with certain software (including
but not limited to OpenSSL) that is licensed under separate terms,
as designated in a particular file or component or in included license
documentation.  The authors of MySQL hereby grant you an additional
permission to link the program and your derivative works with the
separately licensed software that they have either included with
the program or referenced in the documentation.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/

#pragma once

#include <variant>
#include "key_spec.h"
#include "vector-common/vector_constants.h"

namespace storage::innobase::vec {

bool validate_options(const Key_spec &index_def);

struct HnswParam {
  int M{25};
  int max_elements{10000};
  int ef_construction{200};
  vector_constants::Metric metric{vector_constants::Metric::kEuclidean};
};

using VectorIndexParam = std::variant<std::monostate, HnswParam>;

bool parse_options(const Key_spec &index_def, VectorIndexParam &vip);

}  // namespace storage::innobase::vec
