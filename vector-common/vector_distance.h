/* Copyright (c) 2026, Percona and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#pragma once

#include <cstddef>
#include <cstdint>

/** Set SIMD function pointers before first use; safe to call repeatedly. */
void init_vector_distance_functions();

/**
  SIMD tier identifiers.  Values match the tier table in vector_distance.cc.
  Tiers that do not apply to the host platform (e.g. SSE42 on aarch64) are
  always reported as unavailable by vector_distance_tier_available().
*/
enum class VectorDistanceTier {
  Scalar = 0,  ///< Tier 0 — plain C++ scalar
  Sse42,       ///< Tier 1 x86_64 — SSE4.2, 128-bit
  Avx2,        ///< Tier 2 x86_64 — AVX2 + FMA, 256-bit
  Avx512f,     ///< Tier 3 x86_64 — AVX-512F, 512-bit
  Neon,        ///< Tier 1 aarch64 — NEON, 128-bit
  Sve2,        ///< Tier 3 aarch64 — SVE2, scalable (VLA)
};

/**
  Return true when @p tier can be used on the current CPU and build.
  Scalar always returns true. Architecture-specific tiers return false on the
  wrong platform. SVE2 additionally requires the binary to have been compiled
  with __ARM_FEATURE_SVE2 and the OS kernel to advertise HWCAP2_SVE2.
*/
bool vector_distance_tier_available(VectorDistanceTier tier);

/**
  Set all dispatch pointers (wide and narrow) directly to @p tier without
  call_once protection. Intended for benchmarks that want to force a specific
  tier on a host that may support a higher one. Use
  VectorDistanceTier::Scalar to force scalar kernels. Callers must verify
  vector_distance_tier_available() first.
*/
void init_vector_distance_functions_tier(VectorDistanceTier tier);

/** Minimum dims for the wide SIMD kernel; below this, g_*_narrow is used. */
static constexpr uint32_t VECTOR_DISTANCE_WIDE_MIN_DIMS = 16;

/** Wide-path tier selected by init_vector_distance_functions() (dims >=
 * VECTOR_DISTANCE_WIDE_MIN_DIMS). */
VectorDistanceTier vector_distance_wide_tier();

/** Narrow-path tier selected by init_vector_distance_functions() (dims <
 * VECTOR_DISTANCE_WIDE_MIN_DIMS). */
VectorDistanceTier vector_distance_narrow_tier();

/**
  Stable English label for logs/tests, e.g. "AVX-512F", "AVX2 and FMA",
  "software scalar".
*/
const char *vector_distance_tier_label(VectorDistanceTier tier);

/**
  Write a human-readable dispatch summary into @p buf (NUL-terminated).
  Returns the number of bytes written, excluding the terminating NUL.
*/
size_t vector_distance_dispatch_description(char *buf, size_t buf_len);

/**
  Compute squared Euclidean distance: sum((a[i]-b[i])²).
  No sqrt — intended for ranking and as the shared kernel for SQL EUCLIDEAN
  (where sqrt is applied in Item_func_vector_distance::val_real).
  Accepts any byte alignment; SIMD kernels use unaligned loads internally.
  Returns double; SIMD accumulates in float32, reduction uses double for
  precision on large dims and extreme float32 values.
*/
double vector_distance_euclidean_squared(const char *a, const char *b,
                                         uint32_t dims);

/**
  Compute cosine distance between two float vectors encoded as raw bytes.
  Returns +Inf when either vector is all-zeros (undefined cosine — true
  zero-denominator); returns NaN when input elements are NaN/Inf (bad-data
  propagation). The caller must distinguish these two cases.
  Returns double; SIMD accumulates in float32, reduction uses double for
  precision on large dims and extreme float32 values.
*/
double vector_distance_cosine(const char *a, const char *b, uint32_t dims);

/**
  Compute dot product (inner product) between two float vectors encoded as raw
  bytes. Returns sum(a[i]*b[i]). Higher values indicate greater similarity.
  Always finite for finite inputs — no NaN edge cases.
  Returns double; SIMD accumulates in float32, reduction uses double for
  precision on large dims and extreme float32 values.
*/
double vector_distance_dot(const char *a, const char *b, uint32_t dims);

/**
  Compute Manhattan (L1) distance between two float vectors encoded as raw
  bytes. Returns sum(|a[i] - b[i]|). Always >= 0 for finite inputs. No NaN
  edge cases.
  Returns double; SIMD accumulates in float32, reduction uses double for
  precision on large dims and extreme float32 values.
*/
double vector_distance_manhattan(const char *a, const char *b, uint32_t dims);
