/* Copyright (c) 2023 Oracle and/or its affiliates.

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

#ifndef DEBUG_SYNC_SERVICE_GUARD
#define DEBUG_SYNC_SERVICE_GUARD

#include "mysql/components/service.h"
#include "mysql/components/services/mysql_current_thread_reader.h"  // MYSQL_THD

/**
  @ingroup group_components_services_inventory

  Enable capability to process debug_sync point from components.

  MySQL server provides DEBUG_SYNC macro to enable controlled testing.
  This service makes the same functionality to component.

  The usage remains the same way as used in server. E.g.,
        DEBUG_SYNC("debug sync point");
*/
BEGIN_SERVICE_DEFINITION(mysql_debug_sync_service)

/**
  Process debug_sync point.

  @param[in]  thd   The THD pointer to current thread.
  @param[in]  name  The debug point name to process.
*/
DECLARE_METHOD(void, debug_sync, (MYSQL_THD thd, const char *name));

END_SERVICE_DEFINITION(mysql_debug_sync_service)

#endif  // DEBUG_SYNC_SERVICE_GUARD