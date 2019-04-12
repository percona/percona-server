/* Copyright (c) 2018 Percona LLC and/or its affiliates. All rights reserved.

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

#include "system_key_adapter.h"
#include "secure_string.h"

namespace keyring {
// Adds key's version to keyring's key data. The resulting system key data looks
// like this: <key_version>:<keyring key data>
void System_key_adapter::construct_system_key_data() noexcept {
  Secure_ostringstream system_key_data_version_prefix_ss;
  system_key_data_version_prefix_ss << key_version << ':';
  Secure_string system_key_data_version_prefix =
      system_key_data_version_prefix_ss.str();
  size_t system_key_data_candidate_size =
      system_key_data_version_prefix.length() +
      keyring_key->get_key_data_size();
  uchar *system_key_data_candidate =
      new (std::nothrow) uchar[system_key_data_candidate_size];
  if (system_key_data_candidate == nullptr) {
    return;
  }
  memcpy(system_key_data_candidate, system_key_data_version_prefix.c_str(),
         system_key_data_version_prefix.length());
  memcpy(system_key_data_candidate + system_key_data_version_prefix.length(),
         keyring_key->get_key_data(), keyring_key->get_key_data_size());

  // need to "de"-xor keying key data
  keyring_key->xor_data(
      system_key_data_candidate + system_key_data_version_prefix.length(),
      keyring_key->get_key_data_size());
  // next xor system key data as a whole
  keyring_key->xor_data(system_key_data_candidate,
                        system_key_data_candidate_size);

  uchar *null_system_key_data = nullptr;
  if (system_key_data.key_data.compare_exchange_strong(
          null_system_key_data, system_key_data_candidate)) {
    system_key_data.key_data_size = system_key_data_candidate_size;
  } else {
    delete[] system_key_data_candidate;  // too late - system key data was
                                         // already constructed
  }
}

System_key_adapter::System_key_data::System_key_data()
    : key_data(nullptr), key_data_size(0) {}

System_key_adapter::System_key_data::~System_key_data() { free(); }

void System_key_adapter::System_key_data::free() {
  if (key_data.load()) {
    DBUG_ASSERT(key_data_size <= 512);
    memset_s(key_data.load(), 512, 0, key_data_size);
    delete[] key_data.load();
    key_data.store(nullptr);
    key_data_size = 0;
  }
}

}  // namespace keyring
