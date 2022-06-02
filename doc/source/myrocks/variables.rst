.. _myrocks_server_variables:

========================
MyRocks Server Variables
========================

The MyRocks server variables expose configuration
of the underlying RocksDB engine.
There several ways to set these variables:

* For production deployments,
  you should have all variables defined in the configuration file.

* *Dynamic* variables can be changed at runtime using the ``SET`` statement.

* If you want to test things out, you can set some of the variables
  when starting ``mysqld`` using corresponding command-line options.

If a variable was not set in either the configuration file
or as a command-line option,
the default value is used.

Also, all variables can exist in one or both of the following scopes:

* *Global* scope defines how the variable affects overall server operation.

* *Session* scope defines how the variable affects operation
  for individual client connections.

The following server variables are available:

.. list-table::
   :header-rows: 1

   * - Variable Name
   * - :ref:`rocksdb_access_hint_on_compaction_start` 
   * - :ref:`rocksdb_advise_random_on_open`
   * - :ref:`rocksdb_allow_concurrent_memtable_write`
   * - :ref:`rocksdb_allow_mmap_reads`
   * - :ref:`rocksdb_allow_mmap_writes` 
   * - :ref:`rocksdb_allow_to_start_after_corruption`
   * - :ref:`rocksdb_alter_column_default_inplace` 
   * - :ref:`rocksdb_base_background_compactions`
   * - :ref:`rocksdb_blind_delete_primary_key` 
   * - :ref:`rocksdb_block_cache_size`
   * - :ref:`rocksdb_block_restart_interval` 
   * - :ref:`rocksdb_block_size`
   * - :ref:`rocksdb_block_size_deviation`
   * - :ref:`rocksdb_bulk_load`
   * - :ref:`rocksdb_bulk_load_allow_sk`
   * - :ref:`rocksdb_bulk_load_allow_unsorted` 
   * - :ref:`rocksdb_bulk_load_size` 
   * - :ref:`rocksdb_bytes_per_sync`
   * - :ref:`rocksdb_cache_dump`
   * - :ref:`rocksdb_cache_index_and_filter_blocks`
   * - :ref:`rocksdb_checksums_pct`
   * - :ref:`rocksdb_collect_sst_properties`
   * - :ref:`rocksdb_commit_in_the_middle`
   * - :ref:`rocksdb_commit_time_batch_for_recovery`
   * - :ref:`rocksdb_compact_cf`
   * - :ref:`rocksdb_compaction_readahead_size`
   * - :ref:`rocksdb_compaction_sequential_deletes`
   * - :ref:`rocksdb_compaction_sequential_deletes_count_sd` 
   * - :ref:`rocksdb_compaction_sequential_deletes_file_size`
   * - :ref:`rocksdb_compaction_sequential_deletes_window`
   * - :ref:`rocksdb_concurrent_prepare`
   * - :ref:`rocksdb_create_checkpoint`
   * - :ref:`rocksdb_create_if_missing`
   * - :ref:`rocksdb_create_missing_column_families`
   * - :ref:`rocksdb_datadir`
   * - :ref:`rocksdb_db_write_buffer_size` 
   * - :ref:`rocksdb_deadlock_detect`
   * - :ref:`rocksdb_deadlock_detect_depth`
   * - :ref:`rocksdb_debug_optimizer_no_zero_cardinality`
   * - :ref:`rocksdb_debug_ttl_ignore_pk`
   * - :ref:`rocksdb_debug_ttl_read_filter_ts`
   * - :ref:`rocksdb_debug_ttl_rec_ts`
   * - :ref:`rocksdb_debug_ttl_snapshot_ts`
   * - :ref:`rocksdb_default_cf_options`
   * - :ref:`rocksdb_delayed_write_rate`
   * - :ref:`rocksdb_delete_cf`
   * - :ref:`rocksdb_delete_obsolete_files_period_micros`
   * - :ref:`rocksdb_enable_bulk_load_api`
   * - :ref:`rocksdb_enable_insert_with_update_caching`
   * - :ref:`rocksdb_enable_iterate_bounds`
   * - :ref:`rocksdb_enable_native_partition`
   * - :ref:`rocksdb_enable_pipelined_write`
   * - :ref:`rocksdb_enable_remove_orphaned_dropped_cfs`
   * - :ref:`rocksdb_enable_ttl`
   * - :ref:`rocksdb_enable_ttl_read_filtering`
   * - :ref:`rocksdb_enable_thread_tracking`
   * - :ref:`rocksdb_enable_write_thread_adaptive_yield`
   * - :ref:`rocksdb_error_if_exists`
   * - :ref:`rocksdb_error_on_suboptimal_collation`
   * - :ref:`rocksdb_flush_log_at_trx_commit`
   * - :ref:`rocksdb_flush_memtable_on_analyze`
   * - :ref:`rocksdb_force_compute_memtable_stats`
   * - :ref:`rocksdb_force_compute_memtable_stats_cachetime`
   * - :ref:`rocksdb_force_flush_memtable_and_lzero_now`
   * - :ref:`rocksdb_force_flush_memtable_now`
   * - :ref:`rocksdb_force_index_records_in_range`
   * - :ref:`rocksdb_hash_index_allow_collision`
   * - :ref:`rocksdb_ignore_unknown_options`
   * - :ref:`rocksdb_index_type`
   * - :ref:`rocksdb_info_log_level`
   * - :ref:`rocksdb_is_fd_close_on_exec`
   * - :ref:`rocksdb_keep_log_file_num`
   * - :ref:`rocksdb_large_prefix`
   * - :ref:`rocksdb_lock_scanned_rows`
   * - :ref:`rocksdb_lock_wait_timeout`
   * - :ref:`rocksdb_log_file_time_to_roll`
   * - :ref:`rocksdb_manifest_preallocation_size`
   * - :ref:`rocksdb_manual_compaction_bottommost_level`
   * - :ref:`rocksdb_manual_wal_flush`
   * - :ref:`rocksdb_master_skip_tx_api`
   * - :ref:`rocksdb_max_background_compactions`
   * - :ref:`rocksdb_max_background_flushes`
   * - :ref:`rocksdb_max_background_jobs`
   * - :ref:`rocksdb_max_bottom_pri_background_compactions`
   * - :ref:`rocksdb_max_latest_deadlocks`
   * - :ref:`rocksdb_max_log_file_size`
   * - :ref:`rocksdb_max_manifest_file_size`
   * - :ref:`rocksdb_max_open_files`
   * - :ref:`rocksdb_max_row_locks`
   * - :ref:`rocksdb_max_subcompactions`
   * - :ref:`rocksdb_max_total_wal_size`
   * - :ref:`rocksdb_merge_buf_size`
   * - :ref:`rocksdb_merge_combine_read_size`
   * - :ref:`rocksdb_merge_tmp_file_removal_delay_ms`
   * - :ref:`rocksdb_new_table_reader_for_compaction_inputs`
   * - :ref:`rocksdb_no_block_cache`
   * - :ref:`rocksdb_no_create_column_family`
   * - :ref:`rocksdb_override_cf_options`
   * - :ref:`rocksdb_paranoid_checks`
   * - :ref:`rocksdb_pause_background_work`
   * - :ref:`rocksdb_perf_context_level`
   * - :ref:`rocksdb_persistent_cache_path`
   * - :ref:`rocksdb_persistent_cache_size_mb`
   * - :ref:`rocksdb_pin_l0_filter_and_index_blocks_in_cache`
   * - :ref:`rocksdb_print_snapshot_conflict_queries`
   * - :ref:`rocksdb_rate_limiter_bytes_per_sec`
   * - :ref:`rocksdb_read_free_rpl`
   * - :ref:`rocksdb_read_free_rpl_tables`
   * - :ref:`rocksdb_records_in_range`
   * - :ref:`rocksdb_reset_stats`
   * - :ref:`rocksdb_rollback_on_timeout`
   * - :ref:`rocksdb_rpl_skip_tx_api`
   * - :ref:`rocksdb_seconds_between_stat_computes`
   * - :ref:`rocksdb_signal_drop_index_thread`
   * - :ref:`rocksdb_sim_cache_size`
   * - :ref:`rocksdb_skip_bloom_filter_on_read`
   * - :ref:`rocksdb_skip_fill_cache`
   * - :ref:`rocksdb_skip_locks_if_skip_unique_check`
   * - :ref:`rocksdb_sst_mgr_rate_bytes_per_sec`
   * - :ref:`rocksdb_stats_dump_period_sec`
   * - :ref:`rocksdb_stats_level`
   * - :ref:`rocksdb_stats_recalc_rate`
   * - :ref:`rocksdb_store_row_debug_checksums`
   * - :ref:`rocksdb_strict_collation_check`
   * - :ref:`rocksdb_strict_collation_exceptions`
   * - :ref:`rocksdb_table_cache_numshardbits`
   * - :ref:`rocksdb_table_stats_background_thread_nice_value`
   * - :ref:`rocksdb_table_stats_max_num_rows_scanned`
   * - :ref:`rocksdb_table_stats_recalc_threshold_count`
   * - :ref:`rocksdb_table_stats_recalc_threshold_pct`
   * - :ref:`rocksdb_table_stats_sampling_pct`
   * - :ref:`rocksdb_table_stats_use_table_scan`
   * - :ref:`rocksdb_tmpdir`
   * - :ref:`rocksdb_two_write_queues`
   * - :ref:`rocksdb_trace_block_cache_access`
   * - :ref:`rocksdb_trace_queries`
   * - :ref:`rocksdb_trace_sst_api`
   * - :ref:`rocksdb_track_and_verify_wals_in_manifest`
   * - :ref:`rocksdb_unsafe_for_binlog`
   * - :ref:`rocksdb_update_cf_options`
   * - :ref:`rocksdb_use_adaptive_mutex`
   * - :ref:`rocksdb_use_default_sk_cf`
   * - :ref:`rocksdb_use_direct_io_for_flush_and_compaction`
   * - :ref:`rocksdb_use_direct_reads`
   * - :ref:`rocksdb_use_fsync`
   * - :ref:`rocksdb_validate_tables`
   * - :ref:`rocksdb_verify_row_debug_checksums`
   * - :ref:`rocksdb_wal_bytes_per_sync`
   * - :ref:`rocksdb_wal_dir`
   * - :ref:`rocksdb_wal_recovery_mode`
   * - :ref:`rocksdb_wal_size_limit_mb`
   * - :ref:`rocksdb_wal_ttl_seconds`
   * - :ref:`rocksdb_whole_key_filtering`
   * - :ref:`rocksdb_write_batch_max_bytes`
   * - :ref:`rocksdb_write_disable_wal`
   * - :ref:`rocksdb_write_ignore_missing_column_families`
   * - :ref:`rocksdb_write_policy`

.. _rocksdb_access_hint_on_compaction_start:

.. rubric:: ``rocksdb_access_hint_on_compaction_start``

Implemented in :ref:`5.7.19-17`.

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-access-hint-on-compaction-start``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - String or numeric
   * - Default
     - ``NORMAL`` or ``1``

Specifies the file access pattern once a compaction is started,
applied to all input files of a compaction.
Possible values are:

* ``0`` = ``NONE``
* ``1`` = ``NORMAL`` (default)
* ``2`` = ``SEQUENTIAL``
* ``3`` = ``WILLNEED``

.. _rocksdb_advise_random_on_open:

.. rubric:: ``rocksdb_advise_random_on_open``

Implemented in :ref:`5.7.19-17`.

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-advise-random-on-open``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``ON``

Specifies whether to hint the underlying file system
that the file access pattern is random,
when a data file is opened.
Enabled by default.

.. _rocksdb_allow_concurrent_memtable_write:

.. rubric:: ``rocksdb_allow_concurrent_memtable_write``

Implemented in :ref:`5.7.19-17`.

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-allow-concurrent-memtable-write``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Specifies whether to allow multiple writers to update memtables in parallel.
Disabled by default.

.. _rocksdb_allow_to_start_after_corruption:

.. rubric:: ``rocksdb_allow_to_start_after_corruption``

Implemented in :ref:`5.7.21-20`.

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb_allow_to_start_after_corruption``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Specifies whether to allow server to restart once MyRocks reported data
corruption. Disabled by default.

Once corruption is detected server writes marker file (named
ROCKSDB_CORRUPTED) in the data directory and aborts. If marker file exists,
then mysqld exits on startup with an error message. The restart failure will
continue until the problem is solved or until mysqld is started with this
variable turned on in the command line.

.. note:: Not all memtables support concurrent writes.

.. _rocksdb_allow_mmap_reads:

.. rubric:: ``rocksdb_allow_mmap_reads``

Implemented in :ref:`5.7.19-17`.

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-allow-mmap-reads``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Specifies whether to allow the OS to map a data file into memory for reads.
Disabled by default.
If you enable this, make sure that :ref:`rocksdb_use_direct_reads` is disabled.

.. _rocksdb_allow_mmap_writes:

.. rubric:: ``rocksdb_allow_mmap_writes``

Implemented in :ref:`5.7.19-17`.

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-allow-mmap-writes``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Specifies whether to allow the OS to map a data file into memory for writes.
Disabled by default.

.. _rocksdb_alter_column_default_inplace:

.. rubric:: ``rocksdb_alter_column_default_inplace``

Implemented in :ref:`5.7.35-38`.

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-alter-column-default-inplace``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``ON``

Allow inplace alter for alter column default operation.

.. _rocksdb_base_background_compactions:

.. rubric:: ``rocksdb_base_background_compactions``

Implemented in :ref:`5.7.19-17`.

Replaced by :ref:`rocksdb_max_background_jobs` in :ref:`5.7.20-18`.

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-base-background-compactions``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``1``

Specifies the suggested number of concurrent background compaction jobs,
submitted to the default LOW priority thread pool in RocksDB.
Default is ``1``.
Allowed range of values is from ``-1`` to ``64``.
Maximum depends on the :ref:`rocksdb_max_background_compactions`
variable. This variable has been replaced in *Percona Server for MySQL* :ref:`5.7.20-18`
by :ref:`rocksdb_max_background_jobs`, which automatically decides how
many threads to allocate towards flush/compaction.

.. _rocksdb_blind_delete_primary_key:

.. rubric:: ``rocksdb_blind_delete_primary_key``

Implemented in :ref:`5.7.30-33`.

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-blind-delete-primary-key``
   * - Dynamic
     - Yes
   * - Scope
     - Global, Session
   * - Data type
     - Boolean
   * - Default
     - ``OFF``
  
Skips verifying if rows exists before executing deletes. The following conditions
must be met:

* The variable is enabled
* Only a single table listed in the ``DELETE`` statement
* The table has only a primary key with no secondary keys

.. _rocksdb_block_cache_size:

.. rubric:: ``rocksdb_block_cache_size``

Implemented in :ref:`5.7.19-17`.

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-block-cache-size``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``536870912``

Specifies the size of the LRU block cache for RocksDB.
This memory is reserved for the block cache,
which is in addition to any filesystem caching that may occur.

Minimum value is ``1024``,
because that's the size of one block.

Default value is ``536870912``.

Maximum value is ``9223372036854775807``.

.. _rocksdb_block_restart_interval:

.. rubric:: ``rocksdb_block_restart_interval``

Implemented in :ref:`5.7.19-17`.

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-block-restart-interval``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``16``

Specifies the number of keys for each set of delta encoded data.
Default value is ``16``.
Allowed range is from ``1`` to ``2147483647``.

.. _rocksdb_block_size:

.. rubric:: ``rocksdb_block_size``

Implemented in :ref:`5.7.19-17`.

Minimum value has changed from ``0`` to ``1024`` in :ref:`5.7.20-18`

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-block-size``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``4096``

Specifies the size of the data block for reading RocksDB data files.
Default value is ``4096``.
Allowed range is from ``1024`` to ``18446744073709551615``.

.. _rocksdb_block_size_deviation:

.. rubric:: ``rocksdb_block_size_deviation``

Implemented in :ref:`5.7.19-17`.

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-block-size-deviation``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``10``

Specifies the threshold for free space allowed in a data block
(see :ref:`rocksdb_block_size`).
If there is less space remaining,
close the block (and write to new block).
Default value is ``10``, meaning that the block is not closed
until there is less than 10 bits of free space remaining.

Allowed range is from ``1`` to ``2147483647``.

.. _rocksdb_bulk_load_allow_sk:

.. rubric:: ``rocksdb_bulk_load_allow_sk``

Implemented in :ref:`5.7.23-23`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-bulk-load-allow-sk``
   * - Dynamic
     - Yes
   * - Scope
     - Global, Session
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Enabling this variable allows secondary keys to be added using the bulk loading
feature. This variable can be toggled only when bulk load is disabled, i.e.
when :ref:`rocksdb_bulk_load` is ``OFF``.

.. _rocksdb_bulk_load_allow_unsorted:

.. rubric:: ``rocksdb_bulk_load_allow_unsorted``

Implemented in :ref:`5.7.20-18`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-bulk-load-allow-unsorted``
   * - Dynamic
     - Yes
   * - Scope
     - Global, Session
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

By default, the bulk loader requires its input to be sorted in the primary
key order. If enabled, unsorted inputs are allowed too, which are then
sorted by the bulkloader itself, at a performance penalty.

.. _rocksdb_bulk_load:

.. rubric:: ``rocksdb_bulk_load``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-bulk-load``
   * - Dynamic
     - Yes
   * - Scope
     - Global, Session
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Specifies whether to use bulk load:
MyRocks will ignore checking keys for uniqueness
or acquiring locks during transactions.
Disabled by default.
Enable this only if you are certain that there are no row conflicts,
for example, when setting up a new MyRocks instance from a MySQL dump.

When the `rocksdb_bulk_load` variable is enabled, it behaves as if the variable `rocksdb_commit_in_the_middle` is enabled, even if the variable `rocksdb_commit_in_the_middle` is disabled.

.. _rocksdb_bulk_load_size:

.. rubric:: ``rocksdb_bulk_load_size``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-bulk-load-size``
   * - Dynamic
     - Yes
   * - Scope
     - Global, Session
   * - Data type
     - Numeric
   * - Default
     - ``1000``

Specifies the number of keys to accumulate
before committing them to the storage engine when bulk load is enabled
(see :ref:`rocksdb_bulk_load`).
Default value is ``1000``,
which means that a batch can contain up to 1000 records
before they are implicitly committed.
Allowed range is from ``1`` to ``1073741824``.

.. _rocksdb_bytes_per_sync:

.. rubric:: ``rocksdb_bytes_per_sync``

Implemented in :ref:`5.7.19-17`.

Changed to dynamic in :ref:`5.7.21-20`.

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-bytes-per-sync``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``0``

Specifies how often should the OS sync files to disk
as they are being written, asynchronously, in the background.
This operation can be used to smooth out write I/O over time.
Default value is ``0`` meaning that files are never synced.
Allowed range is up to ``18446744073709551615``.

.. _rocksdb_cache_dump:

.. rubric:: ``rocksdb_cache_dump``

Implemented in :ref:`5.7.30-33`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-cache-dump``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``ON``

Includes RocksDB block cache content in core dump. This variable is
enabled by default.

.. _rocksdb_cache_index_and_filter_blocks:

.. rubric:: ``rocksdb_cache_index_and_filter_blocks``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-cache-index-and-filter-blocks``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``ON``

Specifies whether RocksDB should use the block cache for caching the index
and bloomfilter data blocks from each data file.
Enabled by default.
If you disable this feature,
RocksDB will allocate additional memory to maintain these data blocks.

.. _rocksdb_checksums_pct:

.. rubric:: ``rocksdb_checksums_pct``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-checksums-pct``
   * - Dynamic
     - Yes
   * - Scope
     - Global, Session
   * - Data type
     - Numeric
   * - Default
     - ``100``

Specifies the percentage of rows to be checksummed.
Default value is ``100`` (checksum all rows).
Allowed range is from ``0`` to ``100``.

.. _rocksdb_collect_sst_properties:

.. rubric:: ``rocksdb_collect_sst_properties``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-collect-sst-properties``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``ON``

Specifies whether to collect statistics on each data file
to improve optimizer behavior.
Enabled by default.

.. _rocksdb_commit_in_the_middle:

.. rubric:: ``rocksdb_commit_in_the_middle``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-commit-in-the-middle``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Specifies whether to commit rows implicitly when a batch contains more than the value of :ref:`rocksdb_bulk_load_size`.

This variable is disabled by default. 

When the `rocksdb_bulk_load` variable is enabled, it behaves as if the variable `rocksdb_commit_in_the_middle` is enabled, even if the variable `rocksdb_commit_in_the_middle` is disabled.

.. _rocksdb_commit_time_batch_for_recovery:

.. rubric:: ``rocksdb_commit_time_batch_for_recovery``

Implemented in :ref:`5.7.23-23`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-commit-time-batch-for-recovery``
   * - Dynamic
     - Yes
   * - Scope
     - Global, Session
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Specifies whether to write the commit time write batch into the database or
not.

.. note:: If the commit time write batch is only useful for recovery, then
          writing to WAL is enough.

.. _rocksdb_compact_cf:

.. rubric:: ``rocksdb_compact_cf``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-compact-cf``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - String
   * - Default
     - 

Specifies the name of the column family to compact.

.. _rocksdb_compaction_readahead_size:

.. rubric:: ``rocksdb_compaction_readahead_size``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-compaction-readahead-size``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``0``

Specifies the size of reads to perform ahead of compaction.
Default value is ``0``.
Set this to at least 2 megabytes (``16777216``)
when using MyRocks with spinning disks
to ensure sequential reads instead of random.
Maximum allowed value is ``18446744073709551615``.

.. note:: If you set this variable to a non-zero value,
   :ref:`rocksdb_new_table_reader_for_compaction_inputs` is enabled.

.. _rocksdb_compaction_sequential_deletes:

.. rubric:: ``rocksdb_compaction_sequential_deletes``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-compaction-sequential-deletes``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``0``

Specifies the threshold to trigger compaction on a file
if it has more than this number of sequential delete markers.
Default value is ``0`` meaning that compaction is not triggered
regardless of the number of delete markers.
Maximum allowed value is ``2000000`` (two million delete markers).

.. note:: Depending on workload patterns,
   MyRocks can potentially maintain large numbers of delete markers,
   which increases latency of queries.
   This compaction feature will reduce latency,
   but may also increase the MyRocks write rate.
   Use this variable together with
   :ref:`rocksdb_compaction_sequential_deletes_file_size`
   to only perform compaction on large files.

.. _rocksdb_compaction_sequential_deletes_count_sd:

.. rubric:: ``rocksdb_compaction_sequential_deletes_count_sd``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-compaction-sequential-deletes-count-sd``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Specifies whether to count single deletes as delete markers
recognized by :ref:`rocksdb_compaction_sequential_deletes`.
Disabled by default.

.. _rocksdb_compaction_sequential_deletes_file_size:

.. rubric:: ``rocksdb_compaction_sequential_deletes_file_size``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-compaction-sequential-deletes-file-size``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``0``

Specifies the minimum file size required to trigger compaction on it
by :ref:`rocksdb_compaction_sequential_deletes`.
Default value is ``0``,
meaning that compaction is triggered regardless of file size.
Allowed range is from ``-1`` to ``9223372036854775807``.

.. _rocksdb_compaction_sequential_deletes_window:

.. rubric:: ``rocksdb_compaction_sequential_deletes_window``

Implemented in :ref:`5.7.19-17` 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-compaction-sequential-deletes-window``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``0``

Specifies the size of the window for counting delete markers
by :ref:`rocksdb_compaction_sequential_deletes`.
Default value is ``0``.
Allowed range is up to ``2000000`` (two million).

.. _rocksdb_concurrent_prepare:

.. rubric:: ``rocksdb_concurrent_prepare``

Implemented in :ref:`5.7.20-18`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-concurrent_prepare``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``ON``

When enabled this variable allows/encourages threads that are using
two-phase commit to ``prepare`` in parallel. Variable has been
deprecated in the *Percona Server for MySQL* 5.7.21-20, as it has been
renamed in upstream to :ref:`rocksdb_two_write_queues`.

.. _rocksdb_create_checkpoint:

.. rubric:: ``rocksdb_create_checkpoint``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-create-checkpoint``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - String
   * - Default
     - 

Specifies the directory where MyRocks should create a checkpoint.
Empty by default.

.. _rocksdb_create_if_missing:

.. rubric:: ``rocksdb_create_if_missing``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-create-if-missing``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``ON``

Specifies whether MyRocks should create its database if it does not exist.
Enabled by default.

.. _rocksdb_create_missing_column_families:

.. rubric:: ``rocksdb_create_missing_column_families``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-create-missing-column-families``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Specifies whether MyRocks should create new column families
if they do not exist.
Disabled by default.

.. _rocksdb_datadir:

.. rubric:: ``rocksdb_datadir``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-datadir``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - String
   * - Default
     - ``./.rocksdb``

Specifies the location of the MyRocks data directory.
By default, it is created in the current working directory.

.. _rocksdb_db_write_buffer_size:

.. rubric:: ``rocksdb_db_write_buffer_size``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-db-write-buffer-size``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``0``

Specifies the maximum size of all memtables used to store writes in MyRocks
across all column families. When this size is reached, the data is flushed
to persistent media.
The default value is ``0``.
The allowed range is up to ``18446744073709551615``.

.. _rocksdb_deadlock_detect:

.. rubric:: ``rocksdb_deadlock_detect``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-deadlock-detect``
   * - Dynamic
     - Yes
   * - Scope
     - Global, Session
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Specifies whether MyRocks should detect deadlocks.
Disabled by default.

.. _rocksdb_deadlock_detect_depth:

.. rubric:: ``rocksdb_deadlock_detect_depth``

Implemented in :ref:`5.7.20-18`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-deadlock-detect-depth``
   * - Dynamic
     - Yes
   * - Scope
     - Global, Session
   * - Data type
     - Numeric
   * - Default
     - ``50``

Specifies the number of transactions deadlock detection will traverse
through before assuming deadlock.

.. _rocksdb_debug_optimizer_no_zero_cardinality:

.. rubric:: ``rocksdb_debug_optimizer_no_zero_cardinality``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-debug-optimizer-no-zero-cardinality``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``ON``

Specifies whether MyRocks should prevent zero cardinality
by always overriding it with some value.

.. _rocksdb_debug_ttl_ignore_pk:

.. rubric:: ``rocksdb_debug_ttl_ignore_pk``

Implemented in :ref:`5.7.20-18`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-debug-ttl-ignore-pk``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

For debugging purposes only. If true, compaction filtering will not occur
on Primary Key TTL data. This variable is a no-op in non-debug builds.

.. _rocksdb_debug_ttl_read_filter_ts:

.. rubric:: ``rocksdb_debug_ttl_read_filter_ts``

Implemented in :ref:`5.7.20-18`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb_debug-ttl-read-filter-ts``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``0``

For debugging purposes only.  Overrides the TTL read
filtering time to time + :ref:`debug_ttl_read_filter_ts`.
A value of ``0`` denotes that the variable is not set.
This variable is a no-op in non-debug builds.

.. _rocksdb_debug_ttl_rec_ts:

.. rubric:: ``rocksdb_debug_ttl_rec_ts``

Implemented in :ref:`5.7.20-18` 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-debug-ttl-rec-ts``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``0``

For debugging purposes only.  Overrides the TTL of
records to ``now()`` + :ref:`debug_ttl_rec_ts`.
The value can be +/- to simulate a record inserted in the past vs a record
inserted in the "future". A value of ``0`` denotes that the
variable is not set.
This variable is a no-op in non-debug builds.

.. _rocksdb_debug_ttl_snapshot_ts:

.. rubric:: ``rocksdb_debug_ttl_snapshot_ts``

Implemented in :ref:`5.7.20-18`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb_debug_ttl_ignore_pk``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``0``

For debugging purposes only.  Sets the snapshot during
compaction to ``now()`` + :ref:`rocksdb_debug_set_ttl_snapshot_ts`.
The value can be +/- to simulate a snapshot in the past vs a
snapshot created in the "future". A value of ``0`` denotes
that the variable is not set. This variable is a no-op in
non-debug builds.

.. _rocksdb_default_cf_options:

.. rubric:: ``rocksdb_default_cf_options``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-default-cf-options``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - String
   * - Default
     - 

Specifies the default column family options for MyRocks.
Empty by default.

.. _rocksdb_delayed_write_rate:

.. rubric:: ``rocksdb_delayed_write_rate``

Implemented in :ref:`5.7.19-17` 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-delayed-write-rate``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``16777216``

Specifies the write rate in bytes per second, which should be used
if MyRocks hits a soft limit or threshold for writes.
Default value is ``16777216`` (16 MB/sec).
Allowed range is from ``0`` to ``18446744073709551615``.

.. _rocksdb_delete_cf:

.. rubric:: ``rocksdb_delete_cf``

Implemented in :ref:`5.7.30-33`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-delete-cf``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - String
   * - Default
     - ""

Deletes the column family by name. The default value is "", an empty 
string.

For example: ::

    SET @@global.ROCKSDB_DELETE_CF = 'cf_primary_key';
 
.. _rocksdb_delete_obsolete_files_period_micros:

.. rubric:: ``rocksdb_delete_obsolete_files_period_micros``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-delete-obsolete-files-period-micros``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``21600000000``   

Specifies the period in microseconds to delete obsolete files
regardless of files removed during compaction.
Default value is ``21600000000`` (6 hours).
Allowed range is up to ``9223372036854775807``.

.. _rocksdb_enable_bulk_load_api:

.. rubric:: ``rocksdb_enable_bulk_load_api``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-enable-bulk-load-api``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``ON``

Specifies whether to use the ``SSTFileWriter`` feature for bulk loading,
This feature bypasses the memtable,
but requires keys to be inserted into the table
in either ascending or descending order.
Enabled by default.
If disabled, bulk loading uses the normal write path via the memtable
and does not require keys to be inserted in any order.

.. _rocksdb_enable_insert_with_update_caching:

.. rubric:: ``rocksdb_enable_insert_with_update_caching``

Implemented in :ref:`5.7.30-33`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-enable-insert-with-update-caching``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``ON``

Specifies whether to enable optimization where the read is cached from a 
failed insertion attempt in INSERT ON DUPLICATE KEY UPDATE.

.. _rocksdb_enable_iterate_bounds:

.. rubric:: ``rocksdb_enable_iterate_bounds``

Implemented in :ref:`5.7.30-33`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-enable-iterate-bounds``
   * - Dynamic
     - Yes
   * - Scope
     - Global, Local
   * - Data type
     - Boolean
   * - Default
     - ``TRUE``

Enables the rocksdb iterator upper bounds and lower bounds in read options.

The default value is ``TRUE``.

.. _rocksdb_enable_native_partition:

.. rubric:: ``rocksdb_enable_native_partition``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-enable-native-partition``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

This variable is **experimental** and should not be used in production.

This variable enables native partitioning and may be used when upgrading to 8.0.

.. _rocksdb_enable_pipelined_write:

.. rubric:: ``rocksdb_enable_pipelined_write``

Implemented in :ref:`5.7.35-38`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-enable-pipelined-write``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

DBOptions::enable_pipelined_write for RocksDB.

If ``enable_pipelined_write`` is ``true``, a separate write thread is maintained for WAL write and memtable write. A write thread first enters the WAL writer queue and then the memtable writer queue. A pending thread on the WAL writer queue only waits for the previous WAL write operations but does not wait for memtable write operations. Enabling the feature may improve write throughput and reduce latency of the prepare phase of a two-phase commit.

.. _rocksdb_enable_remove_orphaned_dropped_cfs:

.. rubric:: ``rocksdb_enable_remove_orphaned_dropped_cfs``

Implemented in :ref:`5.7.30-33`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-enable-remove-orphaned-dropped-cfs``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``TRUE``

Enables the removal of dropped column families (cfs) from metadata if the cfs do 
not exist in the cf manager.

The default value is ``TRUE``.

.. _rocksdb_enable_ttl:

.. rubric:: ``rocksdb_enable_ttl``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-enable-ttl``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``ON``

Specifies whether to keep expired TTL records during compaction.
Enabled by default.
If disabled, expired TTL records will be dropped during compaction.

.. _rocksdb_enable_ttl_read_filtering:

.. rubric:: ``rocksdb_enable_ttl_read_filtering``

Implemented in :ref:`5.7.20-18`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-enable-ttl-read-filtering``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``ON``

For tables with TTL, expired records are skipped/filtered
out during processing and in query results. Disabling
this will allow these records to be seen, but as a result
rows may disappear in the middle of transactions as they
are dropped during compaction. **Use with caution.**

.. _rocksdb_enable_thread_tracking:

.. rubric:: ``rocksdb_enable_thread_tracking``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-enable-thread-tracking``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Specifies whether to enable tracking the status of threads
accessing the database.
Disabled by default.
If enabled, thread status will be available via ``GetThreadList()``.

.. _rocksdb_enable_write_thread_adaptive_yield:

.. rubric:: ``rocksdb_enable_write_thread_adaptive_yield``

Implemented in :ref:`5.7.19-17` 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-enable-write-thread-adaptive-yield``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Specifies whether the MyRocks write batch group leader
should wait up to the maximum allowed time
before blocking on a mutex.
Disabled by default.
Enable it to increase throughput for concurrent workloads.

.. _rocksdb_error_if_exists:

.. rubric:: ``rocksdb_error_if_exists``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-error-if-exists``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Specifies whether to report an error when a database already exists.
Disabled by default.

.. _rocksdb_error_on_suboptimal_collation:

.. rubric:: ``rocksdb_error_on_suboptimal_collation``

Implemented in :ref:`5.7.23-23`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-error-on-suboptimal-collation``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``ON``

Specifies whether to report an error instead of a warning if an index is
created on a char field where the table has a sub-optimal collation (case
insensitive). Enabled by default.

.. _rocksdb_flush_log_at_trx_commit:

.. rubric:: ``rocksdb_flush_log_at_trx_commit``

Implemented in :ref:`5.7.19-17` 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-flush-log-at-trx-commit``
   * - Dynamic
     - Yes
   * - Scope
     - Global, Session
   * - Data type
     - Numeric
   * - Default
     - ``1``

Specifies whether to sync on every transaction commit,
similar to `innodb_flush_log_at_trx_commit <https://dev.mysql.com/doc/refman/5.7/en/innodb-parameters.html#sysvar_innodb_flush_log_at_trx_commit>`__.
Enabled by default, which ensures ACID compliance.

Possible values:

* ``0``: Do not sync on transaction commit.
  This provides better performance, but may lead to data inconsistency
  in case of a crash.

* ``1``: Sync on every transaction commit.
  This is set by default and recommended
  as it ensures data consistency,
  but reduces performance.

* ``2``: Sync every second.

.. _rocksdb_flush_memtable_on_analyze:

.. rubric:: ``rocksdb_flush_memtable_on_analyze``

Implemented in :ref:`5.7.19-17`.

Removed in :ref:`5.7.21-20`.

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-flush-memtable-on-analyze``
   * - Dynamic
     - Yes
   * - Scope
     - Global, Session
   * - Data type
     - Boolean
   * - Default
     - ``ON``

Specifies whether to flush the memtable when running ``ANALYZE`` on a table.
Enabled by default.
This ensures accurate cardinality
by including data in the memtable for calculating stats.

.. _rocksdb_force_compute_memtable_stats:

.. rubric:: ``rocksdb_force_compute_memtable_stats``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-force-compute-memtable-stats``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``ON``

Specifies whether data in the memtables should be included
for calculating index statistics
used by the query optimizer.
Enabled by default.
This provides better accuracy, but may reduce performance.

.. _rocksdb_force_compute_memtable_stats_cachetime:

.. rubric:: ``rocksdb_force_compute_memtable_stats_cachetime``

Implemented in :ref:`5.7.20-18`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-force-compute-memtable-stats-cachetime``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``60000000``

Specifies for how long the cached value of memtable statistics should
be used instead of computing it every time during the query plan analysis.

.. _rocksdb_force_flush_memtable_and_lzero_now:

.. rubric:: ``rocksdb_force_flush_memtable_and_lzero_now``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-force-flush-memtable-and-lzero-now``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Works similar to :ref:`force_flush_memtable_now`
but also flushes all L0 files.

.. _rocksdb_force_flush_memtable_now:

.. rubric:: ``rocksdb_force_flush_memtable_now``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-force-flush-memtable-now``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Forces MyRocks to immediately flush all memtables out to data files.

.. warning:: Use with caution!
   Write requests will be blocked until all memtables are flushed.

.. _rocksdb_force_index_records_in_range:

.. rubric:: ``rocksdb_force_index_records_in_range``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-force-index-records-in-range``
   * - Dynamic
     - Yes
   * - Scope
     - Global, Session
   * - Data type
     - Numeric
   * - Default
     - ``1``

Specifies the value used to override the number of rows
returned to query optimizer when ``FORCE INDEX`` is used.
Default value is ``1``.
Allowed range is from ``0`` to ``2147483647``.
Set to ``0`` if you do not want to override the returned value.

.. _rocksdb_hash_index_allow_collision:

.. rubric:: ``rocksdb_hash_index_allow_collision``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-hash-index-allow-collision``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``ON``

Specifies whether hash collisions are allowed.
Enabled by default, which uses less memory.
If disabled, full prefix is stored to prevent hash collisions.

.. _rocksdb_ignore_unknown_options:

.. rubric:: ``rocksdb_ignore_unknown_options``

Implemented in :ref:`5.7.20-18`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ````
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``ON``

When enabled, it allows RocksDB to receive unknown options and not exit.

.. _rocksdb_index_type:

.. rubric:: ``rocksdb_index_type``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-index-type``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Enum
   * - Default
     - ``kBinarySearch``

Specifies the type of indexing used by MyRocks:

* ``kBinarySearch``: Binary search (default).

* ``kHashSearch``: Hash search.

.. _rocksdb_info_log_level:

.. rubric:: ``rocksdb_info_log_level``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-info-log-level``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Enum
   * - Default
     - ``error_level``

Specifies the level for filtering messages written by MyRocks
to the ``mysqld`` log.

* ``debug_level``: Maximum logging (everything including debugging
  log messages)
* ``info_level``
* ``warn_level``
* ``error_level`` (default)
* ``fatal_level``: Minimum logging (only fatal error messages logged)

.. _rocksdb_is_fd_close_on_exec:

.. rubric:: ``rocksdb_is_fd_close_on_exec``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-is-fd-close-on-exec``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``ON``

Specifies whether child processes should inherit open file jandles.
Enabled by default.

.. _rocksdb_large_prefix:

.. rubric:: ``rocksdb_large_prefix``

Implemented in :ref:`5.7.20-18` 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-large-prefix``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

When enabled, this option allows index key prefixes longer than 767 bytes
(up to 3072 bytes). This option mirrors the `innodb_large_prefix
<https://dev.mysql.com/doc/refman/5.7/en/innodb-parameters.html#sysvar_innodb_large_prefix>`_
The values for :ref:`rocksdb_large_prefix` should be the same between
source and replica.

.. _rocksdb_keep_log_file_num:

.. rubric:: ``rocksdb_keep_log_file_num``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-keep-log-file-num``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``1000``

Specifies the maximum number of info log files to keep.
Default value is ``1000``.
Allowed range is from ``1`` to ``18446744073709551615``.

.. _rocksdb_lock_scanned_rows:

.. rubric:: ``rocksdb_lock_scanned_rows``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-lock-scanned-rows``
   * - Dynamic
     - Yes
   * - Scope
     - Global, Session
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Specifies whether to hold the lock on rows that are scanned during ``UPDATE``
and not actually updated.
Disabled by default.

.. _rocksdb_lock_wait_timeout:

.. rubric:: ``rocksdb_lock_wait_timeout``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-lock-wait-timeout``
   * - Dynamic
     - Yes
   * - Scope
     - Global, Session
   * - Data type
     - Numeric
   * - Default
     - ``1``

Specifies the number of seconds MyRocks should wait to acquire a row lock
before aborting the request.
Default value is ``1``.
Allowed range is up to ``1073741824``.

.. _rocksdb_log_file_time_to_roll:

.. rubric:: ``rocksdb_log_file_time_to_roll``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-log-file-time-to-roll``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``0``

Specifies the period (in seconds) for rotating the info log files.
Default value is ``0``, meaning that the log file is not rotated.
Allowed range is up to ``18446744073709551615``.

.. _rocksdb_manifest_preallocation_size:

.. rubric:: ``rocksdb_manifest_preallocation_size``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-manifest-preallocation-size``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``0``

Specifies the number of bytes to preallocate for the MANIFEST file
used by MyRocks to store information
about column families, levels, active files, etc.
Default value is ``0``.
Allowed range is up to ``18446744073709551615``.

.. note:: A value of ``4194304`` (4 MB) is reasonable
   to reduce random I/O on XFS.

.. _rocksdb_manual_compaction_bottommost_level:

.. rubric:: ``rocksdb_manual_compaction_bottommost_level``

Implemented in :ref:`5.7.35-38`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-manual-compaction-bottommost-level``
   * - Dynamic
     - Yes
   * - Scope
     - Global, Session
   * - Data type
     - Enum
   * - Default
     - ``kForceOptimized``

Option for skipping bottommost level compaction during manual compaction. The values are the following:

* ``kSkip`` - Skip bottommost level compaction

* ``kIfHaveCompactionFilter`` - Only compact the bottommost level if there is a compaction filter

* ``kForce`` - Always compact the bottommost level

* ``kForceOptimized`` - The default value. Always compact the bottommost level but in the bottommost level avoid double-compacting files created

.. _rocksdb_manual_wal_flush:

.. rubric:: ``rocksdb_manual_wal_flush``

Implemented in :ref:`5.7.20-18`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-manual-wal-flush``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``ON``

This variable can be used to disable automatic/timed WAL flushing and instead
rely on the application to do the flushing.

.. _rocksdb_master_skip_tx_api:

.. rubric:: ``rocksdb_master_skip_tx_api``

Implemented in :ref:`5.7.30-33`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ````
   * - Dynamic
     - Yes
   * - Scope
     - Global, Session
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

When enabled, uses the WriteBatch API, which is faster. The session does not
hold any lock on row access. This variable is not effective on replica.

.. note::

    Due to the disabled row locks, improper use of the variable can cause data 
    corruption or inconsistency.

.. _rocksdb_max_background_compactions:

.. rubric:: ``rocksdb_max_background_compactions``

Implemented in :ref:`5.7.19-17`. 

Replaced by :ref:`rocksdb_max_background_jobs` in :ref:`5.7.20-18`.

Re-implemented in :ref:`5.7.31-34`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-max-background-compactions``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``-1``

Sets DBOptions:: max_background_compactions for RocksDB.
Default value is ``-1``. Allowed range is up to ``64``.
This variable has been replaced in *Percona Server for MySQL* :ref:`5.7.20-18`
by :ref:`rocksdb_max_background_jobs`, which automatically decides how
many threads to allocate towards flush/compaction.
This variable has been re-implemented in *Percona Server for MySQL* 5.7.31-34.

.. _rocksdb_max_background_flushes:

.. rubric:: ``rocksdb_max_background_flushes``

Implemented in :ref:`5.7.19-17`.

Replaced by :ref:`rocksdb_max_background_jobs` in :ref:`5.7.20-18`.

Re-implemented in :ref:`5.7.31-34`.  

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-max-background-flushes``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``-1``

Sets DBOptions:: max_background_flushes for RocksDB.
Default value is ``-1``. Allowed range is up to ``64``.
This variable has been replaced in *Percona Server for MySQL* :ref:`5.7.20-18`
by :ref:`rocksdb_max_background_jobs`, which automatically decides how
many threads to allocate towards flush/compaction.
This variable has been re-implemented in *Percona Server for MySQL* 5.7.31-34.

.. _rocksdb_max_background_jobs:

.. rubric:: ``rocksdb_max_background_jobs``

Implemented in :ref:`5.7.20-18`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-max-background-jobs``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``2``

This variable has been introduced in *Percona Server for MySQL* :ref:`5.7.20-18`
to replace :ref:`rocksdb_base_background_compactions`,
:ref:`rocksdb_max_background_compactions`, and
:ref:`rocksdb_max_background_flushes` variables. This variable specifies
the maximum number of background jobs. It automatically decides
how many threads to allocate towards flush/compaction. It was implemented to
reduce the number of (confusing) options users and can tweak and push the
responsibility down to RocksDB level.

.. _rocksdb_max_bottom_pri_background_compactions:

.. rubric:: ``rocksdb_max_bottom_pri_background_compactions``

Implemented in :ref:`5.7.31-34`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb_max_bottom_pri_background_compactions``
   * - Dynamic
     - No
   * - Data type
     - Unsigned integer
   * - Default
     - ``0``

Creates a specified number of threads, sets a lower CPU priority, and letting compactions use them. The maximum compaction concurrency is capped by ``rocksdb_max_background_compactions`` or ``rocksdb_max_background_jobs``

The minimum value is ``0`` and the maximum value is ``64``.

.. _rocksdb_max_latest_deadlocks:

.. rubric:: ``rocksdb_max_latest_deadlocks``

Implemented in :ref:`5.7.20-18`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-max-latest-deadlocks``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``5``

Specifies the maximum number of recent deadlocks to store.

.. _rocksdb_max_log_file_size:

.. rubric:: ``rocksdb_max_log_file_size``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-max-log-file-size``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``0``

Specifies the maximum size for info log files,
after which the log is rotated.
Default value is ``0``, meaning that only one log file is used.
Allowed range is up to ``18446744073709551615``.

Also see :ref:`rocksdb_log_file_time_to_roll`.

.. _rocksdb_max_manifest_file_size:

.. rubric:: ``rocksdb_max_manifest_file_size``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-manifest-log-file-size``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``18446744073709551615``

Specifies the maximum size of the MANIFEST data file,
after which it is rotated.
Default value is also the maximum, making it practically unlimited:
only one manifest file is used.

.. _rocksdb_max_open_files:

.. rubric:: ``rocksdb_max_open_files``

Implemented in :ref:`5.7.19-17`.

Default value changed to ``1000`` in :ref:`5.7.19-17`.

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-max-open-files``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``1000``

Specifies the maximum number of file handles opened by MyRocks.
Values in the range between ``0`` and ``open_files_limit``
are taken as they are. If :ref:`rocksdb_max_open_files` value is
greater than ``open_files_limit``, it will be reset to 1/2 of
``open_files_limit``, and a warning will be emitted to the ``mysqld``
error log. A value of ``-2`` denotes auto tuning: just sets
:ref:`rocksdb_max_open_files` value to 1/2 of ``open_files_limit``.
Finally, ``-1`` means no limit, i.e. an infinite number of file handles.

.. warning::

  Setting :ref:`rocksdb_max_open_files` to ``-1`` is dangerous,
  as server may quickly run out of file handles in this case.

.. _rocksdb_max_row_locks:

.. rubric:: ``rocksdb_max_row_locks``

Implemented in :ref:`5.7.19-17`. 

Default value has changed from ``1073741824`` to ``1048576`` in :ref:`5.7.21-21`. 

Scope changed to ``Global`` in :ref:`5.7.32-35`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-max-row-locks``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``1048576``


Specifies the limit on the maximum number of row locks a transaction can have
before it fails.
Default value is also the maximum, making it practically unlimited:
transactions never fail due to row locks.

.. _rocksdb_max_subcompactions:

.. rubric:: ``rocksdb_max_subcompactions``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-max-subcompactions``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``1``

Specifies the maximum number of threads allowed for each compaction job.
Default value of ``1`` means no subcompactions (one thread per compaction job).
Allowed range is up to ``64``.

.. _rocksdb_max_total_wal_size:

.. rubric:: ``rocksdb_max_total_wal_size``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-max-total-wal-size``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``0``

Specifies the maximum total size of WAL (write-ahead log) files,
after which memtables are flushed.
Default value is ``0``: WAL size limit is chosen dynamically.
Allowed range is up to ``9223372036854775807``.

.. _rocksdb_merge_buf_size:

.. rubric:: ``rocksdb_merge_buf_size``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-merge-buf-size``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``67108864``

Specifies the size (in bytes) of the merge-sort buffers
used to accumulate data during secondary key creation.
New entries are written directly to the lowest level in the database,
instead of updating indexes through the memtable and L0.
These values are sorted using merge-sort,
with buffers set to 64 MB by default (``67108864``).
Allowed range is from ``100`` to ``18446744073709551615``.

.. _rocksdb_merge_combine_read_size:

.. rubric:: ``rocksdb_merge_combine_read_size``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-merge-combine-read-size``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``1073741824``

Specifies the size (in bytes) of the merge-combine buffer
used for the merge-sort algorithm
as described in :ref:`rocksdb_merge_buf_size`.
Default size is 1 GB (``1073741824``).
Allowed range is from ``100`` to ``18446744073709551615``.

.. _rocksdb_merge_tmp_file_removal_delay_ms:

.. rubric:: ``rocksdb_merge_tmp_file_removal_delay_ms``

Implemented in :ref:`5.7.20-18`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb_merge_tmp_file_removal_delay_ms``
   * - Dynamic
     - Yes
   * - Scope
     - Global, Session
   * - Data type
     - Numeric
   * - Default
     - ``0``

Fast secondary index creation creates merge files when needed. After finishing
secondary index creation, merge files are removed. By default, the file removal
is done without any sleep, so removing GBs of merge files within <1s may
happen, which will cause trim stalls on Flash. This variable can be used to
rate limit the delay in milliseconds.

.. _rocksdb_new_table_reader_for_compaction_inputs:

.. rubric:: ``rocksdb_new_table_reader_for_compaction_inputs``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-new-table-reader-for-compaction-inputs``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Specifies whether MyRocks should create a new file descriptor and table reader
for each compaction input.
Disabled by default.
Enabling this may increase memory consumption,
but will also allow pre-fetch options to be specified for compaction
input files without impacting table readers used for user queries.

.. _rocksdb_no_block_cache:

.. rubric:: ``rocksdb_no_block_cache``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-no-block-cache``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Specifies whether to disable the block cache for column families.
Variable is disabled by default,
meaning that using the block cache is allowed.

.. _rocksdb_no_create_column_family:

.. rubric:: ``rocksdb_no_create_column_family``

Implemented in :ref:`5.7.23-24`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-no-create-column-family``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Specifies whether column families can created implicitly via an index comment.
If this variable is set to ``ON``, then column families must already exist or
must be present within the :ref:`rocksdb_override_cf_options` for a user to
assign and index to a column family.

.. _rocksdb_override_cf_options:

.. rubric:: ``rocksdb_override_cf_options``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-override-cf-options``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - String
   * - Default
     - 

Specifies option overrides for each column family.
Empty by default.

.. _rocksdb_paranoid_checks:

.. rubric:: ``rocksdb_paranoid_checks``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-paranoid-checks``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``ON``

Specifies whether MyRocks should re-read the data file
as soon as it is created to verify correctness.
Enabled by default.

.. _rocksdb_pause_background_work:

.. rubric:: ``rocksdb_pause_background_work``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-pause-background-work``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Specifies whether MyRocks should pause all background operations.
Disabled by default. There is no practical reason for a user to ever
use this variable because it is intended as a test synchronization tool
for the MyRocks MTR test suites.

.. warning::

  If someone were to set a :ref:`rocksdb_force_flush_memtable_now` to
  ``1`` while :ref:`rocksdb_pause_background_work` is set to ``1``,
  the client that issued the ``rocksdb_force_flush_memtable_now=1`` will be
  blocked indefinitely until :ref:`rocksdb_pause_background_work`
  is set to ``0``.

.. _rocksdb_perf_context_level:

.. rubric:: ``rocksdb_perf_context_level``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-perf-context-level``
   * - Dynamic
     - Yes
   * - Scope
     - Global, Session
   * - Data type
     - Numeric
   * - Default
     - ``0``

Specifies the level of information to capture with the Perf Context plugins.
Default value is ``0``.
Allowed range is up to ``5``.

.. list-table::
   :header-rows: 1

   * - Value
     - Description
   * - 0
     - Unknown setting
   * - 1
     - Disable perf stats
   * - 2 
     - Enable only count stats
   * - 3
     - Enable count stats and time stats except for mutexes
   * - 4
     - Enable count stats, time stats, except for wall time or CPU time for mutexes
   * - 5 
     - Enable count and time stats

.. _rocksdb_persistent_cache_path:

.. rubric:: ``rocksdb_persistent_cache_path``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-persistent-cache-path``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - String
   * - Default
     - 

Specifies the path to the persistent cache.
Set this together with :ref:`rocksdb_persistent_cache_size_mb`.

.. _rocksdb_persistent_cache_size_mb:

.. rubric:: ``rocksdb_persistent_cache_size_mb``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-persistent-cache-size-mb``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``0``

Specifies the size of the persisten cache in megabytes.
Default is ``0`` (persistent cache disabled).
Allowed range is up to ``18446744073709551615``.
Set this together with :ref:`rocksdb_persistent_cache_path`.

.. _rocksdb_pin_l0_filter_and_index_blocks_in_cache:

.. rubric:: ``rocksdb_pin_l0_filter_and_index_blocks_in_cache``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-pin-l0-filter-and-index-blocks-in-cache``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``ON``

Specifies whether MyRocks pins the filter and index blocks in the cache
if :ref:`rocksdb_cache_index_and_filter_blocks` is enabled.
Enabled by default.

.. _rocksdb_print_snapshot_conflict_queries:

.. rubric:: ``rocksdb_print_snapshot_conflict_queries``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-print-snapshot-conflict-queries``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Specifies whether queries that generate snapshot conflicts
should be logged to the error log.
Disabled by default.

.. _rocksdb_rate_limiter_bytes_per_sec:

.. rubric:: ``rocksdb_rate_limiter_bytes_per_sec``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-rate-limiter-bytes-per-sec``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``0``

Specifies the maximum rate at which MyRocks can write to media
via memtable flushes and compaction.
Default value is ``0`` (write rate is not limited).
Allowed range is up to ``9223372036854775807``.

.. _rocksdb_read_free_rpl:

.. rubric:: ``rocksdb_read_free_rpl``

Implemented in :ref:`5.7.30-33`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-read-free-rpl``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Enum
   * - Default
     - ``OFF``

Use read-free replication, which allows no row lookup during
replication, on the replica.

The options are the following:

* OFF - Disables the variable
* PK_SK - Enables the variable on all tables with a primary key
* PK_ONLY - Enables the variable on tables where the only key is the primary key
   
.. _rocksdb_read_free_rpl_tables:

.. rubric:: ``rocksdb_read_free_rpl_tables``

Disabled in :ref:`5.7.30-33`. 

Implemented in :ref:`5.7.19-17`.

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-read-free-rpl-tables``
   * - Dynamic
     - Yes
   * - Scope
     - Global, Session
   * - Data type
     - String
   * - Default
     -       

Lists tables (as a regular expression)
that should use read-free replication on the replica
(that is, replication without row lookups).
Empty by default.

This variable is disabled in *Percona Server for MySQL* 5.7.30-33. We recommend
that you use ``rocksdb_read_free_rpl`` instead of this variable.

.. _rocksdb_records_in_range:

.. rubric:: ``rocksdb_records_in_range``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-records-in-range``
   * - Dynamic
     - Yes
   * - Scope
     - Global, Session
   * - Data type
     - Numeric
   * - Default
     - ``0``

Specifies the value to override the result of ``records_in_range()``.
Default value is ``0``.
Allowed range is up to ``2147483647``.

.. _rocksdb_reset_stats:

.. rubric:: ``rocksdb_reset_stats``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-reset-stats``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Resets MyRocks internal statistics dynamically
(without restarting the server).

.. _rocksdb_rollback_on_timeout:

.. rubric:: ``rocksdb_rollback_on_timeout``

Implemented in :ref:`5.7.30-33`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-rollback-on-timeout``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

By default, only the last statement on a transaction is rolled back. If
``--rocksdb-rollback-on-timeout=ON``, a transaction timeout causes a rollback of
the entire transaction.

.. _rocksdb_rpl_skip_tx_api:

.. rubric:: ``rocksdb_rpl_skip_tx_api``

Re-implemented in :ref:`5.7.21-21`.

Variable removed in :ref:`5.7.20-19`. 

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-rpl-skip-tx-api``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Specifies whether write batches should be used for replication thread
instead of the transaction API.
Disabled by default.

There are two conditions which are necessary to
use it: row replication format and replica
operating in super read only mode.

.. _rocksdb_seconds_between_stat_computes:

.. rubric:: ``rocksdb_seconds_between_stat_computes``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-seconds-between-stat-computes``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``3600``

Specifies the number of seconds to wait
between recomputation of table statistics for the optimizer.
During that time, only changed indexes are updated.
Default value is ``3600``.
Allowed is from ``0`` to ``4294967295``.

.. _rocksdb_signal_drop_index_thread:

.. rubric:: ``rocksdb_signal_drop_index_thread``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-signal-drop-index-thread``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Signals the MyRocks drop index thread to wake up.

.. _rocksdb_sim_cache_size:

.. rubric:: ``rocksdb_sim_cache_size``

Implemented in :ref:`5.7.20-18`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-sim-cache-size``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``0``

Enables the simulated cache, which allows us to figure out the hit/miss rate
with a specific cache size without changing the real block cache.

.. _rocksdb_skip_bloom_filter_on_read:

.. rubric:: ``rocksdb_skip_bloom_filter_on_read``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-skip-bloom-filter-on_read``
   * - Dynamic
     - Yes
   * - Scope
     - Global, Session
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Specifies whether bloom filters should be skipped on reads.
Disabled by default (bloom filters are not skipped).

.. _rocksdb_skip_fill_cache:

.. rubric:: ``rocksdb_skip_fill_cache``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-skip-fill-cache``
   * - Dynamic
     - Yes
   * - Scope
     - Global, Session
   * - Data type
     - Boolean
   * - Default
     - ``OFF```

Specifies whether to skip caching data on read requests.
Disabled by default (caching is not skipped).

.. _rocksdb_skip_locks_if_skip_unique_check:

.. rubric:: ``rocksdb_skip_locks_if_skip_unique_check``

Implemented in :ref:`5.7.35-38`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``rocksdb_skip_locks_if_skip_unique_check``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - OFF

Skips row locking when unique checks are disabled.

.. _rocksdb_sst_mgr_rate_bytes_per_sec:

.. rubric:: ``rocksdb_sst_mgr_rate_bytes_per_sec``

Default value changed from ``67108864`` to ``0`` in :ref:`5.7.20-18`. 

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-sst-mgr-rate-bytes-per-sec``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``0``

Specifies the maximum rate for writing to data files.
Default value is ``0``. This option is not effective on HDD.
Allowed range is from ``0`` to ``18446744073709551615``.

.. _rocksdb_stats_dump_period_sec:

.. rubric:: ``rocksdb_stats_dump_period_sec``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-stats-dump-period-sec``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``600``

Specifies the period in seconds for performing a dump of the MyRocks statistics
to the info log.
Default value is ``600``.
Allowed range is up to ``2147483647``.

.. _rocksdb_stats_level:

.. rubric:: ``rocksdb_stats_level``

Implemented in :ref:`5.7.30-33`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-stats-level``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``0``

Controls the RocksDB statistics level. The default value is "0" (kExceptHistogramOrTimers),
 which is the fastest level. The maximum value is "4".

.. _rocksdb_stats_recalc_rate:

.. rubric:: ``rocksdb_stats_recalc_rate``

Implemented in :ref:`5.7.23-23`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-stats-recalc-rate``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``0``

Specifies the number of indexes to recalculate per second. Recalculating index
statistics periodically ensures it to match the actual sum from SST files.
Default value is ``0``. Allowed range is up to ``4294967295``.

.. _rocksdb_store_row_debug_checksums:

.. rubric:: ``rocksdb_store_row_debug_checksums``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-store-row-debug-checksums``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Specifies whether to include checksums when writing index or table records.
Disabled by default.

.. _rocksdb_strict_collation_check:

.. rubric:: ``rocksdb_strict_collation_check``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-strict-collation-check``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``ON``

Specifies whether to check and verify
that table indexes have proper collation settings.
Enabled by default.

.. _rocksdb_strict_collation_exceptions:

.. rubric:: ``rocksdb_strict_collation_exceptions``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-strict-collation-exceptions``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - String
   * - Default
     - 

Lists tables (as a regular expression) that should be excluded
from verifying case-sensitive collation
enforced by :ref:`rocksdb_strict_collation_check`.
Empty by default.

.. _rocksdb_table_cache_numshardbits:

.. rubric:: ``rocksdb_table_cache_numshardbits``

Max value changed from ``2147483647`` to ``19`` in :ref:`5.7.20-18`.

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-table-cache-numshardbits``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``6``

Specifies the number if table caches.
Default value is ``6``.
Allowed range is from ``0`` to ``19``.

.. _rocksdb_table_stats_background_thread_nice_value:

.. rubric:: ``rocksdb_table_stats_background_thread_nice_value``

Implemented in :ref:`5.7.30-33`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-table-stats-background-thread-nice-value``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``19``
   
The nice value for index stats.
The minimum = -20 (THREAD_PRIO_MIN)
The maximum = 19 (THREAD_PRIO_MAX)

.. _rocksdb_table_stats_max_num_rows_scanned:

.. rubric:: ``rocksdb_table_stats_max_num_rows_scanned``

Implemented in :ref:`5.7.30-33`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-table-stats-max-num-rows-scanned``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``0``
 
The maximum number of rows to scan in a table scan based on
a cardinality calculation.
The minimum is ``0`` (every modification triggers a stats recalculation).
The maximum is ``18,446,744,073,709,551,615``.

.. _rocksdb_table_stats_recalc_threshold_count:

.. rubric:: ``rocksdb_table_stats_recalc_threshold_count``

Implemented in :ref:`5.7.30-33`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-table-stats-recalc-threshold-count``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``100``

The number of modified rows to trigger a stats recalculation. This is a
dependent variable for stats recalculation. 
The minimum is ``0``.
The maximum is ``18,446,744,073,709,551,615``.

.. _rocksdb_table_stats_recalc_threshold_pct:

.. rubric:: ``rocksdb_table_stats_recalc_threshold_pct``

Implemented in :ref:`5.7.30-33`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-table-stats-recalc-threshold-pct``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``10``

The percentage of the number of modified rows over the total number of rows
to trigger stats recalculations. This is a dependent variable for stats
recalculation.
The minimum value is ``0`` 
The maximum value is ``100`` (RDB_TBL_STATS_RECALC_THRESHOLD_PCT_MAX).

.. _rocksdb_table_stats_sampling_pct:

.. rubric:: ``rocksdb_table_stats_sampling_pct``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-table-stats-sampling-pct``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``10``

Specifies the percentage of entries to sample
when collecting statistics about table properties.
Default value is ``10``.
Allowed range is from ``0`` to ``100``.

.. _rocksdb_table_stats_use_table_scan:

.. rubric:: ``rocksdb_table_stats_use_table_scan``

Implemented in :ref:`5.7.30-33`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-table-stats-use-table-scan``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``FALSE``

Enables table-scan-based index calculations. 
The default value is ``FALSE``.

.. _rocksdb_tmpdir:

.. rubric:: ``rocksdb_tmpdir``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-tmpdir``
   * - Dynamic
     - Yes
   * - Scope
     - Global, Session
   * - Data type
     - String
   * - Default
     - 

Specifies the path to the directory for temporary files during DDL operations.

.. _rocksdb_trace_block_cache_access:

.. rubric:: ``rocksdb_trace_block_cache_access``

Implemented in :ref:`5.7.30-33`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-trace-block-cache-access``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - String
   * - Default
     - ``""``

Defines the block cache trace option string. The format is 
"sampling frequency: max_trace_file_size:trace_file_name." The
sampling frequency value and max_trace_file_size value 
are positive integers. The block accesses are saved to 
the ``rocksdb_datadir/block_cache_traces/trace_file_name``.
The default value is ``""``, an empty string.

.. _rocksdb_trace_queries:

.. rubric:: ``rocksdb_trace_queries``

Implemented in :ref:`5.7.35-38`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-trace-queries``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - String
   * - Default
     - ""

This variable is a trace option string. The format is sampling_frequency:max_trace_file_size:trace_file_name. The sampling_frequency value and max_trace_file_size value are positive integers. The queries are saved to the rocksdb_datadir/queries_traces/trace_file_name.

The file size unit is measured in bytes.

The sampling frequency specifies that one request is sampled from ``sampling_frequency`` requests. If the request is ``1``, all the requests are traced. If the request is ``5``, then for every five requests, one request is traced.

.. _rocksdb_trace_sst_api:

.. rubric:: ``rocksdb_trace_sst_api``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-trace-sst-api``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Specifies whether to generate trace output in the log
for each call to ``SstFileWriter``.
Disabled by default.

.. _rocksdb_track_and_verify_wals_in_manifest:

.. rubric:: ``rocksdb_track_and_verify_wals_in_manifest``

Implemented in :ref:`5.7.35-38`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-track-and-verify-wals-in-manifest``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``ON``

DBOptions::track_and_verify_wals_in_manifest for RocksDB

If true, the log numbers and sizes of the synced WALs are tracked in Manifest, then, during a DB recovery, if a synced WAL is missing from the disk, or the size of the WAL does not match the recorded size in Manifest, an error is reported adn the recovery is aborted. 

.. note::

    This option does not work with a secondary instance.

.. _rocksdb_two_write_queues:

.. rubric:: ``rocksdb_two_write_queues``

Implemented in :ref:`5.7.21-20`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-two_write_queues``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``ON``

When enabled this variable allows/encourages threads that are using
two-phase commit to ``prepare`` in parallel.

.. _rocksdb_unsafe_for_binlog:

.. rubric:: ``rocksdb_unsafe_for_binlog``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-unsafe-for-binlog``
   * - Dynamic
     - Yes
   * - Scope
     - Global, Session
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Specifies whether to allow statement-based binary logging
which may break consistency.
Disabled by default.

.. _rocksdb_update_cf_options:

.. rubric:: ``rocksdb_update_cf_options``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-update-cf-options``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - String
   * - Default
     - 

Specifies option updates for each column family.
Empty by default.

.. _rocksdb_use_adaptive_mutex:

.. rubric:: ``rocksdb_use_adaptive_mutex``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-use-adaptive-mutex``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Specifies whether to use adaptive mutex
which spins in user space before resorting to the kernel.
Disabled by default.

.. _rocksdb_use_default_sk_cf:

.. rubric:: ``rocksdb_use_default_sk_cf``

Implemented in :ref:`5.7.32-35`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-use-default-sk-cf``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - OFF
  
Use ``default_sk`` column family for secondary keys.

.. _rocksdb_use_direct_io_for_flush_and_compaction:

.. rubric:: ``rocksdb_use_direct_io_for_flush_and_compaction``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-use-direct-io-for-flush-and-compaction``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Specifies whether to write to data files directly,
without caches or buffers.
Disabled by default.

.. _rocksdb_use_direct_reads:

.. rubric:: ``rocksdb_use_direct_reads``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-use-direct-reads``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Specifies whether to read data files directly,
without caches or buffers.
Disabled by default.
If you enable this,
make sure that :ref:`rocksdb_allow_mmap_reads` is disabled.

.. _rocksdb_use_fsync:

.. rubric:: ``rocksdb_use_fsync``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-use-fsync``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Specifies whether MyRocks should use ``fsync`` instead of ``fdatasync``
when requesting a sync of a data file.
Disabled by default.

.. _rocksdb_validate_tables:

.. rubric:: ``rocksdb_validate_tables``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-validate-tables``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``1``

Specifies whether to verify that MySQL ``.frm`` files match MyRocks tables.

* ``0``: do not verify.
* ``1``: verify and fail on error (default).
* ``2``: verify and continue with error.

.. _rocksdb_verify_row_debug_checksums:

.. rubric:: ``rocksdb_verify_row_debug_checksums``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-verify-row-debug-checksums``
   * - Dynamic
     - Yes
   * - Scope
     - Global, Session
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Specifies whether to verify checksums when reading index or table records.
Disabled by default.

.. _rocksdb_wal_bytes_per_sync:

.. rubric:: ``rocksdb_wal_bytes_per_sync``

Changed to dynamic in :ref:`5.7.21-20`. 

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-wal-bytes-per-sync``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``0``

Specifies how often should the OS sync WAL (write-ahead log) files to disk
as they are being written, asynchronously, in the background.
This operation can be used to smooth out write I/O over time.
Default value is ``0``, meaning that files are never synced.
Allowed range is up to ``18446744073709551615``.

.. _rocksdb_wal_dir:

.. rubric:: ``rocksdb_wal_dir``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-wal-dir``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - String
   * - Default
     - 

Specifies the path to the directory where MyRocks stores WAL files.

.. _rocksdb_wal_recovery_mode:

.. rubric:: ``rocksdb_wal_recovery_mode``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-wal-recovery-mode``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``2``

.. note:: 

    In version 5.7.31-34 and later, the default is changed from ``1`` to ``2``. 

Specifies the level of tolerance when recovering write-ahead logs (WAL) files
after a system crash.

The following are the options:

 * ``0``: if the last WAL entry is corrupted, truncate the entry and either start the server normally or refuse to start.

 * ``1``: if a WAL entry is corrupted, the server fails to   start and does not recover from the crash.

 * ``2`` (default): if a corrupted WAL entry is detected, truncate all entries after the detected corrupted entry. You can select this setting for replication replicas.

 * ``3``: If a corrupted WAL entry is detected, skip only the corrupted entry and continue the apply WAL entries. This option can be dangerous.

.. _rocksdb_wal_size_limit_mb:

.. rubric:: ``rocksdb_wal_size_limit_mb``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-wal-size-limit-mb``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``0``

Specifies the maximum size of all WAL files in megabytes
before attempting to flush memtables and delete the oldest files.
Default value is ``0`` (never rotated).
Allowed range is up to ``9223372036854775807``.

.. _rocksdb_wal_ttl_seconds:

.. rubric:: ``rocksdb_wal_ttl_seconds``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-wal-ttl-seconds``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``0``

Specifies the timeout in seconds before deleting archived WAL files.
Default is ``0`` (WAL files are not archived).
Allowed range is up to ``9223372036854775807``.

.. _rocksdb_whole_key_filtering:

.. rubric:: ``rocksdb_whole_key_filtering``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-whole-key-filtering``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``ON``

Specifies whether the bloomfilter should use the whole key for filtering
instead of just the prefix.
Enabled by default.
Make sure that lookups use the whole key for matching.

.. _rocksdb_write_batch_max_bytes:

.. rubric:: ``rocksdb_write_batch_max_bytes``

Implemented in :ref:`5.7.20-18`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-write-batch-max-bytes``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Numeric
   * - Default
     - ``0``

Specifies the maximum size of a RocksDB write batch in bytes. ``0`` means no
limit. In case user exceeds the limit following error will be shown:
``ERROR HY000: Status error 10 received from RocksDB: Operation aborted: Memory
limit reached``.

.. _rocksdb_write_disable_wal:

.. rubric:: ``rocksdb_write_disable_wal``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-write-disable-wal``
   * - Dynamic
     - Yes
   * - Scope
     - Global, Session
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Lets you temporarily disable writes to WAL files,
which can be useful for bulk loading.

.. _rocksdb_write_ignore_missing_column_families:

.. rubric:: ``rocksdb_write_ignore_missing_column_families``

Implemented in :ref:`5.7.19-17`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-write-ignore-missing-column-families``
   * - Dynamic
     - Yes
   * - Scope
     - Global, Session
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Specifies whether to ignore writes to column families that do not exist.
Disabled by default (writes to non-existent column families are not ignored).

.. _rocksdb_write_policy:

.. rubric:: ``rocksdb_write_policy``

Implemented in :ref:`5.7.23-23`. 

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--rocksdb-write-policy``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - String
   * - Default
     - ``write_committed``

Specifies when two-phase commit data are actually written into the database.
Allowed values are ``write_committed``, ``write_prepared``, and
``write_unprepared``.

Default value is ``write_committed`` which means data are written at commit
time. If the value is set to ``write_prepared``, then data are written after
the prepare phase of a two-phase transaction. If the value is set to
``write_unprepared``, then data are written before the prepare phase.