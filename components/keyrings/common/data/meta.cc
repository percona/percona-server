/* Copyright (c) 2021, Oracle and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include <climits>
#include <sstream>

#include "meta.h"

namespace {

const std::string versioned_key_prefix = "percona_";

bool parse_key_id_with_version(const std::string &key_id_with_version,
                               std::string &key_id, uint &version) {
  const std::size_t colon_position = key_id_with_version.find_last_of(':');

  if (colon_position == std::string::npos ||
      colon_position == key_id_with_version.length() - 1) {
    return false;
  }

  key_id = key_id_with_version.substr(0, colon_position);
  const std::string parsed_version = key_id_with_version.substr(
      colon_position + 1, key_id_with_version.length() - colon_position);

  if (parsed_version.empty()) {
    return false;
  }

  char *endptr = nullptr;
  uint32_t ulong_key_version = strtoul(parsed_version.c_str(), &endptr, 10);
  if (ulong_key_version > UINT_MAX || endptr == nullptr || *endptr != '\0') {
    return false;
  }

  version = static_cast<uint>(ulong_key_version);
  return true;
}

}  // namespace

namespace keyring_common {
namespace meta {

/** Constructor */
Metadata::Metadata(const std::string key_id, const std::string owner_id,
                   const uint key_version)
    : key_id_(key_id),
      owner_id_(owner_id),
      hash_key_(),
      key_version_(key_version) {
  if (check_key_versioned()) {
    std::string key_id_without_version;
    uint parsed_version = 0;
    if (parse_key_id_with_version(key_id, key_id_without_version,
                                  parsed_version)) {
      key_id_ = key_id_without_version;
      key_version_ = parsed_version;
    }
  } else {
    // Key versioning not supported, always use default version internally
    key_version_ = KEY_DEFAULT_VERSION;
  }

  valid_ = !(key_id_.empty() && owner_id_.empty());
  create_hash_key();
}

Metadata::Metadata(const char *key_id, const char *owner_id,
                   const uint key_version)
    : Metadata(key_id != nullptr ? std::string{key_id} : std::string{},
               owner_id != nullptr ? std::string{owner_id} : std::string{},
               key_version) {}

Metadata::Metadata() : Metadata(std::string{}, std::string{}) {}

/** Copy constructor */
Metadata::Metadata(const Metadata &src)
    : Metadata(src.key_id_, src.owner_id_, src.key_version_) {}

/** Move constructor */
Metadata::Metadata(Metadata &&src) noexcept {
  std::swap(src.key_id_, key_id_);
  std::swap(src.owner_id_, owner_id_);
  std::swap(src.hash_key_, hash_key_);
  std::swap(src.key_version_, key_version_);
  std::swap(src.valid_, valid_);
}

/** Assignment operator */
Metadata &Metadata::operator=(const Metadata &src) = default;

Metadata &Metadata::operator=(Metadata &&src) noexcept {
  std::swap(src.key_id_, key_id_);
  std::swap(src.owner_id_, owner_id_);
  std::swap(src.hash_key_, hash_key_);
  std::swap(src.key_version_, key_version_);
  std::swap(src.valid_, valid_);

  return *this;
}

/** Destructor */
Metadata::~Metadata() { valid_ = false; }

/** Get key ID */
const std::string Metadata::key_id() const { return key_id_; }

/** Get versioned key ID */
const std::string Metadata::versioned_key_id() const {
  if (check_key_versioned()) {
    std::stringstream id;
    id << key_id_ << ":" << key_version_;
    return id.str();
  }

  return key_id();
}

/** Get owner info */
const std::string Metadata::owner_id() const { return owner_id_; }

/** Get key version */
uint Metadata::key_version() const { return key_version_; }

/** Check if key supports versioning */
bool Metadata::check_key_versioned() const {
  return !key_id_.empty() && key_id_.compare(0, versioned_key_prefix.length(),
                                             versioned_key_prefix) == 0;
}

/** Validity of metadata object */
bool Metadata::valid() const { return valid_; }

/** create hash key */
void Metadata::create_hash_key() {
  if (valid_) {
    hash_key_ = key_id_;
    if (!owner_id_.empty()) {
      hash_key_.push_back('\0');
      hash_key_.append(owner_id_);
    }
    hash_key_.push_back('\0');
    hash_key_.append(std::to_string(key_version_));
  }
}

}  // namespace meta
}  // namespace keyring_common
