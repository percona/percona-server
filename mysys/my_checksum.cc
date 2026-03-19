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
*/

#include "my_checksum.h"
#include "crc32_hw_intel.h"
#include <mutex>

namespace mycrc32 {

static crc32_pclmul_ctx crc32_intel_ctx;
static std::once_flag crc32_intel_ctx_once;

static void crc32_intel_init_ctx() {
  crc32_hw_intel_init_ieee_ctx(&crc32_intel_ctx);
}

std::uint32_t crc32_hw_x86(std::uint32_t crc,
                            const unsigned char *buf,
                            size_t len) {
  if (!buf || len == 0)
    return crc;

  std::call_once(crc32_intel_ctx_once, crc32_intel_init_ctx);
  return crc32_hw_intel(buf, len, crc, &crc32_intel_ctx);
}

}  // namespace mycrc32
