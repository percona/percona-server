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

/**
  @file vector-common/vector_distance.cc

  Euclidean, cosine and dot-product distance for VECTOR columns (float32).

  Public entry points — vector_distance_euclidean_squared(),
  vector_distance_cosine(), vector_distance_dot(), vector_distance_manhattan()
  — are declared in
  vector-common/vector_distance.h.  Call init_vector_distance_functions() once
  before first use so the dispatch pointers are set to the best kernel for the
  host CPU.

  All kernels accept const char * and any byte alignment.  SIMD paths use
  unaligned load intrinsics (_mm_loadu_ps, _mm256_loadu_ps, _mm512_loadu_ps,
  vld1q_f32, svld1_f32) rather than aligned variants: VECTOR payloads may
  come from columns or misaligned SUBSTR blobs, and on modern x86/ARM CPUs
  unaligned loads have the same throughput as aligned loads when the data
  happens to be aligned.  Aligned intrinsics would fault on misaligned
  addresses without improving performance.  The one remaining slowdown is
  cache-line crossing: if a load spans a 64-byte cache-line boundary the
  CPU must fetch from two lines, which costs extra regardless of whether
  the instruction is MOVUPS or MOVAPS — that penalty depends on runtime
  address, not on choosing loadu vs load.  Scalar tails use memcpy to
  avoid UB on misaligned float *.

  Dim-aware dispatch (Opt 2)
  --------------------------
  Wide-tier kernels (AVX2, AVX-512, SVE2) have a minimum useful dimension: the
  SIMD body only fires when dims ≥ register_width (8 for AVX2, 16 for AVX-512).
  Below that threshold, calling a wide-tier kernel pays register-init overhead
  with no SIMD benefit.

  To avoid this regression, the public wrappers use two function pointer sets:
    g_*        — the widest available tier; used for dims ≥
  VECTOR_DISTANCE_WIDE_MIN_DIMS g_*_narrow — SSE4.2 / NEON (fills at dim ≥ 4);
  used for dims < VECTOR_DISTANCE_WIDE_MIN_DIMS

  SIMD tier model
  ---------------
  Kernels are grouped into four tiers. init_vector_distance_functions() selects
  the highest tier the CPU and OS support at runtime. Each SIMD function is
  compiled with its own GCC/Clang target attribute so this translation unit
  stays at the baseline ISA; no global -mavx2 / -march=native is required.

  +------+----------+--------------------------------+-------------------+
  | Tier | Name     | Optimization target            | Register width    |
  +------+----------+--------------------------------+-------------------+
  |  0   | Scalar   | Any x86_64 / ARM64 (fallback)  | 32/64-bit         |
  |  1   | Legacy   | SSE4.2 (Intel/AMD) / NEON (ARM)| 128-bit           |
  |  2   | Standard | AVX2 + FMA (Intel/AMD)         | 256-bit           |
  |  3   | Ultra    | AVX-512 (Intel/AMD) / SVE2(ARM)| 512-bit+ (VLA)    |
  +------+----------+--------------------------------+-------------------+

  Tier 0 — Scalar (all architectures)
    euclidean_scalar, cosine_scalar, dot_product_scalar, manhattan_scalar
    Default function pointers; also forced by
    init_vector_distance_functions_tier(VectorDistanceTier::Scalar).

  Tier 1 — Legacy
    x86_64: euclidean_sse, cosine_sse, dot_product_sse, manhattan_sse
            (target("sse4.2"), 4 floats/iter)  cpu_has_sse42()
    aarch64: euclidean_neon, cosine_neon, dot_product_neon, manhattan_neon
             (target("+simd"), 4 floats/iter)  Always enabled on ARMv8.

  Tier 2 — Standard (x86_64 only)
    euclidean_avx2, cosine_avx2, dot_product_avx2, manhattan_avx2
    (target("avx2"), 8 floats/iter)  cpu_has_avx2_fma()

  Tier 3 — Ultra
    x86_64: euclidean_avx512, cosine_avx512, dot_product_avx512,
  manhattan_avx512 (target("avx512f"), 16 floats/iter)  cpu_has_avx512f()
    aarch64: euclidean_sve2, cosine_sve2, dot_product_sve2, manhattan_sve2
             (target("+sve2"), scalable VLA)

  Runtime dispatch (init_vector_distance_functions)
  ------------------------------------
    x86_64:  AVX-512 -> AVX2+FMA -> SSE4.2 -> scalar (wide)
             SSE4.2 -> scalar (narrow, dims < VECTOR_DISTANCE_WIDE_MIN_DIMS)
    aarch64: NEON or SVE2 (wide); NEON (narrow)
    other:   scalar only (no-op init)
    _WIN32 (x64 and ARM64): scalar only — SIMD tiers compiled out at build time.

  Euclidean kernels (euclidean_*) return sum((a[i]-b[i])²) without sqrt.
  SQL applies std::sqrt for the EUCLIDEAN metric; EUCLIDEAN_SQUARED uses the
  kernel result directly.

  Cosine distance returns +Inf as a sentinel when either vector has zero norm
  (undefined cosine); the SQL layer (Item_func_vector_distance::val_real)
  detects it via std::isinf and maps it to NULL.
*/

#include "vector-common/vector_distance.h"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>

#include "mysql/attribute.h"  // MY_ATTRIBUTE

// Platform guards — mirror ut0crc32.h:53-69
//
// On _WIN32 (x64 and ARM64) SIMD tiers are not wired up; scalar-only,
// same pragmatic approach as CRC32_DEFAULT in ut0crc32.h.
#if !defined(_WIN32)
#if defined(__x86_64__) || defined(_M_X64)
#define VECTOR_DISTANCE_x86_64
#elif defined(__aarch64__) || defined(_M_ARM64)
#define VECTOR_DISTANCE_AARCH64
#endif
#endif

#if !defined(VECTOR_DISTANCE_x86_64) && !defined(VECTOR_DISTANCE_AARCH64)
#define VECTOR_DISTANCE_DEFAULT
#endif

// SVE2 is opt-in: it requires the toolchain to compile the unit with
// __ARM_FEATURE_SVE2 (e.g. -march=armv8-a+sve2 or armv9-a). Without that we
// keep NEON-only behaviour and avoid pulling in <arm_sve.h>.
#if defined(VECTOR_DISTANCE_AARCH64) && defined(__ARM_FEATURE_SVE2)
#define VECTOR_DISTANCE_HAS_SVE2
#endif

// ---------------------------------------------------------------------------
// Scalar kernels — always compiled, safe on every architecture.
// Take const char * so they can handle any byte alignment: dereferencing a
// float * that is not suitably aligned is UB, so each element is fetched
// with memcpy into a local float instead.  Compilers see the fixed 4-byte
// size and emit a single load instruction — no function call, no overhead.
// Scalar path accumulates in double.  Inputs are float32, but double
// avoids overflow/precision loss (e.g. (2e38)² is Inf in float, finite
// in double) and matches the SIMD reduction path below.
// ---------------------------------------------------------------------------

static double euclidean_scalar(const char *a_raw, const char *b_raw,
                               uint32_t dims) {
  double result = 0.0;
  for (uint32_t i = 0; i < dims; i++) {
    float av, bv;
    memcpy(&av, a_raw + i * sizeof(float), sizeof(float));
    memcpy(&bv, b_raw + i * sizeof(float), sizeof(float));
    const double d = av - bv;
    result += d * d;
  }
  return result;
}

static double cosine_scalar(const char *a_raw, const char *b_raw,
                            uint32_t dims) {
  double ab = 0.0, norm_a = 0.0, norm_b = 0.0;
  for (uint32_t i = 0; i < dims; i++) {
    float av, bv;
    memcpy(&av, a_raw + i * sizeof(float), sizeof(float));
    memcpy(&bv, b_raw + i * sizeof(float), sizeof(float));
    ab += (double)av * bv;
    norm_a += (double)av * av;
    norm_b += (double)bv * bv;
  }
  const double denom = sqrt(norm_a * norm_b);
  // +Inf sentinel: zero-denom means undefined cosine (zero-vector input).
  if (denom == 0.0) return std::numeric_limits<double>::infinity();
  return 1.0 - ab / denom;
}

static double dot_product_scalar(const char *a_raw, const char *b_raw,
                                 uint32_t dims) {
  double ab = 0.0;
  for (uint32_t i = 0; i < dims; i++) {
    float av, bv;
    memcpy(&av, a_raw + i * sizeof(float), sizeof(float));
    memcpy(&bv, b_raw + i * sizeof(float), sizeof(float));
    ab += (double)av * bv;
  }
  return ab;
}

static double manhattan_scalar(const char *a_raw, const char *b_raw,
                               uint32_t dims) {
  double result = 0.0;
  for (uint32_t i = 0; i < dims; i++) {
    float av, bv;
    memcpy(&av, a_raw + i * sizeof(float), sizeof(float));
    memcpy(&bv, b_raw + i * sizeof(float), sizeof(float));
    const double d = (double)av - bv;
    result += std::fabs(d);
  }
  return result;
}

// ---------------------------------------------------------------------------
// Function pointers — initialized to scalar; init_vector_distance_functions()
// may promote. Wide pointers (g_*) are used for dims ≥
// VECTOR_DISTANCE_WIDE_MIN_DIMS. Narrow pointers (g_*_narrow) are used for dims
// < VECTOR_DISTANCE_WIDE_MIN_DIMS to avoid dispatching to AVX-512/AVX2 when the
// SIMD loop cannot fire (needs ≥ 16/8 elements).
// ---------------------------------------------------------------------------

using vector_distance_fn_t = double (*)(const char *, const char *, uint32_t);

static vector_distance_fn_t g_euclidean = euclidean_scalar;
static vector_distance_fn_t g_cosine = cosine_scalar;
static vector_distance_fn_t g_dot_product = dot_product_scalar;
static vector_distance_fn_t g_manhattan = manhattan_scalar;

static vector_distance_fn_t g_euclidean_narrow = euclidean_scalar;
static vector_distance_fn_t g_cosine_narrow = cosine_scalar;
static vector_distance_fn_t g_dot_product_narrow = dot_product_scalar;
static vector_distance_fn_t g_manhattan_narrow = manhattan_scalar;

static VectorDistanceTier g_wide_tier = VectorDistanceTier::Scalar;
static VectorDistanceTier g_narrow_tier = VectorDistanceTier::Scalar;

// ---------------------------------------------------------------------------
// SIMD kernels — see file header for the full tier table and dispatch order.
// ---------------------------------------------------------------------------

#ifdef VECTOR_DISTANCE_x86_64

#include <immintrin.h>

// CPU feature detection -----------------------------------------------------

static bool cpu_has_sse42() {
#if defined(__GNUC__) || defined(__clang__)
  return __builtin_cpu_supports("sse4.2");
#else
  return false;
#endif
}

static bool cpu_has_avx2_fma() {
#if defined(__GNUC__) || defined(__clang__)
  return __builtin_cpu_supports("avx2") && __builtin_cpu_supports("fma");
#else
  return false;
#endif
}

// __builtin_cpu_supports already gates AVX-512 on OS support: libgcc and
// compiler-rt only report the feature when OSXSAVE is set and XCR0 enables
// the full ZMM/opmask state, so no manual XGETBV check is needed (hnswlib
// does it by hand only because it reads raw CPUID bits).
static bool cpu_has_avx512f() {
#if defined(__GNUC__) || defined(__clang__)
  return __builtin_cpu_supports("avx512f");
#else
  return false;
#endif
}

// SIMD loops accumulate in float32 registers for full vector width.
// Each kernel promotes to double once (horizontal sum + scalar tail);
// float32-only reduction would not be faster and loses precision.
// However, individual float32 products (d*d for Euclidean, a[i]*b[i] for
// cosine/dot) can overflow to +Inf for extreme float32 inputs (e.g. element
// difference near FLT_MAX).  Each kernel checks for a non-finite horizontal
// sum and falls back to the scalar path, which uses double throughout.

// Tier 1 — SSE4.2, 4 floats per iteration -----------------------------------

// Horizontal sum: reduce the 4 float lanes of an SSE register to one float
// (SSE counterpart of the AVX-512 _mm512_reduce_add_ps intrinsic).
MY_ATTRIBUTE((target("sse4.2")))
static float hsum128_sse(__m128 v) {
  __m128 s = _mm_hadd_ps(v, v);
  s = _mm_hadd_ps(s, s);
  return _mm_cvtss_f32(s);
}

MY_ATTRIBUTE((target("sse4.2")))
static double euclidean_sse(const char *a_raw, const char *b_raw,
                            uint32_t dims) {
  const float *a = reinterpret_cast<const float *>(a_raw);
  const float *b = reinterpret_cast<const float *>(b_raw);
  __m128 sum = _mm_setzero_ps();
  uint32_t i = 0;
  for (; i + 4 <= dims; i += 4) {
    __m128 d = _mm_sub_ps(_mm_loadu_ps(a + i), _mm_loadu_ps(b + i));
    sum = _mm_add_ps(sum, _mm_mul_ps(d, d));
  }
  double result = hsum128_sse(sum);
  if (!std::isfinite(result)) return euclidean_scalar(a_raw, b_raw, dims);
  for (; i < dims; i++) {
    float av, bv;
    memcpy(&av, a_raw + i * sizeof(float), sizeof(float));
    memcpy(&bv, b_raw + i * sizeof(float), sizeof(float));
    const double d = av - bv;
    result += d * d;
  }
  return result;
}

MY_ATTRIBUTE((target("sse4.2")))
static double cosine_sse(const char *a_raw, const char *b_raw, uint32_t dims) {
  const float *a = reinterpret_cast<const float *>(a_raw);
  const float *b = reinterpret_cast<const float *>(b_raw);
  __m128 vec_ab = _mm_setzero_ps();
  __m128 vec_norm_a = _mm_setzero_ps();
  __m128 vec_norm_b = _mm_setzero_ps();
  uint32_t i = 0;
  for (; i + 4 <= dims; i += 4) {
    __m128 vec_a = _mm_loadu_ps(a + i);
    __m128 vec_b = _mm_loadu_ps(b + i);
    vec_ab = _mm_add_ps(vec_ab, _mm_mul_ps(vec_a, vec_b));
    vec_norm_a = _mm_add_ps(vec_norm_a, _mm_mul_ps(vec_a, vec_a));
    vec_norm_b = _mm_add_ps(vec_norm_b, _mm_mul_ps(vec_b, vec_b));
  }
  double ab = hsum128_sse(vec_ab);
  double norm_a = hsum128_sse(vec_norm_a);
  double norm_b = hsum128_sse(vec_norm_b);
  if (!std::isfinite(ab) || !std::isfinite(norm_a) || !std::isfinite(norm_b))
    return cosine_scalar(a_raw, b_raw, dims);
  for (; i < dims; i++) {
    float av, bv;
    memcpy(&av, a_raw + i * sizeof(float), sizeof(float));
    memcpy(&bv, b_raw + i * sizeof(float), sizeof(float));
    ab += (double)av * bv;
    norm_a += (double)av * av;
    norm_b += (double)bv * bv;
  }
  const double denom = sqrt(norm_a * norm_b);
  if (denom == 0.0) return std::numeric_limits<double>::infinity();
  return 1.0 - ab / denom;
}

MY_ATTRIBUTE((target("sse4.2")))
static double dot_product_sse(const char *a_raw, const char *b_raw,
                              uint32_t dims) {
  const float *a = reinterpret_cast<const float *>(a_raw);
  const float *b = reinterpret_cast<const float *>(b_raw);
  __m128 vec_ab = _mm_setzero_ps();
  uint32_t i = 0;
  for (; i + 4 <= dims; i += 4)
    vec_ab = _mm_add_ps(vec_ab,
                        _mm_mul_ps(_mm_loadu_ps(a + i), _mm_loadu_ps(b + i)));
  double ab = hsum128_sse(vec_ab);
  if (!std::isfinite(ab)) return dot_product_scalar(a_raw, b_raw, dims);
  for (; i < dims; i++) {
    float av, bv;
    memcpy(&av, a_raw + i * sizeof(float), sizeof(float));
    memcpy(&bv, b_raw + i * sizeof(float), sizeof(float));
    ab += (double)av * bv;
  }
  return ab;
}

MY_ATTRIBUTE((target("sse4.2")))
static double manhattan_sse(const char *a_raw, const char *b_raw,
                            uint32_t dims) {
  const float *a = reinterpret_cast<const float *>(a_raw);
  const float *b = reinterpret_cast<const float *>(b_raw);
  // SSE has no float abs intrinsic.  -0.0f is a float with only the sign bit
  // set, so andnot(sign_mask, d) clears each lane's sign bit: fabs(d) per lane.
  const __m128 sign_mask = _mm_set1_ps(-0.0f);
  __m128 sum = _mm_setzero_ps();
  uint32_t i = 0;
  for (; i + 4 <= dims; i += 4) {
    __m128 d = _mm_sub_ps(_mm_loadu_ps(a + i), _mm_loadu_ps(b + i));
    sum = _mm_add_ps(sum, _mm_andnot_ps(sign_mask, d));
  }
  double result = hsum128_sse(sum);
  if (!std::isfinite(result)) return manhattan_scalar(a_raw, b_raw, dims);
  for (; i < dims; i++) {
    float av, bv;
    memcpy(&av, a_raw + i * sizeof(float), sizeof(float));
    memcpy(&bv, b_raw + i * sizeof(float), sizeof(float));
    result += std::fabs((double)av - bv);
  }
  return result;
}

// Tier 2 — AVX2 + FMA, 8 floats per iteration -------------------------------

// Horizontal sum: reduce the 8 float lanes of an AVX2 register to one float
// (AVX2 counterpart of the AVX-512 _mm512_reduce_add_ps intrinsic).
// Lambdas don't inherit a function's target attribute in GCC; use a static
// helper so _mm_hadd_ps and friends are compiled with the AVX2 ISA.
MY_ATTRIBUTE((target("avx2")))
static float hsum256(__m256 v) {
  __m128 lo = _mm256_castps256_ps128(v);
  __m128 hi = _mm256_extractf128_ps(v, 1);
  __m128 s = _mm_add_ps(lo, hi);
  s = _mm_hadd_ps(s, s);
  s = _mm_hadd_ps(s, s);
  return _mm_cvtss_f32(s);
}

MY_ATTRIBUTE((target("avx2,fma")))
static double euclidean_avx2(const char *a_raw, const char *b_raw,
                             uint32_t dims) {
  const float *a = reinterpret_cast<const float *>(a_raw);
  const float *b = reinterpret_cast<const float *>(b_raw);
  __m256 sum = _mm256_setzero_ps();
  uint32_t i = 0;
  for (; i + 8 <= dims; i += 8) {
    __m256 d = _mm256_sub_ps(_mm256_loadu_ps(a + i), _mm256_loadu_ps(b + i));
    sum = _mm256_fmadd_ps(d, d, sum);
  }
  double result = hsum256(sum);
  if (!std::isfinite(result)) return euclidean_scalar(a_raw, b_raw, dims);
  for (; i < dims; i++) {
    float av, bv;
    memcpy(&av, a_raw + i * sizeof(float), sizeof(float));
    memcpy(&bv, b_raw + i * sizeof(float), sizeof(float));
    const double d = av - bv;
    result += d * d;
  }
  return result;
}

MY_ATTRIBUTE((target("avx2,fma")))
static double cosine_avx2(const char *a_raw, const char *b_raw, uint32_t dims) {
  const float *a = reinterpret_cast<const float *>(a_raw);
  const float *b = reinterpret_cast<const float *>(b_raw);
  __m256 vec_ab = _mm256_setzero_ps();
  __m256 vec_norm_a = _mm256_setzero_ps();
  __m256 vec_norm_b = _mm256_setzero_ps();
  uint32_t i = 0;
  for (; i + 8 <= dims; i += 8) {
    __m256 vec_a = _mm256_loadu_ps(a + i);
    __m256 vec_b = _mm256_loadu_ps(b + i);
    vec_ab = _mm256_fmadd_ps(vec_a, vec_b, vec_ab);
    vec_norm_a = _mm256_fmadd_ps(vec_a, vec_a, vec_norm_a);
    vec_norm_b = _mm256_fmadd_ps(vec_b, vec_b, vec_norm_b);
  }
  double ab = hsum256(vec_ab);
  double norm_a = hsum256(vec_norm_a);
  double norm_b = hsum256(vec_norm_b);
  if (!std::isfinite(ab) || !std::isfinite(norm_a) || !std::isfinite(norm_b))
    return cosine_scalar(a_raw, b_raw, dims);
  for (; i < dims; i++) {
    float av, bv;
    memcpy(&av, a_raw + i * sizeof(float), sizeof(float));
    memcpy(&bv, b_raw + i * sizeof(float), sizeof(float));
    ab += (double)av * bv;
    norm_a += (double)av * av;
    norm_b += (double)bv * bv;
  }
  const double denom = sqrt(norm_a * norm_b);
  if (denom == 0.0) return std::numeric_limits<double>::infinity();
  return 1.0 - ab / denom;
}

MY_ATTRIBUTE((target("avx2,fma")))
static double dot_product_avx2(const char *a_raw, const char *b_raw,
                               uint32_t dims) {
  const float *a = reinterpret_cast<const float *>(a_raw);
  const float *b = reinterpret_cast<const float *>(b_raw);
  __m256 vec_ab = _mm256_setzero_ps();
  uint32_t i = 0;
  for (; i + 8 <= dims; i += 8)
    vec_ab =
        _mm256_fmadd_ps(_mm256_loadu_ps(a + i), _mm256_loadu_ps(b + i), vec_ab);
  double ab = hsum256(vec_ab);
  if (!std::isfinite(ab)) return dot_product_scalar(a_raw, b_raw, dims);
  for (; i < dims; i++) {
    float av, bv;
    memcpy(&av, a_raw + i * sizeof(float), sizeof(float));
    memcpy(&bv, b_raw + i * sizeof(float), sizeof(float));
    ab += (double)av * bv;
  }
  return ab;
}

MY_ATTRIBUTE((target("avx2")))
static double manhattan_avx2(const char *a_raw, const char *b_raw,
                             uint32_t dims) {
  const float *a = reinterpret_cast<const float *>(a_raw);
  const float *b = reinterpret_cast<const float *>(b_raw);
  // AVX2 has no float abs intrinsic.  -0.0f is a float with only the sign bit
  // set, so andnot(sign_mask, d) clears each lane's sign bit: fabs(d) per lane.
  const __m256 sign_mask = _mm256_set1_ps(-0.0f);
  __m256 sum = _mm256_setzero_ps();
  uint32_t i = 0;
  for (; i + 8 <= dims; i += 8) {
    __m256 d = _mm256_sub_ps(_mm256_loadu_ps(a + i), _mm256_loadu_ps(b + i));
    sum = _mm256_add_ps(sum, _mm256_andnot_ps(sign_mask, d));
  }
  double result = hsum256(sum);
  if (!std::isfinite(result)) return manhattan_scalar(a_raw, b_raw, dims);
  for (; i < dims; i++) {
    float av, bv;
    memcpy(&av, a_raw + i * sizeof(float), sizeof(float));
    memcpy(&bv, b_raw + i * sizeof(float), sizeof(float));
    result += std::fabs((double)av - bv);
  }
  return result;
}

// Tier 3 — AVX-512F, 16 floats per iteration --------------------------------

MY_ATTRIBUTE((target("avx512f")))
static double euclidean_avx512(const char *a_raw, const char *b_raw,
                               uint32_t dims) {
  const float *a = reinterpret_cast<const float *>(a_raw);
  const float *b = reinterpret_cast<const float *>(b_raw);
  __m512 sum = _mm512_setzero_ps();
  uint32_t i = 0;
  for (; i + 16 <= dims; i += 16) {
    __m512 d = _mm512_sub_ps(_mm512_loadu_ps(a + i), _mm512_loadu_ps(b + i));
    sum = _mm512_fmadd_ps(d, d, sum);
  }
  double result = _mm512_reduce_add_ps(sum);
  if (!std::isfinite(result)) return euclidean_scalar(a_raw, b_raw, dims);
  for (; i < dims; i++) {
    float av, bv;
    memcpy(&av, a_raw + i * sizeof(float), sizeof(float));
    memcpy(&bv, b_raw + i * sizeof(float), sizeof(float));
    const double d = av - bv;
    result += d * d;
  }
  return result;
}

MY_ATTRIBUTE((target("avx512f")))
static double cosine_avx512(const char *a_raw, const char *b_raw,
                            uint32_t dims) {
  const float *a = reinterpret_cast<const float *>(a_raw);
  const float *b = reinterpret_cast<const float *>(b_raw);
  __m512 vec_ab = _mm512_setzero_ps();
  __m512 vec_norm_a = _mm512_setzero_ps();
  __m512 vec_norm_b = _mm512_setzero_ps();
  uint32_t i = 0;
  for (; i + 16 <= dims; i += 16) {
    __m512 vec_a = _mm512_loadu_ps(a + i);
    __m512 vec_b = _mm512_loadu_ps(b + i);
    vec_ab = _mm512_fmadd_ps(vec_a, vec_b, vec_ab);
    vec_norm_a = _mm512_fmadd_ps(vec_a, vec_a, vec_norm_a);
    vec_norm_b = _mm512_fmadd_ps(vec_b, vec_b, vec_norm_b);
  }
  double ab = _mm512_reduce_add_ps(vec_ab);
  double norm_a = _mm512_reduce_add_ps(vec_norm_a);
  double norm_b = _mm512_reduce_add_ps(vec_norm_b);
  if (!std::isfinite(ab) || !std::isfinite(norm_a) || !std::isfinite(norm_b))
    return cosine_scalar(a_raw, b_raw, dims);
  for (; i < dims; i++) {
    float av, bv;
    memcpy(&av, a_raw + i * sizeof(float), sizeof(float));
    memcpy(&bv, b_raw + i * sizeof(float), sizeof(float));
    ab += (double)av * bv;
    norm_a += (double)av * av;
    norm_b += (double)bv * bv;
  }
  const double denom = sqrt(norm_a * norm_b);
  if (denom == 0.0) return std::numeric_limits<double>::infinity();
  return 1.0 - ab / denom;
}

MY_ATTRIBUTE((target("avx512f")))
static double dot_product_avx512(const char *a_raw, const char *b_raw,
                                 uint32_t dims) {
  const float *a = reinterpret_cast<const float *>(a_raw);
  const float *b = reinterpret_cast<const float *>(b_raw);
  __m512 vec_ab = _mm512_setzero_ps();
  uint32_t i = 0;
  for (; i + 16 <= dims; i += 16)
    vec_ab =
        _mm512_fmadd_ps(_mm512_loadu_ps(a + i), _mm512_loadu_ps(b + i), vec_ab);
  double ab = _mm512_reduce_add_ps(vec_ab);
  if (!std::isfinite(ab)) return dot_product_scalar(a_raw, b_raw, dims);
  for (; i < dims; i++) {
    float av, bv;
    memcpy(&av, a_raw + i * sizeof(float), sizeof(float));
    memcpy(&bv, b_raw + i * sizeof(float), sizeof(float));
    ab += (double)av * bv;
  }
  return ab;
}

MY_ATTRIBUTE((target("avx512f")))
static double manhattan_avx512(const char *a_raw, const char *b_raw,
                               uint32_t dims) {
  const float *a = reinterpret_cast<const float *>(a_raw);
  const float *b = reinterpret_cast<const float *>(b_raw);
  __m512 sum = _mm512_setzero_ps();
  uint32_t i = 0;
  for (; i + 16 <= dims; i += 16) {
    __m512 d = _mm512_sub_ps(_mm512_loadu_ps(a + i), _mm512_loadu_ps(b + i));
    sum = _mm512_add_ps(sum, _mm512_abs_ps(d));
  }
  double result = _mm512_reduce_add_ps(sum);
  if (!std::isfinite(result)) return manhattan_scalar(a_raw, b_raw, dims);
  for (; i < dims; i++) {
    float av, bv;
    memcpy(&av, a_raw + i * sizeof(float), sizeof(float));
    memcpy(&bv, b_raw + i * sizeof(float), sizeof(float));
    result += std::fabs((double)av - bv);
  }
  return result;
}

#endif  // VECTOR_DISTANCE_x86_64

#ifdef VECTOR_DISTANCE_AARCH64

#include <arm_neon.h>

// Tier 1 — NEON (Advanced SIMD), 4 floats per iteration ---------------------
// Fused multiply-add (vfmaq_f32 -> FMLA) is baseline ARMv8-A, so unlike the
// x86 SSE4.2 tier no separate feature gate is needed; single rounding matches
// the AVX2/AVX-512/SVE2 tiers.

// Horizontal sum: reduce the 4 float lanes of a NEON register to one float
// (NEON counterpart of the AVX-512 _mm512_reduce_add_ps intrinsic).
// Same lambda issue applies on NEON; extract as a static attributed helper.
MY_ATTRIBUTE((target("+simd")))
static float hsum4(float32x4_t v) {
  float32x2_t s = vadd_f32(vget_low_f32(v), vget_high_f32(v));
  s = vpadd_f32(s, s);
  return vget_lane_f32(s, 0);
}

MY_ATTRIBUTE((target("+simd")))
static double euclidean_neon(const char *a_raw, const char *b_raw,
                             uint32_t dims) {
  const float *a = reinterpret_cast<const float *>(a_raw);
  const float *b = reinterpret_cast<const float *>(b_raw);
  float32x4_t sum = vdupq_n_f32(0.0f);
  uint32_t i = 0;
  for (; i + 4 <= dims; i += 4) {
    float32x4_t d = vsubq_f32(vld1q_f32(a + i), vld1q_f32(b + i));
    sum = vfmaq_f32(sum, d, d);
  }
  double result = hsum4(sum);
  if (!std::isfinite(result)) return euclidean_scalar(a_raw, b_raw, dims);
  for (; i < dims; i++) {
    float av, bv;
    memcpy(&av, a_raw + i * sizeof(float), sizeof(float));
    memcpy(&bv, b_raw + i * sizeof(float), sizeof(float));
    const double d = av - bv;
    result += d * d;
  }
  return result;
}

MY_ATTRIBUTE((target("+simd")))
static double cosine_neon(const char *a_raw, const char *b_raw, uint32_t dims) {
  const float *a = reinterpret_cast<const float *>(a_raw);
  const float *b = reinterpret_cast<const float *>(b_raw);
  float32x4_t vec_ab = vdupq_n_f32(0.0f);
  float32x4_t vec_norm_a = vdupq_n_f32(0.0f);
  float32x4_t vec_norm_b = vdupq_n_f32(0.0f);
  uint32_t i = 0;
  for (; i + 4 <= dims; i += 4) {
    float32x4_t vec_a = vld1q_f32(a + i);
    float32x4_t vec_b = vld1q_f32(b + i);
    vec_ab = vfmaq_f32(vec_ab, vec_a, vec_b);
    vec_norm_a = vfmaq_f32(vec_norm_a, vec_a, vec_a);
    vec_norm_b = vfmaq_f32(vec_norm_b, vec_b, vec_b);
  }
  double ab = hsum4(vec_ab);
  double norm_a = hsum4(vec_norm_a);
  double norm_b = hsum4(vec_norm_b);
  if (!std::isfinite(ab) || !std::isfinite(norm_a) || !std::isfinite(norm_b))
    return cosine_scalar(a_raw, b_raw, dims);
  for (; i < dims; i++) {
    float av, bv;
    memcpy(&av, a_raw + i * sizeof(float), sizeof(float));
    memcpy(&bv, b_raw + i * sizeof(float), sizeof(float));
    ab += (double)av * bv;
    norm_a += (double)av * av;
    norm_b += (double)bv * bv;
  }
  const double denom = sqrt(norm_a * norm_b);
  if (denom == 0.0) return std::numeric_limits<double>::infinity();
  return 1.0 - ab / denom;
}

MY_ATTRIBUTE((target("+simd")))
static double dot_product_neon(const char *a_raw, const char *b_raw,
                               uint32_t dims) {
  const float *a = reinterpret_cast<const float *>(a_raw);
  const float *b = reinterpret_cast<const float *>(b_raw);
  float32x4_t vec_ab = vdupq_n_f32(0.0f);
  uint32_t i = 0;
  for (; i + 4 <= dims; i += 4)
    vec_ab = vfmaq_f32(vec_ab, vld1q_f32(a + i), vld1q_f32(b + i));
  double ab = hsum4(vec_ab);
  if (!std::isfinite(ab)) return dot_product_scalar(a_raw, b_raw, dims);
  for (; i < dims; i++) {
    float av, bv;
    memcpy(&av, a_raw + i * sizeof(float), sizeof(float));
    memcpy(&bv, b_raw + i * sizeof(float), sizeof(float));
    ab += (double)av * bv;
  }
  return ab;
}

MY_ATTRIBUTE((target("+simd")))
static double manhattan_neon(const char *a_raw, const char *b_raw,
                             uint32_t dims) {
  const float *a = reinterpret_cast<const float *>(a_raw);
  const float *b = reinterpret_cast<const float *>(b_raw);
  float32x4_t sum = vdupq_n_f32(0.0f);
  uint32_t i = 0;
  for (; i + 4 <= dims; i += 4) {
    float32x4_t d = vsubq_f32(vld1q_f32(a + i), vld1q_f32(b + i));
    sum = vaddq_f32(sum, vabsq_f32(d));
  }
  double result = hsum4(sum);
  if (!std::isfinite(result)) return manhattan_scalar(a_raw, b_raw, dims);
  for (; i < dims; i++) {
    float av, bv;
    memcpy(&av, a_raw + i * sizeof(float), sizeof(float));
    memcpy(&bv, b_raw + i * sizeof(float), sizeof(float));
    result += std::fabs((double)av - bv);
  }
  return result;
}

#ifdef VECTOR_DISTANCE_HAS_SVE2

#include <arm_sve.h>
#include <sys/auxv.h>

// HWCAP2_SVE2 may not be exposed by older libc headers; the bit is stable
// in the Linux kernel UAPI (linux/include/uapi/asm-generic/hwcap.h).
#ifndef HWCAP2_SVE2
#define HWCAP2_SVE2 (1UL << 1)
#endif

// Tier 3 — SVE2, scalable (VLA) — predicated loads handle any alignment -----

static bool cpu_has_sve2() {
#if defined(__linux__)
  return (getauxval(AT_HWCAP2) & HWCAP2_SVE2) != 0;
#else
  return false;
#endif
}

MY_ATTRIBUTE((target("+sve2")))
static double euclidean_sve2(const char *a_raw, const char *b_raw,
                             uint32_t dims) {
  const float *a = reinterpret_cast<const float *>(a_raw);
  const float *b = reinterpret_cast<const float *>(b_raw);
  svfloat32_t sum = svdup_n_f32(0.0f);
  uint32_t i = 0;
  svbool_t pg = svwhilelt_b32_u32(i, dims);
  while (svptest_first(svptrue_b32(), pg)) {
    svfloat32_t vec_a = svld1_f32(pg, a + i);
    svfloat32_t vec_b = svld1_f32(pg, b + i);
    svfloat32_t d = svsub_f32_x(pg, vec_a, vec_b);
    // Merging form keeps inactive lanes of sum unchanged on the tail.
    sum = svmla_f32_m(pg, sum, d, d);
    i += svcntw();
    pg = svwhilelt_b32_u32(i, dims);
  }
  const double result = svaddv_f32(svptrue_b32(), sum);
  if (!std::isfinite(result)) return euclidean_scalar(a_raw, b_raw, dims);
  return result;
}

MY_ATTRIBUTE((target("+sve2")))
static double cosine_sve2(const char *a_raw, const char *b_raw, uint32_t dims) {
  const float *a = reinterpret_cast<const float *>(a_raw);
  const float *b = reinterpret_cast<const float *>(b_raw);
  svfloat32_t vec_ab = svdup_n_f32(0.0f);
  svfloat32_t vec_norm_a = svdup_n_f32(0.0f);
  svfloat32_t vec_norm_b = svdup_n_f32(0.0f);
  uint32_t i = 0;
  svbool_t pg = svwhilelt_b32_u32(i, dims);
  while (svptest_first(svptrue_b32(), pg)) {
    svfloat32_t vec_a = svld1_f32(pg, a + i);
    svfloat32_t vec_b = svld1_f32(pg, b + i);
    vec_ab = svmla_f32_m(pg, vec_ab, vec_a, vec_b);
    vec_norm_a = svmla_f32_m(pg, vec_norm_a, vec_a, vec_a);
    vec_norm_b = svmla_f32_m(pg, vec_norm_b, vec_b, vec_b);
    i += svcntw();
    pg = svwhilelt_b32_u32(i, dims);
  }
  const double ab = svaddv_f32(svptrue_b32(), vec_ab);
  const double norm_a = svaddv_f32(svptrue_b32(), vec_norm_a);
  const double norm_b = svaddv_f32(svptrue_b32(), vec_norm_b);
  if (!std::isfinite(ab) || !std::isfinite(norm_a) || !std::isfinite(norm_b))
    return cosine_scalar(a_raw, b_raw, dims);
  const double denom = sqrt(norm_a * norm_b);
  if (denom == 0.0) return std::numeric_limits<double>::infinity();
  return 1.0 - ab / denom;
}

MY_ATTRIBUTE((target("+sve2")))
static double dot_product_sve2(const char *a_raw, const char *b_raw,
                               uint32_t dims) {
  const float *a = reinterpret_cast<const float *>(a_raw);
  const float *b = reinterpret_cast<const float *>(b_raw);
  svfloat32_t vec_ab = svdup_n_f32(0.0f);
  uint32_t i = 0;
  svbool_t pg = svwhilelt_b32_u32(i, dims);
  while (svptest_first(svptrue_b32(), pg)) {
    svfloat32_t vec_a = svld1_f32(pg, a + i);
    svfloat32_t vec_b = svld1_f32(pg, b + i);
    vec_ab = svmla_f32_m(pg, vec_ab, vec_a, vec_b);
    i += svcntw();
    pg = svwhilelt_b32_u32(i, dims);
  }
  const double ab = svaddv_f32(svptrue_b32(), vec_ab);
  if (!std::isfinite(ab)) return dot_product_scalar(a_raw, b_raw, dims);
  return ab;
}

MY_ATTRIBUTE((target("+sve2")))
static double manhattan_sve2(const char *a_raw, const char *b_raw,
                             uint32_t dims) {
  const float *a = reinterpret_cast<const float *>(a_raw);
  const float *b = reinterpret_cast<const float *>(b_raw);
  svfloat32_t sum = svdup_n_f32(0.0f);
  uint32_t i = 0;
  svbool_t pg = svwhilelt_b32_u32(i, dims);
  while (svptest_first(svptrue_b32(), pg)) {
    svfloat32_t vec_a = svld1_f32(pg, a + i);
    svfloat32_t vec_b = svld1_f32(pg, b + i);
    svfloat32_t d = svsub_f32_x(pg, vec_a, vec_b);
    sum = svadd_f32_m(pg, sum, svabs_f32_x(pg, d));
    i += svcntw();
    pg = svwhilelt_b32_u32(i, dims);
  }
  const double result = svaddv_f32(svptrue_b32(), sum);
  if (!std::isfinite(result)) return manhattan_scalar(a_raw, b_raw, dims);
  return result;
}

#endif  // VECTOR_DISTANCE_HAS_SVE2

#endif  // VECTOR_DISTANCE_AARCH64

// Tier dispatch helpers — must follow all kernel definitions above.

static void apply_wide_tier(VectorDistanceTier tier) {
  g_wide_tier = tier;
  switch (tier) {
    case VectorDistanceTier::Scalar:
      g_euclidean = euclidean_scalar;
      g_cosine = cosine_scalar;
      g_dot_product = dot_product_scalar;
      g_manhattan = manhattan_scalar;
      break;
#ifdef VECTOR_DISTANCE_x86_64
    case VectorDistanceTier::Sse42:
      g_euclidean = euclidean_sse;
      g_cosine = cosine_sse;
      g_dot_product = dot_product_sse;
      g_manhattan = manhattan_sse;
      break;
    case VectorDistanceTier::Avx2:
      g_euclidean = euclidean_avx2;
      g_cosine = cosine_avx2;
      g_dot_product = dot_product_avx2;
      g_manhattan = manhattan_avx2;
      break;
    case VectorDistanceTier::Avx512f:
      g_euclidean = euclidean_avx512;
      g_cosine = cosine_avx512;
      g_dot_product = dot_product_avx512;
      g_manhattan = manhattan_avx512;
      break;
#endif
#ifdef VECTOR_DISTANCE_AARCH64
    case VectorDistanceTier::Neon:
      g_euclidean = euclidean_neon;
      g_cosine = cosine_neon;
      g_dot_product = dot_product_neon;
      g_manhattan = manhattan_neon;
      break;
#ifdef VECTOR_DISTANCE_HAS_SVE2
    case VectorDistanceTier::Sve2:
      g_euclidean = euclidean_sve2;
      g_cosine = cosine_sve2;
      g_dot_product = dot_product_sve2;
      g_manhattan = manhattan_sve2;
      break;
#endif
#endif
    default:
      break;
  }
}

static void apply_narrow_tier(VectorDistanceTier tier) {
  g_narrow_tier = tier;
  switch (tier) {
    case VectorDistanceTier::Scalar:
      g_euclidean_narrow = euclidean_scalar;
      g_cosine_narrow = cosine_scalar;
      g_dot_product_narrow = dot_product_scalar;
      g_manhattan_narrow = manhattan_scalar;
      break;
#ifdef VECTOR_DISTANCE_x86_64
    case VectorDistanceTier::Sse42:
      g_euclidean_narrow = euclidean_sse;
      g_cosine_narrow = cosine_sse;
      g_dot_product_narrow = dot_product_sse;
      g_manhattan_narrow = manhattan_sse;
      break;
#endif
#ifdef VECTOR_DISTANCE_AARCH64
    case VectorDistanceTier::Neon:
      g_euclidean_narrow = euclidean_neon;
      g_cosine_narrow = cosine_neon;
      g_dot_product_narrow = dot_product_neon;
      g_manhattan_narrow = manhattan_neon;
      break;
#ifdef VECTOR_DISTANCE_HAS_SVE2
    case VectorDistanceTier::Sve2:
      g_euclidean_narrow = euclidean_sve2;
      g_cosine_narrow = cosine_sve2;
      g_dot_product_narrow = dot_product_sve2;
      g_manhattan_narrow = manhattan_sve2;
      break;
#endif
#endif
    default:
      break;
  }
}

// init_vector_distance_functions — promote g_* and g_*_narrow (see file header)

void init_vector_distance_functions() {
  apply_wide_tier(VectorDistanceTier::Scalar);
  apply_narrow_tier(VectorDistanceTier::Scalar);
#ifdef VECTOR_DISTANCE_x86_64
  // x86_64: Tier 3 -> Tier 2 -> Tier 1 -> Tier 0 (wide)
  if (cpu_has_avx512f()) {
    apply_wide_tier(VectorDistanceTier::Avx512f);
  } else if (cpu_has_avx2_fma()) {
    apply_wide_tier(VectorDistanceTier::Avx2);
  } else if (cpu_has_sse42()) {
    apply_wide_tier(VectorDistanceTier::Sse42);
  }
  // Narrow path: SSE4.2 fills at dim ≥ 4; use it when available.
  if (cpu_has_sse42()) {
    apply_narrow_tier(VectorDistanceTier::Sse42);
  }
#endif
#ifdef VECTOR_DISTANCE_AARCH64
  // Tier 1 (NEON), optionally Tier 3 (SVE2)
  apply_wide_tier(VectorDistanceTier::Neon);
  apply_narrow_tier(VectorDistanceTier::Neon);
#ifdef VECTOR_DISTANCE_HAS_SVE2
  if (cpu_has_sve2()) {  // Tier 3 over Tier 1 (wide only)
    apply_wide_tier(VectorDistanceTier::Sve2);
  }
#endif
#endif
}

bool vector_distance_tier_available(VectorDistanceTier tier) {
  switch (tier) {
    case VectorDistanceTier::Scalar:
      return true;
#ifdef VECTOR_DISTANCE_x86_64
    case VectorDistanceTier::Sse42:
      return cpu_has_sse42();
    case VectorDistanceTier::Avx2:
      return cpu_has_avx2_fma();
    case VectorDistanceTier::Avx512f:
      return cpu_has_avx512f();
#endif
#ifdef VECTOR_DISTANCE_AARCH64
    case VectorDistanceTier::Neon:
      return true;  // mandatory on ARMv8
    case VectorDistanceTier::Sve2:
#ifdef VECTOR_DISTANCE_HAS_SVE2
      return cpu_has_sve2();
#else
      return false;
#endif
#endif
    default:
      return false;
  }
}

void init_vector_distance_functions_tier(VectorDistanceTier tier) {
  switch (tier) {
    case VectorDistanceTier::Scalar:
      apply_wide_tier(VectorDistanceTier::Scalar);
      apply_narrow_tier(VectorDistanceTier::Scalar);
      break;
#ifdef VECTOR_DISTANCE_x86_64
    case VectorDistanceTier::Sse42:
      apply_wide_tier(VectorDistanceTier::Sse42);
      apply_narrow_tier(VectorDistanceTier::Sse42);
      break;
    case VectorDistanceTier::Avx2:
      apply_wide_tier(VectorDistanceTier::Avx2);
      if (cpu_has_sse42()) apply_narrow_tier(VectorDistanceTier::Sse42);
      break;
    case VectorDistanceTier::Avx512f:
      apply_wide_tier(VectorDistanceTier::Avx512f);
      if (cpu_has_sse42()) apply_narrow_tier(VectorDistanceTier::Sse42);
      break;
#endif
#ifdef VECTOR_DISTANCE_AARCH64
    case VectorDistanceTier::Neon:
      apply_wide_tier(VectorDistanceTier::Neon);
      apply_narrow_tier(VectorDistanceTier::Neon);
      break;
#ifdef VECTOR_DISTANCE_HAS_SVE2
    case VectorDistanceTier::Sve2:
      apply_wide_tier(VectorDistanceTier::Sve2);
      apply_narrow_tier(VectorDistanceTier::Neon);
      break;
#endif
#endif
    default:
      // Unsupported tier on this build; callers must check
      // vector_distance_tier_available() first.
      assert(false);
      break;
  }
}

VectorDistanceTier vector_distance_wide_tier() { return g_wide_tier; }

VectorDistanceTier vector_distance_narrow_tier() { return g_narrow_tier; }

const char *vector_distance_tier_label(VectorDistanceTier tier) {
  switch (tier) {
    case VectorDistanceTier::Scalar:
      return "software scalar";
    case VectorDistanceTier::Sse42:
      return "SSE4.2";
    case VectorDistanceTier::Avx2:
      return "AVX2 and FMA";
    case VectorDistanceTier::Avx512f:
      return "AVX-512F";
    case VectorDistanceTier::Neon:
      return "NEON";
    case VectorDistanceTier::Sve2:
      return "SVE2";
  }
  return "unknown";
}

size_t vector_distance_dispatch_description(char *buf, size_t buf_len) {
  if (buf == nullptr || buf_len == 0) return 0;

  const VectorDistanceTier wide = g_wide_tier;
  const VectorDistanceTier narrow = g_narrow_tier;
  int n;
  if (wide == narrow) {
    if (wide == VectorDistanceTier::Scalar) {
      n = snprintf(buf, buf_len, " software scalar.");
    } else {
      n = snprintf(buf, buf_len, " hardware accelerated %s.",
                   vector_distance_tier_label(wide));
    }
  } else {
    n = snprintf(
        buf, buf_len,
        " hardware accelerated %s "
        "(dimensions >= %u) and %s (dimensions < %u).",
        vector_distance_tier_label(wide), VECTOR_DISTANCE_WIDE_MIN_DIMS,
        vector_distance_tier_label(narrow), VECTOR_DISTANCE_WIDE_MIN_DIMS);
  }
  if (n < 0) {
    buf[0] = '\0';
    return 0;
  }
  if (static_cast<size_t>(n) >= buf_len) return buf_len - 1;
  return static_cast<size_t>(n);
}

// Public wrappers — dim-aware dispatch: narrow path for dims <
// VECTOR_DISTANCE_WIDE_MIN_DIMS avoids sending small inputs to AVX-512/AVX2
// where the SIMD loop cannot fire.

double vector_distance_euclidean_squared(const char *a, const char *b,
                                         uint32_t dims) {
  return (dims < VECTOR_DISTANCE_WIDE_MIN_DIMS ? g_euclidean_narrow
                                               : g_euclidean)(a, b, dims);
}

double vector_distance_cosine(const char *a, const char *b, uint32_t dims) {
  return (dims < VECTOR_DISTANCE_WIDE_MIN_DIMS ? g_cosine_narrow : g_cosine)(
      a, b, dims);
}

double vector_distance_dot(const char *a, const char *b, uint32_t dims) {
  return (dims < VECTOR_DISTANCE_WIDE_MIN_DIMS ? g_dot_product_narrow
                                               : g_dot_product)(a, b, dims);
}

double vector_distance_manhattan(const char *a, const char *b, uint32_t dims) {
  return (dims < VECTOR_DISTANCE_WIDE_MIN_DIMS ? g_manhattan_narrow
                                               : g_manhattan)(a, b, dims);
}
