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

#ifndef vec0vec_h
#define vec0vec_h

#include <variant>
#include "key.h"
#include "key_spec.h"
#include "vector-common/vector_constants.h"

namespace storage::innobase::vec {

/** A distance kernel: the signature vector-common's kernels have and the
one HNSW's vec_dist_func_t names. Spelled out here so this header does
not have to pull in the graph template. */
using vec_metric_func_t = double (*)(const char *a, const char *b,
                                     uint32_t dims);

struct HnswParam {
  int M{25};
  int max_elements{10000};
  int ef_construction{200};
  vector_constants::Metric metric{vector_constants::Metric::kEuclidean};
  /** The kernel `metric` selects, resolved by the parser so that the
  metric and the function cannot drift apart: whoever builds a graph uses
  this rather than picking a kernel of its own. Never null once
  parse_options has returned false. */
  vec_metric_func_t dist{nullptr};
};

using VectorIndexParam = std::variant<std::monostate, HnswParam>;

/** Check the SHAPE of a vector index as the user wrote it: a
non-prefixed column and an SE-specific algorithm. Not the column count -
this runs from check_engine(), ahead of prepare_key(), so a multi-column
vector key is still standing here and the server rejects it later.
Only meaningful at DDL time, which is why it takes a Key_spec: by the
time a table is opened the definition has already been through here and
come back from the DD. Reports through my_error().
@return true on error */
bool validate_options(const Key_spec &index_def);

/** Turn a TYPE token plus its WITH(...) list into typed parameters.

This checks the CONTENTS - that the TYPE token is one we implement, that
every WITH key is a parameter we support, and that its value is legal -
and reports through my_error() like the shape check above. What it does
not look at is the shape, which is what lets it serve both DDL and table
open; "parse" here is not a promise that nothing is rejected.
@param[in]   type    the TYPE token, e.g. "hnsw"
@param[in]   params  the WITH(...) pairs, empty for none
@param[out]  vip     the parameters, on success
@return true on error */
bool parse_options(LEX_CSTRING type, const Vector_index_params_YY *params,
                   VectorIndexParam &vip);

/** DDL-time overload: checks the shape, then the contents. */
bool parse_options(const Key_spec &index_def, VectorIndexParam &vip);

/** Open-time overload: the definition came back from the DD, so its
shape was settled at DDL time - contents only. */
bool parse_options(const KEY &key, VectorIndexParam &vip);

}  // namespace storage::innobase::vec

#endif /* vec0vec_h */
