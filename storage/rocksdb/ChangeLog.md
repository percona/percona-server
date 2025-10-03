# MyRocks Changelog

## MyRocks 9.3.1-3
**This version was shipped with Percona Server 8.0.44-35 and 8.4.7-7.**
* PS-9840 – Prevents an assertion failure when a metadata table (DD table) exists but isn’t registered in RocksDB
· [Jira](https://perconadev.atlassian.net/browse/PS-9840)

* PS-9838 – Fixed an assertion failure in `guess_rec_per_key()` by correcting the record count estimation formula.
· [Jira](https://perconadev.atlassian.net/browse/PS-9838) · [Changes](https://github.com/percona/percona-server/commit/d4ae78b9f753)

* PS-10210 – Prevented misconfigurations by aligning MyRocks variable ranges with internal RocksDB limits.
· [Jira](https://perconadev.atlassian.net/browse/PS-10210) · [Changes](https://github.com/percona/percona-server/commit/48d6a1b1fc20)

* PS-10075 – Resolved checksum mismatch errors in key-value pairs by properly handling per-field metadata during record unpacking.
· [Jira](https://perconadev.atlassian.net/browse/PS-10075) · [Changes](https://github.com/percona/percona-server/commit/7c34db4ef730)

* PS-10067 – Improved handling of per-field metadata in `Rdb_convert_to_record_key_decoder::skip()`.
· [Jira](https://perconadev.atlassian.net/browse/PS-10067) · [Changes](https://github.com/percona/percona-server/commit/b2109a4fe1c8)

## MyRocks 9.3.1-2
**This version was shipped with Percona Server 8.0.43-34 and 8.4.6-6.**
* PS-9666 – Fixed a crash when inserting into a TTL-enabled table with `unique_checks=OFF`.
· [Jira](https://perconadev.atlassian.net/browse/PS-9666) · [Changes](https://github.com/percona/percona-server/commit/ecc87980002a)

* PS-10067 – Prevented overflow in `unpack_unknown_varlength()` that could cause crashes.
· [Jira](https://perconadev.atlassian.net/browse/PS-10067) · [Changes](https://github.com/percona/percona-server/commit/37a50cc41d93)


## MyRocks 9.3.1-1
**This version was shipped with Percona Server 8.0.42-33 and 8.4.5-5.**
* Updates RocksDB submodule to 9.3.1.

### New MyRocks Variables:

* `rocksdb_bulk_load_compression_parallel_threads` – Number of threads for parallel compression during bulk load.
* `rocksdb_bulk_load_enable_unique_key_check` – Enable checking for unique keys during bulk load.
* `rocksdb_debug_skip_bloom_filter_check_on_iterator_bounds` – Skip bloom filter checks on iterator bounds (debug).
* `rocksdb_enable_udt_in_mem` – Enable use of user-defined types (UDTs) in memory.
* `rocksdb_invalid_create_option_action` – Action when an invalid create option is encountered.
* `rocksdb_io_error_action` – Action when an I/O error occurs.
* `rocksdb_table_stats_skip_system_cf` – Skip stats collection for system column families.
* `rocksdb_use_io_uring` – Enable io_uring for asynchronous I/O.
* `rocksdb_enable_instant_ddl` – Enable instant DDL operations.
* `rocksdb_enable_instant_ddl_for_append_column` – Enable instant DDL for appending columns.
* `rocksdb_enable_instant_ddl_for_column_default_changes` – Enable instant DDL for column default changes.
* `rocksdb_enable_instant_ddl_for_drop_index_changes` – Enable instant DDL for dropping indexes.
* `rocksdb_enable_instant_ddl_for_table_comment_changes` – Enable instant DDL for table comment changes.
* `rocksdb-bulk-load-compression-parallel-threads` – Alias for `rocksdb_bulk_load_compression_parallel_threads`.
* `rocksdb-bulk-load-enable-unique-key-check` – Alias for `rocksdb_bulk_load_enable_unique_key_check`.
* `rocksdb-debug-skip-bloom-filter-check-on-iterator-bounds` – Alias for `rocksdb_debug_skip_bloom_filter_check_on_iterator_bounds`.

### Changes to Default Values of MyRocks Variables:

* `rocksdb_disable_instant_ddl`: Default value changed from **ON** to **OFF**.
* `rocksdb_file_checksums`: Data type changed from **Boolean** to **ENUM**. Default value changed from **OFF** to **CHECKSUMS_OFF**.
* `rocksdb_compaction_readahead_size`: Default value changed from **0** to **2,097,152**.

### Deprecated MyRocks Variable:

* `rocksdb_disable_instant_ddl` – This variable is deprecated and expected to be removed in a future release.

### Removed MyRocks Variables:

* `rocksdb-access-hint-on-compaction-start`
* `rocksdb_large_prefix`
* `rocksdb_strict_collation_check`
* `rocksdb_strict_collation_exceptions`


## MyRocks 8.5.1-1
**This version was shipped with Percona Server 8.0.36-28 and 8.4.0-1.**
* Updates RocksDB submodule to 8.5.1.

### New MyRocks Variables:

* `rocksdb_block_cache_numshardbits` – Number of shards for block cache.
* `rocksdb_check_iterate_bounds` – Enable bounds checking for iterators.
* `rocksdb_compact_lzero_now` – Trigger immediate compaction of L0 files.
* `rocksdb_file_checksums` – Enable file checksums.
* `rocksdb_max_file_opening_threads` – Maximum number of threads for opening files.
* `rocksdb_partial_index_ignore_killed` – Ignore killed partial indexes during operations.

### Changes to Default Values of MyRocks Variables:

* `rocksdb_compaction_sequential_deletes`: Default value changed from **0** to **14999**.
* `rocksdb_compaction_sequential_deletes_count_sd`: Default value changed from **OFF** to **ON**.
* `rocksdb_compaction_sequential_deletes_window`: Default value changed from **0** to **15000**.
* `rocksdb_force_flush_memtable_now`: Default value changed from **ON** to **OFF**.
* `rocksdb_large_prefix`: Default value changed from **OFF** to **ON**.

### Deprecated MyRocks Variable:

* `rocksdb_large_prefix` – Deprecated and expected to be removed in a future release.


## MyRocks 7.10.2-1
**This version was shipped with Percona Server 8.0.33-25.**
* Updates RocksDB submodule to 7.10.2.

### New MyRocks Variables:

* `rocksdb_bulk_load_use_sst_partitioner` – Use SST partitioner during bulk load.
* `rocksdb_use_hyper_clock_cache` – Enable hyper clock cache.
* `rocksdb_converter_record_cached_length` – Cached length for record converter.
* `rocksdb_alter_table_comment_inplace` – Allow altering table comment inplace.
* `rocksdb_charge_memory` – Enable memory charging for operations.
* `rocksdb_column_default_value_as_expression` – Treat column default value as expression.
* `rocksdb_corrupt_data_action` – Action to take on corrupt data.
* `rocksdb_disable_instant_ddl` – Disable instant DDL operations.
* `rocksdb_enable_delete_range_for_drop_index` – Enable DeleteRange for dropping indexes.
* `rocksdb_partial_index_blind_delete` – Enable blind delete for partial indexes.
* `rocksdb_protection_bytes_per_key` – Set protection bytes per key.
* `rocksdb_use_write_buffer_manager` – Enable write buffer manager.

### New Information Schema Table:

* `ROCKSDB_LIVE_FILES_METADATA`

### Removed MyRocks Variables:

* `rocksdb_instant_ddl` – Use `rocksdb_disable_instant_ddl` instead.


## MyRocks 7.0.1-1
**This version was shipped with Percona Server 8.0.29-21.**
* Updates RocksDB submodule to 7.0.1.

### New MyRocks Variables:

* `rocksdb_enable_tmp_table` – Enable the use of temporary tables in RocksDB.
* `rocksdb_instant_ddl` – Enable instant DDL operations to apply schema changes without full table rebuild.

### Changes to Default Values of MyRocks Variables:

* `rocksdb_force_flush_memtable_now`: Default value changed to **ON**.

### Removed MyRocks Variables:

* `rocksdb_hash_index_allow_collision` – Deprecated variable for hash index collisions.
* `rocksdb_new_table_reader_for_compaction_inputs` – Deprecated variable for using new table reader during compaction.
