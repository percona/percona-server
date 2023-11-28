/* Copyright (c) 2023, Oracle and/or its affiliates.

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

#ifndef PLUGIN_PFS_TABLE_BINLOG_STORAGE_ITERATOR_TESTS_H_
#define PLUGIN_PFS_TABLE_BINLOG_STORAGE_ITERATOR_TESTS_H_

#include <mysql/components/services/binlog_storage_iterator.h>
#include <mysql/components/services/pfs_plugin_table_service.h>
#include <string>
#include "libbinlogevents/include/binary_log.h"
#include "mysql_version.h"

namespace binlog::service::iterators::tests {

const std::string TABLE_NAME{"binlog_storage_iterator_entries"};

static const uint32_t MAX_STORAGE_NAME_SIZE{1024};

bool register_pfs_tables();
bool unregister_pfs_tables();

struct Row {
  binary_log::Log_event_type event_type{binary_log::UNKNOWN_EVENT};
  std::string event_name{};
  std::string storage_details{};
  std::string trx_uuid{};
  uint64_t trx_seqno{0};
  uint64_t start_position{0};
  uint64_t end_position{0};
  std::string extra{};

  void reset() {
    event_type = binary_log::UNKNOWN_EVENT;
    event_name = binary_log::get_event_type_as_string(event_type);
    storage_details = "";
    trx_seqno = 0;
    trx_uuid = "";
    start_position = 0;
    extra = "";
  }
};

/* A structure to define a handle for table in plugin/component code. */
struct Cs_entries_table {
  struct Row row;
  binary_log::Format_description_event fde{BINLOG_VERSION,
                                           MYSQL_SERVER_VERSION};
  my_h_binlog_storage_iterator iterator;
  unsigned char *buffer{nullptr};
  uint64_t buffer_capacity{0};
  uint64_t buffer_size{0};
  unsigned long long s_current_row_pos{0};
  bool is_error{false};

  bool extend_buffer_capacity(uint64_t size = 0);
  void delete_buffer() const;
};

PSI_table_handle *open_table(PSI_pos **pos);
void close_table(PSI_table_handle *handle);
int rnd_next(PSI_table_handle *handle);
int rnd_init(PSI_table_handle *h, bool scan);
void init_share(PFS_engine_table_share_proxy *share);

}  // namespace binlog::service::iterators::tests

#endif /* PLUGIN_PFS_TABLE_BINLOG_STORAGE_ITERATOR_TESTS_H_ */
