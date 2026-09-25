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

#include "components/audit_log_filter/query_output.h"

#include "components/audit_log_filter/charset_conversion.h"
#include "my_dbug.h"
#include "my_sys.h"
#include "mysql/components/services/defs/event_tracking_parse_defs.h"
#include "mysql/components/services/defs/event_tracking_query_defs.h"
#include "mysql/strings/m_ctype.h"

#include <new>
#include <stdexcept>
#include <type_traits>

namespace audit_log_filter {
namespace {
std::string_view to_string_view(const mysql_cstring_with_length &query) {
  if (query.length == 0) return {};
  if (query.str == nullptr) {
    throw std::invalid_argument("Invalid audit query buffer");
  }
  return {query.str, query.length};
}

std::string convert(std::string_view query, const std::string &charset) {
  if (query.empty()) return {};
  const CHARSET_INFO *source =
      charset.empty()
          ? nullptr
          : get_charset_by_csname(charset.c_str(), MY_CS_PRIMARY, MYF(0));
  DBUG_EXECUTE_IF("audit_log_filter_unknown_query_charset", source = nullptr;);
  return convert_query_to_utf8mb4(query, source);
}
}  // namespace

bool query_output_is_ready(const AuditRecordVariant &record) noexcept {
  return std::visit(
      [](const auto &rec) {
        using Record = std::decay_t<decltype(rec)>;
        if constexpr (std::is_same_v<Record, AuditRecordGeneral> ||
                      std::is_same_v<Record, AuditRecordTableAccess> ||
                      std::is_same_v<Record, AuditRecordQuery> ||
                      std::is_same_v<Record, AuditRecordParse>) {
          return rec.extended_info.query_output.has_value();
        }
        return true;
      },
      record);
}

void prepare_query_output(AuditRecordVariant &record) {
  std::visit(
      [](auto &rec) {
        using Record = std::decay_t<decltype(rec)>;
        if constexpr (std::is_same_v<Record, AuditRecordGeneral> ||
                      std::is_same_v<Record, AuditRecordTableAccess> ||
                      std::is_same_v<Record, AuditRecordQuery> ||
                      std::is_same_v<Record, AuditRecordParse>) {
          auto &extra = rec.extended_info;
          extra.query_output.reset();
          DBUG_EXECUTE_IF("audit_log_filter_query_output_bad_alloc",
                          throw std::bad_alloc(););
          QueryOutput output;
          if (!extra.digest.empty()) {
            output.query = sanitize_utf8(extra.digest);
          } else if constexpr (std::is_same_v<Record, AuditRecordGeneral> ||
                               std::is_same_v<Record, AuditRecordTableAccess>) {
            if (!extra.query) {
              throw std::runtime_error("Failed to capture audit query");
            }
            output.query = convert(*extra.query, extra.query_charset);
          } else {
            output.query =
                convert(to_string_view(rec.event->query), extra.query_charset);
          }
          if constexpr (std::is_same_v<Record, AuditRecordParse>) {
            if (rec.event->rewritten_query != nullptr) {
              output.rewritten_query =
                  convert(to_string_view(*rec.event->rewritten_query),
                          extra.query_charset);
            }
          }
          extra.query_output = std::move(output);
        }
      },
      record);
}
}  // namespace audit_log_filter
