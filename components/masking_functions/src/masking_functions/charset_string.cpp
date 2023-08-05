/* Copyright (c) 2023 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   51 Franklin Street, Suite 500, Boston, MA 02110-1335 USA */

#include "masking_functions/charset_string.hpp"

#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>

#include "masking_functions/string_service_tuple.hpp"

namespace {
my_h_string to_h_string(void *h) noexcept {
  return static_cast<my_h_string>(h);
}
CHARSET_INFO_h to_cs_info_h(void *h) noexcept {
  return static_cast<CHARSET_INFO_h>(h);
}
}  // anonymous namespace

namespace masking_functions {

void charset_string::deleter::operator()(void *ptr) const noexcept {
  if (ptr != nullptr) (*services->factory->destroy)(to_h_string(ptr));
}

charset_string::collation_type charset_string::get_collation_by_name(
    const string_service_tuple &services, const char *collation_name) noexcept {
  assert(collation_name != nullptr);
  return (*services.charset->get)(collation_name);
}

charset_string::charset_string(const string_service_tuple &services,
                               std::string_view buffer,
                               collation_type collation)
    : impl_{nullptr, deleter{&services}} {
  if (collation == nullptr) throw std::runtime_error{"unknown collation"};

  my_h_string local_handle = nullptr;
  if ((*get_services().factory->create)(&local_handle) != 0)
    throw std::runtime_error{"cannot create an empty string"};
  assert(local_handle != nullptr);
  impl_.reset(local_handle);

  if ((*get_services().converter->convert_from_buffer)(
          local_handle, buffer.data(), buffer.size(),
          to_cs_info_h(collation)) != 0)
    throw std::runtime_error{"cannot create an string from a buffer"};
}

std::size_t charset_string::get_size_in_characters() const noexcept {
  assert(impl_);
  uint size_in_characters = 0;
  [[maybe_unused]] auto status =
      (*get_services().character_access->get_char_length)(
          to_h_string(impl_.get()), &size_in_characters);
  assert(status == 0);
  return static_cast<std::size_t>(size_in_characters);
}

std::size_t charset_string::get_size_in_bytes() const noexcept {
  assert(impl_);
  uint size_in_bytes = 0;
  [[maybe_unused]] auto status = (*get_services().byte_access->get_byte_length)(
      to_h_string(impl_.get()), &size_in_bytes);
  assert(status == 0);
  return static_cast<std::size_t>(size_in_bytes);
}

void charset_string::clear() noexcept {
  assert(impl_);
  [[maybe_unused]] auto status =
      (*get_services().reset->reset)(to_h_string(impl_.get()));
  assert(status == 0);
}

std::uint32_t charset_string::operator[](std::size_t index) const noexcept {
  assert(impl_);
  ulong ch{0};
  [[maybe_unused]] auto status = (*get_services().character_access->get_char)(
      to_h_string(impl_.get()), index, &ch);
  assert(status == 0);
  return static_cast<std::uint32_t>(ch);
}

std::uint32_t charset_string::at(std::size_t index) const {
  if (index >= get_size_in_characters()) {
    throw std::out_of_range{"charset_string"};
  }
  return (*this)[index];
}

charset_string &charset_string::append(const charset_string &another) {
  assert(impl_);
  assert(another.impl_);

  if (get_collation() != another.get_collation())
    throw std::runtime_error{
        "cannot concatenate strings with different collations"};
  if ((*get_services().append->append)(to_h_string(impl_.get()),
                                       to_h_string(another.impl_.get())) != 0)
    throw std::runtime_error{"cannot concatenate strings"};
  return *this;
}

charset_string charset_string::substr(std::size_t offset,
                                      std::size_t count) const {
  assert(impl_);

  my_h_string result_handle = nullptr;
  if ((*get_services().substr->substr)(to_h_string(impl_.get()), offset, count,
                                       &result_handle) != 0)
    throw std::runtime_error{"cannot extract substring"};

  charset_string result;
  result.impl_ = impl_type{result_handle, deleter{&get_services()}};
  return result;
}

charset_string charset_string::convert_to_collation_copy(
    collation_type collation) const {
  assert(impl_);
  if (collation == nullptr) throw std::runtime_error{"unknown collation"};
  // if the same collation is requested, just return a copy
  if (get_collation() == collation) return {*this};

  static constexpr std::size_t max_mbchar_length = 4;
  // in the worst case scenario, when the string has only emoji symbold which
  // will be 4-byte encoded (for instance in UTF32), we will need <num_chars> *
  // 4 + 1 bytes for the buffer (including the trailing '\0' character)
  std::size_t buffer_length = get_size_in_characters() * max_mbchar_length + 1;

  // due to the flaw in the 'convert_to_buffer()' interface, which does not
  // allow to receive the number of bytes written to the output buffer,
  // we are pre-filling the buffer with a marker character 'marker_char' before
  // the call and expect the conversion function to terminate its output
  // with the '\0' character
  // the marker character cannot be '\0' as some multibyte characters may be
  // represented by sequences having trailing '\0' (like '\u0100')
  //
  // TODO: this need to be changed to 'convert_to_buffer_v2()' (or something
  // similar) once Oracle fixes the 'mysql_string_charset_converter' API
  static constexpr char marker_char = '*';
  std::string buffer(buffer_length, marker_char);

  if ((*get_services().converter->convert_to_buffer)(
          to_h_string(impl_.get()), buffer.data(), buffer_length,
          to_cs_info_h(collation)) != 0)
    throw std::runtime_error{"cannot convert to another collation"};

  // now we expect the buffer to be filled with
  // .. .. .. 00 42 42 .. 42
  //          ^^
  //   end of the output

  // we just need to find this last '\0' character starting from the end of the
  // string
  auto find_res = buffer.rfind('\0');
  if (find_res == std::string::npos)
    throw std::runtime_error{"unexpected collation conversion output"};
  buffer.resize(find_res);
  return {get_services(), buffer, collation};
}

int charset_string::compare(const charset_string &another) const {
  assert(impl_);
  assert(another.impl_);

  int result{0};
  const auto collation = get_collation();
  charset_string conversion_buffer;
  const charset_string &rhs =
      smart_convert_to_collation(another, collation, conversion_buffer);

  [[maybe_unused]] auto status = (*get_services().compare->compare)(
      to_h_string(impl_.get()), to_h_string(rhs.impl_.get()), &result);
  assert(status == 0);
  return result;
}

charset_string::internal_data charset_string::get_internal() const noexcept {
  assert(impl_);
  const char *ptr = nullptr;
  std::size_t length = 0;
  CHARSET_INFO_h collation = nullptr;
  [[maybe_unused]] auto status =
      (*get_services().get_data_in_charset->get_data)(
          to_h_string(impl_.get()), &ptr, &length, &collation);
  assert(status == 0);
  assert(collation != nullptr);
  return {{ptr, length}, collation};
}

const charset_string &smart_convert_to_collation(
    const charset_string &cs, charset_string::collation_type collation,
    charset_string &buffer) {
  if (collation == nullptr) throw std::runtime_error{"unknown collation"};
  if (cs.get_collation() == collation) return cs;
  buffer = cs.convert_to_collation_copy(collation);
  return buffer;
}

}  // namespace masking_functions
