.. _tokudb_status_variables:

=======================
TokuDB Status Variables
=======================

|TokuDB| status variables provide details about the inner workings of |TokuDB|
storage engine and they can be useful in tuning the storage engine to a
particular environment. 

You can view these variables and their values by running:

.. code-block:: mysql

  mysql> SHOW STATUS LIKE 'tokudb%';

TokuDB Status Variables Summary
-------------------------------

The following global status variables are available:

.. tabularcolumns:: |p{9cm}|p{6cm}|

.. list-table::
   :header-rows: 1

   * - Name
     - Var Type
   * - :variable:`Tokudb_DB_OPENS`
     - integer 
   * - :variable:`Tokudb_DB_CLOSES`
     - integer
   * - :variable:`Tokudb_DB_OPEN_CURRENT`
     - integer
   * - :variable:`Tokudb_DB_OPEN_MAX`
     - integer
   * - :variable:`Tokudb_LEAF_ENTRY_MAX_COMMITTED_XR`
     - integer
   * - :variable:`Tokudb_LEAF_ENTRY_MAX_PROVISIONAL_XR`
     - integer
   * - :variable:`Tokudb_LEAF_ENTRY_EXPANDED`
     - integer
   * - :variable:`Tokudb_LEAF_ENTRY_MAX_MEMSIZE`
     - integer
   * - :variable:`Tokudb_LEAF_ENTRY_APPLY_GC_BYTES_IN`
     - integer
   * - :variable:`Tokudb_LEAF_ENTRY_APPLY_GC_BYTES_OUT`
     - integer
   * - :variable:`Tokudb_LEAF_ENTRY_NORMAL_GC_BYTES_IN`
     - integer
   * - :variable:`Tokudb_LEAF_ENTRY_NORMAL_GC_BYTES_OUT`
     - integer
   * - :variable:`Tokudb_CHECKPOINT_PERIOD`
     - integer
   * - :variable:`Tokudb_CHECKPOINT_FOOTPRINT`
     - integer
   * - :variable:`Tokudb_CHECKPOINT_LAST_BEGAN`
     - datetime
   * - :variable:`Tokudb_CHECKPOINT_LAST_COMPLETE_BEGAN`
     - datetime
   * - :variable:`Tokudb_CHECKPOINT_LAST_COMPLETE_ENDED`
     - datetime
   * - :variable:`Tokudb_CHECKPOINT_DURATION`
     - integer
   * - :variable:`Tokudb_CHECKPOINT_DURATION_LAST`
     - integer
   * - :variable:`Tokudb_CHECKPOINT_LAST_LSN`
     - integer
   * - :variable:`Tokudb_CHECKPOINT_TAKEN`
     - integer
   * - :variable:`Tokudb_CHECKPOINT_FAILED`
     - integer
   * - :variable:`Tokudb_CHECKPOINT_WAITERS_NOW`
     - integer
   * - :variable:`Tokudb_CHECKPOINT_WAITERS_MAX`
     - integer
   * - :variable:`Tokudb_CHECKPOINT_CLIENT_WAIT_ON_MO`
     - integer
   * - :variable:`Tokudb_CHECKPOINT_CLIENT_WAIT_ON_CS`
     - integer
   * - :variable:`Tokudb_CHECKPOINT_BEGIN_TIME`
     - integer
   * - :variable:`Tokudb_CHECKPOINT_LONG_BEGIN_TIME`
     - integer
   * - :variable:`Tokudb_CHECKPOINT_LONG_BEGIN_COUNT`
     - integer
   * - :variable:`Tokudb_CHECKPOINT_END_TIME`
     - integer
   * - :variable:`Tokudb_CHECKPOINT_LONG_END_TIME`
     - integer
   * - :variable:`Tokudb_CHECKPOINT_LONG_END_COUNT`
     - integer
   * - :variable:`Tokudb_CACHETABLE_MISS`
     - integer
   * - :variable:`Tokudb_CACHETABLE_MISS_TIME`
     - integer
   * - :variable:`Tokudb_CACHETABLE_PREFETCHES`
     - integer
   * - :variable:`Tokudb_CACHETABLE_SIZE_CURRENT`
     - integer
   * - :variable:`Tokudb_CACHETABLE_SIZE_LIMIT`
     - integer
   * - :variable:`Tokudb_CACHETABLE_SIZE_WRITING`
     - integer
   * - :variable:`Tokudb_CACHETABLE_SIZE_NONLEAF`
     - integer
   * - :variable:`Tokudb_CACHETABLE_SIZE_LEAF`
     - integer
   * - :variable:`Tokudb_CACHETABLE_SIZE_ROLLBACK`
     - integer
   * - :variable:`Tokudb_CACHETABLE_SIZE_CACHEPRESSURE`
     - integer
   * - :variable:`Tokudb_CACHETABLE_SIZE_CLONED`
     - integer
   * - :variable:`Tokudb_CACHETABLE_EVICTIONS`
     - integer
   * - :variable:`Tokudb_CACHETABLE_CLEANER_EXECUTIONS`
     - integer
   * - :variable:`Tokudb_CACHETABLE_CLEANER_PERIOD`
     - integer
   * - :variable:`Tokudb_CACHETABLE_CLEANER_ITERATIONS`
     - integer
   * - :variable:`Tokudb_CACHETABLE_WAIT_PRESSURE_COUNT`
     - integer
   * - :variable:`Tokudb_CACHETABLE_WAIT_PRESSURE_TIME`
     - integer
   * - :variable:`Tokudb_CACHETABLE_LONG_WAIT_PRESSURE_COUNT`
     - integer
   * - :variable:`Tokudb_CACHETABLE_LONG_WAIT_PRESSURE_TIME`
     - integer
   * - :variable:`Tokudb_CACHETABLE_POOL_CLIENT_NUM_THREADS`
     - integer
   * - :variable:`Tokudb_CACHETABLE_POOL_CLIENT_NUM_THREADS_ACTIVE`
     - integer
   * - :variable:`Tokudb_CACHETABLE_POOL_CLIENT_QUEUE_SIZE`
     - integer
   * - :variable:`Tokudb_CACHETABLE_POOL_CLIENT_MAX_QUEUE_SIZE`
     - integer
   * - :variable:`Tokudb_CACHETABLE_POOL_CLIENT_TOTAL_ITEMS_PROCESSED`
     - integer
   * - :variable:`Tokudb_CACHETABLE_POOL_CLIENT_TOTAL_EXECUTION_TIME`
     - integer
   * - :variable:`Tokudb_CACHETABLE_POOL_CACHETABLE_NUM_THREADS`
     - integer
   * - :variable:`Tokudb_CACHETABLE_POOL_CACHETABLE_NUM_THREADS_ACTIVE`
     - integer
   * - :variable:`Tokudb_CACHETABLE_POOL_CACHETABLE_QUEUE_SIZE`
     - integer
   * - :variable:`Tokudb_CACHETABLE_POOL_CACHETABLE_MAX_QUEUE_SIZE`
     - integer
   * - :variable:`Tokudb_CACHETABLE_POOL_CACHETABLE_TOTAL_ITEMS_PROCESSED`
     - integer
   * - :variable:`Tokudb_CACHETABLE_POOL_CACHETABLE_TOTAL_EXECUTION_TIME`
     - integer
   * - :variable:`Tokudb_CACHETABLE_POOL_CHECKPOINT_NUM_THREADS`
     - integer
   * - :variable:`Tokudb_CACHETABLE_POOL_CHECKPOINT_NUM_THREADS_ACTIVE`
     - integer
   * - :variable:`Tokudb_CACHETABLE_POOL_CHECKPOINT_QUEUE_SIZE`
     - integer
   * - :variable:`Tokudb_CACHETABLE_POOL_CHECKPOINT_MAX_QUEUE_SIZE`
     - integer
   * - :variable:`Tokudb_CACHETABLE_POOL_CHECKPOINT_TOTAL_ITEMS_PROCESSED`
     - integer
   * - :variable:`Tokudb_CACHETABLE_POOL_CHECKPOINT_TOTAL_EXECUTION_TIME`
     - integer
   * - :variable:`Tokudb_LOCKTREE_MEMORY_SIZE`
     - integer
   * - :variable:`Tokudb_LOCKTREE_MEMORY_SIZE_LIMIT`
     - integer
   * - :variable:`Tokudb_LOCKTREE_ESCALATION_NUM`
     - integer
   * - :variable:`Tokudb_LOCKTREE_ESCALATION_SECONDS`
     - numeric
   * - :variable:`Tokudb_LOCKTREE_LATEST_POST_ESCALATION_MEMORY_SIZE`
     - integer
   * - :variable:`Tokudb_LOCKTREE_OPEN_CURRENT`
     - integer
   * - :variable:`Tokudb_LOCKTREE_PENDING_LOCK_REQUESTS`
     - integer
   * - :variable:`Tokudb_LOCKTREE_STO_ELIGIBLE_NUM`
     - integer
   * - :variable:`Tokudb_LOCKTREE_STO_ENDED_NUM`
     - integer
   * - :variable:`Tokudb_LOCKTREE_STO_ENDED_SECONDS`
     - numeric
   * - :variable:`Tokudb_LOCKTREE_WAIT_COUNT`
     - integer
   * - :variable:`Tokudb_LOCKTREE_WAIT_TIME`
     - integer
   * - :variable:`Tokudb_LOCKTREE_LONG_WAIT_COUNT`
     - integer
   * - :variable:`Tokudb_LOCKTREE_LONG_WAIT_TIME`
     - integer
   * - :variable:`Tokudb_LOCKTREE_TIMEOUT_COUNT`
     - integer
   * - :variable:`Tokudb_LOCKTREE_WAIT_ESCALATION_COUNT`
     - integer
   * - :variable:`Tokudb_LOCKTREE_WAIT_ESCALATION_TIME`
     - integer
   * - :variable:`Tokudb_LOCKTREE_LONG_WAIT_ESCALATION_COUNT`
     - integer
   * - :variable:`Tokudb_LOCKTREE_LONG_WAIT_ESCALATION_TIME`
     - integer
   * - :variable:`Tokudb_DICTIONARY_UPDATES`
     - integer
   * - :variable:`Tokudb_DICTIONARY_BROADCAST_UPDATES`
     - integer
   * - :variable:`Tokudb_DESCRIPTOR_SET`
     - integer
   * - :variable:`Tokudb_MESSAGES_IGNORED_BY_LEAF_DUE_TO_MSN`
     - integer
   * - :variable:`Tokudb_TOTAL_SEARCH_RETRIES`
     - integer
   * - :variable:`Tokudb_SEARCH_TRIES_GT_HEIGHT`
     - integer
   * - :variable:`Tokudb_SEARCH_TRIES_GT_HEIGHTPLUS3`
     - integer
   * - :variable:`Tokudb_LEAF_NODES_FLUSHED_NOT_CHECKPOINT`
     - integer
   * - :variable:`Tokudb_LEAF_NODES_FLUSHED_NOT_CHECKPOINT_BYTES`
     - integer
   * - :variable:`Tokudb_LEAF_NODES_FLUSHED_NOT_CHECKPOINT_UNCOMPRESSED_BYTES`
     - integer
   * - :variable:`Tokudb_LEAF_NODES_FLUSHED_NOT_CHECKPOINT_SECONDS`
     - numeric
   * - :variable:`Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_NOT_CHECKPOINT`
     - integer
   * - :variable:`Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_NOT_CHECKPOINT_BYTES`
     - integer
   * - :variable:`Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_NOT_CHECKPOINT_UNCOMPRESSE`
     - integer
   * - :variable:`Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_NOT_CHECKPOINT_SECONDS`
     - numeric
   * - :variable:`Tokudb_LEAF_NODES_FLUSHED_CHECKPOINT`
     - integer
   * - :variable:`Tokudb_LEAF_NODES_FLUSHED_CHECKPOINT_BYTES`
     - integer
   * - :variable:`Tokudb_LEAF_NODES_FLUSHED_CHECKPOINT_UNCOMPRESSED_BYTES`
     - integer
   * - :variable:`Tokudb_LEAF_NODES_FLUSHED_CHECKPOINT_SECONDS`
     - numeric
   * - :variable:`Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_CHECKPOINT`
     - integer
   * - :variable:`Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_CHECKPOINT_BYTES`
     - integer
   * - :variable:`Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_CHECKPOINT_UNCOMPRESSED_BY`
     - integer
   * - :variable:`Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_CHECKPOINT_SECONDS`
     - numeric
   * - :variable:`Tokudb_LEAF_NODE_COMPRESSION_RATIO`
     - numeric
   * - :variable:`Tokudb_NONLEAF_NODE_COMPRESSION_RATIO`
     - numeric
   * - :variable:`Tokudb_OVERALL_NODE_COMPRESSION_RATIO`
     - numeric
   * - :variable:`Tokudb_NONLEAF_NODE_PARTIAL_EVICTIONS`
     - numeric
   * - :variable:`Tokudb_NONLEAF_NODE_PARTIAL_EVICTIONS_BYTES`
     - integer
   * - :variable:`Tokudb_LEAF_NODE_PARTIAL_EVICTIONS`
     - integer
   * - :variable:`Tokudb_LEAF_NODE_PARTIAL_EVICTIONS_BYTES`
     - integer
   * - :variable:`Tokudb_LEAF_NODE_FULL_EVICTIONS`
     - integer
   * - :variable:`Tokudb_LEAF_NODE_FULL_EVICTIONS_BYTES`
     - integer
   * - :variable:`Tokudb_NONLEAF_NODE_FULL_EVICTIONS`
     - integer
   * - :variable:`Tokudb_NONLEAF_NODE_FULL_EVICTIONS_BYTES`
     - integer
   * - :variable:`Tokudb_LEAF_NODES_CREATED`
     - integer
   * - :variable:`Tokudb_NONLEAF_NODES_CREATED`
     - integer
   * - :variable:`Tokudb_LEAF_NODES_DESTROYED`
     - integer
   * - :variable:`Tokudb_NONLEAF_NODES_DESTROYED`
     - integer
   * - :variable:`Tokudb_MESSAGES_INJECTED_AT_ROOT_BYTES`
     - integer
   * - :variable:`Tokudb_MESSAGES_FLUSHED_FROM_H1_TO_LEAVES_BYTES`
     - integer
   * - :variable:`Tokudb_MESSAGES_IN_TREES_ESTIMATE_BYTES`
     - integer
   * - :variable:`Tokudb_MESSAGES_INJECTED_AT_ROOT`
     - integer
   * - :variable:`Tokudb_BROADCASE_MESSAGES_INJECTED_AT_ROOT`
     - integer
   * - :variable:`Tokudb_BASEMENTS_DECOMPRESSED_TARGET_QUERY`
     - integer
   * - :variable:`Tokudb_BASEMENTS_DECOMPRESSED_PRELOCKED_RANGE`
     - integer
   * - :variable:`Tokudb_BASEMENTS_DECOMPRESSED_PREFETCH`
     - integer
   * - :variable:`Tokudb_BASEMENTS_DECOMPRESSED_FOR_WRITE`
     - integer
   * - :variable:`Tokudb_BUFFERS_DECOMPRESSED_TARGET_QUERY`
     - integer
   * - :variable:`Tokudb_BUFFERS_DECOMPRESSED_PRELOCKED_RANGE`
     - integer
   * - :variable:`Tokudb_BUFFERS_DECOMPRESSED_PREFETCH`
     - integer
   * - :variable:`Tokudb_BUFFERS_DECOMPRESSED_FOR_WRITE`
     - integer
   * - :variable:`Tokudb_PIVOTS_FETCHED_FOR_QUERY`
     - integer
   * - :variable:`Tokudb_PIVOTS_FETCHED_FOR_QUERY_BYTES`
     - integer
   * - :variable:`Tokudb_PIVOTS_FETCHED_FOR_QUERY_SECONDS`
     - numeric
   * - :variable:`Tokudb_PIVOTS_FETCHED_FOR_PREFETCH`
     - integer
   * - :variable:`Tokudb_PIVOTS_FETCHED_FOR_PREFETCH_BYTES`
     - integer
   * - :variable:`Tokudb_PIVOTS_FETCHED_FOR_PREFETCH_SECONDS`
     - numeric
   * - :variable:`Tokudb_PIVOTS_FETCHED_FOR_WRITE`
     - integer
   * - :variable:`Tokudb_PIVOTS_FETCHED_FOR_WRITE_BYTES`
     - integer
   * - :variable:`Tokudb_PIVOTS_FETCHED_FOR_WRITE_SECONDS`
     - numeric
   * - :variable:`Tokudb_BASEMENTS_FETCHED_TARGET_QUERY`
     - integer
   * - :variable:`Tokudb_BASEMENTS_FETCHED_TARGET_QUERY_BYTES`
     - integer
   * - :variable:`Tokudb_BASEMENTS_FETCHED_TARGET_QUERY_SECONDS`
     - numeric
   * - :variable:`Tokudb_BASEMENTS_FETCHED_PRELOCKED_RANGE`
     - integer
   * - :variable:`Tokudb_BASEMENTS_FETCHED_PRELOCKED_RANGE_BYTES`
     - integer
   * - :variable:`Tokudb_BASEMENTS_FETCHED_PRELOCKED_RANGE_SECONDS`
     - numeric
   * - :variable:`Tokudb_BASEMENTS_FETCHED_PREFETCH`
     - integer
   * - :variable:`Tokudb_BASEMENTS_FETCHED_PREFETCH_BYTES`
     - integer
   * - :variable:`Tokudb_BASEMENTS_FETCHED_PREFETCH_SECONDS`
     - numeric
   * - :variable:`Tokudb_BASEMENTS_FETCHED_FOR_WRITE`
     - integer
   * - :variable:`Tokudb_BASEMENTS_FETCHED_FOR_WRITE_BYTES`
     - integer
   * - :variable:`Tokudb_BASEMENTS_FETCHED_FOR_WRITE_SECONDS`
     - numeric
   * - :variable:`Tokudb_BUFFERS_FETCHED_TARGET_QUERY`
     - integer
   * - :variable:`Tokudb_BUFFERS_FETCHED_TARGET_QUERY_BYTES`
     - integer
   * - :variable:`Tokudb_BUFFERS_FETCHED_TARGET_QUERY_SECONDS`
     - numeric
   * - :variable:`Tokudb_BUFFERS_FETCHED_PRELOCKED_RANGE`
     - integer
   * - :variable:`Tokudb_BUFFERS_FETCHED_PRELOCKED_RANGE_BYTES`
     - integer
   * - :variable:`Tokudb_BUFFERS_FETCHED_PRELOCKED_RANGE_SECONDS`
     - numeric
   * - :variable:`Tokudb_BUFFERS_FETCHED_PREFETCH`
     - integer
   * - :variable:`Tokudb_BUFFERS_FETCHED_PREFETCH_BYTES`
     - integer
   * - :variable:`Tokudb_BUFFERS_FETCHED_PREFETCH_SECONDS`
     - numeric
   * - :variable:`Tokudb_BUFFERS_FETCHED_FOR_WRITE`
     - integer
   * - :variable:`Tokudb_BUFFERS_FETCHED_FOR_WRITE_BYTES`
     - integer
   * - :variable:`Tokudb_BUFFERS_FETCHED_FOR_WRITE_SECONDS`
     - integer
   * - :variable:`Tokudb_LEAF_COMPRESSION_TO_MEMORY_SECONDS`
     - numeric
   * - :variable:`Tokudb_LEAF_SERIALIZATION_TO_MEMORY_SECONDS`
     - numeric
   * - :variable:`Tokudb_LEAF_DECOMPRESSION_TO_MEMORY_SECONDS`
     - numeric
   * - :variable:`Tokudb_LEAF_DESERIALIZATION_TO_MEMORY_SECONDS`
     - numeric
   * - :variable:`Tokudb_NONLEAF_COMPRESSION_TO_MEMORY_SECONDS`
     - numeric
   * - :variable:`Tokudb_NONLEAF_SERIALIZATION_TO_MEMORY_SECONDS`
     - numeric
   * - :variable:`Tokudb_NONLEAF_DECOMPRESSION_TO_MEMORY_SECONDS`
     - numeric
   * - :variable:`Tokudb_NONLEAF_DESERIALIZATION_TO_MEMORY_SECONDS`
     - numeric
   * - :variable:`Tokudb_PROMOTION_ROOTS_SPLIT`
     - integer
   * - :variable:`Tokudb_PROMOTION_LEAF_ROOTS_INJECTED_INTO`
     - integer
   * - :variable:`Tokudb_PROMOTION_H1_ROOTS_INJECTED_INTO`
     - integer
   * - :variable:`Tokudb_PROMOTION_INJECTIONS_AT_DEPTH_0`
     - integer
   * - :variable:`Tokudb_PROMOTION_INJECTIONS_AT_DEPTH_1`
     - integer
   * - :variable:`Tokudb_PROMOTION_INJECTIONS_AT_DEPTH_2`
     - integer
   * - :variable:`Tokudb_PROMOTION_INJECTIONS_AT_DEPTH_3`
     - integer
   * - :variable:`Tokudb_PROMOTION_INJECTIONS_LOWER_THAN_DEPTH_3`
     - integer
   * - :variable:`Tokudb_PROMOTION_STOPPED_NONEMPTY_BUFFER`
     - integer
   * - :variable:`Tokudb_PROMOTION_STOPPED_AT_HEIGHT_1`
     - integer
   * - :variable:`Tokudb_PROMOTION_STOPPED_CHILD_LOCKED_OR_NOT_IN_MEMORY`
     - integer
   * - :variable:`Tokudb_PROMOTION_STOPPED_CHILD_NOT_FULLY_IN_MEMORY`
     - integer
   * - :variable:`Tokudb_PROMOTION_STOPPED_AFTER_LOCKING_CHILD`
     - integer
   * - :variable:`Tokudb_BASEMENT_DESERIALIZATION_FIXED_KEY`
     - integer
   * - :variable:`Tokudb_BASEMENT_DESERIALIZATION_VARIABLE_KEY`
     - integer
   * - :variable:`Tokudb_PRO_RIGHTMOST_LEAF_SHORTCUT_SUCCESS`
     - integer
   * - :variable:`Tokudb_PRO_RIGHTMOST_LEAF_SHORTCUT_FAIL_POS`
     - integer
   * - :variable:`Tokudb_RIGHTMOST_LEAF_SHORTCUT_FAIL_REACTIVE`
     - integer
   * - :variable:`Tokudb_CURSOR_SKIP_DELETED_LEAF_ENTRY`
     - integer
   * - :variable:`Tokudb_FLUSHER_CLEANER_TOTAL_NODES`
     - integer
   * - :variable:`Tokudb_FLUSHER_CLEANER_H1_NODES`
     - integer
   * - :variable:`Tokudb_FLUSHER_CLEANER_HGT1_NODES`
     - integer
   * - :variable:`Tokudb_FLUSHER_CLEANER_EMPTY_NODES`
     - integer
   * - :variable:`Tokudb_FLUSHER_CLEANER_NODES_DIRTIED`
     - integer
   * - :variable:`Tokudb_FLUSHER_CLEANER_MAX_BUFFER_SIZE`
     - integer
   * - :variable:`Tokudb_FLUSHER_CLEANER_MIN_BUFFER_SIZE`
     - integer
   * - :variable:`Tokudb_FLUSHER_CLEANER_TOTAL_BUFFER_SIZE`
     - integer
   * - :variable:`Tokudb_FLUSHER_CLEANER_MAX_BUFFER_WORKDONE`
     - integer
   * - :variable:`Tokudb_FLUSHER_CLEANER_MIN_BUFFER_WORKDONE`
     - integer
   * - :variable:`Tokudb_FLUSHER_CLEANER_TOTAL_BUFFER_WORKDONE`
     - integer
   * - :variable:`Tokudb_FLUSHER_CLEANER_NUM_LEAF_MERGES_STARTED`
     - integer
   * - :variable:`Tokudb_FLUSHER_CLEANER_NUM_LEAF_MERGES_RUNNING`
     - integer
   * - :variable:`Tokudb_FLUSHER_CLEANER_NUM_LEAF_MERGES_COMPLETED`
     - integer
   * - :variable:`Tokudb_FLUSHER_CLEANER_NUM_DIRTIED_FOR_LEAF_MERGE`
     - integer
   * - :variable:`Tokudb_FLUSHER_FLUSH_TOTAL`
     - integer
   * - :variable:`Tokudb_FLUSHER_FLUSH_IN_MEMORY`
     - integer
   * - :variable:`Tokudb_FLUSHER_FLUSH_NEEDED_IO`
     - integer
   * - :variable:`Tokudb_FLUSHER_FLUSH_CASCADES`
     - integer
   * - :variable:`Tokudb_FLUSHER_FLUSH_CASCADES_1`
     - integer
   * - :variable:`Tokudb_FLUSHER_FLUSH_CASCADES_2`
     - integer
   * - :variable:`Tokudb_FLUSHER_FLUSH_CASCADES_3`
     - integer
   * - :variable:`Tokudb_FLUSHER_FLUSH_CASCADES_4`
     - integer
   * - :variable:`Tokudb_FLUSHER_FLUSH_CASCADES_5`
     - integer
   * - :variable:`Tokudb_FLUSHER_FLUSH_CASCADES_GT_5`
     - integer
   * - :variable:`Tokudb_FLUSHER_SPLIT_LEAF`
     - integer
   * - :variable:`Tokudb_FLUSHER_SPLIT_NONLEAF`
     - integer
   * - :variable:`Tokudb_FLUSHER_MERGE_LEAF`
     - integer
   * - :variable:`Tokudb_FLUSHER_MERGE_NONLEAF`
     - integer
   * - :variable:`Tokudb_FLUSHER_BALANCE_LEAF`
     - integer
   * - :variable:`Tokudb_HOT_NUM_STARTED`
     - integer
   * - :variable:`Tokudb_HOT_NUM_COMPLETED`
     - integer
   * - :variable:`Tokudb_HOT_NUM_ABORTED`
     - integer
   * - :variable:`Tokudb_HOT_MAX_ROOT_FLUSH_COUNT`
     - integer
   * - :variable:`Tokudb_TXN_BEGIN`
     - integer
   * - :variable:`Tokudb_TXN_BEGIN_READ_ONLY`
     - integer
   * - :variable:`Tokudb_TXN_COMMITS`
     - integer
   * - :variable:`Tokudb_TXN_ABORTS`
     - integer
   * - :variable:`Tokudb_LOGGER_NEXT_LSN`
     - integer
   * - :variable:`Tokudb_LOGGER_WRITES`
     - integer
   * - :variable:`Tokudb_LOGGER_WRITES_BYTES`
     - integer
   * - :variable:`Tokudb_LOGGER_WRITES_UNCOMPRESSED_BYTES`
     - integer
   * - :variable:`Tokudb_LOGGER_WRITES_SECONDS`
     - numeric
   * - :variable:`Tokudb_LOGGER_WAIT_LONG`
     - integer
   * - :variable:`Tokudb_LOADER_NUM_CREATED`
     - integer
   * - :variable:`Tokudb_LOADER_NUM_CURRENT`
     - integer
   * - :variable:`Tokudb_LOADER_NUM_MAX`
     - integer
   * - :variable:`Tokudb_MEMORY_MALLOC_COUNT`
     - integer
   * - :variable:`Tokudb_MEMORY_FREE_COUNT`
     - integer
   * - :variable:`Tokudb_MEMORY_REALLOC_COUNT`
     - integer
   * - :variable:`Tokudb_MEMORY_MALLOC_FAIL`
     - integer
   * - :variable:`Tokudb_MEMORY_REALLOC_FAIL`
     - integer
   * - :variable:`Tokudb_MEMORY_REQUESTED`
     - integer
   * - :variable:`Tokudb_MEMORY_USED`
     - integer
   * - :variable:`Tokudb_MEMORY_FREED`
     - integer
   * - :variable:`Tokudb_MEMORY_MAX_REQUESTED_SIZE`
     - integer
   * - :variable:`Tokudb_MEMORY_LAST_FAILED_SIZE`
     - integer
   * - :variable:`Tokudb_MEM_ESTIMATED_MAXIMUM_MEMORY_FOOTPRINT`
     - integer
   * - :variable:`Tokudb_MEMORY_MALLOCATOR_VERSION`
     - string
   * - :variable:`Tokudb_MEMORY_MMAP_THRESHOLD`
     - integer
   * - :variable:`Tokudb_FILESYSTEM_THREADS_BLOCKED_BY_FULL_DISK`
     - integer
   * - :variable:`Tokudb_FILESYSTEM_FSYNC_TIME`
     - integer
   * - :variable:`Tokudb_FILESYSTEM_FSYNC_NUM`
     - integer
   * - :variable:`Tokudb_FILESYSTEM_LONG_FSYNC_TIME`
     - integer
   * - :variable:`Tokudb_FILESYSTEM_LONG_FSYNC_NUM`
     - integer

.. variable:: Tokudb_DB_OPENS

This variable shows the number of times an individual PerconaFT dictionary file
was opened. This is a not a useful value for a regular user to use for any
purpose due to layers of open/close caching on top.

.. variable:: Tokudb_DB_CLOSES

This variable shows the number of times an individual PerconaFT dictionary file
was closed. This is a not a useful value for a regular user to use for any
purpose due to layers of open/close caching on top.

.. variable:: Tokudb_DB_OPEN_CURRENT

This variable shows the number of currently opened databases.

.. variable:: Tokudb_DB_OPEN_MAX

This variable shows the maximum number of concurrently opened databases.

.. variable:: Tokudb_LEAF_ENTRY_MAX_COMMITTED_XR

This variable shows the maximum number of committed transaction records that
were stored on disk in a new or modified row.

.. variable:: Tokudb_LEAF_ENTRY_MAX_PROVISIONAL_XR

This variable shows the maximum number of provisional transaction records that
were stored on disk in a new or modified row.

.. variable:: Tokudb_LEAF_ENTRY_EXPANDED

This variable shows the number of times that an expanded memory mechanism was
used to store a new or modified row on disk.

.. variable:: Tokudb_LEAF_ENTRY_MAX_MEMSIZE

This variable shows the maximum number of bytes that were stored on disk as a
new or modified row. This is the maximum uncompressed size of any row stored in
|TokuDB| that was created or modified since the server started.

.. variable:: Tokudb_LEAF_ENTRY_APPLY_GC_BYTES_IN

This variable shows the total number of bytes of leaf nodes data before
performing garbage collection for non-flush events.

.. variable:: Tokudb_LEAF_ENTRY_APPLY_GC_BYTES_OUT

This variable shows the total number of bytes of leaf nodes data after
performing garbage collection for non-flush events.

.. variable:: Tokudb_LEAF_ENTRY_NORMAL_GC_BYTES_IN

This variable shows the total number of bytes of leaf nodes data before
performing garbage collection for flush events.

.. variable:: Tokudb_LEAF_ENTRY_NORMAL_GC_BYTES_OUT

This variable shows the total number of bytes of leaf nodes data after
performing garbage collection for flush events.

.. variable:: Tokudb_CHECKPOINT_PERIOD

This variable shows the interval in seconds between the end of an automatic
checkpoint and the beginning of the next automatic checkpoint.

.. variable:: Tokudb_CHECKPOINT_FOOTPRINT

This variable shows at what stage the checkpointer is at. It's used for
debugging purposes only and not a useful value for a normal user.

.. variable:: Tokudb_CHECKPOINT_LAST_BEGAN

This variable shows the time the last checkpoint began. If a checkpoint is
currently in progress, then this time may be later than the time the last
checkpoint completed. If no checkpoint has ever taken place, then this value
will be ``Dec 31, 1969`` on Linux hosts.

.. variable:: Tokudb_CHECKPOINT_LAST_COMPLETE_BEGAN

This variable shows the time the last complete checkpoint started. Any data
that changed after this time will not be captured in the checkpoint.

.. variable:: Tokudb_CHECKPOINT_LAST_COMPLETE_ENDED

This variable shows the time the last complete checkpoint ended.

.. variable:: Tokudb_CHECKPOINT_DURATION

This variable shows time (in seconds) required to complete all
checkpoints.

.. variable:: Tokudb_CHECKPOINT_DURATION_LAST

This variable shows time (in seconds) required to complete the last
checkpoint.

.. variable:: Tokudb_CHECKPOINT_LAST_LSN

This variable shows the last successful checkpoint LSN. Each checkpoint from
the time the PerconaFT environment is created has a monotonically incrementing
LSN. This is not a useful value for a normal user to use for any purpose other
than having some idea of how many checkpoints have occurred since the system
was first created.

.. variable:: Tokudb_CHECKPOINT_TAKEN

This variable shows the number of complete checkpoints that have been taken.

.. variable:: Tokudb_CHECKPOINT_FAILED

This variable shows the number of checkpoints that have failed for any reason.

.. variable:: Tokudb_CHECKPOINT_WAITERS_NOW

This variable shows the current number of threads waiting for the ``checkpoint
safe`` lock. This is a not a useful value for a regular user to use for any
purpose.

.. variable:: Tokudb_CHECKPOINT_WAITERS_MAX

This variable shows the maximum number of threads that concurrently waited for
the ``checkpoint safe`` lock. This is a not a useful value for a regular user to
use for any purpose.

.. variable:: Tokudb_CHECKPOINT_CLIENT_WAIT_ON_MO

This variable shows the number of times a non-checkpoint client thread waited
for the multi-operation lock. It is an internal ``rwlock`` that is similar in
nature to the |InnoDB| kernel mutex, it effectively halts all access to the
PerconaFT API when write locked. The ``begin`` phase of the checkpoint takes
this lock for a brief period.

.. variable:: Tokudb_CHECKPOINT_CLIENT_WAIT_ON_CS

This variable shows the number of times a non-checkpoint client thread waited
for the checkpoint-safe lock. This is the lock taken when you ``SET
tokudb_checkpoint_lock=1``. If a client trying to lock/postpone the
checkpointer has to wait for the currently running checkpoint to complete, that
wait time will be reflected here and summed. This is not a useful metric as
regular users should never be manipulating the checkpoint lock.

.. variable:: Tokudb_CHECKPOINT_BEGIN_TIME

This variable shows the cumulative time (in microseconds) required to mark all
dirty nodes as pending a checkpoint.

.. variable:: Tokudb_CHECKPOINT_LONG_BEGIN_TIME

This variable shows the cumulative actual time (in microseconds) of checkpoint
``begin`` stages that took longer than 1 second.

.. variable:: Tokudb_CHECKPOINT_LONG_BEGIN_COUNT

This variable shows the number of checkpoints whose ``begin`` stage took longer
than 1 second.

.. variable:: Tokudb_CHECKPOINT_END_TIME

This variable shows the time spent in checkpoint end operation in seconds.

.. variable:: Tokudb_CHECKPOINT_LONG_END_TIME

This variable shows the total time of long checkpoints in seconds.

.. variable:: Tokudb_CHECKPOINT_LONG_END_COUNT

This variable shows the number of checkpoints whose ``end_checkpoint``
operations exceeded 1 minute.

.. variable:: Tokudb_CACHETABLE_MISS

This variable shows the number of times the application was unable to access
the data in the internal cache. A cache miss means that date will need to be
read from disk.

.. variable:: Tokudb_CACHETABLE_MISS_TIME

This variable shows the total time, in microseconds, of how long the database
has had to wait for a disk read to complete.

.. variable:: Tokudb_CACHETABLE_PREFETCHES

This variable shows the total number of times that a block of memory has been
prefetched into the database's cache. Data is prefetched when the database's
algorithms determine that a block of memory is likely to be accessed by the
application.

.. variable:: Tokudb_CACHETABLE_SIZE_CURRENT

This variable shows how much of the uncompressed data, in bytes, is
currently in the database's internal cache.

.. variable:: Tokudb_CACHETABLE_SIZE_LIMIT

This variable shows how much of the uncompressed data, in bytes, will fit in
the database's internal cache.

.. variable:: Tokudb_CACHETABLE_SIZE_WRITING

This variable shows the number of bytes that are currently queued up to be
written to disk.

.. variable:: Tokudb_CACHETABLE_SIZE_NONLEAF

This variable shows the amount of memory, in bytes, the current set of non-leaf
nodes occupy in the cache.

.. variable:: Tokudb_CACHETABLE_SIZE_LEAF

This variable shows the amount of memory, in bytes, the current set of
(decompressed) leaf nodes occupy in the cache.

.. variable:: Tokudb_CACHETABLE_SIZE_ROLLBACK

This variable shows the rollback nodes size, in bytes, in the cache.

.. variable:: Tokudb_CACHETABLE_SIZE_CACHEPRESSURE

This variable shows the number of bytes causing cache pressure (the sum of
buffers and work done counters), helps to understand if cleaner threads are
keeping up with workload. It should really be looked at as more of a value to
use in a ratio of cache pressure / cache table size. The closer that ratio
evaluates to 1, the higher the cache pressure.

.. variable:: Tokudb_CACHETABLE_SIZE_CLONED

This variable shows the amount of memory, in bytes, currently used for cloned
nodes. During the checkpoint operation, dirty nodes are cloned prior to
serialization/compression, then written to disk. After which, the memory for
the cloned block is returned for re-use.

.. variable:: Tokudb_CACHETABLE_EVICTIONS

This variable shows the number of blocks evicted from cache. On its own this is
not a useful number as its impact on performance depends entirely on the
hardware and workload in use. For example, two workloads, one random, one
linear for the same starting data set will have two wildly different eviction
patterns.

.. variable:: Tokudb_CACHETABLE_CLEANER_EXECUTIONS

This variable shows the total number of times the cleaner thread loop has
executed.

.. variable:: Tokudb_CACHETABLE_CLEANER_PERIOD

|TokuDB| includes a cleaner thread that optimizes indexes in the background.
This variable is the time, in seconds, between the completion of a group of
cleaner operations and the beginning of the next group of cleaner operations.
The cleaner operations run on a background thread performing work that does not
need to be done on the client thread.

.. variable:: Tokudb_CACHETABLE_CLEANER_ITERATIONS

This variable shows the number of cleaner operations that are performed every
cleaner period.

.. variable:: Tokudb_CACHETABLE_WAIT_PRESSURE_COUNT

This variable shows the number of times a thread was stalled due to cache
pressure.

.. variable:: Tokudb_CACHETABLE_WAIT_PRESSURE_TIME

This variable shows the total time, in microseconds, waiting on cache pressure
to subside.

.. variable:: Tokudb_CACHETABLE_LONG_WAIT_PRESSURE_COUNT

This variable shows the number of times a thread was stalled for more than one
second due to cache pressure.

.. variable:: Tokudb_CACHETABLE_LONG_WAIT_PRESSURE_TIME

This variable shows the total time, in microseconds, waiting on cache pressure
to subside for more than one second.

.. variable:: Tokudb_CACHETABLE_POOL_CLIENT_NUM_THREADS

This variable shows the number of threads in the client thread pool.

.. variable:: Tokudb_CACHETABLE_POOL_CLIENT_NUM_THREADS_ACTIVE

This variable shows the number of currently active threads in the client
thread pool.

.. variable:: Tokudb_CACHETABLE_POOL_CLIENT_QUEUE_SIZE

This variable shows the number of currently queued work items in the client
thread pool.

.. variable:: Tokudb_CACHETABLE_POOL_CLIENT_MAX_QUEUE_SIZE

This variable shows the largest number of queued work items in the client
thread pool.

.. variable:: Tokudb_CACHETABLE_POOL_CLIENT_TOTAL_ITEMS_PROCESSED

This variable shows the total number of work items processed in the client
thread pool.

.. variable:: Tokudb_CACHETABLE_POOL_CLIENT_TOTAL_EXECUTION_TIME

This variable shows the total execution time of processing work items in the
client thread pool.

.. variable:: Tokudb_CACHETABLE_POOL_CACHETABLE_NUM_THREADS

This variable shows the number of threads in the cachetable threadpool.

.. variable:: Tokudb_CACHETABLE_POOL_CACHETABLE_NUM_THREADS_ACTIVE

This variable shows the number of currently active threads in the cachetable
thread pool.

.. variable:: Tokudb_CACHETABLE_POOL_CACHETABLE_QUEUE_SIZE

This variable shows the number of currently queued work items in the cachetable
thread pool. 

.. variable:: Tokudb_CACHETABLE_POOL_CACHETABLE_MAX_QUEUE_SIZE

This variable shows the largest number of queued work items in the cachetable
thread pool.

.. variable:: Tokudb_CACHETABLE_POOL_CACHETABLE_TOTAL_ITEMS_PROCESSED

This variable shows the total number of work items processed in the cachetable
thread pool.

.. variable:: Tokudb_CACHETABLE_POOL_CACHETABLE_TOTAL_EXECUTION_TIME

This variable shows the total execution time of processing work items in the
cachetable thread pool.

.. variable:: Tokudb_CACHETABLE_POOL_CHECKPOINT_NUM_THREADS

This variable shows the number of threads in the checkpoint threadpool.

.. variable:: Tokudb_CACHETABLE_POOL_CHECKPOINT_NUM_THREADS_ACTIVE

This variable shows the number of currently active threads in the checkpoint
thread pool.

.. variable:: Tokudb_CACHETABLE_POOL_CHECKPOINT_QUEUE_SIZE

This variable shows the number of currently queued work items in the checkpoint
thread pool. 

.. variable:: Tokudb_CACHETABLE_POOL_CHECKPOINT_MAX_QUEUE_SIZE

This variable shows the largest number of queued work items in the checkpoint
thread pool.

.. variable:: Tokudb_CACHETABLE_POOL_CHECKPOINT_TOTAL_ITEMS_PROCESSED

This variable shows the total number of work items processed in the checkpoint
thread pool.

.. variable:: Tokudb_CACHETABLE_POOL_CHECKPOINT_TOTAL_EXECUTION_TIME

This variable shows the total execution time of processing work items in the
checkpoint thread pool.

.. variable:: Tokudb_LOCKTREE_MEMORY_SIZE

This variable shows the amount of memory, in bytes, that the locktree is
currently using.

.. variable:: Tokudb_LOCKTREE_MEMORY_SIZE_LIMIT

This variable shows the maximum amount of memory, in bytes, that the locktree
is allowed to use.

.. variable:: Tokudb_LOCKTREE_ESCALATION_NUM

This variable shows the number of times the locktree needed to run lock
escalation to reduce its memory footprint.

.. variable:: Tokudb_LOCKTREE_ESCALATION_SECONDS

This variable shows the total number of seconds spent performing locktree
escalation.

.. variable:: Tokudb_LOCKTREE_LATEST_POST_ESCALATION_MEMORY_SIZE

This variable shows the locktree size, in bytes, after most current locktree
escalation.

.. variable:: Tokudb_LOCKTREE_OPEN_CURRENT

This variable shows the number of locktrees that are currently opened.

.. variable:: Tokudb_LOCKTREE_PENDING_LOCK_REQUESTS

This variable shows the number of requests waiting for a lock grant.

.. variable:: Tokudb_LOCKTREE_STO_ELIGIBLE_NUM

This variable shows the number of locktrees eligible for ``Single Transaction
optimizations``. STO optimization are behaviors that can happen within the
locktree when there is exactly one transaction active within the locktree. This
is a not a useful value for a regular user to use for any purpose.

.. variable:: Tokudb_LOCKTREE_STO_ENDED_NUM

This variable shows the total number of times a ``Single Transaction
Optimization`` was ended early due to another transaction starting. STO
optimization are behaviors that can happen within the locktree when there is
exactly one transaction active within the locktree. This is a not a useful
value for a regular user to use for any purpose.

.. variable:: Tokudb_LOCKTREE_STO_ENDED_SECONDS

This variable shows the total number of seconds ending the ``Single
Transaction Optimizations``. STO optimization are behaviors that can happen
within the locktree when there is exactly one transaction active within the
locktree. This is a not a useful value for a regular user to use for any
purpose.

.. variable:: Tokudb_LOCKTREE_WAIT_COUNT

This variable shows the number of times that a lock request could not be
acquired because of a conflict with some other transaction. PerconaFT lock
request  cycles to try to obtain a lock, if it can not get a lock, it
sleeps/waits and times out, checks to get the lock again, repeat. This value
indicates the number of cycles it needed to execute before it obtained the
lock. 

.. variable:: Tokudb_LOCKTREE_WAIT_TIME

This variable shows the total time, in microseconds, spent by client waiting
for a lock conflict to be resolved.

.. variable:: Tokudb_LOCKTREE_LONG_WAIT_COUNT

This variable shows number of lock waits greater than one second in duration.

.. variable:: Tokudb_LOCKTREE_LONG_WAIT_TIME

This variable shows the total time, in microseconds, of the long waits.

.. variable:: Tokudb_LOCKTREE_TIMEOUT_COUNT

This variable shows the number of times that a lock request timed out.

.. variable:: Tokudb_LOCKTREE_WAIT_ESCALATION_COUNT

When the sum of the sizes of locks taken reaches the lock tree limit, we run
lock escalation on a background thread. The clients threads need to wait for
escalation to consolidate locks and free up memory. This variables shows the
number of times a client thread had to wait on lock escalation.

.. variable:: Tokudb_LOCKTREE_WAIT_ESCALATION_TIME

This variable shows the total time, in microseconds, that a client thread spent
waiting for lock escalation to free up memory.

.. variable:: Tokudb_LOCKTREE_LONG_WAIT_ESCALATION_COUNT

This variable shows number of times that a client thread had to wait on lock
escalation and the wait time was greater than one second.

.. variable:: Tokudb_LOCKTREE_LONG_WAIT_ESCALATION_TIME

This variable shows the total time, in microseconds, of the long waits for lock
escalation to free up memory.

.. variable:: Tokudb_DICTIONARY_UPDATES

This variable shows the total number of rows that have been updated in all
primary and secondary indexes combined, if those updates have been done with a
separate recovery log entry per index.

.. variable:: Tokudb_DICTIONARY_BROADCAST_UPDATES

This variable shows the number of broadcast updates that have been successfully
performed. A broadcast update is an update that affects all rows in a
dictionary.

.. variable:: Tokudb_DESCRIPTOR_SET

This variable shows the number of time a descriptor was updated when the entire
dictionary was updated (for example, when the schema has been changed).

.. variable:: Tokudb_MESSAGES_IGNORED_BY_LEAF_DUE_TO_MSN

This variable shows the number of messages that were ignored by a leaf because
it had already been applied.

.. variable:: Tokudb_TOTAL_SEARCH_RETRIES

Internal value that is no use to anyone other than a developer debugging a
specific query/search issue.

.. variable:: Tokudb_SEARCH_TRIES_GT_HEIGHT

Internal value that is no use to anyone other than a developer debugging a
specific query/search issue.

.. variable:: Tokudb_SEARCH_TRIES_GT_HEIGHTPLUS3

Internal value that is no use to anyone other than a developer debugging a
specific query/search issue.

.. variable:: Tokudb_LEAF_NODES_FLUSHED_NOT_CHECKPOINT

This variable shows the number of leaf nodes flushed to disk, not for
checkpoint.

.. variable:: Tokudb_LEAF_NODES_FLUSHED_NOT_CHECKPOINT_BYTES

This variable shows the size, in bytes, of leaf nodes flushed to disk, not
for checkpoint.

.. variable:: Tokudb_LEAF_NODES_FLUSHED_NOT_CHECKPOINT_UNCOMPRESSED_BYTES

This variable shows the size, in bytes, of uncompressed leaf nodes flushed to
disk not for checkpoint.

.. variable:: Tokudb_LEAF_NODES_FLUSHED_NOT_CHECKPOINT_SECONDS

This variable shows the number of seconds waiting for I/O when writing leaf
nodes flushed to disk, not for checkpoint

.. variable:: Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_NOT_CHECKPOINT

This variable shows the number of non-leaf nodes flushed to disk, not for
checkpoint.

.. variable:: Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_NOT_CHECKPOINT_BYTES

This variable shows the size, in bytes, of non-leaf nodes flushed to disk, not
for checkpoint.

.. variable:: Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_NOT_CHECKPOINT_UNCOMPRESSE

This variable shows the size, in bytes, of uncompressed non-leaf nodes flushed
to disk not for checkpoint.

.. variable:: Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_NOT_CHECKPOINT_SECONDS

This variable shows the number of seconds waiting for I/O when writing non-leaf
nodes flushed to disk, not for checkpoint

.. variable:: Tokudb_LEAF_NODES_FLUSHED_CHECKPOINT

This variable shows the number of leaf nodes flushed to disk, for checkpoint.

.. variable:: Tokudb_LEAF_NODES_FLUSHED_CHECKPOINT_BYTES

This variable shows the size, in bytes, of leaf nodes flushed to disk, for
checkpoint.

.. variable:: Tokudb_LEAF_NODES_FLUSHED_CHECKPOINT_UNCOMPRESSED_BYTES

This variable shows the size, in bytes, of uncompressed leaf nodes flushed to
disk for checkpoint.

.. variable:: Tokudb_LEAF_NODES_FLUSHED_CHECKPOINT_SECONDS

This variable shows the number of seconds waiting for I/O when writing leaf
nodes flushed to disk for checkpoint

.. variable:: Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_CHECKPOINT

This variable shows the number of non-leaf nodes flushed to disk, for
checkpoint.

.. variable:: Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_CHECKPOINT_BYTES

This variable shows the size, in bytes, of non-leaf nodes flushed to disk, for
checkpoint.

.. variable:: Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_CHECKPOINT_UNCOMPRESSED_BY

This variable shows the size, in bytes, of uncompressed non-leaf nodes flushed
to disk for checkpoint.

.. variable:: Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_CHECKPOINT_SECONDS

This variable shows the number of seconds waiting for I/O when writing non-leaf
nodes flushed to disk for checkpoint

.. variable:: Tokudb_LEAF_NODE_COMPRESSION_RATIO

This variable shows the ratio of uncompressed bytes (in-memory) to compressed
bytes (on-disk) for leaf nodes.

.. variable:: Tokudb_NONLEAF_NODE_COMPRESSION_RATIO

This variable shows the ratio of uncompressed bytes (in-memory) to compressed
bytes (on-disk) for non-leaf nodes.

.. variable:: Tokudb_OVERALL_NODE_COMPRESSION_RATIO

This variable shows the ratio of uncompressed bytes (in-memory) to compressed
bytes (on-disk) for all nodes.

.. variable:: Tokudb_NONLEAF_NODE_PARTIAL_EVICTIONS

This variable shows the number of times a partition of a non-leaf node was
evicted from the cache.

.. variable:: Tokudb_NONLEAF_NODE_PARTIAL_EVICTIONS_BYTES

This variable shows the amount, in bytes, of memory freed by evicting
partitions of non-leaf nodes from the cache.

.. variable:: Tokudb_LEAF_NODE_PARTIAL_EVICTIONS

This variable shows the number of times a partition of a leaf node was evicted
from the cache.

.. variable:: Tokudb_LEAF_NODE_PARTIAL_EVICTIONS_BYTES

This variable shows the amount, in bytes, of memory freed by evicting
partitions of leaf nodes from the cache.

.. variable:: Tokudb_LEAF_NODE_FULL_EVICTIONS

This variable shows the number of times a full leaf node was evicted from the
cache.

.. variable:: Tokudb_LEAF_NODE_FULL_EVICTIONS_BYTES

This variable shows the amount, in bytes, of memory freed by evicting full leaf
nodes from the cache.

.. variable:: Tokudb_NONLEAF_NODE_FULL_EVICTIONS

This variable shows the number of times a full non-leaf node was evicted from
the cache.

.. variable:: Tokudb_NONLEAF_NODE_FULL_EVICTIONS_BYTES

This variable shows the amount, in bytes, of memory freed by evicting full
non-leaf nodes from the cache.

.. variable:: Tokudb_LEAF_NODES_CREATED

This variable shows the number of created leaf nodes.

.. variable:: Tokudb_NONLEAF_NODES_CREATED

This variable shows the number of created non-leaf nodes.

.. variable:: Tokudb_LEAF_NODES_DESTROYED

This variable shows the number of destroyed leaf nodes.

.. variable:: Tokudb_NONLEAF_NODES_DESTROYED

This variable shows the number of destroyed non-leaf nodes.

.. variable:: Tokudb_MESSAGES_INJECTED_AT_ROOT_BYTES

This variable shows the size, in bytes, of messages injected at root (for all
trees).

.. variable:: Tokudb_MESSAGES_FLUSHED_FROM_H1_TO_LEAVES_BYTES

This variable shows the size, in bytes, of messages flushed from ``h1`` nodes
to leaves.

.. variable:: Tokudb_MESSAGES_IN_TREES_ESTIMATE_BYTES

This variable shows the estimated size, in bytes, of messages currently in
trees. 

.. variable:: Tokudb_MESSAGES_INJECTED_AT_ROOT

This variables shows the number of messages that were injected at root node of
a tree.

.. variable:: Tokudb_BROADCASE_MESSAGES_INJECTED_AT_ROOT

This variable shows the number of broadcast messages dropped into the root node
of a tree. These are things such as the result of ``OPTIMIZE TABLE`` and a few
other operations. This is not a useful metric for a regular user to use for any
purpose.

.. variable:: Tokudb_BASEMENTS_DECOMPRESSED_TARGET_QUERY

This variable shows the number of basement nodes decompressed for queries.

.. variable:: Tokudb_BASEMENTS_DECOMPRESSED_PRELOCKED_RANGE

This variable shows the number of basement nodes aggressively decompressed by
queries.

.. variable:: Tokudb_BASEMENTS_DECOMPRESSED_PREFETCH

This variable shows the number of basement nodes decompressed by a prefetch
thread.

.. variable:: Tokudb_BASEMENTS_DECOMPRESSED_FOR_WRITE

This variable shows the number of basement nodes decompressed for writes.

.. variable:: Tokudb_BUFFERS_DECOMPRESSED_TARGET_QUERY

This variable shows the number of buffers decompressed for queries.

.. variable:: Tokudb_BUFFERS_DECOMPRESSED_PRELOCKED_RANGE

This variable shows the number of buffers decompressed by queries aggressively.

.. variable:: Tokudb_BUFFERS_DECOMPRESSED_PREFETCH

This variable shows the number of buffers decompressed by a prefetch thread.

.. variable:: Tokudb_BUFFERS_DECOMPRESSED_FOR_WRITE

This variable shows the number of buffers decompressed for writes.

.. variable:: Tokudb_PIVOTS_FETCHED_FOR_QUERY

This variable shows the number of pivot nodes fetched for queries.

.. variable:: Tokudb_PIVOTS_FETCHED_FOR_QUERY_BYTES

This variable shows the number of bytes of pivot nodes fetched for queries.

.. variable:: Tokudb_PIVOTS_FETCHED_FOR_QUERY_SECONDS

This variable shows the number of seconds waiting for I/O when fetching pivot
nodes for queries.

.. variable:: Tokudb_PIVOTS_FETCHED_FOR_PREFETCH

This variable shows the number of pivot nodes fetched by a prefetch thread.

.. variable:: Tokudb_PIVOTS_FETCHED_FOR_PREFETCH_BYTES

This variable shows the number of bytes of pivot nodes fetched for queries.

.. variable:: Tokudb_PIVOTS_FETCHED_FOR_PREFETCH_SECONDS

This variable shows the number seconds waiting for I/O when fetching pivot
nodes by a prefetch thread.

.. variable:: Tokudb_PIVOTS_FETCHED_FOR_WRITE

This variable shows the number of pivot nodes fetched for writes.

.. variable:: Tokudb_PIVOTS_FETCHED_FOR_WRITE_BYTES

This variable shows the number of bytes of pivot nodes fetched for writes.

.. variable:: Tokudb_PIVOTS_FETCHED_FOR_WRITE_SECONDS

This variable shows the number of seconds waiting for I/O when fetching pivot
nodes for writes.

.. variable:: Tokudb_BASEMENTS_FETCHED_TARGET_QUERY

This variable shows the number of basement nodes fetched from disk for queries.

.. variable:: Tokudb_BASEMENTS_FETCHED_TARGET_QUERY_BYTES

This variable shows the number of basement node bytes fetched from disk for
queries.

.. variable:: Tokudb_BASEMENTS_FETCHED_TARGET_QUERY_SECONDS

This variable shows the number of seconds waiting for I/O when fetching
basement nodes from disk for queries.

.. variable:: Tokudb_BASEMENTS_FETCHED_PRELOCKED_RANGE

This variable shows the number of basement nodes fetched from disk
aggressively.

.. variable:: Tokudb_BASEMENTS_FETCHED_PRELOCKED_RANGE_BYTES

This variable shows the number of basement node bytes fetched from disk
aggressively.

.. variable:: Tokudb_BASEMENTS_FETCHED_PRELOCKED_RANGE_SECONDS

This variable shows the number of seconds waiting for I/O when fetching
basement nodes from disk aggressively.

.. variable:: Tokudb_BASEMENTS_FETCHED_PREFETCH

This variable shows the number of basement nodes fetched from disk by a
prefetch thread.

.. variable:: Tokudb_BASEMENTS_FETCHED_PREFETCH_BYTES

This variable shows the number of basement node bytes fetched from disk by a
prefetch thread.

.. variable:: Tokudb_BASEMENTS_FETCHED_PREFETCH_SECONDS

This variable shows the number of seconds waiting for I/O when fetching
basement nodes from disk by a prefetch thread.

.. variable:: Tokudb_BASEMENTS_FETCHED_FOR_WRITE

This variable shows the number of buffers fetched from disk for writes.

.. variable:: Tokudb_BASEMENTS_FETCHED_FOR_WRITE_BYTES

This variable shows the number of buffer bytes fetched from disk for writes.

.. variable:: Tokudb_BASEMENTS_FETCHED_FOR_WRITE_SECONDS

This variable shows the number of seconds waiting for I/O when fetching buffers
from disk for writes.

.. variable:: Tokudb_BUFFERS_FETCHED_TARGET_QUERY

This variable shows the number of buffers fetched from disk for queries.

.. variable:: Tokudb_BUFFERS_FETCHED_TARGET_QUERY_BYTES

This variable shows the number of buffer bytes fetched from disk for queries.

.. variable:: Tokudb_BUFFERS_FETCHED_TARGET_QUERY_SECONDS

This variable shows the number of seconds waiting for I/O when fetching buffers
from disk for queries.

.. variable:: Tokudb_BUFFERS_FETCHED_PRELOCKED_RANGE

This variable shows the number of buffers fetched from disk aggressively.

.. variable:: Tokudb_BUFFERS_FETCHED_PRELOCKED_RANGE_BYTES

This variable shows the number of buffer bytes fetched from disk aggressively.

.. variable:: Tokudb_BUFFERS_FETCHED_PRELOCKED_RANGE_SECONDS

This variable shows the number of seconds waiting for I/O when fetching buffers
from disk aggressively.

.. variable:: Tokudb_BUFFERS_FETCHED_PREFETCH

This variable shows the number of buffers fetched from disk aggressively.

.. variable:: Tokudb_BUFFERS_FETCHED_PREFETCH_BYTES

This variable shows the number of buffer bytes fetched from disk by a prefetch
thread.

.. variable:: Tokudb_BUFFERS_FETCHED_PREFETCH_SECONDS

This variable shows the number of seconds waiting for I/O when fetching buffers
from disk by a prefetch thread.

.. variable:: Tokudb_BUFFERS_FETCHED_FOR_WRITE

This variable shows the number of buffers fetched from disk for writes.

.. variable:: Tokudb_BUFFERS_FETCHED_FOR_WRITE_BYTES

This variable shows the number of buffer bytes fetched from disk for writes.

.. variable:: Tokudb_BUFFERS_FETCHED_FOR_WRITE_SECONDS

This variable shows the number of seconds waiting for I/O when fetching buffers
from disk for writes.

.. variable:: Tokudb_LEAF_COMPRESSION_TO_MEMORY_SECONDS

This variable shows the total time, in seconds, spent compressing leaf nodes.

.. variable:: Tokudb_LEAF_SERIALIZATION_TO_MEMORY_SECONDS

This variable shows the total time, in seconds, spent serializing leaf nodes.

.. variable:: Tokudb_LEAF_DECOMPRESSION_TO_MEMORY_SECONDS

This variable shows the total time, in seconds, spent decompressing leaf nodes.

.. variable:: Tokudb_LEAF_DESERIALIZATION_TO_MEMORY_SECONDS

This variable shows the total time, in seconds, spent deserializing leaf nodes.

.. variable:: Tokudb_NONLEAF_COMPRESSION_TO_MEMORY_SECONDS

This variable shows the total time, in seconds, spent compressing non leaf
nodes.

.. variable:: Tokudb_NONLEAF_SERIALIZATION_TO_MEMORY_SECONDS

This variable shows the total time, in seconds, spent serializing non leaf
nodes.

.. variable:: Tokudb_NONLEAF_DECOMPRESSION_TO_MEMORY_SECONDS

This variable shows the total time, in seconds, spent decompressing non leaf
nodes.

.. variable:: Tokudb_NONLEAF_DESERIALIZATION_TO_MEMORY_SECONDS

This variable shows the total time, in seconds, spent deserializing non leaf
nodes.

.. variable:: Tokudb_PROMOTION_ROOTS_SPLIT

This variable shows the number of times the root split during promotion.

.. variable:: Tokudb_PROMOTION_LEAF_ROOTS_INJECTED_INTO

This variable shows the number of times a message stopped at a root with
height ``0``.

.. variable:: Tokudb_PROMOTION_H1_ROOTS_INJECTED_INTO

This variable shows the number of times a message stopped at a root with
height ``1``.

.. variable:: Tokudb_PROMOTION_INJECTIONS_AT_DEPTH_0

This variable shows the number of times a message stopped at depth ``0``.

.. variable:: Tokudb_PROMOTION_INJECTIONS_AT_DEPTH_1

This variable shows the number of times a message stopped at depth ``1``.

.. variable:: Tokudb_PROMOTION_INJECTIONS_AT_DEPTH_2

This variable shows the number of times a message stopped at depth ``2``.

.. variable:: Tokudb_PROMOTION_INJECTIONS_AT_DEPTH_3

This variable shows the number of times a message stopped at depth ``3``.

.. variable:: Tokudb_PROMOTION_INJECTIONS_LOWER_THAN_DEPTH_3

This variable shows the number of times a message was promoted past depth
``3``.

.. variable:: Tokudb_PROMOTION_STOPPED_NONEMPTY_BUFFER

This variable shows the number of times a message stopped because it reached
a nonempty buffer.

.. variable:: Tokudb_PROMOTION_STOPPED_AT_HEIGHT_1

This variable shows the number of times a message stopped because it had
reached height ``1``.

.. variable:: Tokudb_PROMOTION_STOPPED_CHILD_LOCKED_OR_NOT_IN_MEMORY

This variable shows the number of times a message stopped because it could not
cheaply get access to a child.

.. variable:: Tokudb_PROMOTION_STOPPED_CHILD_NOT_FULLY_IN_MEMORY

This variable shows the number of times a message stopped because it could not
cheaply get access to a child.

.. variable:: Tokudb_PROMOTION_STOPPED_AFTER_LOCKING_CHILD

This variable shows the number of times a message stopped before a child which
had been locked.

.. variable:: Tokudb_BASEMENT_DESERIALIZATION_FIXED_KEY

This variable shows the number of basement nodes deserialized where all keys
had the same size, leaving the basement in a format that is optimal for
in-memory workloads.

.. variable:: Tokudb_BASEMENT_DESERIALIZATION_VARIABLE_KEY

This variable shows the number of basement nodes deserialized where all keys
did not have the same size, and thus ineligible for an in-memory optimization.

.. variable:: Tokudb_PRO_RIGHTMOST_LEAF_SHORTCUT_SUCCESS

This variable shows the number of times a message injection detected a series
of sequential inserts to the rightmost side of the tree and successfully
applied an insert message directly to the rightmost leaf node. This is a not a
useful value for a regular user to use for any purpose.

.. variable:: Tokudb_PRO_RIGHTMOST_LEAF_SHORTCUT_FAIL_POS

This variable shows the number of times a message injection detected a series
of sequential inserts to the rightmost side of the tree and was unable to
follow the pattern of directly applying an insert message directly to the
rightmost leaf node because the key does not continue the sequence. This is a
not a useful value for a regular user to use for any purpose.

.. variable:: Tokudb_RIGHTMOST_LEAF_SHORTCUT_FAIL_REACTIVE

This variable shows the number of times a message injection detected a series
of sequential inserts to the rightmost side of the tree and was unable to
follow the pattern of directly applying an insert message directly to the
rightmost leaf node because the leaf is full. This is a not a useful value for
a regular user to use for any purpose.

.. variable:: Tokudb_CURSOR_SKIP_DELETED_LEAF_ENTRY

This variable shows the number of leaf entries skipped during search/scan
because the result of message application and reconciliation of the leaf entry
MVCC stack reveals that the leaf entry is ``deleted`` in the current
transactions view. It is a good indicator that there might be excessive garbage
in a tree if a range scan seems to take too long.

.. variable:: Tokudb_FLUSHER_CLEANER_TOTAL_NODES

This variable shows the total number of nodes potentially flushed by flusher or
cleaner threads. This is a not a useful value for a regular user to use for any
purpose.

.. variable:: Tokudb_FLUSHER_CLEANER_H1_NODES

This variable shows the number of height ``1`` nodes that had messages flushed
by flusher or cleaner threads, i.e., internal nodes immediately above leaf
nodes. This is a not a useful value for a regular user to use for any purpose.

.. variable:: Tokudb_FLUSHER_CLEANER_HGT1_NODES

This variable shows the number of nodes with height greater than ``1`` that had
messages flushed by flusher or cleaner threads. This is a not a useful value
for a regular user to use for any purpose.

.. variable:: Tokudb_FLUSHER_CLEANER_EMPTY_NODES

This variable shows the number of nodes cleaned by flusher or cleaner threads
which had empty message buffers. This is a not a useful value for a regular
user to use for any purpose.

.. variable:: Tokudb_FLUSHER_CLEANER_NODES_DIRTIED

This variable shows the number of nodes dirtied by flusher or cleaner threads
as a result of flushing messages downward. This is a not a useful value for a
regular user to use for any purpose.

.. variable:: Tokudb_FLUSHER_CLEANER_MAX_BUFFER_SIZE

This variable shows the maximum bytes in a message buffer flushed by flusher or
cleaner threads. This is a not a useful value for a regular user to use for any
purpose.

.. variable:: Tokudb_FLUSHER_CLEANER_MIN_BUFFER_SIZE

This variable shows the minimum bytes in a message buffer flushed by flusher or
cleaner threads. This is a not a useful value for a regular user to use for any
purpose.

.. variable:: Tokudb_FLUSHER_CLEANER_TOTAL_BUFFER_SIZE

This variable shows the total bytes in buffers flushed by flusher and cleaner
threads. This is a not a useful value for a regular user to use for any purpose.

.. variable:: Tokudb_FLUSHER_CLEANER_MAX_BUFFER_WORKDONE

This variable shows the maximum bytes worth of work done in a message buffer
flushed by flusher or cleaner threads. This is a not a useful value for a
regular user to use for any purpose.

.. variable:: Tokudb_FLUSHER_CLEANER_MIN_BUFFER_WORKDONE

This variable shows the minimum bytes worth of work done in a message buffer
flushed by flusher or cleaner threads. This is a not a useful value for a
regular user to use for any purpose.

.. variable:: Tokudb_FLUSHER_CLEANER_TOTAL_BUFFER_WORKDONE

This variable shows the total bytes worth of work done in buffers flushed by
flusher or cleaner threads. This is a not a useful value for a regular user to
use for any purpose.

.. variable:: Tokudb_FLUSHER_CLEANER_NUM_LEAF_MERGES_STARTED

This variable shows the number of times flusher and cleaner threads tried to
merge two leafs. This is a not a useful value for a regular user to use for any
purpose.

.. variable:: Tokudb_FLUSHER_CLEANER_NUM_LEAF_MERGES_RUNNING

This variable shows the number of flusher and cleaner threads leaf merges in
progress. This is a not a useful value for a regular user to use for any
purpose.

.. variable:: Tokudb_FLUSHER_CLEANER_NUM_LEAF_MERGES_COMPLETED

This variable shows the number of successful flusher and cleaner threads leaf
merges. This is a not a useful value for a regular user to use for any purpose.

.. variable:: Tokudb_FLUSHER_CLEANER_NUM_DIRTIED_FOR_LEAF_MERGE

This variable shows the number of nodes dirtied by flusher or cleaner threads
performing leaf node merges. This is a not a useful value for a regular user to
use for any purpose.

.. variable:: Tokudb_FLUSHER_FLUSH_TOTAL

This variable shows the total number of flushes done by flusher threads or
cleaner threads. This is a not a useful value for a regular user to use for any
purpose.

.. variable:: Tokudb_FLUSHER_FLUSH_IN_MEMORY

This variable shows the number of in memory flushes (required no disk reads) by
flusher or cleaner threads. This is a not a useful value for a regular user to
use for any purpose.

.. variable:: Tokudb_FLUSHER_FLUSH_NEEDED_IO

This variable shows the number of flushes that read something off disk by
flusher or cleaner threads. This is a not a useful value for a regular user to
use for any purpose.

.. variable:: Tokudb_FLUSHER_FLUSH_CASCADES

This variable shows the number of flushes that triggered a flush in child node
by flusher or cleaner threads. This is a not a useful value for a regular user
to use for any purpose.

.. variable:: Tokudb_FLUSHER_FLUSH_CASCADES_1

This variable shows the number of flushes that triggered one cascading flush by
flusher or cleaner threads. This is a not a useful value for a regular user to
use for any purpose.

.. variable:: Tokudb_FLUSHER_FLUSH_CASCADES_2

This variable shows the number of flushes that triggered two cascading flushes
by flusher or cleaner threads. This is a not a useful value for a regular user
to use for any purpose.

.. variable:: Tokudb_FLUSHER_FLUSH_CASCADES_3

This variable shows the number of flushes that triggered three cascading
flushes by flusher or cleaner threads. This is a not a useful value for a
regular user to use for any purpose.

.. variable:: Tokudb_FLUSHER_FLUSH_CASCADES_4

This variable shows the number of flushes that triggered four cascading
flushes by flusher or cleaner threads. This is a not a useful value for a
regular user to use for any purpose.

.. variable:: Tokudb_FLUSHER_FLUSH_CASCADES_5

This variable shows the number of flushes that triggered five cascading
flushes by flusher or cleaner threads. This is a not a useful value for a
regular user to use for any purpose.

.. variable:: Tokudb_FLUSHER_FLUSH_CASCADES_GT_5

This variable shows the number of flushes that triggered more than five
cascading flushes by flusher or cleaner threads. This is a not a useful value
for a regular user to use for any purpose.

.. variable:: Tokudb_FLUSHER_SPLIT_LEAF

This variable shows the total number of leaf node splits done by flusher
threads or cleaner threads. This is a not a useful value for a regular user to
use for any purpose.

.. variable:: Tokudb_FLUSHER_SPLIT_NONLEAF

This variable shows the total number of non-leaf node splits done by flusher
threads or cleaner threads. This is a not a useful value for a regular user to
use for any purpose.

.. variable:: Tokudb_FLUSHER_MERGE_LEAF

This variable shows the total number of leaf node merges done by flusher
threads or cleaner threads. This is a not a useful value for a regular user to
use for any purpose.

.. variable:: Tokudb_FLUSHER_MERGE_NONLEAF

This variable shows the total number of non-leaf node merges done by flusher
threads or cleaner threads. This is a not a useful value for a regular user to
use for any purpose.

.. variable:: Tokudb_FLUSHER_BALANCE_LEAF

This variable shows the number of times two adjacent leaf nodes were rebalanced
or had their content redistributed evenly by flusher or cleaner threads. This
is a not a useful value for a regular user to use for any purpose.

.. variable:: Tokudb_HOT_NUM_STARTED

This variable shows the number of hot operations started (``OPTIMIZE TABLE``).
This is a not a useful value for a regular user to use for any purpose.

.. variable:: Tokudb_HOT_NUM_COMPLETED

This variable shows the number of hot operations completed (``OPTIMIZE TABLE``).
This is a not a useful value for a regular user to use for any purpose.

.. variable:: Tokudb_HOT_NUM_ABORTED

This variable shows the number of hot operations aborted (``OPTIMIZE TABLE``).
This is a not a useful value for a regular user to use for any purpose.

.. variable:: Tokudb_HOT_MAX_ROOT_FLUSH_COUNT

This variable shows the maximum number of flushes from root ever required to
optimize trees. This is a not a useful value for a regular user to use for any
purpose.

.. variable:: Tokudb_TXN_BEGIN

This variable shows the number of transactions that have been started.

.. variable:: Tokudb_TXN_BEGIN_READ_ONLY

This variable shows the number of read-only transactions started.

.. variable:: Tokudb_TXN_COMMITS

This variable shows the total number of transactions that have been committed.

.. variable:: Tokudb_TXN_ABORTS

This variable shows the total number of transactions that have been aborted.

.. variable:: Tokudb_LOGGER_NEXT_LSN

This variable shows the recovery logger next LSN. This is a not a useful value
for a regular user to use for any purpose.

.. variable:: Tokudb_LOGGER_WRITES

This variable shows the number of times the logger has written to disk.

.. variable:: Tokudb_LOGGER_WRITES_BYTES

This variable shows the number of bytes the logger has written to disk.

.. variable:: Tokudb_LOGGER_WRITES_UNCOMPRESSED_BYTES

This variable shows the number of uncompressed bytes the logger has written to
disk.

.. variable:: Tokudb_LOGGER_WRITES_SECONDS

This variable shows the number of seconds waiting for IO when writing logs to
disk.

.. variable:: Tokudb_LOGGER_WAIT_LONG

This variable shows the number of times a logger write operation required 100ms
or more.

.. variable:: Tokudb_LOADER_NUM_CREATED

This variable shows the number of times one of our internal objects, a loader,
has been created.

.. variable:: Tokudb_LOADER_NUM_CURRENT

This variable shows the number of loaders that currently exist.

.. variable:: Tokudb_LOADER_NUM_MAX

This variable shows the maximum number of loaders that ever existed
simultaneously.

.. variable:: Tokudb_MEMORY_MALLOC_COUNT

This variable shows the number of ``malloc`` operations by PerconaFT.

.. variable:: Tokudb_MEMORY_FREE_COUNT

This variable shows the number of ``free`` operations by PerconaFT.

.. variable:: Tokudb_MEMORY_REALLOC_COUNT

This variable shows the number of ``realloc`` operations by PerconaFT.

.. variable:: Tokudb_MEMORY_MALLOC_FAIL

This variable shows the number of ``malloc`` operations that failed by
PerconaFT.

.. variable:: Tokudb_MEMORY_REALLOC_FAIL

This variable shows the number of ``realloc`` operations that failed by
PerconaFT.

.. variable:: Tokudb_MEMORY_REQUESTED

This variable shows the number of bytes requested by PerconaFT.

.. variable:: Tokudb_MEMORY_USED

This variable shows the number of bytes used (requested + overhead) by
PerconaFT.

.. variable:: Tokudb_MEMORY_FREED

This variable shows the number of bytes freed by PerconaFT.

.. variable:: Tokudb_MEMORY_MAX_REQUESTED_SIZE

This variable shows the largest attempted allocation size by PerconaFT.

.. variable:: Tokudb_MEMORY_LAST_FAILED_SIZE

This variable shows the size of the last failed allocation attempt by
PerconaFT.

.. variable:: Tokudb_MEM_ESTIMATED_MAXIMUM_MEMORY_FOOTPRINT

This variable shows the maximum memory footprint of the storage engine, the
max value of (used - freed).

.. variable:: Tokudb_MEMORY_MALLOCATOR_VERSION

This variable shows the mallocator version detected by PerconaFT.

.. variable:: Tokudb_MEMORY_MMAP_THRESHOLD

This variable shows the ``mmap`` threshold in PerconaFT, anything larger than
this gets ``mmap'ed``.

.. variable:: Tokudb_FILESYSTEM_THREADS_BLOCKED_BY_FULL_DISK

This variable shows the number of threads that are currently blocked because
they are attempting to write to a full disk. This is normally zero. If this
value is non-zero, then a warning will appear in the ``disk free space`` field.

.. variable:: Tokudb_FILESYSTEM_FSYNC_TIME

This variable shows the total time, in microseconds, used to ``fsync`` to disk.

.. variable:: Tokudb_FILESYSTEM_FSYNC_NUM

This variable shows the total number of times the database has flushed the
operating system's file buffers to disk.

.. variable:: Tokudb_FILESYSTEM_LONG_FSYNC_TIME

This variable shows the total time, in microseconds, used to ``fsync`` to dis
k when the operation required more than one second.

.. variable:: Tokudb_FILESYSTEM_LONG_FSYNC_NUM

This variable shows the total number of times the database has flushed the
operating system's file buffers to disk and this operation required more than
one second.
