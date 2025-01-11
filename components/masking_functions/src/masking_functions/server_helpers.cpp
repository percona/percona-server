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

#include "masking_functions/server_helpers.hpp"

#include <rwlock_scoped_lock.h>

#include "sql/mysqld.h"

namespace masking_functions {

// this function is put into a separate translation unit as it uses internal
// 'mysqld' interface - 'sql/mysqld.h'
bool execute_under_lock_if_not_in_shutdown(
    const lock_protected_function &func) {
  rwlock_scoped_lock rdlock(&LOCK_server_shutting_down, false, __FILE__,
                            __LINE__);
  if (server_shutting_down) {
    return false;
  }
  func();
  return true;
}

}  // namespace masking_functions
