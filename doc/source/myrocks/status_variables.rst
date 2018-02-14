.. _myrocks_status_variables:

========================
MyRocks Status Variables
========================

MyRocks status variables provide details
about the inner workings of the storage engine
and they can be useful in tuning the storage engine
to a particular environment.

You can view these variables and their values by running:

.. code-block:: mysql

  mysql> SHOW STATUS LIKE 'rocksdb%';

The following global status variables are available:

.. tabularcolumns:: |p{9cm}|p{6cm}|

.. list-table::
   :header-rows: 1

   * - Name
     - Var Type
   * - :variable:`rocksdb_rows_deleted`
     - Numeric
   * - :variable:`rocksdb_rows_inserted`
     - Numeric
   * - :variable:`rocksdb_rows_read`
     - Numeric
   * - :variable:`rocksdb_rows_updated`
     - Numeric
   * - :variable:`rocksdb_rows_expired`
     - Numeric
   * - :variable:`rocksdb_system_rows_deleted`
     - Numeric
   * - :variable:`rocksdb_system_rows_inserted`
     - Numeric
   * - :variable:`rocksdb_system_rows_read`
     - Numeric
   * - :variable:`rocksdb_system_rows_updated`
     - Numeric
   * - :variable:`rocksdb_memtable_total`
     - Numeric
   * - :variable:`rocksdb_memtable_unflushed`
     - Numeric
   * - :variable:`rocksdb_queries_point`
     - Numeric
   * - :variable:`rocksdb_queries_range`
     - Numeric
   * - :variable:`rocksdb_covered_secondary_key_lookups`
     - Numeric
   * - :variable:`rocksdb_block_cache_add`
     - Numeric
   * - :variable:`rocksdb_block_cache_add_failures`
     - Numeric
   * - :variable:`rocksdb_block_cache_bytes_read`
     - Numeric
   * - :variable:`rocksdb_block_cache_bytes_write`
     - Numeric
   * - :variable:`rocksdb_block_cache_data_add`
     - Numeric
   * - :variable:`rocksdb_block_cache_data_bytes_insert`
     - Numeric
   * - :variable:`rocksdb_block_cache_data_hit`
     - Numeric
   * - :variable:`rocksdb_block_cache_data_miss`
     - Numeric
   * - :variable:`rocksdb_block_cache_filter_add`
     - Numeric
   * - :variable:`rocksdb_block_cache_filter_bytes_evict`
     - Numeric
   * - :variable:`rocksdb_block_cache_filter_bytes_insert`
     - Numeric
   * - :variable:`rocksdb_block_cache_filter_hit`
     - Numeric
   * - :variable:`rocksdb_block_cache_filter_miss`
     - Numeric
   * - :variable:`rocksdb_block_cache_hit`
     - Numeric
   * - :variable:`rocksdb_block_cache_index_add`
     - Numeric
   * - :variable:`rocksdb_block_cache_index_bytes_evict`
     - Numeric
   * - :variable:`rocksdb_block_cache_index_bytes_insert`
     - Numeric
   * - :variable:`rocksdb_block_cache_index_hit`
     - Numeric
   * - :variable:`rocksdb_block_cache_index_miss`
     - Numeric
   * - :variable:`rocksdb_block_cache_miss`
     - Numeric
   * - :variable:`rocksdb_block_cache_compressed_hit`
     - Numeric
   * - :variable:`rocksdb_block_cache_compressed_miss`
     - Numeric
   * - :variable:`rocksdb_bloom_filter_prefix_checked`
     - Numeric
   * - :variable:`rocksdb_bloom_filter_prefix_useful`
     - Numeric
   * - :variable:`rocksdb_bloom_filter_useful`
     - Numeric
   * - :variable:`rocksdb_bytes_read`
     - Numeric
   * - :variable:`rocksdb_bytes_written`
     - Numeric
   * - :variable:`rocksdb_compact_read_bytes`
     - Numeric
   * - :variable:`rocksdb_compact_write_bytes`
     - Numeric
   * - :variable:`rocksdb_compaction_key_drop_new`
     - Numeric
   * - :variable:`rocksdb_compaction_key_drop_obsolete`
     - Numeric
   * - :variable:`rocksdb_compaction_key_drop_user`
     - Numeric
   * - :variable:`rocksdb_flush_write_bytes`
     - Numeric
   * - :variable:`rocksdb_get_hit_l0`
     - Numeric
   * - :variable:`rocksdb_get_hit_l1`
     - Numeric
   * - :variable:`rocksdb_get_hit_l2_and_up`
     - Numeric
   * - :variable:`rocksdb_get_updates_since_calls`
     - Numeric
   * - :variable:`rocksdb_iter_bytes_read`
     - Numeric
   * - :variable:`rocksdb_memtable_hit`
     - Numeric
   * - :variable:`rocksdb_memtable_miss`
     - Numeric
   * - :variable:`rocksdb_no_file_closes`
     - Numeric
   * - :variable:`rocksdb_no_file_errors`
     - Numeric
   * - :variable:`rocksdb_no_file_opens`
     - Numeric
   * - :variable:`rocksdb_num_iterators`
     - Numeric
   * - :variable:`rocksdb_number_block_not_compressed`
     - Numeric
   * - :variable:`rocksdb_number_db_next`
     - Numeric
   * - :variable:`rocksdb_number_db_next_found`
     - Numeric
   * - :variable:`rocksdb_number_db_prev`
     - Numeric
   * - :variable:`rocksdb_number_db_prev_found`
     - Numeric
   * - :variable:`rocksdb_number_db_seek`
     - Numeric
   * - :variable:`rocksdb_number_db_seek_found`
     - Numeric
   * - :variable:`rocksdb_number_deletes_filtered`
     - Numeric
   * - :variable:`rocksdb_number_keys_read`
     - Numeric
   * - :variable:`rocksdb_number_keys_updated`
     - Numeric
   * - :variable:`rocksdb_number_keys_written`
     - Numeric
   * - :variable:`rocksdb_number_merge_failures`
     - Numeric
   * - :variable:`rocksdb_number_multiget_bytes_read`
     - Numeric
   * - :variable:`rocksdb_number_multiget_get`
     - Numeric
   * - :variable:`rocksdb_number_multiget_keys_read`
     - Numeric
   * - :variable:`rocksdb_number_reseeks_iteration`
     - Numeric
   * - :variable:`rocksdb_number_sst_entry_delete`
     - Numeric
   * - :variable:`rocksdb_number_sst_entry_merge`
     - Numeric
   * - :variable:`rocksdb_number_sst_entry_other`
     - Numeric
   * - :variable:`rocksdb_number_sst_entry_put`
     - Numeric
   * - :variable:`rocksdb_number_sst_entry_singledelete`
     - Numeric
   * - :variable:`rocksdb_number_stat_computes`
     - Numeric
   * - :variable:`rocksdb_number_superversion_acquires`
     - Numeric
   * - :variable:`rocksdb_number_superversion_cleanups`
     - Numeric
   * - :variable:`rocksdb_number_superversion_releases`
     - Numeric
   * - :variable:`rocksdb_rate_limit_delay_millis`
     - Numeric
   * - :variable:`rocksdb_row_lock_deadlocks`
     - Numeric
   * - :variable:`rocksdb_row_lock_wait_timeouts`
     - Numeric
   * - :variable:`rocksdb_snapshot_conflict_errors`
     - Numeric
   * - :variable:`rocksdb_stall_l0_file_count_limit_slowdowns`
     - Numeric
   * - :variable:`rocksdb_stall_locked_l0_file_count_limit_slowdowns`
     - Numeric
   * - :variable:`rocksdb_stall_l0_file_count_limit_stops`
     - Numeric
   * - :variable:`rocksdb_stall_locked_l0_file_count_limit_stops`
     - Numeric
   * - :variable:`rocksdb_stall_pending_compaction_limit_stops`
     - Numeric
   * - :variable:`rocksdb_stall_pending_compaction_limit_slowdowns`
     - Numeric
   * - :variable:`rocksdb_stall_memtable_limit_stops`
     - Numeric
   * - :variable:`rocksdb_stall_memtable_limit_slowdowns`
     - Numeric
   * - :variable:`rocksdb_stall_total_stops`
     - Numeric
   * - :variable:`rocksdb_stall_total_slowdowns`
     - Numeric
   * - :variable:`rocksdb_stall_micros`
     - Numeric
   * - :variable:`rocksdb_wal_bytes`
     - Numeric
   * - :variable:`rocksdb_wal_group_syncs`
     - Numeric
   * - :variable:`rocksdb_wal_synced`
     - Numeric
   * - :variable:`rocksdb_write_other`
     - Numeric
   * - :variable:`rocksdb_write_self`
     - Numeric
   * - :variable:`rocksdb_write_timedout`
     - Numeric
   * - :variable:`rocksdb_write_wal`
     - Numeric

.. variable:: rocksdb_rows_deleted

This variable shows the number of rows that were deleted from MyRocks tables.

.. variable:: rocksdb_rows_inserted

This variable shows the number of rows that were inserted into MyRocks tables.

.. variable:: rocksdb_rows_read

This variable shows the number of rows that were read from MyRocks tables.

.. variable:: rocksdb_rows_updated

This variable shows the number of rows that were updated in MyRocks tables.

.. variable:: rocksdb_rows_expired

This variable shows the number of expired rows in MyRocks tables.

.. variable:: rocksdb_system_rows_deleted

This variable shows the number of rows that were deleted
from MyRocks system tables.

.. variable:: rocksdb_system_rows_inserted

This variable shows the number of rows that were inserted
into MyRocks system tables.

.. variable:: rocksdb_system_rows_read

This variable shows the number of rows that were read
from MyRocks system tables.

.. variable:: rocksdb_system_rows_updated

This variable shows the number of rows that were updated
in MyRocks system tables.

.. variable:: rocksdb_memtable_total

This variable shows the memory usage, in bytes, of all memtables.

.. variable:: rocksdb_memtable_unflushed

This variable shows the memory usage, in bytes, of all unflushed memtables.

.. variable:: rocksdb_queries_point

This variable shows the number of single row queries.

.. variable:: rocksdb_queries_range

This variable shows the number of multi/range row queries.

.. variable:: rocksdb_covered_secondary_key_lookups

This variable shows the number of lookups via secondary index that were able to
return all fields requested directly from the secondary index when the
secondary index contained a field that is only a prefix of the
``varchar`` column.

.. variable:: rocksdb_block_cache_add

This variable shows the number of blocks added to block cache.

.. variable:: rocksdb_block_cache_add_failures

This variable shows the number of failures when adding blocks to block cache.

.. variable:: rocksdb_block_cache_bytes_read

This variable shows the number of bytes read from cache.

.. variable:: rocksdb_block_cache_bytes_write

This variable shows the number of bytes written into cache.

.. variable:: rocksdb_block_cache_data_add

This variable shows the number of data blocks added to block cache.

.. variable:: rocksdb_block_cache_data_bytes_insert

This variable shows the number of bytes of data blocks inserted into cache.

.. variable:: rocksdb_block_cache_data_hit

This variable shows the number of cache hits when accessing the
data block from the block cache.

.. variable:: rocksdb_block_cache_data_miss

This variable shows the number of cache misses when accessing the
data block from the block cache.

.. variable:: rocksdb_block_cache_filter_add

This variable shows the number of filter blocks added to block cache.

.. variable:: rocksdb_block_cache_filter_bytes_evict

This variable shows the number of bytes of bloom filter blocks
removed from cache.

.. variable:: rocksdb_block_cache_filter_bytes_insert

This variable shows the number of bytes of bloom filter blocks
inserted into cache.

.. variable:: rocksdb_block_cache_filter_hit

This variable shows the number of times cache hit when accessing filter block
from block cache.

.. variable:: rocksdb_block_cache_filter_miss

This variable shows the number of times cache miss when accessing filter
block from block cache.

.. variable:: rocksdb_block_cache_hit

This variable shows the total number of block cache hits.

.. variable:: rocksdb_block_cache_index_add

This variable shows the number of index blocks added to block cache.

.. variable:: rocksdb_block_cache_index_bytes_evict

This variable shows the number of bytes of index block erased from cache.

.. variable:: rocksdb_block_cache_index_bytes_insert

This variable shows the number of bytes of index blocks inserted into cache.

.. variable:: rocksdb_block_cache_index_hit

This variable shows the total number of block cache index hits.

.. variable:: rocksdb_block_cache_index_miss

This variable shows the number of times cache hit when accessing index
block from block cache.

.. variable:: rocksdb_block_cache_miss

This variable shows the total number of block cache misses.

.. variable:: rocksdb_block_cache_compressed_hit

This variable shows the number of hits in the compressed block cache.

.. variable:: rocksdb_block_cache_compressed_miss

This variable shows the number of misses in the compressed block cache.

.. variable:: rocksdb_bloom_filter_prefix_checked

This variable shows the number of times bloom was checked before
creating iterator on a file.

.. variable:: rocksdb_bloom_filter_prefix_useful

This variable shows the number of times the check was useful in avoiding
iterator creation (and thus likely IOPs).

.. variable:: rocksdb_bloom_filter_useful

This variable shows the number of times bloom filter has avoided file reads.

.. variable:: rocksdb_bytes_read

This variable shows the total number of uncompressed bytes read. It could be
either from memtables, cache, or table files.

.. variable:: rocksdb_bytes_written

This variable shows the total number of uncompressed bytes written.

.. variable:: rocksdb_compact_read_bytes

This variable shows the number of bytes read during compaction

.. variable:: rocksdb_compact_write_bytes

This variable shows the number of bytes written during compaction.

.. variable:: rocksdb_compaction_key_drop_new

This variable shows the number of key drops during compaction because
it was overwritten with a newer value.

.. variable:: rocksdb_compaction_key_drop_obsolete

This variable shows the number of key drops during compaction because
it was obsolete.

.. variable:: rocksdb_compaction_key_drop_user

This variable shows the number of key drops during compaction because
user compaction function has dropped the key.

.. variable:: rocksdb_flush_write_bytes

This variable shows the number of bytes written during flush.

.. variable:: rocksdb_get_hit_l0

This variable shows the number of ``Get()`` queries served by L0.

.. variable:: rocksdb_get_hit_l1

This variable shows the number of ``Get()`` queries served by L1.

.. variable:: rocksdb_get_hit_l2_and_up

This variable shows the number of ``Get()`` queries served by L2 and up.

.. variable:: rocksdb_get_updates_since_calls

This variable shows the number of calls to ``GetUpdatesSince`` function.
Useful to keep track of transaction log iterator refreshes

.. variable:: rocksdb_iter_bytes_read

This variable shows the number of uncompressed bytes read from an iterator.
It includes size of key and value.

.. variable:: rocksdb_memtable_hit

This variable shows the number of memtable hits.

.. variable:: rocksdb_memtable_miss

This variable shows the number of memtable misses.

.. variable:: rocksdb_no_file_closes

This variable shows the number of time file were closed.

.. variable:: rocksdb_no_file_errors

This variable shows number of errors trying to read in data from an sst file.

.. variable:: rocksdb_no_file_opens

This variable shows the number of time file were opened.

.. variable:: rocksdb_num_iterators

This variable shows the number of currently open iterators.

.. variable:: rocksdb_number_block_not_compressed

This variable shows the number of uncompressed blocks.

.. variable:: rocksdb_number_db_next

This variable shows the number of calls to ``next``.

.. variable:: rocksdb_number_db_next_found

This variable shows the number of calls to ``next`` that returned data.

.. variable:: rocksdb_number_db_prev

This variable shows the number of calls to ``prev``.

.. variable:: rocksdb_number_db_prev_found

This variable shows the number of calls to ``prev`` that returned data.

.. variable:: rocksdb_number_db_seek

This variable shows the number of calls to ``seek``.

.. variable:: rocksdb_number_db_seek_found

This variable shows the number of calls to ``seek`` that returned data.

.. variable:: rocksdb_number_deletes_filtered

This variable shows the number of deleted records that were not required to be
written to storage because key did not exist.

.. variable:: rocksdb_number_keys_read

This variable shows the number of keys read.

.. variable:: rocksdb_number_keys_updated

This variable shows the number of keys updated, if inplace update is enabled.

.. variable:: rocksdb_number_keys_written

This variable shows the number of keys written to the database.

.. variable:: rocksdb_number_merge_failures

This variable shows the number of failures performing merge operator actions
in RocksDB.

.. variable:: rocksdb_number_multiget_bytes_read

This variable shows the number of bytes read during RocksDB
``MultiGet()`` calls.

.. variable:: rocksdb_number_multiget_get

This variable shows the number ``MultiGet()`` requests to RocksDB.

.. variable:: rocksdb_number_multiget_keys_read

This variable shows the keys read via ``MultiGet()``.

.. variable:: rocksdb_number_reseeks_iteration

This variable shows the number of times reseek happened inside an iteration to
skip over large number of keys with same userkey.

.. variable:: rocksdb_number_sst_entry_delete

This variable shows the total number of delete markers written by MyRocks.

.. variable:: rocksdb_number_sst_entry_merge

This variable shows the total number of merge keys written by MyRocks.

.. variable:: rocksdb_number_sst_entry_other

This variable shows the total number of non-delete, non-merge, non-put keys
written by MyRocks.

.. variable:: rocksdb_number_sst_entry_put

This variable shows the total number of put keys written by MyRocks.

.. variable:: rocksdb_number_sst_entry_singledelete

This variable shows the total number of single delete keys written by MyRocks.

.. variable:: rocksdb_number_stat_computes

This variable isn't used anymore and will be removed in future releases.

.. variable:: rocksdb_number_superversion_acquires

This variable shows the number of times the superversion structure has been
acquired in RocksDB, this is used for tracking all of the files for the
database.

.. variable:: rocksdb_number_superversion_cleanups

.. variable:: rocksdb_number_superversion_releases

.. variable:: rocksdb_rate_limit_delay_millis

This variable isn't used anymore and will be removed in future releases.

.. variable:: rocksdb_row_lock_deadlocks

This variable shows the total number of deadlocks that have been detected since the instance was started.

.. variable:: rocksdb_row_lock_wait_timeouts

This variable shows the total number of row lock wait timeouts that have been detected since the instance was started.

.. variable:: rocksdb_snapshot_conflict_errors

This variable shows the number of snapshot conflict errors occurring during
write transactions that forces the transaction to rollback.

.. variable:: rocksdb_stall_l0_file_count_limit_slowdowns

This variable shows the slowdowns in write due to L0 being close to full.

.. variable:: rocksdb_stall_locked_l0_file_count_limit_slowdowns

This variable shows the slowdowns in write due to L0 being close to full and
compaction for L0 is already in progress.

.. variable:: rocksdb_stall_l0_file_count_limit_stops

This variable shows the stalls in write due to L0 being full.

.. variable:: rocksdb_stall_locked_l0_file_count_limit_stops

This variable shows the stalls in write due to L0 being full and compaction
for L0 is already in progress.

.. variable:: rocksdb_stall_pending_compaction_limit_stops

This variable shows the stalls in write due to hitting limits set for max
number of pending compaction bytes.

.. variable:: rocksdb_stall_pending_compaction_limit_slowdowns

This variable shows the slowdowns in write due to getting close to limits set
for max number of pending compaction bytes.

.. variable:: rocksdb_stall_memtable_limit_stops

This variable shows the stalls in write due to hitting max number of
``memTables`` allowed.

.. variable:: rocksdb_stall_memtable_limit_slowdowns

This variable shows the slowdowns in writes due to getting close to
max number of memtables allowed.

.. variable:: rocksdb_stall_total_stops

This variable shows the total number of write stalls.

.. variable:: rocksdb_stall_total_slowdowns

This variable shows the total number of write slowdowns.

.. variable:: rocksdb_stall_micros

This variable shows how long (in microseconds) the writer had to wait for
compaction or flush to finish.

.. variable:: rocksdb_wal_bytes

This variables shows the number of bytes written to WAL.

.. variable:: rocksdb_wal_group_syncs

This variable shows the number of group commit WAL file syncs
that have occurred.

.. variable:: rocksdb_wal_synced

This variable shows the number of times WAL sync was done.

.. variable:: rocksdb_write_other

This variable shows the number of writes processed by another thread.

.. variable:: rocksdb_write_self

This variable shows the number of writes that were processed
by a requesting thread.

.. variable:: rocksdb_write_timedout

This variable shows the number of writes ending up with timed-out.

.. variable:: rocksdb_write_wal

This variable shows the number of Write calls that request WAL.
