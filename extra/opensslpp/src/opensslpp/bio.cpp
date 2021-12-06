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

#include <openssl/bio.h>
#include <openssl/buffer.h>

#include "opensslpp/bio.hpp"

#include <opensslpp/core_error.hpp>

#include "opensslpp/bio_accessor.hpp"

namespace opensslpp {

void bio::bio_deleter::operator()(void *b) const noexcept {
  if (b != nullptr) BIO_free_all(static_cast<BIO *>(b));
}

bio::bio() : impl_(BIO_new(BIO_s_mem())) {
  if (!impl_) throw core_error{"cannot create new rw bio"};
}

bio::bio(const std::string &buffer)
    : impl_(BIO_new_mem_buf(buffer.c_str(), buffer.size())) {
  if (!impl_) throw core_error{"cannot create new ro bio"};
}

std::string bio::str() const {
  char *bio_mem_ptr = nullptr;
  const long bio_mem_len = BIO_get_mem_data(
      bio_accessor::get_impl_const_casted(*this), &bio_mem_ptr);
  if (bio_mem_ptr == nullptr || bio_mem_len <= 0)
    throw core_error{"cannot convert bio to string"};
  return {bio_mem_ptr, static_cast<std::size_t>(bio_mem_len - 1L)};
}

}  // namespace opensslpp
