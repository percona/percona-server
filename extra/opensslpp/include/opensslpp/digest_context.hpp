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

#ifndef OPENSSLPP_DIGEST_CONTEXT_HPP
#define OPENSSLPP_DIGEST_CONTEXT_HPP

#include <memory>
#include <string>

#include <opensslpp/digest_context_fwd.hpp>

#include <opensslpp/accessor_fwd.hpp>

namespace opensslpp {

class digest_context final {
  friend class accessor<digest_context>;

 public:
  digest_context() noexcept = default;
  explicit digest_context(const std::string &type);
  ~digest_context() noexcept = default;

  digest_context(const digest_context &obj);
  digest_context(digest_context &&obj) noexcept = default;

  digest_context &operator=(const digest_context &obj);
  digest_context &operator=(digest_context &&obj) noexcept = default;

  void swap(digest_context &obj) noexcept;

  bool is_empty() const noexcept { return !impl_; }

  std::size_t get_size_in_bytes() const noexcept;

  void update(const std::string &data);
  std::string finalize();

 private:
  // should not be declared final as this prevents optimization for empty
  // deleter in std::unique_ptr
  struct digest_context_deleter {
    void operator()(void *dc) const noexcept;
  };

  using impl_ptr = std::unique_ptr<void, digest_context_deleter>;
  impl_ptr impl_;
};

}  // namespace opensslpp

#endif
