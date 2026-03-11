/* Copyright (c) 2026 Percona LLC and/or its affiliates. All rights reserved.

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

#ifndef LIVE_STATS_H_INCLUDED
#define LIVE_STATS_H_INCLUDED

#include <algorithm>
#include <cmath>
#include <cstdint>

/**
Utility class which allows to gather basic statistics such as min, max, mean and
variance for a population in incremental fashion (using Welford's online
algorithm).
*/

class LiveStats {
 public:
  struct Stats {
    int64_t count = 0;
    double min = 0.0;
    double max = 0.0;
    double average = 0.0;
    double deviation = 0.0;
  };

 private:
  int64_t count = 0;
  double c_min = 0.0, c_max = 0.0, m1 = 0.0, m2 = 0.0;

 public:
  LiveStats() = default;
  LiveStats(const LiveStats &) = delete;
  LiveStats(LiveStats &&) = delete;
  LiveStats &operator=(const LiveStats &) = delete;
  LiveStats &operator=(LiveStats &&) = delete;
  ~LiveStats() = default;

  void addValue(double x) {
    count++;
    if (count == 1) {
      c_min = c_max = m1 = x;
      m2 = 0.0;
    } else {
      c_min = std::min(c_min, x);
      c_max = std::max(c_max, x);
      double delta = x - m1;
      m1 += delta / count;
      m2 += delta * (x - m1);
    }
  }

  void reset() {
    count = 0;
    c_min = c_max = m1 = m2 = 0.0;
  }

  Stats getStats() const {
    return {.count = count,
            .min = c_min,
            .max = c_max,
            .average = m1,
            .deviation = (count > 1) ? std::sqrt(m2 / count) : 0.0};
  }

  /**
   * Merge statistics collected by another LiveStats instance into this one.
   * Uses the parallel Welford algorithm to combine counts, min/max,
   * mean and M2 (sum of squared deviations) exactly.
   */
  void join(const LiveStats &other) {
    if (other.count == 0) return;
    if (count == 0) {
      count = other.count;
      c_min = other.c_min;
      c_max = other.c_max;
      m1 = other.m1;
      m2 = other.m2;
      return;
    }
    int64_t combined = count + other.count;
    double delta = other.m1 - m1;
    m2 = m2 + other.m2 +
         (delta * delta * static_cast<double>(count) *
          static_cast<double>(other.count) / static_cast<double>(combined));
    m1 = (m1 * count + other.m1 * other.count) / static_cast<double>(combined);
    c_min = std::min(c_min, other.c_min);
    c_max = std::max(c_max, other.c_max);
    count = combined;
  }
};

#endif  // LIVE_STATS_H_INCLUDED
