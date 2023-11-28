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

#ifndef MYSQL_COMPONENTS_SERVICES_EVENT_TRACKING_GENERAL_SERVICE_H
#define MYSQL_COMPONENTS_SERVICES_EVENT_TRACKING_GENERAL_SERVICE_H

#include "mysql/components/service.h"
#include "mysql/components/services/defs/event_tracking_general_defs.h"

/**
  @file mysql/components/services/event_tracking_general_service.h
  General event tracking.

  @sa @ref EVENT_TRACKING_GENERAL_CONSUMER_EXAMPLE
*/

/**  A handle to obtain details related to general event */
DEFINE_SERVICE_HANDLE(event_tracking_general_information_handle);

/**
  @ingroup event_tracking_services_inventory

  @anchor EVENT_TRACKING_GENERAL_SERVICE

  A service to track and consume general events.

  Producer of the event will broadcast notify all interested
  consumers of the event.

  @sa @ref EVENT_TRACKING_GENERAL_CONSUMER_EXAMPLE
*/

BEGIN_SERVICE_DEFINITION(event_tracking_general)

/**
  Process a general event

  @param [in] data  Event specific data

  @returns Status of processing the event
    @retval false Success
    @retval true  Error
*/
DECLARE_BOOL_METHOD(notify, (const mysql_event_tracking_general_data *data));

END_SERVICE_DEFINITION(event_tracking_general)

/**
  @ingroup event_tracking_services_inventory

  @anchor EVENT_TRACKING_GENERAL_INFORMATION

  A service to fetch additional data about authentication event
*/

BEGIN_SERVICE_DEFINITION(event_tracking_general_information)

/**
  Initialize authentication event data handle

  @param [out] handle  Handle to authentication event data

  @returns Status of handle creation
    @retval false Success
    @retval true  Error
*/
DECLARE_BOOL_METHOD(init, (event_tracking_general_information_handle * handle));

/**
  Deinitialize authentication event data handle

  @param [in, out] handle Handle to be deinitialized

  @returns Status of operation
    @retval false Success
    @retval true  Error
*/
DECLARE_BOOL_METHOD(deinit, (event_tracking_general_information_handle handle));

/**
  Get information about given authentication event

  Accepted names and corresponding value type

  "external_user" -> mysql_cstring_with_length
  "time" -> uint64_t
  "rows" -> uint64_t

  @param [in]  handle Event tracking information handle
  @param [in]  name   Data identifier
  @param [out] value  Value of the identifier

  @returns status of the operation
    @retval false Success
    @retval true  Error
*/
DECLARE_BOOL_METHOD(get, (event_tracking_general_information_handle handle,
                          const char *name, void *value));

END_SERVICE_DEFINITION(event_tracking_general_information)

#endif  // !MYSQL_COMPONENTS_SERVICES_EVENT_TRACKING_GENERAL_SERVICE_H
