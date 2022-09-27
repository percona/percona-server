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
   * - :ref:`rocksdb_rows_deleted`
     - Numeric
   * - :ref:`rocksdb_rows_inserted`
     - Numeric
   * - :ref:`rocksdb_rows_read`
     - Numeric
   * - :ref:`rocksdb_rows_unfiltered_no_snapshot`
     - Numeric
   * - :ref:`rocksdb_rows_updated`
     - Numeric
   * - :ref:`rocksdb_rows_expired`
     - Numeric
   * - :ref:`rocksdb_system_rows_deleted`
     - Numeric
   * - :ref:`rocksdb_system_rows_inserted`
     - Numeric
   * - :ref:`rocksdb_system_rows_read`
     - Numeric
   * - :ref:`rocksdb_system_rows_updated`
     - Numeric
   * - :ref:`rocksdb_memtable_total`
     - Numeric
   * - :ref:`rocksdb_memtable_unflushed`
     - Numeric
   * - :ref:`rocksdb_queries_point`
     - Numeric
   * - :ref:`rocksdb_queries_range`
     - Numeric
   * - :ref:`rocksdb_covered_secondary_key_lookups`
     - Numeric
   * - :ref:`rocksdb_additional_compactions_trigger`
     - Numeric
   * - :ref:`rocksdb_block_cache_add`
     - Numeric
   * - :ref:`rocksdb_block_cache_add_failures`
     - Numeric
   * - :ref:`rocksdb_block_cache_bytes_read`
     - Numeric
   * - :ref:`rocksdb_block_cache_bytes_write`
     - Numeric
   * - :ref:`rocksdb_block_cache_data_add`
     - Numeric
   * - :ref:`rocksdb_block_cache_data_bytes_insert`
     - Numeric
   * - :ref:`rocksdb_block_cache_data_hit`
     - Numeric
   * - :ref:`rocksdb_block_cache_data_miss`
     - Numeric
   * - :ref:`rocksdb_block_cache_filter_add`
     - Numeric
   * - :ref:`rocksdb_block_cache_filter_bytes_evict`
     - Numeric
   * - :ref:`rocksdb_block_cache_filter_bytes_insert`
     - Numeric
   * - :ref:`rocksdb_block_cache_filter_hit`
     - Numeric
   * - :ref:`rocksdb_block_cache_filter_miss`
     - Numeric
   * - :ref:`rocksdb_block_cache_hit`
     - Numeric
   * - :ref:`rocksdb_block_cache_index_add`
     - Numeric
   * - :ref:`rocksdb_block_cache_index_bytes_evict`
     - Numeric
   * - :ref:`rocksdb_block_cache_index_bytes_insert`
     - Numeric
   * - :ref:`rocksdb_block_cache_index_hit`
     - Numeric
   * - :ref:`rocksdb_block_cache_index_miss`
     - Numeric
   * - :ref:`rocksdb_block_cache_miss`
     - Numeric
   * - :ref:`rocksdb_block_cache_compressed_hit`
     - Numeric
   * - :ref:`rocksdb_block_cache_compressed_miss`
     - Numeric
   * - :ref:`rocksdb_bloom_filter_prefix_checked`
     - Numeric
   * - :ref:`rocksdb_bloom_filter_prefix_useful`
     - Numeric
   * - :ref:`rocksdb_bloom_filter_useful`
     - Numeric
   * - :ref:`rocksdb_bytes_read`
     - Numeric
   * - :ref:`rocksdb_bytes_written`
     - Numeric
   * - :ref:`rocksdb_compact_read_bytes`
     - Numeric
   * - :ref:`rocksdb_compact_write_bytes`
     - Numeric
   * - :ref:`rocksdb_compaction_key_drop_new`
     - Numeric
   * - :ref:`rocksdb_compaction_key_drop_obsolete`
     - Numeric
   * - :ref:`rocksdb_compaction_key_drop_user`
     - Numeric
   * - :ref:`rocksdb_flush_write_bytes`
     - Numeric
   * - :ref:`rocksdb_get_hit_l0`
     - Numeric
   * - :ref:`rocksdb_get_hit_l1`
     - Numeric
   * - :ref:`rocksdb_get_hit_l2_and_up`
     - Numeric
   * - :ref:`rocksdb_get_updates_since_calls`
     - Numeric
   * - :ref:`rocksdb_iter_bytes_read`
     - Numeric
   * - :ref:`rocksdb_memtable_hit`
     - Numeric
   * - :ref:`rocksdb_memtable_miss`
     - Numeric
   * - :ref:`rocksdb_no_file_closes`
     - Numeric
   * - :ref:`rocksdb_no_file_errors`
     - Numeric
   * - :ref:`rocksdb_no_file_opens`
     - Numeric
   * - :ref:`rocksdb_num_iterators`
     - Numeric
   * - :ref:`rocksdb_number_block_not_compressed`
     - Numeric
   * - :ref:`rocksdb_number_db_next`
     - Numeric
   * - :ref:`rocksdb_number_db_next_found`
     - Numeric
   * - :ref:`rocksdb_number_db_prev`
     - Numeric
   * - :ref:`rocksdb_number_db_prev_found`
     - Numeric
   * - :ref:`rocksdb_number_db_seek`
     - Numeric
   * - :ref:`rocksdb_number_db_seek_found`
     - Numeric
   * - :ref:`rocksdb_number_deletes_filtered`
     - Numeric
   * - :ref:`rocksdb_number_keys_read`
     - Numeric
   * - :ref:`rocksdb_number_keys_updated`
     - Numeric
   * - :ref:`rocksdb_number_keys_written`
     - Numeric
   * - :ref:`rocksdb_number_merge_failures`
     - Numeric
   * - :ref:`rocksdb_number_multiget_bytes_read`
     - Numeric
   * - :ref:`rocksdb_number_multiget_get`
     - Numeric
   * - :ref:`rocksdb_number_multiget_keys_read`
     - Numeric
   * - :ref:`rocksdb_number_reseeks_iteration`
     - Numeric
   * - :ref:`rocksdb_number_sst_entry_delete`
     - Numeric
   * - :ref:`rocksdb_number_sst_entry_merge`
     - Numeric
   * - :ref:`rocksdb_number_sst_entry_other`
     - Numeric
   * - :ref:`rocksdb_number_sst_entry_put`
     - Numeric
   * - :ref:`rocksdb_number_sst_entry_singledelete`
     - Numeric
   * - :ref:`rocksdb_number_stat_computes`
     - Numeric
   * - :ref:`rocksdb_number_superversion_acquires`
     - Numeric
   * - :ref:`rocksdb_number_superversion_cleanups`
     - Numeric
   * - :ref:`rocksdb_number_superversion_releases`
     - Numeric
   * - :ref:`rocksdb_rate_limit_delay_millis`
     - Numeric
   * - :ref:`rocksdb_row_lock_deadlocks`
     - Numeric
   * - :ref:`rocksdb_row_lock_wait_timeouts`
     - Numeric
   * - :ref:`rocksdb_snapshot_conflict_errors`
     - Numeric
   * - :ref:`rocksdb_stall_l0_file_count_limit_slowdowns`
     - Numeric
   * - :ref:`rocksdb_stall_locked_l0_file_count_limit_slowdowns`
     - Numeric
   * - :ref:`rocksdb_stall_l0_file_count_limit_stops`
     - Numeric
   * - :ref:`rocksdb_stall_locked_l0_file_count_limit_stops`
     - Numeric
   * - :ref:`rocksdb_stall_pending_compaction_limit_stops`
     - Numeric
   * - :ref:`rocksdb_stall_pending_compaction_limit_slowdowns`
     - Numeric
   * - :ref:`rocksdb_stall_memtable_limit_stops`
     - Numeric
   * - :ref:`rocksdb_stall_memtable_limit_slowdowns`
     - Numeric
   * - :ref:`rocksdb_stall_total_stops`
     - Numeric
   * - :ref:`rocksdb_stall_total_slowdowns`
     - Numeric
   * - :ref:`rocksdb_stall_micros`
     - Numeric
   * - :ref:`rocksdb_wal_bytes`
     - Numeric
   * - :ref:`rocksdb_wal_group_syncs`
     - Numeric
   * - :ref:`rocksdb_wal_synced`
     - Numeric
   * - :ref:`rocksdb_write_other`
     - Numeric
   * - :ref:`rocksdb_write_self`
     - Numeric
   * - :ref:`rocksdb_write_timedout`
     - Numeric
   * - :ref:`rocksdb_write_wal`
     - Numeric

.. _rocksdb_rows_deleted:

.. rubric:: ``rocksdb_rows_deleted``

This variable shows the number of rows that were deleted from MyRocks tables.

.. _rocksdb_rows_inserted:

.. rubric:: ``rocksdb_rows_inserted``

This variable shows the number of rows that were inserted into MyRocks tables.

.. _rocksdb_rows_read:

.. rubric:: ``rocksdb_rows_read``

This variable shows the number of rows that were read from MyRocks tables.

.. _rocksdb_rows_unfiltered_no_snapshot:

.. rubric:: ``rocksdb_rows_unfiltered_no_snapshot``

This variable shows how many reads need TTL and have no snapshot timestamp.

.. _rocksdb_rows_updated:

.. rubric:: ``rocksdb_rows_updated``

This variable shows the number of rows that were updated in MyRocks tables.

.. _rocksdb_rows_expired:

.. rubric:: ``rocksdb_rows_expired``

This variable shows the number of expired rows in MyRocks tables.

.. _rocksdb_system_rows_deleted:

.. rubric:: ``rocksdb_system_rows_deleted``

This variable shows the number of rows that were deleted
from MyRocks system tables.

.. _rocksdb_system_rows_inserted:

.. rubric:: ``rocksdb_system_rows_inserted``

This variable shows the number of rows that were inserted
into MyRocks system tables.

.. _rocksdb_system_rows_read:

.. rubric:: ``ocksdb_system_rows_read``

This variable shows the number of rows that were read
from MyRocks system tables.

.. _rocksdb_system_rows_updated:

.. rubric:: ``rocksdb_system_rows_updated``

This variable shows the number of rows that were updated
in MyRocks system tables.

.. _rocksdb_memtable_total:

.. rubric:: ``rocksdb_memtable_total``

This variable shows the memory usage, in bytes, of all memtables.

.. _rocksdb_memtable_unflushed:

.. rubric:: ``rocksdb_memtable_unflushed``

This variable shows the memory usage, in bytes, of all unflushed memtables.

.. _rocksdb_queries_point:

.. rubric:: ``rocksdb_queries_point``

This variable shows the number of single row queries.

.. _rocksdb_queries_range:

.. rubric:: ``rocksdb_queries_range``

This variable shows the number of multi/range row queries.

.. _rocksdb_covered_secondary_key_lookups:

.. rubric:: ``rocksdb_covered_secondary_key_lookups``

This variable shows the number of lookups via secondary index that were able to
return all fields requested directly from the secondary index when the
secondary index contained a field that is only a prefix of the
``varchar`` column.

.. _rocksdb_additional_compactions_trigger:

.. rubric:: ``rocksdb_additional_compactions_trigger``

This variable shows the number of triggered additional compactions.
MyRocks triggers an additional compaction if (number of deletions / number of entries) > (rocksdb_compaction_sequential_deletes / rocksdb_compaction_sequential_deletes_window)
in the SST file.

.. _rocksdb_block_cache_add:

.. rubric:: ``rocksdb_block_cache_add``

This variable shows the number of blocks added to block cache.

.. _rocksdb_block_cache_add_failures:

.. rubric:: ``rocksdb_block_cache_add_failures``

This variable shows the number of failures when adding blocks to block cache.

.. _rocksdb_block_cache_bytes_read:

.. rubric:: ``rocksdb_block_cache_bytes_read``

This variable shows the number of bytes read from cache.

.. _rocksdb_block_cache_bytes_write:

.. rubric:: ``rocksdb_block_cache_bytes_write``

This variable shows the number of bytes written into cache.

.. _rocksdb_block_cache_data_add:

.. rubric:: ``rocksdb_block_cache_data_add``

This variable shows the number of data blocks added to block cache.

.. _rocksdb_block_cache_data_bytes_insert:

.. rubric:: ``rocksdb_block_cache_data_bytes_insert``

This variable shows the number of bytes of data blocks inserted into cache.

.. _rocksdb_block_cache_data_hit:

.. rubric:: ``rocksdb_block_cache_data_hit``

This variable shows the number of cache hits when accessing the
data block from the block cache.

.. _rocksdb_block_cache_data_miss:

.. rubric:: ``rocksdb_block_cache_data_miss``

This variable shows the number of cache misses when accessing the
data block from the block cache.

.. _rocksdb_block_cache_filter_add:

.. rubric:: ``rocksdb_block_cache_filter_add``

This variable shows the number of filter blocks added to block cache.

.. _rocksdb_block_cache_filter_bytes_evict:

.. rubric:: ``rocksdb_block_cache_filter_bytes_evict``

This variable shows the number of bytes of bloom filter blocks
removed from cache.

.. _rocksdb_block_cache_filter_bytes_insert:

.. rubric:: ``rocksdb_block_cache_filter_bytes_insert``

This variable shows the number of bytes of bloom filter blocks
inserted into cache.

.. _rocksdb_block_cache_filter_hit:

.. rubric:: ``rocksdb_block_cache_filter_hit``

This variable shows the number of times cache hit when accessing filter block
from block cache.

.. _rocksdb_block_cache_filter_miss:

.. rubric:: ``rocksdb_block_cache_filter_miss``

This variable shows the number of times cache miss when accessing filter
block from block cache.

.. _rocksdb_block_cache_hit:

.. rubric:: ``rocksdb_block_cache_hit``

This variable shows the total number of block cache hits.

.. _rocksdb_block_cache_index_add:

.. rubric:: ``rocksdb_block_cache_index_add``

This variable shows the number of index blocks added to block cache.

.. _rocksdb_block_cache_index_bytes_evict:

.. rubric:: ``rocksdb_block_cache_index_bytes_evict``

This variable shows the number of bytes of index block erased from cache.

.. _rocksdb_block_cache_index_bytes_insert:

.. rubric:: ``rocksdb_block_cache_index_bytes_insert``

This variable shows the number of bytes of index blocks inserted into cache.

.. _rocksdb_block_cache_index_hit:

.. rubric:: ``rocksdb_block_cache_index_hit``

This variable shows the total number of block cache index hits.

.. _rocksdb_block_cache_index_miss:

.. rubric:: ``rocksdb_block_cache_index_miss``

This variable shows the number of times cache hit when accessing index
block from block cache.

.. _rocksdb_block_cache_miss:

.. rubric:: ``rocksdb_block_cache_miss``

This variable shows the total number of block cache misses.

.. _rocksdb_block_cache_compressed_hit:

.. rubric:: ``rocksdb_block_cache_compressed_hit``

This variable shows the number of hits in the compressed block cache.

.. _rocksdb_block_cache_compressed_miss:

.. rubric:: ``rocksdb_block_cache_compressed_miss``

This variable shows the number of misses in the compressed block cache.

.. _rocksdb_bloom_filter_prefix_checked:

.. rubric:: ``rocksdb_bloom_filter_prefix_checked``

This variable shows the number of times bloom was checked before
creating iterator on a file.

.. _rocksdb_bloom_filter_prefix_useful:

.. rubric:: ``rocksdb_bloom_filter_prefix_useful``

This variable shows the number of times the check was useful in avoiding
iterator creation (and thus likely IOPs).

.. _rocksdb_bloom_filter_useful:

.. rubric:: ``rocksdb_bloom_filter_useful``

This variable shows the number of times bloom filter has avoided file reads.

.. _rocksdb_bytes_read:

.. rubric:: ``rocksdb_bytes_read``

This variable shows the total number of uncompressed bytes read. It could be
either from memtables, cache, or table files.

.. _rocksdb_bytes_written:

.. rubric:: ``rocksdb_bytes_written``

This variable shows the total number of uncompressed bytes written.

.. _rocksdb_compact_read_bytes:

.. rubric:: ``rocksdb_compact_read_bytes``

This variable shows the number of bytes read during compaction

.. _rocksdb_compact_write_bytes:

.. rubric:: ``rocksdb_compact_write_bytes``

This variable shows the number of bytes written during compaction.

.. _rocksdb_compaction_key_drop_new:

.. rubric:: ``rocksdb_compaction_key_drop_new``

This variable shows the number of key drops during compaction because
it was overwritten with a newer value.

.. _rocksdb_compaction_key_drop_obsolete:

.. rubric:: ``rocksdb_compaction_key_drop_obsolete``

This variable shows the number of key drops during compaction because
it was obsolete.

.. _rocksdb_compaction_key_drop_user:

.. rubric:: ``rocksdb_compaction_key_drop_user``

This variable shows the number of key drops during compaction because
user compaction function has dropped the key.

.. _rocksdb_flush_write_bytes:

.. rubric:: ``rocksdb_flush_write_bytes``

This variable shows the number of bytes written during flush.

.. _rocksdb_get_hit_l0:

.. rubric:: ``rocksdb_get_hit_l0``

This variable shows the number of ``Get()`` queries served by L0.

.. _rocksdb_get_hit_l1:

.. rubric:: ``rocksdb_get_hit_l1``

This variable shows the number of ``Get()`` queries served by L1.

.. _rocksdb_get_hit_l2_and_up:

.. rubric:: ``rocksdb_get_hit_l2_and_up``

This variable shows the number of ``Get()`` queries served by L2 and up.

.. _rocksdb_get_updates_since_calls:

.. rubric:: ``rocksdb_get_updates_since_calls``

This variable shows the number of calls to ``GetUpdatesSince`` function.
Useful to keep track of transaction log iterator refreshes

.. _rocksdb_iter_bytes_read:

.. rubric:: ``rocksdb_iter_bytes_read``

This variable shows the number of uncompressed bytes read from an iterator.
It includes size of key and value.

.. _rocksdb_memtable_hit:

.. rubric:: ``rocksdb_memtable_hit``

This variable shows the number of memtable hits.

.. _rocksdb_memtable_miss:

.. rubric:: ``rocksdb_memtable_miss``

This variable shows the number of memtable misses.

.. _rocksdb_no_file_closes:

.. rubric:: ``rocksdb_no_file_closes``

This variable shows the number of time file were closed.

.. _rocksdb_no_file_errors:

.. rubric:: ``rocksdb_no_file_errors``

This variable shows number of errors trying to read in data from an sst file.

.. _rocksdb_no_file_opens:

.. rubric:: ``rocksdb_no_file_opens``

This variable shows the number of time file were opened.

.. _rocksdb_num_iterators:

.. rubric:: ``rocksdb_num_iterators``

This variable shows the number of currently open iterators.

.. _rocksdb_number_block_not_compressed:

.. rubric:: ``rocksdb_number_block_not_compressed``

This variable shows the number of uncompressed blocks.

.. _rocksdb_number_db_next:

.. rubric:: ``rocksdb_number_db_next``

This variable shows the number of calls to ``next``.

.. _rocksdb_number_db_next_found:

.. rubric:: ``rocksdb_number_db_next_found``

This variable shows the number of calls to ``next`` that returned data.

.. _rocksdb_number_db_prev:

.. rubric:: ``rocksdb_number_db_prev``

This variable shows the number of calls to ``prev``.

.. _rocksdb_number_db_prev_found:

.. rubric:: ``rocksdb_number_db_prev_found``

This variable shows the number of calls to ``prev`` that returned data.

.. _rocksdb_number_db_seek:

.. rubric:: ``rocksdb_number_db_seek``

This variable shows the number of calls to ``seek``.

.. _rocksdb_number_db_seek_found:

.. rubric:: ``rocksdb_number_db_seek_found``

This variable shows the number of calls to ``seek`` that returned data.

.. _rocksdb_number_deletes_filtered:

.. rubric:: ``rocksdb_number_deletes_filtered``

This variable shows the number of deleted records that were not required to be
written to storage because key did not exist.

.. _rocksdb_number_keys_read:

.. rubric:: ``rocksdb_number_keys_read``

This variable shows the number of keys read.

.. _rocksdb_number_keys_updated:

.. rubric:: ``rocksdb_number_keys_updated``

This variable shows the number of keys updated, if inplace update is enabled.

.. _rocksdb_number_keys_written:

.. rubric:: ``rocksdb_number_keys_written``

This variable shows the number of keys written to the database.

.. _rocksdb_number_merge_failures:

.. rubric:: ``rocksdb_number_merge_failures``

This variable shows the number of failures performing merge operator actions
in RocksDB.

.. _rocksdb_number_multiget_bytes_read:

.. rubric:: ``rocksdb_number_multiget_bytes_read``

This variable shows the number of bytes read during RocksDB
``MultiGet()`` calls.

.. _rocksdb_number_multiget_get:

.. rubric:: ``rocksdb_number_multiget_get``

This variable shows the number ``MultiGet()`` requests to RocksDB.

.. _rocksdb_number_multiget_keys_read:

.. rubric:: ``rocksdb_number_multiget_keys_read``

This variable shows the keys read via ``MultiGet()``.

.. _rocksdb_number_reseeks_iteration:

.. rubric:: ``rocksdb_number_reseeks_iteration``

This variable shows the number of times reseek happened inside an iteration to
skip over large number of keys with same userkey.

.. _rocksdb_number_sst_entry_delete:

.. rubric:: ``rocksdb_number_sst_entry_delete``

This variable shows the total number of delete markers written by MyRocks.

.. _rocksdb_number_sst_entry_merge:

.. rubric:: ``rocksdb_number_sst_entry_merge``

This variable shows the total number of merge keys written by MyRocks.

.. _rocksdb_number_sst_entry_other:

.. rubric:: ``rocksdb_number_sst_entry_other``

This variable shows the total number of non-delete, non-merge, non-put keys
written by MyRocks.

.. _rocksdb_number_sst_entry_put:

.. rubric:: ``rocksdb_number_sst_entry_put``

This variable shows the total number of put keys written by MyRocks.

.. _rocksdb_number_sst_entry_singledelete:

.. rubric:: ``rocksdb_number_sst_entry_singledelete``

This variable shows the total number of single delete keys written by MyRocks.

.. _rocksdb_number_stat_computes:

.. rubric:: ``rocksdb_number_stat_computes``

This variable isn't used anymore and will be removed in future releases.

.. _rocksdb_number_superversion_acquires:

.. rubric:: ``rocksdb_number_superversion_acquires``

This variable shows the number of times the superversion structure has been
acquired in RocksDB, this is used for tracking all of the files for the
database.

.. _rocksdb_number_superversion_cleanups:

.. rubric:: ``rocksdb_number_superversion_cleanups``

.. _rocksdb_number_superversion_releases:

.. rubric:: ``rocksdb_number_superversion_releases``

.. _rocksdb_rate_limit_delay_millis:

.. rubric:: ``rocksdb_rate_limit_delay_millis``

This variable was removed in *Percona Server for MySQL* :ref:`5.7.23-23`.

.. _rocksdb_row_lock_deadlocks:

.. rubric:: ``rocksdb_row_lock_deadlocks``

This variable shows the total number of deadlocks that have been detected since the instance was started.

.. _rocksdb_row_lock_wait_timeouts:

.. rubric:: ``rocksdb_row_lock_wait_timeouts``

This variable shows the total number of row lock wait timeouts that have been detected since the instance was started.

.. _rocksdb_snapshot_conflict_errors:

.. rubric:: ``rocksdb_snapshot_conflict_errors``

This variable shows the number of snapshot conflict errors occurring during
write transactions that forces the transaction to rollback.

.. _rocksdb_stall_l0_file_count_limit_slowdowns:

.. rubric:: ``rocksdb_stall_l0_file_count_limit_slowdowns``

This variable shows the slowdowns in write due to L0 being close to full.

.. _rocksdb_stall_locked_l0_file_count_limit_slowdowns:

.. rubric:: ``rocksdb_stall_locked_l0_file_count_limit_slowdowns``

This variable shows the slowdowns in write due to L0 being close to full and
compaction for L0 is already in progress.

.. _rocksdb_stall_l0_file_count_limit_stops:

.. rubric:: ``rocksdb_stall_l0_file_count_limit_stops``

This variable shows the stalls in write due to L0 being full.

.. _rocksdb_stall_locked_l0_file_count_limit_stops:

.. rubric:: ``rocksdb_stall_locked_l0_file_count_limit_stops``

This variable shows the stalls in write due to L0 being full and compaction
for L0 is already in progress.

.. _rocksdb_stall_pending_compaction_limit_stops:

.. rubric:: ``rocksdb_stall_pending_compaction_limit_stops``

This variable shows the stalls in write due to hitting limits set for max
number of pending compaction bytes.

.. _rocksdb_stall_pending_compaction_limit_slowdowns:

.. rubric:: ``rocksdb_stall_pending_compaction_limit_slowdowns``

This variable shows the slowdowns in write due to getting close to limits set
for max number of pending compaction bytes.

.. _rocksdb_stall_memtable_limit_stops:

.. rubric:: ``rocksdb_stall_memtable_limit_stops``

This variable shows the stalls in write due to hitting max number of
``memTables`` allowed.

.. _rocksdb_stall_memtable_limit_slowdowns:

.. rubric:: ``rocksdb_stall_memtable_limit_slowdowns``

This variable shows the slowdowns in writes due to getting close to
max number of memtables allowed.

.. _rocksdb_stall_total_stops:

.. rubric:: ``rocksdb_stall_total_stops``

This variable shows the total number of write stalls.

.. _rocksdb_stall_total_slowdowns:

.. rubric:: ``rocksdb_stall_total_slowdowns``

This variable shows the total number of write slowdowns.

.. _rocksdb_stall_micros:

.. rubric:: ``rocksdb_stall_micros``

This variable shows how long (in microseconds) the writer had to wait for
compaction or flush to finish.

.. _rocksdb_wal_bytes:

.. rubric:: ``rocksdb_wal_bytes``

This variables shows the number of bytes written to WAL.

.. _rocksdb_wal_group_syncs:

.. rubric:: ``rocksdb_wal_group_syncs``

This variable shows the number of group commit WAL file syncs
that have occurred.

.. _rocksdb_wal_synced:

.. rubric:: ``rocksdb_wal_synced``

This variable shows the number of times WAL sync was done.

.. _rocksdb_write_other:

.. rubric:: ``rocksdb_write_other``

This variable shows the number of writes processed by another thread.

.. _rocksdb_write_self:

.. rubric:: ``rocksdb_write_self``

This variable shows the number of writes that were processed
by a requesting thread.

.. _rocksdb_write_timedout:

.. rubric:: ``rocksdb_write_timedout``

This variable shows the number of writes ending up with timed-out.

.. _rocksdb_write_wal:

.. rubric:: ``rocksdb_write_wal``

This variable shows the number of Write calls that request WAL.
