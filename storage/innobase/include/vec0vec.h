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

#include <string_view>
#include <variant>
#include "key.h"
#include "key_spec.h"

namespace storage::innobase::vec {

struct HnswParam {
  int M{25};
  int max_elements{10000};
  int ef_construction{200};
  std::string_view metric{"euclidean"};
};

using VectorIndexParam = std::variant<std::monostate, HnswParam>;

/** Validate the SHAPE of a vector index as the user wrote it: single
non-prefixed column, SE-specific algorithm, a TYPE token we know. Only
meaningful at DDL time, which is why it takes a Key_spec — by the time a
table is opened the definition has already been through here and come
back from the DD. Reports through my_error().
@return true on error */
bool validate_options(const Key_spec &index_def);

/** Turn a TYPE token plus its WITH(...) list into typed parameters.
Pure parsing: no shape checks, so it serves both DDL and table open.
@param[in]   type    the TYPE token, e.g. "hnsw"
@param[in]   params  the WITH(...) pairs, may be nullptr for none
@param[out]  vip     the parameters, on success
@return true on error */
bool parse_options(LEX_CSTRING type, const Vector_index_params_YY *params,
                   VectorIndexParam &vip);

/** DDL-time overload: validates the shape, then parses. */
bool parse_options(const Key_spec &index_def, VectorIndexParam &vip);

/** Open-time overload: the definition came back from the DD, so there is
nothing left to validate — parse only. */
bool parse_options(const KEY &key, VectorIndexParam &vip);

}  // namespace storage::innobase::vec
