/* Copyright (c) 2024 Percona LLC and/or its affiliates. All rights reserved.

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

#ifndef TELEMETRY_SYSVARS_ALLOWLIST_H
#define TELEMETRY_SYSVARS_ALLOWLIST_H

#include <string_view>

/*
  Allow list for global system variables collected by the telemetry
  component. The names below are quoted, comma-joined and ready to drop
  into SQL `IN (...)` (without surrounding parentheses). Adjacent string
  literals concatenate at translation time, so this is one contiguous
  read-only blob in `.rodata`; no allocation, no runtime construction.

  Maintenance:
    * keep entries lexicographically sorted;
    * every line ends with a trailing comma except the last;
    * when adding/removing the last entry, fix up the comma on its neighbor.

  Policy: names whose values are usually numeric, enum, or ON/OFF only.
  Paths, host lists, SSL file names, etc. are excluded; values are still
  filtered against `/` and `\` characters at collection time.

  Scope: common MySQL tuning, Percona-specific tuning, clone plugin, RocksDB
  SE (when installed), semi-sync replication, binlog retention/compression.
*/
inline constexpr std::string_view kSysvarsAllowlistCsv =
    "'authentication_policy',"
    "'binlog_checksum',"
    "'binlog_expire_logs_auto_purge',"
    "'binlog_expire_logs_seconds',"
    "'binlog_format',"
    "'binlog_order_commits',"
    "'binlog_row_image',"
    "'binlog_space_limit',"
    "'binlog_transaction_compression',"
    "'binlog_transaction_compression_level_zstd',"
    "'binlog_transaction_dependency_history_size',"
    "'clone_autotune_concurrency',"
    "'clone_block_ddl',"
    "'clone_buffer_size',"
    "'clone_compression_algorithm',"
    "'clone_ddl_timeout',"
    "'clone_delay_after_data_drop',"
    "'clone_donor_timeout_after_network_failure',"
    "'clone_enable_compression',"
    "'clone_max_concurrency',"
    "'clone_max_data_bandwidth',"
    "'clone_max_network_bandwidth',"
    "'clone_zstd_compression_level',"
    "'disconnect_on_expired_password',"
    "'div_precision_increment',"
    "'enforce_gtid_consistency',"
    "'expand_fast_index_creation',"
    "'explicit_defaults_for_timestamp',"
    "'general_log',"
    "'group_replication_consistency',"
    "'group_replication_exit_state_action',"
    "'group_replication_flow_control_mode',"
    "'group_replication_single_primary_mode',"
    "'gtid_mode',"
    "'host_cache_size',"
    "'innodb_adaptive_flushing',"
    "'innodb_adaptive_hash_index',"
    "'innodb_buffer_pool_instances',"
    "'innodb_buffer_pool_size',"
    "'innodb_change_buffering',"
    "'innodb_checksum_algorithm',"
    "'innodb_ddl_threads',"
    "'innodb_deadlock_detect',"
    "'innodb_dedicated_server',"
    "'innodb_doublewrite',"
    "'innodb_file_per_table',"
    "'innodb_fill_factor',"
    "'innodb_flush_log_at_trx_commit',"
    "'innodb_flush_method',"
    "'innodb_flush_neighbors',"
    "'innodb_ft_cache_size',"
    "'innodb_io_capacity',"
    "'innodb_io_capacity_max',"
    "'innodb_log_buffer_size',"
    "'innodb_lru_scan_depth',"
    "'innodb_max_dirty_pages_pct',"
    "'innodb_max_dirty_pages_pct_lwm',"
    "'innodb_old_blocks_pct',"
    "'innodb_old_blocks_time',"
    "'innodb_open_files',"
    "'innodb_page_cleaners',"
    "'innodb_parallel_read_threads',"
    "'innodb_purge_threads',"
    "'innodb_read_ahead_threshold',"
    "'innodb_read_io_threads',"
    "'innodb_redo_log_capacity',"
    "'innodb_spin_wait_delay',"
    "'innodb_stats_persistent',"
    "'innodb_strict_mode',"
    "'innodb_sync_spin_loops',"
    "'innodb_thread_concurrency',"
    "'innodb_write_io_threads',"
    "'join_buffer_size',"
    "'lock_wait_timeout',"
    "'log_bin',"
    "'log_error_verbosity',"
    "'log_replica_updates',"
    "'long_query_time',"
    "'max_allowed_packet',"
    "'max_connections',"
    "'max_heap_table_size',"
    "'max_join_size',"
    "'max_prepared_stmt_count',"
    "'max_slowlog_files',"
    "'max_slowlog_size',"
    "'net_read_timeout',"
    "'net_write_timeout',"
    "'open_files_limit',"
    "'optimizer_switch',"
    "'performance_schema',"
    "'read_buffer_size',"
    "'read_only',"
    "'read_rnd_buffer_size',"
    "'relay_log_recovery',"
    "'replica_compressed_protocol',"
    "'replica_parallel_type',"
    "'replica_parallel_workers',"
    "'replica_preserve_commit_order',"
    "'replica_type_conversions',"
    "'rocksdb_block_cache_size',"
    "'rocksdb_block_size',"
    "'rocksdb_bytes_per_sync',"
    "'rocksdb_compaction_readahead_size',"
    "'rocksdb_db_write_buffer_size',"
    "'rocksdb_enable_bulk_load_api',"
    "'rocksdb_flush_log_at_trx_commit',"
    "'rocksdb_max_background_compactions',"
    "'rocksdb_max_background_flushes',"
    "'rocksdb_max_background_jobs',"
    "'rocksdb_max_open_files',"
    "'rocksdb_max_subcompactions',"
    "'rocksdb_use_io_uring',"
    "'rocksdb_write_policy',"
    "'rpl_semi_sync_replica_enabled',"
    "'rpl_semi_sync_source_enabled',"
    "'skip_name_resolve',"
    "'slow_query_log',"
    "'slow_query_log_always_write_time',"
    "'sort_buffer_size',"
    "'sql_mode',"
    "'sql_require_primary_key',"
    "'super_read_only',"
    "'sync_binlog',"
    "'sync_source_info',"
    "'table_definition_cache',"
    "'table_open_cache',"
    "'table_open_cache_instances',"
    "'terminology_use_previous',"
    "'thread_cache_size',"
    "'thread_handling',"
    "'thread_pool_high_prio_mode',"
    "'thread_pool_high_prio_tickets',"
    "'thread_pool_idle_timeout',"
    "'thread_pool_max_threads',"
    "'thread_pool_min_threads',"
    "'thread_pool_oversubscribe',"
    "'thread_pool_size',"
    "'thread_pool_stall_limit',"
    "'tmp_table_size',"
    "'userstat',"
    "'wait_timeout'";

#endif /* TELEMETRY_SYSVARS_ALLOWLIST_H */
