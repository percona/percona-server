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

#include <gtest/gtest.h>

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <random>
#include <string>
#include <vector>

#include "my_pointer_arithmetic.h"
#include "vector-common/vector_distance.h"

namespace vector_distance_unittest {

// ---------------------------------------------------------------------------
// Reference implementations — always scalar, double precision.
// Used to verify SIMD paths; independent of vector_distance.cc internals.
// ---------------------------------------------------------------------------

static double ref_euclidean_squared(const float *a, const float *b,
                                    uint32_t n) {
  double sum = 0.0;
  for (uint32_t i = 0; i < n; i++) {
    const double d = a[i] - b[i];
    sum += d * d;
  }
  return sum;
}

static double ref_euclidean(const float *a, const float *b, uint32_t n) {
  return std::sqrt(ref_euclidean_squared(a, b, n));
}

// Smallest positive offset from base that is not alignof(float)-aligned.
// A fixed +1 byte offset is not reliable: when the buffer sits at address
// % alignof(float) == alignof(float) - 1 (seen on Apple Silicon stacks),
// base+1 is still float-aligned.
static size_t misaligned_float_offset(uintptr_t base) {
  for (size_t offset = 1; offset < alignof(float); ++offset) {
    if ((base + offset) % alignof(float) != 0) return offset;
  }
  assert(false);
  return 1;
}

// Mirrors Item_func_vector_distance EUCLIDEAN branch.
static double euclidean_l2(const char *a, const char *b, uint32_t dims) {
  return std::sqrt(vector_distance_euclidean_squared(a, b, dims));
}

static double ref_dot_product(const float *a, const float *b, uint32_t n) {
  double ab = 0.0;
  for (uint32_t i = 0; i < n; i++) ab += (double)a[i] * b[i];
  return ab;
}

static double ref_cosine(const float *a, const float *b, uint32_t n) {
  double ab = 0.0, na = 0.0, nb = 0.0;
  for (uint32_t i = 0; i < n; i++) {
    ab += a[i] * b[i];
    na += a[i] * a[i];
    nb += b[i] * b[i];
  }
  const double denom = std::sqrt(na * nb);
  if (denom == 0.0) return std::numeric_limits<double>::infinity();
  return 1.0 - ab / denom;
}

static double ref_manhattan(const float *a, const float *b, uint32_t n) {
  double result = 0.0;
  for (uint32_t i = 0; i < n; i++) result += std::fabs((double)a[i] - b[i]);
  return result;
}

// ---------------------------------------------------------------------------
// Fixture — initialises dispatch pointers once per test suite
// ---------------------------------------------------------------------------

class VectorDistanceTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() { init_vector_distance_functions(); }
};

// ---------------------------------------------------------------------------
// Known-value correctness
// ---------------------------------------------------------------------------

TEST_F(VectorDistanceTest, EuclideanSquaredKnownValues) {
  // [0,0] → [3,4] = 25.0 (squared 3-4-5 right triangle)
  alignas(32) float a[] = {0.0f, 0.0f};
  alignas(32) float b[] = {3.0f, 4.0f};
  EXPECT_NEAR(
      vector_distance_euclidean_squared((const char *)a, (const char *)b, 2),
      25.0, 1e-6);

  // Identical vectors → distance 0
  alignas(32) float c[] = {1.0f, 2.0f, 3.0f};
  EXPECT_NEAR(
      vector_distance_euclidean_squared((const char *)c, (const char *)c, 3),
      0.0, 1e-9);
}

TEST_F(VectorDistanceTest, EuclideanKnownValues) {
  // [0,0] → [3,4] = 5.0 (3-4-5 right triangle)
  alignas(32) float a[] = {0.0f, 0.0f};
  alignas(32) float b[] = {3.0f, 4.0f};
  EXPECT_NEAR(euclidean_l2((const char *)a, (const char *)b, 2), 5.0, 1e-6);

  // Identical vectors → distance 0
  alignas(32) float c[] = {1.0f, 2.0f, 3.0f};
  EXPECT_NEAR(euclidean_l2((const char *)c, (const char *)c, 3), 0.0, 1e-9);
}

TEST_F(VectorDistanceTest, CosineKnownValues) {
  // Identical unit vectors → distance 0
  alignas(32) float same[] = {1.0f, 0.0f, 0.0f};
  EXPECT_NEAR(vector_distance_cosine((const char *)same, (const char *)same, 3),
              0.0, 1e-6);

  // Orthogonal vectors → distance 1
  alignas(32) float x[] = {1.0f, 0.0f};
  alignas(32) float y[] = {0.0f, 1.0f};
  EXPECT_NEAR(vector_distance_cosine((const char *)x, (const char *)y, 2), 1.0,
              1e-6);

  // Anti-parallel → distance 2
  alignas(32) float pos[] = {1.0f, 1.0f};
  alignas(32) float neg[] = {-1.0f, -1.0f};
  EXPECT_NEAR(vector_distance_cosine((const char *)pos, (const char *)neg, 2),
              2.0, 1e-6);
}

TEST_F(VectorDistanceTest, DotProductKnownValues) {
  // Orthogonal vectors → dot product 0
  alignas(32) float x[] = {1.0f, 0.0f};
  alignas(32) float y[] = {0.0f, 1.0f};
  EXPECT_NEAR(vector_distance_dot((const char *)x, (const char *)y, 2), 0.0,
              1e-9);

  // Identical unit vector → dot product 1
  alignas(32) float u[] = {1.0f, 0.0f, 0.0f};
  EXPECT_NEAR(vector_distance_dot((const char *)u, (const char *)u, 3), 1.0,
              1e-9);

  // Known values: [1,2,3]·[4,5,6] = 4+10+18 = 32
  alignas(32) float a[] = {1.0f, 2.0f, 3.0f};
  alignas(32) float b[] = {4.0f, 5.0f, 6.0f};
  EXPECT_NEAR(vector_distance_dot((const char *)a, (const char *)b, 3), 32.0,
              1e-6);
}

TEST_F(VectorDistanceTest, ManhattanKnownValues) {
  // Identical vectors → distance 0
  alignas(32) float same[] = {1.0f, 2.0f, 3.0f};
  EXPECT_NEAR(
      vector_distance_manhattan((const char *)same, (const char *)same, 3), 0.0,
      1e-9);

  // [0,0] → [3,4] = |3| + |4| = 7 (compare: Euclidean gives 5)
  alignas(32) float a[] = {0.0f, 0.0f};
  alignas(32) float b[] = {3.0f, 4.0f};
  EXPECT_NEAR(vector_distance_manhattan((const char *)a, (const char *)b, 2),
              7.0, 1e-6);

  // [1,7,3,16,5] → [1,2,3,4,5] = 0+5+0+12+0 = 17
  alignas(32) float c[] = {1.0f, 7.0f, 3.0f, 16.0f, 5.0f};
  alignas(32) float d[] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
  EXPECT_NEAR(vector_distance_manhattan((const char *)c, (const char *)d, 5),
              17.0, 1e-6);
}

// ---------------------------------------------------------------------------
// Zero-vector guard: cosine must return +Inf sentinel (val_real maps to NULL)
// ---------------------------------------------------------------------------

TEST_F(VectorDistanceTest, CosineZeroVectorReturnsInf) {
  alignas(32) float z[] = {0.0f, 0.0f};
  alignas(32) float a[] = {1.0f, 2.0f};
  EXPECT_TRUE(
      std::isinf(vector_distance_cosine((const char *)z, (const char *)a, 2)));
  EXPECT_TRUE(
      std::isinf(vector_distance_cosine((const char *)a, (const char *)z, 2)));
  EXPECT_TRUE(
      std::isinf(vector_distance_cosine((const char *)z, (const char *)z, 2)));
}

// ---------------------------------------------------------------------------
// Unaligned path: misaligned buffer must give the same result as aligned
// ---------------------------------------------------------------------------

static void CheckUnalignedMatchesAligned(uint32_t dims) {
  SCOPED_TRACE(::testing::Message() << "dims=" << dims);

  std::mt19937 rng(dims);
  std::uniform_real_distribution<float> dist(-10.0f, 10.0f);
  std::vector<float> fa(dims), fb(dims);
  for (auto &x : fa) x = dist(rng);
  for (auto &x : fb) x = dist(rng);

  std::vector<char> buf_a(dims * sizeof(float) + alignof(float));
  std::vector<char> buf_b(dims * sizeof(float) + alignof(float));
  const size_t off_a =
      misaligned_float_offset(reinterpret_cast<uintptr_t>(buf_a.data()));
  const size_t off_b =
      misaligned_float_offset(reinterpret_cast<uintptr_t>(buf_b.data()));
  std::memcpy(buf_a.data() + off_a, fa.data(), dims * sizeof(float));
  std::memcpy(buf_b.data() + off_b, fb.data(), dims * sizeof(float));

  const char *ma = buf_a.data() + off_a;
  const char *mb = buf_b.data() + off_b;
  ASSERT_FALSE(is_aligned_to(ma, alignof(float)));
  ASSERT_FALSE(is_aligned_to(mb, alignof(float)));

  const char *aa = (const char *)fa.data();
  const char *ab = (const char *)fb.data();

  EXPECT_DOUBLE_EQ(vector_distance_euclidean_squared(aa, ab, dims),
                   vector_distance_euclidean_squared(ma, mb, dims));
  EXPECT_DOUBLE_EQ(euclidean_l2(aa, ab, dims), euclidean_l2(ma, mb, dims));
  EXPECT_DOUBLE_EQ(vector_distance_cosine(aa, ab, dims),
                   vector_distance_cosine(ma, mb, dims));
  EXPECT_DOUBLE_EQ(vector_distance_dot(aa, ab, dims),
                   vector_distance_dot(ma, mb, dims));
  EXPECT_DOUBLE_EQ(vector_distance_manhattan(aa, ab, dims),
                   vector_distance_manhattan(ma, mb, dims));
}

TEST_F(VectorDistanceTest, UnalignedMatchesAligned) {
  // dims=8 is below VECTOR_DISTANCE_WIDE_MIN_DIMS (narrow tier: SSE4.2/NEON/
  // scalar). dims=35 is above it and odd, so it exercises the wide tier's
  // (AVX2/AVX-512/SVE2) loadu path together with its tail handling.
  CheckUnalignedMatchesAligned(8);
  CheckUnalignedMatchesAligned(35);
}

// ---------------------------------------------------------------------------
// SIMD parity: aligned result matches double-precision scalar reference.
// On CPUs without SIMD both sides run the same scalar code, so the test
// degenerates into an identity check — still a useful correctness signal.
// ---------------------------------------------------------------------------

TEST_F(VectorDistanceTest, EuclideanSquaredParityWithReference) {
  std::mt19937 rng(42);
  std::uniform_real_distribution<float> dist(-10.0f, 10.0f);

  for (uint32_t dims : {4u, 8u, 16u, 32u, 128u, 512u}) {
    // Use heap vectors; malloc guarantees at least 16-byte alignment,
    // which satisfies our alignof(float)=4 dispatch gate.
    std::vector<float> a(dims), b(dims);
    for (auto &x : a) x = dist(rng);
    for (auto &x : b) x = dist(rng);

    const double got = vector_distance_euclidean_squared(
        (const char *)a.data(), (const char *)b.data(), dims);
    const double ref = ref_euclidean_squared(a.data(), b.data(), dims);
    // Allow 0.01% relative tolerance for float-precision SIMD accumulation.
    EXPECT_NEAR(got, ref, ref * 1e-4 + 1e-9) << "dims=" << dims;
  }
}

TEST_F(VectorDistanceTest, EuclideanParityWithReference) {
  std::mt19937 rng(42);
  std::uniform_real_distribution<float> dist(-10.0f, 10.0f);

  for (uint32_t dims : {4u, 8u, 16u, 32u, 128u, 512u}) {
    std::vector<float> a(dims), b(dims);
    for (auto &x : a) x = dist(rng);
    for (auto &x : b) x = dist(rng);

    const double got =
        euclidean_l2((const char *)a.data(), (const char *)b.data(), dims);
    const double ref = ref_euclidean(a.data(), b.data(), dims);
    EXPECT_NEAR(got, ref, ref * 1e-4 + 1e-9) << "dims=" << dims;
  }
}

TEST_F(VectorDistanceTest, CosineParityWithReference) {
  std::mt19937 rng(123);
  std::uniform_real_distribution<float> dist(-5.0f, 5.0f);

  for (uint32_t dims : {4u, 8u, 16u, 32u, 128u, 512u}) {
    std::vector<float> a(dims), b(dims);
    for (auto &x : a) x = dist(rng);
    for (auto &x : b) x = dist(rng);

    const double got = vector_distance_cosine((const char *)a.data(),
                                              (const char *)b.data(), dims);
    const double ref = ref_cosine(a.data(), b.data(), dims);
    EXPECT_NEAR(got, ref, 1e-4) << "dims=" << dims;
  }
}

TEST_F(VectorDistanceTest, DotProductParityWithReference) {
  std::mt19937 rng(77);
  std::uniform_real_distribution<float> dist(-10.0f, 10.0f);

  for (uint32_t dims : {4u, 8u, 16u, 32u, 128u, 512u}) {
    std::vector<float> a(dims), b(dims);
    for (auto &x : a) x = dist(rng);
    for (auto &x : b) x = dist(rng);

    const double got = vector_distance_dot((const char *)a.data(),
                                           (const char *)b.data(), dims);
    const double ref = ref_dot_product(a.data(), b.data(), dims);
    EXPECT_NEAR(got, ref, std::abs(ref) * 1e-4 + 1e-9) << "dims=" << dims;
  }
}

TEST_F(VectorDistanceTest, ManhattanParityWithReference) {
  std::mt19937 rng(55);
  std::uniform_real_distribution<float> dist(-10.0f, 10.0f);

  for (uint32_t dims : {4u, 8u, 16u, 32u, 128u, 512u}) {
    std::vector<float> a(dims), b(dims);
    for (auto &x : a) x = dist(rng);
    for (auto &x : b) x = dist(rng);

    const double got = vector_distance_manhattan((const char *)a.data(),
                                                 (const char *)b.data(), dims);
    const double ref = ref_manhattan(a.data(), b.data(), dims);
    EXPECT_NEAR(got, ref, ref * 1e-4 + 1e-9) << "dims=" << dims;
  }
}

// ---------------------------------------------------------------------------
// Per-tier parity tests
//
// A separate parameterized fixture calls init_vector_distance_functions_tier()
// for each registered tier, skipping tiers that are unavailable on this CPU or
// build. This ensures every SIMD kernel is tested for correctness independently
// — including inferior tiers on CPUs that support a higher one.
//
// The existing VectorDistanceTest suite is untouched; it still exercises the
// production path via init_vector_distance_functions() (highest tier on this
// CPU).
// ---------------------------------------------------------------------------

static const char *tier_name(VectorDistanceTier tier) {
  switch (tier) {
    case VectorDistanceTier::Scalar:
      return "Scalar";
    case VectorDistanceTier::Sse42:
      return "Sse42";
    case VectorDistanceTier::Avx2:
      return "Avx2";
    case VectorDistanceTier::Avx512f:
      return "Avx512f";
    case VectorDistanceTier::Neon:
      return "Neon";
    case VectorDistanceTier::Sve2:
      return "Sve2";
  }
  return "Unknown";
}

class VectorDistanceTierParityTest
    : public ::testing::TestWithParam<VectorDistanceTier> {
 protected:
  void SetUp() override {
    const VectorDistanceTier t = GetParam();
    if (!vector_distance_tier_available(t))
      GTEST_SKIP() << tier_name(t) << " not available on this CPU/build";
    init_vector_distance_functions_tier(t);
#if defined(__x86_64__) || defined(_M_X64)
    if (t == VectorDistanceTier::Avx2 || t == VectorDistanceTier::Avx512f) {
      EXPECT_EQ(vector_distance_wide_tier(), t);
      EXPECT_EQ(vector_distance_narrow_tier(),
                vector_distance_tier_available(VectorDistanceTier::Sse42)
                    ? VectorDistanceTier::Sse42
                    : VectorDistanceTier::Scalar);
    }
#endif
  }
  void TearDown() override { init_vector_distance_functions(); }
};

TEST_P(VectorDistanceTierParityTest, EuclideanSquaredParityPerTier) {
  std::mt19937 rng(42);
  std::uniform_real_distribution<float> dist(-10.0f, 10.0f);

  // 16383 is
  //   16·1023+15 for AVX-512
  //   8·2047+7 for AVX2
  //   4·4095+3 for SSE/NEON
  // so every tail loop is tested.
  for (uint32_t dims : {4u, 8u, 32u, 128u, 1024u, 16383u}) {
    std::vector<float> a(dims), b(dims);
    for (auto &x : a) x = dist(rng);
    for (auto &x : b) x = dist(rng);

    const double got = vector_distance_euclidean_squared(
        (const char *)a.data(), (const char *)b.data(), dims);
    const double ref = ref_euclidean_squared(a.data(), b.data(), dims);
    EXPECT_NEAR(got, ref, ref * 1e-4 + 1e-9)
        << "tier=" << tier_name(GetParam()) << " dims=" << dims;
  }
}

TEST_P(VectorDistanceTierParityTest, EuclideanParityPerTier) {
  std::mt19937 rng(42);
  std::uniform_real_distribution<float> dist(-10.0f, 10.0f);

  for (uint32_t dims : {4u, 8u, 32u, 128u, 1024u, 16383u}) {
    std::vector<float> a(dims), b(dims);
    for (auto &x : a) x = dist(rng);
    for (auto &x : b) x = dist(rng);

    const double got =
        euclidean_l2((const char *)a.data(), (const char *)b.data(), dims);
    const double ref = ref_euclidean(a.data(), b.data(), dims);
    EXPECT_NEAR(got, ref, ref * 1e-4 + 1e-9)
        << "tier=" << tier_name(GetParam()) << " dims=" << dims;
  }
}

TEST_P(VectorDistanceTierParityTest, CosineParityPerTier) {
  std::mt19937 rng(123);
  std::uniform_real_distribution<float> dist(-5.0f, 5.0f);

  for (uint32_t dims : {4u, 8u, 32u, 128u, 1024u, 16383u}) {
    std::vector<float> a(dims), b(dims);
    for (auto &x : a) x = dist(rng);
    for (auto &x : b) x = dist(rng);

    const double got = vector_distance_cosine((const char *)a.data(),
                                              (const char *)b.data(), dims);
    const double ref = ref_cosine(a.data(), b.data(), dims);
    EXPECT_NEAR(got, ref, 1e-4)
        << "tier=" << tier_name(GetParam()) << " dims=" << dims;
  }
}

TEST_P(VectorDistanceTierParityTest, DotProductParityPerTier) {
  std::mt19937 rng(77);
  std::uniform_real_distribution<float> dist(-10.0f, 10.0f);

  for (uint32_t dims : {4u, 8u, 32u, 128u, 1024u, 16383u}) {
    std::vector<float> a(dims), b(dims);
    for (auto &x : a) x = dist(rng);
    for (auto &x : b) x = dist(rng);

    const double got = vector_distance_dot((const char *)a.data(),
                                           (const char *)b.data(), dims);
    const double ref = ref_dot_product(a.data(), b.data(), dims);
    EXPECT_NEAR(got, ref, std::abs(ref) * 1e-4 + 1e-9)
        << "tier=" << tier_name(GetParam()) << " dims=" << dims;
  }
}

TEST_P(VectorDistanceTierParityTest, ManhattanParityPerTier) {
  std::mt19937 rng(55);
  std::uniform_real_distribution<float> dist(-10.0f, 10.0f);

  for (uint32_t dims : {4u, 8u, 32u, 128u, 1024u, 16383u}) {
    std::vector<float> a(dims), b(dims);
    for (auto &x : a) x = dist(rng);
    for (auto &x : b) x = dist(rng);

    const double got = vector_distance_manhattan((const char *)a.data(),
                                                 (const char *)b.data(), dims);
    const double ref = ref_manhattan(a.data(), b.data(), dims);
    EXPECT_NEAR(got, ref, ref * 1e-4 + 1e-9)
        << "tier=" << tier_name(GetParam()) << " dims=" << dims;
  }
}

TEST_P(VectorDistanceTierParityTest, OverflowFallbackPerTier) {
  // A 16-dim vector (>= 16 triggers the wide kernel) with one element = 2e38.
  // (2e38)^2 ~ 4e76 overflows float32 (FLT_MAX ~ 3.4e38); the SIMD accumulator
  // becomes +Inf without the fallback.  The scalar path uses double throughout
  // and returns a finite result.  Verify the fix: result must be finite and
  // equal to the scalar reference.
  constexpr uint32_t dims = 16;
  std::vector<float> a(dims, 0.0f), b(dims, 0.0f);
  a[0] = 2e38f;

  // Euclidean squared: scalar = (2e38)^2; broken SIMD would give +Inf (-> SQL
  // NULL).
  const double got_e = vector_distance_euclidean_squared(
      (const char *)a.data(), (const char *)b.data(), dims);
  EXPECT_TRUE(std::isfinite(got_e)) << "tier=" << tier_name(GetParam());
  EXPECT_EQ(got_e, ref_euclidean_squared(a.data(), b.data(), dims))
      << "tier=" << tier_name(GetParam());

  // Euclidean L2: scalar = 2e38; mirrors SQL EUCLIDEAN (sqrt of squared).
  const double got_l2 =
      euclidean_l2((const char *)a.data(), (const char *)b.data(), dims);
  EXPECT_TRUE(std::isfinite(got_l2)) << "tier=" << tier_name(GetParam());
  EXPECT_EQ(got_l2, ref_euclidean(a.data(), b.data(), dims))
      << "tier=" << tier_name(GetParam());

  // Manhattan: scalar = 2e38.
  const double got_m = vector_distance_manhattan((const char *)a.data(),
                                                 (const char *)b.data(), dims);
  EXPECT_TRUE(std::isfinite(got_m)) << "tier=" << tier_name(GetParam());
  EXPECT_EQ(got_m, ref_manhattan(a.data(), b.data(), dims))
      << "tier=" << tier_name(GetParam());

  // Dot: a[0]*b2[0] = 2e38*2e38 overflows float32; scalar = 4e76 (finite in
  // double).
  std::vector<float> b2(dims, 0.0f);
  b2[0] = 2e38f;
  const double got_d = vector_distance_dot((const char *)a.data(),
                                           (const char *)b2.data(), dims);
  EXPECT_TRUE(std::isfinite(got_d)) << "tier=" << tier_name(GetParam());
  EXPECT_EQ(got_d, ref_dot_product(a.data(), b2.data(), dims))
      << "tier=" << tier_name(GetParam());

  // Cosine: a == a (same pointer) => cosine distance = 0.
  // Float32 norm overflow to Inf causes 1 - Inf/Inf = NaN without the fix.
  const double got_c = vector_distance_cosine((const char *)a.data(),
                                              (const char *)a.data(), dims);
  EXPECT_NEAR(got_c, 0.0, 1e-9) << "tier=" << tier_name(GetParam());
}

static std::string tier_param_name(
    const ::testing::TestParamInfo<VectorDistanceTier> &info) {
  return tier_name(info.param);
}

#if defined(__x86_64__) || defined(_M_X64)
INSTANTIATE_TEST_SUITE_P(AllTiers, VectorDistanceTierParityTest,
                         ::testing::Values(VectorDistanceTier::Scalar,
                                           VectorDistanceTier::Sse42,
                                           VectorDistanceTier::Avx2,
                                           VectorDistanceTier::Avx512f),
                         tier_param_name);
#elif defined(__aarch64__) || defined(_M_ARM64)
INSTANTIATE_TEST_SUITE_P(AllTiers, VectorDistanceTierParityTest,
                         ::testing::Values(VectorDistanceTier::Scalar,
                                           VectorDistanceTier::Neon,
                                           VectorDistanceTier::Sve2),
                         tier_param_name);
#else
INSTANTIATE_TEST_SUITE_P(AllTiers, VectorDistanceTierParityTest,
                         ::testing::Values(VectorDistanceTier::Scalar),
                         tier_param_name);
#endif

// ---------------------------------------------------------------------------
// Dispatch description / tier reporting
// ---------------------------------------------------------------------------

class VectorDistanceDispatchTest : public ::testing::Test {
 protected:
  void TearDown() override { init_vector_distance_functions(); }
};

TEST_F(VectorDistanceDispatchTest, ProductionInitIsIdempotent) {
  init_vector_distance_functions();
  const auto wide = vector_distance_wide_tier();
  const auto narrow = vector_distance_narrow_tier();
  init_vector_distance_functions();
  EXPECT_EQ(vector_distance_wide_tier(), wide);
  EXPECT_EQ(vector_distance_narrow_tier(), narrow);
}

TEST_F(VectorDistanceDispatchTest, ProductionInitRestoredAfterScalarOverride) {
  init_vector_distance_functions();
  const VectorDistanceTier prod_wide = vector_distance_wide_tier();
  const VectorDistanceTier prod_narrow = vector_distance_narrow_tier();

  init_vector_distance_functions_tier(VectorDistanceTier::Scalar);
  EXPECT_EQ(vector_distance_wide_tier(), VectorDistanceTier::Scalar);
  EXPECT_EQ(vector_distance_narrow_tier(), VectorDistanceTier::Scalar);

  init_vector_distance_functions();
  EXPECT_EQ(vector_distance_wide_tier(), prod_wide);
  EXPECT_EQ(vector_distance_narrow_tier(), prod_narrow);
}

TEST_F(VectorDistanceDispatchTest, DescriptionIsFragmentForLogMessage) {
  init_vector_distance_functions();
  char msg[256];
  const size_t len = vector_distance_dispatch_description(msg, sizeof(msg));
  EXPECT_GT(len, 0u);
  // The description is spliced into ER_VECTOR_DISTANCE_SIMD_DISPATCH's
  // "For VECTOR_DISTANCE using %s", so it is a sentence fragment: a leading
  // space, no repeated "DISTANCE()"/"VECTOR_DISTANCE" wording of its own.
  EXPECT_EQ(msg[0], ' ');
  EXPECT_EQ(std::string(msg).find("DISTANCE"), std::string::npos);
}

TEST_F(VectorDistanceDispatchTest, ForcedScalarTierUpdatesDescription) {
  init_vector_distance_functions_tier(VectorDistanceTier::Scalar);
  EXPECT_EQ(vector_distance_wide_tier(), VectorDistanceTier::Scalar);
  EXPECT_EQ(vector_distance_narrow_tier(), VectorDistanceTier::Scalar);

  char msg[256];
  vector_distance_dispatch_description(msg, sizeof(msg));
  EXPECT_NE(std::string(msg).find("software scalar"), std::string::npos);
}

#if defined(__x86_64__) || defined(_M_X64)
TEST_F(VectorDistanceDispatchTest,
       SplitDispatchMentionsDimensionsWhenWideDiffers) {
  if (!vector_distance_tier_available(VectorDistanceTier::Avx2) ||
      !vector_distance_tier_available(VectorDistanceTier::Sse42)) {
    GTEST_SKIP() << "requires AVX2 wide path and SSE4.2 narrow path";
  }

  init_vector_distance_functions();
  ASSERT_NE(vector_distance_wide_tier(), vector_distance_narrow_tier());

  char msg[256];
  vector_distance_dispatch_description(msg, sizeof(msg));
  const std::string ge =
      "dimensions >= " + std::to_string(VECTOR_DISTANCE_WIDE_MIN_DIMS);
  const std::string lt =
      "dimensions < " + std::to_string(VECTOR_DISTANCE_WIDE_MIN_DIMS);
  EXPECT_NE(std::string(msg).find(ge), std::string::npos);
  EXPECT_NE(std::string(msg).find(lt), std::string::npos);
  EXPECT_NE(std::string(msg).find("SSE4.2"), std::string::npos);
}
#endif

}  // namespace vector_distance_unittest
