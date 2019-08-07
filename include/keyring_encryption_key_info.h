/* Copyright (c) 2019 Percona LLC and/or its affiliates. All rights reserved.

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

#ifndef keyring_encryption_key_info_h
#define keyring_encryption_key_info_h

#include <cstdint>

using EncryptionKeyId = uint32_t;

static constexpr uint ENCRYPTION_KEY_VERSION_INVALID = (~(uint)0);
static constexpr uint FIL_DEFAULT_ENCRYPTION_KEY = 0;
static constexpr uint ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED = 0;

struct KeyringEncryptionKeyIdInfo {
  KeyringEncryptionKeyIdInfo(bool was_encryption_key_id_set,
                             EncryptionKeyId encryption_key_id)
      : was_encryption_key_id_set(was_encryption_key_id_set),
        id(encryption_key_id) {}

  KeyringEncryptionKeyIdInfo() {}

  bool was_encryption_key_id_set{false};
  EncryptionKeyId id{FIL_DEFAULT_ENCRYPTION_KEY};
};

#endif
