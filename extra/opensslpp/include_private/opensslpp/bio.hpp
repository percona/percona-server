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

#ifndef OPENSSLPP_BIO_HPP
#define OPENSSLPP_BIO_HPP

#include <memory>
#include <string>

#include "opensslpp/bio_fwd.hpp"

#include <opensslpp/accessor_fwd.hpp>

namespace opensslpp {

class bio final {
  friend class accessor<bio>;

 public:
  bio();
  bio(const std::string &buffer);

  ~bio() noexcept = default;

  bio(const bio &obj) = delete;
  bio(bio &&obj) noexcept = default;

  bio &operator=(const bio &obj) = delete;
  bio &operator=(bio &&obj) noexcept = default;

  std::string str() const;

 private:
  // should not be declared final as this prevents optimization for empty
  // deleter in std::unique_ptr
  struct bio_deleter {
    void operator()(void *b) const noexcept;
  };

  using impl_ptr = std::unique_ptr<void, bio_deleter>;
  impl_ptr impl_;
};

}  // namespace opensslpp

#endif
