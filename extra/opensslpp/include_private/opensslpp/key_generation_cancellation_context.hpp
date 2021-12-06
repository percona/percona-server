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

#ifndef OPENSSLPP_KEY_GENERATION_CANCELLATION_CONTEXT_HPP
#define OPENSSLPP_KEY_GENERATION_CANCELLATION_CONTEXT_HPP

#include <memory>

#include "opensslpp/key_generation_cancellation_context_fwd.hpp"

#include <opensslpp/accessor_fwd.hpp>

#include "opensslpp/key_generation_cancellation_callback_fwd.hpp"

namespace opensslpp {

class key_generation_cancellation_context {
  friend class accessor<key_generation_cancellation_context>;

 public:
  explicit key_generation_cancellation_context(
      const key_generation_cancellation_callback &cancellation_callback);
  ~key_generation_cancellation_context() noexcept = default;

  key_generation_cancellation_context(
      const key_generation_cancellation_context &) = delete;
  key_generation_cancellation_context(
      key_generation_cancellation_context &&) noexcept = default;

  key_generation_cancellation_context &operator=(
      const key_generation_cancellation_context &) = delete;
  key_generation_cancellation_context &operator=(
      key_generation_cancellation_context &&) noexcept = default;

  bool is_cancelled() const noexcept { return cancelled_; }

 private:
  struct bn_gencb_deleter {
    void operator()(void *ptr) const noexcept;
  };
  using impl_ptr = std::unique_ptr<void, bn_gencb_deleter>;
  impl_ptr impl_;
  const key_generation_cancellation_callback *cancellation_callback_;
  bool cancelled_;
};

}  // namespace opensslpp

#endif
