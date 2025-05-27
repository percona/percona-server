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

#ifndef OPENSSLPP_EVP_PKEY_HPP
#define OPENSSLPP_EVP_PKEY_HPP

#include <cstdint>
#include <memory>
#include <ostream>
#include <string>
#include <string_view>

#include <opensslpp/evp_pkey_fwd.hpp>

#include <opensslpp/accessor_fwd.hpp>
#include <opensslpp/evp_pkey_algorithm_fwd.hpp>
#include <opensslpp/key_generation_cancellation_callback_fwd.hpp>

namespace opensslpp {

class evp_pkey final {
  friend class accessor<evp_pkey>;

 public:
  evp_pkey() noexcept = default;
  ~evp_pkey() noexcept = default;

  evp_pkey(const evp_pkey &obj);
  evp_pkey(evp_pkey &&obj) noexcept = default;

  evp_pkey &operator=(const evp_pkey &obj);
  evp_pkey &operator=(evp_pkey &&obj) noexcept = default;

  void swap(evp_pkey &obj) noexcept;

  bool is_empty() const noexcept { return !impl_; }
  evp_pkey_algorithm get_algorithm() const noexcept;
  bool is_private() const noexcept;
  std::size_t get_size_in_bits() const noexcept;
  std::size_t get_size_in_bytes() const noexcept;

  evp_pkey derive_public_key() const;

  static evp_pkey generate(
      evp_pkey_algorithm algorithm, std::uint32_t bits,
      const key_generation_cancellation_callback &cancellation_callback =
          key_generation_cancellation_callback{});

  static std::string export_private_pem(const evp_pkey &key);
  static std::string export_public_pem(const evp_pkey &key);

  static evp_pkey import_private_pem(std::string_view pem);
  static evp_pkey import_public_pem(std::string_view pem);

 private:
  // should not be declared final as this prevents optimization for empty
  // deleter in std::unique_ptr
  struct evp_pkey_deleter {
    void operator()(void *evp_pkey) const noexcept;
  };

  using impl_ptr = std::unique_ptr<void, evp_pkey_deleter>;
  impl_ptr impl_;

  static void validate_if_algorithm_supported(evp_pkey_algorithm algorithm);
};

std::ostream &operator<<(std::ostream &os, const evp_pkey &obj);

}  // namespace opensslpp

#endif
