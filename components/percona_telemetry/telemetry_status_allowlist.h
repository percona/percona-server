/* Copyright (c) 2026 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; version 2 of
   the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#ifndef TELEMETRY_STATUS_ALLOWLIST_H
#define TELEMETRY_STATUS_ALLOWLIST_H

#include <string_view>

/*
  Allow list for performance_schema.global_status names collected by the
  telemetry component. The names below are quoted, comma-joined and ready
  to drop into SQL `IN (...)` (without surrounding parentheses). Adjacent
  string literals concatenate at translation time, so this is one
  contiguous read-only blob in `.rodata`; no allocation, no runtime
  construction.

  Maintenance:
    * keep entries lexicographically sorted;
    * every line ends with a trailing comma except the last;
    * when adding/removing the last entry, fix up the comma on its neighbor.

  Counters and numeric aggregates only; no SSL strings, paths, hostnames,
  or InnoDB human-readable status strings. Values are still filtered
  against `/` and `\` characters at collection time.
*/
inline constexpr std::string_view kStatusAllowlistCsv =
    "'Aborted_clients',"
    "'Aborted_connects',"
    "'auth_openid_connect_users_authenticated',"
    "'auth_openid_connect_users_denied',"
    "'auth_openid_connect_users_proxied',"
    "'auth_openid_connect_users_with_roles',"
    "'auth_openid_connect_configured_idps',"
    "'auth_openid_connect_configured_idps_using_jwks',"
    "'auth_openid_connect_configured_group_role_maps',"
    "'Acl_cache_items_count',"
    "'Binlog_cache_disk_use',"
    "'Binlog_cache_use',"
    "'Binlog_stmt_cache_disk_use',"
    "'Binlog_stmt_cache_use',"
    "'Bytes_received',"
    "'Bytes_sent',"
    "'Com_delete',"
    "'Com_insert',"
    "'Com_select',"
    "'Com_update',"
    "'Connection_errors_internal',"
    "'Connection_errors_max_connections',"
    "'Connection_errors_peer_address',"
    "'Connections',"
    "'Created_tmp_disk_tables',"
    "'Created_tmp_tables',"
    "'Handler_commit',"
    "'Handler_delete',"
    "'Handler_read_first',"
    "'Handler_read_key',"
    "'Handler_read_next',"
    "'Handler_read_prev',"
    "'Handler_read_rnd',"
    "'Handler_read_rnd_next',"
    "'Handler_update',"
    "'Handler_write',"
    "'Innodb_buffer_pool_read_requests',"
    "'Innodb_buffer_pool_reads',"
    "'Innodb_data_read',"
    "'Innodb_data_reads',"
    "'Innodb_data_writes',"
    "'Innodb_data_written',"
    "'Innodb_dblwr_pages_written',"
    "'Innodb_dblwr_writes',"
    "'Innodb_log_waits',"
    "'Innodb_log_write_requests',"
    "'Innodb_log_writes',"
    "'Innodb_os_log_written',"
    "'Innodb_pages_created',"
    "'Innodb_pages_read',"
    "'Innodb_pages_written',"
    "'Innodb_redo_log_read_only',"
    "'Innodb_row_lock_time',"
    "'Innodb_row_lock_time_avg',"
    "'Innodb_row_lock_time_max',"
    "'Innodb_row_lock_waits',"
    "'Innodb_rows_deleted',"
    "'Innodb_rows_inserted',"
    "'Innodb_rows_read',"
    "'Innodb_rows_updated',"
    "'Libcoredumper_enabled',"
    "'Max_used_connections',"
    "'Open_files',"
    "'Open_table_definitions',"
    "'Open_tables',"
    "'Prepared_stmt_count',"
    "'Queries',"
    "'Questions',"
    "'Select_full_join',"
    "'Select_full_range_join',"
    "'Select_range',"
    "'Select_range_check',"
    "'Select_scan',"
    "'Slow_queries',"
    "'Sort_merge_passes',"
    "'Sort_range',"
    "'Sort_rows',"
    "'Sort_scan',"
    "'Table_locks_immediate',"
    "'Table_locks_waited',"
    "'Threadpool_idle_threads',"
    "'Threadpool_threads',"
    "'Threads_cached',"
    "'Threads_connected',"
    "'Threads_created',"
    "'Threads_running'";

#endif /* TELEMETRY_STATUS_ALLOWLIST_H */
