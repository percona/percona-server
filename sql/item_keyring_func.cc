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

#include "item_keyring_func.h"
#include <mysql/service_mysql_keyring.h>
#include <algorithm>
#include <cstring>
#include <sstream>
#include "mysqld.h"
#include "sql/auth/auth_acls.h"
#include "sql/sql_lex.h"
#include "sql_class.h"  // THD
#include "system_key.h"

bool Item_func_rotate_system_key::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (Item_bool_func::itemize(pc, res)) return true;
  pc->thd->lex->set_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_SYSTEM_FUNCTION);
  pc->thd->lex->safe_to_cache_query = false;
  return false;
}

longlong Item_func_rotate_system_key::val_int() {
  assert(fixed);

  if (args[0]->result_type() != STRING_RESULT)  // String argument expected
    return 0;

  String buffer;
  String *arg_str = args[0]->val_str(&buffer);

  if (!arg_str)  // Out-of memory happened. The error has been reported.
    return 0;    // Or: the underlying field is NULL

  return calc_value(arg_str) ? 1 : 0;
}

bool Item_func_rotate_system_key::calc_value(const String *arg) {
  assert(strlen(server_uuid) != 0);
  size_t key_length = 0;

  if (!is_valid_percona_system_key(arg->ptr(), &key_length)) return false;

  std::ostringstream key_id_with_uuid_ss;
  key_id_with_uuid_ss << arg->ptr() << '-' << server_uuid;
  std::string key_id_with_uuid = key_id_with_uuid_ss.str();

  // It should only be possible to rotate already existing key.
  // First check that system key exists.
  char *key_type = nullptr;
  size_t key_len;
  void *key = nullptr;
  if (my_key_fetch(key_id_with_uuid.c_str(), &key_type, NULL, &key, &key_len) ||
      nullptr == key) {
    if (nullptr != key) {
      my_free(key);
    }
    if (nullptr != key_type) {
      my_free(key_type);
    }
    return 0;
  }
  assert(memcmp(key_type, "AES", 3) == 0);
  my_free(key_type);
  my_free(key);
  key = key_type = nullptr;

  // rotate the key
  return !(my_key_generate(key_id_with_uuid.c_str(), "AES", NULL, key_length));
}

bool Item_func_rotate_system_key::fix_fields(THD *thd, Item **ref) {
  const bool res = Item_bool_func::fix_fields(thd, ref);
  if (!res && !thd->security_context()->check_access(SUPER_ACL)) {
    my_error(ER_SPECIFIC_ACCESS_DENIED_ERROR, MYF(0), "SUPER");
    return true;
  }
  return res;
}
