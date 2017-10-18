.. _myrocks_variables:

=================
MyRocks Variables
=================

MyRocks provides variables to tune performance
and manage the behavior of the storage engine.

.. contents::
   :local:

.. _myrocks_server_variables:

MyRocks Server Variables
------------------------

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

.. tabularcolumns:: |p{9cm}|p{2cm}|p{2cm}|p{2cm}|

.. list-table::
   :header-rows: 1

   * - Name
     - Command Line
     - Dynamic
     - Scope
   * - :variable:`rocksdb_access_hint_on_compaction_start`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_advise_random_on_open`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_allow_concurrent_memtable_write`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_allow_mmap_reads`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_allow_mmap_writes`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_base_background_compactions`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_block_cache_size`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_block_restart_interval`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_block_size`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_block_size_deviation`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_bulk_load`
     - Yes
     - Yes
     - Global, Session
   * - :variable:`rocksdb_bulk_load_size`
     - Yes
     - Yes
     - Global
   * - :variable:`rocksdb_bytes_per_sync`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_cache_index_and_filter_blocks`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_checksums_pct`
     - Yes
     - Yes
     - Global, Session
   * - :variable:`rocksdb_collect_sst_properties`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_commit_in_the_middle`
     - Yes
     - Yes
     - Global
   * - :variable:`rocksdb_compact_cf`
     - Yes
     - Yes
     - Global
   * - :variable:`rocksdb_compaction_readahead_size`
     - Yes
     - Yes
     - Global
   * - :variable:`rocksdb_compaction_sequential_deletes`
     - Yes
     - Yes
     - Global
   * - :variable:`rocksdb_compaction_sequential_deletes_count_sd`
     - Yes
     - Yes
     - Global
   * - :variable:`rocksdb_compaction_sequential_deletes_file_size`
     - Yes
     - Yes
     - Global
   * - :variable:`rocksdb_compaction_sequential_deletes_window`
     - Yes
     - Yes
     - Global
   * - :variable:`rocksdb_create_checkpoint`
     - Yes
     - Yes
     - Global
   * - :variable:`rocksdb_create_if_missing`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_create_missing_column_families`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_datadir`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_db_write_buffer_size`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_deadlock_detect`
     - Yes
     - Yes
     - Global, Session
   * - :variable:`rocksdb_debug_optimizer_no_zero_cardinality`
     - Yes
     - Yes
     - Global, Session
   * - :variable:`rocksdb_debug_ttl_ignore_pk`
     - Yes
     - Yes
     - Global
   * - :variable:`rocksdb_default_cf_options`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_delayed_write_rate`
     - Yes
     - Yes
     - Global
   * - :variable:`rocksdb_delete_obsolete_files_period_micros`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_enable_bulk_load_api`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_enable_ttl`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_enable_thread_tracking`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_enable_write_thread_adaptive_yield`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_error_if_exists`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_flush_log_at_trx_commit`
     - Yes
     - Yes
     - Global, Session
   * - :variable:`rocksdb_flush_memtable_on_analyze`
     - Yes
     - Yes
     - Global, Session
   * - :variable:`rocksdb_force_compute_memtable_stats`
     - Yes
     - Yes
     - Global
   * - :variable:`rocksdb_force_compute_memtable_stats_cachetime`
     - Yes
     - Yes
     - Global
   * - :variable:`rocksdb_force_flush_memtable_and_lzero_now`
     - Yes
     - Yes
     - Global
   * - :variable:`rocksdb_force_flush_memtable_now`
     - Yes
     - Yes
     - Global
   * - :variable:`rocksdb_force_index_records_in_range`
     - Yes
     - Yes
     - Global, Session
   * - :variable:`rocksdb_hash_index_allow_collision`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_index_type`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_info_log_level`
     - Yes
     - Yes
     - Global
   * - :variable:`rocksdb_is_fd_close_on_exec`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_keep_log_file_num`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_lock_scanned_rows`
     - Yes
     - Yes
     - Global, Session
   * - :variable:`rocksdb_lock_wait_timeout`
     - Yes
     - Yes
     - Global, Session
   * - :variable:`rocksdb_log_file_time_to_roll`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_manifest_preallocation_size`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_max_background_compactions`
     - Yes
     - Yes
     - Global
   * - :variable:`rocksdb_max_background_flushes`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_max_log_file_size`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_max_manifest_file_size`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_max_open_files`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_max_row_locks`
     - Yes
     - Yes
     - Global, Session
   * - :variable:`rocksdb_max_subcompactions`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_max_total_wal_size`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_merge_buf_size`
     - Yes
     - Yes
     - Global, Session
   * - :variable:`rocksdb_merge_combine_read_size`
     - Yes
     - Yes
     - Global, Session
   * - :variable:`rocksdb_merge_tmp_file_removal_delay_ms`
     - Yes
     - Yes
     - Global, Session
   * - :variable:`rocksdb_new_table_reader_for_compaction_inputs`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_no_block_cache`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_override_cf_options`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_paranoid_checks`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_pause_background_work`
     - Yes
     - Yes
     - Global
   * - :variable:`rocksdb_perf_context_level`
     - Yes
     - Yes
     - Global, Session
   * - :variable:`rocksdb_persistent_cache_path`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_persistent_cache_size_mb`
     - Yes
     - No
     - Global, Session
   * - :variable:`rocksdb_pin_l0_filter_and_index_blocks_in_cache`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_print_snapshot_conflict_queries`
     - Yes
     - Yes
     - Global
   * - :variable:`rocksdb_rate_limiter_bytes_per_sec`
     - Yes
     - Yes
     - Global
   * - :variable:`rocksdb_read_free_rpl_tables`
     - Yes
     - Yes
     - Global, Session
   * - :variable:`rocksdb_records_in_range`
     - Yes
     - Yes
     - Global, Session
   * - :variable:`rocksdb_reset_stats`
     - Yes
     - Yes
     - Global
   * - :variable:`rocksdb_rpl_skip_tx_api`
     - Yes
     - Yes
     - Global
   * - :variable:`rocksdb_seconds_between_stat_computes`
     - Yes
     - Yes
     - Global
   * - :variable:`rocksdb_signal_drop_index_thread`
     - Yes
     - Yes
     - Global
   * - :variable:`rocksdb_skip_bloom_filter_on_read`
     - Yes
     - Yes
     - Global, Session
   * - :variable:`rocksdb_skip_fill_cache`
     - Yes
     - Yes
     - Global, Session
   * - :variable:`rocksdb_sst_mgr_rate_bytes_per_sec`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_stats_dump_period_sec`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_store_row_debug_checksums`
     - Yes
     - Yes
     - Global, Session
   * - :variable:`rocksdb_strict_collation_check`
     - Yes
     - Yes
     - Global
   * - :variable:`rocksdb_strict_collation_exceptions`
     - Yes
     - Yes
     - Global
   * - :variable:`rocksdb_table_cache_numshardbits`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_table_stats_sampling_pct`
     - Yes
     - Yes
     - Global
   * - :variable:`rocksdb_tmpdir`
     - Yes
     - Yes
     - Global, Session
   * - :variable:`rocksdb_trace_sst_api`
     - Yes
     - Yes
     - Global, Session
   * - :variable:`rocksdb_unsafe_for_binlog`
     - Yes
     - Yes
     - Global, Session
   * - :variable:`rocksdb_update_cf_options`
     - Yes
     - Yes
     - Global
   * - :variable:`rocksdb_use_adaptive_mutex`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_use_direct_io_for_flush_and_compaction`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_use_direct_reads`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_use_fsync`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_validate_tables`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_verify_row_debug_checksums`
     - Yes
     - Yes
     - Global, Session
   * - :variable:`rocksdb_wal_bytes_per_sync`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_wal_dir`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_wal_recovery_mode`
     - Yes
     - Yes
     - Global
   * - :variable:`rocksdb_wal_size_limit_mb`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_wal_ttl_seconds`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_whole_key_filtering`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_write_disable_wal`
     - Yes
     - Yes
     - Global, Session
   * - :variable:`rocksdb_write_ignore_missing_column_families`
     - Yes
     - Yes
     - Global, Session

.. variable:: rocksdb_access_hint_on_compaction_start

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-access-hint-on-compaction-start``
  :dyn: No
  :scope: Global
  :vartype: String or Numeric
  :default: ``NORMAL`` or ``1``

Specifies the file access pattern once a compaction is started,
applied to all input files of a compaction.
Possible values are:

* ``0`` = ``NONE``
* ``1`` = ``NORMAL`` (default)
* ``2`` = ``SEQUENTIAL``
* ``3`` = ``WILLNEED``

.. variable:: rocksdb_advise_random_on_open

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-advise-random-on-open``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``ON``

Specifies whether to hint the underlying file system
that the file access pattern is random,
when a data file is opened.
Enabled by default.

.. variable:: rocksdb_allow_concurrent_memtable_write

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-allow-concurrent-memtable-write``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Specifies whether to allow multiple writers to update memtables in parallel.
Disabled by default.

.. note:: Not all memtables support concurrent writes.

.. variable:: rocksdb_allow_mmap_reads

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-allow-mmap-reads``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Specifies whether to allow the OS to map a data file into memory for reads.
Disabled by default.
If you enable this,
make sure that :variable:`rocksdb_use_direct_reads` is disabled.

.. variable:: rocksdb_allow_mmap_writes

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-allow-mmap-writes``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Specifies whether to allow the OS to map a data file into memory for writes.
Disabled by default.

.. variable:: rocksdb_base_background_compactions

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-base-background-compactions``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: ``1``

Specifies the suggested number of concurrent background compaction jobs,
submitted to the default LOW priority thread pool in RocksDB.
Default is ``1``.
Allowed range of values is from ``-1`` to ``64``.
Maximum depends on the :variable:`rocksdb_max_background_compactions`
variable.

.. variable:: rocksdb_block_cache_size

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-block-cache-size``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: ``536870912``

Specifies the size of the LRU block cache for RocksDB.
This memory is reserved for the block cache,
which is in addition to any filesystem caching that may occur.

Minimum value is ``1024``,
because that's the size of one block.

Default value is ``536870912``.

Maximum value is ``9223372036854775807``.

.. variable:: rocksdb_block_restart_interval

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-block-restart-interval``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: ``16``

Specifies the number of keys for each set of delta encoded data.
Default value is ``16``.
Allowed range is from ``1`` to ``2147483647``.

.. variable:: rocksdb_block_size

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-block-size``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: ``4096``

Specifies the size of the data block for reading RocksDB data files.
Default value is ``4096``.
Allowed range is from ``1`` to ``18446744073709551615``.

.. variable:: rocksdb_block_size_deviation

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-block-size-deviation``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: ``10``

Specifies the threshold for free space allowed in a data block
(see :variable:`rocksdb_block_size`).
If there is less space remaining,
close the block (and write to new block).
Default value is ``10``, meaning that the block is not closed
until there is less than 10 bits of free space remaining.

Allowed range is from ``1`` to ``2147483647``.

.. variable:: rocksdb_bulk_load

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-bulk-load``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Boolean
  :default: ``OFF``

Specifies whether to use bulk load:
MyRocks will ignore checking keys for uniqueness
or acquiring locks during transactions.
Disabled by default.
Enable this only if you are certain that there are no row conflicts,
for example, when setting up a new MyRocks instance from a MySQL dump.

Enabling this variable will also enable
the :variable:`rocksdb_commit_in_the_middle` variable.

.. variable:: rocksdb_bulk_load_size

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-bulk-load-size``
  :dyn: Yes
  :scope: Global. Session
  :vartype: Numeric
  :default: ``1000``

Specifies the number of keys to accumulate
before committing them to the storage engine when bulk load is enabled
(see :variable:`rocksdb_bulk_load`).
Default value is ``1000``,
which means that a batch can contain up to 1000 records
before they are implicitly committed.
Allowed range is from ``1`` to ``1073741824``.

.. variable:: rocksdb_bytes_per_sync

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-bytes-per-sync``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: ``0``

Specifies how often should the OS sync files to disk
as they are being written, asynchronously, in the background.
This operation can be used to smooth out write I/O over time.
Default value is ``0`` meaning that files are never synced.
Allowed range is up to ``18446744073709551615``.

.. variable:: rocksdb_cache_index_and_filter_blocks

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-cache-index-and-filter-blocks``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``ON``

Specifies whether RocksDB should use the block cache for caching the index
and bloomfilter data blocks from each data file.
Enabled by default.
If you disable this feature,
RocksDB will allocate additional memory to maintain these data blocks.

.. variable:: rocksdb_checksums_pct

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-checksums-pct``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Numeric
  :default: ``100``

Specifies the percentage of rows to be checksummed.
Default value is ``100`` (checksum all rows).
Allowed range is from ``0`` to ``100``.

.. variable:: rocksdb_collect_sst_properties

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-collect-sst-properties``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``ON``

Specifies whether to collect statistics on each data file
to improve optimizer behavior.
Enabled by default.

.. variable:: rocksdb_commit_in_the_middle

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-commit-in-the-middle``
  :dyn: Yes
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Specifies whether to commit rows implicitly
when a batch contains more than the value of
:variable:`rocksdb_bulk_load_size`.
This is disabled by default
and will be enabled if :variable:`rocksdb_bulk_load` is enabled.

.. variable:: rocksdb_compact_cf

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-compact-cf``
  :dyn: Yes
  :scope: Global
  :vartype: String
  :default:

Specifies the name of the column family to compact.

.. variable:: rocksdb_compaction_readahead_size

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-compaction-readahead-size``
  :dyn: Yes
  :scope: Global
  :vartype: Numeric
  :default: ``0``

Specifies the size of reads to perform ahead of compaction.
Default value is ``0``.
Set this to at least 2 megabytes (``16777216``)
when using MyRocks with spinning disks
to ensure sequential reads instead of random.
Maximum allowed value is ``18446744073709551615``.

.. note:: If you set this variable to a non-zero value,
   :variable:`new_table_reader_for_compaction_inputs` is enabled.

.. variable:: rocksdb_compaction_sequential_deletes

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-compaction-sequential-deletes``
  :dyn: Yes
  :scope: Global
  :vartype: Numeric
  :default: ``0``

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
   :variable:`rocksdb_compaction_sequential_deletes_file_size`
   to only perform compaction on large files.

.. variable:: rocksdb_compaction_sequential_deletes_count_sd

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-compaction-sequential-deletes-count-sd``
  :dyn: Yes
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Specifies whether to count single deletes as delete markers
recognized by :variable:`rocksdb_compaction_sequential_deletes`.
Disabled by default.

.. variable:: rocksdb_compaction_sequential_deletes_file_size

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-compaction-sequential-deletes-file-size``
  :dyn: Yes
  :scope: Global
  :vartype: Numeric
  :default: ``0``

Specifies the minimum file size required to trigger compaction on it
by :variable:`rocksdb_compaction_sequential_deletes`.
Default value is ``0``,
meaning that compaction is triggered regardless of file size.
Allowed range is from ``-1`` to ``9223372036854775807``.

.. variable:: rocksdb_compaction_sequential_deletes_window

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-compaction-sequential-deletes-window``
  :dyn: Yes
  :scope: Global
  :vartype: Numeric
  :default: ``0``

Specifies the size of the window for counting delete markers
by :variable:`rocksdb_compaction_sequential_deletes`.
Default value is ``0``.
Allowed range is up to ``2000000`` (two million).

.. variable:: rocksdb_create_checkpoint

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-create-checkpoint``
  :dyn: Yes
  :scope: Global
  :vartype: String
  :default:

Specifies the directory where MyRocks should create a checkpoint.
Empty by default.

.. variable:: rocksdb_create_if_missing

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-create-if-missing``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``ON``

Specifies whether MyRocks should create its database if it does not exist.
Enabled by default.

.. variable:: rocksdb_create_missing_column_families

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-create-missing-column-families``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Specifies whether MyRocks should create new column families
if they do not exist.
Disabled by default.

.. variable:: rocksdb_datadir

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-datadir``
  :dyn: No
  :scope: Global
  :vartype: String
  :default: ``./.rocksdb``

Specifies the location of the MyRocks data directory.
By default, it is created in the current working directory.

.. variable:: rocksdb_db_write_buffer_size

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-db-write-buffer-size``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: ``0``

Specifies the size of the memtable used to store writes in MyRocks.
This is the size per column family.
When this size is reached, the memtable is flushed to persistent media.
Default value is ``0``.
Allowed range is up to ``18446744073709551615``.

.. variable:: rocksdb_deadlock_detect

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-deadlock-detect``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Boolean
  :default: ``OFF``

Specifies whether MyRocks should detect deadlocks.
Disabled by default.

.. variable:: rocksdb_debug_optimizer_no_zero_cardinality

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-debug-optimizer-no-zero-cardinality``
  :dyn: Yes
  :scope: Global
  :vartype: Boolean
  :default: ``ON``

Specifies whether MyRocks should prevent zero cardinality
by always overriding it with some value.

.. variable:: rocksdb_debug_ttl_ignore_pk

  :version 5.7.20-18: Implemented
  :cli: ``--rocksdb-debug-ttl-ignore-pk``
  :dyn: Yes
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Specifies whether compaction filtering should occur on primary key TTL data.
Disable by default.

.. note:: Use only in debug builds.

.. variable:: rocksdb_default_cf_options

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-default-cf-options``
  :dyn: No
  :scope: Global
  :vartype: String
  :default:

Specifies the default column family options for MyRocks.
Empty by default.

.. variable:: rocksdb_delayed_write_rate

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-delayed-write-rate``
  :dyn: Yes
  :scope: Global
  :vartype: Numeric
  :default: ``16777216``

Specifies the write rate in bytes per second, which should be used
if MyRocks hits a soft limit or threshold for writes.
Default value is ``16777216`` (16 MB/sec).
Allowed range is from ``0`` to ``18446744073709551615``.

.. variable:: rocksdb_delete_obsolete_files_period_micros

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-delete-obsolete-files-period-micros``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: ``21600000000``

Specifies the period in microseconds to delete obsolete files
regardless of files removed during compaction.
Default value is ``21600000000`` (6 hours).
Allowed range is up to ``9223372036854775807``.

.. variable:: rocksdb_enable_bulk_load_api

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-enable-bulk-load-api``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``ON``

Specifies whether to use the ``SSTFileWriter`` feature for bulk loading,
This feature bypasses the memtable,
but requires keys to be inserted into the table
in either ascending or descending order.
Enabled by default.
If disabled, bulk loading uses the normal write path via the memtable
and does not require keys to be inserted in any order.

.. variable:: rocksdb_enable_ttl

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-enable-ttl``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``ON``

Specifies whether to keep expired TTL records during compaction.
Enabled by default.
If disabled, expired TTL records will be dropped during compaction.

.. variable:: rocksdb_enable_thread_tracking

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-enable-thread-tracking``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Specifies whether to enable tracking the status of threads
accessing the database.
Disabled by default.
If enabled, thread status will be available via ``GetThreadList()``.

.. variable:: rocksdb_enable_write_thread_adaptive_yield

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-enable-write-thread-adaptive-yield``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Specifies whether the MyRocks write batch group leader
should wait up to the maximum allowed time
before blocking on a mutex.
Disabled by default.
Enable it to increase throughput for concurrent workloads.

.. variable:: rocksdb_error_if_exists

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-error-if-exists``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Specifies whether to report an error when a database already exists.
Disabled by default.

.. variable:: rocksdb_flush_log_at_trx_commit

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-flush-log-at-trx-commit``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Numeric
  :default: ``1``

Specifies whether to sync on every transaction commit,
similar to |innodb_flush_log_at_trx_commit|_.
Enabled by default, which ensures ACID compliance.

.. |innodb_flush_log_at_trx_commit| replace:: ``innodb_flush_log_at_trx_commit``
.. _innodb_flush_log_at_trx_commit: https://dev.mysql.com/doc/refman/5.7/en/innodb-parameters.html#sysvar_innodb_flush_log_at_trx_commit

Possible values:

* ``0``: Do not sync on transaction commit.
  This provides better performance, but may lead to data inconsistency
  in case of a crash.

* ``1``: Sync on every transaction commit.
  This is set by default and recommended
  as it ensures data consistency,
  but reduces performance.

* ``2``: Sync every second.

.. variable:: rocksdb_flush_memtable_on_analyze

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-flush-memtable-on-analyze``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Boolean
  :default: ``ON``

Specifies whether to flush the memtable when running ``ANALYZE`` on a table.
Enabled by default.
This ensures accurate cardinality
by including data in the memtable for calculating stats.

.. variable:: rocksdb_force_compute_memtable_stats

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-force-compute-memtable-stats``
  :dyn: Yes
  :scope: Global
  :vartype: Boolean
  :default: ``ON``

Specifies whether data in the memtables should be included
for calculating index statistics
used by the query optimizer.
Enabled by default.
This provides better accuracy, but may reduce performance.

.. variable:: rocksdb_force_compute_memtable_stats_cachetime

  :version 5.7.20-18: Implemented
  :cli: ``--rocksdb-force-compute-memtable-stats-cachetime``
  :dyn: Yes
  :scope: Global
  :vartype: Numeric
  :default: ``60000000``

Specifies the time in microseconds to cache memtable estimates.
Default is 60 seconds.

.. variable:: rocksdb_force_flush_memtable_and_lzero_now

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-force-flush-memtable-and-lzero-now``
  :dyn: Yes
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Works similar to :variable:`force_flush_memtable_now`
but also flushes all L0 files.

.. variable:: rocksdb_force_flush_memtable_now

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-force-flush-memtable-now``
  :dyn: Yes
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Forces MyRocks to immediately flush all memtables out to data files.

.. warning:: Use with caution!
   Write requests will be blocked until all memtables are flushed.

.. variable:: rocksdb_force_index_records_in_range

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-force-index-records-in-range``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Numeric
  :default: ``1``

Specifies the value used to override the number of rows
returned to query optimizer when ``FORCE INDEX`` is used.
Default value is ``1``.
Allowed range is from ``0`` to ``2147483647``.
Set to ``0`` if you do not want to override the returned value.

.. variable:: rocksdb_hash_index_allow_collision

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-hash-index-allow-collision``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``ON``

Specifies whether hash collisions are allowed.
Enabled by default, which uses less memory.
If disabled, full prefix is stored to prevent hash collisions.

.. variable:: rocksdb_index_type

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-index-type``
  :dyn: No
  :scope: Global
  :vartype: Enum
  :default: ``kBinarySearch``

Specifies the type of indexing used by MyRocks:

* ``kBinarySearch``: Binary search (default).

* ``kHashSearch``: Hash search.

.. variable:: rocksdb_info_log_level

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-info-log-level``
  :dyn: Yes
  :scope: Global
  :vartype: Enum
  :default: ``error_level``

Specifies the level for filtering messages written by MyRocks
to the ``mysqld`` log.

* ``debug_level``: Maximum logging (everything including debugging
  log messages)
* ``info_level``
* ``warn_level``
* ``error_level`` (default)
* ``fatal_level``: Minimum logging (only fatal error messages logged)

.. variable:: rocksdb_is_fd_close_on_exec

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-is-fd-close-on-exec``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``ON``

Specifies whether child processes should inherit open file jandles.
Enabled by default.

.. variable:: rocksdb_keep_log_file_num

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-keep-log-file-num``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: ``1000``

Specifies the maximum number of info log files to keep.
Default value is ``1000``.
Allowed range is from ``1`` to ``18446744073709551615``.

.. variable:: rocksdb_lock_scanned_rows

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-lock-scanned-rows``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Boolean
  :default: ``OFF``

Specifies whether to hold the lock on rows that are scanned during ``UPDATE``
and not actually updated.
Disabled by default.

.. variable:: rocksdb_lock_wait_timeout

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-lock-wait-timeout``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Numeric
  :default: ``1``

Specifies the number of seconds MyRocks should wait to acquire a row lock
before aborting the request.
Default value is ``1``.
Allowed range is up to ``1073741824``.

.. variable:: rocksdb_log_file_time_to_roll

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-log-file-time-to-roll``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: ``0``

Specifies the period (in seconds) for rotating the info log files.
Default value is ``0``, meaning that the log file is not rotated.
Allowed range is up to ``18446744073709551615``.

.. variable:: rocksdb_manifest_preallocation_size

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-manifest-preallocation-size``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: ``0``

Specifies the number of bytes to preallocate for the MANIFEST file
used by MyRocks to store information
about column families, levels, active files, etc.
Default value is ``0``.
Allowed range is up to ``18446744073709551615``.

.. note:: A value of ``4194304`` (4 MB) is reasonable
   to reduce random I/O on XFS.

.. variable:: rocksdb_max_background_compactions

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-max-background-compactions``
  :dyn: Yes
  :scope: Global
  :vartype: Numeric
  :default: ``1``

Specifies the maximum number of concurrent background compaction threads,
submitted to the low-priority thread pool.
Default value is ``1``.
Allowed range is up to ``64``.

.. variable:: rocksdb_max_background_flushes

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-max-background-flushes``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: ``1``

Specifies the maximum number of concurrent background memtable flush threads,
submitted to the high-priority thread-pool.
Default value is ``1``.
Allowed range is up to ``64``.

.. variable:: rocksdb_max_log_file_size

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-max-log-file-size``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: ``0``

Specifies the maximum size for info log files,
after which the log is rotated.
Default value is ``0``, meaning that only one log file is used.
Allowed range is up to ``18446744073709551615``.

Also see :variable:`rocksdb_log_file_time_to_roll`.

.. variable:: rocksdb_max_manifest_file_size

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-manifest-log-file-size``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: ``18446744073709551615``

Specifies the maximum size of the MANIFEST data file,
after which it is rotated.
Default value is also the maximum, making it practically unlimited:
only one manifest file is used.

.. variable:: rocksdb_max_open_files

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-max-open-files``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: ``4294967295``

Specifies the maximum number of file handles opened by MyRocks.
Default value is also the maximum, making it practically unlimited:
all opened files remain open.

.. variable:: rocksdb_max_row_locks

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-max-row-locks``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Numeric
  :default: ``1073741824``

Specifies the limit on the maximum number of row locks a transaction can have
before it fails.
Default value is also the maximum, making it practically unlimited:
transactions never fail due to row locks.

.. variable:: rocksdb_max_subcompactions

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-max-subcompactions``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: ``1``

Specifies the maximum number of threads allowed for each compaction job.
Default value of ``1`` means no subcompactions (one thread per compaction job).
Allowed range is up to ``64``.

.. variable:: rocksdb_max_total_wal_size

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-max-total-wal-size``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: ``0``

Specifies the maximum total size of WAL (write-ahead log) files,
after which memtables are flushed.
Default value is ``0``: WAL size limit is chosen dynamically.
Allowed range is up to ``9223372036854775807``.

.. variable:: rocksdb_merge_buf_size

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-merge-buf-size``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Numeric
  :default: ``67108864``

Specifies the size (in bytes) of the merge-sort buffers
used to accumulate data during secondary key creation.
New entries are written directly to the lowest level in the database,
instead of updating indexes through the memtable and L0.
These values are sorted using merge-sort,
with buffers set to 64 MB by default (``67108864``).
Allowed range is from ``100`` to ``18446744073709551615``.

.. variable:: rocksdb_merge_combine_read_size

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-merge-combine-read-size``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Numeric
  :default: ``1073741824``

Specifies the size (in bytes) of the merge-combine buffer
used for the merge-sort algorithm
as described in :variable:`rocksdb_merge_buf_size`.
Default size is 1 GB (``1073741824``).
Allowed range is from ``100`` to ``18446744073709551615``.

.. variable:: rocksdb_merge_tmp_file_removal_delay_ms

  :version 5.7.20-18: Implemented
  :cli: ``--rocksdb-merge-tmp-file-removal-delay-ms``
  :dyn: No
  :scope: Global, Session
  :vartype: Numeric
  :default: ``0``

Specifies the duration (in milliseconds) to sleep
when removing chunks of the temporary merge file
created during fast index creation.
Default is ``0``, which can cause trim stalls on flash storage
when a lot of files are removed without any delay.
The size of the chunks is controlled with :variable:`rocksdb_merge_buf_size`.

.. variable:: rocksdb_new_table_reader_for_compaction_inputs

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-new-table-reader-for-compaction-inputs``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Specifies whether MyRocks should create a new file descriptor and table reader
for each compaction input.
Disabled by default.
Enabling this may increase memory consumption,
but will also allow pre-fetch options to be specified for compaction
input files without impacting table readers used for user queries.

.. variable:: rocksdb_no_block_cache

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-no-block-cache``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Specifies whether to disable the block cache for column families.
Variable is disabled by default,
meaning that using the block cache is allowed.

.. variable:: rocksdb_override_cf_options

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-override-cf-options``
  :dyn: No
  :scope: Global
  :vartype: String
  :default:

Specifies option overrides for each column family.
Empty by default.

.. variable:: rocksdb_paranoid_checks

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-paranoid-checks``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``ON``

Specifies whether MyRocks should re-read the data file
as soon as it is created to verify correctness.
Enabled by default.

.. variable:: rocksdb_pause_background_work

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-pause-background-work``
  :dyn: Yes
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Specifies whether MyRocks should pause all background operations.
Disabled by default.

.. variable:: rocksdb_perf_context_level

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-perf-context-level``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Numeric
  :default: ``0``

Specifies the level of information to capture with the Perf Context plugins.
Default value is ``0``.
Allowed range is up to ``4``.

.. variable:: rocksdb_persistent_cache_path

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-persistent-cache-path``
  :dyn: No
  :scope: Global
  :vartype: String
  :default:

Specifies the path to the persistent cache.
Set this together with :variable:`rocksdb_persistent_cache_size_mb`.

.. variable:: rocksdb_persistent_cache_size_mb

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-persistent-cache-size-mb``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: ``0``

Specifies the size of the persisten cache in megabytes.
Default is ``0`` (persistent cache disabled).
Allowed range is up to ``18446744073709551615``.
Set this together with :variable:`rocksdb_persistent_cache_path`.

.. variable:: rocksdb_pin_l0_filter_and_index_blocks_in_cache

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-pin-l0-filter-and-index-blocks-in-cache``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``ON``

Specifies whether MyRocks pins the filter and index blocks in the cache
if :variable:`rocksdb_cache_index_and_filter_blocks` is enabled.
Enabled by default.

.. variable:: rocksdb_print_snapshot_conflict_queries

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-print-snapshot-conflict-queries``
  :dyn: Yes
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Specifies whether queries that generate snapshot conflicts
should be logged to the error log.
Disabled by default.

.. variable:: rocksdb_rate_limiter_bytes_per_sec

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-rate-limiter-bytes-per-sec``
  :dyn: Yes
  :scope: Global
  :vartype: Numeric
  :default: ``0``

Specifies the maximum rate at which MyRocks can write to media
via memtable flushes and compaction.
Default value is ``0`` (write rate is not limited).
Allowed range is up to ``9223372036854775807``.

.. variable:: rocksdb_read_free_rpl_tables

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-read-free-rpl-tables``
  :dyn: Yes
  :scope: Global, Session
  :vartype: String
  :default:

Lists tables (as a regular expression)
that should use read-free replication on the slave
(that is, replication without row lookups).
Empty by default.

.. variable:: rocksdb_records_in_range

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-records-in-range``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Numeric
  :default: ``0``

Specifies the value to overrride the result of ``records_in_range()``.
Default value is ``0``.
Allowed range is up to ``2147483647``.

.. variable:: rocksdb_reset_stats

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-reset-stats``
  :dyn: Yes
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Resets MyRocks internal statistics dynamically
(without restarting the server).

.. variable:: rocksdb_rpl_skip_tx_api

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-rpl-skip-tx-api``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Specifies whether write batches should be used for replication thread
instead of the transaction API.
Disabled by default.

.. variable:: rocksdb_seconds_between_stat_computes

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-seconds-between-stat-computes``
  :dyn: Yes
  :scope: Global
  :vartype: Numeric
  :default: ``3600``

Specifies the number of seconds to wait
between recomputation of table statistics for the optimizer.
During that time, only changed indexes are updated.
Default value is ``3600``.
Allowed is from ``0`` to ``4294967295``.

.. variable:: rocksdb_signal_drop_index_thread

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-signal-drop-index-thread``
  :dyn: Yes
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Signals the MyRocks drop index thread to wake up.

.. variable:: rocksdb_skip_bloom_filter_on_read

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-skip-bloom-filter-on_read``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Boolean
  :default: ``OFF``

Specifies whether bloom filters should be skipped on reads.
Disabled by default (bloom filters are not skipped).

.. variable:: rocksdb_skip_fill_cache

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-skip-fill-cache``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Boolean
  :default: ``OFF``

Specifies whether to skip caching data on read requests.
Disabled by default (caching is not skipped).

.. variable:: rocksdb_sst_mgr_rate_bytes_per_sec

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-sst-mgr-rate-bytes-per-sec``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Numeric
  :default: ``67108864``

Specifies the maximum rate for writing to data files.
Default value is ``67108864`` (64 MB/sec).
Allowed range is from ``0`` to ``18446744073709551615``.

.. variable:: rocksdb_stats_dump_period_sec

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-stats-dump-period-sec``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: ``600``

Specifies the period in seconds for performing a dump of the MyRocks statistics
to the info log.
Default value is ``600``.
Allowed range is up to ``2147483647``.

.. variable:: rocksdb_store_row_debug_checksums

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-store-row-debug-checksums``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Boolean
  :default: ``OFF``

Specifies whether to include checksums when writing index or table records.
Disabled by default.

.. variable:: rocksdb_strict_collation_check

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-strict-collation-check``
  :dyn: Yes
  :scope: Global
  :vartype: Boolean
  :default: ``ON``

Specifies whether to check and verify
that table indexes have proper collation settings.
Enabled by default.

.. variable:: rocksdb_strict_collation_exceptions

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-strict-collation-exceptions``
  :dyn: Yes
  :scope: Global
  :vartype: String
  :default:

Lists tables (as a regular expression) that should be excluded
from verifying case-sensitive collation
enforced by :variable:`rocksdb_strict_collation_check`.
Empty by default.

.. variable:: rocksdb_table_cache_numshardbits

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-table-cache-numshardbits``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: ``6``

Specifies the number if table caches.
Default value is ``6``.
Allowed range is from ``0`` to ``2147483647``.

.. variable:: rocksdb_table_stats_sampling_pct

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-table-stats-sampling-pct``
  :dyn: Yes
  :scope: Global
  :vartype: Numeric
  :default: ``10``

Specifies the percentage of entries to sample
when collecting statistics about table properties.
Default value is ``10``.
Allowed range is from ``0`` to ``100``.

.. variable:: rocksdb_tmpdir

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-tmpdir``
  :dyn: Yes
  :scope: Global, Session
  :vartype: String
  :default:

Specifies the path to the directory for temporary files during DDL operations.

.. variable:: rocksdb_trace_sst_api

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-trace-sst-api``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Boolean
  :default: ``OFF``

Specifies whether to generate trace output in the log
for each call to ``SstFileWriter``.
Disabled by default.

.. variable:: rocksdb_unsafe_for_binlog

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-unsafe-for-binlog``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Boolean
  :default: ``OFF``

Specifies whether to allow statement-based binary logging
which may break consistency.
Disabled by default.

.. variable:: rocksdb_update_cf_options

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-update-cf-options``
  :dyn: No
  :scope: Global
  :vartype: String
  :default:

Specifies option updates for each column family.
Empty by default.

.. variable:: rocksdb_use_adaptive_mutex

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-use-adaptive-mutex``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Specifies whether to use adaptive mutex
which spins in user space before resorting to the kernel.
Disabled by default.

.. variable:: rocksdb_use_direct_io_for_flush_and_compaction

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-use-direct-io-for-flush-and-compaction``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Specifies whether to write to data files directly,
without caches or buffers.
Disabled by default.

.. variable:: rocksdb_use_direct_reads

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-use-direct-reads``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Specifies whether to read data files directly,
without caches or buffers.
Disabled by default.
If you enable this,
make sure that :variable:`rocksdb_allow_mmap_reads` is disabled.

.. variable:: rocksdb_use_fsync

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-use-fsync``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Specifies whether MyRocks should use ``fsync`` instead of ``fdatasync``
when requesting a sync of a data file.
Disabled by default.

.. variable:: rocksdb_validate_tables

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-validate-tables``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: ``1``

Specifies whether to verify that MySQL ``.frm`` files match MyRocks tables.

* ``0``: do not verify.
* ``1``: verify and fail on error (default).
* ``2``: verify and continue with error.

.. variable:: rocksdb_verify_row_debug_checksums

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-verify-row-debug-checksums``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Boolean
  :default: ``OFF``

Specifies whether to verify checksums when reading index or table records.
Disabled by default.

.. variable:: rocksdb_wal_bytes_per_sync

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-wal-bytes-per-sync``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: ``0``

Specifies how often should the OS sync WAL (write-ahead log) files to disk
as they are being written, asynchronously, in the background.
This operation can be used to smooth out write I/O over time.
Default value is ``0``, meaning that files are never synced.
Allowed range is up to ``18446744073709551615``.

.. variable:: rocksdb_wal_dir

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-wal-dir``
  :dyn: No
  :scope: Global
  :vartype: String
  :default:

Specifies the path to the directory where MyRocks stores WAL files.

.. variable:: rocksdb_wal_recovery_mode

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-wal-recovery-mode``
  :dyn: Yes
  :scope: Global
  :vartype: Numeric
  :default: ``1``

Specifies the level of tolerance when recovering WAL files
after a system crash.
Default is ``1``.
Allowed range is from ``0`` to ``3``.

.. variable:: rocksdb_wal_size_limit_mb

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-wal-size-limit-mb``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: ``0``

Specifies the maximum size of all WAL files in megabytes
before attempting to flush memtables and delete the oldest files.
Default value is ``0`` (never rotated).
Allowed range is up to ``9223372036854775807``.

.. variable:: rocksdb_wal_ttl_seconds

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-wal-ttl-seconds``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: ``0``

Specifies the timeout in seconds before deleting archived WAL files.
Default is ``0`` (archived WAL files are never deleted).
Allowed range is up to ``9223372036854775807``.

.. variable:: rocksdb_whole_key_filtering

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-whole-key-filtering``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``ON``

Specifies whether the bloomfilter should use the whole key for filtering
instead of just the prefix.
Enabled by default.
Make sure that lookups use the whole key for matching.

.. variable:: rocksdb_write_disable_wal

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-write-disable-wal``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Boolean
  :default: ``OFF``

Lets you temporarily disable writes to WAL files,
which can be useful for bulk loading.

.. variable:: rocksdb_write_ignore_missing_column_families

  :version 5.7.19-17: Implemented
  :cli: ``--rocksdb-write-ignore-missing-column-families``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Boolean
  :default: ``OFF``

Specifies whether to ignore writes to column families that do not exist.
Disabled by default (writes to non-existent column families are not ignored).
