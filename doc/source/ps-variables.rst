.. _ps_variables:

================================================================================
 List of variables introduced in *Percona Server for MySQL* 8.0
================================================================================

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
   * - :ref:`audit_log_buffer_size`
     - Yes
     - Yes
     - Global
     - No
   * - :ref:`audit_log_file`
     - Yes
     - Yes
     - Global
     - No
   * - :ref:`audit_log_flush`
     - Yes
     - Yes
     - Global
     - Yes
   * - :ref:`audit_log_format`
     - Yes
     - Yes
     - Global
     - No
   * - :ref:`audit_log_handler`
     - Yes
     - Yes
     - Global
     - No
   * - :ref:`audit_log_policy`
     - Yes
     - Yes
     - Global
     - Yes
   * - :ref:`audit_log_rotate_on_size`
     - Yes
     - Yes
     - Global
     - No
   * - :ref:`audit_log_rotations`
     - Yes
     - Yes
     - Global
     - No
   * - :ref:`audit_log_strategy`
     - Yes
     - Yes
     - Global
     - No
   * - :ref:`audit_log_syslog_facility`
     - Yes
     - Yes
     - Global
     - No
   * - :ref:`audit_log_syslog_ident`
     - Yes
     - Yes
     - Global
     - No
   * - :ref:`audit_log_syslog_priority`
     - Yes
     - Yes
     - Global
     - No
   * - :ref:`csv_mode`
     - Yes
     - Yes
     - Both
     - Yes
   * - :ref:`enforce_storage_engine`
     - Yes
     - Yes
     - Global
     - No
   * - :ref:`expand_fast_index_creation`
     - Yes
     - No
     - Both
     - Yes
   * - :ref:`extra_max_connections`
     - Yes
     - Yes
     - Global
     - Yes
   * - :ref:`extra_port`
     - Yes
     - Yes
     - Global
     - No
   * - :ref:`have_backup_locks`
     - Yes
     - No
     - Global
     - No
   * - :ref:`have_backup_safe_binlog_info`
     - Yes
     - No
     - Global
     - No
   * - :ref:`have_snapshot_cloning`
     - Yes
     - No
     - Global
     - No
   * - :ref:`innodb_cleaner_lsn_age_factor`
     - Yes
     - Yes
     - Global
     - Yes
   * - :ref:`innodb_corrupt_table_action`
     - Yes
     - Yes
     - Global
     - Yes
   * - :ref:`innodb_empty_free_list_algorithm`
     - Yes
     - Yes
     - Global
     - Yes
   * - :ref:`innodb_encrypt_online_alter_logs`
     - Yes
     - Yes
     - Global
     - Yes
   * - :ref:`innodb_encrypt_tables`
     - Yes
     - Yes
     - Global
     - Yes
   * - :ref:`innodb_kill_idle_transaction`
     - Yes
     - Yes
     - Global
     - Yes
   * - :ref:`innodb_max_bitmap_file_size`
     - Yes
     - Yes
     - Global
     - Yes
   * - :ref:`innodb_max_changed_pages`
     - Yes
     - Yes
     - Global
     - Yes
   * - :ref:`innodb_print_lock_wait_timeout_info`
     - Yes
     - Yes
     - Global
     - Yes
   * - :ref:`innodb_show_locks_held`
     - Yes
     - Yes
     - Global
     - Yes
   * - :ref:`innodb_temp_tablespace_encrypt`
     - Yes
     - Yes
     - Global
     - No
   * - :ref:`innodb_track_changed_pages`
     - Yes
     - Yes
     - Global
     - No
   * - :ref:`keyring_vault_config`
     - Yes
     - Yes
     - Global
     - Yes
   * - :ref:`keyring_vault_timeout`
     - Yes
     - Yes
     - Global
     - Yes
   * - :ref:`log_slow_filter`
     - Yes
     - Yes
     - Both
     - Yes
   * - :ref:`log_slow_rate_limit`
     - Yes
     - Yes
     - Both
     - Yes
   * - :ref:`log_slow_rate_type`
     - Yes
     - Yes
     - Global
     - Yes
   * - :ref:`log_slow_sp_statements`
     - Yes
     - Yes
     - Global
     - Yes
   * - :ref:`log_slow_verbosity`
     - Yes
     - Yes
     - Both
     - Yes
   * - :ref:`log_warnings_suppress`
     - Yes
     - Yes
     - Global
     - Yes
   * - :ref:`proxy_protocol_networks`
     - Yes
     - Yes
     - Global
     - No
   * - :ref:`query_response_time_flush`
     - Yes
     - No
     - Global
     - No
   * - :ref:`query_response_time_range_base`
     - Yes
     - Yes
     - Global
     - Yes
   * - :ref:`query_response_time_stats`
     - Yes
     - Yes
     - Global
     - Yes
   * - :ref:`slow_query_log_always_write_time`
     - Yes
     - Yes
     - Global
     - Yes
   * - :ref:`slow_query_log_use_global_control`
     - Yes
     - Yes
     - Global
     - Yes
   * - :ref:`thread_pool_high_prio_mode`
     - Yes
     - Yes
     - Both
     - Yes
   * - :ref:`thread_pool_high_prio_tickets`
     - Yes
     - Yes
     - Both
     - Yes
   * - :ref:`thread_pool_idle_timeout`
     - Yes
     - Yes
     - Global
     - Yes
   * - :ref:`thread_pool_max_threads`
     - Yes
     - Yes
     - Global
     - Yes
   * - :ref:`thread_pool_oversubscribe`
     - Yes
     - Yes
     - Global
     - Yes
   * - :ref:`thread_pool_size`
     - Yes
     - Yes
     - Global
     - Yes
   * - :ref:`thread_pool_stall_limit`
     - Yes
     - Yes
     - Global
     - No
   * - :ref:`thread_statistics`
     - Yes
     - Yes
     - Global
     - Yes
   * - :ref:`tokudb_alter_print_error`
     -
     -
     -
     -
   * - :ref:`tokudb_analyze_delete_fractionref
     -
     -
     -
     -
   * - :ref:`tokudb_analyze_in_background`
     - Yes
     - Yes
     - Both
     - Yes
   * - :ref:`tokudb_analyze_mode`
     - Yes
     - Yes
     - Both
     - Yes
   * - :ref:`tokudb_analyze_throttle`
     - Yes
     - Yes
     - Both
     - Yes
   * - :ref:`tokudb_analyze_time`
     - Yes
     - Yes
     - Both
     - Yes
   * - :ref:`tokudb_auto_analyze`
     - Yes
     - Yes
     - Both
     - Yes
   * - :ref:`tokudb_block_size`
     -
     -
     -
     -
   * - :ref:`tokudb_bulk_fetch`
     -
     -
     -
     -
   * - :ref:`tokudb_cache_size`
     -
     -
     -
     -
   * - :ref:`tokudb_cachetable_pool_threads`
     - Yes
     - Yes
     - Global
     - No
   * - :ref:`tokudb_cardinality_scale_percent`
     -
     -
     -
     -
   * - :ref:`tokudb_check_jemalloc`
     -
     -
     -
     -
   * - :ref:`tokudb_checkpoint_lock`
     -
     -
     -
     -
   * - :ref:`tokudb_checkpoint_on_flush_logs`
     -
     -
     -
     -
   * - :ref:`tokudb_checkpoint_pool_threads`
     - Yes
     - Yes
     - Global
     - No
   * - :ref:`tokudb_checkpointing_period`
     -
     -
     -
     -
   * - :ref:`tokudb_cleaner_iterations`
     -
     -
     -
     -
   * - :ref:`tokudb_cleaner_period`
     -
     -
     -
     -
   * - :ref:`tokudb_client_pool_threads`
     - Yes
     - Yes
     - Global
     - No
   * - :ref:`tokudb_commit_sync`
     -
     -
     -
     -
   * - :ref:`tokudb_compress_buffers_before_eviction`
     - Yes
     - Yes
     - Global
     - No
   * - :ref:`tokudb_create_index_online`
     -
     -
     -
     -
   * - :ref:`tokudb_data_dir`
     -
     -
     -
     -
   * - :ref:`tokudb_debug`
     -
     -
     -
     -
   * - :ref:`tokudb_directio`
     -
     -
     -
     -
   * - :ref:`tokudb_disable_hot_alter`
     -
     -
     -
     -
   * - :ref:`tokudb_disable_prefetching`
     -
     -
     -
     -
   * - :ref:`tokudb_disable_slow_alter`
     -
     -
     -
     -
   * - :ref:`tokudb_empty_scan`
     -
     -
     -
     -
   * - :ref:`tokudb_enable_partial_eviction`
     - Yes
     - Yes
     - Global
     - No
   * - :ref:`tokudb_fanout`
     - Yes
     - Yes
     - Both
     - Yes
   * - :ref:`tokudb_fs_reserve_percent`
     -
     -
     -
     -
   * - :ref:`tokudb_fsync_log_period`
     -
     -
     -
     -
   * - :ref:`tokudb_hide_default_row_format`
     -
     -
     -
     -
   * - :ref:`tokudb_killed_time`
     -
     -
     -
     -
   * - :ref:`tokudb_last_lock_timeout`
     -
     -
     -
     -
   * - :ref:`tokudb_load_save_space`
     -
     -
     -
     -
   * - :ref:`tokudb_loader_memory_size`
     -
     -
     -
     -
   * - :ref:`tokudb_lock_timeout`
     -
     -
     -
     -
   * - :ref:`tokudb_lock_timeout_debug`
     -
     -
     -
     -
   * - :ref:`tokudb_log_dir`
     -
     -
     -
     -
   * - :ref:`tokudb_max_lock_memory`
     -
     -
     -
     -
   * - :ref:`tokudb_optimize_index_fraction`
     -
     -
     -
     -
   * - :ref:`tokudb_optimize_index_name`
     -
     -
     -
     -
   * - :ref:`tokudb_optimize_throttle`
     -
     -
     -
     -
   * - :ref:`tokudb_pk_insert_mode`
     -
     -
     -
     -
   * - :ref:`tokudb_prelock_empty`
     -
     -
     -
     -
   * - :ref:`tokudb_read_block_size`
     -
     -
     -
     -
   * - :ref:`tokudb_read_buf_size`
     -
     -
     -
     -
   * - :ref:`tokudb_read_status_frequency`
     -
     -
     -
     -
   * - :ref:`tokudb_row_format`
     -
     -
     -
     -
   * - :ref:`tokudb_rpl_check_readonly`
     -
     -
     -
     -
   * - :ref:`tokudb_rpl_lookup_rows`
     -
     -
     -
     -
   * - :ref:`tokudb_rpl_lookup_rows_delay`
     -
     -
     -
     -
   * - :ref:`tokudb_rpl_unique_checks`
     -
     -
     -
     -
   * - :ref:`tokudb_rpl_unique_checks_delay`
     -
     -
     -
     -
   * - :ref:`tokudb_strip_frm_data`
     - Yes
     - Yes
     - Global
     - No
   * - :ref:`tokudb_support_xa`
     -
     -
     -
     -
   * - :ref:`tokudb_tmp_dir`
     -
     -
     -
     -
   * - :ref:`tokudb_version`
     -
     -
     -
     -
   * - :ref:`tokudb_write_status_frequency`
     -
     -
     -
     -
   * - :ref:`userstat`
     - Yes
     - Yes
     - Global
     - Yes
   * - :ref:`version_comment`
     - Yes
     - Yes
     - Global
     - Yes
   * - :ref:`version_suffix`
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
   * - :ref:`Binlog_snapshot_file`
     - String
     - Global
   * - :ref:`Binlog_snapshot_position`
     - Numeric
     - Global
   * - :ref:`Com_lock_binlog_for_backup`
     - Numeric
     - Both
   * - :ref:`Com_lock_tables_for_backup`
     - Numeric
     - Both
   * - :ref:`Com_show_client_statistics`
     - Numeric
     - Both
   * - :ref:`Com_show_index_statistics`
     - Numeric
     - Both
   * - :ref:`Com_show_table_statistics`
     - Numeric
     - Both
   * - :ref:`Com_show_thread_statistics`
     - Numeric
     - Both
   * - :ref:`Com_show_user_statistics`
     - Numeric
     - Both
   * - :ref:`Com_unlock_binlog`
     - Numeric
     - Both
   * - :ref:`Innodb_background_log_sync`
     - Numeric
     - Global
   * - :ref:`Innodb_buffer_pool_pages_LRU_flushed`
     - Numeric
     - Global
   * - :ref:`Innodb_buffer_pool_pages_made_not_young`
     - Numeric
     - Global
   * - :ref:`Innodb_buffer_pool_pages_made_young`
     - Numeric
     - Global
   * - :ref:`Innodb_buffer_pool_pages_old`
     - Numeric
     - Global
   * - :ref:`Innodb_checkpoint_age`
     - Numeric
     - Global
   * - :ref:`Innodb_checkpoint_max_age`
     - Numeric
     - Global
   * - :ref:`Innodb_ibuf_free_list`
     - Numeric
     - Global
   * - :ref:`Innodb_ibuf_segment_size`
     - Numeric
     - Global
   * - :ref:`Innodb_lsn_current`
     - Numeric
     - Global
   * - :ref:`Innodb_lsn_flushed`
     - Numeric
     - Global
   * - :ref:`Innodb_lsn_last_checkpoint`
     - Numeric
     - Global
   * - :ref:`Innodb_master_thread_active_loops`
     - Numeric
     - Global
   * - :ref:`Innodb_master_thread_idle_loops`
     - Numeric
     - Global
   * - :ref:`Innodb_max_trx_id`
     - Numeric
     - Global
   * - :ref:`Innodb_mem_adaptive_hash`
     - Numeric
     - Global
   * - :ref:`Innodb_mem_dictionary`
     - Numeric
     - Global
   * - :ref:`Innodb_oldest_view_low_limit_trx_id`
     - Numeric
     - Global
   * - :ref:`Innodb_purge_trx_id`
     - Numeric
     - Global
   * - :ref:`Innodb_purge_undo_no`
     - Numeric
     - Global
   * - :ref:`Threadpool_idle_threads`
     - Numeric
     - Global
   * - :ref:`Threadpool_threads`
     - Numeric
     - Global
   * - :ref:`Tokudb_DB_OPENS`
     -
     -
   * - :ref:`Tokudb_DB_CLOSES`
     -
     -
   * - :ref:`Tokudb_DB_OPEN_CURRENT`
     -
     -
   * - :ref:`Tokudb_DB_OPEN_MAX`
     -
     -
   * - :ref:`Tokudb_LEAF_ENTRY_MAX_COMMITTED_XR`
     -
     -
   * - :ref:`Tokudb_LEAF_ENTRY_MAX_PROVISIONAL_XR`
     -
     -
   * - :ref:`Tokudb_LEAF_ENTRY_EXPANDED`
     -
     -
   * - :ref:`Tokudb_LEAF_ENTRY_MAX_MEMSIZE`
     -
     -
   * - :ref:`Tokudb_LEAF_ENTRY_APPLY_GC_BYTES_IN`
     -
     -
   * - :ref:`Tokudb_LEAF_ENTRY_APPLY_GC_BYTES_OUT`
     -
     -
   * - :ref:`Tokudb_LEAF_ENTRY_NORMAL_GC_BYTES_IN`
     -
     -
   * - :ref:`Tokudb_LEAF_ENTRY_NORMAL_GC_BYTES_OUT`
     -
     -
   * - :ref:`Tokudb_CHECKPOINT_PERIOD`
     -
     -
   * - :ref:`Tokudb_CHECKPOINT_FOOTPRINT`
     -
     -
   * - :ref:`Tokudb_CHECKPOINT_LAST_BEGAN`
     -
     -
   * - :ref:`Tokudb_CHECKPOINT_LAST_COMPLETE_BEGAN`
     -
     -
   * - :ref:`Tokudb_CHECKPOINT_LAST_COMPLETE_ENDED`
     -
     -
   * - :ref:`Tokudb_CHECKPOINT_DURATION`
     -
     -
   * - :ref:`Tokudb_CHECKPOINT_DURATION_LAST`
     -
     -
   * - :ref:`Tokudb_CHECKPOINT_LAST_LSN`
     -
     -
   * - :ref:`Tokudb_CHECKPOINT_TAKEN`
     -
     -
   * - :ref:`Tokudb_CHECKPOINT_FAILED`
     -
     -
   * - :ref:`Tokudb_CHECKPOINT_WAITERS_NOW`
     -
     -
   * - :ref:`Tokudb_CHECKPOINT_WAITERS_MAX`
     -
     -
   * - :ref:`Tokudb_CHECKPOINT_CLIENT_WAIT_ON_MO`
     -
     -
   * - :ref:`Tokudb_CHECKPOINT_CLIENT_WAIT_ON_CS`
     -
     -
   * - :ref:`Tokudb_CHECKPOINT_BEGIN_TIME`
     -
     -
   * - :ref:`Tokudb_CHECKPOINT_LONG_BEGIN_TIME`
     -
     -
   * - :ref:`Tokudb_CHECKPOINT_LONG_BEGIN_COUNT`
     -
     -
   * - :ref:`Tokudb_CHECKPOINT_END_TIME`
     -
     -
   * - :ref:`Tokudb_CHECKPOINT_LONG_END_TIME`
     -
     -
   * - :ref:`Tokudb_CHECKPOINT_LONG_END_COUNT`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_MISS`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_MISS_TIME`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_PREFETCHES`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_SIZE_CURRENT`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_SIZE_LIMIT`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_SIZE_WRITING`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_SIZE_NONLEAF`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_SIZE_LEAF`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_SIZE_ROLLBACK`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_SIZE_CACHEPRESSURE`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_SIZE_CLONED`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_EVICTIONS`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_CLEANER_EXECUTIONS`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_CLEANER_PERIOD`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_CLEANER_ITERATIONS`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_WAIT_PRESSURE_COUNT`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_WAIT_PRESSURE_TIME`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_LONG_WAIT_PRESSURE_COUNT`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_LONG_WAIT_PRESSURE_TIME`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_POOL_CLIENT_NUM_THREADS`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_POOL_CLIENT_NUM_THREADS_ACTIVE`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_POOL_CLIENT_QUEUE_SIZE`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_POOL_CLIENT_MAX_QUEUE_SIZE`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_POOL_CLIENT_TOTAL_ITEMS_PROCESSED`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_POOL_CLIENT_TOTAL_EXECUTION_TIME`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_POOL_CACHETABLE_NUM_THREADS`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_POOL_CACHETABLE_NUM_THREADS_ACTIVE`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_POOL_CACHETABLE_QUEUE_SIZE`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_POOL_CACHETABLE_MAX_QUEUE_SIZE`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_POOL_CACHETABLE_TOTAL_ITEMS_PROCESSED`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_POOL_CACHETABLE_TOTAL_EXECUTION_TIME`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_POOL_CHECKPOINT_NUM_THREADS`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_POOL_CHECKPOINT_NUM_THREADS_ACTIVE`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_POOL_CHECKPOINT_QUEUE_SIZE`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_POOL_CHECKPOINT_MAX_QUEUE_SIZE`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_POOL_CHECKPOINT_TOTAL_ITEMS_PROCESSED`
     -
     -
   * - :ref:`Tokudb_CACHETABLE_POOL_CHECKPOINT_TOTAL_EXECUTION_TIME`
     -
     -
   * - :ref:`Tokudb_LOCKTREE_MEMORY_SIZE`
     -
     -
   * - :ref:`Tokudb_LOCKTREE_MEMORY_SIZE_LIMIT`
     -
     -
   * - :ref:`Tokudb_LOCKTREE_ESCALATION_NUM`
     -
     -
   * - :ref:`Tokudb_LOCKTREE_ESCALATION_SECONDS`
     -
     -
   * - :ref:`Tokudb_LOCKTREE_LATEST_POST_ESCALATION_MEMORY_SIZE`
     -
     -
   * - :ref:`Tokudb_LOCKTREE_OPEN_CURRENT`
     -
     -
   * - :ref:`Tokudb_LOCKTREE_PENDING_LOCK_REQUESTS`
     -
     -
   * - :ref:`Tokudb_LOCKTREE_STO_ELIGIBLE_NUM`
     -
     -
   * - :ref:`Tokudb_LOCKTREE_STO_ENDED_NUM`
     -
     -
   * - :ref:`Tokudb_LOCKTREE_STO_ENDED_SECONDS`
     -
     -
   * - :ref:`Tokudb_LOCKTREE_WAIT_COUNT`
     -
     -
   * - :ref:`Tokudb_LOCKTREE_WAIT_TIME`
     -
     -
   * - :ref:`Tokudb_LOCKTREE_LONG_WAIT_COUNT`
     -
     -
   * - :ref:`Tokudb_LOCKTREE_LONG_WAIT_TIME`
     -
     -
   * - :ref:`Tokudb_LOCKTREE_TIMEOUT_COUNT`
     -
     -
   * - :ref:`Tokudb_LOCKTREE_WAIT_ESCALATION_COUNT`
     -
     -
   * - :ref:`Tokudb_LOCKTREE_WAIT_ESCALATION_TIME`
     -
     -
   * - :ref:`Tokudb_LOCKTREE_LONG_WAIT_ESCALATION_COUNT`
     -
     -
   * - :ref:`Tokudb_LOCKTREE_LONG_WAIT_ESCALATION_TIME`
     -
     -
   * - :ref:`Tokudb_DICTIONARY_UPDATES`
     -
     -
   * - :ref:`Tokudb_DICTIONARY_BROADCAST_UPDATES`
     -
     -
   * - :ref:`Tokudb_DESCRIPTOR_SET`
     -
     -
   * - :ref:`Tokudb_MESSAGES_IGNORED_BY_LEAF_DUE_TO_MSN`
     -
     -
   * - :ref:`Tokudb_TOTAL_SEARCH_RETRIES`
     -
     -
   * - :ref:`Tokudb_SEARCH_TRIES_GT_HEIGHT`
     -
     -
   * - :ref:`Tokudb_SEARCH_TRIES_GT_HEIGHTPLUS3`
     -
     -
   * - :ref:`Tokudb_LEAF_NODES_FLUSHED_NOT_CHECKPOINT`
     -
     -
   * - :ref:`Tokudb_LEAF_NODES_FLUSHED_NOT_CHECKPOINT_BYTES`
     -
     -
   * - :ref:`Tokudb_LEAF_NODES_FLUSHED_NOT_CHECKPOINT_UNCOMPRESSED_BYTES`
     -
     -
   * - :ref:`Tokudb_LEAF_NODES_FLUSHED_NOT_CHECKPOINT_SECONDS`
     -
     -
   * - :ref:`Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_NOT_CHECKPOINT`
     -
     -
   * - :ref:`Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_NOT_CHECKPOINT_BYTES`
     -
     -
   * - :ref:`Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_NOT_CHECKPOINT_UNCOMPRESSE`
     -
     -
   * - :ref:`Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_NOT_CHECKPOINT_SECONDS`
     -
     -
   * - :ref:`Tokudb_LEAF_NODES_FLUSHED_CHECKPOINT`
     -
     -
   * - :ref:`Tokudb_LEAF_NODES_FLUSHED_CHECKPOINT_BYTES`
     -
     -
   * - :ref:`Tokudb_LEAF_NODES_FLUSHED_CHECKPOINT_UNCOMPRESSED_BYTES`
     -
     -
   * - :ref:`Tokudb_LEAF_NODES_FLUSHED_CHECKPOINT_SECONDS`
     -
     -
   * - :ref:`Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_CHECKPOINT`
     -
     -
   * - :ref:`Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_CHECKPOINT_BYTES`
     -
     -
   * - :ref:`Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_CHECKPOINT_UNCOMPRESSED_BY`
     -
     -
   * - :ref:`Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_CHECKPOINT_SECONDS`
     -
     -
   * - :ref:`Tokudb_LEAF_NODE_COMPRESSION_RATIO`
     -
     -
   * - :ref:`Tokudb_NONLEAF_NODE_COMPRESSION_RATIO`
     -
     -
   * - :ref:`Tokudb_OVERALL_NODE_COMPRESSION_RATIO`
     -
     -
   * - :ref:`Tokudb_NONLEAF_NODE_PARTIAL_EVICTIONS`
     -
     -
   * - :ref:`Tokudb_NONLEAF_NODE_PARTIAL_EVICTIONS_BYTES`
     -
     -
   * - :ref:`Tokudb_LEAF_NODE_PARTIAL_EVICTIONS`
     -
     -
   * - :ref:`Tokudb_LEAF_NODE_PARTIAL_EVICTIONS_BYTES`
     -
     -
   * - :ref:`Tokudb_LEAF_NODE_FULL_EVICTIONS`
     -
     -
   * - :ref:`Tokudb_LEAF_NODE_FULL_EVICTIONS_BYTES`
     -
     -
   * - :ref:`Tokudb_NONLEAF_NODE_FULL_EVICTIONS`
     -
     -
   * - :ref:`Tokudb_NONLEAF_NODE_FULL_EVICTIONS_BYTES`
     -
     -
   * - :ref:`Tokudb_LEAF_NODES_CREATED`
     -
     -
   * - :ref:`Tokudb_NONLEAF_NODES_CREATED`
     -
     -
   * - :ref:`Tokudb_LEAF_NODES_DESTROYED`
     -
     -
   * - :ref:`Tokudb_NONLEAF_NODES_DESTROYED`
     -
     -
   * - :ref:`Tokudb_MESSAGES_INJECTED_AT_ROOT_BYTES`
     -
     -
   * - :ref:`Tokudb_MESSAGES_FLUSHED_FROM_H1_TO_LEAVES_BYTES`
     -
     -
   * - :ref:`Tokudb_MESSAGES_IN_TREES_ESTIMATE_BYTES`
     -
     -
   * - :ref:`Tokudb_MESSAGES_INJECTED_AT_ROOT`
     -
     -
   * - :ref:`Tokudb_BROADCASE_MESSAGES_INJECTED_AT_ROOT`
     -
     -
   * - :ref:`Tokudb_BASEMENTS_DECOMPRESSED_TARGET_QUERY`
     -
     -
   * - :ref:`Tokudb_BASEMENTS_DECOMPRESSED_PRELOCKED_RANGE`
     -
     -
   * - :ref:`Tokudb_BASEMENTS_DECOMPRESSED_PREFETCH`
     -
     -
   * - :ref:`Tokudb_BASEMENTS_DECOMPRESSED_FOR_WRITE`
     -
     -
   * - :ref:`Tokudb_BUFFERS_DECOMPRESSED_TARGET_QUERY`
     -
     -
   * - :ref:`Tokudb_BUFFERS_DECOMPRESSED_PRELOCKED_RANGE`
     -
     -
   * - :ref:`Tokudb_BUFFERS_DECOMPRESSED_PREFETCH`
     -
     -
   * - :ref:`Tokudb_BUFFERS_DECOMPRESSED_FOR_WRITE`
     -
     -
   * - :ref:`Tokudb_PIVOTS_FETCHED_FOR_QUERY`
     -
     -
   * - :ref:`Tokudb_PIVOTS_FETCHED_FOR_QUERY_BYTES`
     -
     -
   * - :ref:`Tokudb_PIVOTS_FETCHED_FOR_QUERY_SECONDS`
     -
     -
   * - :ref:`Tokudb_PIVOTS_FETCHED_FOR_PREFETCH`
     -
     -
   * - :ref:`Tokudb_PIVOTS_FETCHED_FOR_PREFETCH_BYTES`
     -
     -
   * - :ref:`Tokudb_PIVOTS_FETCHED_FOR_PREFETCH_SECONDS`
     -
     -
   * - :ref:`Tokudb_PIVOTS_FETCHED_FOR_WRITE`
     -
     -
   * - :ref:`Tokudb_PIVOTS_FETCHED_FOR_WRITE_BYTES`
     -
     -
   * - :ref:`Tokudb_PIVOTS_FETCHED_FOR_WRITE_SECONDS`
     -
     -
   * - :ref:`Tokudb_BASEMENTS_FETCHED_TARGET_QUERY`
     -
     -
   * - :ref:`Tokudb_BASEMENTS_FETCHED_TARGET_QUERY_BYTES`
     -
     -
   * - :ref:`Tokudb_BASEMENTS_FETCHED_TARGET_QUERY_SECONDS`
     -
     -
   * - :ref:`Tokudb_BASEMENTS_FETCHED_PRELOCKED_RANGE`
     -
     -
   * - :ref:`Tokudb_BASEMENTS_FETCHED_PRELOCKED_RANGE_BYTES`
     -
     -
   * - :ref:`Tokudb_BASEMENTS_FETCHED_PRELOCKED_RANGE_SECONDS`
     -
     -
   * - :ref:`Tokudb_BASEMENTS_FETCHED_PREFETCH`
     -
     -
   * - :ref:`Tokudb_BASEMENTS_FETCHED_PREFETCH_BYTES`
     -
     -
   * - :ref:`Tokudb_BASEMENTS_FETCHED_PREFETCH_SECONDS`
     -
     -
   * - :ref:`Tokudb_BASEMENTS_FETCHED_FOR_WRITE`
     -
     -
   * - :ref:`Tokudb_BASEMENTS_FETCHED_FOR_WRITE_BYTES`
     -
     -
   * - :ref:`Tokudb_BASEMENTS_FETCHED_FOR_WRITE_SECONDS`
     -
     -
   * - :ref:`Tokudb_BUFFERS_FETCHED_TARGET_QUERY`
     -
     -
   * - :ref:`Tokudb_BUFFERS_FETCHED_TARGET_QUERY_BYTES`
     -
     -
   * - :ref:`Tokudb_BUFFERS_FETCHED_TARGET_QUERY_SECONDS`
     -
     -
   * - :ref:`Tokudb_BUFFERS_FETCHED_PRELOCKED_RANGE`
     -
     -
   * - :ref:`Tokudb_BUFFERS_FETCHED_PRELOCKED_RANGE_BYTES`
     -
     -
   * - :ref:`Tokudb_BUFFERS_FETCHED_PRELOCKED_RANGE_SECONDS`
     -
     -
   * - :ref:`Tokudb_BUFFERS_FETCHED_PREFETCH`
     -
     -
   * - :ref:`Tokudb_BUFFERS_FETCHED_PREFETCH_BYTES`
     -
     -
   * - :ref:`Tokudb_BUFFERS_FETCHED_PREFETCH_SECONDS`
     -
     -
   * - :ref:`Tokudb_BUFFERS_FETCHED_FOR_WRITE`
     -
     -
   * - :ref:`Tokudb_BUFFERS_FETCHED_FOR_WRITE_BYTES`
     -
     -
   * - :ref:`Tokudb_BUFFERS_FETCHED_FOR_WRITE_SECONDS`
     -
     -
   * - :ref:`Tokudb_LEAF_COMPRESSION_TO_MEMORY_SECONDS`
     -
     -
   * - :ref:`Tokudb_LEAF_SERIALIZATION_TO_MEMORY_SECONDS`
     -
     -
   * - :ref:`Tokudb_LEAF_DECOMPRESSION_TO_MEMORY_SECONDS`
     -
     -
   * - :ref:`Tokudb_LEAF_DESERIALIZATION_TO_MEMORY_SECONDS`
     -
     -
   * - :ref:`Tokudb_NONLEAF_COMPRESSION_TO_MEMORY_SECONDS`
     -
     -
   * - :ref:`Tokudb_NONLEAF_SERIALIZATION_TO_MEMORY_SECONDS`
     -
     -
   * - :ref:`Tokudb_NONLEAF_DECOMPRESSION_TO_MEMORY_SECONDS`
     -
     -
   * - :ref:`Tokudb_NONLEAF_DESERIALIZATION_TO_MEMORY_SECONDS`
     -
     -
   * - :ref:`Tokudb_PROMOTION_ROOTS_SPLIT`
     -
     -
   * - :ref:`Tokudb_PROMOTION_LEAF_ROOTS_INJECTED_INTO`
     -
     -
   * - :ref:`Tokudb_PROMOTION_H1_ROOTS_INJECTED_INTO`
     -
     -
   * - :ref:`Tokudb_PROMOTION_INJECTIONS_AT_DEPTH_0`
     -
     -
   * - :ref:`Tokudb_PROMOTION_INJECTIONS_AT_DEPTH_1`
     -
     -
   * - :ref:`Tokudb_PROMOTION_INJECTIONS_AT_DEPTH_2`
     -
     -
   * - :ref:`Tokudb_PROMOTION_INJECTIONS_AT_DEPTH_3`
     -
     -
   * - :ref:`Tokudb_PROMOTION_INJECTIONS_LOWER_THAN_DEPTH_3`
     -
     -
   * - :ref:`Tokudb_PROMOTION_STOPPED_NONEMPTY_BUFFER`
     -
     -
   * - :ref:`Tokudb_PROMOTION_STOPPED_AT_HEIGHT_1`
     -
     -
   * - :ref:`Tokudb_PROMOTION_STOPPED_CHILD_LOCKED_OR_NOT_IN_MEMORY`
     -
     -
   * - :ref:`Tokudb_PROMOTION_STOPPED_CHILD_NOT_FULLY_IN_MEMORY`
     -
     -
   * - :ref:`Tokudb_PROMOTION_STOPPED_AFTER_LOCKING_CHILD`
     -
     -
   * - :ref:`Tokudb_BASEMENT_DESERIALIZATION_FIXED_KEY`
     -
     -
   * - :ref:`Tokudb_BASEMENT_DESERIALIZATION_VARIABLE_KEY`
     -
     -
   * - :ref:`Tokudb_PRO_RIGHTMOST_LEAF_SHORTCUT_SUCCESS`
     -
     -
   * - :ref:`Tokudb_PRO_RIGHTMOST_LEAF_SHORTCUT_FAIL_POS`
     -
     -
   * - :ref:`Tokudb_RIGHTMOST_LEAF_SHORTCUT_FAIL_REACTIVE`
     -
     -
   * - :ref:`Tokudb_CURSOR_SKIP_DELETED_LEAF_ENTRY`
     -
     -
   * - :ref:`Tokudb_FLUSHER_CLEANER_TOTAL_NODES`
     -
     -
   * - :ref:`Tokudb_FLUSHER_CLEANER_H1_NODES`
     -
     -
   * - :ref:`Tokudb_FLUSHER_CLEANER_HGT1_NODES`
     -
     -
   * - :ref:`Tokudb_FLUSHER_CLEANER_EMPTY_NODES`
     -
     -
   * - :ref:`Tokudb_FLUSHER_CLEANER_NODES_DIRTIED`
     -
     -
   * - :ref:`Tokudb_FLUSHER_CLEANER_MAX_BUFFER_SIZE`
     -
     -
   * - :ref:`Tokudb_FLUSHER_CLEANER_MIN_BUFFER_SIZE`
     -
     -
   * - :ref:`Tokudb_FLUSHER_CLEANER_TOTAL_BUFFER_SIZE`
     -
     -
   * - :ref:`Tokudb_FLUSHER_CLEANER_MAX_BUFFER_WORKDONE`
     -
     -
   * - :ref:`Tokudb_FLUSHER_CLEANER_MIN_BUFFER_WORKDONE`
     -
     -
   * - :ref:`Tokudb_FLUSHER_CLEANER_TOTAL_BUFFER_WORKDONE`
     -
     -
   * - :ref:`Tokudb_FLUSHER_CLEANER_NUM_LEAF_MERGES_STARTED`
     -
     -
   * - :ref:`Tokudb_FLUSHER_CLEANER_NUM_LEAF_MERGES_RUNNING`
     -
     -
   * - :ref:`Tokudb_FLUSHER_CLEANER_NUM_LEAF_MERGES_COMPLETED`
     -
     -
   * - :ref:`Tokudb_FLUSHER_CLEANER_NUM_DIRTIED_FOR_LEAF_MERGE`
     -
     -
   * - :ref:`Tokudb_FLUSHER_FLUSH_TOTAL`
     -
     -
   * - :ref:`Tokudb_FLUSHER_FLUSH_IN_MEMORY`
     -
     -
   * - :ref:`Tokudb_FLUSHER_FLUSH_NEEDED_IO`
     -
     -
   * - :ref:`Tokudb_FLUSHER_FLUSH_CASCADES`
     -
     -
   * - :ref:`Tokudb_FLUSHER_FLUSH_CASCADES_1`
     -
     -
   * - :ref:`Tokudb_FLUSHER_FLUSH_CASCADES_2`
     -
     -
   * - :ref:`Tokudb_FLUSHER_FLUSH_CASCADES_3`
     -
     -
   * - :ref:`Tokudb_FLUSHER_FLUSH_CASCADES_4`
     -
     -
   * - :ref:`Tokudb_FLUSHER_FLUSH_CASCADES_5`
     -
     -
   * - :ref:`Tokudb_FLUSHER_FLUSH_CASCADES_GT_5`
     -
     -
   * - :ref:`Tokudb_FLUSHER_SPLIT_LEAF`
     -
     -
   * - :ref:`Tokudb_FLUSHER_SPLIT_NONLEAF`
     -
     -
   * - :ref:`Tokudb_FLUSHER_MERGE_LEAF`
     -
     -
   * - :ref:`Tokudb_FLUSHER_MERGE_NONLEAF`
     -
     -
   * - :ref:`Tokudb_FLUSHER_BALANCE_LEAF`
     -
     -
   * - :ref:`Tokudb_HOT_NUM_STARTED`
     -
     -
   * - :ref:`Tokudb_HOT_NUM_COMPLETED`
     -
     -
   * - :ref:`Tokudb_HOT_NUM_ABORTED`
     -
     -
   * - :ref:`Tokudb_HOT_MAX_ROOT_FLUSH_COUNT`
     -
     -
   * - :ref:`Tokudb_TXN_BEGIN`
     -
     -
   * - :ref:`Tokudb_TXN_BEGIN_READ_ONLY`
     -
     -
   * - :ref:`Tokudb_TXN_COMMITS`
     -
     -
   * - :ref:`Tokudb_TXN_ABORTS`
     -
     -
   * - :ref:`Tokudb_LOGGER_NEXT_LSN`
     -
     -
   * - :ref:`Tokudb_LOGGER_WRITES`
     -
     -
   * - :ref:`Tokudb_LOGGER_WRITES_BYTES`
     -
     -
   * - :ref:`Tokudb_LOGGER_WRITES_UNCOMPRESSED_BYTES`
     -
     -
   * - :ref:`Tokudb_LOGGER_WRITES_SECONDS`
     -
     -
   * - :ref:`Tokudb_LOGGER_WAIT_LONG`
     -
     -
   * - :ref:`Tokudb_LOADER_NUM_CREATED`
     -
     -
   * - :ref:`Tokudb_LOADER_NUM_CURRENT`
     -
     -
   * - :ref:`Tokudb_LOADER_NUM_MAX`
     -
     -
   * - :ref:`Tokudb_MEMORY_MALLOC_COUNT`
     -
     -
   * - :ref:`Tokudb_MEMORY_FREE_COUNT`
     -
     -
   * - :ref:`Tokudb_MEMORY_REALLOC_COUNT`
     -
     -
   * - :ref:`Tokudb_MEMORY_MALLOC_FAIL`
     -
     -
   * - :ref:`Tokudb_MEMORY_REALLOC_FAIL`
     -
     -
   * - :ref:`Tokudb_MEMORY_REQUESTED`
     -
     -
   * - :ref:`Tokudb_MEMORY_USED`
     -
     -
   * - :ref:`Tokudb_MEMORY_FREED`
     -
     -
   * - :ref:`Tokudb_MEMORY_MAX_REQUESTED_SIZE`
     -
     -
   * - :ref:`Tokudb_MEMORY_LAST_FAILED_SIZE`
     -
     -
   * - :ref:`Tokudb_MEM_ESTIMATED_MAXIMUM_MEMORY_FOOTPRINT`
     -
     -
   * - :ref:`Tokudb_MEMORY_MALLOCATOR_VERSION`
     -
     -
   * - :ref:`Tokudb_MEMORY_MMAP_THRESHOLD`
     -
     -
   * - :ref:`Tokudb_FILESYSTEM_THREADS_BLOCKED_BY_FULL_DISK`
     -
     -
   * - :ref:`Tokudb_FILESYSTEM_FSYNC_TIME`
     -
     -
   * - :ref:`Tokudb_FILESYSTEM_FSYNC_NUM`
     -
     -
   * - :ref:`Tokudb_FILESYSTEM_LONG_FSYNC_TIME`
     -
     -
   * - :ref:`Tokudb_FILESYSTEM_LONG_FSYNC_NUM`
     -
     -
