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

#ifndef SQL_SERVER_COMPONENT_EVENT_TRACKING_INFORMATION_IMP_H
#define SQL_SERVER_COMPONENT_EVENT_TRACKING_INFORMATION_IMP_H

#include "mysql/components/component_implementation.h"
#include "mysql/components/services/event_tracking_authentication_service.h"
#include "mysql/components/services/event_tracking_general_service.h"

class Event_tracking_authentication_information_imp {
 public:
  /**
    Initialize authentication event data handle

    @param [out] handle  Handle to authentication event data

    @returns Status of handle creation
      @retval false Success
      @retval true  Error
  */
  static DEFINE_BOOL_METHOD(init,
                            (event_tracking_authentication_information_handle *
                             handle));

  /**
    Deinitialize authentication event data handle

    @param [in, out] handle Handle to be deinitialized

    @returns Status of operation
      @retval false Success
      @retval true  Error
  */
  static DEFINE_BOOL_METHOD(
      deinit, (event_tracking_authentication_information_handle handle));

  /**
    Get information about given authentication event

    Accepted names and corresponding value type

    "authentcation_method_count" -> uint8_t *
    "new_user" -> mysql_cstring_with_length *
    "new_host" -> mysql_cstring_with_length *
    "is_role" -> boolean *

    @param [in]  handle Event tracking information handle
    @param [in]  name   Data identifier
    @param [out] value  Value of the identifier

    @returns status of the operation
      @retval false Success
      @retval true  Error
  */
  static DEFINE_BOOL_METHOD(
      get, (event_tracking_authentication_information_handle handle,
            const char *name, void *value));
};

class Event_tracking_authentication_method_imp {
 public:
  /**
    Get information about authentication method

    Accepted names and corresponding value type

    "name" -> mysql_cstring_with_length *

    @param [in]  handle  Handle to authentication method structure
                         Valid until
                      @sa event_tracking_authentication_information_handle_imp
                         is valid
    @param [in]  index   Location
    @param [in]  name    Data identifier
    @param [out] value   Data value

    @returns status of the operation
      @retval false Success
      @retval true  Error
  */
  static DEFINE_BOOL_METHOD(get,
                            (event_tracking_authentication_method_handle handle,
                             unsigned int index, const char *name,
                             void *value));
};

class Event_tracking_general_information_imp {
 public:
  /**
    Initialize authentication event data handle

    @param [out] handle  Handle to authentication event data

    @returns Status of handle creation
      @retval false Success
      @retval true  Error
  */
  static DEFINE_BOOL_METHOD(init, (event_tracking_general_information_handle *
                                   handle));

  /**
    Deinitialize authentication event data handle

    @param [in, out] handle Handle to be deinitialized

    @returns Status of operation
      @retval false Success
      @retval true  Error
  */
  static DEFINE_BOOL_METHOD(deinit,
                            (event_tracking_general_information_handle handle));

  /**
    Get information about given authentication event

    Accepted names and corresponding value type

    "external_user" -> mysql_cstring_with_length *
    "time" -> uint64_t
    "rows" -> uint64_t

    @param [in]  handle Event tracking information handle
    @param [in]  name   Data identifier
    @param [out] value  Value of the identifier

    @returns status of the operation
      @retval false Success
      @retval true  Error
  */
  static DEFINE_BOOL_METHOD(get,
                            (event_tracking_general_information_handle handle,
                             const char *name, void *value));
};

#endif  // !SQL_SERVER_COMPONENT_EVENT_TRACKING_INFORMATION_IMP_H
