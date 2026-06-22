/* Copyright (c) 2026, Percona LLC and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is designed to work with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have either included with
   the program or referenced in the documentation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef AUTH_PLUGIN_SHUTDOWN_INCLUDED
#define AUTH_PLUGIN_SHUTDOWN_INCLUDED

bool begin_auth_plugin_operation();
void end_auth_plugin_operation();
void start_auth_plugin_shutdown_and_wait();

class Auth_plugin_operation_guard {
 public:
  Auth_plugin_operation_guard()
      : m_has_operation(begin_auth_plugin_operation()) {}

  ~Auth_plugin_operation_guard() {
    if (m_has_operation) end_auth_plugin_operation();
  }

  bool has_operation() const { return m_has_operation; }
  explicit operator bool() const { return m_has_operation; }

 private:
  bool m_has_operation;
};

#endif  // AUTH_PLUGIN_SHUTDOWN_INCLUDED
