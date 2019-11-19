.. _myrocks_server_variables:

================================================================================
MyRocks Server Variables
================================================================================

The MyRocks server variables expose configuration of the underlying RocksDB
engine.  There several ways to set these variables:

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
   * - :variable:`rocksdb_allow_to_start_after_corruption`
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
     - Yes
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
   * - :variable:`rocksdb_bulk_load_allow_unsorted`
     - Yes
     - Yes
     - Global, Session
   * - :variable:`rocksdb_bulk_load_size`
     - Yes
     - Yes
     - Global
   * - :variable:`rocksdb_bytes_per_sync`
     - Yes
     - Yes
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
   * - :variable:`rocksdb_concurrent_prepare`
     - Yes
     - No
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
   * - :variable:`rocksdb_create_temporary_checkpoint`
     - Yes
     - Yes
     - Session
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
   * - :variable:`rocksdb_deadlock_detect_depth`
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
   * - :variable:`rocksdb_debug_ttl_read_filter_ts`
     - Yes
     - Yes
     - Global
   * - :variable:`rocksdb_debug_ttl_rec_ts`
     - Yes
     - Yes
     - Global
   * - :variable:`rocksdb_debug_ttl_snapshot_ts`
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
   * - :variable:`rocksdb_disable_file_deletions`
     - Yes
     - Yes
     - Session
   * - :variable:`rocksdb_enable_bulk_load_api`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_enable_ttl`
     - Yes
     - No
     - Global
   * - :variable:`rocksdb_enable_ttl_read_filtering`
     - Yes
     - Yes
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
   * - :variable:`rocksdb_ignore_unknown_options`
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
   * - :variable:`rocksdb_large_prefix`
     - Yes
     - Yes
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
   * - :variable:`rocksdb_manual_wal_flush`
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
   * - :variable:`rocksdb_max_background_jobs`
     - Yes
     - Yes
     - Global
   * - :variable:`rocksdb_max_latest_deadlocks`
     - Yes
     - Yes
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
   * - :variable:`rocksdb_no_create_column_family`
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
   * - :variable:`rocksdb_sim_cache_size`
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
   * - :variable:`rocksdb_two_write_queues`
     - Yes
     - No
     - Global
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
     - Yes
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
   * - :variable:`rocksdb_write_batch_max_bytes`
     - Yes
     - Yes
     - Global, Session
   * - :variable:`rocksdb_write_disable_wal`
     - Yes
     - Yes
     - Global, Session
   * - :variable:`rocksdb_write_ignore_missing_column_families`
     - Yes
     - Yes
     - Global, Session

.. variable:: rocksdb_access_hint_on_compaction_start

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

  :cli: ``--rocksdb-allow-concurrent-memtable-write``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Specifies whether to allow multiple writers to update memtables in parallel.
Disabled by default.

.. variable:: rocksdb_allow_to_start_after_corruption

  :cli: ``--rocksdb_allow_to_start_after_corruption``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Specifies whether to allow server to restart once MyRocks reported data
corruption. Disabled by default.

Once corruption is detected server writes marker file (named
ROCKSDB_CORRUPTED) in the data directory and aborts. If marker file exists,
then mysqld exits on startup with an error message. The restart failure will
continue until the problem is solved or until mysqld is started with this
variable turned on in the command line.

.. note:: Not all memtables support concurrent writes.

.. variable:: rocksdb_allow_mmap_reads

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

  :cli: ``--rocksdb-allow-mmap-writes``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Specifies whether to allow the OS to map a data file into memory for writes.
Disabled by default.

.. variable:: rocksdb_base_background_compactions

  :cli: ``--rocksdb-base-background-compactions``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: ``2``

Specifies the suggested number of concurrent background compaction jobs,
submitted to the default LOW priority thread pool in RocksDB.  Default is ``1``.
Allowed range of values is from ``-1`` to ``64``.  Maximum depends on the
:variable:`rocksdb_max_background_compactions` variable. This variable was
replaced with :variable:`rocksdb_max_background_jobs`, which automatically
decides how many threads to allocate towards flush/compaction.

.. variable:: rocksdb_block_cache_size

  :cli: ``--rocksdb-block-cache-size``
  :dyn: Yes
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

  :cli: ``--rocksdb-block-restart-interval``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: ``16``

Specifies the number of keys for each set of delta encoded data.
Default value is ``16``.
Allowed range is from ``1`` to ``2147483647``.

.. variable:: rocksdb_block_size

  :cli: ``--rocksdb-block-size``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: 16 KB

Specifies the size of the data block for reading RocksDB data files.
The default value is ``16 KB``.
The allowed range is from ``1024`` to ``18446744073709551615`` bytes.

.. variable:: rocksdb_block_size_deviation

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

.. variable:: rocksdb_bulk_load_allow_unsorted

  :cli: ``--rocksdb-bulk-load-allow-unsorted``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Boolean
  :default: ``OFF``

By default, the bulk loader requires its input to be sorted in the primary
key order. If enabled, unsorted inputs are allowed too, which are then
sorted by the bulkloader itself, at a performance penalty.

.. variable:: rocksdb_bulk_load

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

  :cli: ``--rocksdb-bytes-per-sync``
  :dyn: Yes
  :scope: Global
  :vartype: Numeric
  :default: ``0``

Specifies how often should the OS sync files to disk
as they are being written, asynchronously, in the background.
This operation can be used to smooth out write I/O over time.
Default value is ``0`` meaning that files are never synced.
Allowed range is up to ``18446744073709551615``.

.. variable:: rocksdb_cache_index_and_filter_blocks

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

  :cli: ``--rocksdb-checksums-pct``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Numeric
  :default: ``100``

Specifies the percentage of rows to be checksummed.
Default value is ``100`` (checksum all rows).
Allowed range is from ``0`` to ``100``.

.. variable:: rocksdb_collect_sst_properties

  :cli: ``--rocksdb-collect-sst-properties``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``ON``

Specifies whether to collect statistics on each data file
to improve optimizer behavior.
Enabled by default.

.. variable:: rocksdb_commit_in_the_middle

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

  :cli: ``--rocksdb-compact-cf``
  :dyn: Yes
  :scope: Global
  :vartype: String
  :default:

Specifies the name of the column family to compact.

.. variable:: rocksdb_compaction_readahead_size

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
   :variable:`rocksdb_new_table_reader_for_compaction_inputs` is enabled.

.. variable:: rocksdb_compaction_sequential_deletes

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

.. note::

   Depending on workload patterns, MyRocks can potentially maintain large
   numbers of delete markers, which increases latency of queries.  This
   compaction feature will reduce latency, but may also increase the MyRocks
   write rate.  Use this variable together with
   :variable:`rocksdb_compaction_sequential_deletes_file_size` to only perform
   compaction on large files.

.. variable:: rocksdb_compaction_sequential_deletes_count_sd

  :cli: ``--rocksdb-compaction-sequential-deletes-count-sd``
  :dyn: Yes
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Specifies whether to count single deletes as delete markers
recognized by :variable:`rocksdb_compaction_sequential_deletes`.
Disabled by default.

.. variable:: rocksdb_compaction_sequential_deletes_file_size

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

  :cli: ``--rocksdb-compaction-sequential-deletes-window``
  :dyn: Yes
  :scope: Global
  :vartype: Numeric
  :default: ``0``

Specifies the size of the window for counting delete markers
by :variable:`rocksdb_compaction_sequential_deletes`.
Default value is ``0``.
Allowed range is up to ``2000000`` (two million).

.. variable:: rocksdb_concurrent_prepare

  :cli: ``--rocksdb-concurrent_prepare``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``ON``

When enabled this variable allows/encourages threads that are using
two-phase commit to ``prepare`` in parallel. This variable was
renamed in upstream to :variable:`rocksdb_two_write_queues`.

.. variable:: rocksdb_create_checkpoint

  :cli: ``--rocksdb-create-checkpoint``
  :dyn: Yes
  :scope: Global
  :vartype: String
  :default:

Specifies the directory where MyRocks should create a checkpoint.
Empty by default.

.. variable:: rocksdb_create_if_missing

  :cli: ``--rocksdb-create-if-missing``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``ON``

Specifies whether MyRocks should create its database if it does not exist.
Enabled by default.

.. variable:: rocksdb_create_missing_column_families

  :cli: ``--rocksdb-create-missing-column-families``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Specifies whether MyRocks should create new column families
if they do not exist.
Disabled by default.

.. variable:: rocksdb_create_temporary_checkpoint

  :cli: ``--rocksdb-create-temporary-checkpoint``
  :dyn: Yes
  :scope: Session
  :vartype: String

This variable has been implemented in |Percona Server| :rn:`8.0.15-6`.
When specified it will create a temporary RocksDB 'checkpoint' or
'snapshot' in the :term:`datadir`. If the session ends with an existing
checkpoint, or if the variable is reset to another value, the checkpoint
will get removed. This variable should be used by backup tools. Prolonged
use or other misuse can have serious side effects to the server instance.

.. variable:: rocksdb_datadir

  :cli: ``--rocksdb-datadir``
  :dyn: No
  :scope: Global
  :vartype: String
  :default: ``./.rocksdb``

Specifies the location of the MyRocks data directory.
By default, it is created in the current working directory.

.. variable:: rocksdb_db_write_buffer_size

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

  :cli: ``--rocksdb-deadlock-detect``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Boolean
  :default: ``OFF``

Specifies whether MyRocks should detect deadlocks.
Disabled by default.

.. variable:: rocksdb_deadlock_detect_depth

  :cli: ``--rocksdb-deadlock-detect-depth``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Numeric
  :default: ``50``

Specifies the number of transactions deadlock detection will traverse
through before assuming deadlock.

.. variable:: rocksdb_debug_optimizer_no_zero_cardinality

  :cli: ``--rocksdb-debug-optimizer-no-zero-cardinality``
  :dyn: Yes
  :scope: Global
  :vartype: Boolean
  :default: ``ON``

Specifies whether MyRocks should prevent zero cardinality
by always overriding it with some value.

.. variable:: rocksdb_debug_ttl_ignore_pk

  :cli: ``--rocksdb-debug-ttl-ignore-pk``
  :dyn: Yes
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

For debugging purposes only. If true, compaction filtering will not occur
on Primary Key TTL data. This variable is a no-op in non-debug builds.

.. variable:: rocksdb_debug_ttl_read_filter_ts

  :cli: ``--rocksdb_debug-ttl-read-filter-ts``
  :dyn: Yes
  :scope: Global
  :vartype: Numeric
  :default: ``0``

For debugging purposes only.  Overrides the TTL read
filtering time to time + :variable:`debug_ttl_read_filter_ts`.
A value of ``0`` denotes that the variable is not set.
This variable is a no-op in non-debug builds.

.. variable:: rocksdb_debug_ttl_rec_ts

  :cli: ``--rocksdb-debug-ttl-rec-ts``
  :dyn: Yes
  :scope: Global
  :vartype: Numeric
  :default: ``0``

For debugging purposes only.  Overrides the TTL of
records to ``now()`` + :variable:`debug_ttl_rec_ts`.
The value can be +/- to simulate a record inserted in the past vs a record
inserted in the "future". A value of ``0`` denotes that the
variable is not set.
This variable is a no-op in non-debug builds.

.. variable:: rocksdb_debug_ttl_snapshot_ts

  :cli: ``--rocksdb_debug_ttl_ignore_pk``
  :dyn: Yes
  :scope: Global
  :vartype: Numeric
  :default: ``0``

For debugging purposes only.  Sets the snapshot during
compaction to ``now()`` + :variable:`rocksdb_debug_set_ttl_snapshot_ts`.
The value can be +/- to simulate a snapshot in the past vs a
snapshot created in the "future". A value of ``0`` denotes
that the variable is not set. This variable is a no-op in
non-debug builds.

.. variable:: rocksdb_default_cf_options

  :cli: ``--rocksdb-default-cf-options``
  :dyn: No
  :scope: Global
  :vartype: String
  :default: | block_based_table_factory= { cache_index_and_filter_blocks=1;
	    |                             filter_policy=bloomfilter:10:false;
	    |                             whole_key_filtering=1};
	    | level_compaction_dynamic_level_bytes=true;
	    | optimize_filters_for_hits=true;
	    | compaction_pri=kMinOverlappingRatio;
	    | compression=kLZ4Compression;
	    | bottommost_compression=kLZ4Compression;

Specifies the default column family options for |MyRocks|. On startup, the
server applies this option to all existing column families. This option is
read-only at runtime.

.. variable:: rocksdb_delayed_write_rate

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

  :cli: ``--rocksdb-delete-obsolete-files-period-micros``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: ``21600000000``

Specifies the period in microseconds to delete obsolete files
regardless of files removed during compaction.
Default value is ``21600000000`` (6 hours).
Allowed range is up to ``9223372036854775807``.

.. variable:: rocksdb_disable_file_deletions


  :cli: ``--rocksdb-disable-file-deletions``
  :dyn: Yes
  :scope: Session
  :vartype: Boolean
  :default: ``OFF``

This variable has been implemented in |Percona Server| :rn:`8.0.15-6`.
It allows a client to temporarily disable RocksDB deletion
of old ``WAL`` and ``.sst`` files for the purposes of making a consistent
backup. If the client session terminates for any reason after disabling
deletions and has not re-enabled deletions, they will be explicitly
re-enabled. This variable should be used by backup tools. Prolonged
use or other misuse can have serious side effects to the server instance.

.. variable:: rocksdb_enable_bulk_load_api

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

  :cli: ``--rocksdb-enable-ttl``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``ON``

Specifies whether to keep expired TTL records during compaction.
Enabled by default.
If disabled, expired TTL records will be dropped during compaction.

.. variable:: rocksdb_enable_ttl_read_filtering

  :cli: ``--rocksdb-enable-ttl-read-filtering``
  :dyn: Yes
  :scope: Global
  :vartype: Boolean
  :default: ``ON``

For tables with TTL, expired records are skipped/filtered
out during processing and in query results. Disabling
this will allow these records to be seen, but as a result
rows may disappear in the middle of transactions as they
are dropped during compaction. **Use with caution.**

.. variable:: rocksdb_enable_thread_tracking

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

  :cli: ``--rocksdb-error-if-exists``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Specifies whether to report an error when a database already exists.
Disabled by default.

.. variable:: rocksdb_flush_log_at_trx_commit

  :cli: ``--rocksdb-flush-log-at-trx-commit``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Numeric
  :default: ``1``

Specifies whether to sync on every transaction commit,
similar to |innodb_flush_log_at_trx_commit|_.
Enabled by default, which ensures ACID compliance.

.. |innodb_flush_log_at_trx_commit| replace:: ``innodb_flush_log_at_trx_commit``
.. _innodb_flush_log_at_trx_commit: https://dev.mysql.com/doc/refman/8.0/en/innodb-parameters.html#sysvar_innodb_flush_log_at_trx_commit

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

  :cli: ``--rocksdb-force-compute-memtable-stats-cachetime``
  :dyn: Yes
  :scope: Global
  :vartype: Numeric
  :default: 60000000

Specifies for how long the cached value of memtable statistics should
be used instead of computing it every time during the query plan analysis.

.. variable:: rocksdb_force_flush_memtable_and_lzero_now

  :cli: ``--rocksdb-force-flush-memtable-and-lzero-now``
  :dyn: Yes
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Works similar to :variable:`force_flush_memtable_now`
but also flushes all L0 files.

.. variable:: rocksdb_force_flush_memtable_now

  :cli: ``--rocksdb-force-flush-memtable-now``
  :dyn: Yes
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Forces MyRocks to immediately flush all memtables out to data files.

.. warning:: Use with caution!
   Write requests will be blocked until all memtables are flushed.

.. variable:: rocksdb_force_index_records_in_range

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

  :cli: ``--rocksdb-hash-index-allow-collision``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``ON``

Specifies whether hash collisions are allowed.
Enabled by default, which uses less memory.
If disabled, full prefix is stored to prevent hash collisions.

.. variable:: rocksdb_ignore_unknown_options

  :cli: ``--rocksdb-ignore-unknown-options``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``ON``

When enabled, it allows RocksDB to receive unknown options and not exit.

.. variable:: rocksdb_index_type

  :cli: ``--rocksdb-index-type``
  :dyn: No
  :scope: Global
  :vartype: Enum
  :default: ``kBinarySearch``

Specifies the type of indexing used by MyRocks:

* ``kBinarySearch``: Binary search (default).

* ``kHashSearch``: Hash search.

.. variable:: rocksdb_info_log_level

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

  :cli: ``--rocksdb-is-fd-close-on-exec``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``ON``

Specifies whether child processes should inherit open file jandles.
Enabled by default.

.. variable:: rocksdb_large_prefix

  :cli: ``--rocksdb-large-prefix``
  :dyn: Yes
  :scope: Global
  :vartype: Boolean
  :default: ``TRUE``

When enabled, this option allows index key prefixes longer than 767 bytes (up to
3072 bytes). This option mirrors the `innodb_large_prefix
<https://dev.mysql.com/doc/refman/8.0/en/innodb-parameters.html#sysvar_innodb_large_prefix>`_
The values for :variable:`rocksdb_large_prefix` should be the same between
master and slave.

.. note::

    In version 8.0.16-7 and later, the default value is changed to ``TRUE``.

.. variable:: rocksdb_keep_log_file_num

  :cli: ``--rocksdb-keep-log-file-num``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: ``1000``

Specifies the maximum number of info log files to keep.
Default value is ``1000``.
Allowed range is from ``1`` to ``18446744073709551615``.

.. variable:: rocksdb_lock_scanned_rows

  :cli: ``--rocksdb-lock-scanned-rows``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Boolean
  :default: ``OFF``

Specifies whether to hold the lock on rows that are scanned during ``UPDATE``
and not actually updated.
Disabled by default.

.. variable:: rocksdb_lock_wait_timeout

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

  :cli: ``--rocksdb-log-file-time-to-roll``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: ``0``

Specifies the period (in seconds) for rotating the info log files.
Default value is ``0``, meaning that the log file is not rotated.
Allowed range is up to ``18446744073709551615``.

.. variable:: rocksdb_manifest_preallocation_size

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

.. note::

   A value of ``4194304`` (4 MB) is reasonable to reduce random I/O on XFS.

.. variable:: rocksdb_manual_wal_flush

  :cli: ``--rocksdb-manual-wal-flush``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``ON``

This variable can be used to disable automatic/timed WAL flushing and instead
rely on the application to do the flushing.

.. variable:: rocksdb_max_background_compactions

  :cli: ``--rocksdb-max-background-compactions``
  :dyn: Yes
  :scope: Global
  :vartype: Numeric
  :default: ``1``

Specifies the maximum number of concurrent background compaction threads,
submitted to the low-priority thread pool.  Default value is ``1``. Allowed
range is up to ``64``.  This variable has been replaced with
:variable:`rocksdb_max_background_jobs`, which automatically decides how many
threads to allocate towards flush/compaction.

Replaced with :variable:`rocksdb_max_background_jobs`

.. variable:: rocksdb_max_background_flushes

  :cli: ``--rocksdb-max-background-flushes``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: ``1``

Specifies the maximum number of concurrent background memtable flush
threads, submitted to the high-priority thread-pool.  Default value is
``1``. Allowed range is up to ``64``.  This variable has been replaced
with :variable:`rocksdb_max_background_jobs`, which automatically
decides how many threads to allocate towards flush/compaction.

Replaced with :variable:`rocksdb_max_background_jobs`

.. variable:: rocksdb_max_background_jobs

  :cli: ``--rocksdb-max-background-jobs``
  :dyn: Yes
  :scope: Global
  :vartype: Numeric
  :default: ``2``

This variable replaced :variable:`rocksdb_base_background_compactions`,
:variable:`rocksdb_max_background_compactions`, and
:variable:`rocksdb_max_background_flushes` variables. This variable specifies
the maximum number of background jobs. It automatically decides
how many threads to allocate towards flush/compaction. It was implemented to
reduce the number of (confusing) options users and can tweak and push the
responsibility down to RocksDB level.

.. variable:: rocksdb_max_latest_deadlocks

  :cli: ``--rocksdb-max-latest-deadlocks``
  :dyn: Yes
  :scope: Global
  :vartype: Numeric
  :default: ``5``

Specifies the maximum number of recent deadlocks to store.

.. variable:: rocksdb_max_log_file_size

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

  :cli: ``--rocksdb-max-open-files``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: ``1000``

Specifies the maximum number of file handles opened by MyRocks.
Values in the range between ``0`` and ``open_files_limit``
are taken as they are. If :variable:`rocksdb_max_open_files` value is
greater than ``open_files_limit``, it will be reset to 1/2 of
``open_files_limit``, and a warning will be emitted to the ``mysqld``
error log. A value of ``-2`` denotes auto tuning: just sets
:variable:`rocksdb_max_open_files` value to 1/2 of ``open_files_limit``.
Finally, ``-1`` means no limit, i.e. an infinite number of file handles.

.. warning::

  Setting :variable:`rocksdb_max_open_files` to ``-1`` is dangerous,
  as server may quickly run out of file handles in this case.

.. variable:: rocksdb_max_row_locks

  :cli: ``--rocksdb-max-row-locks``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Numeric
  :default: ``1048576``

Specifies the limit on the maximum number of row locks a transaction can have
before it fails.
Default value is also the maximum, making it practically unlimited:
transactions never fail due to row locks.

.. variable:: rocksdb_max_subcompactions

  :cli: ``--rocksdb-max-subcompactions``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: ``1``

Specifies the maximum number of threads allowed for each compaction job.
Default value of ``1`` means no subcompactions (one thread per compaction job).
Allowed range is up to ``64``.

.. variable:: rocksdb_max_total_wal_size

  :cli: ``--rocksdb-max-total-wal-size``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: 2 GB

Specifies the maximum total size of WAL (write-ahead log) files,
after which memtables are flushed.
Default value is ``2 GB``
The allowed range is up to ``9223372036854775807``.

.. variable:: rocksdb_merge_buf_size

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

  :cli: ``--rocksdb_merge_tmp_file_removal_delay_ms``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Numeric
  :default: ``0``

Fast secondary index creation creates merge files when needed. After finishing
secondary index creation, merge files are removed. By default, the file removal
is done without any sleep, so removing GBs of merge files within <1s may
happen, which will cause trim stalls on Flash. This variable can be used to
rate limit the delay in milliseconds.

.. variable:: rocksdb_new_table_reader_for_compaction_inputs

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

  :cli: ``--rocksdb-no-block-cache``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Specifies whether to disable the block cache for column families.
Variable is disabled by default,
meaning that using the block cache is allowed.

.. variable:: rocksdb_no_create_column_family

  :cli: ``--rocksdb-no-create-column-family``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``ON``

Controls the processing of the |column-family| name given in the |sql.comment|
clause in the |sql.create-table| or |sql.alter-table| statement in case the |cf|
name does not refer to an existing |cf|.

If |opt.nccf| is set to `NO`, a new |cf| will be created and the new index will
be placed into it.

If |opt.nccf| is set to `YES`, no new |cf| will be created and the index will be
placed into the `default` |cf|. A warning is issued in this case informing that
the specified |cf| does not exist and cannot be created.

.. seealso::

   More information about column families
      :ref:`ps.myrocks.column-family`

.. variable:: rocksdb_override_cf_options

  :cli: ``--rocksdb-override-cf-options``
  :dyn: No
  :scope: Global
  :vartype: String
  :default:

Specifies option overrides for each column family.
Empty by default.

.. variable:: rocksdb_paranoid_checks

  :cli: ``--rocksdb-paranoid-checks``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``ON``

Specifies whether MyRocks should re-read the data file
as soon as it is created to verify correctness.
Enabled by default.

.. variable:: rocksdb_pause_background_work

  :cli: ``--rocksdb-pause-background-work``
  :dyn: Yes
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Specifies whether MyRocks should pause all background operations.
Disabled by default. There is no practical reason for a user to ever
use this variable because it is intended as a test synchronization tool
for the MyRocks MTR test suites.

.. warning::

  If someone were to set a :variable:`rocksdb_force_flush_memtable_now` to
  ``1`` while :variable:`rocksdb_pause_background_work` is set to ``1``,
  the client that issued the ``rocksdb_force_flush_memtable_now=1`` will be
  blocked indefinitely until :variable:`rocksdb_pause_background_work`
  is set to ``0``.

.. variable:: rocksdb_perf_context_level

  :cli: ``--rocksdb-perf-context-level``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Numeric
  :default: ``0``

Specifies the level of information to capture with the Perf Context plugins.
Default value is ``0``.
Allowed range is up to ``4``.

.. variable:: rocksdb_persistent_cache_path

  :cli: ``--rocksdb-persistent-cache-path``
  :dyn: No
  :scope: Global
  :vartype: String
  :default:

Specifies the path to the persistent cache.
Set this together with :variable:`rocksdb_persistent_cache_size_mb`.

.. variable:: rocksdb_persistent_cache_size_mb

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

  :cli: ``--rocksdb-pin-l0-filter-and-index-blocks-in-cache``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``ON``

Specifies whether MyRocks pins the filter and index blocks in the cache
if :variable:`rocksdb_cache_index_and_filter_blocks` is enabled.
Enabled by default.

.. variable:: rocksdb_print_snapshot_conflict_queries

  :cli: ``--rocksdb-print-snapshot-conflict-queries``
  :dyn: Yes
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Specifies whether queries that generate snapshot conflicts
should be logged to the error log.
Disabled by default.

.. variable:: rocksdb_rate_limiter_bytes_per_sec

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

  :cli: ``--rocksdb-records-in-range``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Numeric
  :default: ``0``

Specifies the value to override the result of ``records_in_range()``.
Default value is ``0``.
Allowed range is up to ``2147483647``.

.. variable:: rocksdb_reset_stats

  :cli: ``--rocksdb-reset-stats``
  :dyn: Yes
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Resets MyRocks internal statistics dynamically
(without restarting the server).

.. variable:: rocksdb_rpl_skip_tx_api

  :cli: ``--rocksdb-rpl-skip-tx-api``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Specifies whether write batches should be used for replication thread
instead of the transaction API.
Disabled by default.

There are two conditions which are necessary to
use it: row replication format and slave
operating in super read only mode.

.. variable:: rocksdb_seconds_between_stat_computes

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

  :cli: ``--rocksdb-signal-drop-index-thread``
  :dyn: Yes
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Signals the MyRocks drop index thread to wake up.

.. variable:: rocksdb_sim_cache_size

  :cli: ``--rocksdb-sim-cache-size``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: ``0``

Enables the simulated cache, which allows us to figure out the hit/miss rate
with a specific cache size without changing the real block cache.

.. variable:: rocksdb_skip_bloom_filter_on_read

  :cli: ``--rocksdb-skip-bloom-filter-on_read``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Boolean
  :default: ``OFF``

Specifies whether bloom filters should be skipped on reads.
Disabled by default (bloom filters are not skipped).

.. variable:: rocksdb_skip_fill_cache

  :cli: ``--rocksdb-skip-fill-cache``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Boolean
  :default: ``OFF``

Specifies whether to skip caching data on read requests.
Disabled by default (caching is not skipped).

.. variable:: rocksdb_sst_mgr_rate_bytes_per_sec

  :cli: ``--rocksdb-sst-mgr-rate-bytes-per-sec``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Numeric
  :default: ``0``

Specifies the maximum rate for writing to data files.
Default value is ``0``. This option is not effective on HDD.
Allowed range is from ``0`` to ``18446744073709551615``.

.. variable:: rocksdb_stats_dump_period_sec

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

  :cli: ``--rocksdb-store-row-debug-checksums``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Boolean
  :default: ``OFF``

Specifies whether to include checksums when writing index or table records.
Disabled by default.

.. variable:: rocksdb_strict_collation_check

  :cli: ``--rocksdb-strict-collation-check``
  :dyn: Yes
  :scope: Global
  :vartype: Boolean
  :default: ``ON``

Specifies whether to check and verify
that table indexes have proper collation settings.
Enabled by default.

.. variable:: rocksdb_strict_collation_exceptions

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

  :cli: ``--rocksdb-table-cache-numshardbits``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: ``6``

Specifies the number if table caches.
The default value is ``6``.
The allowed range is from ``0`` to ``19``.

.. variable:: rocksdb_table_stats_sampling_pct

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

  :cli: ``--rocksdb-tmpdir``
  :dyn: Yes
  :scope: Global, Session
  :vartype: String
  :default:

Specifies the path to the directory for temporary files during DDL operations.

.. variable:: rocksdb_trace_sst_api

  :cli: ``--rocksdb-trace-sst-api``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Boolean
  :default: ``OFF``

Specifies whether to generate trace output in the log
for each call to ``SstFileWriter``.
Disabled by default.

.. variable:: rocksdb_two_write_queues

  :cli: ``--rocksdb-two_write_queues``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``ON``

When enabled this variable allows/encourages threads that are using
two-phase commit to ``prepare`` in parallel.

.. variable:: rocksdb_unsafe_for_binlog

  :cli: ``--rocksdb-unsafe-for-binlog``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Boolean
  :default: ``OFF``

Specifies whether to allow statement-based binary logging
which may break consistency.
Disabled by default.

.. variable:: rocksdb_update_cf_options

  :cli: ``--rocksdb-update-cf-options``
  :dyn: No
  :scope: Global
  :vartype: String
  :default:

Specifies option updates for each column family.
Empty by default.

.. variable:: rocksdb_use_adaptive_mutex

  :cli: ``--rocksdb-use-adaptive-mutex``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Specifies whether to use adaptive mutex
which spins in user space before resorting to the kernel.
Disabled by default.

.. variable:: rocksdb_use_direct_io_for_flush_and_compaction

  :cli: ``--rocksdb-use-direct-io-for-flush-and-compaction``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Specifies whether to write to data files directly,
without caches or buffers.
Disabled by default.

.. variable:: rocksdb_use_direct_reads

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

  :cli: ``--rocksdb-use-fsync``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

Specifies whether MyRocks should use ``fsync`` instead of ``fdatasync``
when requesting a sync of a data file.
Disabled by default.

.. variable:: rocksdb_validate_tables

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

  :cli: ``--rocksdb-verify-row-debug-checksums``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Boolean
  :default: ``OFF``

Specifies whether to verify checksums when reading index or table records.
Disabled by default.

.. variable:: rocksdb_wal_bytes_per_sync

  :cli: ``--rocksdb-wal-bytes-per-sync``
  :dyn: Yes
  :scope: Global
  :vartype: Numeric
  :default: ``0``

Specifies how often should the OS sync WAL (write-ahead log) files to disk
as they are being written, asynchronously, in the background.
This operation can be used to smooth out write I/O over time.
Default value is ``0``, meaning that files are never synced.
Allowed range is up to ``18446744073709551615``.

.. variable:: rocksdb_wal_dir

  :cli: ``--rocksdb-wal-dir``
  :dyn: No
  :scope: Global
  :vartype: String
  :default:

Specifies the path to the directory where MyRocks stores WAL files.

.. variable:: rocksdb_wal_recovery_mode

  :cli: ``--rocksdb-wal-recovery-mode``
  :dyn: Yes
  :scope: Global
  :vartype: Numeric
  :default: ``1``

Specifies the level of tolerance when recovering write-ahead logs (WAL) files
after a system crash.

The following are the options:

 * ``0``: if the last WAL entry is corrupted, truncate the entry and either start the server normally or refuse to start.

 * ``1`` (default): if a WAL entry is corrupted, the server fails to   start and does not recover from the crash.

 * ``2``: if a corrupted WAL entry is detected, truncate all entries after the detected corrupted entry. You can select this setting for replication slaves.

 * ``3``: If a corrupted WAL entry is detected, skip only the corrupted entry and continue the apply WAL entries. This option can be dangerous.

.. variable:: rocksdb_wal_size_limit_mb

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

  :cli: ``--rocksdb-wal-ttl-seconds``
  :dyn: No
  :scope: Global
  :vartype: Numeric
  :default: ``0``

Specifies the timeout in seconds before deleting archived WAL files.
Default is ``0`` (archived WAL files are never deleted).
Allowed range is up to ``9223372036854775807``.

.. variable:: rocksdb_whole_key_filtering

  :cli: ``--rocksdb-whole-key-filtering``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``ON``

Specifies whether the bloomfilter should use the whole key for filtering
instead of just the prefix.
Enabled by default.
Make sure that lookups use the whole key for matching.

.. variable:: rocksdb_write_batch_max_bytes

  :cli: ``--rocksdb-write-batch-max-bytes``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Numeric
  :default: ``0``

Specifies the maximum size of a RocksDB write batch in bytes. ``0`` means no
limit. In case user exceeds the limit following error will be shown:
``ERROR HY000: Status error 10 received from RocksDB: Operation aborted: Memory
limit reached``.

.. variable:: rocksdb_write_disable_wal

  :cli: ``--rocksdb-write-disable-wal``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Boolean
  :default: ``OFF``

Lets you temporarily disable writes to WAL files,
which can be useful for bulk loading.

.. variable:: rocksdb_write_ignore_missing_column_families

  :cli: ``--rocksdb-write-ignore-missing-column-families``
  :dyn: Yes
  :scope: Global, Session
  :vartype: Boolean
  :default: ``OFF``

Specifies whether to ignore writes to column families that do not exist.
Disabled by default (writes to non-existent column families are not ignored).

.. include:: ../.res/replace.opt.txt
.. include:: ../.res/replace.concept.txt
