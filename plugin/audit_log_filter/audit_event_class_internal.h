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

enum class audit_filter_event_class_t { AUDIT_FILTER_INTERNAL_CLASS };

enum class audit_filter_event_subclass_t {
  AUDIT_FILTER_INTERNAL_AUDIT,
  AUDIT_FILTER_INTERNAL_NOAUDIT
};

/**
 * @struct audit_filter_event_audit
 * Structure for AUDIT_FILTER_INTERNAL_AUDIT event.
 */
struct audit_filter_event_internal_audit {
  audit_filter_event_subclass_t event_subclass;
  uint32 server_id;
};

/**
 * @struct audit_filter_event_audit
 * Structure for AUDIT_FILTER_INTERNAL_NOAUDIT event.
 */
struct audit_filter_event_internal_noaudit {
  audit_filter_event_subclass_t event_subclass;
  uint32 server_id;
};

}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_EVENT_CLASS_INTERNAL_H_INCLUDED
