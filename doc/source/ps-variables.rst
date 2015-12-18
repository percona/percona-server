.. _ps_variables:

====================================================
 List of variables introduced in Percona Server 5.7
====================================================

System Variables
================

.. tabularcolumns:: |p{8.5cm}|p{1.5cm}|p{1.5cm}|p{2cm}|p{1.5cm}|

.. list-table::
   :header-rows: 1

   * - Name
     - Cmd-Line
     - Option File
     - Var Scope
     - Dynamic
   * - :variable:`audit_log_buffer_size`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`audit_log_file`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`audit_log_flush`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`audit_log_format`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`audit_log_handler`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`audit_log_policy`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`audit_log_rotate_on_size`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`audit_log_rotations`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`audit_log_strategy`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`audit_log_syslog_facility`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`audit_log_syslog_ident`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`audit_log_syslog_priority`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`csv_mode`
     - Yes
     - Yes
     - Both
     - Yes
   * - :variable:`enforce_storage_engine`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`expand_fast_index_creation`
     - Yes
     - No
     - Both
     - Yes
   * - :variable:`extra_max_connections`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`extra_port`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`have_backup_locks`
     - Yes
     - No
     - Global
     - No
   * - :variable:`have_backup_safe_binlog_info`
     - Yes
     - No
     - Global
     - No
   * - :variable:`have_snapshot_cloning`
     - Yes
     - No
     - Global
     - No
   * - :variable:`innodb_cleaner_lsn_age_factor`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`innodb_corrupt_table_action`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`innodb_empty_free_list_algorithm`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`innodb_kill_idle_transaction`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`innodb_max_bitmap_file_size`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`innodb_max_changed_pages`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`innodb_show_locks_held`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`innodb_show_verbose_locks`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`innodb_track_changed_pages`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`innodb_use_global_flush_log_at_trx_commit`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`log_slow_filter`
     - Yes
     - Yes
     - Both
     - Yes
   * - :variable:`log_slow_rate_limit`
     - Yes
     - Yes
     - Both
     - Yes
   * - :variable:`log_slow_rate_type`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`log_slow_sp_statements`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`log_slow_verbosity`
     - Yes
     - Yes
     - Both
     - Yes
   * - :variable:`log_warnings_suppress`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`max_binlog_files`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`max_slowlog_files`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`max_slowlog_size`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`proxy_protocol_networks`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`pseudo_server_id`
     - Yes
     - No
     - Session
     - Yes
   * - :variable:`query_cache_strip_comments`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`query_response_time_flush`
     - Yes
     - No
     - Global
     - No
   * - :variable:`query_response_time_range_base`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`query_response_time_stats`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`slow_query_log_always_write_time`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`slow_query_log_use_global_control`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`thread_pool_high_prio_mode`
     - Yes
     - Yes
     - Both
     - Yes
   * - :variable:`thread_pool_high_prio_tickets`
     - Yes
     - Yes
     - Both
     - Yes
   * - :variable:`thread_pool_idle_timeout`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`thread_pool_max_threads`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`thread_pool_oversubscribe`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`thread_pool_size`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`thread_pool_stall_limit`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`thread_statistics`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`tokudb_alter_print_error`
     -
     -
     -
     -
   * - :variable:`tokudb_analyze_delete_fraction`
     -
     -
     -
     -
   * - :variable:`tokudb_analyze_in_background`
     - Yes
     - Yes
     - Both
     - Yes
   * - :variable:`tokudb_analyze_mode`
     - Yes
     - Yes
     - Both
     - Yes
   * - :variable:`tokudb_analyze_throttle`
     - Yes
     - Yes
     - Both
     - Yes
   * - :variable:`tokudb_analyze_time`
     - Yes
     - Yes
     - Both
     - Yes
   * - :variable:`tokudb_auto_analyze`
     - Yes
     - Yes
     - Both
     - Yes
   * - :variable:`tokudb_block_size`
     -
     -
     -
     -
   * - :variable:`tokudb_bulk_fetch`
     -
     -
     -
     -
   * - :variable:`tokudb_cache_size`
     -
     -
     -
     -
   * - :variable:`tokudb_cachetable_pool_threads`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`tokudb_cardinality_scale_percent`
     -
     -
     -
     -
   * - :variable:`tokudb_check_jemalloc`
     -
     -
     -
     -
   * - :variable:`tokudb_checkpoint_lock`
     -
     -
     -
     -
   * - :variable:`tokudb_checkpoint_on_flush_logs`
     -
     -
     -
     -
   * - :variable:`tokudb_checkpoint_pool_threads`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`tokudb_checkpointing_period`
     -
     -
     -
     -
   * - :variable:`tokudb_cleaner_iterations`
     -
     -
     -
     -
   * - :variable:`tokudb_cleaner_period`
     -
     -
     -
     -
   * - :variable:`tokudb_client_pool_threads`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`tokudb_commit_sync`
     -
     -
     -
     -
   * - :variable:`tokudb_compress_buffers_before_eviction`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`tokudb_create_index_online`
     -
     -
     -
     -
   * - :variable:`tokudb_data_dir`
     -
     -
     -
     -
   * - :variable:`tokudb_debug`
     -
     -
     -
     -
   * - :variable:`tokudb_directio`
     -
     -
     -
     -
   * - :variable:`tokudb_disable_hot_alter`
     -
     -
     -
     -
   * - :variable:`tokudb_disable_prefetching`
     -
     -
     -
     -
   * - :variable:`tokudb_disable_slow_alter`
     -
     -
     -
     -
   * - :variable:`tokudb_empty_scan`
     -
     -
     -
     -
   * - :variable:`tokudb_enable_partial_eviction`
     - Yes
     - Yes
     - Global 
     - No
   * - :variable:`tokudb_fanout`
     - Yes
     - Yes
     - Both
     - Yes
   * - :variable:`tokudb_fs_reserve_percent`
     -
     -
     -
     -
   * - :variable:`tokudb_fsync_log_period`
     -
     -
     -
     -
   * - :variable:`tokudb_hide_default_row_format`
     -
     -
     -
     -
   * - :variable:`tokudb_killed_time`
     -
     -
     -
     -
   * - :variable:`tokudb_last_lock_timeout`
     -
     -
     -
     -
   * - :variable:`tokudb_load_save_space`
     -
     -
     -
     -
   * - :variable:`tokudb_loader_memory_size`
     -
     -
     -
     -
   * - :variable:`tokudb_lock_timeout`
     -
     -
     -
     -
   * - :variable:`tokudb_lock_timeout_debug`
     -
     -
     -
     -
   * - :variable:`tokudb_log_dir`
     -
     -
     -
     -
   * - :variable:`tokudb_max_lock_memory`
     -
     -
     -
     -
   * - :variable:`tokudb_optimize_index_fraction`
     -
     -
     -
     -
   * - :variable:`tokudb_optimize_index_name`
     -
     -
     -
     -
   * - :variable:`tokudb_optimize_throttle`
     -
     -
     -
     -
   * - :variable:`tokudb_pk_insert_mode`
     -
     -
     -
     -
   * - :variable:`tokudb_prelock_empty`
     -
     -
     -
     -
   * - :variable:`tokudb_read_block_size`
     -
     -
     -
     -
   * - :variable:`tokudb_read_buf_size`
     -
     -
     -
     -
   * - :variable:`tokudb_read_status_frequency`
     -
     -
     -
     -
   * - :variable:`tokudb_row_format`
     -
     -
     -
     -
   * - :variable:`tokudb_rpl_check_readonly`
     -
     -
     -
     -
   * - :variable:`tokudb_rpl_lookup_rows`
     -
     -
     -
     -
   * - :variable:`tokudb_rpl_lookup_rows_delay`
     -
     -
     -
     -
   * - :variable:`tokudb_rpl_unique_checks`
     -
     -
     -
     -
   * - :variable:`tokudb_rpl_unique_checks_delay`
     -
     -
     -
     -
   * - :variable:`tokudb_strip_frm_data`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`tokudb_support_xa`
     -
     -
     -
     -
   * - :variable:`tokudb_tmp_dir`
     -
     -
     -
     -
   * - :variable:`tokudb_version`
     -
     -
     -
     -
   * - :variable:`tokudb_write_status_frequency`
     -
     -
     -
     -
   * - :variable:`userstat`
     - Yes
     - Yes
     - Global
     - Yes

Status Variables
================

.. tabularcolumns:: |p{13cm}|p{1.5cm}|p{1.5cm}|

.. list-table::
   :header-rows: 1

   * - Name
     - Var Type
     - Var Scope
   * - :variable:`Binlog_snapshot_file`
     - String
     - Global
   * - :variable:`Binlog_snapshot_position`
     - Numeric
     - Global
   * - :variable:`Com_lock_binlog_for_backup`
     - Numeric
     - Both
   * - :variable:`Com_lock_tables_for_backup`
     - Numeric
     - Both
   * - :variable:`Com_show_client_statistics`
     - Numeric
     - Both
   * - :variable:`Com_show_index_statistics`
     - Numeric
     - Both
   * - :variable:`Com_show_table_statistics`
     - Numeric
     - Both
   * - :variable:`Com_show_thread_statistics`  
     - Numeric
     - Both
   * - :variable:`Com_show_user_statistics`
     - Numeric
     - Both
   * - :variable:`Com_unlock_binlog`
     - Numeric
     - Both
   * - :variable:`Innodb_background_log_sync`
     - Numeric
     - Global
   * - :variable:`Innodb_buffer_pool_pages_LRU_flushed`
     - Numeric
     - Global
   * - :variable:`Innodb_buffer_pool_pages_made_not_young`
     - Numeric
     - Global
   * - :variable:`Innodb_buffer_pool_pages_made_young`
     - Numeric
     - Global
   * - :variable:`Innodb_buffer_pool_pages_old`
     - Numeric
     - Global
   * - :variable:`Innodb_checkpoint_age`
     - Numeric
     - Global
   * - :variable:`Innodb_checkpoint_max_age`
     - Numeric
     - Global
   * - :variable:`Innodb_ibuf_free_list`
     - Numeric
     - Global
   * - :variable:`Innodb_ibuf_segment_size`
     - Numeric
     - Global
   * - :variable:`Innodb_lsn_current`
     - Numeric
     - Global
   * - :variable:`Innodb_lsn_flushed`
     - Numeric
     - Global
   * - :variable:`Innodb_lsn_last_checkpoint`
     - Numeric
     - Global
   * - :variable:`Innodb_master_thread_active_loops`
     - Numeric
     - Global
   * - :variable:`Innodb_master_thread_idle_loops`
     - Numeric
     - Global
   * - :variable:`Innodb_max_trx_id`
     - Numeric
     - Global
   * - :variable:`Innodb_mem_adaptive_hash`
     - Numeric
     - Global
   * - :variable:`Innodb_mem_dictionary`
     - Numeric
     - Global
   * - :variable:`Innodb_oldest_view_low_limit_trx_id`
     - Numeric
     - Global
   * - :variable:`Innodb_purge_trx_id`
     - Numeric
     - Global
   * - :variable:`Innodb_purge_undo_no`
     - Numeric
     - Global
   * - :variable:`Threadpool_idle_threads`
     - Numeric
     - Global
   * - :variable:`Threadpool_threads`
     - Numeric
     - Global
