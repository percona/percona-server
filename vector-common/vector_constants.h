/* Copyright (c) 2024, 2026, Oracle and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is designed to work with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have either included with
   the program or referenced in the documentation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#pragma once

#include <stdint.h>

#include <algorithm>
#include <cassert>
#include <string_view>

#include "my_inttypes.h"
#include "mysql/strings/m_ctype.h"
#include "template_utils.h"

namespace vector_constants {
// maximum dimensions in a vector column
constexpr unsigned int max_dimensions = 16383;

/**
  Cost and row estimate assigned to vector index access in the cost-based
  optimizer. Chosen so that it always loses against any other access method,
  since a vector index is only ever activated for ORDER BY ... LIMIT. Large but
  finite, because std::numeric_limits<double>::max() overflows when the
  optimizer adds costs together.
*/
constexpr double prohibitive_cost = 1e100;

enum class Metric {
  kEuclidean,
  kEuclideanSquared,
  kCosine,
  kDotProduct,
  kManhattan
};

/** The name each Metric is spelled with in WITH(...). One table, so that
    metric_from_name() and metric_name() cannot drift apart. */
struct Metric_name {
  std::string_view name;
  Metric metric;
};
inline constexpr Metric_name kMetrics[] = {
    {"euclidean", Metric::kEuclidean},
    {"euclidean_squared", Metric::kEuclideanSquared},
    {"cosine", Metric::kCosine},
    {"dot", Metric::kDotProduct},
    {"manhattan", Metric::kManhattan},
};

/** Maps a case-insensitive metric name to a Metric value.
    Returns nullptr on no match. */
inline const Metric *metric_from_name(std::string_view name) {
  const auto *it = std::find_if(
      std::begin(kMetrics), std::end(kMetrics), [&](const auto &candidate) {
        return !my_strnncoll(
            &my_charset_latin1, pointer_cast<const uchar *>(name.data()),
            name.size(), pointer_cast<const uchar *>(candidate.name.data()),
            candidate.name.size());
      });
  return it != std::end(kMetrics) ? &it->metric : nullptr;
}

/** The canonical (lowercase) name of a Metric, the inverse of
    metric_from_name(). */
inline std::string_view metric_name(Metric metric) {
  const auto *it = std::find_if(
      std::begin(kMetrics), std::end(kMetrics),
      [&](const auto &candidate) { return candidate.metric == metric; });
  assert(it != std::end(kMetrics));
  return it->name;
}

/**
  HNSW index parameters: the defaults and the accepted ranges.

  The defaults apply only when an index is created. CREATE resolves every
  parameter the user left out and stores the full set in the DD, so an
  existing index never reads these again: changing a default here changes
  new indexes only, never the graph of an index already built (PS-11612).
*/
namespace hnsw {
constexpr int default_M = 25;
constexpr int default_ef_construction = 200;
constexpr Metric default_metric = Metric::kEuclidean;
constexpr int default_max_elements = 10000;

/** Each node's neighbor buffer is (layer + 2) * M pointers
    (vector-common/hnsw.h), so an unbounded M turns even a single-row index
    into a multi-gigabyte allocation; 200 keeps that buffer small while
    leaving headroom over any M a real workload would choose. */
constexpr int min_M = 2;
constexpr int max_M = 200;

/** The size of the candidate list an INSERT searches with. The graph raises
    it to at least M (vector-common/hnsw.h), so the lower bound only rules out
    nonsense; the upper bound keeps one insert's search from growing without
    limit. */
constexpr int min_ef_construction = 1;
constexpr int max_ef_construction = 4096;
}  // namespace hnsw

}  // namespace vector_constants

static inline uint32_t get_dimensions(const uint32_t length,
                                      const uint32_t precision) {
  if (length == 0 || (length % precision > 0)) {
    return UINT32_MAX;
  }
  return length / precision;
}
