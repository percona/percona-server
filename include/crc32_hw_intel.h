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

#pragma once

#include <stddef.h>
#include <stdint.h>

/*
 * Hardware-accelerated CRC32 (IEEE, zlib-compatible) using
 * Intel PCLMULQDQ + SSE2/SSSE3 intrinsics.
 *
 * This header only declares a minimal context and the main
 * entry point used by mysys/my_checksum. The implementation
 * lives in crc32_hw_intel.cc.
 */

/* Precomputed constants for PCLMUL-based CRC32 folding.
 * rk1, rk2, rk5, rk6, rk7 correspond to values derived from
 * the generator polynomial (IEEE CRC32, 0x04C11DB7) as
 * described in Intel's "Fast CRC Computation for Generic
 * Polynomials Using PCLMULQDQ Instruction".
 */
struct crc32_pclmul_ctx {
  uint64_t rk1;
  uint64_t rk2;
  uint64_t rk5;
  uint64_t rk6;
  uint64_t rk7;
  uint64_t rk8;
};

/*
 * Compute CRC32 (IEEE, reflected, zlib-compatible) over the given buffer,
 * using Intel PCLMULQDQ-based folding. The caller is responsible for
 * providing a context initialized with the correct rk* constants for
 * the desired polynomial.
 *
 * Parameters:
 *   data - pointer to input buffer (may be unaligned)
 *   len  - size of input buffer in bytes
 *   crc  - initial CRC value (reflected, 32-bit)
 *   ctx  - pointer to precomputed PCLMUL constants
 *
 * Returns:
 *   Updated CRC value after processing the buffer.
 */
uint32_t crc32_hw_intel(const unsigned char *data,
                        size_t len,
                        uint32_t crc,
                        const crc32_pclmul_ctx *ctx);

// Initialize Intel-like PCLMULQDQ context for IEEE CRC32 (same polynomial as zlib).
void crc32_hw_intel_init_ieee_ctx(crc32_pclmul_ctx *ctx);
