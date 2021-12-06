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

#ifndef OPENSSLPP_DH_KEY_HPP
#define OPENSSLPP_DH_KEY_HPP

#include <cstdint>
#include <memory>
#include <ostream>
#include <string>

#include <opensslpp/dh_key_fwd.hpp>

#include <opensslpp/accessor_fwd.hpp>
#include <opensslpp/big_number_fwd.hpp>
#include <opensslpp/key_generation_cancellation_callback_fwd.hpp>

namespace opensslpp {

class dh_key final {
  friend class accessor<dh_key>;

 public:
  static const std::uintmax_t generator_2;
  static const std::uintmax_t generator_5;
  static const std::uintmax_t default_generator;

 public:
  dh_key() noexcept = default;
  ~dh_key() noexcept = default;

  dh_key(const dh_key &obj);
  dh_key(dh_key &&obj) noexcept = default;

  dh_key &operator=(const dh_key &obj);
  dh_key &operator=(dh_key &&obj) noexcept = default;

  void swap(dh_key &obj) noexcept;

  bool is_empty() const noexcept { return !impl_; }
  bool has_public_component() const noexcept;
  bool has_private_component() const noexcept;

  std::size_t get_size_in_bits() const noexcept;
  std::size_t get_size_in_bytes() const noexcept;
  std::size_t get_security_size_in_bits() const noexcept;

  big_number get_public_component() const;
  big_number get_private_component() const;

  void promote_to_key();
  dh_key derive_public_key() const;

  static dh_key generate_parameters(
      std::uint32_t bits, std::uintmax_t generator = default_generator,
      const key_generation_cancellation_callback &cancellation_callback =
          key_generation_cancellation_callback{});

  static std::string export_parameters_pem(const dh_key &key);
  static std::string export_private_pem(const dh_key &key);
  static std::string export_public_pem(const dh_key &key);

  static dh_key import_parameters_pem(const std::string &pem);
  static dh_key import_private_pem(const std::string &pem);
  static dh_key import_public_pem(const std::string &pem);

 private:
  // should not be declared final as this prevents optimization for empty
  // deleter in std::unique_ptr
  struct dh_deleter {
    void operator()(void *dhp) const noexcept;
  };

  using impl_ptr = std::unique_ptr<void, dh_deleter>;
  impl_ptr impl_;
};

std::ostream &operator<<(std::ostream &os, const dh_key &obj);

}  // namespace opensslpp

#endif
