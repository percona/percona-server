.. _ps_variables:

====================================================
 List of variables introduced in Percona Server 5.7
====================================================

System Variables
================

.. tabularcolumns:: |p{7.5cm}|p{1.5cm}|p{1.5cm}|p{2cm}|p{1.5cm}|p{1.5cm}|
.. list-table::
   :header-rows: 1

   * - Name
     - Cmd-Line
     - Option File
     - Var Scope
     - Dynamic
     - Status
   * - :variable:`audit_log_buffer_size`
     - Yes
     - Yes
     - Global
     - No
     -
   * - :variable:`audit_log_file`
     - Yes
     - Yes
     - Global
     - No
     -
   * - :variable:`audit_log_flush`
     - Yes
     - Yes
     - Global
     - Yes
     - 
   * - :variable:`audit_log_format`
     - Yes
     - Yes
     - Global
     - No
     -
   * - :variable:`audit_log_handler`
     - Yes
     - Yes
     - Global
     - No
     -
   * - :variable:`audit_log_policy`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`audit_log_rotate_on_size`
     - Yes
     - Yes
     - Global
     - No
     -
   * - :variable:`audit_log_rotations`
     - Yes
     - Yes
     - Global
     - No
     -
   * - :variable:`audit_log_strategy`
     - Yes
     - Yes
     - Global
     - No
     -
   * - :variable:`audit_log_syslog_facility`
     - Yes
     - Yes
     - Global
     - No
     -
   * - :variable:`audit_log_syslog_ident`
     - Yes
     - Yes
     - Global
     - No
     -
   * - :variable:`audit_log_syslog_priority`
     - Yes
     - Yes
     - Global
     - No
     -
   * - :variable:`binlog_space_limit`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`csv_mode`
     - Yes
     - Yes
     - Both
     - Yes
     -
   * - :variable:`encrypt_binlog`
     - Yes
     - Yes
     - Global
     - No
     -
   * - :variable:`encrypt-tmp-files`
     - Yes
     - Yes
     - Global
     - No
     -
   * - :variable:`enforce_storage_engine`
     - Yes
     - Yes
     - Global
     - No
     -
   * - :variable:`expand_fast_index_creation`
     - Yes
     - No
     - Both
     - Yes
     -
   * - :variable:`extra_max_connections`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`extra_port`
     - Yes
     - Yes
     - Global
     - No
     -
   * - :variable:`ft_query_extra_word_chars`
     - Yes
     - Yes
     - Both
     - Yes
     -
   * - :variable:`have_backup_locks`
     - Yes
     - No
     - Global
     - No
     -
   * - :variable:`have_backup_safe_binlog_info`
     - Yes
     - No
     - Global
     - No
     -
   * - :variable:`have_snapshot_cloning`
     - Yes
     - No
     - Global
     - No
     -
   * - :variable:`innodb_background_scrub_data_compressed`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`innodb_background_scrub_data_uncompressed`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`innodb_cleaner_lsn_age_factor`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`innodb_corrupt_table_action`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`innodb_default_encryption_key_id`
     - Yes
     - Yes
     - Session
     - Yes
     -
   * - :variable:`innodb_empty_free_list_algorithm`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`innodb_encrypt_online_alter_logs`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`innodb_encrypt_tables`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`innodb_kill_idle_transaction`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`innodb_max_bitmap_file_size`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`innodb_max_changed_pages`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`innodb_online_encryption_rotate_key_age`
     - Yes
     - Yes
     - Global
     - Yes
     - Deprecated
   * - :variable:`innodb_online_encryption_threads`
     - Yes
     - Yes
     - Global
     - Yes
     - Deprecated
   * - :variable:`innodb_parallel_dblwr_encrypt`
     - Yes
     - Yes
     - Global
     - Yes
     - Deprecated 5.7.31-34
   * - :variable:`innodb_print_lock_wait_timeout_info`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`innodb_redo_log_encrypt`
     - Yes
     - Yes
     - Global
     - Yes
     - Deprecated: 5.7.31-34
   * - :variable:`innodb_scrub_log`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`innodb_scrub_log_speed`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`innodb_show_locks_held`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`innodb_show_verbose_locks`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`innodb_sys_tablespace_encrypt`
     - Yes
     - Yes
     - Global
     - No
     - Deprecated
   * - :variable:`innodb_temp_tablespace_encrypt`
     - Yes
     - Yes
     - Global
     - No
     - Deprecated
   * - :variable:`innodb_track_changed_pages`
     - Yes
     - Yes
     - Global
     - No
     -
   * - :variable:`innodb_undo_log_encrypt`
     - Yes
     - Yes
     - Global
     - Yes
     - Deprecated
   * - :variable:`innodb_use_global_flush_log_at_trx_commit`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`keyring_vault_config`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`keyring_vault_timeout`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`log_slow_filter`
     - Yes
     - Yes
     - Both
     - Yes
     -
   * - :variable:`log_slow_rate_limit`
     - Yes
     - Yes
     - Both
     - Yes
     -
   * - :variable:`log_slow_rate_type`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`log_slow_sp_statements`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`log_slow_verbosity`
     - Yes
     - Yes
     - Both
     - Yes
     -
   * - :variable:`log_warnings_suppress`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`max_binlog_files`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`max_slowlog_files`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`max_slowlog_size`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`proxy_protocol_networks`
     - Yes
     - Yes
     - Global
     - No
     -
   * - :variable:`pseudo_server_id`
     - Yes
     - No
     - Session
     - Yes
     -
   * - :variable:`query_cache_strip_comments`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`query_response_time_flush`
     - Yes
     - No
     - Global
     - No
     -
   * - :variable:`query_response_time_range_base`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`query_response_time_stats`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`slow_query_log_always_write_time`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`slow_query_log_use_global_control`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`thread_pool_high_prio_mode`
     - Yes
     - Yes
     - Both
     - Yes
     -
   * - :variable:`thread_pool_high_prio_tickets`
     - Yes
     - Yes
     - Both
     - Yes
     -
   * - :variable:`thread_pool_idle_timeout`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`thread_pool_max_threads`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`thread_pool_oversubscribe`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`thread_pool_size`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`thread_pool_stall_limit`
     - Yes
     - Yes
     - Global
     - No
     -
   * - :variable:`thread_statistics`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`tokudb_alter_print_error`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_analyze_delete_fraction`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_analyze_in_background`
     - Yes
     - Yes
     - Both
     - Yes
     -
   * - :variable:`tokudb_analyze_mode`
     - Yes
     - Yes
     - Both
     - Yes
     -
   * - :variable:`tokudb_analyze_throttle`
     - Yes
     - Yes
     - Both
     - Yes
     -
   * - :variable:`tokudb_analyze_time`
     - Yes
     - Yes
     - Both
     - Yes
     -
   * - :variable:`tokudb_auto_analyze`
     - Yes
     - Yes
     - Both
     - Yes
     -
   * - :variable:`tokudb_block_size`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_bulk_fetch`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_cache_size`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_cachetable_pool_threads`
     - Yes
     - Yes
     - Global
     - No
     -
   * - :variable:`tokudb_cardinality_scale_percent`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_check_jemalloc`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_checkpoint_lock`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_checkpoint_on_flush_logs`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_checkpoint_pool_threads`
     - Yes
     - Yes
     - Global
     - No
     -
   * - :variable:`tokudb_checkpointing_period`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_cleaner_iterations`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_cleaner_period`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_client_pool_threads`
     - Yes
     - Yes
     - Global
     - No
     -
   * - :variable:`tokudb_commit_sync`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_compress_buffers_before_eviction`
     - Yes
     - Yes
     - Global
     - No
     -
   * - :variable:`tokudb_create_index_online`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_data_dir`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_debug`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_directio`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_disable_hot_alter`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_disable_prefetching`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_disable_slow_alter`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_empty_scan`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_enable_partial_eviction`
     - Yes
     - Yes
     - Global
     - No
     -
   * - :variable:`tokudb_fanout`
     - Yes
     - Yes
     - Both
     - Yes
     -
   * - :variable:`tokudb_fs_reserve_percent`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_fsync_log_period`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_hide_default_row_format`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_killed_time`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_last_lock_timeout`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_load_save_space`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_loader_memory_size`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_lock_timeout`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_lock_timeout_debug`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_log_dir`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_max_lock_memory`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_optimize_index_fraction`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_optimize_index_name`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_optimize_throttle`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_pk_insert_mode`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_prelock_empty`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_read_block_size`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_read_buf_size`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_read_status_frequency`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_row_format`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_rpl_check_readonly`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_rpl_lookup_rows`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_rpl_lookup_rows_delay`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_rpl_unique_checks`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_rpl_unique_checks_delay`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_strip_frm_data`
     - Yes
     - Yes
     - Global
     - No
     -
   * - :variable:`tokudb_support_xa`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_tmp_dir`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_version`
     -
     -
     -
     -
     -
   * - :variable:`tokudb_write_status_frequency`
     -
     -
     -
     -
     -
   * - :variable:`userstat`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`version_comment`
     - Yes
     - Yes
     - Global
     - Yes
     -
   * - :variable:`version_suffix`
     - Yes
     - Yes
     - Global
     - Yes
     -

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
   * - :variable:`Tokudb_DB_OPENS`
     -
     -
   * - :variable:`Tokudb_DB_CLOSES`
     -
     -
   * - :variable:`Tokudb_DB_OPEN_CURRENT`
     -
     -
   * - :variable:`Tokudb_DB_OPEN_MAX`
     -
     -
   * - :variable:`Tokudb_LEAF_ENTRY_MAX_COMMITTED_XR`
     -
     -
   * - :variable:`Tokudb_LEAF_ENTRY_MAX_PROVISIONAL_XR`
     -
     -
   * - :variable:`Tokudb_LEAF_ENTRY_EXPANDED`
     -
     -
   * - :variable:`Tokudb_LEAF_ENTRY_MAX_MEMSIZE`
     -
     -
   * - :variable:`Tokudb_LEAF_ENTRY_APPLY_GC_BYTES_IN`
     -
     -
   * - :variable:`Tokudb_LEAF_ENTRY_APPLY_GC_BYTES_OUT`
     -
     -
   * - :variable:`Tokudb_LEAF_ENTRY_NORMAL_GC_BYTES_IN`
     -
     -
   * - :variable:`Tokudb_LEAF_ENTRY_NORMAL_GC_BYTES_OUT`
     -
     -
   * - :variable:`Tokudb_CHECKPOINT_PERIOD`
     -
     -
   * - :variable:`Tokudb_CHECKPOINT_FOOTPRINT`
     -
     -
   * - :variable:`Tokudb_CHECKPOINT_LAST_BEGAN`
     -
     -
   * - :variable:`Tokudb_CHECKPOINT_LAST_COMPLETE_BEGAN`
     -
     -
   * - :variable:`Tokudb_CHECKPOINT_LAST_COMPLETE_ENDED`
     -
     -
   * - :variable:`Tokudb_CHECKPOINT_DURATION`
     -
     -
   * - :variable:`Tokudb_CHECKPOINT_DURATION_LAST`
     -
     -
   * - :variable:`Tokudb_CHECKPOINT_LAST_LSN`
     -
     -
   * - :variable:`Tokudb_CHECKPOINT_TAKEN`
     -
     -
   * - :variable:`Tokudb_CHECKPOINT_FAILED`
     -
     -
   * - :variable:`Tokudb_CHECKPOINT_WAITERS_NOW`
     -
     -
   * - :variable:`Tokudb_CHECKPOINT_WAITERS_MAX`
     -
     -
   * - :variable:`Tokudb_CHECKPOINT_CLIENT_WAIT_ON_MO`
     -
     -
   * - :variable:`Tokudb_CHECKPOINT_CLIENT_WAIT_ON_CS`
     -
     -
   * - :variable:`Tokudb_CHECKPOINT_BEGIN_TIME`
     -
     -
   * - :variable:`Tokudb_CHECKPOINT_LONG_BEGIN_TIME`
     -
     -
   * - :variable:`Tokudb_CHECKPOINT_LONG_BEGIN_COUNT`
     -
     -
   * - :variable:`Tokudb_CHECKPOINT_END_TIME`
     -
     -
   * - :variable:`Tokudb_CHECKPOINT_LONG_END_TIME`
     -
     -
   * - :variable:`Tokudb_CHECKPOINT_LONG_END_COUNT`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_MISS`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_MISS_TIME`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_PREFETCHES`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_SIZE_CURRENT`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_SIZE_LIMIT`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_SIZE_WRITING`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_SIZE_NONLEAF`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_SIZE_LEAF`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_SIZE_ROLLBACK`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_SIZE_CACHEPRESSURE`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_SIZE_CLONED`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_EVICTIONS`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_CLEANER_EXECUTIONS`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_CLEANER_PERIOD`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_CLEANER_ITERATIONS`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_WAIT_PRESSURE_COUNT`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_WAIT_PRESSURE_TIME`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_LONG_WAIT_PRESSURE_COUNT`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_LONG_WAIT_PRESSURE_TIME`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_POOL_CLIENT_NUM_THREADS`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_POOL_CLIENT_NUM_THREADS_ACTIVE`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_POOL_CLIENT_QUEUE_SIZE`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_POOL_CLIENT_MAX_QUEUE_SIZE`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_POOL_CLIENT_TOTAL_ITEMS_PROCESSED`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_POOL_CLIENT_TOTAL_EXECUTION_TIME`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_POOL_CACHETABLE_NUM_THREADS`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_POOL_CACHETABLE_NUM_THREADS_ACTIVE`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_POOL_CACHETABLE_QUEUE_SIZE`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_POOL_CACHETABLE_MAX_QUEUE_SIZE`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_POOL_CACHETABLE_TOTAL_ITEMS_PROCESSED`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_POOL_CACHETABLE_TOTAL_EXECUTION_TIME`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_POOL_CHECKPOINT_NUM_THREADS`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_POOL_CHECKPOINT_NUM_THREADS_ACTIVE`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_POOL_CHECKPOINT_QUEUE_SIZE`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_POOL_CHECKPOINT_MAX_QUEUE_SIZE`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_POOL_CHECKPOINT_TOTAL_ITEMS_PROCESSED`
     -
     -
   * - :variable:`Tokudb_CACHETABLE_POOL_CHECKPOINT_TOTAL_EXECUTION_TIME`
     -
     -
   * - :variable:`Tokudb_LOCKTREE_MEMORY_SIZE`
     -
     -
   * - :variable:`Tokudb_LOCKTREE_MEMORY_SIZE_LIMIT`
     -
     -
   * - :variable:`Tokudb_LOCKTREE_ESCALATION_NUM`
     -
     -
   * - :variable:`Tokudb_LOCKTREE_ESCALATION_SECONDS`
     -
     -
   * - :variable:`Tokudb_LOCKTREE_LATEST_POST_ESCALATION_MEMORY_SIZE`
     -
     -
   * - :variable:`Tokudb_LOCKTREE_OPEN_CURRENT`
     -
     -
   * - :variable:`Tokudb_LOCKTREE_PENDING_LOCK_REQUESTS`
     -
     -
   * - :variable:`Tokudb_LOCKTREE_STO_ELIGIBLE_NUM`
     -
     -
   * - :variable:`Tokudb_LOCKTREE_STO_ENDED_NUM`
     -
     -
   * - :variable:`Tokudb_LOCKTREE_STO_ENDED_SECONDS`
     -
     -
   * - :variable:`Tokudb_LOCKTREE_WAIT_COUNT`
     -
     -
   * - :variable:`Tokudb_LOCKTREE_WAIT_TIME`
     -
     -
   * - :variable:`Tokudb_LOCKTREE_LONG_WAIT_COUNT`
     -
     -
   * - :variable:`Tokudb_LOCKTREE_LONG_WAIT_TIME`
     -
     -
   * - :variable:`Tokudb_LOCKTREE_TIMEOUT_COUNT`
     -
     -
   * - :variable:`Tokudb_LOCKTREE_WAIT_ESCALATION_COUNT`
     -
     -
   * - :variable:`Tokudb_LOCKTREE_WAIT_ESCALATION_TIME`
     -
     -
   * - :variable:`Tokudb_LOCKTREE_LONG_WAIT_ESCALATION_COUNT`
     -
     -
   * - :variable:`Tokudb_LOCKTREE_LONG_WAIT_ESCALATION_TIME`
     -
     -
   * - :variable:`Tokudb_DICTIONARY_UPDATES`
     -
     -
   * - :variable:`Tokudb_DICTIONARY_BROADCAST_UPDATES`
     -
     -
   * - :variable:`Tokudb_DESCRIPTOR_SET`
     -
     -
   * - :variable:`Tokudb_MESSAGES_IGNORED_BY_LEAF_DUE_TO_MSN`
     -
     -
   * - :variable:`Tokudb_TOTAL_SEARCH_RETRIES`
     -
     -
   * - :variable:`Tokudb_SEARCH_TRIES_GT_HEIGHT`
     -
     -
   * - :variable:`Tokudb_SEARCH_TRIES_GT_HEIGHTPLUS3`
     -
     -
   * - :variable:`Tokudb_LEAF_NODES_FLUSHED_NOT_CHECKPOINT`
     -
     -
   * - :variable:`Tokudb_LEAF_NODES_FLUSHED_NOT_CHECKPOINT_BYTES`
     -
     -
   * - :variable:`Tokudb_LEAF_NODES_FLUSHED_NOT_CHECKPOINT_UNCOMPRESSED_BYTES`
     -
     -
   * - :variable:`Tokudb_LEAF_NODES_FLUSHED_NOT_CHECKPOINT_SECONDS`
     -
     -
   * - :variable:`Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_NOT_CHECKPOINT`
     -
     -
   * - :variable:`Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_NOT_CHECKPOINT_BYTES`
     -
     -
   * - :variable:`Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_NOT_CHECKPOINT_UNCOMPRESSE`
     -
     -
   * - :variable:`Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_NOT_CHECKPOINT_SECONDS`
     -
     -
   * - :variable:`Tokudb_LEAF_NODES_FLUSHED_CHECKPOINT`
     -
     -
   * - :variable:`Tokudb_LEAF_NODES_FLUSHED_CHECKPOINT_BYTES`
     -
     -
   * - :variable:`Tokudb_LEAF_NODES_FLUSHED_CHECKPOINT_UNCOMPRESSED_BYTES`
     -
     -
   * - :variable:`Tokudb_LEAF_NODES_FLUSHED_CHECKPOINT_SECONDS`
     -
     -
   * - :variable:`Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_CHECKPOINT`
     -
     -
   * - :variable:`Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_CHECKPOINT_BYTES`
     -
     -
   * - :variable:`Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_CHECKPOINT_UNCOMPRESSED_BY`
     -
     -
   * - :variable:`Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_CHECKPOINT_SECONDS`
     -
     -
   * - :variable:`Tokudb_LEAF_NODE_COMPRESSION_RATIO`
     -
     -
   * - :variable:`Tokudb_NONLEAF_NODE_COMPRESSION_RATIO`
     -
     -
   * - :variable:`Tokudb_OVERALL_NODE_COMPRESSION_RATIO`
     -
     -
   * - :variable:`Tokudb_NONLEAF_NODE_PARTIAL_EVICTIONS`
     -
     -
   * - :variable:`Tokudb_NONLEAF_NODE_PARTIAL_EVICTIONS_BYTES`
     -
     -
   * - :variable:`Tokudb_LEAF_NODE_PARTIAL_EVICTIONS`
     -
     -
   * - :variable:`Tokudb_LEAF_NODE_PARTIAL_EVICTIONS_BYTES`
     -
     -
   * - :variable:`Tokudb_LEAF_NODE_FULL_EVICTIONS`
     -
     -
   * - :variable:`Tokudb_LEAF_NODE_FULL_EVICTIONS_BYTES`
     -
     -
   * - :variable:`Tokudb_NONLEAF_NODE_FULL_EVICTIONS`
     -
     -
   * - :variable:`Tokudb_NONLEAF_NODE_FULL_EVICTIONS_BYTES`
     -
     -
   * - :variable:`Tokudb_LEAF_NODES_CREATED`
     -
     -
   * - :variable:`Tokudb_NONLEAF_NODES_CREATED`
     -
     -
   * - :variable:`Tokudb_LEAF_NODES_DESTROYED`
     -
     -
   * - :variable:`Tokudb_NONLEAF_NODES_DESTROYED`
     -
     -
   * - :variable:`Tokudb_MESSAGES_INJECTED_AT_ROOT_BYTES`
     -
     -
   * - :variable:`Tokudb_MESSAGES_FLUSHED_FROM_H1_TO_LEAVES_BYTES`
     -
     -
   * - :variable:`Tokudb_MESSAGES_IN_TREES_ESTIMATE_BYTES`
     -
     -
   * - :variable:`Tokudb_MESSAGES_INJECTED_AT_ROOT`
     -
     -
   * - :variable:`Tokudb_BROADCASE_MESSAGES_INJECTED_AT_ROOT`
     -
     -
   * - :variable:`Tokudb_BASEMENTS_DECOMPRESSED_TARGET_QUERY`
     -
     -
   * - :variable:`Tokudb_BASEMENTS_DECOMPRESSED_PRELOCKED_RANGE`
     -
     -
   * - :variable:`Tokudb_BASEMENTS_DECOMPRESSED_PREFETCH`
     -
     -
   * - :variable:`Tokudb_BASEMENTS_DECOMPRESSED_FOR_WRITE`
     -
     -
   * - :variable:`Tokudb_BUFFERS_DECOMPRESSED_TARGET_QUERY`
     -
     -
   * - :variable:`Tokudb_BUFFERS_DECOMPRESSED_PRELOCKED_RANGE`
     -
     -
   * - :variable:`Tokudb_BUFFERS_DECOMPRESSED_PREFETCH`
     -
     -
   * - :variable:`Tokudb_BUFFERS_DECOMPRESSED_FOR_WRITE`
     -
     -
   * - :variable:`Tokudb_PIVOTS_FETCHED_FOR_QUERY`
     -
     -
   * - :variable:`Tokudb_PIVOTS_FETCHED_FOR_QUERY_BYTES`
     -
     -
   * - :variable:`Tokudb_PIVOTS_FETCHED_FOR_QUERY_SECONDS`
     -
     -
   * - :variable:`Tokudb_PIVOTS_FETCHED_FOR_PREFETCH`
     -
     -
   * - :variable:`Tokudb_PIVOTS_FETCHED_FOR_PREFETCH_BYTES`
     -
     -
   * - :variable:`Tokudb_PIVOTS_FETCHED_FOR_PREFETCH_SECONDS`
     -
     -
   * - :variable:`Tokudb_PIVOTS_FETCHED_FOR_WRITE`
     -
     -
   * - :variable:`Tokudb_PIVOTS_FETCHED_FOR_WRITE_BYTES`
     -
     -
   * - :variable:`Tokudb_PIVOTS_FETCHED_FOR_WRITE_SECONDS`
     -
     -
   * - :variable:`Tokudb_BASEMENTS_FETCHED_TARGET_QUERY`
     -
     -
   * - :variable:`Tokudb_BASEMENTS_FETCHED_TARGET_QUERY_BYTES`
     -
     -
   * - :variable:`Tokudb_BASEMENTS_FETCHED_TARGET_QUERY_SECONDS`
     -
     -
   * - :variable:`Tokudb_BASEMENTS_FETCHED_PRELOCKED_RANGE`
     -
     -
   * - :variable:`Tokudb_BASEMENTS_FETCHED_PRELOCKED_RANGE_BYTES`
     -
     -
   * - :variable:`Tokudb_BASEMENTS_FETCHED_PRELOCKED_RANGE_SECONDS`
     -
     -
   * - :variable:`Tokudb_BASEMENTS_FETCHED_PREFETCH`
     -
     -
   * - :variable:`Tokudb_BASEMENTS_FETCHED_PREFETCH_BYTES`
     -
     -
   * - :variable:`Tokudb_BASEMENTS_FETCHED_PREFETCH_SECONDS`
     -
     -
   * - :variable:`Tokudb_BASEMENTS_FETCHED_FOR_WRITE`
     -
     -
   * - :variable:`Tokudb_BASEMENTS_FETCHED_FOR_WRITE_BYTES`
     -
     -
   * - :variable:`Tokudb_BASEMENTS_FETCHED_FOR_WRITE_SECONDS`
     -
     -
   * - :variable:`Tokudb_BUFFERS_FETCHED_TARGET_QUERY`
     -
     -
   * - :variable:`Tokudb_BUFFERS_FETCHED_TARGET_QUERY_BYTES`
     -
     -
   * - :variable:`Tokudb_BUFFERS_FETCHED_TARGET_QUERY_SECONDS`
     -
     -
   * - :variable:`Tokudb_BUFFERS_FETCHED_PRELOCKED_RANGE`
     -
     -
   * - :variable:`Tokudb_BUFFERS_FETCHED_PRELOCKED_RANGE_BYTES`
     -
     -
   * - :variable:`Tokudb_BUFFERS_FETCHED_PRELOCKED_RANGE_SECONDS`
     -
     -
   * - :variable:`Tokudb_BUFFERS_FETCHED_PREFETCH`
     -
     -
   * - :variable:`Tokudb_BUFFERS_FETCHED_PREFETCH_BYTES`
     -
     -
   * - :variable:`Tokudb_BUFFERS_FETCHED_PREFETCH_SECONDS`
     -
     -
   * - :variable:`Tokudb_BUFFERS_FETCHED_FOR_WRITE`
     -
     -
   * - :variable:`Tokudb_BUFFERS_FETCHED_FOR_WRITE_BYTES`
     -
     -
   * - :variable:`Tokudb_BUFFERS_FETCHED_FOR_WRITE_SECONDS`
     -
     -
   * - :variable:`Tokudb_LEAF_COMPRESSION_TO_MEMORY_SECONDS`
     -
     -
   * - :variable:`Tokudb_LEAF_SERIALIZATION_TO_MEMORY_SECONDS`
     -
     -
   * - :variable:`Tokudb_LEAF_DECOMPRESSION_TO_MEMORY_SECONDS`
     -
     -
   * - :variable:`Tokudb_LEAF_DESERIALIZATION_TO_MEMORY_SECONDS`
     -
     -
   * - :variable:`Tokudb_NONLEAF_COMPRESSION_TO_MEMORY_SECONDS`
     -
     -
   * - :variable:`Tokudb_NONLEAF_SERIALIZATION_TO_MEMORY_SECONDS`
     -
     -
   * - :variable:`Tokudb_NONLEAF_DECOMPRESSION_TO_MEMORY_SECONDS`
     -
     -
   * - :variable:`Tokudb_NONLEAF_DESERIALIZATION_TO_MEMORY_SECONDS`
     -
     -
   * - :variable:`Tokudb_PROMOTION_ROOTS_SPLIT`
     -
     -
   * - :variable:`Tokudb_PROMOTION_LEAF_ROOTS_INJECTED_INTO`
     -
     -
   * - :variable:`Tokudb_PROMOTION_H1_ROOTS_INJECTED_INTO`
     -
     -
   * - :variable:`Tokudb_PROMOTION_INJECTIONS_AT_DEPTH_0`
     -
     -
   * - :variable:`Tokudb_PROMOTION_INJECTIONS_AT_DEPTH_1`
     -
     -
   * - :variable:`Tokudb_PROMOTION_INJECTIONS_AT_DEPTH_2`
     -
     -
   * - :variable:`Tokudb_PROMOTION_INJECTIONS_AT_DEPTH_3`
     -
     -
   * - :variable:`Tokudb_PROMOTION_INJECTIONS_LOWER_THAN_DEPTH_3`
     -
     -
   * - :variable:`Tokudb_PROMOTION_STOPPED_NONEMPTY_BUFFER`
     -
     -
   * - :variable:`Tokudb_PROMOTION_STOPPED_AT_HEIGHT_1`
     -
     -
   * - :variable:`Tokudb_PROMOTION_STOPPED_CHILD_LOCKED_OR_NOT_IN_MEMORY`
     -
     -
   * - :variable:`Tokudb_PROMOTION_STOPPED_CHILD_NOT_FULLY_IN_MEMORY`
     -
     -
   * - :variable:`Tokudb_PROMOTION_STOPPED_AFTER_LOCKING_CHILD`
     -
     -
   * - :variable:`Tokudb_BASEMENT_DESERIALIZATION_FIXED_KEY`
     -
     -
   * - :variable:`Tokudb_BASEMENT_DESERIALIZATION_VARIABLE_KEY`
     -
     -
   * - :variable:`Tokudb_PRO_RIGHTMOST_LEAF_SHORTCUT_SUCCESS`
     -
     -
   * - :variable:`Tokudb_PRO_RIGHTMOST_LEAF_SHORTCUT_FAIL_POS`
     -
     -
   * - :variable:`Tokudb_RIGHTMOST_LEAF_SHORTCUT_FAIL_REACTIVE`
     -
     -
   * - :variable:`Tokudb_CURSOR_SKIP_DELETED_LEAF_ENTRY`
     -
     -
   * - :variable:`Tokudb_FLUSHER_CLEANER_TOTAL_NODES`
     -
     -
   * - :variable:`Tokudb_FLUSHER_CLEANER_H1_NODES`
     -
     -
   * - :variable:`Tokudb_FLUSHER_CLEANER_HGT1_NODES`
     -
     -
   * - :variable:`Tokudb_FLUSHER_CLEANER_EMPTY_NODES`
     -
     -
   * - :variable:`Tokudb_FLUSHER_CLEANER_NODES_DIRTIED`
     -
     -
   * - :variable:`Tokudb_FLUSHER_CLEANER_MAX_BUFFER_SIZE`
     -
     -
   * - :variable:`Tokudb_FLUSHER_CLEANER_MIN_BUFFER_SIZE`
     -
     -
   * - :variable:`Tokudb_FLUSHER_CLEANER_TOTAL_BUFFER_SIZE`
     -
     -
   * - :variable:`Tokudb_FLUSHER_CLEANER_MAX_BUFFER_WORKDONE`
     -
     -
   * - :variable:`Tokudb_FLUSHER_CLEANER_MIN_BUFFER_WORKDONE`
     -
     -
   * - :variable:`Tokudb_FLUSHER_CLEANER_TOTAL_BUFFER_WORKDONE`
     -
     -
   * - :variable:`Tokudb_FLUSHER_CLEANER_NUM_LEAF_MERGES_STARTED`
     -
     -
   * - :variable:`Tokudb_FLUSHER_CLEANER_NUM_LEAF_MERGES_RUNNING`
     -
     -
   * - :variable:`Tokudb_FLUSHER_CLEANER_NUM_LEAF_MERGES_COMPLETED`
     -
     -
   * - :variable:`Tokudb_FLUSHER_CLEANER_NUM_DIRTIED_FOR_LEAF_MERGE`
     -
     -
   * - :variable:`Tokudb_FLUSHER_FLUSH_TOTAL`
     -
     -
   * - :variable:`Tokudb_FLUSHER_FLUSH_IN_MEMORY`
     -
     -
   * - :variable:`Tokudb_FLUSHER_FLUSH_NEEDED_IO`
     -
     -
   * - :variable:`Tokudb_FLUSHER_FLUSH_CASCADES`
     -
     -
   * - :variable:`Tokudb_FLUSHER_FLUSH_CASCADES_1`
     -
     -
   * - :variable:`Tokudb_FLUSHER_FLUSH_CASCADES_2`
     -
     -
   * - :variable:`Tokudb_FLUSHER_FLUSH_CASCADES_3`
     -
     -
   * - :variable:`Tokudb_FLUSHER_FLUSH_CASCADES_4`
     -
     -
   * - :variable:`Tokudb_FLUSHER_FLUSH_CASCADES_5`
     -
     -
   * - :variable:`Tokudb_FLUSHER_FLUSH_CASCADES_GT_5`
     -
     -
   * - :variable:`Tokudb_FLUSHER_SPLIT_LEAF`
     -
     -
   * - :variable:`Tokudb_FLUSHER_SPLIT_NONLEAF`
     -
     -
   * - :variable:`Tokudb_FLUSHER_MERGE_LEAF`
     -
     -
   * - :variable:`Tokudb_FLUSHER_MERGE_NONLEAF`
     -
     -
   * - :variable:`Tokudb_FLUSHER_BALANCE_LEAF`
     -
     -
   * - :variable:`Tokudb_HOT_NUM_STARTED`
     -
     -
   * - :variable:`Tokudb_HOT_NUM_COMPLETED`
     -
     -
   * - :variable:`Tokudb_HOT_NUM_ABORTED`
     -
     -
   * - :variable:`Tokudb_HOT_MAX_ROOT_FLUSH_COUNT`
     -
     -
   * - :variable:`Tokudb_TXN_BEGIN`
     -
     -
   * - :variable:`Tokudb_TXN_BEGIN_READ_ONLY`
     -
     -
   * - :variable:`Tokudb_TXN_COMMITS`
     -
     -
   * - :variable:`Tokudb_TXN_ABORTS`
     -
     -
   * - :variable:`Tokudb_LOGGER_NEXT_LSN`
     -
     -
   * - :variable:`Tokudb_LOGGER_WRITES`
     -
     -
   * - :variable:`Tokudb_LOGGER_WRITES_BYTES`
     -
     -
   * - :variable:`Tokudb_LOGGER_WRITES_UNCOMPRESSED_BYTES`
     -
     -
   * - :variable:`Tokudb_LOGGER_WRITES_SECONDS`
     -
     -
   * - :variable:`Tokudb_LOGGER_WAIT_LONG`
     -
     -
   * - :variable:`Tokudb_LOADER_NUM_CREATED`
     -
     -
   * - :variable:`Tokudb_LOADER_NUM_CURRENT`
     -
     -
   * - :variable:`Tokudb_LOADER_NUM_MAX`
     -
     -
   * - :variable:`Tokudb_MEMORY_MALLOC_COUNT`
     -
     -
   * - :variable:`Tokudb_MEMORY_FREE_COUNT`
     -
     -
   * - :variable:`Tokudb_MEMORY_REALLOC_COUNT`
     -
     -
   * - :variable:`Tokudb_MEMORY_MALLOC_FAIL`
     -
     -
   * - :variable:`Tokudb_MEMORY_REALLOC_FAIL`
     -
     -
   * - :variable:`Tokudb_MEMORY_REQUESTED`
     -
     -
   * - :variable:`Tokudb_MEMORY_USED`
     -
     -
   * - :variable:`Tokudb_MEMORY_FREED`
     -
     -
   * - :variable:`Tokudb_MEMORY_MAX_REQUESTED_SIZE`
     -
     -
   * - :variable:`Tokudb_MEMORY_LAST_FAILED_SIZE`
     -
     -
   * - :variable:`Tokudb_MEM_ESTIMATED_MAXIMUM_MEMORY_FOOTPRINT`
     -
     -
   * - :variable:`Tokudb_MEMORY_MALLOCATOR_VERSION`
     -
     -
   * - :variable:`Tokudb_MEMORY_MMAP_THRESHOLD`
     -
     -
   * - :variable:`Tokudb_FILESYSTEM_THREADS_BLOCKED_BY_FULL_DISK`
     -
     -
   * - :variable:`Tokudb_FILESYSTEM_FSYNC_TIME`
     -
     -
   * - :variable:`Tokudb_FILESYSTEM_FSYNC_NUM`
     -
     -
   * - :variable:`Tokudb_FILESYSTEM_LONG_FSYNC_TIME`
     -
     -
   * - :variable:`Tokudb_FILESYSTEM_LONG_FSYNC_NUM`
     -
     -
