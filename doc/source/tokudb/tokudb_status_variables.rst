.. _tokudb_status_variables:

=======================
TokuDB Status Variables
=======================

.. Important:: 

   Starting with :ref:`8.0.28-19`, the TokuDB storage engine is no longer supported. We have removed the storage engine from the installation packages and disabled the storage engine in our binary builds.

   Starting with :ref:`8.0.26-16`, the binary builds and packages include but disable the TokuDB storage engine plugins. The ``tokudb_enabled`` option and the ``tokudb_backup_enabled`` option control the state of the plugins and have a default setting of ``FALSE``. The result of attempting to load the plugins are the plugins fail to initialize and print a deprecation message.

   We recommend :ref:`migrate-myrocks`. To enable the plugins to migrate to another storage engine, set the ``tokudb_enabled`` and ``tokudb_backup_enabled`` options to ``TRUE`` in your ``my.cnf`` file and restart your server instance. Then, you can load the plugins.

   The TokuDB Storage Engine was `declared as deprecated <https://www.percona.com/doc/percona-server/8.0/release-notes/Percona-Server-8.0.13-3.html>`__ in Percona Server for MySQL 8.0. For more information, see the Percona blog post: `Heads-Up: TokuDB Support Changes and Future Removal from Percona Server for MySQL 8.0 <https://www.percona.com/blog/2021/05/21/tokudb-support-changes-and-future-removal-from-percona-server-for-mysql-8-0/>`__.

*TokuDB* status variables provide details about the inner workings of *TokuDB*
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
   * - :ref:`Tokudb_DB_OPENS`
     - integer 
   * - :ref:`Tokudb_DB_CLOSES`
     - integer
   * - :ref:`Tokudb_DB_OPEN_CURRENT`
     - integer
   * - :ref:`Tokudb_DB_OPEN_MAX`
     - integer
   * - :ref:`Tokudb_LEAF_ENTRY_MAX_COMMITTED_XR`
     - integer
   * - :ref:`Tokudb_LEAF_ENTRY_MAX_PROVISIONAL_XR`
     - integer
   * - :ref:`Tokudb_LEAF_ENTRY_EXPANDED`
     - integer
   * - :ref:`Tokudb_LEAF_ENTRY_MAX_MEMSIZE`
     - integer
   * - :ref:`Tokudb_LEAF_ENTRY_APPLY_GC_BYTES_IN`
     - integer
   * - :ref:`Tokudb_LEAF_ENTRY_APPLY_GC_BYTES_OUT`
     - integer
   * - :ref:`Tokudb_LEAF_ENTRY_NORMAL_GC_BYTES_IN`
     - integer
   * - :ref:`Tokudb_LEAF_ENTRY_NORMAL_GC_BYTES_OUT`
     - integer
   * - :ref:`Tokudb_CHECKPOINT_PERIOD`
     - integer
   * - :ref:`Tokudb_CHECKPOINT_FOOTPRINT`
     - integer
   * - :ref:`Tokudb_CHECKPOINT_LAST_BEGAN`
     - datetime
   * - :ref:`Tokudb_CHECKPOINT_LAST_COMPLETE_BEGAN`
     - datetime
   * - :ref:`Tokudb_CHECKPOINT_LAST_COMPLETE_ENDED`
     - datetime
   * - :ref:`Tokudb_CHECKPOINT_DURATION`
     - integer
   * - :ref:`Tokudb_CHECKPOINT_DURATION_LAST`
     - integer
   * - :ref:`Tokudb_CHECKPOINT_LAST_LSN`
     - integer
   * - :ref:`Tokudb_CHECKPOINT_TAKEN`
     - integer
   * - :ref:`Tokudb_CHECKPOINT_FAILED`
     - integer
   * - :ref:`Tokudb_CHECKPOINT_WAITERS_NOW`
     - integer
   * - :ref:`Tokudb_CHECKPOINT_WAITERS_MAX`
     - integer
   * - :ref:`Tokudb_CHECKPOINT_CLIENT_WAIT_ON_MO`
     - integer
   * - :ref:`Tokudb_CHECKPOINT_CLIENT_WAIT_ON_CS`
     - integer
   * - :ref:`Tokudb_CHECKPOINT_BEGIN_TIME`
     - integer
   * - :ref:`Tokudb_CHECKPOINT_LONG_BEGIN_TIME`
     - integer
   * - :ref:`Tokudb_CHECKPOINT_LONG_BEGIN_COUNT`
     - integer
   * - :ref:`Tokudb_CHECKPOINT_END_TIME`
     - integer
   * - :ref:`Tokudb_CHECKPOINT_LONG_END_TIME`
     - integer
   * - :ref:`Tokudb_CHECKPOINT_LONG_END_COUNT`
     - integer
   * - :ref:`Tokudb_CACHETABLE_MISS`
     - integer
   * - :ref:`Tokudb_CACHETABLE_MISS_TIME`
     - integer
   * - :ref:`Tokudb_CACHETABLE_PREFETCHES`
     - integer
   * - :ref:`Tokudb_CACHETABLE_SIZE_CURRENT`
     - integer
   * - :ref:`Tokudb_CACHETABLE_SIZE_LIMIT`
     - integer
   * - :ref:`Tokudb_CACHETABLE_SIZE_WRITING`
     - integer
   * - :ref:`Tokudb_CACHETABLE_SIZE_NONLEAF`
     - integer
   * - :ref:`Tokudb_CACHETABLE_SIZE_LEAF`
     - integer
   * - :ref:`Tokudb_CACHETABLE_SIZE_ROLLBACK`
     - integer
   * - :ref:`Tokudb_CACHETABLE_SIZE_CACHEPRESSURE`
     - integer
   * - :ref:`Tokudb_CACHETABLE_SIZE_CLONED`
     - integer
   * - :ref:`Tokudb_CACHETABLE_EVICTIONS`
     - integer
   * - :ref:`Tokudb_CACHETABLE_CLEANER_EXECUTIONS`
     - integer
   * - :ref:`Tokudb_CACHETABLE_CLEANER_PERIOD`
     - integer
   * - :ref:`Tokudb_CACHETABLE_CLEANER_ITERATIONS`
     - integer
   * - :ref:`Tokudb_CACHETABLE_WAIT_PRESSURE_COUNT`
     - integer
   * - :ref:`Tokudb_CACHETABLE_WAIT_PRESSURE_TIME`
     - integer
   * - :ref:`Tokudb_CACHETABLE_LONG_WAIT_PRESSURE_COUNT`
     - integer
   * - :ref:`Tokudb_CACHETABLE_LONG_WAIT_PRESSURE_TIME`
     - integer
   * - :ref:`Tokudb_CACHETABLE_POOL_CLIENT_NUM_THREADS`
     - integer
   * - :ref:`Tokudb_CACHETABLE_POOL_CLIENT_NUM_THREADS_ACTIVE`
     - integer
   * - :ref:`Tokudb_CACHETABLE_POOL_CLIENT_QUEUE_SIZE`
     - integer
   * - :ref:`Tokudb_CACHETABLE_POOL_CLIENT_MAX_QUEUE_SIZE`
     - integer
   * - :ref:`Tokudb_CACHETABLE_POOL_CLIENT_TOTAL_ITEMS_PROCESSED`
     - integer
   * - :ref:`Tokudb_CACHETABLE_POOL_CLIENT_TOTAL_EXECUTION_TIME`
     - integer
   * - :ref:`Tokudb_CACHETABLE_POOL_CACHETABLE_NUM_THREADS`
     - integer
   * - :ref:`Tokudb_CACHETABLE_POOL_CACHETABLE_NUM_THREADS_ACTIVE`
     - integer
   * - :ref:`Tokudb_CACHETABLE_POOL_CACHETABLE_QUEUE_SIZE`
     - integer
   * - :ref:`Tokudb_CACHETABLE_POOL_CACHETABLE_MAX_QUEUE_SIZE`
     - integer
   * - :ref:`Tokudb_CACHETABLE_POOL_CACHETABLE_TOTAL_ITEMS_PROCESSED`
     - integer
   * - :ref:`Tokudb_CACHETABLE_POOL_CACHETABLE_TOTAL_EXECUTION_TIME`
     - integer
   * - :ref:`Tokudb_CACHETABLE_POOL_CHECKPOINT_NUM_THREADS`
     - integer
   * - :ref:`Tokudb_CACHETABLE_POOL_CHECKPOINT_NUM_THREADS_ACTIVE`
     - integer
   * - :ref:`Tokudb_CACHETABLE_POOL_CHECKPOINT_QUEUE_SIZE`
     - integer
   * - :ref:`Tokudb_CACHETABLE_POOL_CHECKPOINT_MAX_QUEUE_SIZE`
     - integer
   * - :ref:`Tokudb_CACHETABLE_POOL_CHECKPOINT_TOTAL_ITEMS_PROCESSED`
     - integer
   * - :ref:`Tokudb_CACHETABLE_POOL_CHECKPOINT_TOTAL_EXECUTION_TIME`
     - integer
   * - :ref:`Tokudb_LOCKTREE_MEMORY_SIZE`
     - integer
   * - :ref:`Tokudb_LOCKTREE_MEMORY_SIZE_LIMIT`
     - integer
   * - :ref:`Tokudb_LOCKTREE_ESCALATION_NUM`
     - integer
   * - :ref:`Tokudb_LOCKTREE_ESCALATION_SECONDS`
     - numeric
   * - :ref:`Tokudb_LOCKTREE_LATEST_POST_ESCALATION_MEMORY_SIZE`
     - integer
   * - :ref:`Tokudb_LOCKTREE_OPEN_CURRENT`
     - integer
   * - :ref:`Tokudb_LOCKTREE_PENDING_LOCK_REQUESTS`
     - integer
   * - :ref:`Tokudb_LOCKTREE_STO_ELIGIBLE_NUM`
     - integer
   * - :ref:`Tokudb_LOCKTREE_STO_ENDED_NUM`
     - integer
   * - :ref:`Tokudb_LOCKTREE_STO_ENDED_SECONDS`
     - numeric
   * - :ref:`Tokudb_LOCKTREE_WAIT_COUNT`
     - integer
   * - :ref:`Tokudb_LOCKTREE_WAIT_TIME`
     - integer
   * - :ref:`Tokudb_LOCKTREE_LONG_WAIT_COUNT`
     - integer
   * - :ref:`Tokudb_LOCKTREE_LONG_WAIT_TIME`
     - integer
   * - :ref:`Tokudb_LOCKTREE_TIMEOUT_COUNT`
     - integer
   * - :ref:`Tokudb_LOCKTREE_WAIT_ESCALATION_COUNT`
     - integer
   * - :ref:`Tokudb_LOCKTREE_WAIT_ESCALATION_TIME`
     - integer
   * - :ref:`Tokudb_LOCKTREE_LONG_WAIT_ESCALATION_COUNT`
     - integer
   * - :ref:`Tokudb_LOCKTREE_LONG_WAIT_ESCALATION_TIME`
     - integer
   * - :ref:`Tokudb_DICTIONARY_UPDATES`
     - integer
   * - :ref:`Tokudb_DICTIONARY_BROADCAST_UPDATES`
     - integer
   * - :ref:`Tokudb_DESCRIPTOR_SET`
     - integer
   * - :ref:`Tokudb_MESSAGES_IGNORED_BY_LEAF_DUE_TO_MSN`
     - integer
   * - :ref:`Tokudb_TOTAL_SEARCH_RETRIES`
     - integer
   * - :ref:`Tokudb_SEARCH_TRIES_GT_HEIGHT`
     - integer
   * - :ref:`Tokudb_SEARCH_TRIES_GT_HEIGHTPLUS3`
     - integer
   * - :ref:`Tokudb_LEAF_NODES_FLUSHED_NOT_CHECKPOINT`
     - integer
   * - :ref:`Tokudb_LEAF_NODES_FLUSHED_NOT_CHECKPOINT_BYTES`
     - integer
   * - :ref:`Tokudb_LEAF_NODES_FLUSHED_NOT_CHECKPOINT_UNCOMPRESSED_BYTES`
     - integer
   * - :ref:`Tokudb_LEAF_NODES_FLUSHED_NOT_CHECKPOINT_SECONDS`
     - numeric
   * - :ref:`Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_NOT_CHECKPOINT`
     - integer
   * - :ref:`Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_NOT_CHECKPOINT_BYTES`
     - integer
   * - :ref:`Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_NOT_CHECKPOINT_UNCOMPRESSE`
     - integer
   * - :ref:`Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_NOT_CHECKPOINT_SECONDS`
     - numeric
   * - :ref:`Tokudb_LEAF_NODES_FLUSHED_CHECKPOINT`
     - integer
   * - :ref:`Tokudb_LEAF_NODES_FLUSHED_CHECKPOINT_BYTES`
     - integer
   * - :ref:`Tokudb_LEAF_NODES_FLUSHED_CHECKPOINT_UNCOMPRESSED_BYTES`
     - integer
   * - :ref:`Tokudb_LEAF_NODES_FLUSHED_CHECKPOINT_SECONDS`
     - numeric
   * - :ref:`Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_CHECKPOINT`
     - integer
   * - :ref:`Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_CHECKPOINT_BYTES`
     - integer
   * - :ref:`Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_CHECKPOINT_UNCOMPRESSED_BY`
     - integer
   * - :ref:`Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_CHECKPOINT_SECONDS`
     - numeric
   * - :ref:`Tokudb_LEAF_NODE_COMPRESSION_RATIO`
     - numeric
   * - :ref:`Tokudb_NONLEAF_NODE_COMPRESSION_RATIO`
     - numeric
   * - :ref:`Tokudb_OVERALL_NODE_COMPRESSION_RATIO`
     - numeric
   * - :ref:`Tokudb_NONLEAF_NODE_PARTIAL_EVICTIONS`
     - numeric
   * - :ref:`Tokudb_NONLEAF_NODE_PARTIAL_EVICTIONS_BYTES`
     - integer
   * - :ref:`Tokudb_LEAF_NODE_PARTIAL_EVICTIONS`
     - integer
   * - :ref:`Tokudb_LEAF_NODE_PARTIAL_EVICTIONS_BYTES`
     - integer
   * - :ref:`Tokudb_LEAF_NODE_FULL_EVICTIONS`
     - integer
   * - :ref:`Tokudb_LEAF_NODE_FULL_EVICTIONS_BYTES`
     - integer
   * - :ref:`Tokudb_NONLEAF_NODE_FULL_EVICTIONS`
     - integer
   * - :ref:`Tokudb_NONLEAF_NODE_FULL_EVICTIONS_BYTES`
     - integer
   * - :ref:`Tokudb_LEAF_NODES_CREATED`
     - integer
   * - :ref:`Tokudb_NONLEAF_NODES_CREATED`
     - integer
   * - :ref:`Tokudb_LEAF_NODES_DESTROYED`
     - integer
   * - :ref:`Tokudb_NONLEAF_NODES_DESTROYED`
     - integer
   * - :ref:`Tokudb_MESSAGES_INJECTED_AT_ROOT_BYTES`
     - integer
   * - :ref:`Tokudb_MESSAGES_FLUSHED_FROM_H1_TO_LEAVES_BYTES`
     - integer
   * - :ref:`Tokudb_MESSAGES_IN_TREES_ESTIMATE_BYTES`
     - integer
   * - :ref:`Tokudb_MESSAGES_INJECTED_AT_ROOT`
     - integer
   * - :ref:`Tokudb_BROADCASE_MESSAGES_INJECTED_AT_ROOT`
     - integer
   * - :ref:`Tokudb_BASEMENTS_DECOMPRESSED_TARGET_QUERY`
     - integer
   * - :ref:`Tokudb_BASEMENTS_DECOMPRESSED_PRELOCKED_RANGE`
     - integer
   * - :ref:`Tokudb_BASEMENTS_DECOMPRESSED_PREFETCH`
     - integer
   * - :ref:`Tokudb_BASEMENTS_DECOMPRESSED_FOR_WRITE`
     - integer
   * - :ref:`Tokudb_BUFFERS_DECOMPRESSED_TARGET_QUERY`
     - integer
   * - :ref:`Tokudb_BUFFERS_DECOMPRESSED_PRELOCKED_RANGE`
     - integer
   * - :ref:`Tokudb_BUFFERS_DECOMPRESSED_PREFETCH`
     - integer
   * - :ref:`Tokudb_BUFFERS_DECOMPRESSED_FOR_WRITE`
     - integer
   * - :ref:`Tokudb_PIVOTS_FETCHED_FOR_QUERY`
     - integer
   * - :ref:`Tokudb_PIVOTS_FETCHED_FOR_QUERY_BYTES`
     - integer
   * - :ref:`Tokudb_PIVOTS_FETCHED_FOR_QUERY_SECONDS`
     - numeric
   * - :ref:`Tokudb_PIVOTS_FETCHED_FOR_PREFETCH`
     - integer
   * - :ref:`Tokudb_PIVOTS_FETCHED_FOR_PREFETCH_BYTES`
     - integer
   * - :ref:`Tokudb_PIVOTS_FETCHED_FOR_PREFETCH_SECONDS`
     - numeric
   * - :ref:`Tokudb_PIVOTS_FETCHED_FOR_WRITE`
     - integer
   * - :ref:`Tokudb_PIVOTS_FETCHED_FOR_WRITE_BYTES`
     - integer
   * - :ref:`Tokudb_PIVOTS_FETCHED_FOR_WRITE_SECONDS`
     - numeric
   * - :ref:`Tokudb_BASEMENTS_FETCHED_TARGET_QUERY`
     - integer
   * - :ref:`Tokudb_BASEMENTS_FETCHED_TARGET_QUERY_BYTES`
     - integer
   * - :ref:`Tokudb_BASEMENTS_FETCHED_TARGET_QUERY_SECONDS`
     - numeric
   * - :ref:`Tokudb_BASEMENTS_FETCHED_PRELOCKED_RANGE`
     - integer
   * - :ref:`Tokudb_BASEMENTS_FETCHED_PRELOCKED_RANGE_BYTES`
     - integer
   * - :ref:`Tokudb_BASEMENTS_FETCHED_PRELOCKED_RANGE_SECONDS`
     - numeric
   * - :ref:`Tokudb_BASEMENTS_FETCHED_PREFETCH`
     - integer
   * - :ref:`Tokudb_BASEMENTS_FETCHED_PREFETCH_BYTES`
     - integer
   * - :ref:`Tokudb_BASEMENTS_FETCHED_PREFETCH_SECONDS`
     - numeric
   * - :ref:`Tokudb_BASEMENTS_FETCHED_FOR_WRITE`
     - integer
   * - :ref:`Tokudb_BASEMENTS_FETCHED_FOR_WRITE_BYTES`
     - integer
   * - :ref:`Tokudb_BASEMENTS_FETCHED_FOR_WRITE_SECONDS`
     - numeric
   * - :ref:`Tokudb_BUFFERS_FETCHED_TARGET_QUERY`
     - integer
   * - :ref:`Tokudb_BUFFERS_FETCHED_TARGET_QUERY_BYTES`
     - integer
   * - :ref:`Tokudb_BUFFERS_FETCHED_TARGET_QUERY_SECONDS`
     - numeric
   * - :ref:`Tokudb_BUFFERS_FETCHED_PRELOCKED_RANGE`
     - integer
   * - :ref:`Tokudb_BUFFERS_FETCHED_PRELOCKED_RANGE_BYTES`
     - integer
   * - :ref:`Tokudb_BUFFERS_FETCHED_PRELOCKED_RANGE_SECONDS`
     - numeric
   * - :ref:`Tokudb_BUFFERS_FETCHED_PREFETCH`
     - integer
   * - :ref:`Tokudb_BUFFERS_FETCHED_PREFETCH_BYTES`
     - integer
   * - :ref:`Tokudb_BUFFERS_FETCHED_PREFETCH_SECONDS`
     - numeric
   * - :ref:`Tokudb_BUFFERS_FETCHED_FOR_WRITE`
     - integer
   * - :ref:`Tokudb_BUFFERS_FETCHED_FOR_WRITE_BYTES`
     - integer
   * - :ref:`Tokudb_BUFFERS_FETCHED_FOR_WRITE_SECONDS`
     - integer
   * - :ref:`Tokudb_LEAF_COMPRESSION_TO_MEMORY_SECONDS`
     - numeric
   * - :ref:`Tokudb_LEAF_SERIALIZATION_TO_MEMORY_SECONDS`
     - numeric
   * - :ref:`Tokudb_LEAF_DECOMPRESSION_TO_MEMORY_SECONDS`
     - numeric
   * - :ref:`Tokudb_LEAF_DESERIALIZATION_TO_MEMORY_SECONDS`
     - numeric
   * - :ref:`Tokudb_NONLEAF_COMPRESSION_TO_MEMORY_SECONDS`
     - numeric
   * - :ref:`Tokudb_NONLEAF_SERIALIZATION_TO_MEMORY_SECONDS`
     - numeric
   * - :ref:`Tokudb_NONLEAF_DECOMPRESSION_TO_MEMORY_SECONDS`
     - numeric
   * - :ref:`Tokudb_NONLEAF_DESERIALIZATION_TO_MEMORY_SECONDS`
     - numeric
   * - :ref:`Tokudb_PROMOTION_ROOTS_SPLIT`
     - integer
   * - :ref:`Tokudb_PROMOTION_LEAF_ROOTS_INJECTED_INTO`
     - integer
   * - :ref:`Tokudb_PROMOTION_H1_ROOTS_INJECTED_INTO`
     - integer
   * - :ref:`Tokudb_PROMOTION_INJECTIONS_AT_DEPTH_0`
     - integer
   * - :ref:`Tokudb_PROMOTION_INJECTIONS_AT_DEPTH_1`
     - integer
   * - :ref:`Tokudb_PROMOTION_INJECTIONS_AT_DEPTH_2`
     - integer
   * - :ref:`Tokudb_PROMOTION_INJECTIONS_AT_DEPTH_3`
     - integer
   * - :ref:`Tokudb_PROMOTION_INJECTIONS_LOWER_THAN_DEPTH_3`
     - integer
   * - :ref:`Tokudb_PROMOTION_STOPPED_NONEMPTY_BUFFER`
     - integer
   * - :ref:`Tokudb_PROMOTION_STOPPED_AT_HEIGHT_1`
     - integer
   * - :ref:`Tokudb_PROMOTION_STOPPED_CHILD_LOCKED_OR_NOT_IN_MEMORY`
     - integer
   * - :ref:`Tokudb_PROMOTION_STOPPED_CHILD_NOT_FULLY_IN_MEMORY`
     - integer
   * - :ref:`Tokudb_PROMOTION_STOPPED_AFTER_LOCKING_CHILD`
     - integer
   * - :ref:`Tokudb_BASEMENT_DESERIALIZATION_FIXED_KEY`
     - integer
   * - :ref:`Tokudb_BASEMENT_DESERIALIZATION_VARIABLE_KEY`
     - integer
   * - :ref:`Tokudb_PRO_RIGHTMOST_LEAF_SHORTCUT_SUCCESS`
     - integer
   * - :ref:`Tokudb_PRO_RIGHTMOST_LEAF_SHORTCUT_FAIL_POS`
     - integer
   * - :ref:`Tokudb_RIGHTMOST_LEAF_SHORTCUT_FAIL_REACTIVE`
     - integer
   * - :ref:`Tokudb_CURSOR_SKIP_DELETED_LEAF_ENTRY`
     - integer
   * - :ref:`Tokudb_FLUSHER_CLEANER_TOTAL_NODES`
     - integer
   * - :ref:`Tokudb_FLUSHER_CLEANER_H1_NODES`
     - integer
   * - :ref:`Tokudb_FLUSHER_CLEANER_HGT1_NODES`
     - integer
   * - :ref:`Tokudb_FLUSHER_CLEANER_EMPTY_NODES`
     - integer
   * - :ref:`Tokudb_FLUSHER_CLEANER_NODES_DIRTIED`
     - integer
   * - :ref:`Tokudb_FLUSHER_CLEANER_MAX_BUFFER_SIZE`
     - integer
   * - :ref:`Tokudb_FLUSHER_CLEANER_MIN_BUFFER_SIZE`
     - integer
   * - :ref:`Tokudb_FLUSHER_CLEANER_TOTAL_BUFFER_SIZE`
     - integer
   * - :ref:`Tokudb_FLUSHER_CLEANER_MAX_BUFFER_WORKDONE`
     - integer
   * - :ref:`Tokudb_FLUSHER_CLEANER_MIN_BUFFER_WORKDONE`
     - integer
   * - :ref:`Tokudb_FLUSHER_CLEANER_TOTAL_BUFFER_WORKDONE`
     - integer
   * - :ref:`Tokudb_FLUSHER_CLEANER_NUM_LEAF_MERGES_STARTED`
     - integer
   * - :ref:`Tokudb_FLUSHER_CLEANER_NUM_LEAF_MERGES_RUNNING`
     - integer
   * - :ref:`Tokudb_FLUSHER_CLEANER_NUM_LEAF_MERGES_COMPLETED`
     - integer
   * - :ref:`Tokudb_FLUSHER_CLEANER_NUM_DIRTIED_FOR_LEAF_MERGE`
     - integer
   * - :ref:`Tokudb_FLUSHER_FLUSH_TOTAL`
     - integer
   * - :ref:`Tokudb_FLUSHER_FLUSH_IN_MEMORY`
     - integer
   * - :ref:`Tokudb_FLUSHER_FLUSH_NEEDED_IO`
     - integer
   * - :ref:`Tokudb_FLUSHER_FLUSH_CASCADES`
     - integer
   * - :ref:`Tokudb_FLUSHER_FLUSH_CASCADES_1`
     - integer
   * - :ref:`Tokudb_FLUSHER_FLUSH_CASCADES_2`
     - integer
   * - :ref:`Tokudb_FLUSHER_FLUSH_CASCADES_3`
     - integer
   * - :ref:`Tokudb_FLUSHER_FLUSH_CASCADES_4`
     - integer
   * - :ref:`Tokudb_FLUSHER_FLUSH_CASCADES_5`
     - integer
   * - :ref:`Tokudb_FLUSHER_FLUSH_CASCADES_GT_5`
     - integer
   * - :ref:`Tokudb_FLUSHER_SPLIT_LEAF`
     - integer
   * - :ref:`Tokudb_FLUSHER_SPLIT_NONLEAF`
     - integer
   * - :ref:`Tokudb_FLUSHER_MERGE_LEAF`
     - integer
   * - :ref:`Tokudb_FLUSHER_MERGE_NONLEAF`
     - integer
   * - :ref:`Tokudb_FLUSHER_BALANCE_LEAF`
     - integer
   * - :ref:`Tokudb_HOT_NUM_STARTED`
     - integer
   * - :ref:`Tokudb_HOT_NUM_COMPLETED`
     - integer
   * - :ref:`Tokudb_HOT_NUM_ABORTED`
     - integer
   * - :ref:`Tokudb_HOT_MAX_ROOT_FLUSH_COUNT`
     - integer
   * - :ref:`Tokudb_TXN_BEGIN`
     - integer
   * - :ref:`Tokudb_TXN_BEGIN_READ_ONLY`
     - integer
   * - :ref:`Tokudb_TXN_COMMITS`
     - integer
   * - :ref:`Tokudb_TXN_ABORTS`
     - integer
   * - :ref:`Tokudb_LOGGER_NEXT_LSN`
     - integer
   * - :ref:`Tokudb_LOGGER_WRITES`
     - integer
   * - :ref:`Tokudb_LOGGER_WRITES_BYTES`
     - integer
   * - :ref:`Tokudb_LOGGER_WRITES_UNCOMPRESSED_BYTES`
     - integer
   * - :ref:`Tokudb_LOGGER_WRITES_SECONDS`
     - numeric
   * - :ref:`Tokudb_LOGGER_WAIT_LONG`
     - integer
   * - :ref:`Tokudb_LOADER_NUM_CREATED`
     - integer
   * - :ref:`Tokudb_LOADER_NUM_CURRENT`
     - integer
   * - :ref:`Tokudb_LOADER_NUM_MAX`
     - integer
   * - :ref:`Tokudb_MEMORY_MALLOC_COUNT`
     - integer
   * - :ref:`Tokudb_MEMORY_FREE_COUNT`
     - integer
   * - :ref:`Tokudb_MEMORY_REALLOC_COUNT`
     - integer
   * - :ref:`Tokudb_MEMORY_MALLOC_FAIL`
     - integer
   * - :ref:`Tokudb_MEMORY_REALLOC_FAIL`
     - integer
   * - :ref:`Tokudb_MEMORY_REQUESTED`
     - integer
   * - :ref:`Tokudb_MEMORY_USED`
     - integer
   * - :ref:`Tokudb_MEMORY_FREED`
     - integer
   * - :ref:`Tokudb_MEMORY_MAX_REQUESTED_SIZE`
     - integer
   * - :ref:`Tokudb_MEMORY_LAST_FAILED_SIZE`
     - integer
   * - :ref:`Tokudb_MEM_ESTIMATED_MAXIMUM_MEMORY_FOOTPRINT`
     - integer
   * - :ref:`Tokudb_MEMORY_MALLOCATOR_VERSION`
     - string
   * - :ref:`Tokudb_MEMORY_MMAP_THRESHOLD`
     - integer
   * - :ref:`Tokudb_FILESYSTEM_THREADS_BLOCKED_BY_FULL_DISK`
     - integer
   * - :ref:`Tokudb_FILESYSTEM_FSYNC_TIME`
     - integer
   * - :ref:`Tokudb_FILESYSTEM_FSYNC_NUM`
     - integer
   * - :ref:`Tokudb_FILESYSTEM_LONG_FSYNC_TIME`
     - integer
   * - :ref:`Tokudb_FILESYSTEM_LONG_FSYNC_NUM`
     - integer

.. _Tokudb_DB_OPENS:

.. rubric:: ``Tokudb_DB_OPENS``

This variable shows the number of times an individual PerconaFT dictionary file
was opened. This is a not a useful value for a regular user to use for any
purpose due to layers of open/close caching on top.

.. _Tokudb_DB_CLOSES:

.. rubric:: ``Tokudb_DB_CLOSES``

This variable shows the number of times an individual PerconaFT dictionary file
was closed. This is a not a useful value for a regular user to use for any
purpose due to layers of open/close caching on top.

.. _Tokudb_DB_OPEN_CURRENT:

.. rubric:: ``Tokudb_DB_OPEN_CURRENT``

This variable shows the number of currently opened databases.

.. _Tokudb_DB_OPEN_MAX:

.. rubric:: ``Tokudb_DB_OPEN_MAX``

This variable shows the maximum number of concurrently opened databases.

.. _Tokudb_LEAF_ENTRY_MAX_COMMITTED_XR:

.. rubric:: ``Tokudb_LEAF_ENTRY_MAX_COMMITTED_XR``

This variable shows the maximum number of committed transaction records that
were stored on disk in a new or modified row.

.. _Tokudb_LEAF_ENTRY_MAX_PROVISIONAL_XR:

.. rubric:: ``Tokudb_LEAF_ENTRY_MAX_PROVISIONAL_XR``

This variable shows the maximum number of provisional transaction records that
were stored on disk in a new or modified row.

.. _Tokudb_LEAF_ENTRY_EXPANDED:

.. rubric:: ``Tokudb_LEAF_ENTRY_EXPANDED``

This variable shows the number of times that an expanded memory mechanism was
used to store a new or modified row on disk.

.. _Tokudb_LEAF_ENTRY_MAX_MEMSIZE:

.. rubric:: ``Tokudb_LEAF_ENTRY_MAX_MEMSIZE``

This variable shows the maximum number of bytes that were stored on disk as a
new or modified row. This is the maximum uncompressed size of any row stored in
*TokuDB* that was created or modified since the server started.

.. _Tokudb_LEAF_ENTRY_APPLY_GC_BYTES_IN:

.. rubric:: ``Tokudb_LEAF_ENTRY_APPLY_GC_BYTES_IN``

This variable shows the total number of bytes of leaf nodes data before
performing garbage collection for non-flush events.

.. _Tokudb_LEAF_ENTRY_APPLY_GC_BYTES_OUT:

.. rubric:: ``Tokudb_LEAF_ENTRY_APPLY_GC_BYTES_OUT``

This variable shows the total number of bytes of leaf nodes data after
performing garbage collection for non-flush events.

.. _Tokudb_LEAF_ENTRY_NORMAL_GC_BYTES_IN:

.. rubric:: ``Tokudb_LEAF_ENTRY_NORMAL_GC_BYTES_IN``

This variable shows the total number of bytes of leaf nodes data before
performing garbage collection for flush events.

.. _Tokudb_LEAF_ENTRY_NORMAL_GC_BYTES_OUT:

.. rubric:: ``Tokudb_LEAF_ENTRY_NORMAL_GC_BYTES_OUT``

This variable shows the total number of bytes of leaf nodes data after
performing garbage collection for flush events.

.. _Tokudb_CHECKPOINT_PERIOD:

.. rubric:: ``Tokudb_CHECKPOINT_PERIOD``

This variable shows the interval in seconds between the end of an automatic
checkpoint and the beginning of the next automatic checkpoint.

.. _Tokudb_CHECKPOINT_FOOTPRINT:

.. rubric:: ``Tokudb_CHECKPOINT_FOOTPRINT``

This variable shows at what stage the checkpointer is at. It's used for
debugging purposes only and not a useful value for a normal user.

.. _Tokudb_CHECKPOINT_LAST_BEGAN:

.. rubric:: ``Tokudb_CHECKPOINT_LAST_BEGAN``

This variable shows the time the last checkpoint began. If a checkpoint is
currently in progress, then this time may be later than the time the last
checkpoint completed. If no checkpoint has ever taken place, then this value
will be ``Dec 31, 1969`` on Linux hosts.

.. _Tokudb_CHECKPOINT_LAST_COMPLETE_BEGAN:

.. rubric:: ``Tokudb_CHECKPOINT_LAST_COMPLETE_BEGAN``

This variable shows the time the last complete checkpoint started. Any data
that changed after this time will not be captured in the checkpoint.

.. _Tokudb_CHECKPOINT_LAST_COMPLETE_ENDED:

.. rubric:: ``Tokudb_CHECKPOINT_LAST_COMPLETE_ENDED``

This variable shows the time the last complete checkpoint ended.

.. _Tokudb_CHECKPOINT_DURATION:

.. rubric:: ``Tokudb_CHECKPOINT_DURATION``

This variable shows time (in seconds) required to complete all
checkpoints.

.. _Tokudb_CHECKPOINT_DURATION_LAST:

.. rubric:: ``Tokudb_CHECKPOINT_DURATION_LAST``

This variable shows time (in seconds) required to complete the last
checkpoint.

.. _Tokudb_CHECKPOINT_LAST_LSN:

.. rubric:: ``Tokudb_CHECKPOINT_LAST_LSN``

This variable shows the last successful checkpoint LSN. Each checkpoint from
the time the PerconaFT environment is created has a monotonically incrementing
LSN. This is not a useful value for a normal user to use for any purpose other
than having some idea of how many checkpoints have occurred since the system
was first created.

.. _Tokudb_CHECKPOINT_TAKEN:

.. rubric:: ``Tokudb_CHECKPOINT_TAKEN`` 

This variable shows the number of complete checkpoints that have been taken.

.. _Tokudb_CHECKPOINT_FAILED:

.. rubric:: ``Tokudb_CHECKPOINT_FAILED`` 

This variable shows the number of checkpoints that have failed for any reason.

.. _Tokudb_CHECKPOINT_WAITERS_NOW:

.. rubric:: ``Tokudb_CHECKPOINT_WAITERS_NOW`` 

This variable shows the current number of threads waiting for the ``checkpoint
safe`` lock. This is a not a useful value for a regular user to use for any
purpose.

.. _Tokudb_CHECKPOINT_WAITERS_MAX:

.. rubric:: ``Tokudb_CHECKPOINT_WAITERS_MAX`` 

This variable shows the maximum number of threads that concurrently waited for
the ``checkpoint safe`` lock. This is a not a useful value for a regular user to
use for any purpose.

.. _Tokudb_CHECKPOINT_CLIENT_WAIT_ON_MO:

.. rubric:: ``Tokudb_CHECKPOINT_CLIENT_WAIT_ON_MO`` 

This variable shows the number of times a non-checkpoint client thread waited
for the multi-operation lock. It is an internal ``rwlock`` that is similar in
nature to the *InnoDB* kernel mutex, it effectively halts all access to the
PerconaFT API when write locked. The ``begin`` phase of the checkpoint takes
this lock for a brief period.

.. _Tokudb_CHECKPOINT_CLIENT_WAIT_ON_CS:

.. rubric:: ``Tokudb_CHECKPOINT_CLIENT_WAIT_ON_CS`` 

This variable shows the number of times a non-checkpoint client thread waited
for the checkpoint-safe lock. This is the lock taken when you ``SET
tokudb_checkpoint_lock=1``. If a client trying to lock/postpone the
checkpointer has to wait for the currently running checkpoint to complete, that
wait time will be reflected here and summed. This is not a useful metric as
regular users should never be manipulating the checkpoint lock.

.. _Tokudb_CHECKPOINT_BEGIN_TIME:

.. rubric:: ``Tokudb_CHECKPOINT_BEGIN_TIME`` 

This variable shows the cumulative time (in microseconds) required to mark all
dirty nodes as pending a checkpoint.

.. _Tokudb_CHECKPOINT_LONG_BEGIN_TIME:

.. rubric:: ``Tokudb_CHECKPOINT_LONG_BEGIN_TIME`` 

This variable shows the cumulative actual time (in microseconds) of checkpoint
``begin`` stages that took longer than 1 second.

.. _Tokudb_CHECKPOINT_LONG_BEGIN_COUNT:

.. rubric:: ``Tokudb_CHECKPOINT_LONG_BEGIN_COUNT``

This variable shows the number of checkpoints whose ``begin`` stage took longer
than 1 second.

.. _Tokudb_CHECKPOINT_END_TIME:

.. rubric:: ``Tokudb_CHECKPOINT_END_TIME`` 

This variable shows the time spent in checkpoint end operation in seconds.

.. _Tokudb_CHECKPOINT_LONG_END_TIME:

.. rubric:: ``Tokudb_CHECKPOINT_LONG_END_TIME`` 

This variable shows the total time of long checkpoints in seconds.

.. _Tokudb_CHECKPOINT_LONG_END_COUNT:

.. rubric:: ``Tokudb_CHECKPOINT_LONG_END_COUNT`` 

This variable shows the number of checkpoints whose ``end_checkpoint``
operations exceeded 1 minute.

.. _Tokudb_CACHETABLE_MISS:

.. rubric:: ``Tokudb_CACHETABLE_MISS`` 

This variable shows the number of times the application was unable to access
the data in the internal cache. A cache miss means that date will need to be
read from disk.

.. _Tokudb_CACHETABLE_MISS_TIME:

.. rubric:: ``Tokudb_CACHETABLE_MISS_TIME``  

This variable shows the total time, in microseconds, of how long the database
has had to wait for a disk read to complete.

.. _Tokudb_CACHETABLE_PREFETCHES:

.. rubric:: ``Tokudb_CACHETABLE_PREFETCHES``  

This variable shows the total number of times that a block of memory has been
prefetched into the database's cache. Data is prefetched when the database's
algorithms determine that a block of memory is likely to be accessed by the
application.

.. _Tokudb_CACHETABLE_SIZE_CURRENT:

.. rubric:: ``Tokudb_CACHETABLE_SIZE_CURRENT``  

This variable shows how much of the uncompressed data, in bytes, is
currently in the database's internal cache.

.. _Tokudb_CACHETABLE_SIZE_LIMIT:

.. rubric:: ``Tokudb_CACHETABLE_SIZE_LIMIT``  

This variable shows how much of the uncompressed data, in bytes, will fit in
the database's internal cache.

.. _Tokudb_CACHETABLE_SIZE_WRITING:

.. rubric:: ``Tokudb_CACHETABLE_SIZE_WRITING``  

This variable shows the number of bytes that are currently queued up to be
written to disk.

.. _Tokudb_CACHETABLE_SIZE_NONLEAF:

.. rubric:: ``Tokudb_CACHETABLE_SIZE_NONLEAF``  

This variable shows the amount of memory, in bytes, the current set of non-leaf
nodes occupy in the cache.

.. _Tokudb_CACHETABLE_SIZE_LEAF:

.. rubric:: ``Tokudb_CACHETABLE_SIZE_LEAF`` 
 
This variable shows the amount of memory, in bytes, the current set of
(decompressed) leaf nodes occupy in the cache.

.. _Tokudb_CACHETABLE_SIZE_ROLLBACK:

.. rubric:: ``Tokudb_CACHETABLE_SIZE_ROLLBACK``  

This variable shows the rollback nodes size, in bytes, in the cache.

.. _Tokudb_CACHETABLE_SIZE_CACHEPRESSURE:

.. rubric:: ``Tokudb_CACHETABLE_SIZE_CACHEPRESSURE``  

This variable shows the number of bytes causing cache pressure (the sum of
buffers and work done counters), helps to understand if cleaner threads are
keeping up with workload. It should really be looked at as more of a value to
use in a ratio of cache pressure / cache table size. The closer that ratio
evaluates to 1, the higher the cache pressure.

.. _Tokudb_CACHETABLE_SIZE_CLONED:

.. rubric:: ``Tokudb_CACHETABLE_SIZE_CLONED`` 

This variable shows the amount of memory, in bytes, currently used for cloned
nodes. During the checkpoint operation, dirty nodes are cloned prior to
serialization/compression, then written to disk. After which, the memory for
the cloned block is returned for re-use.

.. _Tokudb_CACHETABLE_EVICTIONS:

.. rubric:: ``Tokudb_CACHETABLE_EVICTIONS`` 

This variable shows the number of blocks evicted from cache. On its own this is
not a useful number as its impact on performance depends entirely on the
hardware and workload in use. For example, two workloads, one random, one
linear for the same starting data set will have two wildly different eviction
patterns.

.. _Tokudb_CACHETABLE_CLEANER_EXECUTIONS:

.. rubric:: ``Tokudb_CACHETABLE_CLEANER_EXECUTIONS`` 

This variable shows the total number of times the cleaner thread loop has
executed.

.. _Tokudb_CACHETABLE_CLEANER_PERIOD:

.. rubric:: ``Tokudb_CACHETABLE_CLEANER_PERIOD`` 

*TokuDB* includes a cleaner thread that optimizes indexes in the background.
This variable is the time, in seconds, between the completion of a group of
cleaner operations and the beginning of the next group of cleaner operations.
The cleaner operations run on a background thread performing work that does not
need to be done on the client thread.

.. _Tokudb_CACHETABLE_CLEANER_ITERATIONS:

.. rubric:: ``Tokudb_CACHETABLE_CLEANER_ITERATIONS`` 

This variable shows the number of cleaner operations that are performed every
cleaner period.

.. _Tokudb_CACHETABLE_WAIT_PRESSURE_COUNT:

.. rubric:: ``Tokudb_CACHETABLE_WAIT_PRESSURE_COUNT`` 

This variable shows the number of times a thread was stalled due to cache
pressure. 

.. _Tokudb_CACHETABLE_WAIT_PRESSURE_TIME:

.. rubric:: ``Tokudb_CACHETABLE_WAIT_PRESSURE_TIME`` 

This variable shows the total time, in microseconds, waiting on cache pressure
to subside.

.. _Tokudb_CACHETABLE_LONG_WAIT_PRESSURE_COUNT:

.. rubric:: ``Tokudb_CACHETABLE_LONG_WAIT_PRESSURE_COUNT`` 

This variable shows the number of times a thread was stalled for more than one
second due to cache pressure.

.. _Tokudb_CACHETABLE_LONG_WAIT_PRESSURE_TIME:

.. rubric:: ``Tokudb_CACHETABLE_LONG_WAIT_PRESSURE_TIME`` 

This variable shows the total time, in microseconds, waiting on cache pressure
to subside for more than one second.

.. _Tokudb_CACHETABLE_POOL_CLIENT_NUM_THREADS:

.. rubric:: ``Tokudb_CACHETABLE_POOL_CLIENT_NUM_THREADS`` 

This variable shows the number of threads in the client thread pool.

.. _Tokudb_CACHETABLE_POOL_CLIENT_NUM_THREADS_ACTIVE:

.. rubric:: ``Tokudb_CACHETABLE_POOL_CLIENT_NUM_THREADS_ACTIVE`` 

This variable shows the number of currently active threads in the client
thread pool.

.. _Tokudb_CACHETABLE_POOL_CLIENT_QUEUE_SIZE:

.. rubric:: ``Tokudb_CACHETABLE_POOL_CLIENT_QUEUE_SIZE`` 

This variable shows the number of currently queued work items in the client
thread pool.

.. _Tokudb_CACHETABLE_POOL_CLIENT_MAX_QUEUE_SIZE:

.. rubric:: ``Tokudb_CACHETABLE_POOL_CLIENT_MAX_QUEUE_SIZE`` 

This variable shows the largest number of queued work items in the client
thread pool.

.. _Tokudb_CACHETABLE_POOL_CLIENT_TOTAL_ITEMS_PROCESSED:

.. rubric:: ``Tokudb_CACHETABLE_POOL_CLIENT_TOTAL_ITEMS_PROCESSED`` 

This variable shows the total number of work items processed in the client
thread pool.

.. _Tokudb_CACHETABLE_POOL_CLIENT_TOTAL_EXECUTION_TIME:

.. rubric:: ``Tokudb_CACHETABLE_POOL_CLIENT_TOTAL_EXECUTION_TIME`` 

This variable shows the total execution time of processing work items in the
client thread pool.

.. _Tokudb_CACHETABLE_POOL_CACHETABLE_NUM_THREADS:

.. rubric:: ``Tokudb_CACHETABLE_POOL_CACHETABLE_NUM_THREADS`` 

This variable shows the number of threads in the cachetable threadpool.

.. _Tokudb_CACHETABLE_POOL_CACHETABLE_NUM_THREADS_ACTIVE:

.. rubric:: ``Tokudb_CACHETABLE_POOL_CACHETABLE_NUM_THREADS_ACTIVE`` 

This variable shows the number of currently active threads in the cachetable
thread pool.

.. _Tokudb_CACHETABLE_POOL_CACHETABLE_QUEUE_SIZE:

.. rubric:: ``Tokudb_CACHETABLE_POOL_CACHETABLE_QUEUE_SIZE`` 

This variable shows the number of currently queued work items in the cachetable
thread pool. 

.. _Tokudb_CACHETABLE_POOL_CACHETABLE_MAX_QUEUE_SIZE:

.. rubric:: ``Tokudb_CACHETABLE_POOL_CACHETABLE_MAX_QUEUE_SIZE`` 

This variable shows the largest number of queued work items in the cachetable
thread pool.

.. _Tokudb_CACHETABLE_POOL_CACHETABLE_TOTAL_ITEMS_PROCESSED:

.. rubric:: ``Tokudb_CACHETABLE_POOL_CACHETABLE_TOTAL_ITEMS_PROCESSED`` 

This variable shows the total number of work items processed in the cachetable
thread pool.

.. _Tokudb_CACHETABLE_POOL_CACHETABLE_TOTAL_EXECUTION_TIME:

.. rubric:: ``Tokudb_CACHETABLE_POOL_CACHETABLE_TOTAL_EXECUTION_TIME`` 

This variable shows the total execution time of processing work items in the
cachetable thread pool.

.. _Tokudb_CACHETABLE_POOL_CHECKPOINT_NUM_THREADS:

.. rubric:: ``Tokudb_CACHETABLE_POOL_CHECKPOINT_NUM_THREADS`` 

This variable shows the number of threads in the checkpoint threadpool.

.. _Tokudb_CACHETABLE_POOL_CHECKPOINT_NUM_THREADS_ACTIVE:

.. rubric:: ``Tokudb_CACHETABLE_POOL_CHECKPOINT_NUM_THREADS_ACTIVE`` 

This variable shows the number of currently active threads in the checkpoint
thread pool.

.. _Tokudb_CACHETABLE_POOL_CHECKPOINT_QUEUE_SIZE:

.. rubric:: ``Tokudb_CACHETABLE_POOL_CHECKPOINT_QUEUE_SIZE`` 

This variable shows the number of currently queued work items in the checkpoint
thread pool. 

.. _Tokudb_CACHETABLE_POOL_CHECKPOINT_MAX_QUEUE_SIZE:

.. rubric:: ``Tokudb_CACHETABLE_POOL_CHECKPOINT_MAX_QUEUE_SIZE`` 

This variable shows the largest number of queued work items in the checkpoint
thread pool.

.. _Tokudb_CACHETABLE_POOL_CHECKPOINT_TOTAL_ITEMS_PROCESSED:

.. rubric:: ``Tokudb_CACHETABLE_POOL_CHECKPOINT_TOTAL_ITEMS_PROCESSED`` 

This variable shows the total number of work items processed in the checkpoint
thread pool.

.. _Tokudb_CACHETABLE_POOL_CHECKPOINT_TOTAL_EXECUTION_TIME:

.. rubric:: ``Tokudb_CACHETABLE_POOL_CHECKPOINT_TOTAL_EXECUTION_TIME`` 

This variable shows the total execution time of processing work items in the
checkpoint thread pool.

.. _Tokudb_LOCKTREE_MEMORY_SIZE:

.. rubric:: ``Tokudb_LOCKTREE_MEMORY_SIZE`` 

This variable shows the amount of memory, in bytes, that the locktree is
currently using.

.. _Tokudb_LOCKTREE_MEMORY_SIZE_LIMIT:

.. rubric:: ``Tokudb_LOCKTREE_MEMORY_SIZE_LIMIT`` 

This variable shows the maximum amount of memory, in bytes, that the locktree
is allowed to use.

.. _Tokudb_LOCKTREE_ESCALATION_NUM:

.. rubric:: ``Tokudb_LOCKTREE_ESCALATION_NUM`` 

This variable shows the number of times the locktree needed to run lock
escalation to reduce its memory footprint.

.. _Tokudb_LOCKTREE_ESCALATION_SECONDS:

.. rubric:: ``Tokudb_LOCKTREE_ESCALATION_SECONDS`` 

This variable shows the total number of seconds spent performing locktree
escalation.

.. _Tokudb_LOCKTREE_LATEST_POST_ESCALATION_MEMORY_SIZE:

.. rubric:: ``Tokudb_LOCKTREE_LATEST_POST_ESCALATION_MEMORY_SIZE`` 

This variable shows the locktree size, in bytes, after most current locktree
escalation.

.. _Tokudb_LOCKTREE_OPEN_CURRENT:

.. rubric:: ``Tokudb_LOCKTREE_OPEN_CURRENT`` 

This variable shows the number of locktrees that are currently opened.

.. _Tokudb_LOCKTREE_PENDING_LOCK_REQUESTS:

.. rubric:: ``Tokudb_LOCKTREE_PENDING_LOCK_REQUESTS`` 

This variable shows the number of requests waiting for a lock grant.

.. _Tokudb_LOCKTREE_STO_ELIGIBLE_NUM:

.. rubric:: ``Tokudb_LOCKTREE_STO_ELIGIBLE_NUM`` 

This variable shows the number of locktrees eligible for ``Single Transaction
optimizations``. STO optimization are behaviors that can happen within the
locktree when there is exactly one transaction active within the locktree. This
is a not a useful value for a regular user to use for any purpose.

.. _Tokudb_LOCKTREE_STO_ENDED_NUM:

.. rubric:: ``Tokudb_LOCKTREE_STO_ENDED_NUM`` 

This variable shows the total number of times a ``Single Transaction
Optimization`` was ended early due to another transaction starting. STO
optimization are behaviors that can happen within the locktree when there is
exactly one transaction active within the locktree. This is a not a useful
value for a regular user to use for any purpose.

.. _Tokudb_LOCKTREE_STO_ENDED_SECONDS:

.. rubric:: ``Tokudb_LOCKTREE_STO_ENDED_SECONDS`` 

This variable shows the total number of seconds ending the ``Single
Transaction Optimizations``. STO optimization are behaviors that can happen
within the locktree when there is exactly one transaction active within the
locktree. This is a not a useful value for a regular user to use for any
purpose.

.. _Tokudb_LOCKTREE_WAIT_COUNT:

.. rubric:: ``Tokudb_LOCKTREE_WAIT_COUNT`` 

This variable shows the number of times that a lock request could not be
acquired because of a conflict with some other transaction. PerconaFT lock
request  cycles to try to obtain a lock, if it can not get a lock, it
sleeps/waits and times out, checks to get the lock again, repeat. This value
indicates the number of cycles it needed to execute before it obtained the
lock. 

.. _Tokudb_LOCKTREE_WAIT_TIME:

.. rubric:: ``Tokudb_LOCKTREE_WAIT_TIME`` 

This variable shows the total time, in microseconds, spent by client waiting
for a lock conflict to be resolved.

.. _Tokudb_LOCKTREE_LONG_WAIT_COUNT:

.. rubric:: ``Tokudb_LOCKTREE_LONG_WAIT_COUNT`` 

This variable shows number of lock waits greater than one second in duration.

.. _Tokudb_LOCKTREE_LONG_WAIT_TIME:

.. rubric:: ``Tokudb_LOCKTREE_LONG_WAIT_TIME`` 

This variable shows the total time, in microseconds, of the long waits.

.. _Tokudb_LOCKTREE_TIMEOUT_COUNT:

.. rubric:: ``Tokudb_LOCKTREE_TIMEOUT_COUNT`` 

This variable shows the number of times that a lock request timed out.

.. _Tokudb_LOCKTREE_WAIT_ESCALATION_COUNT:

.. rubric:: ``Tokudb_LOCKTREE_WAIT_ESCALATION_COUNT`` 

When the sum of the sizes of locks taken reaches the lock tree limit, we run
lock escalation on a background thread. The clients threads need to wait for
escalation to consolidate locks and free up memory. This variables shows the
number of times a client thread had to wait on lock escalation.

.. _Tokudb_LOCKTREE_WAIT_ESCALATION_TIME:

.. rubric:: ``Tokudb_LOCKTREE_WAIT_ESCALATION_TIME`` 

This variable shows the total time, in microseconds, that a client thread spent
waiting for lock escalation to free up memory.

.. _Tokudb_LOCKTREE_LONG_WAIT_ESCALATION_COUNT:

.. rubric:: ``Tokudb_LOCKTREE_LONG_WAIT_ESCALATION_COUNT`` 

This variable shows number of times that a client thread had to wait on lock
escalation and the wait time was greater than one second.

.. _Tokudb_LOCKTREE_LONG_WAIT_ESCALATION_TIME:
.. rubric:: ``Tokudb_LOCKTREE_LONG_WAIT_ESCALATION_TIME`` 

This variable shows the total time, in microseconds, of the long waits for lock
escalation to free up memory.

.. _Tokudb_DICTIONARY_UPDATES:

.. rubric:: ``Tokudb_DICTIONARY_UPDATES`` 

This variable shows the total number of rows that have been updated in all
primary and secondary indexes combined, if those updates have been done with a
separate recovery log entry per index.

.. _Tokudb_DICTIONARY_BROADCAST_UPDATES:

.. rubric:: ``Tokudb_DICTIONARY_BROADCAST_UPDATES`` 

This variable shows the number of broadcast updates that have been successfully
performed. A broadcast update is an update that affects all rows in a
dictionary.

.. _Tokudb_DESCRIPTOR_SET:

.. rubric:: ``Tokudb_DESCRIPTOR_SET`` 

This variable shows the number of time a descriptor was updated when the entire
dictionary was updated (for example, when the schema has been changed).

.. _Tokudb_MESSAGES_IGNORED_BY_LEAF_DUE_TO_MSN:

.. rubric:: ``Tokudb_MESSAGES_IGNORED_BY_LEAF_DUE_TO_MSN`` 

This variable shows the number of messages that were ignored by a leaf because
it had already been applied.

.. _Tokudb_TOTAL_SEARCH_RETRIES:

.. rubric:: ``Tokudb_TOTAL_SEARCH_RETRIES`` 

Internal value that is no use to anyone other than a developer debugging a
specific query/search issue.

.. _Tokudb_SEARCH_TRIES_GT_HEIGHT:

.. rubric:: ``Tokudb_SEARCH_TRIES_GT_HEIGHT`` 

Internal value that is no use to anyone other than a developer debugging a
specific query/search issue.

.. _Tokudb_SEARCH_TRIES_GT_HEIGHTPLUS3:

.. rubric:: ``Tokudb_SEARCH_TRIES_GT_HEIGHTPLUS3`` 

Internal value that is no use to anyone other than a developer debugging a
specific query/search issue.

.. _Tokudb_LEAF_NODES_FLUSHED_NOT_CHECKPOINT:

.. rubric:: ``Tokudb_LEAF_NODES_FLUSHED_NOT_CHECKPOINT`` 

This variable shows the number of leaf nodes flushed to disk, not for
checkpoint.

.. _Tokudb_LEAF_NODES_FLUSHED_NOT_CHECKPOINT_BYTES:

.. rubric:: ``Tokudb_LEAF_NODES_FLUSHED_NOT_CHECKPOINT_BYTES`` 

This variable shows the size, in bytes, of leaf nodes flushed to disk, not
for checkpoint.

.. _Tokudb_LEAF_NODES_FLUSHED_NOT_CHECKPOINT_UNCOMPRESSED_BYTES:

.. rubric:: ``Tokudb_LEAF_NODES_FLUSHED_NOT_CHECKPOINT_UNCOMPRESSED_BYTES`` 

This variable shows the size, in bytes, of uncompressed leaf nodes flushed to
disk not for checkpoint.

.. _Tokudb_LEAF_NODES_FLUSHED_NOT_CHECKPOINT_SECONDS:

.. rubric:: ``Tokudb_LEAF_NODES_FLUSHED_NOT_CHECKPOINT_SECONDS`` 

This variable shows the number of seconds waiting for I/O when writing leaf
nodes flushed to disk, not for checkpoint

.. _Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_NOT_CHECKPOINT:

.. rubric:: ``Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_NOT_CHECKPOINT`` 

This variable shows the number of non-leaf nodes flushed to disk, not for
checkpoint.

.. _Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_NOT_CHECKPOINT_BYTES:

.. rubric:: ``Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_NOT_CHECKPOINT_BYTES`` 

This variable shows the size, in bytes, of non-leaf nodes flushed to disk, not
for checkpoint.

.. _Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_NOT_CHECKPOINT_UNCOMPRESSE:

.. rubric:: ``Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_NOT_CHECKPOINT_UNCOMPRESSE`` 

This variable shows the size, in bytes, of uncompressed non-leaf nodes flushed
to disk not for checkpoint.

.. _Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_NOT_CHECKPOINT_SECONDS:

.. rubric:: ``Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_NOT_CHECKPOINT_SECONDS`` 

This variable shows the number of seconds waiting for I/O when writing non-leaf
nodes flushed to disk, not for checkpoint

.. _Tokudb_LEAF_NODES_FLUSHED_CHECKPOINT:

.. rubric:: ``Tokudb_LEAF_NODES_FLUSHED_CHECKPOINT`` 

This variable shows the number of leaf nodes flushed to disk, for checkpoint.

.. _Tokudb_LEAF_NODES_FLUSHED_CHECKPOINT_BYTES:

.. rubric:: ``Tokudb_LEAF_NODES_FLUSHED_CHECKPOINT_BYTES`` 

This variable shows the size, in bytes, of leaf nodes flushed to disk, for
checkpoint.

.. _Tokudb_LEAF_NODES_FLUSHED_CHECKPOINT_UNCOMPRESSED_BYTES:

.. rubric:: ``Tokudb_LEAF_NODES_FLUSHED_CHECKPOINT_UNCOMPRESSED_BYTES`` 

This variable shows the size, in bytes, of uncompressed leaf nodes flushed to
disk for checkpoint.

.. _Tokudb_LEAF_NODES_FLUSHED_CHECKPOINT_SECONDS:

.. rubric:: ``Tokudb_LEAF_NODES_FLUSHED_CHECKPOINT_SECONDS`` 

This variable shows the number of seconds waiting for I/O when writing leaf
nodes flushed to disk for checkpoint

.. _Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_CHECKPOINT:

.. rubric:: ``Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_CHECKPOINT`` 

This variable shows the number of non-leaf nodes flushed to disk, for
checkpoint.

.. _Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_CHECKPOINT_BYTES:

.. rubric:: ``Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_CHECKPOINT_BYTES`` 

This variable shows the size, in bytes, of non-leaf nodes flushed to disk, for
checkpoint.

.. _Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_CHECKPOINT_UNCOMPRESSED_BY:

.. rubric:: ``Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_CHECKPOINT_UNCOMPRESSED_BY`` 

This variable shows the size, in bytes, of uncompressed non-leaf nodes flushed
to disk for checkpoint.

.. _Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_CHECKPOINT_SECONDS:

.. rubric:: ``Tokudb_NONLEAF_NODES_FLUSHED_TO_DISK_CHECKPOINT_SECONDS`` 

This variable shows the number of seconds waiting for I/O when writing non-leaf
nodes flushed to disk for checkpoint

.. _Tokudb_LEAF_NODE_COMPRESSION_RATIO:

.. rubric:: ``Tokudb_LEAF_NODE_COMPRESSION_RATIO`` 

This variable shows the ratio of uncompressed bytes (in-memory) to compressed
bytes (on-disk) for leaf nodes.

.. _Tokudb_NONLEAF_NODE_COMPRESSION_RATIO:

.. rubric:: ``Tokudb_NONLEAF_NODE_COMPRESSION_RATIO`` 

This variable shows the ratio of uncompressed bytes (in-memory) to compressed
bytes (on-disk) for non-leaf nodes.

.. _Tokudb_OVERALL_NODE_COMPRESSION_RATIO:

.. rubric:: ``Tokudb_OVERALL_NODE_COMPRESSION_RATIO`` 

This variable shows the ratio of uncompressed bytes (in-memory) to compressed
bytes (on-disk) for all nodes.

.. _Tokudb_NONLEAF_NODE_PARTIAL_EVICTIONS:

.. rubric:: ``Tokudb_NONLEAF_NODE_PARTIAL_EVICTIONS`` 

This variable shows the number of times a partition of a non-leaf node was
evicted from the cache.

.. _Tokudb_NONLEAF_NODE_PARTIAL_EVICTIONS_BYTES:

.. rubric:: ``Tokudb_NONLEAF_NODE_PARTIAL_EVICTIONS_BYTES`` 

This variable shows the amount, in bytes, of memory freed by evicting
partitions of non-leaf nodes from the cache.

.. _Tokudb_LEAF_NODE_PARTIAL_EVICTIONS:

.. rubric:: ``Tokudb_LEAF_NODE_PARTIAL_EVICTIONS`` 

This variable shows the number of times a partition of a leaf node was evicted
from the cache.

.. _Tokudb_LEAF_NODE_PARTIAL_EVICTIONS_BYTES:

.. rubric:: ``Tokudb_LEAF_NODE_PARTIAL_EVICTIONS_BYTES`` 

This variable shows the amount, in bytes, of memory freed by evicting
partitions of leaf nodes from the cache.

.. _Tokudb_LEAF_NODE_FULL_EVICTIONS:

.. rubric:: ``Tokudb_LEAF_NODE_FULL_EVICTIONS`` 

This variable shows the number of times a full leaf node was evicted from the
cache.

.. _Tokudb_LEAF_NODE_FULL_EVICTIONS_BYTES:

.. rubric:: ``Tokudb_LEAF_NODE_FULL_EVICTIONS_BYTES`` 

This variable shows the amount, in bytes, of memory freed by evicting full leaf
nodes from the cache.

.. _Tokudb_NONLEAF_NODE_FULL_EVICTIONS:

.. rubric:: ``Tokudb_NONLEAF_NODE_FULL_EVICTIONS`` 

This variable shows the number of times a full non-leaf node was evicted from
the cache.

.. _Tokudb_NONLEAF_NODE_FULL_EVICTIONS_BYTES:

.. rubric:: ``Tokudb_NONLEAF_NODE_FULL_EVICTIONS_BYTES`` 

This variable shows the amount, in bytes, of memory freed by evicting full
non-leaf nodes from the cache.

.. _Tokudb_LEAF_NODES_CREATED:

.. rubric:: ``Tokudb_LEAF_NODES_CREATED`` 

This variable shows the number of created leaf nodes.

.. _Tokudb_NONLEAF_NODES_CREATED:

.. rubric:: ``Tokudb_NONLEAF_NODES_CREATED`` 

This variable shows the number of created non-leaf nodes.

.. _Tokudb_LEAF_NODES_DESTROYED:

.. rubric:: ``Tokudb_LEAF_NODES_DESTROYED`` 

This variable shows the number of destroyed leaf nodes.

.. _Tokudb_NONLEAF_NODES_DESTROYED:

.. rubric:: ``Tokudb_NONLEAF_NODES_DESTROYED`` 

This variable shows the number of destroyed non-leaf nodes.

.. _Tokudb_MESSAGES_INJECTED_AT_ROOT_BYTES:

.. rubric:: ``Tokudb_MESSAGES_INJECTED_AT_ROOT_BYTES`` 

This variable shows the size, in bytes, of messages injected at root (for all
trees).

.. _Tokudb_MESSAGES_FLUSHED_FROM_H1_TO_LEAVES_BYTES:

.. rubric:: ``Tokudb_MESSAGES_FLUSHED_FROM_H1_TO_LEAVES_BYTES`` 

This variable shows the size, in bytes, of messages flushed from ``h1`` nodes
to leaves.

.. _Tokudb_MESSAGES_IN_TREES_ESTIMATE_BYTES:

.. rubric:: ``Tokudb_MESSAGES_IN_TREES_ESTIMATE_BYTES`` 

This variable shows the estimated size, in bytes, of messages currently in
trees. 

.. _Tokudb_MESSAGES_INJECTED_AT_ROOT:

.. rubric:: ``Tokudb_MESSAGES_INJECTED_AT_ROOT`` 

This variables shows the number of messages that were injected at root node of
a tree.

.. _Tokudb_BROADCASE_MESSAGES_INJECTED_AT_ROOT:

.. rubric:: ``Tokudb_BROADCASE_MESSAGES_INJECTED_AT_ROOT`` 

This variable shows the number of broadcast messages dropped into the root node
of a tree. These are things such as the result of ``OPTIMIZE TABLE`` and a few
other operations. This is not a useful metric for a regular user to use for any
purpose.

.. _Tokudb_BASEMENTS_DECOMPRESSED_TARGET_QUERY:

.. rubric:: ``Tokudb_BASEMENTS_DECOMPRESSED_TARGET_QUERY`` 

This variable shows the number of basement nodes decompressed for queries.

.. _Tokudb_BASEMENTS_DECOMPRESSED_PRELOCKED_RANGE:

.. rubric:: ``Tokudb_BASEMENTS_DECOMPRESSED_PRELOCKED_RANGE`` 

This variable shows the number of basement nodes aggressively decompressed by
queries.

.. _Tokudb_BASEMENTS_DECOMPRESSED_PREFETCH:

.. rubric:: ``Tokudb_BASEMENTS_DECOMPRESSED_PREFETCH`` 

This variable shows the number of basement nodes decompressed by a prefetch
thread.

.. _Tokudb_BASEMENTS_DECOMPRESSED_FOR_WRITE:

.. rubric:: ``Tokudb_BASEMENTS_DECOMPRESSED_FOR_WRITE`` 

This variable shows the number of basement nodes decompressed for writes.

.. _Tokudb_BUFFERS_DECOMPRESSED_TARGET_QUERY:

.. rubric:: ``Tokudb_BUFFERS_DECOMPRESSED_TARGET_QUERY`` 

This variable shows the number of buffers decompressed for queries.

.. _Tokudb_BUFFERS_DECOMPRESSED_PRELOCKED_RANGE:

.. rubric:: ``Tokudb_BUFFERS_DECOMPRESSED_PRELOCKED_RANGE`` 

This variable shows the number of buffers decompressed by queries aggressively.

.. _Tokudb_BUFFERS_DECOMPRESSED_PREFETCH:

.. rubric:: ``Tokudb_BUFFERS_DECOMPRESSED_PREFETCH`` 

This variable shows the number of buffers decompressed by a prefetch thread.

.. _Tokudb_BUFFERS_DECOMPRESSED_FOR_WRITE:

.. rubric:: ``Tokudb_BUFFERS_DECOMPRESSED_FOR_WRITE`` 

This variable shows the number of buffers decompressed for writes.

.. _Tokudb_PIVOTS_FETCHED_FOR_QUERY:

.. rubric:: ``Tokudb_PIVOTS_FETCHED_FOR_QUERY`` 

This variable shows the number of pivot nodes fetched for queries.

.. _Tokudb_PIVOTS_FETCHED_FOR_QUERY_BYTES:

.. rubric:: ``Tokudb_PIVOTS_FETCHED_FOR_QUERY_BYTES`` 

This variable shows the number of bytes of pivot nodes fetched for queries.

.. _Tokudb_PIVOTS_FETCHED_FOR_QUERY_SECONDS:

.. rubric:: ``Tokudb_PIVOTS_FETCHED_FOR_QUERY_SECONDS`` 

This variable shows the number of seconds waiting for I/O when fetching pivot
nodes for queries.

.. _Tokudb_PIVOTS_FETCHED_FOR_PREFETCH:

.. rubric:: ``Tokudb_PIVOTS_FETCHED_FOR_PREFETCH`` 

This variable shows the number of pivot nodes fetched by a prefetch thread.

.. _Tokudb_PIVOTS_FETCHED_FOR_PREFETCH_BYTES:

.. rubric:: ``Tokudb_PIVOTS_FETCHED_FOR_PREFETCH_BYTES`` 

This variable shows the number of bytes of pivot nodes fetched for queries.

.. _Tokudb_PIVOTS_FETCHED_FOR_PREFETCH_SECONDS:

.. rubric:: ``Tokudb_PIVOTS_FETCHED_FOR_PREFETCH_SECONDS`` 

This variable shows the number seconds waiting for I/O when fetching pivot
nodes by a prefetch thread.

.. _Tokudb_PIVOTS_FETCHED_FOR_WRITE:

.. rubric:: ``Tokudb_PIVOTS_FETCHED_FOR_WRITE`` 

This variable shows the number of pivot nodes fetched for writes.

.. _Tokudb_PIVOTS_FETCHED_FOR_WRITE_BYTES:

.. rubric:: ``Tokudb_PIVOTS_FETCHED_FOR_WRITE_BYTES`` 

This variable shows the number of bytes of pivot nodes fetched for writes.

.. _Tokudb_PIVOTS_FETCHED_FOR_WRITE_SECONDS:

.. rubric:: ``Tokudb_PIVOTS_FETCHED_FOR_WRITE_SECONDS`` 

This variable shows the number of seconds waiting for I/O when fetching pivot
nodes for writes.

.. _Tokudb_BASEMENTS_FETCHED_TARGET_QUERY:

.. rubric:: ``Tokudb_BASEMENTS_FETCHED_TARGET_QUERY`` 

This variable shows the number of basement nodes fetched from disk for queries.

.. _Tokudb_BASEMENTS_FETCHED_TARGET_QUERY_BYTES:

.. rubric:: ``Tokudb_BASEMENTS_FETCHED_TARGET_QUERY_BYTES`` 

This variable shows the number of basement node bytes fetched from disk for
queries.

.. _Tokudb_BASEMENTS_FETCHED_TARGET_QUERY_SECONDS:

.. rubric:: ``Tokudb_BASEMENTS_FETCHED_TARGET_QUERY_SECONDS`` 

This variable shows the number of seconds waiting for I/O when fetching
basement nodes from disk for queries.

.. _Tokudb_BASEMENTS_FETCHED_PRELOCKED_RANGE:

.. rubric:: ``Tokudb_BASEMENTS_FETCHED_PRELOCKED_RANGE`` 

This variable shows the number of basement nodes fetched from disk
aggressively.

.. _Tokudb_BASEMENTS_FETCHED_PRELOCKED_RANGE_BYTES:

.. rubric:: ``Tokudb_BASEMENTS_FETCHED_PRELOCKED_RANGE_BYTES`` 

This variable shows the number of basement node bytes fetched from disk
aggressively.

.. _Tokudb_BASEMENTS_FETCHED_PRELOCKED_RANGE_SECONDS:

.. rubric:: ``Tokudb_BASEMENTS_FETCHED_PRELOCKED_RANGE_SECONDS`` 

This variable shows the number of seconds waiting for I/O when fetching
basement nodes from disk aggressively.

.. _Tokudb_BASEMENTS_FETCHED_PREFETCH:

.. rubric:: ``Tokudb_BASEMENTS_FETCHED_PREFETCH`` 

This variable shows the number of basement nodes fetched from disk by a
prefetch thread.

.. _Tokudb_BASEMENTS_FETCHED_PREFETCH_BYTES:

.. rubric:: ``Tokudb_BASEMENTS_FETCHED_PREFETCH_BYTES`` 

This variable shows the number of basement node bytes fetched from disk by a
prefetch thread.


.. _Tokudb_BASEMENTS_FETCHED_PREFETCH_SECONDS:

.. rubric:: ``Tokudb_BASEMENTS_FETCHED_PREFETCH_SECONDS`` 

This variable shows the number of seconds waiting for I/O when fetching
basement nodes from disk by a prefetch thread.

.. _Tokudb_BASEMENTS_FETCHED_FOR_WRITE:

.. rubric:: ``Tokudb_BASEMENTS_FETCHED_FOR_WRITE`` 

This variable shows the number of buffers fetched from disk for writes.

.. _Tokudb_BASEMENTS_FETCHED_FOR_WRITE_BYTES:

.. rubric:: ``Tokudb_BASEMENTS_FETCHED_FOR_WRITE_BYTES`` 

This variable shows the number of buffer bytes fetched from disk for writes.

.. _Tokudb_BASEMENTS_FETCHED_FOR_WRITE_SECONDS:

.. rubric:: ``Tokudb_BASEMENTS_FETCHED_FOR_WRITE_SECONDS`` 

This variable shows the number of seconds waiting for I/O when fetching buffers
from disk for writes.

.. _Tokudb_BUFFERS_FETCHED_TARGET_QUERY:

.. rubric:: ``Tokudb_BUFFERS_FETCHED_TARGET_QUERY`` 

This variable shows the number of buffers fetched from disk for queries.

.. _Tokudb_BUFFERS_FETCHED_TARGET_QUERY_BYTES:

.. rubric:: ``Tokudb_BUFFERS_FETCHED_TARGET_QUERY_BYTES`` 

This variable shows the number of buffer bytes fetched from disk for queries.

.. _Tokudb_BUFFERS_FETCHED_TARGET_QUERY_SECONDS:

.. rubric:: ``Tokudb_BUFFERS_FETCHED_TARGET_QUERY_SECONDS`` 

This variable shows the number of seconds waiting for I/O when fetching buffers
from disk for queries.

.. _Tokudb_BUFFERS_FETCHED_PRELOCKED_RANGE:

.. rubric:: ``Tokudb_BUFFERS_FETCHED_PRELOCKED_RANGE`` 

This variable shows the number of buffers fetched from disk aggressively.

.. _Tokudb_BUFFERS_FETCHED_PRELOCKED_RANGE_BYTES:

.. rubric:: ``Tokudb_BUFFERS_FETCHED_PRELOCKED_RANGE_BYTES`` 

This variable shows the number of buffer bytes fetched from disk aggressively.

.. _Tokudb_BUFFERS_FETCHED_PRELOCKED_RANGE_SECONDS:

.. rubric:: ``Tokudb_BUFFERS_FETCHED_PRELOCKED_RANGE_SECONDS`` 

This variable shows the number of seconds waiting for I/O when fetching buffers
from disk aggressively.

.. _Tokudb_BUFFERS_FETCHED_PREFETCH:

.. rubric:: ``Tokudb_BUFFERS_FETCHED_PREFETCH`` 

This variable shows the number of buffers fetched from disk aggressively.

.. _Tokudb_BUFFERS_FETCHED_PREFETCH_BYTES:

.. rubric:: ``Tokudb_BUFFERS_FETCHED_PREFETCH_BYTES`` 

This variable shows the number of buffer bytes fetched from disk by a prefetch
thread.

.. _Tokudb_BUFFERS_FETCHED_PREFETCH_SECONDS:

.. rubric:: ``Tokudb_BUFFERS_FETCHED_PREFETCH_SECONDS`` 

This variable shows the number of seconds waiting for I/O when fetching buffers
from disk by a prefetch thread.

.. _Tokudb_BUFFERS_FETCHED_FOR_WRITE:

.. rubric:: ``Tokudb_BUFFERS_FETCHED_FOR_WRITE`` 

This variable shows the number of buffers fetched from disk for writes.

.. _Tokudb_BUFFERS_FETCHED_FOR_WRITE_BYTES:

.. rubric:: ``Tokudb_BUFFERS_FETCHED_FOR_WRITE_BYTES`` 

This variable shows the number of buffer bytes fetched from disk for writes.

.. _Tokudb_BUFFERS_FETCHED_FOR_WRITE_SECONDS:

.. rubric:: ``Tokudb_BUFFERS_FETCHED_FOR_WRITE_SECONDS`` 

This variable shows the number of seconds waiting for I/O when fetching buffers
from disk for writes.

.. _Tokudb_LEAF_COMPRESSION_TO_MEMORY_SECONDS:

.. rubric:: ``Tokudb_LEAF_COMPRESSION_TO_MEMORY_SECONDS`` 

This variable shows the total time, in seconds, spent compressing leaf nodes.

.. _Tokudb_LEAF_SERIALIZATION_TO_MEMORY_SECONDS:

.. rubric:: ``Tokudb_LEAF_SERIALIZATION_TO_MEMORY_SECONDS`` 

This variable shows the total time, in seconds, spent serializing leaf nodes.

.. _Tokudb_LEAF_DECOMPRESSION_TO_MEMORY_SECONDS:

.. rubric:: ``Tokudb_LEAF_DECOMPRESSION_TO_MEMORY_SECONDS`` 

This variable shows the total time, in seconds, spent decompressing leaf nodes.

.. _Tokudb_LEAF_DESERIALIZATION_TO_MEMORY_SECONDS:

.. rubric:: ``Tokudb_LEAF_DESERIALIZATION_TO_MEMORY_SECONDS`` 

This variable shows the total time, in seconds, spent deserializing leaf nodes.

.. _Tokudb_NONLEAF_COMPRESSION_TO_MEMORY_SECONDS:

.. rubric:: ``Tokudb_NONLEAF_COMPRESSION_TO_MEMORY_SECONDS`` 

This variable shows the total time, in seconds, spent compressing non leaf
nodes.

.. _Tokudb_NONLEAF_SERIALIZATION_TO_MEMORY_SECONDS:

.. rubric:: ``Tokudb_NONLEAF_SERIALIZATION_TO_MEMORY_SECONDS`` 

This variable shows the total time, in seconds, spent serializing non leaf
nodes.

.. _Tokudb_NONLEAF_DECOMPRESSION_TO_MEMORY_SECONDS:

.. rubric:: ``Tokudb_NONLEAF_DECOMPRESSION_TO_MEMORY_SECONDS`` 

This variable shows the total time, in seconds, spent decompressing non leaf
nodes.

.. _Tokudb_NONLEAF_DESERIALIZATION_TO_MEMORY_SECONDS:

.. rubric:: ``Tokudb_NONLEAF_DESERIALIZATION_TO_MEMORY_SECONDS`` 

This variable shows the total time, in seconds, spent deserializing non leaf
nodes.

.. _Tokudb_PROMOTION_ROOTS_SPLIT:

.. rubric:: ``Tokudb_PROMOTION_ROOTS_SPLIT`` 

This variable shows the number of times the root split during promotion.

.. _Tokudb_PROMOTION_LEAF_ROOTS_INJECTED_INTO:

.. rubric:: ``Tokudb_PROMOTION_LEAF_ROOTS_INJECTED_INTO`` 

This variable shows the number of times a message stopped at a root with
height ``0``.

.. _Tokudb_PROMOTION_H1_ROOTS_INJECTED_INTO:

.. rubric:: ``Tokudb_PROMOTION_H1_ROOTS_INJECTED_INTO`` 

This variable shows the number of times a message stopped at a root with
height ``1``.

.. _Tokudb_PROMOTION_INJECTIONS_AT_DEPTH_0:

.. rubric:: ``Tokudb_PROMOTION_INJECTIONS_AT_DEPTH_0`` 

This variable shows the number of times a message stopped at depth ``0``.

.. _Tokudb_PROMOTION_INJECTIONS_AT_DEPTH_1:

.. rubric:: ``Tokudb_PROMOTION_INJECTIONS_AT_DEPTH_1`` 

This variable shows the number of times a message stopped at depth ``1``.

.. _Tokudb_PROMOTION_INJECTIONS_AT_DEPTH_2:

.. rubric:: ``Tokudb_PROMOTION_INJECTIONS_AT_DEPTH_2`` 

This variable shows the number of times a message stopped at depth ``2``.

.. _Tokudb_PROMOTION_INJECTIONS_AT_DEPTH_3:

.. rubric:: ``Tokudb_PROMOTION_INJECTIONS_AT_DEPTH_3`` 

This variable shows the number of times a message stopped at depth ``3``.

.. _Tokudb_PROMOTION_INJECTIONS_LOWER_THAN_DEPTH_3:

.. rubric:: ``Tokudb_PROMOTION_INJECTIONS_LOWER_THAN_DEPTH_3`` 

This variable shows the number of times a message was promoted past depth
``3``.

.. _Tokudb_PROMOTION_STOPPED_NONEMPTY_BUFFER:

.. rubric:: ``Tokudb_PROMOTION_STOPPED_NONEMPTY_BUFFER`` 

This variable shows the number of times a message stopped because it reached
a nonempty buffer.

.. _Tokudb_PROMOTION_STOPPED_AT_HEIGHT_1:

.. rubric:: ``Tokudb_PROMOTION_STOPPED_AT_HEIGHT_1`` 

This variable shows the number of times a message stopped because it had
reached height ``1``.

.. _Tokudb_PROMOTION_STOPPED_CHILD_LOCKED_OR_NOT_IN_MEMORY:

.. rubric:: ``Tokudb_PROMOTION_STOPPED_CHILD_LOCKED_OR_NOT_IN_MEMORY`` 

This variable shows the number of times a message stopped because it could not
cheaply get access to a child.

.. _Tokudb_PROMOTION_STOPPED_CHILD_NOT_FULLY_IN_MEMORY:

.. rubric:: ``Tokudb_PROMOTION_STOPPED_CHILD_NOT_FULLY_IN_MEMORY`` 

This variable shows the number of times a message stopped because it could not
cheaply get access to a child.

.. _Tokudb_PROMOTION_STOPPED_AFTER_LOCKING_CHILD:

.. rubric:: ``Tokudb_PROMOTION_STOPPED_AFTER_LOCKING_CHILD`` 

This variable shows the number of times a message stopped before a child which
had been locked.

.. _Tokudb_BASEMENT_DESERIALIZATION_FIXED_KEY:

.. rubric:: ``Tokudb_BASEMENT_DESERIALIZATION_FIXED_KEY`` 

This variable shows the number of basement nodes deserialized where all keys
had the same size, leaving the basement in a format that is optimal for
in-memory workloads.

.. _Tokudb_BASEMENT_DESERIALIZATION_VARIABLE_KEY:

.. rubric:: ``Tokudb_BASEMENT_DESERIALIZATION_VARIABLE_KEY`` 

This variable shows the number of basement nodes deserialized where all keys
did not have the same size, and thus ineligible for an in-memory optimization.

.. _Tokudb_PRO_RIGHTMOST_LEAF_SHORTCUT_SUCCESS:

.. rubric:: ``Tokudb_PRO_RIGHTMOST_LEAF_SHORTCUT_SUCCESS`` 

This variable shows the number of times a message injection detected a series
of sequential inserts to the rightmost side of the tree and successfully
applied an insert message directly to the rightmost leaf node. This is a not a
useful value for a regular user to use for any purpose.

.. _Tokudb_PRO_RIGHTMOST_LEAF_SHORTCUT_FAIL_POS:

.. rubric:: ``Tokudb_PRO_RIGHTMOST_LEAF_SHORTCUT_FAIL_POS`` 

This variable shows the number of times a message injection detected a series
of sequential inserts to the rightmost side of the tree and was unable to
follow the pattern of directly applying an insert message directly to the
rightmost leaf node because the key does not continue the sequence. This is a
not a useful value for a regular user to use for any purpose.

.. _Tokudb_RIGHTMOST_LEAF_SHORTCUT_FAIL_REACTIVE:

.. rubric:: ``Tokudb_RIGHTMOST_LEAF_SHORTCUT_FAIL_REACTIVE`` 

This variable shows the number of times a message injection detected a series
of sequential inserts to the rightmost side of the tree and was unable to
follow the pattern of directly applying an insert message directly to the
rightmost leaf node because the leaf is full. This is a not a useful value for
a regular user to use for any purpose.

.. _Tokudb_CURSOR_SKIP_DELETED_LEAF_ENTRY:

.. rubric:: ``Tokudb_CURSOR_SKIP_DELETED_LEAF_ENTRY`` 

This variable shows the number of leaf entries skipped during search/scan
because the result of message application and reconciliation of the leaf entry
MVCC stack reveals that the leaf entry is ``deleted`` in the current
transactions view. It is a good indicator that there might be excessive garbage
in a tree if a range scan seems to take too long.

.. _Tokudb_FLUSHER_CLEANER_TOTAL_NODES:

.. rubric:: ``Tokudb_FLUSHER_CLEANER_TOTAL_NODES`` 

This variable shows the total number of nodes potentially flushed by flusher or
cleaner threads. This is a not a useful value for a regular user to use for any
purpose.

.. _Tokudb_FLUSHER_CLEANER_H1_NODES:

.. rubric:: ``Tokudb_FLUSHER_CLEANER_H1_NODES`` 

This variable shows the number of height ``1`` nodes that had messages flushed
by flusher or cleaner threads, i.e., internal nodes immediately above leaf
nodes. This is a not a useful value for a regular user to use for any purpose.

.. _Tokudb_FLUSHER_CLEANER_HGT1_NODES:

.. rubric:: ``Tokudb_FLUSHER_CLEANER_HGT1_NODES`` 

This variable shows the number of nodes with height greater than ``1`` that had
messages flushed by flusher or cleaner threads. This is a not a useful value
for a regular user to use for any purpose.

.. _Tokudb_FLUSHER_CLEANER_EMPTY_NODES:

.. rubric:: ``Tokudb_FLUSHER_CLEANER_EMPTY_NODES`` 

This variable shows the number of nodes cleaned by flusher or cleaner threads
which had empty message buffers. This is a not a useful value for a regular
user to use for any purpose.

.. _Tokudb_FLUSHER_CLEANER_NODES_DIRTIED:

.. rubric:: ``Tokudb_FLUSHER_CLEANER_NODES_DIRTIED`` 

This variable shows the number of nodes dirtied by flusher or cleaner threads
as a result of flushing messages downward. This is a not a useful value for a
regular user to use for any purpose.

.. _Tokudb_FLUSHER_CLEANER_MAX_BUFFER_SIZE:

.. rubric:: ``Tokudb_FLUSHER_CLEANER_MAX_BUFFER_SIZE`` 

This variable shows the maximum bytes in a message buffer flushed by flusher or
cleaner threads. This is a not a useful value for a regular user to use for any
purpose.

.. _Tokudb_FLUSHER_CLEANER_MIN_BUFFER_SIZE:

.. rubric:: ``Tokudb_FLUSHER_CLEANER_MIN_BUFFER_SIZE`` 

This variable shows the minimum bytes in a message buffer flushed by flusher or
cleaner threads. This is a not a useful value for a regular user to use for any
purpose.

.. _Tokudb_FLUSHER_CLEANER_TOTAL_BUFFER_SIZE:

.. rubric:: ``Tokudb_FLUSHER_CLEANER_TOTAL_BUFFER_SIZE`` 

This variable shows the total bytes in buffers flushed by flusher and cleaner
threads. This is a not a useful value for a regular user to use for any purpose.

.. _Tokudb_FLUSHER_CLEANER_MAX_BUFFER_WORKDONE:

.. rubric:: ``Tokudb_FLUSHER_CLEANER_MAX_BUFFER_WORKDONE`` 

This variable shows the maximum bytes worth of work done in a message buffer
flushed by flusher or cleaner threads. This is a not a useful value for a
regular user to use for any purpose.

.. _Tokudb_FLUSHER_CLEANER_MIN_BUFFER_WORKDONE:

.. rubric:: ``Tokudb_FLUSHER_CLEANER_MIN_BUFFER_WORKDONE`` 

This variable shows the minimum bytes worth of work done in a message buffer
flushed by flusher or cleaner threads. This is a not a useful value for a
regular user to use for any purpose.

.. _Tokudb_FLUSHER_CLEANER_TOTAL_BUFFER_WORKDONE:

.. rubric:: ``Tokudb_FLUSHER_CLEANER_TOTAL_BUFFER_WORKDONE`` 

This variable shows the total bytes worth of work done in buffers flushed by
flusher or cleaner threads. This is a not a useful value for a regular user to
use for any purpose.

.. _Tokudb_FLUSHER_CLEANER_NUM_LEAF_MERGES_STARTED:

.. rubric:: ``Tokudb_FLUSHER_CLEANER_NUM_LEAF_MERGES_STARTED`` 

This variable shows the number of times flusher and cleaner threads tried to
merge two leafs. This is a not a useful value for a regular user to use for any
purpose.

.. _Tokudb_FLUSHER_CLEANER_NUM_LEAF_MERGES_RUNNING:

.. rubric:: ``Tokudb_FLUSHER_CLEANER_NUM_LEAF_MERGES_RUNNING`` 

This variable shows the number of flusher and cleaner threads leaf merges in
progress. This is a not a useful value for a regular user to use for any
purpose.

.. _Tokudb_FLUSHER_CLEANER_NUM_LEAF_MERGES_COMPLETED:

.. rubric:: ``Tokudb_FLUSHER_CLEANER_NUM_LEAF_MERGES_COMPLETED`` 

This variable shows the number of successful flusher and cleaner threads leaf
merges. This is a not a useful value for a regular user to use for any purpose.

.. _Tokudb_FLUSHER_CLEANER_NUM_DIRTIED_FOR_LEAF_MERGE:

.. rubric:: ``Tokudb_FLUSHER_CLEANER_NUM_DIRTIED_FOR_LEAF_MERGE`` 

This variable shows the number of nodes dirtied by flusher or cleaner threads
performing leaf node merges. This is a not a useful value for a regular user to
use for any purpose.

.. _Tokudb_FLUSHER_FLUSH_TOTAL:

.. rubric:: ``Tokudb_FLUSHER_FLUSH_TOTAL`` 

This variable shows the total number of flushes done by flusher threads or
cleaner threads. This is a not a useful value for a regular user to use for any
purpose.

.. _Tokudb_FLUSHER_FLUSH_IN_MEMORY:

.. rubric:: ``Tokudb_FLUSHER_FLUSH_IN_MEMORY`` 

This variable shows the number of in memory flushes (required no disk reads) by
flusher or cleaner threads. This is a not a useful value for a regular user to
use for any purpose.

.. _Tokudb_FLUSHER_FLUSH_NEEDED_IO:

.. rubric:: ``Tokudb_FLUSHER_FLUSH_NEEDED_IO`` 

This variable shows the number of flushes that read something off disk by
flusher or cleaner threads. This is a not a useful value for a regular user to
use for any purpose.

.. _Tokudb_FLUSHER_FLUSH_CASCADES:

.. rubric:: ``Tokudb_FLUSHER_FLUSH_CASCADES`` 

This variable shows the number of flushes that triggered a flush in child node
by flusher or cleaner threads. This is a not a useful value for a regular user
to use for any purpose.

.. _Tokudb_FLUSHER_FLUSH_CASCADES_1:

.. rubric:: ``Tokudb_FLUSHER_FLUSH_CASCADES_1`` 

This variable shows the number of flushes that triggered one cascading flush by
flusher or cleaner threads. This is a not a useful value for a regular user to
use for any purpose.

.. _Tokudb_FLUSHER_FLUSH_CASCADES_2:

.. rubric:: ``Tokudb_FLUSHER_FLUSH_CASCADES_2`` 

This variable shows the number of flushes that triggered two cascading flushes
by flusher or cleaner threads. This is a not a useful value for a regular user
to use for any purpose.

.. _Tokudb_FLUSHER_FLUSH_CASCADES_3:

.. rubric:: ``Tokudb_FLUSHER_FLUSH_CASCADES_3`` 

This variable shows the number of flushes that triggered three cascading
flushes by flusher or cleaner threads. This is a not a useful value for a
regular user to use for any purpose.

.. _Tokudb_FLUSHER_FLUSH_CASCADES_4:

.. rubric:: ``Tokudb_FLUSHER_FLUSH_CASCADES_4`` 

This variable shows the number of flushes that triggered four cascading
flushes by flusher or cleaner threads. This is a not a useful value for a
regular user to use for any purpose.

.. _Tokudb_FLUSHER_FLUSH_CASCADES_5:

.. rubric:: ``Tokudb_FLUSHER_FLUSH_CASCADES_5`` 

This variable shows the number of flushes that triggered five cascading
flushes by flusher or cleaner threads. This is a not a useful value for a
regular user to use for any purpose.

.. _Tokudb_FLUSHER_FLUSH_CASCADES_GT_5:

.. rubric:: ``Tokudb_FLUSHER_FLUSH_CASCADES_GT_5`` 

This variable shows the number of flushes that triggered more than five
cascading flushes by flusher or cleaner threads. This is a not a useful value
for a regular user to use for any purpose.

.. _Tokudb_FLUSHER_SPLIT_LEAF:

.. rubric:: ``Tokudb_FLUSHER_SPLIT_LEAF`` 

This variable shows the total number of leaf node splits done by flusher
threads or cleaner threads. This is a not a useful value for a regular user to
use for any purpose.

.. _Tokudb_FLUSHER_SPLIT_NONLEAF:

.. rubric:: ``Tokudb_FLUSHER_SPLIT_NONLEAF`` 

This variable shows the total number of non-leaf node splits done by flusher
threads or cleaner threads. This is a not a useful value for a regular user to
use for any purpose.

.. _Tokudb_FLUSHER_MERGE_LEAF:

.. rubric:: ``Tokudb_FLUSHER_MERGE_LEAF`` 

This variable shows the total number of leaf node merges done by flusher
threads or cleaner threads. This is a not a useful value for a regular user to
use for any purpose.

.. _Tokudb_FLUSHER_MERGE_NONLEAF:

.. rubric:: ``Tokudb_FLUSHER_MERGE_NONLEAF`` 

This variable shows the total number of non-leaf node merges done by flusher
threads or cleaner threads. This is a not a useful value for a regular user to
use for any purpose.

.. _Tokudb_FLUSHER_BALANCE_LEAF:

.. rubric:: ``Tokudb_FLUSHER_BALANCE_LEAF`` 

This variable shows the number of times two adjacent leaf nodes were rebalanced
or had their content redistributed evenly by flusher or cleaner threads. This
is a not a useful value for a regular user to use for any purpose.

.. _Tokudb_HOT_NUM_STARTED:

.. rubric:: ``Tokudb_HOT_NUM_STARTED`` 

This variable shows the number of hot operations started (``OPTIMIZE TABLE``).
This is a not a useful value for a regular user to use for any purpose.

.. _Tokudb_HOT_NUM_COMPLETED:

.. rubric:: ``Tokudb_HOT_NUM_COMPLETED`` 

This variable shows the number of hot operations completed (``OPTIMIZE TABLE``).
This is a not a useful value for a regular user to use for any purpose.

.. _Tokudb_HOT_NUM_ABORTED:

.. rubric:: ``Tokudb_HOT_NUM_ABORTED`` 

This variable shows the number of hot operations aborted (``OPTIMIZE TABLE``).
This is a not a useful value for a regular user to use for any purpose.

.. _Tokudb_HOT_MAX_ROOT_FLUSH_COUNT:

.. rubric:: ``Tokudb_HOT_MAX_ROOT_FLUSH_COUNT`` 

This variable shows the maximum number of flushes from root ever required to
optimize trees. This is a not a useful value for a regular user to use for any
purpose.

.. _Tokudb_TXN_BEGIN:

.. rubric:: ``Tokudb_TXN_BEGIN`` 

This variable shows the number of transactions that have been started.

.. _Tokudb_TXN_BEGIN_READ_ONLY:

.. rubric:: ``Tokudb_TXN_BEGIN_READ_ONLY`` 

This variable shows the number of read-only transactions started.

.. _Tokudb_TXN_COMMITS:

.. rubric:: ``Tokudb_TXN_COMMITS`` 

This variable shows the total number of transactions that have been committed.

.. _Tokudb_TXN_ABORTS:

.. rubric:: ``Tokudb_TXN_ABORTS`` 

This variable shows the total number of transactions that have been aborted.

.. _Tokudb_LOGGER_NEXT_LSN:

.. rubric:: ``Tokudb_LOGGER_NEXT_LSN`` 

This variable shows the recovery logger next LSN. This is a not a useful value
for a regular user to use for any purpose.

.. _Tokudb_LOGGER_WRITES:

.. rubric:: ``Tokudb_LOGGER_WRITES`` 

This variable shows the number of times the logger has written to disk.

.. _Tokudb_LOGGER_WRITES_BYTES:

.. rubric:: ``Tokudb_LOGGER_WRITES_BYTES`` 

This variable shows the number of bytes the logger has written to disk.

.. _Tokudb_LOGGER_WRITES_UNCOMPRESSED_BYTES:

.. rubric:: ``Tokudb_LOGGER_WRITES_UNCOMPRESSED_BYTES`` 

This variable shows the number of uncompressed bytes the logger has written to
disk.

.. _Tokudb_LOGGER_WRITES_SECONDS:

.. rubric:: ``Tokudb_LOGGER_WRITES_SECONDS`` 

This variable shows the number of seconds waiting for IO when writing logs to
disk.

.. _Tokudb_LOGGER_WAIT_LONG:

.. rubric:: ``Tokudb_LOGGER_WAIT_LONG`` 

This variable shows the number of times a logger write operation required 100ms
or more.

.. _Tokudb_LOADER_NUM_CREATED:

.. rubric:: ``Tokudb_LOADER_NUM_CREATED`` 

This variable shows the number of times one of our internal objects, a loader,
has been created.

.. _Tokudb_LOADER_NUM_CURRENT:

.. rubric:: ``Tokudb_LOADER_NUM_CURRENT`` 

This variable shows the number of loaders that currently exist.

.. _Tokudb_LOADER_NUM_MAX:

.. rubric:: ``Tokudb_LOADER_NUM_MAX`` 

This variable shows the maximum number of loaders that ever existed
simultaneously.

.. _Tokudb_MEMORY_MALLOC_COUNT:

.. rubric:: ``Tokudb_MEMORY_MALLOC_COUNT`` 

This variable shows the number of ``malloc`` operations by PerconaFT.

.. _Tokudb_MEMORY_FREE_COUNT:

.. rubric:: ``Tokudb_MEMORY_FREE_COUNT`` 

This variable shows the number of ``free`` operations by PerconaFT.

.. _Tokudb_MEMORY_REALLOC_COUNT:

.. rubric:: ``Tokudb_MEMORY_REALLOC_COUNT`` 

This variable shows the number of ``realloc`` operations by PerconaFT.

.. _Tokudb_MEMORY_MALLOC_FAIL:

.. rubric:: ``Tokudb_MEMORY_MALLOC_FAIL`` 

This variable shows the number of ``malloc`` operations that failed by
PerconaFT.

.. _Tokudb_MEMORY_REALLOC_FAIL:

.. rubric:: ``Tokudb_MEMORY_REALLOC_FAIL`` 

This variable shows the number of ``realloc`` operations that failed by
PerconaFT.

.. _Tokudb_MEMORY_REQUESTED:

.. rubric:: ``Tokudb_MEMORY_REQUESTED`` 

This variable shows the number of bytes requested by PerconaFT.

.. _Tokudb_MEMORY_USED:

.. rubric:: ``Tokudb_MEMORY_USED`` 

This variable shows the number of bytes used (requested + overhead) by
PerconaFT.

.. _Tokudb_MEMORY_FREED:

.. rubric:: ``Tokudb_MEMORY_FREED`` 

This variable shows the number of bytes freed by PerconaFT.

.. _Tokudb_MEMORY_MAX_REQUESTED_SIZE:

.. rubric:: ``Tokudb_MEMORY_MAX_REQUESTED_SIZE`` 

This variable shows the largest attempted allocation size by PerconaFT.

.. _Tokudb_MEMORY_LAST_FAILED_SIZE:

.. rubric:: ``Tokudb_MEMORY_LAST_FAILED_SIZE`` 

This variable shows the size of the last failed allocation attempt by
PerconaFT.

.. _Tokudb_MEM_ESTIMATED_MAXIMUM_MEMORY_FOOTPRINT:

.. rubric:: ``Tokudb_MEM_ESTIMATED_MAXIMUM_MEMORY_FOOTPRINT`` 

This variable shows the maximum memory footprint of the storage engine, the
max value of (used - freed).

.. _Tokudb_MEMORY_MALLOCATOR_VERSION:

.. rubric:: ``Tokudb_MEMORY_MALLOCATOR_VERSION`` 

This variable shows the version of the memory allocator library detected by
PerconaFT.

.. _Tokudb_MEMORY_MMAP_THRESHOLD:

.. rubric:: ``Tokudb_MEMORY_MMAP_THRESHOLD`` 

This variable shows the ``mmap`` threshold in PerconaFT, anything larger than
this gets ``mmap'ed``.

.. _Tokudb_FILESYSTEM_THREADS_BLOCKED_BY_FULL_DISK:

.. rubric:: ``Tokudb_FILESYSTEM_THREADS_BLOCKED_BY_FULL_DISK`` 

This variable shows the number of threads that are currently blocked because
they are attempting to write to a full disk. This is normally zero. If this
value is non-zero, then a warning will appear in the ``disk free space`` field.

.. _Tokudb_FILESYSTEM_FSYNC_TIME:

.. rubric:: ``Tokudb_FILESYSTEM_FSYNC_TIME`` 

This variable shows the total time, in microseconds, used to ``fsync`` to disk.

.. _Tokudb_FILESYSTEM_FSYNC_NUM:

.. rubric:: ``Tokudb_FILESYSTEM_FSYNC_NUM`` 

This variable shows the total number of times the database has flushed the
operating system's file buffers to disk.

.. _Tokudb_FILESYSTEM_LONG_FSYNC_TIME:

.. rubric:: ``Tokudb_FILESYSTEM_LONG_FSYNC_TIME`` 

This variable shows the total time, in microseconds, used to ``fsync`` to dis
k when the operation required more than one second.

.. _Tokudb_FILESYSTEM_LONG_FSYNC_NUM:

.. rubric:: ``Tokudb_FILESYSTEM_LONG_FSYNC_NUM`` 

This variable shows the total number of times the database has flushed the
operating system's file buffers to disk and this operation required more than
one second.
