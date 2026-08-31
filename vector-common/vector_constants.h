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
#include <string_view>

#include "my_inttypes.h"
#include "mysql/strings/m_ctype.h"
#include "template_utils.h"

namespace vector_constants {
// maximum dimensions in a vector column
constexpr unsigned int max_dimensions = 16383;

enum class Metric {
  kEuclidean,
  kEuclideanSquared,
  kCosine,
  kDotProduct,
  kManhattan
};

/** Maps a case-insensitive metric name to a Metric value.
    Returns nullptr on no match. */
inline const Metric *metric_from_name(std::string_view name) {
  static constexpr struct {
    std::string_view name;
    Metric metric;
  } kMetrics[] = {
      {"euclidean", Metric::kEuclidean},
      {"euclidean_squared", Metric::kEuclideanSquared},
      {"cosine", Metric::kCosine},
      {"dot", Metric::kDotProduct},
      {"manhattan", Metric::kManhattan},
  };
  const auto *it = std::find_if(
      std::begin(kMetrics), std::end(kMetrics), [&](const auto &candidate) {
        return !my_strnncoll(
            &my_charset_latin1, pointer_cast<const uchar *>(name.data()),
            name.size(), pointer_cast<const uchar *>(candidate.name.data()),
            candidate.name.size());
      });
  return it != std::end(kMetrics) ? &it->metric : nullptr;
}

}  // namespace vector_constants

static inline uint32_t get_dimensions(const uint32_t length,
                                      const uint32_t precision) {
  if (length == 0 || (length % precision > 0)) {
    return UINT32_MAX;
  }
  return length / precision;
}
