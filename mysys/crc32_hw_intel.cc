/* Copyright (c) 2026, Alexey Bychko <abychko@gmail.com>

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

   This file implements hardware-accelerated CRC32 (IEEE, zlib-compatible)
   using Intel PCLMULQDQ instruction, based on the algorithm described in:

     Gopal, V., Ozturk, E., Guilford, J., et al. (2009).
     "Fast CRC computation for generic polynomials using PCLMULQDQ
     instruction." Intel Corporation.

   Precomputed constants (rk1..rk8) are taken from Intel's open-source
   soft-crc library (ether_crc32_clmul), used under BSD license. */

#include "crc32_hw_intel.h"

#include <immintrin.h>
#include <string.h>


alignas(16) static const uint8_t crc_xmm_shift_tab[48] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static inline __m128i xmm_shift_left(__m128i reg, unsigned int num) {
  const __m128i *p =
      (const __m128i *)(crc_xmm_shift_tab + 16 - num);
  return _mm_shuffle_epi8(reg, _mm_loadu_si128(p));
}

void crc32_hw_intel_init_ieee_ctx(crc32_pclmul_ctx *ctx) {
  // Constants taken from Intel's Ethernet CRC32 PCLMUL context
  // (ether_crc32_clmul in crc_ether.c).
  ctx->rk1 = 0x0ccaa009eull;
  ctx->rk2 = 0x1751997d0ull;
  ctx->rk5 = 0x0ccaa009eull;
  ctx->rk6 = 0x163cd6124ull;
  ctx->rk7 = 0x1f7011640ull;
  ctx->rk8 = 0x1db710641ull;
}

/*
 * Helper: one 16-byte folding round:
 *   fold = CLMUL(low(fold), rk1) ^ CLMUL(high(fold), rk1) ^ data_block
 */
static inline __m128i crc32_folding_round(__m128i data_block,
                                          __m128i precomp,
                                          __m128i fold)
{
  __m128i t0 = _mm_clmulepi64_si128(fold, precomp, 0x01);
  __m128i t1 = _mm_clmulepi64_si128(fold, precomp, 0x10);
  return _mm_xor_si128(t1, _mm_xor_si128(data_block, t0));
}

/* Reduce 128 bits to 64 bits using rk5/rk6. */
static inline __m128i crc32_reduce_128_to_64(__m128i data128,
                                             __m128i precomp)
{
  __m128i t0 = _mm_clmulepi64_si128(data128, precomp, 0x00);
  __m128i t1 = _mm_srli_si128(data128, 8);
  t0 = _mm_xor_si128(t0, t1);

  __m128i t2 = _mm_slli_si128(t0, 4);
  t1 = _mm_clmulepi64_si128(t2, precomp, 0x10);
  return _mm_xor_si128(t1, t0);
}

/* Final Barrett reduction 64 -> 32 bits using rk7. */
static inline uint32_t crc32_reduce_64_to_32(__m128i data64,
                                             __m128i precomp)
{
  static const uint32_t mask1[4] __attribute__((aligned(16))) = {
    0xffffffffu, 0xffffffffu, 0x00000000u, 0x00000000u
  };
  static const uint32_t mask2[4] __attribute__((aligned(16))) = {
    0x00000000u, 0xffffffffu, 0xffffffffu, 0xffffffffu
  };

  __m128i m1 = _mm_load_si128((const __m128i*)mask1);
  __m128i m2 = _mm_load_si128((const __m128i*)mask2);

  __m128i t0 = _mm_and_si128(data64, m2);
  __m128i t1 = _mm_clmulepi64_si128(t0, precomp, 0x00);
  t1 = _mm_xor_si128(t1, t0);
  t1 = _mm_and_si128(t1, m1);

  __m128i t2 = _mm_clmulepi64_si128(t1, precomp, 0x10);
  t2 = _mm_xor_si128(t2, t1);
  t2 = _mm_xor_si128(t2, t0);

  return (uint32_t)_mm_extract_epi32(t2, 2);
}

/*
 * Main entry: PCLMUL-based CRC32 (IEEE, reflected) over arbitrary buffer.
 * This follows the structure of Intel's crcr32_calc_pclmulqdq, but is
 * trimmed to a minimal form suitable for mysys/my_checksum.
 */
uint32_t crc32_hw_intel(const unsigned char *data,
                        size_t len,
                        uint32_t crc,
                        const crc32_pclmul_ctx *ctx)
{
  if (!data || !len || !ctx)
    return crc;

  __m128i fold, k, tmp;
  uint32_t n = 0;

  // Invert crc before folding in, same as zlib/crc32fast semantics.
  __m128i crc128 = _mm_insert_epi32(_mm_setzero_si128(), (int)~crc, 0);

  if (len < 32) {
    if (len == 16) {
      fold = _mm_loadu_si128((const __m128i*)data);
      fold = _mm_xor_si128(fold, crc128);
      goto reduce_128_64;
    }

if (len < 16) {
  alignas(16) unsigned char buf[16];
  memset(buf, 0, sizeof(buf));
  memcpy(buf, data, len);

  fold = _mm_load_si128((const __m128i*)buf);
  fold = _mm_xor_si128(fold, crc128);

  if (len < 4) {
    fold = xmm_shift_left(fold, 8 - (unsigned)len);
    goto reduce_64_32;
  }

    fold = xmm_shift_left(fold, 16 - (unsigned)len);
    goto reduce_128_64;
  }

    fold = _mm_loadu_si128((const __m128i*)data);
    fold = _mm_xor_si128(fold, crc128);
    n = 16;
    k = _mm_loadu_si128((const __m128i*)&ctx->rk1);
    goto tail_bytes;
  }

  fold = _mm_loadu_si128((const __m128i*)data);
  fold = _mm_xor_si128(fold, crc128);

  k = _mm_loadu_si128((const __m128i*)&ctx->rk1);
  for (n = 16; n + 16 <= len; n += 16) {
    tmp = _mm_loadu_si128((const __m128i*)(data + n));
    fold = crc32_folding_round(tmp, k, fold);
  }

tail_bytes:
  if (n < len) {
    static const uint32_t mask3[4] __attribute__((aligned(16))) = {
      0x80808080u, 0x80808080u, 0x80808080u, 0x80808080u
    };
    static const unsigned char shf_table[32] __attribute__((aligned(16))) = {
      0x00, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
      0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
      0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
    };

    __m128i last16 = _mm_loadu_si128(
      (const __m128i*)(data + len - 16)
    );

    __m128i shf = _mm_loadu_si128(
      (const __m128i*)(shf_table + (len & 15))
    );
    __m128i a = _mm_shuffle_epi8(fold, shf);

    __m128i m3 = _mm_load_si128((const __m128i*)mask3);
    __m128i shf2 = _mm_xor_si128(shf, m3);
    __m128i b = _mm_shuffle_epi8(fold, shf2);
    b = _mm_blendv_epi8(b, last16, shf2);

    __m128i k1 = _mm_loadu_si128((const __m128i*)&ctx->rk1);
    __m128i t0 = _mm_clmulepi64_si128(a, k1, 0x01);
    __m128i t1 = _mm_clmulepi64_si128(a, k1, 0x10);
    fold = _mm_xor_si128(t0, t1);
    fold = _mm_xor_si128(fold, b);
  }

reduce_128_64:
  {
    __m128i k5 = _mm_loadu_si128((const __m128i*)&ctx->rk5);
    fold = crc32_reduce_128_to_64(fold, k5);
  }

reduce_64_32:
  {
    __m128i k7 = _mm_loadu_si128((const __m128i*)&ctx->rk7);
    uint32_t res = crc32_reduce_64_to_32(fold, k7);
  // Invert result to match zlib/crc32fast output semantics.
    return ~res;
  }
}
