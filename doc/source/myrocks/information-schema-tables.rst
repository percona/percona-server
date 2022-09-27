.. _ps.myrocks.information-schema-table:

================================================================================
|myrocks| |information-schema| Tables
================================================================================

When you install the |myrocks| plugin for |MySQL|, the
|information-schema| is extended to include the following tables:

.. contents::
   :local:
   :depth: 1

.. _ps.myrocks.information-schema-table.rocksdb-global-info:

ROCKSDB_GLOBAL_INFO
================================================================================

.. rubric:: Columns

.. list-table::
   :header-rows: 1

   * - Column Name
     - Type
   * - TYPE
     - varchar(513)
   * - NAME
     - varchar(513)
   * - VALUE
     - varchar(513)

.. _ps.myrocks.information-schema-table.rocksdb-cfstats:

ROCKSDB_CFSTATS
================================================================================

.. rubric:: Columns

.. list-table::
   :header-rows: 1

   * - Column Name
     - Type
   * - CF_NAME
     - varchar(193)
   * - STAT_TYPE
     - varchar(193)
   * - VALUE
     - bigint(8) 

.. _ps.myrocks.information-schema-table.rocksdb-trx:

ROCKSDB_TRX
================================================================================

This table stores mappings of |rocksdb| transaction identifiers to |MySQL|
client identifiers to enable associating a |rocksdb| transaction with a |MySQL|
client operation.

.. rubric:: Columns

.. list-table::
   :header-rows: 1

   * - Column Name
     - Type
   * - TRANSACTION_ID
     - bigint(8)  
   * - STATE
     - varchar(193)
   * - NAME
     - varchar(193)
   * - WRITE_COUNT
     - bigint(8)   
   * - LOCK_COUNT
     - bigint(8)   
   * - TIMEOUT_SEC
     - int(4)      
   * - WAITING_KEY
     - varchar(513)
   * - WAITING_COLUMN_FAMILY_ID
     - int(4)      
   * - IS_REPLICATION
     - int(4)      
   * - SKIP_TRX_API
     - int(4)      
   * - READ_ONLY
     - int(4)      
   * - HAS_DEADLOCK_DETECTION
     - int(4)      
   * - NUM_ONGOING_BULKLOAD
     - int(4)      
   * - THREAD_ID
     - int(8)      
   * - QUERY
     - varchar(193)
   
.. _ps.myrocks.information-schema-table.rocksdb-cf-options:

ROCKSDB_CF_OPTIONS
================================================================================

.. rubric:: Columns

.. list-table::
   :header-rows: 1

   * - Column Name
     - Type
   * - CF_NAME
     - varchar(193)
   * - OPTION_TYPE
     - varchar(193)
   * - VALUE
     - varchar(193)

.. _ps.myrocks.information-schema-table.rocksdb-active-compaction-stats:

ROCKSDB_ACTIVE_COMPACTION_STATS
================================================================================

.. rubric:: Columns

.. list-table::
   :header-rows: 1

   * - Column Name
     - Type
   * - THREAD_ID
     - bigint
   * - CF_NAME
     - varchar(513)
   * - INPUT_FILES
     - varchar(513)
   * - OUTPUT_FILES
     - varchar(513)
   * - COMPACTION_REASON
     - varchar(513)

.. _ps.myrocks.information-schema-table.rocksdb-compaction-history:

ROCKSDB_COMPACTION_HISTORY
================================================================================

.. rubric:: Columns

.. list-table:: 
   :header-rows: 1

   * - Column Name
     - Type
   * - THREAD_ID
     - bigint
   * - CF_NAME
     - varchar(513)
   * - INPUT_LEVEL
     - integer
   * - OUTPUT_LEVEL
     - integer
   * - INPUT_FILES
     - varchar(513)
   * - OUTPUT_FILES
     - varchar(513)
   * - COMPACTION_REASON
     - varchar(513)
   * - START_TIMESTAMP
     - bigint
   * - END_TIMESTAMP
     - bigint

.. _ps.myrocks.information-schema-table.rocksdb-compaction-stats:

ROCKSDB_COMPACTION_STATS
================================================================================

.. rubric:: Columns

.. list-table::
   :header-rows: 1

   * - Column Name
     - Type
   * - CF_NAME
     - varchar(193)
   * - LEVEL
     - varchar(513)
   * - TYPE
     - varchar(513)
   * - VALUE
     - double         


.. _ps.myrocks.information-schema-table.rocksdb-dbstats:

ROCKSDB_DBSTATS
================================================================================

.. rubric:: Columns

.. list-table::
   :header-rows: 1

   * - Column Name
     - Type
   * - STAT_TYPE
     - varchar(193)
   * - VALUE
     - bigint(8)

.. _ps.myrocks.information-schema-table.rocksdb-ddl:

ROCKSDB_DDL
================================================================================

.. rubric:: Columns

.. list-table::
   :header-rows: 1

   * - Column Name
     - Type
   * - TABLE_SCHEMA
     - varchar(193)       
   * - TABLE_NAME
     - varchar(193)       
   * - PARTITION_NAME
     - varchar(193)       
   * - INDEX_NAME
     - varchar(193)       
   * - COLUMN_FAMILY
     - int(4)             
   * - INDEX_NUMBER
     - int(4)             
   * - INDEX_TYPE
     - smallint(2)        
   * - KV_FORMAT_VERSION
     - smallint(2)        
   * - TTL_DURATION
     - bigint(8)          
   * - INDEX_FLAGS
     - bigint(8)          
   * - CF
     - varchar(193)       
   * - AUTO_INCREMENT
     - bigint(8) unsigned

.. _ps.myrocks.information-schema-table.rocksdb-index-file-map:

ROCKSDB_INDEX_FILE_MAP
================================================================================

.. rubric:: Columns

.. list-table::
   :header-rows: 1

   * - Column Name
     - Type
   * - COLUMN_FAMILY
     - int(4)
   * - INDEX_NUMBER
     - int(4)
   * - SST_NAME
     - varchar(193)
   * - NUM_ROWS
     - bigint(8)
   * - DATA_SIZE
     - bigint(8)
   * - ENTRY_DELETES
     - bigint(8)
   * - ENTRY_SINGLEDELETES
     - bigint(8)
   * - ENTRY_MERGES
     - bigint(8)
   * - ENTRY_OTHERS
     - bigint(8)
   * - DISTINCT_KEYS_PREFIX
     - varchar(400)
   
.. _ps.myrocks.information-schema-table.rocksdb-locks:

ROCKSDB_LOCKS
================================================================================

This table contains the set of locks granted to |myrocks| transactions.

.. rubric:: Columns

.. list-table::
   :header-rows: 1

   * - Column Name
     - Type
   * - COLUMN_FAMILY_ID
     - int(4)
   * - TRANSACTION_ID
     - int(4)
   * - KEY
     - varchar(513)
   * - MODE
     - varchar(32)

.. _ps.myrocks.information-schema-table.rocksdb-perf-context:

ROCKSDB_PERF_CONTEXT
================================================================================

.. rubric:: Columns

.. list-table::
   :header-rows: 1

   * - Column Name
     - Type
   * - TABLE_SCHEMA
     - varchar(193)
   * - TABLE_NAME
     - varchar(193)
   * - PARTITION_NAME
     - varchar(193)
   * - STAT_TYPE
     - varchar(193)
   * - VALUE
     - bigint(8) 
   
.. _ps.myrocks.information-schema-table.rocksdb-perf-context-global:

ROCKSDB_PERF_CONTEXT_GLOBAL
================================================================================

.. rubric:: Columns

.. list-table::
   :header-rows: 1

   * - Column Name
     - Type
   * - STAT_TYPE
     - varchar(193)
   * - VALUE
     - bigint(8)

.. _ps.myrocks.information-schema-table.rocksdb-deadlock:

ROCKSDB_DEADLOCK
================================================================================

This table records information about deadlocks.

.. rubric:: Columns

.. list-table::
   :header-rows: 1

   * - Column Name
     - Type
   * - DEADLOCK_ID
     - bigint(8)
   * - TRANSACTION_ID
     - bigint(8)
   * - CF_NAME
     - varchar(193)
   * - WAITING_KEY
     - varchar(513)
   * - LOCK_TYPE
     - varchar(193)
   * - INDEX_NAME
     - varchar(193)
   * - TABLE_NAME
     - varchar(193)
   * - ROLLED_BACK
     - bigint(8) 

.. |myrocks| replace:: MyRocks
.. |rocksdb| replace:: RocksDB
.. |information-schema| replace:: Information Schema
