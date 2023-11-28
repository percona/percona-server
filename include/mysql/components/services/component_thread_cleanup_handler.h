/* Copyright (c) 2022, 2023, Oracle and/or its affiliates.

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

#ifndef COMPONENT_THREAD_CLEANUP_HANDLER_GUARD
#define COMPONENT_THREAD_CLEANUP_HANDLER_GUARD

#include <mysql/components/service.h>

DEFINE_SERVICE_HANDLE(component_thread_cleanup_handler);

/**
  @ingroup group_components_services_inventory

A service to cleanup component thread resource.
                                    */
BEGIN_SERVICE_DEFINITION(component_thread_cleanup_handler)

/**
  Component thread exit handler, invoked by
  mysql_service_thread_cleanup_register.

  @return true if error false otherwise.
*/
DECLARE_BOOL_METHOD(exit_handler, ());

END_SERVICE_DEFINITION(component_thread_cleanup_handler)

#endif /* LANGUAGE_SERVICE_GUARD */
