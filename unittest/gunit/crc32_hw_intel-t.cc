#include <gtest/gtest.h>

#include <cstdint>
#include <cstring>
#include <vector>

#include "my_checksum.h"
#include "crc32_hw_intel.h"

namespace crc32_hw_intel_test {

static std::uint32_t crc32_hw_intel_wrapper(std::uint32_t crc,
                                            const unsigned char *buf,
                                            size_t len) {
  crc32_pclmul_ctx ctx{};
  crc32_hw_intel_init_ieee_ctx(&ctx);
  return crc32_hw_intel(buf, len, crc, &ctx);
}

// Helper: compare hw_intel vs zlib for given buffer and seed.
static void check(std::uint32_t seed, const unsigned char *buf, size_t len) {
  EXPECT_EQ(mycrc32::crc32_zlib(seed, buf, len),
            crc32_hw_intel_wrapper(seed, buf, len))
      << "seed=" << std::hex << seed << " len=" << std::dec << len;
}

// Build a buffer of given size filled with a repeating pattern byte.
static std::vector<unsigned char> make_buf(size_t len, unsigned char fill) {
  return std::vector<unsigned char>(len, fill);
}

// Build a buffer with a simple incrementing pattern.
static std::vector<unsigned char> make_inc_buf(size_t len) {
  std::vector<unsigned char> buf(len);
  for (size_t i = 0; i < len; ++i)
    buf[i] = static_cast<unsigned char>(i & 0xff);
  return buf;
}

TEST(Crc32HwIntel, ZlibCompatibilitySmallBuffers) {
  const std::uint32_t seed = 0xbadcafe;

  auto b10_zero = make_buf(10, 0x00);
  auto b10_ff   = make_buf(10, 0xff);

  check(seed, b10_zero.data(), b10_zero.size());
  check(seed, b10_ff.data(),   b10_ff.size());
}

TEST(Crc32HwIntel, StandardVector123456789) {
  const unsigned char msg[] = "123456789";
  const size_t len = sizeof(msg) - 1;

  std::uint32_t z = mycrc32::crc32_zlib(0, msg, len);
  std::uint32_t h = crc32_hw_intel_wrapper(0, msg, len);
  EXPECT_EQ(z, h);
}

// Covers: len < 4 (-> direct Barrett)
TEST(Crc32HwIntel, VeryShortBuffers) {
  const std::uint32_t seeds[] = {0, 0xffffffff, 0xbadcafe, 0x12345678};

  for (std::uint32_t seed : seeds) {
    for (size_t len = 1; len <= 3; ++len) {
      auto buf = make_buf(len, 0xab);
      check(seed, buf.data(), len);

      auto inc = make_inc_buf(len);
      check(seed, inc.data(), len);
    }
  }
}

// Covers: 4 <= len < 16 (-> reduce_128_64 then Barrett)
TEST(Crc32HwIntel, ShortBuffers4to15) {
  const std::uint32_t seeds[] = {0, 0xffffffff, 0xbadcafe};

  for (std::uint32_t seed : seeds) {
    for (size_t len = 4; len <= 15; ++len) {
      auto buf = make_buf(len, 0x5a);
      check(seed, buf.data(), len);

      auto inc = make_inc_buf(len);
      check(seed, inc.data(), len);
    }
  }
}

// Covers: len == 16 (-> exactly one block, no folding loop)
TEST(Crc32HwIntel, ExactlyOneBlock) {
  const std::uint32_t seeds[] = {0, 0xffffffff, 0xbadcafe};

  for (std::uint32_t seed : seeds) {
    auto buf = make_buf(16, 0x42);
    check(seed, buf.data(), 16);

    auto inc = make_inc_buf(16);
    check(seed, inc.data(), 16);
  }
}

// Covers: 17 <= len < 32 (-> partial_bytes / tail handling)
TEST(Crc32HwIntel, PartialBytes17to31) {
  const std::uint32_t seeds[] = {0, 0xffffffff, 0xbadcafe};

  for (std::uint32_t seed : seeds) {
    for (size_t len = 17; len <= 31; ++len) {
      auto buf = make_buf(len, 0x99);
      check(seed, buf.data(), len);

      auto inc = make_inc_buf(len);
      check(seed, inc.data(), len);
    }
  }
}

// Covers: len == 32 (-> folding loop, exactly two blocks)
TEST(Crc32HwIntel, ExactlyTwoBlocks) {
  const std::uint32_t seeds[] = {0, 0xffffffff, 0xbadcafe};

  for (std::uint32_t seed : seeds) {
    auto buf = make_buf(32, 0x11);
    check(seed, buf.data(), 32);

    auto inc = make_inc_buf(32);
    check(seed, inc.data(), 32);
  }
}

// Covers: 33..127 bytes (folding loop + tail)
TEST(Crc32HwIntel, MediumBuffers) {
  const std::uint32_t seeds[] = {0, 0xffffffff, 0xbadcafe};

  for (std::uint32_t seed : seeds) {
    for (size_t len : {33, 48, 63, 64, 65, 96, 127}) {
      auto buf = make_buf(len, 0xcc);
      check(seed, buf.data(), len);

      auto inc = make_inc_buf(len);
      check(seed, inc.data(), len);
    }
  }
}

// Covers: >= 128 bytes (full folding loop)
TEST(Crc32HwIntel, LargeBuffers) {
  const std::uint32_t seeds[] = {0, 0xffffffff, 0xbadcafe};

  for (std::uint32_t seed : seeds) {
    for (size_t len : {128, 256, 512, 1024, 4096}) {
      auto buf = make_buf(len, 0xde);
      check(seed, buf.data(), len);

      auto inc = make_inc_buf(len);
      check(seed, inc.data(), len);
    }
  }
}

// Covers: seed == 0 specifically (common default)
TEST(Crc32HwIntel, SeedZero) {
  const std::uint32_t seed = 0;

  for (size_t len : {1, 3, 4, 7, 15, 16, 17, 31, 32, 64, 128, 1024}) {
    auto buf = make_inc_buf(len);
    check(seed, buf.data(), len);
  }
}

// Covers: seed == 0xffffffff (another common default)
TEST(Crc32HwIntel, SeedAllOnes) {
  const std::uint32_t seed = 0xffffffff;

  for (size_t len : {1, 3, 4, 7, 15, 16, 17, 31, 32, 64, 128, 1024}) {
    auto buf = make_inc_buf(len);
    check(seed, buf.data(), len);
  }
}

}  // namespace crc32_hw_intel_test
