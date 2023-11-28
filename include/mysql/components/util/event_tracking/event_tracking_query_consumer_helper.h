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

#ifndef EVENT_TRACKING_QUERY_CONSUMER_HELPER_H
#define EVENT_TRACKING_QUERY_CONSUMER_HELPER_H

#include "mysql/components/component_implementation.h"
#include "mysql/components/service_implementation.h"
#include "mysql/components/services/event_tracking_query_service.h"

/**
  @file event_tracking_query_consumer_helper.h
  Helper file to create query event consumer
*/

// clang-format off
/**
  @anchor EVENT_TRACKING_QUERY_CONSUMER_EXAMPLE

  @code

  #include "mysql/components/util/event_tracking_query_consumer_helper.h"

  namespace Event_tracking_implementation {

  // Replace following with union of subevents to be filtered
  mysql_event_tracking_query_subclass_t
    Event_tracking_query_implementation::filtered_sub_events = 0;

  bool Event_tracking_query_implementation::callback(
      const mysql_event_tracking_query_data *data [[maybe_unused]]) {
      // Your code goes here
  }
  }  // namespace Event_tracking_implementation

  // Define init/deinit methods for component

  // Component declaration related stuff

  IMPLEMENTS_SERVICE_EVENT_TRACKING_QUERY(<implementation_name>);

  BEGIN_COMPONENT_PROVIDES(<component_name>)
  PROVIDES_SERVICE_EVENT_TRACKING_QUERY(<implementation_name>)
  END_COMPONENT_PROVIDES()

  // Rest of the component declaration code

  @endcode
*/
// clang-format on

#define PROVIDES_SERVICE_EVENT_TRACKING_QUERY(component) \
  PROVIDES_SERVICE(component, event_tracking_query)

#define IMPLEMENTS_SERVICE_EVENT_TRACKING_QUERY(component)                   \
  BEGIN_SERVICE_IMPLEMENTATION(component, event_tracking_query)              \
  Event_tracking_implementation::Event_tracking_query_implementation::notify \
  END_SERVICE_IMPLEMENTATION()

namespace Event_tracking_implementation {
/** Implementation helper class for query events. */
class Event_tracking_query_implementation {
 public:
  /** Sub-events to be filtered/ignored - To be defined by the component */
  static mysql_event_tracking_query_subclass_t filtered_sub_events;

  /** Callback function - To be implemented by component to handle an event */
  static bool callback(const mysql_event_tracking_query_data *data);

  /**
    event_tracking_query service implementation

    @param [in] data  Data related to query event

    @returns Status of operation
      @retval false Success
      @retval true  Failure
  */
  static DEFINE_BOOL_METHOD(notify,
                            (const mysql_event_tracking_query_data *data)) {
    try {
      if (!data) return true;
      if (filtered_sub_events & data->event_subclass) return false;
      return callback(data);
    } catch (...) {
      return true;
    }
  }
};
}  // namespace Event_tracking_implementation

#endif  // !EVENT_TRACKING_QUERY_CONSUMER_HELPER_H
