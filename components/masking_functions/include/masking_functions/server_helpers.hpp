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

#ifndef MASKING_FUNCTIONS_SERVER_HELPERS_HPP
#define MASKING_FUNCTIONS_SERVER_HELPERS_HPP

#include <functional>

namespace masking_functions {

using lock_protected_function = std::function<void()>;

// this function takes a read lock on 'LOCK_server_shutting_down' and
// executes 'func' only if the Server is not in SHUTDOWN mode
// ('server_shutting_down' is false)
// returns false if the Server is in SHUTDOWN mode
// returns true if the Server is not in SHUTDOWN mode and func executed
// without exceptions
// this function may throw if 'func' throws
bool execute_under_lock_if_not_in_shutdown(const lock_protected_function &func);

}  // namespace masking_functions

#endif  // MASKING_FUNCTIONS_SERVER_HELPERS_HPP
