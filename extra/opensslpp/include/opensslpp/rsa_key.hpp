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

#ifndef OPENSSLPP_RSA_KEY_HPP
#define OPENSSLPP_RSA_KEY_HPP

#include <cstdint>
#include <memory>
#include <ostream>
#include <string>

#include <opensslpp/rsa_key_fwd.hpp>

#include <opensslpp/accessor_fwd.hpp>
#include <opensslpp/big_number_fwd.hpp>
#include <opensslpp/key_generation_cancellation_callback_fwd.hpp>
#include <opensslpp/rsa_padding_fwd.hpp>

namespace opensslpp {

class rsa_key final {
  friend class accessor<rsa_key>;

 public:
  static const big_number exponent_3;
  static const big_number exponent_f4;
  static const big_number &default_exponent;

 public:
  rsa_key() noexcept = default;
  ~rsa_key() noexcept = default;

  rsa_key(const rsa_key &obj);
  rsa_key(rsa_key &&obj) noexcept = default;

  rsa_key &operator=(const rsa_key &obj);
  rsa_key &operator=(rsa_key &&obj) noexcept = default;

  void swap(rsa_key &obj) noexcept;

  bool is_empty() const noexcept { return !impl_; }
  bool is_private() const noexcept;
  std::size_t get_size_in_bits() const noexcept;
  std::size_t get_size_in_bytes() const noexcept;

  std::size_t get_max_block_size_in_bytes(rsa_padding padding) const noexcept;

  rsa_key derive_public_key() const;

  static rsa_key generate(
      std::uint32_t bits, const big_number &exponent = default_exponent,
      const key_generation_cancellation_callback &cancellation_callback =
          key_generation_cancellation_callback{});

  static std::string export_private_pem(const rsa_key &key);
  static std::string export_public_pem(const rsa_key &key);

  static rsa_key import_private_pem(const std::string &pem);
  static rsa_key import_public_pem(const std::string &pem);

 private:
  // should not be declared final as this prevents optimization for empty
  // deleter in std::unique_ptr
  struct rsa_deleter {
    void operator()(void *rsa) const noexcept;
  };

  using impl_ptr = std::unique_ptr<void, rsa_deleter>;
  impl_ptr impl_;
};

std::ostream &operator<<(std::ostream &os, const rsa_key &obj);

}  // namespace opensslpp

#endif
