===============================
Misc. INFORMATION_SCHEMA Tables
===============================

This page lists the ``INFORMATION_SCHEMA`` tables added to standard |MySQL| by |Percona Server| that don``t exist elsewhere in the documentation.

Compressed pages in |InnoDB| tables
===================================

These tables hold information on compression and uncompression operations. Their contents is identical, but any read from :table:`INNODB_CMP_RESET` will reset all statistics on compression/uncompression.

These tables were introduced by the |InnoDB| plugin and you can find the full documentation here.

.. table:: INFORMATION_SCHEMA.INNODB_CMP

   :column PAGE_SIZE: Page size for compressed pages (Bytes)
   :column COMPRESS_OPS: Number of compression operations
   :column COMPRESS_OPS_OK: Number of successful compression operations
   :column COMPRESS_TIME: Time spent to compress pages (seconds)
   :column UNCOMPRESS_OPS: Number of uncompression operations
   :column UNCOMPRESS_TIME: Time spent to uncompress pages (seconds)

.. table:: INFORMATION_SCHEMA.INNODB_CMP_RESET

   :column PAGE_SIZE: Page size for compressed pages (Bytes)
   :column COMPRESS_OPS: Number of compression operations
   :column COMPRESS_OPS_OK: Number of successful compression operations
   :column COMPRESS_TIME: Time spent to compress pages (seconds)
   :column UNCOMPRESS_OPS: Number of uncompression operations
   :column UNCOMPRESS_TIME: Time spent to uncompress pages (seconds)

Compressed pages in |InnoDB| buffer pool
========================================

These tables hold information on compressed pages stored in memory in the buffer pool. Their contents is identical, but any read from INNODB_CMPMEM_RESET will reset all statistics on relocation operations.

These tables were introduced by the |InnoDB| plugin and you can find the full documentation here.


.. table:: INFORMATION_SCHEMA.INNODB_CMPMEM

   :column PAGE_SIZE: Page size for compressed pages (Bytes)
   :column PAGES_USED: Number of pages in use
   :column PAGES_FREE: Number of free pages
   :column RELOCATION_OPS: Number of relocation operations
   :column RELOCATION_TIME: Time spent to relocate pages (Microseconds)

.. table:: INFORMATION_SCHEMA.INNODB_CMPMEM_RESET

   :column PAGE_SIZE: Page size for compressed pages (Bytes)
   :column PAGES_USED: Number of pages in use
   :column PAGES_FREE: Number of free pages
   :column RELOCATION_OPS: Number of relocation operations
   :column RELOCATION_TIME: Time spent to relocate pages (Microseconds)

|InnoDB| transactions
=====================

.. table:: INFORMATION_SCHEMA.INNODB_LOCK_WAITS

   :column REQUESTING_TRX_ID:
   :column REQUESTED_LOCK_ID:
   :column BLOCKING_TRX_ID:
   :column BLOCKING_LOCK_ID:

.. table:: INFORMATION_SCHEMA.INNODB_LOCKS

   :column LOCK_ID:  Internal unique lock ID
   :column LOCK_TRX_ID: ID of the transaction holding the lock
   :column LOCK_MODE: Mode of the lock (shared, exclusive, …)
   :column LOCK_TYPE: ``RECORD`` for a record lock and ``TABLE`` for a table lock
   :column LOCK_TABLE: Name of the table holding the lock
   :column LOCK_INDEX: If lock type is ``RECORD``, name of the index
   :column LOCK_SPACE: If lock type is ``RECORD``, tablespace id of the locked record
   :column LOCK_PAGE: If lock type is ``RECORD``, page number of the locked record
   :column LOCK_REC: If lock type is ``RECORD``, heap number of the locked record
   :column LOCK_DATA: If lock type is ``RECORD``, primary key of the locked record

This table contains information on each lock that is requested by any transaction and on each lock that is held by any transaction. You can get details about the transactions involved by joining INNODB_LOCKS with INNODB_TRX on TRX_ID.

This table was introduced by the |InnoDB| plugin and you can find the full documentation here.

.. table:: INFORMATION_SCHEMA.INNODB_TRX

   :column TRX_ID: |InnoDB| internal unique transaction id
   :column TRX_STATE: Execution state. Possible values are: ``Running``, ``Lock_wait``, ``Rolling_back``, ``Committing``
   :column TRX_STARTED:  Transaction start time
   :column TRX_REQUESTED_LOCK_ID: Id of the lock the transaction has requested
   :column TRX_WAIT_STARTED: Date and time when the transaction started waiting for a lock
   :column TRX_WEIGHT: Weight of the transaction, ie approximate number of locked and modified rows
   :column TRX_MYSQL_THREAD_ID: Thread id
   :column TRX_QUERY: SQL query being executed

This table holds information on every transaction running in the |InnoDB| kernel. Contrary to the output of SHOW |InnoDB| STATUS, it doesn``t show information on idle transactions.

This table was introduced by the |InnoDB| plugin and you can find the full documentation here.

Temporary tables
================

.. table:: INFORMATION_SCHEMA.GLOBAL_TEMPORARY_TABLES

   :column SESSION_ID: |MySQL| connection id
   :column TABLE_SCHEMA: Schema in which the temporary table is created
   :column TABLE_NAME: Name of the temporary table
   :column ENGINE: Engine of the temporary table
   :column NAME: Internal name of the temporary table
   :column TABLE_ROWS: Number of rows of the temporary table
   :column AVG_ROW_LENGTH: Average row length of the temporary table
   :column DATA_LENGTH: Size of the data (Bytes)
   :column INDEX_LENGTH: Size of the indexes (Bytes)
   :column CREATE_TIME: Date and time of creation of the temporary table
   :column UPDATE_TIME: Date and time of the latest update of the temporary table

This table holds information on the temporary tables existing for all connections. You don't need the ``SUPER`` privilege to query this table.

.. table:: INFORMATION_SCHEMA.TEMPORARY_TABLES

   :column SESSION_ID: |MySQL| connection id
   :column TABLE_SCHEMA: Schema in which the temporary table is created
   :column TABLE_NAME: Name of the temporary table
   :column ENGINE: Engine of the temporary table
   :column NAME: Internal name of the temporary table
   :column TABLE_ROWS: Number of rows of the temporary table
   :column AVG_ROW_LENGTH: Average row length of the temporary table
   :column DATA_LENGTH: Size of the data (Bytes)
   :column INDEX_LENGTH: Size of the indexes (Bytes)
   :column CREATE_TIME: Date and time of creation of the temporary table
   :column UPDATE_TIME: Date and time of the latest update of the temporary table

This table holds information on the temporary tables existing for the running connection.
