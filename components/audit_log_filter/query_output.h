/* Copyright (c) 2026 Percona LLC and/or its affiliates. All rights reserved.

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

#ifndef AUDIT_LOG_FILTER_QUERY_OUTPUT_H_INCLUDED
#define AUDIT_LOG_FILTER_QUERY_OUTPUT_H_INCLUDED

#include "components/audit_log_filter/audit_record.h"

namespace audit_log_filter {
// Run after filtering and before entering the writer. Publishes output only
// after every query field succeeds. Throws without exposing query text.
void prepare_query_output(AuditRecordVariant &record);
}  // namespace audit_log_filter
#endif  // AUDIT_LOG_FILTER_QUERY_OUTPUT_H_INCLUDED
