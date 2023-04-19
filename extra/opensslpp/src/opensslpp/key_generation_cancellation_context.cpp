/* Copyright (c) 2022 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; version 2 of
   the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#include <openssl/bn.h>

#include "opensslpp/key_generation_cancellation_context.hpp"

#include <opensslpp/core_error.hpp>

#include "opensslpp/key_generation_cancellation_context_accessor.hpp"

namespace opensslpp {

void key_generation_cancellation_context::bn_gencb_deleter::operator()(
    void *ptr) const noexcept {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  delete static_cast<BN_GENCB *>(ptr);
#else
  if (ptr != nullptr) BN_GENCB_free(static_cast<BN_GENCB *>(ptr));
#endif
}

key_generation_cancellation_context::key_generation_cancellation_context(
    const key_generation_cancellation_callback &cancellation_callback)
    : impl_ {
  cancellation_callback ?
#if OPENSSL_VERSION_NUMBER < 0x10100000L
                        new BN_GENCB
#else
                        BN_GENCB_new()
#endif
                        : nullptr
}
, cancellation_callback_{&cancellation_callback}, cancelled_{false} {
  if (cancellation_callback) {
    if (!impl_)
      throw core_error{"cannot create key generation cancellation context"};
    BN_GENCB_set(
        key_generation_cancellation_context_accessor::get_impl(*this),
        +[](int, int, BN_GENCB *cb_ctx) noexcept -> int {
          auto &obj = *static_cast<key_generation_cancellation_context *>(
#if OPENSSL_VERSION_NUMBER < 0x10100000L
              cb_ctx->arg
#else
              BN_GENCB_get_arg(cb_ctx)
#endif
          );
          try {
            obj.cancelled_ = (*obj.cancellation_callback_)();
          } catch (...) {
            obj.cancelled_ = true;
          }

          return obj.cancelled_ ? 0 : 1;
        },
        this);
  }
}

}  // namespace opensslpp
