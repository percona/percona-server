.. _ps_variables:

====================================================
 List of variables introduced in Percona Server 5.6
====================================================

System Variables
================

* :variable:`enforce_storage_engine`
* :variable:`expand_fast_index_creation`
* :variable:`extra_max_connections`
* :variable:`extra_port`
* :variable:`innodb_buffer_pool_populate`
* :variable:`innodb_corrupt_table_action`
* :variable:`innodb_fake_changes`
* :variable:`innodb_kill_idle_transaction`
* :variable:`innodb_locking_fake_changes`
* :variable:`innodb_log_arch_dir`
* :variable:`innodb_log_arch_expire_sec`
* :variable:`innodb_log_archive`
* :variable:`innodb_log_block_size`
* :variable:`innodb_max_bitmap_file_size`
* :variable:`innodb_max_changed_pages`
* :variable:`innodb_show_locks_held`
* :variable:`innodb_show_verbose_locks`
* :variable:`innodb_track_changed_pages`
* :variable:`innodb_use_atomic_writes`
* :variable:`innodb_use_global_flush_log_at_trx_commit`
* :variable:`log_slow_filter`
* :variable:`log_slow_rate_limit`
* :variable:`log_slow_rate_type`
* :variable:`log_slow_sp_statements`
* :variable:`log_slow_verbosity`
* :variable:`log_warnings_suppress`
* :variable:`max_binlog_files`
* :variable:`query_cache_strip_comments`
* :variable:`slow_query_log_timestamp_always`
* :variable:`slow_query_log_timestamp_precision`
* :variable:`slow_query_log_use_global_control`
* :variable:`thread_pool_high_prio_tickets`
* :variable:`thread_pool_idle_timeout`
* :variable:`thread_pool_max_threads`
* :variable:`thread_pool_oversubscribe`
* :variable:`thread_pool_size`
* :variable:`thread_pool_stall_limit`
* :variable:`thread_statistics`
* :variable:`userstat`

Status Variables
================

* :variable:`Com_purge_archived`
* :variable:`Com_purge_archived_before_date`
* :variable:`Com_show_client_statistics`
* :variable:`Com_show_index_statistics`
* :variable:`Com_show_slave_status_nolock`
* :variable:`Com_show_table_statistics`
* :variable:`Com_show_thread_statistics`
* :variable:`Com_show_user_statistics`
* :variable:`Innodb_background_log_sync`
* :variable:`Innodb_buffer_pool_pages_LRU_flushed`
* :variable:`Innodb_buffer_pool_pages_made_not_young`
* :variable:`Innodb_buffer_pool_pages_made_young`
* :variable:`Innodb_buffer_pool_pages_old`
* :variable:`Innodb_checkpoint_age`
* :variable:`Innodb_checkpoint_max_age`
* :variable:`Innodb_deadlocks`
* :variable:`Innodb_history_list_length`
* :variable:`Innodb_ibuf_discarded_delete_marks`
* :variable:`Innodb_ibuf_discarded_deletes`
* :variable:`Innodb_ibuf_discarded_inserts`
* :variable:`Innodb_ibuf_free_list`
* :variable:`Innodb_ibuf_merged_delete_marks`
* :variable:`Innodb_ibuf_merged_deletes`
* :variable:`Innodb_ibuf_merged_inserts`
* :variable:`Innodb_ibuf_merges`
* :variable:`Innodb_ibuf_segment_size`
* :variable:`Innodb_ibuf_size`
* :variable:`Innodb_lsn_current`
* :variable:`Innodb_lsn_flushed`
* :variable:`Innodb_lsn_last_checkpoint`
* :variable:`Innodb_master_thread_active_loops`
* :variable:`Innodb_master_thread_idle_loops`
* :variable:`Innodb_max_trx_id`
* :variable:`Innodb_mem_adaptive_hash`
* :variable:`Innodb_mem_dictionary`
* :variable:`Innodb_mem_total`
* :variable:`Innodb_mutex_os_waits`
* :variable:`Innodb_mutex_spin_rounds`
* :variable:`Innodb_mutex_spin_waits`
* :variable:`Innodb_oldest_view_low_limit_trx_id`
* :variable:`Innodb_purge_trx_id`
* :variable:`Innodb_purge_undo_no`
* :variable:`Innodb_current_row_locks`
* :variable:`Innodb_read_views_memory`
* :variable:`Innodb_descriptors_memory`
* :variable:`Innodb_s_lock_os_waits`
* :variable:`Innodb_s_lock_spin_rounds`
* :variable:`Innodb_s_lock_spin_waits`
* :variable:`Innodb_x_lock_os_waits`
* :variable:`Innodb_x_lock_spin_rounds`
* :variable:`Innodb_x_lock_spin_waits`
* :variable:`Threadpool_idle_threads`
* :variable:`Threadpool_threads`
