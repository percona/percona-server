.. _ps_variables:

====================================================
 List of variables introduced in Percona Server 5.5
====================================================

System Variables
================

.. tabularcolumns:: |p{8cm}|p{1.5cm}|p{2cm}|p{2cm}|p{1.5cm}|

.. list-table::
   :header-rows: 1

   * - Name
     - Cmd-Line    
     - Option File 
     - Var Scope   
     - Dynamic
   * - :variable:`csv_mode`
     - Yes
     - Yes
     - Both
     - Yes
   * - :variable:`enforce_storage_engine`
     - No
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
   * - :variable:`fast_index_creation`
     - Yes
     - No
     - Session
     - Yes
   * - :variable:`have_flashcache`
     - No
     - No
     - Global
     - No
   * - :variable:`have_response_time_distribution`
     - No
     - No
     - Global
     - No
   * - :variable:`innodb_adaptive_flushing_method`
     - Yes
     - Yes 
     - Global
     - Yes
   * - :variable:`innodb_adaptive_hash_index_partitions`
     - Yes 
     - Yes
     - Global
     - No
   * - :variable:`innodb_blocking_buffer_pool_restore`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`innodb_buffer_pool_populate`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`innodb_buffer_pool_restore_at_startup`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`innodb_buffer_pool_shm_checksum`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`innodb_buffer_pool_shm_key`
     - Yes
     - Yes
     - Global 
     - No
   * - :variable:`innodb_checkpoint_age_target`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`innodb_corrupt_table_action`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`innodb_dict_size_limit`
     - Yes
     - Yes 
     - Global
     - Yes
   * - :variable:`innodb_doublewrite_file`
     - Yes
     - Yes 
     - Global
     - No
   * - :variable:`innodb_fake_changes`
     - Yes
     - Yes
     - Both
     - Yes
   * - :variable:`innodb_fast_checksum`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`innodb_flush_neighbor_pages`
     - Yes
     - Yes 
     - Global
     - Yes
   * - :variable:`innodb_ibuf_accel_rate`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`innodb_ibuf_active_contract`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`innodb_ibuf_max_size`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`innodb_import_table_from_xtrabackup`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`innodb_kill_idle_transaction`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`innodb_lazy_drop_table`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`innodb_locking_fake_changes`
     - Yes
     - Yes
     - Both
     - Yes
   * - :variable:`innodb_log_block_size`
     - Yes
     - Yes
     - Global
     - No
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
   * - :variable:`innodb_merge_sort_block_size`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`innodb_page_size`
     - Yes
     - Yes 
     - Global
     - No
   * - :variable:`innodb_read_ahead`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`innodb_recovery_stats`
     - No
     - Yes
     - Global
     - No
   * - :variable:`innodb_recovery_update_relay_log`
     - Yes
     - Yes
     - Global
     - No
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
   * - :variable:`innodb_stats_auto_update`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`innodb_stats_update_need_lock`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`innodb_thread_concurrency_timer_based`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`innodb_track_changed_pages`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`innodb_use_atomic_writes`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`innodb_use_global_flush_log_at_trx_commit`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`innodb_use_sys_stats_table`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`log_slow_admin_statements`
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
   * - :variable:`log_slow_slave_statements`
     - Yes
     - Yes
     - Both
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
   * - :variable:`optimizer_fix`
     - N/A
     - N/A
     - N/A 
     - N/A
   * - :variable:`query_cache_strip_comments`
     - Yes
     - Yes
     - Global
     - Yes
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
   * - :variable:`slow_query_log_timestamp_always`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`slow_query_log_timestamp_precision`
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
   * - :variable:`userstat`
     - Yes
     - Yes
     - Global
     - Yes


Status Variables
================

.. tabularcolumns:: |p{9cm}|p{3cm}|p{3cm}|

.. list-table::
   :header-rows: 1

   * - Name
     - Var Type
     - Var Scope
   * - :variable:`Com_show_client_statistics`
     - Numeric
     - Both
   * - :variable:`Com_show_index_statistics`
     - Numeric
     - Both
   * - :variable:`Com_show_slave_status_nolock`
     - Numeric
     - Both
   * - :variable:`Com_show_table_statistics`
     - Numeric
     - Both
   * - :variable:`Com_show_temporary_tables`
     - Numeric
     - Both
   * - :variable:`Com_show_thread_statistics`
     - Numeric
     - Both
   * - :variable:`Com_show_user_statistics`
     - Numeric
     - Both
   * - :variable:`Flashcache_enabled`
     - Boolean
     - Global
   * - :variable:`Innodb_adaptive_hash_cells`
     - Numeric
     - Global
   * - :variable:`Innodb_adaptive_hash_heap_buffers`
     - Numeric
     - Global
   * - :variable:`Innodb_adaptive_hash_hash_searches`
     - Numeric
     - Global
   * - :variable:`Innodb_adaptive_hash_non_hash_searches`
     - Numeric
     - Global
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
   * - :variable:`Innodb_checkpoint_target_age`
     - Numeric
     - Global
   * - :variable:`Innodb_deadlocks`
     - Numeric
     - Global
   * - :variable:`Innodb_dict_tables`
     - Numeric
     - Global
   * - :variable:`Innodb_history_list_length`
     - Numeric
     - Global
   * - :variable:`Innodb_ibuf_discarded_delete_marks`
     - Numeric
     - Global
   * - :variable:`Innodb_ibuf_discarded_deletes`
     - Numeric
     - Global
   * - :variable:`Innodb_ibuf_discarded_inserts`
     - Numeric
     - Global
   * - :variable:`Innodb_ibuf_free_list`
     - Numeric
     - Global
   * - :variable:`Innodb_ibuf_merged_delete_marks`
     - Numeric
     - Global
   * - :variable:`Innodb_ibuf_merged_deletes`
     - Numeric
     - Global
   * - :variable:`Innodb_ibuf_merged_inserts`
     - Numeric
     - Global
   * - :variable:`Innodb_ibuf_merges`
     - Numeric
     - Global
   * - :variable:`Innodb_ibuf_segment_size`
     - Numeric
     - Global
   * - :variable:`Innodb_ibuf_size`
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
   * - :variable:`Innodb_master_thread_1_second_loops`
     - Numeric
     - Global
   * - :variable:`Innodb_master_thread_10_second_loops`
     - Numeric
     - Global
   * - :variable:`Innodb_master_thread_background_loops`
     - Numeric
     - Global
   * - :variable:`Innodb_master_thread_main_flush_loops`
     - Numeric
     - Global
   * - :variable:`Innodb_master_thread_sleeps`
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
   * - :variable:`Innodb_mem_total`
     - Numeric
     - Global
   * - :variable:`Innodb_mutex_os_waits`
     - Numeric
     - Global
   * - :variable:`Innodb_mutex_spin_rounds`
     - Numeric
     - Global
   * - :variable:`Innodb_mutex_spin_waits`
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
   * - :variable:`Innodb_current_row_locks`
     - Numeric
     - Global
   * - :variable:`Innodb_read_views_memory`
     - Numeric
     - Global
   * - :variable:`Innodb_descriptors_memory`
     - Numeric
     - Global
   * - :variable:`Innodb_s_lock_os_waits`
     - Numeric
     - Global
   * - :variable:`Innodb_s_lock_spin_rounds`
     - Numeric
     - Global
   * - :variable:`Innodb_s_lock_spin_waits`
     - Numeric
     - Global
   * - :variable:`Innodb_x_lock_os_waits`
     - Numeric
     - Global
   * - :variable:`Innodb_x_lock_spin_rounds`
     - Numeric
     - Global
   * - :variable:`Innodb_x_lock_spin_waits`
     - Numeric
     - Global
   * - :variable:`Threadpool_idle_threads`
     - Numeric
     - Global
   * - :variable:`Threadpool_threads`
     - Numeric
     - Global
   * - :variable:`binlog_commits`
     - Numeric
     - Session
   * - :variable:`binlog_group_commits`
     - Numeric
     - Session
