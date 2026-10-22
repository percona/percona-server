# MyRocks Changelog

## MyRocks 9.7.4-1
**This version was shipped with Percona Server 8.4.13-12 and 9.7.4-3.**
* Updates RocksDB submodule to 9.7.4.
* PS-9766 – Ported MyRocks changes from the Facebook MySQL 202407 release
· [Jira](https://perconadev.atlassian.net/browse/PS-9766)

### New Features:

* Added the `SHOW ENGINE ROCKSDB TRANSACTION STATUS` statement. It lists each session's snapshot state, age, and lock and write counts, followed by the most recently detected deadlocks. It requires the `PROCESS` privilege.
* Added the `INFORMATION_SCHEMA.ROCKSDB_BULK_LOAD_HISTORY` table, which lists active and recently completed bulk-load sessions.

### Improvements:

* When `rocksdb_corrupt_data_action=ABORT_SERVER` aborts the server, the error log names the database and query that hit the corrupted data.
* Lock and snapshot conflicts no longer write `Got error 522` entries to the error log. The client still gets `ER_LOCK_DEADLOCK`.
* The MyRocks index statistics thread is visible in `performance_schema.threads`.

### Changes in Behavior:

* `ALTER TABLE`, `DROP TABLE`, `RENAME TABLE` and `TRUNCATE TABLE` fail on a table that has an unfinished bulk load, also in the session that loads it, and a bulk load fails on a table with an active `ALTER TABLE` or `DROP TABLE`, with `ER_LOCK_REFUSED_BY_ENGINE`. DDL of other tables is not affected. `rocksdb_skip_bulk_load_notify_ddl=ON` disables the check.
* `ALTER TABLE ... ALGORITHM=COPY` with `rocksdb_bulk_load=ON` checks a new unique key for duplicates, also with `rocksdb_bulk_load_enable_unique_key_check=OFF`, and fails with `ER_DUP_ENTRY`. Previously the `ALTER TABLE` succeeded and the unique index was left empty.
* `rocksdb_read_free_rpl_tables` is honored again: a table uses read-free replication only if its name matches the variable. Previously the variable was accepted but ignored. The default `.*` matches every table. The pattern is matched against the table name as written in SQL, such as `t-1`. An invalid pattern given at startup makes MyRocks fail to initialize: the server starts without the `ROCKSDB` engine or, with `--rocksdb=FORCE`, does not start.
* Read-free replication is not used for row events applied with `replica_exec_mode=IDEMPOTENT`, so a diverged replica no longer keeps extra primary key rows and stale secondary index entries.
* A transaction no longer hides additional TTL-expired rows while it runs: TTL filtering uses the time of the transaction's first snapshot, also after a statement rollback or a released snapshot.
* An XA transaction uses the RocksDB transaction API also with `rocksdb_master_skip_tx_api` or `rocksdb_rpl_skip_tx_api`, because a write batch cannot be prepared. Previously `XA PREPARE` silently lost the MyRocks writes of such a transaction.

### Bug Fixes:

* PS-10110 – Fixed poor MyRocks query plans caused by 1-row range estimates by refining small `records_in_range()` estimates with a bounded index dive
· [Jira](https://perconadev.atlassian.net/browse/PS-10110) · [Changes](https://github.com/percona/percona-server/commit/9c8b21e9ce52)
* PS-11881 – Fixed a server crash on `SET rocksdb_read_free_rpl_tables` with a deeply nested pattern, and `CREATE TABLE` hanging, while blocking all other MyRocks DDL, on a pattern with catastrophic backtracking. The patterns are now compiled and matched with ICU, as the `REGEXP` functions are, under `regexp_time_limit` and `regexp_stack_limit`; a match that fails counts as no match. The dialect changes from POSIX extended to ICU; patterns made of names, `.`, `*`, alternation and character classes behave the same.
· [Jira](https://perconadev.atlassian.net/browse/PS-11881)
* PS-11903 – Fixed a replicated `UPDATE` applied with read-free replication writing a wrong TTL timestamp for a table with an implicit TTL (`ttl_duration` without `ttl_col`), which made rows expire early or late on the replica and left its primary and secondary indexes inconsistent. Replicated `UPDATE` events of such tables no longer use read-free replication.
· [Jira](https://perconadev.atlassian.net/browse/PS-11903)
* PS-11631 – Fixed a server crash on the next statement in a session after `XA PREPARE` and `XA COMMIT` or `XA ROLLBACK` of a MyRocks transaction. A prepared XA transaction also survives the disconnect of its session, with `xa_detach_on_prepare=OFF`, instead of being rolled back.
· [Jira](https://perconadev.atlassian.net/browse/PS-11631)
* PS-11632 – Fixed prepared MyRocks XA transactions being rolled back at server restart, so that a later `XA COMMIT` lost their changes. A prepared XA transaction is kept, with its row locks, until `XA COMMIT` or `XA ROLLBACK`, and `TRUNCATE`, `ALTER` and `DROP TABLE` wait for it. With the binary log off and `rocksdb_flush_log_at_trx_commit=2`, `XA PREPARE`, `XA COMMIT` and `XA ROLLBACK` of such a transaction are written to the OS, so they survive a crash of the server process. `XA PREPARE` no longer raises Warning 1031.
· [Jira](https://perconadev.atlassian.net/browse/PS-11632)
* A replicated `UPDATE` applied with read-free replication that changed the TTL column of a table with `ttl_col` could leave the secondary keys with the old TTL timestamp, so the row expired early or late in its secondary indexes.
* Partial indexes can be created on tables with TTL. Previously this was rejected with `Partial index cannot have TTL`.
* A read that fails inside RocksDB, for example with an I/O error during a scan, returns an error instead of ending the scan early with partial results.
* With `rocksdb_enable_delete_range_for_drop_index=ON`, the key ranges of dropped tables and indexes are deleted by the background drop-index thread after the DDL statement completes. If RocksDB fails to delete them or to flush the WAL, `rocksdb_io_error_action` applies: the server aborts, or the cleanup is retried later. Previously the deletion ran within the statement and its failure was ignored.
* If RocksDB creates a column family but fails to persist its options, the error is returned to the client and the column family is dropped again. Previously it stayed in RocksDB unknown to MyRocks.
* If repairing a table left by an interrupted `ALTER TABLE` fails at startup, the original table definition is kept, so the repair can be retried after a restart.
* With `rocksdb_enable_tmp_table=ON`, committing an insert into a temporary table with an `AUTO_INCREMENT` column no longer aborts the server.
* `SET rocksdb_create_temporary_checkpoint = DEFAULT` no longer makes the session lose track of its temporary checkpoint, which left the checkpoint directory behind and allowed a second one to be created.
* A `SET` statement with several `rocksdb_update_cf_options` assignments no longer loses the earlier option updates.
* `performance_schema.log_status` lists only live MyRocks WAL files, not archived ones.
* With `rocksdb_allow_mmap_writes=ON` and `rocksdb_flush_log_at_trx_commit=0`, `FLUSH LOGS` no longer fails. It no longer requests a WAL sync, which memory-mapped WAL files do not support, matching the background WAL flush.
* MyRocks messages longer than 1 KiB are no longer truncated in the error log and the RocksDB `LOG` file.
* With `rocksdb_verify_row_debug_checksums=ON`, checksums are verified also for rows that have no columns outside the primary key.
* A failure to write an SST file while bulk loading a table with a partial index reports its error code instead of a generic error.
* Changing `rocksdb_read_free_rpl` or `rocksdb_read_free_rpl_tables` while a replica applies a multi-row `UPDATE` or `DELETE` event no longer makes the replica use the key of the previous row.
* `SET GLOBAL rocksdb_delete_cf` refuses the temporary-table column families `__tmp__` and `__tmp_system__`. Previously dropping `__tmp_system__` made the next `CREATE TEMPORARY TABLE` crash the server.
* If the background drop of a column family fails only when persisting the options file, the column family is still forgotten. Previously MyRocks kept the dropped column family, so the next write to it aborted the server.
* A failed in-place `ALTER TABLE ... ADD INDEX` no longer leaves its sorted keys in the session. Previously the next in-place index creation in the same session failed, or, when it created several indexes, could leave one of them empty.
* A change of `rocksdb_master_skip_tx_api` or `rocksdb_rpl_skip_tx_api` takes effect at the next transaction of a session that already used MyRocks. Previously the session kept using the kind of transaction it had first created.
* Recovering a prepared XA transaction after a crash no longer logs a false failure to prepare it.
* A dump written by `mysqldump` sets `rocksdb_bulk_load` back to its previous value at its end. Previously the end of the dump referred to variables that were never set, so the bulk load of the last MyRocks table stayed open until the restoring session disconnected.
* With `rocksdb_write_batch_mem_free_threshold`, the memory a write batch keeps after a rollback to a savepoint is counted, and write batches of `rocksdb_master_skip_tx_api` and `rocksdb_rpl_skip_tx_api` are freed too.

### Changes in RocksDB 9.4.0 to 9.7.4:

* Reverse lookups and `ORDER BY ... DESC` scans (`SeekForPrev`) on a column family with a prefix bloom filter, `whole_key_filtering=false` and partitioned filters no longer miss rows. SST files written by an earlier version keep the defect until they are rewritten: run a full compaction of such column families (`SET GLOBAL rocksdb_compact_cf='<cf>'`) after the upgrade.
* After `rocksdb_update_cf_options` changes the `prefix_extractor` of a column family with `memtable_prefix_bloom_size_ratio` above 0, reads no longer miss rows that are still in the memtable.
* A checkpoint, created by `rocksdb_create_checkpoint` or for a clone, keeps the RocksDB DB ID of its source. Checkpoints no longer sync the live WAL and contain one extra empty WAL file.
* With `rocksdb_io_error_action=IGNORE_ERROR`, a WAL write error is fatal: MyRocks refuses writes until the server restarts, also after the cause is fixed. A commit that fails on a WAL error is no longer visible to other sessions or after a restart. The default `ABORT_SERVER` is not affected.

### Downgrade:

* The OPTIONS file written by RocksDB 9.7.4 has options that RocksDB 9.3.1 does not know. An earlier release loads it only with `rocksdb_ignore_unknown_options=ON`, the default; with `OFF` the `ROCKSDB` engine fails to load.
* A table with a partial index and TTL cannot be used by an earlier release, which rejects it, and which writes partial-index entries that its compaction then cannot decode, aborting the server. Drop the partial index, or the table, before a downgrade.
* An earlier release rolls back prepared MyRocks XA transactions at startup, and ignores the markers that keep them prepared. Commit or roll them back before a downgrade.

### New MyRocks Variables:

* `rocksdb_bulk_load_history_size` – Number of completed bulk-load sessions kept in `INFORMATION_SCHEMA.ROCKSDB_BULK_LOAD_HISTORY` (default 10, maximum 1024).
* `rocksdb_enable_autoinc_compat_mode` – Lets simple inserts allocate consecutive auto-increment values, as InnoDB does with `innodb_autoinc_lock_mode=2` (default OFF).
* `rocksdb_gap_lock_raise_error` – How MyRocks handles queries that would need unsupported gap locks: `OFF` allows them, `WARNING` allows them with one warning per statement, `ERROR` rejects them (default ERROR).
* `rocksdb_max_trash_db_ratio_pct` – `max_trash_db_ratio` of the SST file manager, in percent (default 100000000000). File deletion speed is still limited by `rocksdb_sst_mgr_rate_bytes_per_sec`.
* `rocksdb_records_in_range_dive_threshold` – If the `records_in_range()` row estimate is below this value, refine it by scanning the index range, reading at most this many keys (default 100, 0 disables).
* `rocksdb_skip_bulk_load_notify_ddl` – Skips the check for conflicts between DDL and active bulk loads (default OFF).
* `rocksdb_write_batch_mem_free_threshold` – Frees a write batch instead of reusing it when its allocated memory is above this many bytes; 0 means no limit (default 0).

### Removed MyRocks Variables:

A configuration file that still sets one of these variables must remove it or use the `loose-` prefix, or the server fails to start.

* `rocksdb_disable_instant_ddl` – It had no effect and was deprecated in MyRocks 9.3.1-1. Use `rocksdb_enable_instant_ddl` and the `rocksdb_enable_instant_ddl_for_*` variables.
* `rocksdb_enable_udt_in_mem` – It had no effect.
* `rocksdb_error_on_suboptimal_collation` – It has had no effect since Percona Server 8.0.23-14.
* `rocksdb_fault_injection_options`
* `rocksdb_seconds_between_stat_computes` – It had no effect.

## MyRocks 9.3.1-5
**This version was shipped with Percona Server 8.0.46-37, 8.4.10-10, and 9.7.1-1.**
* PS-11142 – Fix gcc-16 compilation issues with c++23
· [Jira](https://perconadev.atlassian.net/browse/PS-11142) · [Changes](https://github.com/percona/percona-server/commit/d246ab773875)
* PS-11142 – Fix clang-22, gcc 11-16 / libstdc++ 11-16 compilation issues in RocksDB submodule
· [Jira](https://perconadev.atlassian.net/browse/PS-11142) · [Changes](https://github.com/percona/rocksdb/commit/d0e615524)

## MyRocks 9.3.1-4
**This version was shipped with Percona Server 8.0.45-36 and 8.4.8-8.**
* PS-10287 – Add automatic table definition recovery after crash on ALTER
· [Jira](https://perconadev.atlassian.net/browse/PS-10287) · [Changes](https://github.com/percona/percona-server/commit/a30a6c6a7ecd)

## MyRocks 9.3.1-3
**This version was shipped with Percona Server 8.0.44-35 and 8.4.7-7.**
* PS-9680 – Fix potential data corruption in MyRocks after RocksDB 7.10.0 changes
· [Jira](https://perconadev.atlassian.net/browse/PS-9680) · [Changes](https://github.com/percona/percona-server/commit/247b3ee5a29c)

* PS-10227 – Fixed an assertion failure occurring during RocksDB initialization when system column family statistics were skipped
· [Jira](https://perconadev.atlassian.net/browse/PS-10227) · [Changes](https://github.com/percona/percona-server/commit/04ed7d4f075c)

* PS-9840 – Prevents an assertion failure when a metadata table (DD table) exists but isn’t registered in RocksDB
· [Jira](https://perconadev.atlassian.net/browse/PS-9840) · [Changes](https://github.com/percona/percona-server/commit/805737fdaf9e)

* PS-9838 – Fixed an assertion failure in `guess_rec_per_key()` by correcting the record count estimation formula
· [Jira](https://perconadev.atlassian.net/browse/PS-9838) · [Changes](https://github.com/percona/percona-server/commit/d4ae78b9f753)

* PS-10210 – Prevented misconfigurations by aligning MyRocks variable ranges with internal RocksDB limits
· [Jira](https://perconadev.atlassian.net/browse/PS-10210) · [Changes](https://github.com/percona/percona-server/commit/48d6a1b1fc20)

* PS-10075 – Resolved checksum mismatch errors in key-value pairs by properly handling per-field metadata during record unpacking
· [Jira](https://perconadev.atlassian.net/browse/PS-10075) · [Changes](https://github.com/percona/percona-server/commit/7c34db4ef730)

* PS-10067 – Improved handling of per-field metadata in `Rdb_convert_to_record_key_decoder::skip()`
· [Jira](https://perconadev.atlassian.net/browse/PS-10067) · [Changes](https://github.com/percona/percona-server/commit/b2109a4fe1c8)

* PS-9842 – Invalidate iterator on error to prevent crash in MergingIterator::CurrentForward()
· [Jira](https://perconadev.atlassian.net/browse/PS-9842) · [Changes](https://github.com/percona/percona-server/commit/18993f876eba)

## MyRocks 9.3.1-2
**This version was shipped with Percona Server 8.0.43-34 and 8.4.6-6.**
* PS-9666 – Fixed a crash when inserting into a TTL-enabled table with `unique_checks=OFF`
· [Jira](https://perconadev.atlassian.net/browse/PS-9666) · [Changes](https://github.com/percona/percona-server/commit/ecc87980002a)

* PS-10067 – Prevented overflow in `unpack_unknown_varlength()` that could cause crashes
· [Jira](https://perconadev.atlassian.net/browse/PS-10067) · [Changes](https://github.com/percona/percona-server/commit/37a50cc41d93)


## MyRocks 9.3.1-1
**This version was shipped with Percona Server 8.0.42-33 and 8.4.5-5.**
* Updates RocksDB submodule to 9.3.1.

### New MyRocks Variables:

* `rocksdb_bulk_load_compression_parallel_threads` – Number of threads for parallel compression during bulk load.
* `rocksdb_bulk_load_enable_unique_key_check` – Enable checking for unique keys during bulk load.
* `rocksdb_debug_skip_bloom_filter_check_on_iterator_bounds` – Skip bloom filter checks on iterator bounds (debug).
* `rocksdb_enable_udt_in_mem` – Enable use of user-defined timestamps (UDTs) in memtables.
* `rocksdb_invalid_create_option_action` – Action when an invalid create option is encountered.
* `rocksdb_io_error_action` – Action when an I/O error occurs.
* `rocksdb_table_stats_skip_system_cf` – Skip stats collection for system column families.
* `rocksdb_use_io_uring` – Enable io_uring for asynchronous I/O.
* `rocksdb_enable_instant_ddl` – Enable instant DDL operations.
* `rocksdb_enable_instant_ddl_for_append_column` – Enable instant DDL for appending columns.
* `rocksdb_enable_instant_ddl_for_column_default_changes` – Enable instant DDL for column default changes.
* `rocksdb_enable_instant_ddl_for_drop_index_changes` – Enable instant DDL for dropping indexes.
* `rocksdb_enable_instant_ddl_for_table_comment_changes` – Enable instant DDL for table comment changes.

### Changes to Default Values of MyRocks Variables:

* `rocksdb_disable_instant_ddl`: Default value changed from **ON** to **OFF**.
* `rocksdb_file_checksums`: Data type changed from **Boolean** to **ENUM**. Default value changed from **OFF** to **CHECKSUMS_OFF**.
* `rocksdb_compaction_readahead_size`: Default value changed from **0** to **2,097,152**.

### Deprecated MyRocks Variable:

* `rocksdb_disable_instant_ddl` – This variable is deprecated and expected to be removed in a future release.

### Removed MyRocks Variables:

* `rocksdb_access_hint_on_compaction_start`
* `rocksdb_large_prefix`
* `rocksdb_strict_collation_check` (Percona Server 8.0 only; already removed in 8.4)
* `rocksdb_strict_collation_exceptions` (Percona Server 8.0 only; already removed in 8.4)


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
