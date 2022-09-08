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

#ifndef AUDIT_LOG_FILTER_AUDIT_LOG_READER_H_INCLUDED
#define AUDIT_LOG_FILTER_AUDIT_LOG_READER_H_INCLUDED

#include "plugin/audit_log_filter/component_registry_service.h"

#include "plugin/audit_log_filter/sys_vars.h"

#include <map>
#include <string>

namespace audit_log_filter {

struct AuditLogReaderArgs {
  std::string timestamp;
  uint64_t id;
  uint64_t max_array_length;
  bool close_read_sequence;
};

struct AuditLogReaderContext {
  LogBookmark next_event_bookmark;
};

class AuditLogReader {
 public:
  explicit AuditLogReader(comp_registry_srv_t *comp_registry_srv);

  bool init() noexcept;

  bool read(const AuditLogReaderArgs &reader_args,
            AuditLogReaderContext *reader_context, char *out_buff,
            ulong out_buff_size) noexcept;

  static AuditLogReaderContext *init_reader_session(
      const AuditLogReaderArgs &reader_args) noexcept;

  void close_reader_session(AuditLogReaderContext *reader_context) noexcept;

 private:
  auto get_log_file_handle(
      AuditLogReaderContext *reader_context) const noexcept;

 private:
  comp_registry_srv_t *m_comp_registry_srv;
  std::map<std::string, std::string> m_first_timestamp_to_file_map;
};

}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_AUDIT_LOG_READER_H_INCLUDED
