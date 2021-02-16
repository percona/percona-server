/* Copyright (c) 2018, 2021 Percona LLC and/or its affiliates. All rights reserved.

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

#ifndef MYSQL_GUNIT_INCORRECT_VAULT_KEY_H
#define MYSQL_GUNIT_INCORRECT_VAULT_KEY_H

#include "vault_key.h"

namespace keyring {

struct Incorrect_vault_key : public Vault_key {
  Incorrect_vault_key(const char *a_key_id, const char *a_key_type,
                      const char *a_user_id, const void *a_key,
                      size_t a_key_len)
      : Vault_key(a_key_id, a_key_type, a_user_id, a_key, a_key_len),
        add_to_key_id_length(0),
        add_to_user_id_length(0)
  {
  }

  Incorrect_vault_key(const Incorrect_vault_key &incorrect_vault_key)
      : Vault_key(incorrect_vault_key),
        add_to_key_id_length(incorrect_vault_key.add_to_key_id_length),
        add_to_user_id_length(incorrect_vault_key.add_to_user_id_length)
  {
  }

  int add_to_key_id_length;
  int add_to_user_id_length;

  virtual my_bool get_next_key(IKey **key)
  {
    if (was_key_retrieved)
    {
      *key= NULL;
      return TRUE;
    }
    *key= new Incorrect_vault_key(*this);
    was_key_retrieved= true;
    return FALSE;
  }

  virtual void create_key_signature() const
  {
    if (key_id.empty())
      return;
    std::ostringstream key_signature_ss;
    key_signature_ss << key_id.length() + add_to_key_id_length << '_';
    key_signature_ss << key_id;
    key_signature_ss << user_id.length() + add_to_user_id_length << '_';
    key_signature_ss << user_id;
    key_signature= key_signature_ss.str();
  }
};

}  // namespace keyring

#endif  // MYSQL_GUNIT_INCORRECT_VAULT_KEY_H
