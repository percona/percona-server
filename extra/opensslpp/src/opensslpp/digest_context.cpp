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

#include <array>
#include <cassert>
#include <vector>

#include <openssl/evp.h>

#include <opensslpp/digest_context.hpp>

#include <opensslpp/core_error.hpp>

#include "opensslpp/digest_context_accessor.hpp"

namespace opensslpp {

void digest_context::digest_context_deleter::operator()(
    void *dc) const noexcept {
  if (dc != nullptr) EVP_MD_CTX_destroy(static_cast<EVP_MD_CTX *>(dc));
}

digest_context::digest_context(const std::string &type)
    : impl_{EVP_MD_CTX_create()} {
  if (!impl_) throw core_error{"cannot create digest context"};
  auto md = EVP_get_digestbyname(type.c_str());
  if (md == nullptr) throw core_error{"unknown digest name"};
  if (EVP_DigestInit_ex(digest_context_accessor::get_impl(*this), md,
                        nullptr) == 0)
    throw core_error{"cannot initialize digest context"};
}

digest_context::digest_context(const digest_context &obj)
    : impl_{obj.is_empty() ? nullptr : EVP_MD_CTX_create()} {
  if (!obj.is_empty()) {
    if (!impl_) throw core_error{"cannot duplicate digest context"};
    auto obj_md = EVP_MD_CTX_md(digest_context_accessor::get_impl(obj));
    if (EVP_DigestInit_ex(digest_context_accessor::get_impl(*this), obj_md,
                          nullptr) == 0)
      throw core_error{"cannot initialize duplicated digest context"};
    if (EVP_MD_CTX_copy_ex(digest_context_accessor::get_impl(*this),
                           digest_context_accessor::get_impl(obj)) == 0)
      throw core_error{"cannot copy duplicated digest context data"};
  }
}

digest_context &digest_context::operator=(const digest_context &obj) {
  auto tmp = digest_context{obj};
  swap(tmp);
  return *this;
}

void digest_context::swap(digest_context &obj) noexcept {
  impl_.swap(obj.impl_);
}

std::size_t digest_context::get_size_in_bytes() const noexcept {
  return EVP_MD_CTX_size(digest_context_accessor::get_impl(*this));
}

void digest_context::update(const std::string &data) {
  assert(!is_empty());
  if (EVP_DigestUpdate(digest_context_accessor::get_impl(*this), data.c_str(),
                       data.size()) == 0)
    throw core_error{"cannot hash data into digest context"};
}

std::string digest_context::finalize() {
  assert(!is_empty());

  // TODO: use c++17 non-const std::string::data() member here
  using buffer_type = std::array<unsigned char, EVP_MAX_MD_SIZE + 1>;
  buffer_type res;

  unsigned int res_size = 0;
  if (EVP_DigestFinal_ex(digest_context_accessor::get_impl(*this), res.data(),
                         &res_size) == 0)
    throw core_error{"cannot finalize digest context"};
  assert(res_size <= res.size());

  digest_context_accessor::set_impl(*this, nullptr);
  return {reinterpret_cast<char *>(res.data()),
          static_cast<std::size_t>(res_size)};
}

}  // namespace opensslpp
