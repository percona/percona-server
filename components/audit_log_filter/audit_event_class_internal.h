/* Copyright (c) 2022 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#ifndef AUDIT_LOG_FILTER_EVENT_CLASS_INTERNAL_H_INCLUDED
#define AUDIT_LOG_FILTER_EVENT_CLASS_INTERNAL_H_INCLUDED

#include "my_inttypes.h"

namespace audit_log_filter {

enum class audit_event_class_t {
  AUDIT_GENERAL_CLASS,
  AUDIT_CONNECTION_CLASS,
  AUDIT_PARSE_CLASS,
  AUDIT_AUTHORIZATION_CLASS,
  AUDIT_TABLE_ACCESS_CLASS,
  AUDIT_GLOBAL_VARIABLE_CLASS,
  AUDIT_SERVER_STARTUP_CLASS,
  AUDIT_SERVER_SHUTDOWN_CLASS,
  AUDIT_COMMAND_CLASS,
  AUDIT_QUERY_CLASS,
  AUDIT_STORED_PROGRAM_CLASS,
  AUDIT_AUTHENTICATION_CLASS,
  AUDIT_MESSAGE_CLASS,
  AUDIT_INTERNAL_AUDIT_CLASS,
  AUDIT_INTERNAL_UNKNOWN_CLASS
};

#define INTERNAL_EVENT_TRACKING_AUDIT_AUDIT (1 << 0)
#define INTERNAL_EVENT_TRACKING_AUDIT_NOAUDIT (1 << 1)

typedef unsigned long internal_event_tracking_audit_subclass_t;

/**
 * @struct audit_event_audit
 * Structure for AUDIT_INTERNAL_AUDIT event.
 */
struct internal_event_tracking_audit_data {
  internal_event_tracking_audit_subclass_t event_subclass;
  uint32 server_id;
};

}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_EVENT_CLASS_INTERNAL_H_INCLUDED
