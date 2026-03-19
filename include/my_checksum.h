/*
   Copyright (c) 2020, 2026, Oracle and/or its affiliates.

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

#ifndef MY_CHECKSUM_INCLUDED
#define MY_CHECKSUM_INCLUDED

#include <cassert>
#include <cstdint>      // std::uint32_t
#include <limits>       // std::numeric_limits
#include <type_traits>  // std::is_convertible
#include <cstring>      // memcpy

#include <zlib.h>  // crc32_z

#include "my_compiler.h"
#include "my_config.h"

#ifdef HAVE_ARMV8_CRC32_INTRINSIC
  #include <arm_acle.h>   // __crc32x
  #include <asm/hwcap.h>  // HWCAP_CRC32
  #include <sys/auxv.h>   // getauxval
#endif                  /* HAVE_ARMV8_CRC32_INTRINSIC */

namespace mycrc32 {

// ---------------------------------------------------------------------------
// ARM CRC32
// ---------------------------------------------------------------------------

#ifdef HAVE_ARMV8_CRC32_INTRINSIC

static inline bool have_armv8_hw_crc() {
  // C++11 magic static: initialized once, thread-safe.
  static const bool supported =
      (getauxval(AT_HWCAP) & HWCAP_CRC32) != 0;
  return supported;
}

MY_ATTRIBUTE((target("+crc")))
inline std::uint32_t IntegerCrc32(std::uint32_t crc, std::uint8_t b) {
  return __crc32b(crc, b);
}

MY_ATTRIBUTE((target("+crc")))
inline std::uint32_t IntegerCrc32(std::uint32_t crc, std::uint16_t h) {
  return __crc32h(crc, h);
}

MY_ATTRIBUTE((target("+crc")))
inline std::uint32_t IntegerCrc32(std::uint32_t crc, std::uint32_t w) {
  return __crc32w(crc, w);
}

MY_ATTRIBUTE((target("+crc")))
inline std::uint32_t IntegerCrc32(std::uint32_t crc, std::uint64_t d) {
  return __crc32d(crc, d);
}

#else  // HAVE_ARMV8_CRC32_INTRINSIC

template <class I>
inline std::uint32_t IntegerCrc32(std::uint32_t crc, I i) {
  unsigned char buf[sizeof(I)];
  memcpy(buf, &i, sizeof(I));
  crc = ~crc;
  crc = crc32_z(crc, buf, sizeof(I));
  return ~crc;
}

#endif  // HAVE_ARMV8_CRC32_INTRINSIC

// ---------------------------------------------------------------------------
// zlib fallback
// ---------------------------------------------------------------------------

inline std::uint32_t crc32_zlib(std::uint32_t crc,
                                const unsigned char *buf,
                                size_t len) {
  return static_cast<std::uint32_t>(crc32_z(crc, buf, len));
}

// ---------------------------------------------------------------------------
// x86 hardware CRC32 (PCLMULQDQ + SSE4.2)
// ---------------------------------------------------------------------------

#if defined(__x86_64__)

std::uint32_t crc32_hw_x86(std::uint32_t crc,
                            const unsigned char *buf,
                            size_t len);

static inline bool have_x86_hw_crc() {
#if defined(__GNUC__) || defined(__clang__)
  // C++11 magic static: initialized once, thread-safe.
  static const bool supported = []() {
    unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;

    __asm__ volatile("cpuid"
                     : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                     : "a"(0), "c"(0));
    if (eax < 1)
      return false;

    __asm__ volatile("cpuid"
                     : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                     : "a"(1), "c"(0));

    const bool has_sse42  = (ecx & (1u << 20)) != 0;
    const bool has_pclmul = (ecx & (1u << 1))  != 0;
    return has_sse42 && has_pclmul;
  }();
  return supported;
#else
  return false;
#endif
}

#endif  // __x86_64__

// ---------------------------------------------------------------------------
// PunnedCrc32: ARM hardware path for bulk data
// ---------------------------------------------------------------------------

template <class PT>
inline std::uint32_t PunnedCrc32(std::uint32_t crc,
                                 const unsigned char *buf,
                                 size_t len) {
  crc = ~crc;
  const unsigned char *plast = buf + (len / sizeof(PT)) * sizeof(PT);
  const unsigned char *last  = buf + len;

  for (; buf < plast; buf += sizeof(PT)) {
    PT pv;
    memcpy(&pv, buf, sizeof(PT));
    crc = IntegerCrc32(crc, pv);
  }
  for (; buf < last; ++buf)
    crc = IntegerCrc32(crc, *buf);

  return ~crc;
}

// ---------------------------------------------------------------------------
// Implementation selector: chosen once, thread-safe via magic static.
// ---------------------------------------------------------------------------

using crc32_func_t = std::uint32_t (*)(std::uint32_t,
                                       const unsigned char *,
                                       size_t);

inline crc32_func_t get_active_impl() {
  // C++11 magic static: lambda runs exactly once, result cached.
  static const crc32_func_t impl = []() -> crc32_func_t {
#if defined(__x86_64__)
    if (have_x86_hw_crc())
      return crc32_hw_x86;
#endif

#ifdef HAVE_ARMV8_CRC32_INTRINSIC
    if (have_armv8_hw_crc())
      return [](std::uint32_t crc, const unsigned char *buf, size_t len) {
        return PunnedCrc32<std::uint64_t>(crc, buf, len);
      };
#endif

    return crc32_zlib;
  }();

  return impl;
}

inline std::uint32_t crc32_fast(std::uint32_t crc,
                                const unsigned char *buf,
                                size_t len) {
  return get_active_impl()(crc, buf, len);
}

// Returns a human-readable name of the active CRC32 implementation.
inline const char *crc32_implementation_name() {
#if defined(__x86_64__)
  if (have_x86_hw_crc())
    return "hardware (Intel PCLMULQDQ)";
#endif
#ifdef HAVE_ARMV8_CRC32_INTRINSIC
  if (have_armv8_hw_crc())
    return "hardware (ARMv8 CRC32)";
#endif
  return "software (zlib)";
}

}  // namespace mycrc32

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

using ha_checksum = std::uint32_t;

inline ha_checksum my_checksum(ha_checksum crc,
                               const unsigned char *pos,
                               size_t length) {
  static_assert(std::is_convertible<uLong, ha_checksum>::value,
                "uLong cannot be converted to ha_checksum");
  ha_checksum tmp =
      static_cast<ha_checksum>(mycrc32::crc32_fast(crc, pos, length));
  assert(tmp <= std::numeric_limits<ha_checksum>::max());
  return tmp;
}

#endif  // MY_CHECKSUM_INCLUDED
