.. _tokudb_troubleshooting:

======================
TokuDB Troubleshooting
======================

.. contents::
   :local:
   :depth: 1

.. _tokudb_known_issues:

Known Issues
------------

**Replication and binary logging**: |TokuDB| supports binary logging and replication, with one restriction. |TokuDB| does not implement a lock on the auto-increment function, so concurrent insert statements with one or more of the statements inserting multiple rows may result in a non-deterministic interleaving of the auto-increment values. When running replication with these concurrent inserts, the auto-increment values on the slave table may not match the auto-increment values on the master table. Note that this is only an issue with Statement Based Replication (SBR), and not Row Based Replication (RBR).

For more information about auto-increment and replication, see the |MySQL| Reference Manual: `AUTO_INCREMENT handling in InnoDB <http://dev.mysql.com/doc/refman/5.5/en/innodb-auto-increment-handling.html>`_.

In addition, when using the ``REPLACE INTO`` or ``INSERT IGNORE`` on tables with no secondary indexes or tables where secondary indexes are subsets of the primary, the session variable :variable:`tokudb_pk_insert_mode` controls whether row based replication will work.

**Uninformative error message**: The ``LOAD DATA INFILE`` command can sometimes produce ``ERROR 1030 (HY000): Got error 1 from storage engine``. The message should say that the error is caused by insufficient disk space for the temporary files created by the loader.

**Transparent Huge Pages**: |TokuDB| will refuse to start if transparent huge pages are enabled. Transparent huge page support can be disabled by issuing the following as root:

.. code-block:: console

 # echo never > /sys/kernel/mm/redhat_transparent_hugepage/enabled

.. note:: The previous command needs to be executed after every reboot, because it defaults to ``always``.

**XA behavior vs. InnoDB**: |InnoDB| forces a deadlocked XA transaction to abort, |TokuDB| does not.

.. _tokudb_lock_visualization:

Lock Visualization in TokuDB
----------------------------

|TokuDB| uses key range locks to implement serializable transactions, which are acquired as the transaction progresses. The locks are released when the transaction commits or aborts (this implements two phase locking).

|TokuDB| stores these locks in a data structure called the lock tree. The lock tree stores the set of range locks granted to each transaction. In addition, the lock tree stores the set of locks that are not granted due to a conflict with locks granted to some other transaction. When these other transactions are retired, these pending lock requests are retried. If a pending lock request is not granted before the lock timer expires, then the lock request is aborted.

Lock visualization in |TokuDB| exposes the state of the lock tree with tables in the information schema. We also provide a mechanism that may be used by a database client to retrieve details about lock conflicts that it encountered while executing a transaction.

The ``TOKUDB_TRX`` table
************************

The :table:`TOKUDB_TRX` table in the ``INFORMATION_SCHEMA`` maps |TokuDB| transaction identifiers to |MySQL| client identifiers. This mapping allows one to associate a |TokuDB| transaction with a |MySQL| client operation.

The following query returns the |MySQL| clients that have a live |TokuDB| transaction:

.. code-block:: mysql

 SELECT * FROM INFORMATION_SCHEMA.TOKUDB_TRX,
   INFORMATION_SCHEMA.PROCESSLIST
   WHERE trx_mysql_thread_id = id;

The ``TOKUDB_LOCKS`` table
**************************

The :table:`tokudb_locks` table in the information schema contains the set of locks granted to |TokuDB| transactions.

The following query returns all of the locks granted to some |TokuDB| transaction:

.. code-block:: mysql

 SELECT * FROM INFORMATION_SCHEMA.TOKUDB_LOCKS;

The following query returns the locks granted to some |MySQL| client:

.. code-block:: mysql

 SELECT id FROM INFORMATION_SCHEMA.TOKUDB_LOCKS,
   INFORMATION_SCHEMA.PROCESSLIST
   WHERE locks_mysql_thread_id = id;

The ``TOKUDB_LOCK_WAITS`` table
*******************************

The :table:`tokudb_lock_waits` table in the information schema contains the set of lock requests that are not granted due to a lock conflict with some other transaction.

The following query returns the locks that are waiting to be granted due to a lock conflict with some other transaction:

.. code-block:: mysql

 SELECT * FROM INFORMATION_SCHEMA.TOKUDB_LOCK_WAITS;

The :variable:`tokudb_lock_timeout_debug` session variable
**********************************************************

The :variable:`tokudb_lock_timeout_debug` session variable controls how lock timeouts and lock deadlocks seen by the database client are reported.

The following values are available:

:0: No lock timeouts or lock deadlocks are reported.

:1: A JSON document that describes the lock conflict is stored in the :variable:`tokudb_last_lock_timeout` session variable

:2: A JSON document that describes the lock conflict is printed to the |MySQL| error log.

  *Supported since 7.5.5*: In addition to the JSON document describing the lock conflict, the following lines are printed to the MySQL error log:

  * A line containing the blocked thread id and blocked SQL
  * A line containing the blocking thread id and the blocking SQL.

:3: A JSON document that describes the lock conflict is stored in the :variable:`tokudb_last_lock_timeout` session variable and is printed to the |MySQL| error log.

  *Supported since 7.5.5*: In addition to the JSON document describing the lock conflict, the following lines are printed to the |MySQL| error log:

  * A line containing the blocked thread id and blocked SQL
  * A line containing the blocking thread id and the blocking SQL.

The :variable:`tokudb_last_lock_timeout` session variable
*********************************************************

The :variable:`tokudb_last_lock_timeout` session variable contains a JSON document that describes the last lock conflict seen by the current |MySQL| client. It gets set when a blocked lock request times out or a lock deadlock is detected. The :variable:`tokudb_lock_timeout_debug` session variable should have bit ``0`` set (decimal ``1``).

Example
*******

Suppose that we create a table with a single column that is the primary key.

.. code-block:: mysql

 mysql> SHOW CREATE TABLE table;

 Create Table: CREATE TABLE ‘table‘ (
 ‘id‘ int(11) NOT NULL,
 PRIMARY KEY (‘id‘)) ENGINE=TokuDB DEFAULT CHARSET=latin1

Suppose that we have 2 |MySQL| clients with ID's 1 and 2 respectively. Suppose that |MySQL| client 1 inserts some values into ``table``. |TokuDB| transaction 51 is created for the insert statement. Since autocommit is disabled, transaction 51 is still live after the insert statement completes, and we can query the :table:`tokudb_locks` table in information schema to see the locks that are held by the transaction.

.. code-block:: mysql

 mysql> SET AUTOCOMMIT=OFF;
 mysql> INSERT INTO table VALUES (1),(10),(100);

 Query OK, 3 rows affected (0.00 sec)
 Records: 3  Duplicates: 0  Warnings: 0

 mysql> SELECT * FROM INFORMATION_SCHEMA.TOKUDB_LOCKS;

 +--------------+-----------------------+---------------+----------------+-----------------+--------------------+------------------+-----------------------------+
 | locks_trx_id | locks_mysql_thread_id | locks_dname   | locks_key_left | locks_key_right | locks_table_schema | locks_table_name | locks_table_dictionary_name |
 +--------------+-----------------------+---------------+----------------+-----------------+--------------------+------------------+-----------------------------+
 |           51 |                     1 | ./test/t-main | 0001000000     | 0001000000      | test               | t                | main                        |
 |           51 |                     1 | ./test/t-main | 000a000000     | 000a000000      | test               | t                | main                        |
 |           51 |                     1 | ./test/t-main | 0064000000     | 0064000000      | test               | t                | main                        |
 +--------------+-----------------------+---------------+----------------+-----------------+--------------------+------------------+-----------------------------+
 
 mysql> SELECT * FROM INFORMATION_SCHEMA.TOKUDB_LOCK_WAITS;

 Empty set (0.00 sec)

The keys are currently hex dumped.

Now we switch to the other |MySQL| client with ID 2.

.. code-block:: mysql

 mysql> INSERT INTO table VALUES (2),(20),(100);

The insert gets blocked since there is a conflict on the primary key with value 100.

The granted |TokuDB| locks are:

.. code-block:: mysql

 mysql> SELECT * FROM INFORMATION_SCHEMA.TOKUDB_LOCKS;

 +--------------+-----------------------+---------------+----------------+-----------------+--------------------+------------------+-----------------------------+
 | locks_trx_id | locks_mysql_thread_id | locks_dname   | locks_key_left | locks_key_right | locks_table_schema | locks_table_name | locks_table_dictionary_name |
 +--------------+-----------------------+---------------+----------------+-----------------+--------------------+------------------+-----------------------------+
 |           51 |                     1 | ./test/t-main | 0001000000     | 0001000000      | test               | t                | main                        |
 |           51 |                     1 | ./test/t-main | 000a000000     | 000a000000      | test               | t                | main                        |
 |           51 |                     1 | ./test/t-main | 0064000000     | 0064000000      | test               | t                | main                        |
 |           51 |                     1 | ./test/t-main | 0002000000     | 0002000000      | test               | t                | main                        |
 |           51 |                     1 | ./test/t-main | 0014000000     | 0014000000      | test               | t                | main                        |
 +--------------+-----------------------+---------------+----------------+-----------------+--------------------+------------------+-----------------------------+

The locks that are pending due to a conflict are:

.. code-block:: mysql

 SELECT * FROM INFORMATION_SCHEMA.TOKUDB_LOCK_WAITS;

 +-------------------+-----------------+------------------+---------------------+----------------------+-----------------------+--------------------+------------------+-----------------------------+
 | requesting_trx_id | blocking_trx_id | lock_waits_dname | lock_waits_key_left | lock_waits_key_right | lock_waits_start_time | locks_table_schema | locks_table_name | locks_table_dictionary_name |
 +-------------------+-----------------+------------------+---------------------+----------------------+-----------------------+--------------------+------------------+-----------------------------+
 |                62 |              51 | ./test/t-main    | 0064000000          | 0064000000           |         1380656990910 | test               | t                | main                        |
 +-------------------+-----------------+------------------+---------------------+----------------------+-----------------------+--------------------+------------------+-----------------------------+

Eventually, the lock for client 2 times out, and we can retrieve a JSON document that describes the conflict.

.. code-block:: mysql

 ERROR 1205 (HY000): Lock wait timeout exceeded; try restarting transaction

 mysql> SELECT @@TOKUDB_LAST_LOCK_TIMEOUT;

 +---------------------------------------------------------------------------------------------------------------+
 | @@tokudb_last_lock_timeout                                                                                    |
 +---------------------------------------------------------------------------------------------------------------+
 | "mysql_thread_id":2, "dbname":"./test/t-main", "requesting_txnid":62, "blocking_txnid":51, "key":"0064000000" |
 +---------------------------------------------------------------------------------------------------------------+

 ROLLBACK;

Since transaction 62 was rolled back, all of the locks taken by it are released.

.. code-block:: mysql

 mysql> SELECT * FROM INFORMATION_SCHEMA.TOKUDB_LOCKS;

 +--------------+-----------------------+---------------+----------------+-----------------+--------------------+------------------+-----------------------------+
 | locks_trx_id | locks_mysql_thread_id | locks_dname   | locks_key_left | locks_key_right | locks_table_schema | locks_table_name | locks_table_dictionary_name |
 +--------------+-----------------------+---------------+----------------+-----------------+--------------------+------------------+-----------------------------+
 |           51 |                     1 | ./test/t-main | 0001000000     | 0001000000      | test               | t                | main                        |
 |           51 |                     1 | ./test/t-main | 000a000000     | 000a000000      | test               | t                | main                        |
 |           51 |                     1 | ./test/t-main | 0064000000     | 0064000000      | test               | t                | main                        |
 |           51 |                     2 | ./test/t-main | 0002000000     | 0002000000      | test               | t                | main                        |
 |           51 |                     2 | ./test/t-main | 0014000000     | 0014000000      | test               | t                | main                        |
 +--------------+-----------------------+---------------+----------------+-----------------+--------------------+------------------+-----------------------------+

Engine Status
-------------

Engine status provides details about the inner workings of |TokuDB| and can be useful in tuning your particular environment. The engine status can be determined by running the following command:

.. code-block:: mysql

 SHOW ENGINE tokudb STATUS;

The following is a reference of table status statements:

**cachetable: cleaner executions**
 Total number of times the cleaner thread loop has executed.

**cachetable: cleaner iterations**
 This is the number of cleaner operations that are performed every cleaner period.

**cachetable: cleaner period**
 |TokuDB| includes a cleaner thread that optimizes indexes in the background. This variable is the time, in seconds, between the completion of a group of cleaner operations and the beginning of the next group of cleaner operations. The cleaner operations run on a background thread performing work that does not need to be done on the client thread.

**cachetable: evictions**
 Number of blocks evicted from cache.

**cachetable: long time waiting on cache pressure**
 Total time, in microseconds, waiting on cache pressure to subside for more than 1 second.

**cachetable: miss**
 This is a count of how many times the application was unable to access your data in the internal cache.

**cachetable: miss time**
 This is the total time, in microseconds, of how long the database has had to wait for a disk read to complete.

**cachetable: number of long waits on cache pressure**
 The number of times a thread was stalled for more than 1 second due to cache pressure.

**cachetable: number of waits on cache pressure**
 The number of times a thread was stalled due to cache pressure.

**cachetable: prefetches**
 This is the total number of times that a block of memory has been prefetched into the database's cache. Data is prefetched when the database's algorithms determine that a block of memory is likely to be accessed by the application.

**cachetable: size cachepressure**
 The number of bytes causing cache pressure (the sum of buffers and work done counters), helps to understand if cleaner threads are keeping up with workload.

**cachetable: size current**
 This is a count, in bytes, of how much of your uncompressed data is currently in the database's internal cache.

**cachetable: size currently cloned data for checkpoint**
 Amount of memory, in bytes, currently used for cloned nodes. During the checkpoint operation, dirty nodes are cloned prior to serialization/compression, then written to disk. After which, the memory for the cloned block is returned for re-use.

**cachetable: size leaf**
 The number of bytes of leaf nodes in the cache.

**cachetable: size limit**
 This is a count, in bytes, of how much of your uncompressed data will fit in the database's internal cache.

**cachetable: size nonleaf**
 The number of bytes of non-leaf nodes in the cache.

**cachetable: size rollback**
 The number of bytes of rollback nodes in the cache.

**cachetable: size writing**
 This is the number of bytes that are currently queued up to be written to disk.

**cachetable: time waiting on cache pressure**
 Total time, in microseconds, waiting on cache pressure to subside.

**checkpoint: begin time**
 Cumulative time (in microseconds) required to mark all dirty nodes as pending a checkpoint.

**checkpoint: checkpoints failed**
 This is the number of checkpoints that have failed for any reason.

**checkpoint: checkpoints taken**
 This is the number of complete checkpoints that have been taken.

**checkpoint: footprint**
 Where the database is in the checkpoint process.

**checkpoint: last checkpoint began**
 This is the time the last checkpoint began. If a checkpoint is currently in progress, then this time may be later than the time the last checkpoint completed.

 .. note:: 
 
   If no checkpoint has ever taken place, then this value will be ``Dec 31, 1969`` on Linux hosts.

**checkpoint: last complete checkpoint began**
 This is the time the last complete checkpoint started. Any data that changed after this time will not be captured in the checkpoint.

**checkpoint: last complete checkpoint ended**
 This is the time the last complete checkpoint ended.

**checkpoint: last complete checkpoint LSN**
 This is the Log Sequence Number of the last complete checkpoint.

**checkpoint: long checkpoint begin count**
 The total number of times a checkpoint begin took more than 1 second.

**checkpoint: long checkpoint begin time**
 The total time, in microseconds, of long checkpoint begins. A long checkpoint begin is one taking more than 1 second.

**checkpoint: non-checkpoint client wait on cs lock**
 The number of times a non-checkpoint client thread waited for the checkpoint-safe lock.

**checkpoint: non-checkpoint client wait on mo lock**
 The number of times a non-checkpoint client thread waited for the multi-operation lock.

**checkpoint: period**
 This is the interval in seconds between the end of an automatic checkpoint and the beginning of the next automatic checkpoint.

**checkpoint: time spent during checkpoint (begin and end phases)**
 Time (in seconds) required to complete all checkpoints.

**checkpoint: time spent during last checkpoint (begin and end phases)**
 Time (in seconds) required to complete the last checkpoint.

**checkpoint: waiters max**
 This is the maximum number of threads ever simultaneously waiting for the checkpoint-safe lock to perform a checkpoint.

**checkpoint: waiters now**
 This is the current number of threads simultaneously waiting for the checkpoint-safe lock to perform a checkpoint.

**checkpoint: checkpoint end time**
 The time spent in checkpoint end operation in seconds.
 
**checkpoint: long checkpoint end time**
 The time spent in checkpoint end operation in seconds.
 
**checkpoint: long checkpoint end count**
 This is the count of end_checkpoint operations that exceeded 1 minute.

**context: promotion blocked by a flush**
 Number of times node ``rwlock`` contention was observed within promotion (pinning nodes from root to the buffer to receive the message) because of a buffer flush from parent to child.

**context: promotion blocked by a full eviction (should never happen)**
 Number of times node ``rwlock`` contention was observed within promotion (pinning nodes from root to the buffer to receive the message) because of a full eviction.

**context: promotion blocked by a full fetch (should never happen)**
 Number of times node ``rwlock`` contention was observed within promotion (pinning nodes from root to the buffer to receive the message) because of a full fetch.

**context: promotion blocked by a message application**
 Number of times node ``rwlock`` contention was observed within promotion (pinning nodes from root to the buffer to receive the message) because of message application (applying fresh ancestors messages to a basement node).

**context: promotion blocked by a message injection**
 Number of times node ``rwlock`` contention was observed within promotion (pinning nodes from root to the buffer to receive the message) because of message injection.

**context: promotion blocked by a partial eviction (should never happen)**
 Number of times node ``rwlock`` contention was observed within promotion (pinning nodes from root to the buffer to receive the message) because of a partial eviction.

**context: promotion blocked by a partial fetch (should never happen)**
 Number of times node ``rwlock`` contention was observed within promotion (pinning nodes from root to the buffer to receive the message) because of a partial fetch.

**context: promotion blocked by something uninstrumented**
 Number of times node ``rwlock`` contention was observed within promotion (pinning nodes from root to the buffer to receive the message) because of something uninstrumented.

**context: promotion blocked by the cleaner thread**
 Number of times node ``rwlock`` contention was observed within promotion (pinning nodes from root to the buffer to receive the message) because of a cleaner thread.

**context: something uninstrumented blocked by something uninstrumented**
 Number of times node ``rwlock`` contention was observed for an uninstrumented process because of something uninstrumented.

**context: tree traversals blocked by a flush**
 Number of times node ``rwlock`` contention was observed while pinning nodes from root to leaf because of a buffer flush from parent to child.

**context: tree traversals blocked by a full eviction**
 Number of times node ``rwlock`` contention was observed while pinning nodes from root to leaf because of a full eviction.

**context: tree traversals blocked by a full fetch**
 Number of times node ``rwlock`` contention was observed while pinning nodes from root to leaf because of a full fetch.

**context: tree traversals blocked by a message application**
 Number of times node ``rwlock`` contention was observed while pinning nodes from root to leaf because of message application (applying fresh ancestors messages to a basement node).

**context: tree traversals blocked by a message injection**
 Number of times node ``rwlock`` contention was observed while pinning nodes from root to leaf because of message injection.

**context: tree traversals blocked by a partial eviction**
 Number of times node ``rwlock`` contention was observed while pinning nodes from root to leaf because of a partial eviction.

**context: tree traversals blocked by a partial fetch**
 Number of times node ``rwlock`` contention was observed while pinning nodes from root to leaf because of a partial fetch.

**context: tree traversals blocked by a the cleaner thread**
 Number of times node ``rwlock`` contention was observed while pinning nodes from root to leaf because of a cleaner thread.

**context: tree traversals blocked by something uninstrumented**
 Number of times node ``rwlock`` contention was observed while pinning nodes from root to leaf because of something uninstrumented.

**db closes**
 Number of db close operations.

**db opens**
 Number of db open operations.

**dictionary broadcast updates**
 This is the number of broadcast updates that have been successfully performed. A broadcast update is an update that affects all rows in a dictionary.

**dictionary broadcast updates fail**
 This is the number of broadcast updates that have failed.

**dictionary deletes**
 This is the total number of rows that have been deleted from all primary and secondary indexes combined, if those deletes have been done with a separate recovery log entry per index.

**dictionary deletes fail**
 This is the number of single-index delete operations that failed.

**dictionary inserts**
 This is the total number of rows that have been inserted into all primary and secondary indexes combined, when those inserts have been done with a separate recovery log entry per index. For example, inserting a row into a table with one primary and two secondary indexes will increase this count by three, if the inserts were done with separate recovery log entries.

**dictionary inserts fail**
 This is the number of single-index insert operations that failed.

**dictionary multi deletes**
 This is the total number of rows that have been deleted from all primary and secondary indexes combined, when those deletes have been done with a single recovery log entry for the entire row.

**dictionary multi deletes fail**
 This is the number of multi-index delete operations that failed.

**dictionary multi inserts**
 This is the total number of rows that have been inserted into all primary and secondary indexes combined, when those inserts have been done with a single recovery log entry for the entire row. (For example, inserting a row into a table with one primary and two secondary indexes will normally increase this count by three).

**dictionary multi inserts fail**
 This is the number of multi-index insert operations that failed.

**dictionary multi updates**
 This is the total number of rows that have been updated in all primary and secondary indexes combined, if those updates have been done with a single recovery log entry for the entire row.

**dictionary multi updates fail**
 This is the number of multi-index update operations that failed.

**dictionary updates**
 This is the total number of rows that have been updated in all primary and secondary indexes combined, if those updates have been done with a separate recovery log entry per index.

**dictionary updates fail**
 This is the number of single-index update operations that failed.

**disk free space**
 This is a gross estimate of how much of your file system is available. Possible displays in this field are:
 
 * More than twice the reserve ("more than 10 percent of total file system space")
 * Less than twice the reserve
 * Less than the reserve
 * File system is completely full

**filesystem: ENOSPC redzone state**
 The state of how much disk space exists with respect to the red zone value. Valid values are:

 :0: Space is available
 :1: Warning, with 2x of redzone value. Operations are allowed, but engine status prints a warning.
 :2: In red zone, insert operations are blocked
 :3: All operations are blocked

**filesystem: fsync count**
 This is the total number of times the database has flushed the operating system's file buffers to disk.

**filesystem: fsync time**
 This the total time, in microseconds, used to fsync to disk.

**filesystem: long fsync count**
 This is the total number of times the database has flushed the operating system's file buffers to disk and this operation required more than 1 second.

**filesystem: long fsync time**
 This the total time, in microseconds, used to fsync to disk when the operation required more than 1 second.

**filesystem: most recent disk full**
 This is the most recent time when the disk file system was entirely full. If the disk has never been full, then this value will be "Dec 31, 1969" on Linux hosts.

**filesystem: number of operations rejected by enospc prevention (red zone)**
 This is the number of database inserts that have been rejected because the amount of disk free space was less than the reserve.

**filesystem: number of write operations that returned ENOSPC**
 This is the number of times that an attempt to write to disk failed because the disk was full. If the disk is full, this number will continue increasing until space is available.

**filesystem: threads currently blocked by full disk**
 This is the number of threads that are currently blocked because they are attempting to write to a full disk. This is normally zero. If this value is non-zero, then a warning will appear in the "disk free space" field.

**ft: basements decompressed as a target of a query**
 Number of basement nodes decompressed for queries.

**ft: basements decompressed for prefetch**
 Number of basement nodes decompressed by a prefetch thread.

**ft: basements decompressed for prelocked range**
 Number of basement nodes decompressed by queries aggressively.

**ft: basements decompressed for write**
 Number of basement nodes decompressed for writes.

**ft: basement nodes deserialized with fixed-keysize**
 The number of basement nodes deserialized where all keys had the same size, leaving the basement in a format that is optimal for in-memory workloads.

**ft: basement nodes deserialized with variable-keysize**
 The number of basement nodes deserialized where all keys did not have the same size, and thus ineligible for an in-memory optimization.

**ft: basements fetched as a target of a query (bytes)**
 Number of basement node bytes fetched from disk for queries.

**ft: basements fetched as a target of a query**
 Number of basement nodes fetched from disk for queries.

**ft: basements fetched as a target of a query (seconds)**
 Number of seconds waiting for IO when fetching basement nodes from disk for queries.

**ft: basements fetched for prefetch (bytes)**
 Number of basement node bytes fetched from disk by a prefetch thread.

**ft: basements fetched for prefetch**
 Number of basement nodes fetched from disk by a prefetch thread.

**ft: basements fetched for prefetch (seconds)**
 Number of seconds waiting for IO when fetching basement nodes from disk by a prefetch thread.

**ft: basements fetched for prelocked range (bytes)**
 Number of basement node bytes fetched from disk aggressively.

**ft: basements fetched for prelocked range**
 Number of basement nodes fetched from disk aggressively.

**ft: basements fetched for prelocked range (seconds)**
 Number of seconds waiting for IO when fetching basement nodes from disk aggressively.

**ft: basements fetched for write (bytes)**
 Number of basement node bytes fetched from disk for writes.

**ft: basements fetched for write**
 Number of basement nodes fetched from disk for writes.

**ft: basements fetched for write (seconds)**
 Number of seconds waiting for IO when fetching basement nodes from disk for writes.

**ft: broadcast messages injected at root**
 How many broadcast messages injected at root.

**ft: buffers decompressed as a target of a query**
 Number of buffers decompressed for queries.

**ft: buffers decompressed for prefetch**
 Number of buffers decompressed by a prefetch thread.

**ft: buffers decompressed for prelocked range**
 Number of buffers decompressed by queries aggressively.

**ft: buffers decompressed for write**
 Number of buffers decompressed for writes.

**ft: buffers fetched as a target of a query (bytes)**
 Number of buffer bytes fetched from disk for queries.

**ft: buffers fetched as a target of a query**
 Number of buffers fetched from disk for queries.

**ft: buffers fetched as a target of a query (seconds)**
 Number of seconds waiting for IO when fetching buffers from disk for queries.

**ft: buffers fetched for prefetch (bytes)**
 Number of buffer bytes fetched from disk by a prefetch thread.

**ft: buffers fetched for prefetch**
 Number of buffers fetched from disk by a prefetch thread.

**ft: buffers fetched for prefetch (seconds)**
 Number of seconds waiting for IO when fetching buffers from disk by a prefetch thread.

**ft: buffers fetched for prelocked range (bytes)**
 Number of buffer bytes fetched from disk aggressively.

**ft: buffers fetched for prelocked range**
 Number of buffers fetched from disk aggressively.

**ft: buffers fetched for prelocked range (seconds)**
 Number of seconds waiting for IO when fetching buffers from disk aggressively.

**ft: buffers fetched for write (bytes)**
 Number of buffer bytes fetched from disk for writes.

**ft: buffers fetched for write**
 Number of buffers fetched from disk for writes.

**ft: buffers fetched for write (seconds)**
 Number of seconds waiting for IO when fetching buffers from disk for writes.

**ft: bytes of messages currently in trees (estimate)**
 How many bytes of messages currently in trees (estimate).

**ft: bytes of messages flushed from h1 nodes to leaves**
 How many bytes of messages flushed from h1 nodes to leaves.

**ft: bytes of messages injected at root (all trees)**
 How many bytes of messages injected at root (for all trees).

**ft: descriptor set**
 This is the number of time a descriptor was updated when the entire dictionary was updated (for example, when the schema has been changed).

**ft: leaf compression to memory (seconds)**
 Total time, in seconds, spent compressing leaf nodes.

**ft: leaf decompression to memory (seconds)**
 Total time, in seconds, spent decompressing leaf nodes.

**ft: leaf deserialization to memory (seconds)**
 Total time, in seconds, spent deserializing leaf nodes.

**ft: leaf node full evictions (bytes)**
 The number of bytes freed by evicting full leaf nodes from the cache.

**ft: leaf node full evictions**
 The number of times a full leaf node was evicted from the cache.

**ft: leaf node partial evictions (bytes)**
 The number of bytes freed by evicting partitions of leaf nodes from the cache.

**ft: leaf node partial evictions**
 The number of times a partition of a leaf node was evicted from the cache.

**ft: leaf nodes created**
 Number of leaf nodes created.

**ft: leaf nodes destroyed**
 Number of leaf nodes destroyed.

**ft: leaf nodes flushed to disk (for checkpoint) (bytes)**
 Number of bytes of leaf nodes flushed to disk for checkpoint.

**ft: leaf nodes flushed to disk (for checkpoint)**
 Number of leaf nodes flushed to disk for checkpoint.

**ft: leaf nodes flushed to disk (for checkpoint) (seconds)**
 Number of seconds waiting for IO when writing leaf nodes flushed to disk for checkpoint.

**ft: leaf nodes flushed to disk (for checkpoint) (uncompressed bytes)**
 Number of uncompressed bytes of leaf nodes flushed to disk for checkpoint.

**ft: leaf nodes flushed to disk (not for checkpoint) (bytes)**
 Number of bytes of leaf nodes flushed to disk, not for checkpoint.

**ft: leaf nodes flushed to disk (not for checkpoint)**
 Number of leaf nodes flushed to disk, not for checkpoint.

**ft: leaf nodes flushed to disk (not for checkpoint) (seconds)**
 Number of seconds waiting for IO when writing leaf nodes flushed to disk, not for checkpoint.

**ft: leaf nodes flushed to disk (not for checkpoint) (uncompressed bytes)**
 Number of bytes of leaf nodes flushed to disk, not for checkpoint.

**ft: leaf serialization to memory (seconds)**
 Total time, in seconds, spent serializing leaf nodes.

**ft: messages ignored by leaf due to msn**
 The number of messages that were ignored by a leaf because it had already been applied.

**ft: messages injected at root**
 How many messages injected at root.

**ft: nonleaf compression to memory (seconds)**
 Total time, in seconds, spent compressing non leaf nodes.

**ft: nonleaf decompression to memory (seconds)**
 Total time, in seconds, spent decompressing non leaf nodes.

**ft: nonleaf deserialization to memory (seconds)**
 Total time, in seconds, spent deserializing non leaf nodes.

**ft: nonleaf node full evictions (bytes)**
 The number of bytes freed by evicting full nonleaf nodes from the cache.

**ft: nonleaf node full evictions**
 The number of times a full nonleaf node was evicted from the cache.

**ft: nonleaf node partial evictions (bytes)**
 The number of bytes freed by evicting partitions of nonleaf nodes from the cache.

**ft: nonleaf node partial evictions**
 The number of times a partition of a nonleaf node was evicted from the cache.

**ft: nonleaf nodes created**
 Number of nonleaf nodes created.

**ft: nonleaf nodes destroyed**
 Number of nonleaf nodes destroyed.

**ft: nonleaf nodes flushed to disk (for checkpoint) (bytes)**
 Number of bytes of nonleaf nodes flushed to disk for checkpoint.

**ft: nonleaf nodes flushed to disk (for checkpoint)**
 Number of nonleaf nodes flushed to disk for checkpoint.

**ft: nonleaf nodes flushed to disk (for checkpoint) (seconds)**
 Number of seconds waiting for IO when writing nonleaf nodes flushed to disk for checkpoint.

**ft: nonleaf nodes flushed to disk (for checkpoint) (uncompressed bytes)**
 Number of uncompressed bytes of nonleaf nodes flushed to disk for checkpoint.

**ft: nonleaf nodes flushed to disk (not for checkpoint) (bytes)**
 Number of bytes of nonleaf nodes flushed to disk, not for checkpoint.

**ft: nonleaf nodes flushed to disk (not for checkpoint)**
 Number of nonleaf nodes flushed to disk, not for checkpoint.

**ft: nonleaf nodes flushed to disk (not for checkpoint) (seconds)**
 Number of seconds waiting for IO when writing nonleaf nodes flushed to disk, not for check- point.

**ft: nonleaf nodes flushed to disk (not for checkpoint) (uncompressed bytes)**
 Number of uncompressed bytes of nonleaf nodes flushed to disk, not for checkpoint.

**ft: nonleaf serialization to memory (seconds)**
 Total time, in seconds, spent serializing non leaf nodes.

**ft: pivots fetched for prefetch (bytes)**
 Number of bytes of pivot nodes fetched by a prefetch thread.

**ft: pivots fetched for prefetch**
 Number of pivot nodes fetched by a prefetch thread.

**ft: pivots fetched for prefetch (seconds)**
 Number seconds waiting for IO when fetching pivot nodes by a prefetch thread.

**ft: pivots fetched for query (bytes)**
 Number of bytes of pivot nodes fetched for queries.

**ft: pivots fetched for query**
 Number of pivot nodes fetched for queries.

**ft: pivots fetched for query (seconds)**
 Number of seconds waiting for IO when fetching pivot nodes for queries.

**ft: pivots fetched for write (bytes)**
 Number of bytes of pivot nodes fetched for writes.

**ft: pivots fetched for write**
 Number of pivot nodes fetched for writes.

**ft: pivots fetched for write (seconds)**
 Number of seconds waiting for IO when fetching pivot nodes for writes.

**ft: promotion: h1 roots injected into**
 Number of times a message stopped at a root with height 1.

**ft: promotion: injections at depth 0**
 Number of times a message stopped at depth 0.

**ft: promotion: injections at depth 1**
 Number of times a message stopped at depth 1.

**ft: promotion: injections at depth 2**
 Number of times a message stopped at depth 2.

**ft: promotion: injections at depth 3**
 Number of times a message stopped at depth 3.

**ft: promotion: injections lower than depth 3**
 Number of times a message was promoted past depth 3.

**ft: promotion: leaf roots injected into**
 Number of times a message stopped at a root with height 0.

**ft: promotion: roots split**
 Number of times the root split during promotion.

**ft: promotion: stopped anyway, after locking the child**
 Number of times a message stopped before a child which had been locked.

**ft: promotion: stopped at height 1**
 Number of times a message stopped because it had reached height 1.

**ft: promotion: stopped because of a nonempty buffer**
 Number of times a message stopped because it reached a nonempty buffer.

**ft: promotion: stopped because the child was locked or not at all in memory**
 Number of times a message stopped because it could not cheaply get access to a child.

**ft: promotion: stopped because the child was not fully in memory**
 Number of times a message stopped because it could not cheaply get access to a child.

**ft: promotion: succeeded in using the rightmost leaf shortcut**
 Rightmost insertions used the rightmost-leaf pin path, meaning that the Fractal Tree index detected and properly optimized rightmost inserts.

**ft: promotion: tried the rightmost leaf shortcut but failed (child reactive)**
 Rightmost insertions did not use the rightmost-leaf pin path, due to the leaf being too large (needed to split).

**ft: promotion: tried the rightmost leaf shortcut but failed (out-of-bounds)**
 Rightmost insertions did not use the rightmost-leaf pin path, due to the insert not actually being into the rightmost leaf node.

**ft: searches requiring more tries than the height of the tree**
 Number of searches that required more tries than the height of the tree.

**ft: searches requiring more tries than the height of the tree plus three**
 Number of searches that required more tries than the height of the tree plus three.

**ft: total search retries due to TRY AGAIN**
 Total number of search retries due to TRY AGAIN.

**ft: uncompressed / compressed bytes written (leaf)**
 Ratio of uncompressed bytes (in-memory) to compressed bytes (on-disk) for leaf nodes.

**ft: uncompressed / compressed bytes written (nonleaf)**
 Ratio of uncompressed bytes (in-memory) to compressed bytes (on-disk) for nonleaf nodes.

**ft: uncompressed / compressed bytes written (overall)**
 Ratio of uncompressed bytes (in-memory) to compressed bytes (on-disk) for all nodes.

**ft flusher: cleaner thread leaf merges in progress**
 The number of cleaner thread leaf merges in progress.

**ft flusher: cleaner thread leaf merges successful**
 The number of times the cleaner thread successfully merges a leaf.

**ft flusher: height-greater-than-one nodes flushed by cleaner thread**
 Number of nodes of height > 1 whose message buffers are flushed by cleaner thread.

**ft flusher: height-one nodes flushed by cleaner thread**
 Number of nodes of height one whose message buffers are flushed by cleaner thread.

**ft flusher: leaf node balances**
 Number of times a leaf node is balanced.

**ft flusher: leaf node merges**
 Number of times leaf nodes are merged.

**ft flusher: leaf node splits**
 Number of leaf nodes split.

**ft flusher: max bytes in a buffer flushed by cleaner thread**
 Max number of bytes in message buffer flushed by cleaner thread.

**ft flusher: max workdone in a buffer flushed by cleaner thread**
 Max workdone value of any message buffer flushed by cleaner thread.

**ft flusher: min bytes in a buffer flushed by cleaner thread**
 Min number of bytes in message buffer flushed by cleaner thread.

**ft flusher: min workdone in a buffer flushed by cleaner thread**
 Min workdone value of any message buffer flushed by cleaner thread.

**ft flusher: nodes cleaned which had empty buffers**
 Number of nodes that are selected by cleaner, but whose buffers are empty.

**ft flusher: nodes dirtied by cleaner thread**
 Number of nodes that are made dirty by the cleaner thread.

**ft flusher: nodes dirtied by cleaner thread leaf merges**
 The number of nodes dirtied by the "flush from root" process to merge a leaf node.

**ft flusher: nonleaf node merges**
 Number of times nonleaf nodes are merged.

**ft flusher: nonleaf node splits**
 Number of nonleaf nodes split.

**ft flusher: number of flushes that read something off disk**
 Number of flushes that had to read a child (or part) off disk.

**ft flusher: number of flushes that triggered 1 cascading flush**
 Number of flushes that triggered 1 cascading flush.

**ft flusher: number of flushes that triggered 2 cascading flushes**
 Number of flushes that triggered 2 cascading flushes.

**ft flusher: number of flushes that triggered 3 cascading flushes**
 Number of flushes that triggered 3 cascading flushes.

**ft flusher: number of flushes that triggered 4 cascading flushes**
 Number of flushes that triggered 4 cascading flushes.

**ft flusher: number of flushes that triggered 5 cascading flushes**
 Number of flushes that triggered 5 cascading flushes.

**ft flusher: number of flushes that triggered another flush in child**
 Number of flushes that triggered another flush in the child.

**ft flusher: number of flushes that triggered over 5 cascading flushes**
 Number of flushes that triggered more than 5 cascading flushes.

**ft flusher: number of in memory flushes**
 Number of in-memory flushes.

**ft flusher: times cleaner thread tries to merge a leaf**
 The number of times the cleaner thread tries to merge a leaf.

**ft flusher: total bytes in buffers flushed by cleaner thread**
 Total number of bytes in message buffers flushed by cleaner thread.

**ft flusher: total nodes potentially flushed by cleaner thread**
 Total number of nodes whose buffers are potentially flushed by cleaner thread.

**ft flusher: total number of flushes done by flusher threads or cleaner threads**
 Total number of flushes done by flusher threads or cleaner threads.

**ft flusher: total workdone in buffers flushed by cleaner thread**
 Total workdone value of message buffers flushed by cleaner thread.

**handlerton: primary key bytes inserted**
 Total number of bytes inserted into all primary key indexes.

**hot: max number of flushes from root ever required to optimize a tree**
 The maximum number of flushes from the root ever required to optimize a tree.

**hot: operations aborted**
 The number of HOT operations that have been aborted.

**hot: operations ever started**
 The number of HOT operations that have begun.

**hot: operations successfully completed**
 The number of HOT operations that have successfully completed.

**indexer: max number of indexers that ever existed simultaneously**
 This is the maximum number of indexers that ever existed simultaneously.

**indexer: number of calls to indexer->abort()**
 This is the number of indexers that were aborted.

**indexer: number of calls to indexer->build() failed**
 This is the total number of times that indexes were unable to be created using a indexer

**indexer: number of calls to indexer->build() succeeded**
 This is the total number of times that indexes were created using a indexer.

**indexer: number of calls to indexer->close() that failed**
 This is the number of indexers that were unable to create the requested index(es).

**indexer: number of calls to indexer->close() that succeeded**
 This is the number of indexers that successfully created the requested index(es).

**indexer: number of calls to toku indexer create indexer() that failed**
 This is the number of times a indexer was requested but could not be created.

**indexer: number of indexers currently in existence**
 This is the number of indexers that currently exist.

**indexer: number of indexers successfully created**
 This is the number of times one of our internal objects, a indexer, has been created.

**le: expanded**
 This is the number of times that an expanded memory mechanism was used to store a new or modified row on disk.

**le: max committed xr**
 This is the maximum number of committed transaction records that were stored on disk in a new or modified row.

**le: max memsize**
 This is the maximum number of bytes that were stored on disk as a new or modified row. This is the maximum uncompressed size of any row stored in |TokuDB| that was created or modified since the server started.

**le: max provisional xr**
 This is the maximum number of provisional transaction records that were stored on disk in a new or modified row.

**le: size of leafentries after garbage collection (during message application)**
 Total number of bytes of leaf nodes data after performing garbage collection for non-flush events.

**le: size of leafentries after garbage collection (outside message application)**
 Total number of bytes of leaf nodes data after performing garbage collection for flush events.

**le: size of leafentries before garbage collection (during message application)**
 Total number of bytes of leaf nodes data before performing garbage collection for non-flush events.

**le: size of leafentries before garbage collection (outside message application)**
 Total number of bytes of leaf nodes data before performing garbage collection for flush events.

**loader: max number of loaders that ever existed simultaneously**
 This is the maximum number of loaders that ever existed simultaneously.

**loader: number of calls to loader->abort()**
 This is the number of loaders that were aborted.

**loader: number of calls to loader->close() that failed**
 This is the number of loaders that were unable to create the requested table.

**loader: number of calls to loader->close() that succeeded**
 This is the number of loaders that successfully created the requested table.

**loader: number of calls to loader->put() failed**
 This is the total number of rows that were unable to be inserted using a loader.

**loader: number of calls to loader->put() succeeded**
 This is the total number of rows that were inserted using a loader.

**loader: number of calls to toku loader create loader() that failed**
 This is the number of times a loader was requested but could not be created.

**loader: number of loaders currently in existence**
 This is the number of loaders that currently exist.

**loader: number of loaders successfully created**
 This is the number of times one of our internal objects, a loader, has been created.

**locktree: latest post-escalation memory size**
 Size of the locktree, in bytes, after most current locktree escalation.

**locktree: long time waiting for locks**
 Total time, in microseconds, of the long waits.

**locktree: long time waiting on lock escalation**
 Total time, in microseconds, of the long waits for lock escalation to free up memory.

**locktree: memory size**
 Count, in bytes, that the locktree is currently using.

**locktree: memory size limit**
 Maximum number of bytes that the locktree is allowed to use.

**locktree: number of lock timeouts**
 Count of the number of times that a lock request timed out.

**locktree: number of locktrees eligible for the STO**
 Number of locktrees eligible for "single transaction optimizations".

**locktree: number of locktrees open now**
 Number of locktrees currently open.

**locktree: number of lock waits**
 Number of times that a lock request could not be acquired because of a conflict with some other transaction.

**locktree: number of long lock waits**
 Number of lock waits greater than 1 second in duration.

**locktree: number of long waits on lock escalation**
 Number of times that a client thread had to wait on lock escalation and the wait time was greater than 1 second.

**locktree: number of pending lock requests**
 Number of requesters waiting for a lock grant.

**locktree: number of times a locktree ended the STO early**
 Total number of times a "single transaction optimization" was ended early due to another trans- action starting.

**locktree: number of times lock escalation ran**
 Number of times the locktree needed to run lock escalation to reduce its memory footprint.

**locktree: number of waits on lock escalation**
 When the sum of the sizes of locks taken reaches the lock tree limit, we run lock escalation on a background thread. The clients threads need to wait for escalation to consolidate locks and free up memory. This counter counts the number of times a client thread has to wait on lock escalation.

**locktree: time spent ending the STO early (seconds)**
 Total number of seconds ending "single transaction optimizations".

**locktree: time spent running escalation (seconds)**
 Total number of seconds spent performing locktree escalation.

**locktree: time waiting for locks**
 Total time, in microseconds, spend by some client waiting for a lock conflict to be resolved.

**locktree: time waiting on lock escalation**
 Total time, in microseconds, that a client thread spent waiting for lock escalation to free up memory.

**logger: next LSN**
 This is the next unassigned Log Sequence Number. It will be assigned to the next entry in the recovery log.

**logger: number of long logger write operations**
 Number of times a logger write operation required 100ms or more.

**logger: writes (bytes)**
 Number of bytes the logger has written to disk.

**logger: writes**
 Number of times the logger has written to disk.

**logger: writes (seconds)**
 Number of seconds waiting for IO when writing logs to disk.

**logger: writes (uncompressed bytes)**
 Number of uncompressed the logger has written to disk.

**max open dbs**
 Max number of simultaneously open DBs.

**memory: estimated maximum memory footprint**
 Maximum memory footprint of the storage engine, the max value of (used - freed).

**memory: largest attempted allocation size**
 Largest number of bytes in a single successful malloc() operation.

**memory: mallocator version**
 Version string from in-use memory allocator.

**memory: mmap threshold**
 The threshold for malloc to use mmap.

**memory: number of bytes freed**
 Total number of mallocated bytes freed (used - freed = bytes in use).

**memory: number of bytes requested**
 Total number of bytes requested from mallocator.

**memory: number of bytes used (requested + overhead)**
 Total number of bytes allocated by mallocator.

**memory: number of free operations**
 Number of calls to free().

**memory: number of malloc operations**
Number of calls to malloc().

**memory: number of malloc operations that failed**
 Number of failed calls to malloc().

**memory: number of realloc operations**
 Number of calls to realloc().

**memory: number of realloc operations that failed**
 Number of failed calls to realloc().

**memory: size of the last failed allocation attempt**
 Largest number of bytes in a single failed malloc() operation.

**num open dbs now**
 Number of currently open DBs.

**period, in ms, that recovery log is automatically fsynced**
 ``fsync()`` frequency in milliseconds.

**time now**
 Current date/time on server.

**time of engine startup**
 This is the time when the |TokuDB| storage engine started up. Normally, this is when ``mysqld`` started.

**time of environment creation**
 This is the time when the |TokuDB| storage engine was first started up. Normally, this is when ``mysqld`` was initially installed with |TokuDB| 5.x. If the environment was upgraded from |TokuDB| 4.x (4.2.0 or later), then this will be displayed as "Dec 31, 1969" on Linux hosts.

**txn: aborts**
 This is the total number of transactions that have been aborted.

**txn: begin**
 This is the number of transactions that have been started.

**txn: begin read only**
 Number of read only transactions started.

**txn: successful commits**
 This is the total number of transactions that have been committed.

Global Status
-------------

The :table:`INFORMATION_SCHEMA.GLOBAL_STATUS` table provides details about the inner workings of |TokuDB| and can be useful in tuning your particular environment. The statuses can be determined with the following command:

.. code-block:: mysql

 SELECT * FROM INFORMATION_SCHEMA.GLOBAL_STATUS;

The following global status parameters are available:

``TOKUDB_BASEMENTS_DECOMPRESSED_FOR_WRITE``
 Number of basement nodes decompressed for writes.

``TOKUDB_BASEMENTS_DECOMPRESSED_PREFETCH``
 Number of basement nodes decompressed by a prefetch thread.

``TOKUDB_BASEMENTS_DECOMPRESSED_PRELOCKED_RANGE``
 Number of basement nodes decompressed by queries aggressively.

``TOKUDB_BASEMENTS_DECOMPRESSED_TARGET_QUERY``
 Number of basement nodes decompressed for queries.

``TOKUDB_BASEMENT_DESERIALIZATION_FIXED_KEY``
 Number of basement nodes deserialized where all keys had the same size, leaving the basement in a format that is optimal for in-memory workloads.

``TOKUDB_BASEMENT_DESERIALIZATION_VARIABLE_KEY``
 Number of basement nodes deserialized where all keys did not have the same size, and thus ineligible for an in-memory optimization.

``TOKUDB_BASEMENTS_FETCHED_FOR WRITE_BYTES``
 Number of basement node bytes fetched from disk for writes.

``TOKUDB_BASEMENTS_FETCHED_FOR WRITE``
 Number of basement nodes fetched from disk for writes.

``TOKUDB_BASEMENTS_FETCHED_FOR WRITE_SECONDS``
 Number of seconds waiting for IO when fetching basement nodes from disk for writes.

``TOKUDB_BASEMENTS_FETCHED_PREFETCH_BYTES``
 Number of basement node bytes fetched from disk by a prefetch thread.

``TOKUDB_BASEMENTS_FETCHED_PREFETCH``
 Number of basement nodes fetched from disk by a prefetch thread.

``TOKUDB_BASEMENTS_FETCHED_PREFETCH_SECONDS``
 Number of seconds waiting for IO when fetching basement nodes from disk by a prefetch thread.

``TOKUDB_BASEMENTS_FETCHED_PRELOCKED_RANGE_BYTES``
 Number of basement node bytes fetched from disk aggressively.

``TOKUDB_BASEMENTS_FETCHED_PRELOCKED_RANGE``
 Number of basement nodes fetched from disk aggressively.

``TOKUDB_BASEMENTS_FETCHED_PRELOCKED_RANGE_SECONDS``
 Number of seconds waiting for IO when fetching basement nodes from disk aggressively.

``TOKUDB_BASEMENTS_FETCHED_TARGET_QUERY_BYTES``
 Number of basement node bytes fetched from disk for queries.

``TOKUDB_BASEMENTS_FETCHED_TARGET_QUERY``
 Number of basement nodes fetched from disk for queries.

``TOKUDB_BASEMENTS_FETCHED_TARGET_QUERY_SECONDS``
 Number of seconds waiting for IO when fetching basement nodes from disk for queries.

``TOKUDB_BROADCAST_MESSAGES_INJECTED_AT_ROOT``
 How many broadcast messages injected at root.

``TOKUDB_BUFFERS_DECOMPRESSED_FOR_WRITE``
 Number of buffers decompressed for writes.

``TOKUDB_BUFFERS_DECOMPRESSED_PREFETCH``
 Number of buffers decompressed by a prefetch thread.

``TOKUDB_BUFFERS_DECOMPRESSED_PRELOCKED_RANGE``
 Number of buffers decompressed by queries aggressively.

``TOKUDB_BUFFERS_DECOMPRESSED_TARGET_QUERY``
 Number of buffers decompressed for queries.

``TOKUDB_BUFFERS_FETCHED_FOR_WRITE_BYTES``
 Number of buffer bytes fetched from disk for writes.

``TOKUDB_BUFFERS_FETCHED_FOR_WRITE``
 Number of buffers fetched from disk for writes.

``TOKUDB_BUFFERS_FETCHED_FOR_WRITE_SECONDS``
 Number of seconds waiting for IO when fetching buffers from disk for writes.

``TOKUDB_BUFFERS_FETCHED_PREFETCH_BYTES``
 Number of buffer bytes fetched from disk by a prefetch thread.

``TOKUDB_BUFFERS_FETCHED_PREFETCH``
 Number of buffers fetched from disk by a prefetch thread.

``TOKUDB_BUFFERS_FETCHED_PREFETCH_SECONDS``
 Number of seconds waiting for IO when fetching buffers from disk by a prefetch thread.

``TOKUDB_BUFFERS_FETCHED_PRELOCKED_RANGE_BYTES``
 Number of buffer bytes fetched from disk aggressively.

``TOKUDB_BUFFERS_FETCHED_PRELOCKED_RANGE``
 Number of buffers fetched from disk aggressively.

``TOKUDB_BUFFERS_FETCHED_PRELOCKED_RANGE_SECONDS``
 Number of seconds waiting for IO when fetching buffers from disk aggressively.

``TOKUDB_BUFFERS_FETCHED_TARGET_QUERY_BYTES``
 Number of buffer bytes fetched from disk for queries.

``TOKUDB_BUFFERS_FETCHED_TARGET_QUERY``
 Number of buffers fetched from disk for queries.

``TOKUDB_BUFFERS_FETCHED_TARGET_QUERY_SECONDS``
 Number of seconds waiting for IO when fetching buffers from disk for queries.

``TOKUDB_CACHETABLE_CLEANER_EXECUTIONS``
 Total number of times the cleaner thread loop has executed.

``TOKUDB_CACHETABLE_CLEANER_ITERATIONS``
 This is the number of cleaner operations that are performed every cleaner period.

``TOKUDB_CACHETABLE_CLEANER_PERIOD``
 |TokuDB| includes a cleaner thread that optimizes indexes in the background. This variable is the time, in seconds, between the completion of a group of cleaner operations and the beginning of the next group of cleaner operations. The cleaner operations run on a background thread performing work that does not need to be done on the client thread.

``TOKUDB_CACHETABLE_EVICTIONS``
 Number of blocks evicted from cache.

``TOKUDB_CACHETABLE_LONG_WAIT_PRESSURE_COUNT``
 The number of times a thread was stalled for more than 1 second due to cache pressure.

``TOKUDB_CACHETABLE_LONG_WAIT_PRESSURE_TIME``
 Total time, in microseconds, waiting on cache pressure to subside for more than 1 second.

``TOKUDB_CACHETABLE_POOL_CLIENT_NUM_THREADS``

``TOKUDB_CACHETABLE_POOL_CLIENT_NUM_THREADS_ACTIVE``

``TOKUDB_CACHETABLE_POOL_CLIENT_QUEUE_SIZE``

``TOKUDB_CACHETABLE_POOL_CLIENT_MAX_QUEUE_SIZE``                    

``TOKUDB_CACHETABLE_POOL_CLIENT_TOTAL_ITEMS_PROCESSED``

``TOKUDB_CACHETABLE_POOL_CLIENT_TOTAL_EXECUTION_TIME``

``TOKUDB_CACHETABLE_POOL_CACHETABLE_NUM_THREADS``

``TOKUDB_CACHETABLE_POOL_CACHETABLE_NUM_THREADS_ACTIVE``

``TOKUDB_CACHETABLE_POOL_CACHETABLE_QUEUE_SIZE`` 

``TOKUDB_CACHETABLE_POOL_CACHETABLE_MAX_QUEUE_SIZE``

``TOKUDB_CACHETABLE_POOL_CACHETABLE_TOTAL_ITEMS_PROCESSED``

``TOKUDB_CACHETABLE_POOL_CACHETABLE_TOTAL_EXECUTION_TIME``

``TOKUDB_CACHETABLE_POOL_CHECKPOINT_NUM_THREADS``

``TOKUDB_CACHETABLE_POOL_CHECKPOINT_NUM_THREADS_ACTIVE``            

``TOKUDB_CACHETABLE_POOL_CHECKPOINT_QUEUE_SIZE``

``TOKUDB_CACHETABLE_POOL_CHECKPOINT_MAX_QUEUE_SIZE``

``TOKUDB_CACHETABLE_POOL_CHECKPOINT_TOTAL_ITEMS_PROCESSED``

``TOKUDB_CACHETABLE_POOL_CHECKPOINT_TOTAL_EXECUTION_TIME``

``TOKUDB_CACHETABLE_MISS``
 This is a count of how many times the application was unable to access your data in the internal cache.

``TOKUDB_CACHETABLE_MISS_TIME``
 This is the total time, in microseconds, of how long the database has had to wait for a disk read to complete.

``TOKUDB_CACHETABLE_PREFETCHES``
 This is the total number of times that a block of memory has been prefetched into the database's cache. Data is prefetched when the database's algorithms determine that a block of memory is likely to be accessed by the application.

``TOKUDB_CACHETABLE_SIZE_CACHEPRESSURE``
 The number of bytes causing cache pressure (the sum of buffers and workdone counters), helps to understand if cleaner threads are keeping up with workload.

``TOKUDB_CACHETABLE_SIZE_CLONED``
 Amount of memory, in bytes, currently used for cloned nodes. During the checkpoint operation, dirty nodes are cloned prior to serialization/compression, then written to disk. After which, the memory for the cloned block is returned for re-use.

``TOKUDB_CACHETABLE_SIZE_CURRENT``
 This is a count, in bytes, of how much of your uncompressed data is currently in the database's internal cache.

``TOKUDB_CACHETABLE_SIZE_LEAF``
 The number of bytes of leaf nodes in the cache.

``TOKUDB_CACHETABLE_SIZE_LIMIT``
 This is a count, in bytes, of how much of your uncompressed data will fit in the database's internal cache.

``TOKUDB_CACHETABLE_SIZE_NONLEAF``
 The number of bytes of nonleaf nodes in the cache.

``TOKUDB_CACHETABLE_SIZE_ROLLBACK``
 The number of bytes of rollback nodes in the cache.

``TOKUDB_CACHETABLE_SIZE_WRITING``
 This is the number of bytes that are currently queued up to be written to disk.

``TOKUDB_CACHETABLE_WAIT_PRESSURE_COUNT``
 The number of times a thread was stalled due to cache pressure.

``TOKUDB_CACHETABLE_WAIT_PRESSURE TIME``
 Total time, in microseconds, waiting on cache pressure to subside.

``TOKUDB_CHECKPOINT_BEGIN_TIME``
 Cumulative time (in microseconds) required to mark all dirty nodes as pending a checkpoint.

``TOKUDB_CHECKPOINT_DURATION_LAST``
 Time (in seconds) required to complete the last checkpoint.

``TOKUDB_CHECKPOINT_DURATION``
 Time (in seconds) required to complete all checkpoints.

``TOKUDB_CHECKPOINT_FAILED``
 This is the number of checkpoints that have failed for any reason.

``TOKUDB_CHECKPOINT_LAST_BEGAN``
 This is the time the last checkpoint began. If a checkpoint is currently in progress, then this time may be later than the time the last checkpoint completed. (Note, if no checkpoint has ever taken place, then this value will be "Dec 31, 1969" on Linux hosts.)

``TOKUDB_CHECKPOINT_LAST_COMPLETE_BEGAN``
 This is the time the last complete checkpoint started. Any data that changed after this time will not be captured in the checkpoint.

``TOKUDB_CHECKPOINT_LAST_COMPLETE_ENDED``
 This is the time the last complete checkpoint ended.

``TOKUDB_CHECKPOINT_LONG_CHECKPOINT_BEGIN_COUNT``
 The total number of times a checkpoint begin took more than 1 second.

``TOKUDB_CHECKPOINT_END_TIME``
 The time spent in checkpoint end operation in seconds.

``TOKUDB_CHECKPOINT_LONG_END_COUNT``
 This is the count of end_checkpoint operations that exceeded 1 minute.

``TOKUDB_CHECKPOINT_LONG_END_TIME``
 This is the total time of long checkpoints in seconds.

``TOKUDB_CHECKPOINT_LONG_CHECKPOINT_BEGIN_TIME``
 This is the total time, in microseconds, of long checkpoint begins. A long checkpoint begin is one taking more than 1 second.

``TOKUDB_CHECKPOINT_PERIOD``
 This is the interval in seconds between the end of an automatic checkpoint and the beginning of the next automatic checkpoint.

``TOKUDB_CHECKPOINT_TAKEN``
 This is the number of complete checkpoints that have been taken.

``TOKUDB_DB_CLOSES``
 Number of db close operations.

``TOKUDB_DB_OPEN_CURRENT``
 Number of currently open DBs.

``TOKUDB_DB_OPEN_MAX``
 Max number of simultaneously open DBs.

``TOKUDB_DB_OPENS``
 Number of db open operations.

``TOKUDB_DESCRIPTOR_SET``
 This is the number of time a descriptor was updated when the entire dictionary was updated (for example, when the schema has been changed).

``TOKUDB_DICTIONARY_BROADCAST_UPDATES``
 This is the number of broadcast updates that have been successfully performed. A broadcast update is an update that affects all rows in a dictionary.

``TOKUDB_DICTIONARY_UPDATES``
 This is the total number of rows that have been updated in all primary and secondary indexes combined, if those updates have been done with a separate recovery log entry per index.

``TOKUDB_FILESYSTEM_FSYNC_NUM``
 This is the total number of times the database has flushed the operating system's file buffers to disk.

``TOKUDB_FILESYSTEM_FSYNC_TIME``
 This the total time, in microseconds, used to fsync to disk.

``TOKUDB_FILESYSTEM_LONG_FSYNC_NUM``
 This is the total number of times the database has flushed the operating system's file buffers to disk and this operation required more than 1 second.

``TOKUDB_FILESYSTEM_LONG_FSYNC_TIME``
 This the total time, in microseconds, used to fsync to disk when the operation required more than 1 second.

``TOKUDB_FILESYSTEM_THREADS_BLOCKED_BY_FULL_DISK``
 This is the number of threads that are currently blocked because they are attempting to write to a full disk. This is normally zero. If this value is non-zero, then a warning will appear in the "disk free space" field.

``TOKUDB_LEAF_COMPRESSION_TO_MEMORY_SECONDS``
 Total time, in seconds, spent compressing leaf nodes.

``TOKUDB_LEAF_DECOMPRESSION_TO_MEMORY_SECONDS``
 Total time, in seconds, spent decompressing leaf nodes.

``TOKUDB_LEAF_DESERIALIZATION_TO_MEMORY_SECONDS``
 Total time, in seconds, spent deserializing leaf nodes.

``TOKUDB_LEAF_NODE_COMPRESSION_RATIO``
 Ratio of uncompressed bytes (in-memory) to compressed bytes (on-disk) for leaf nodes.

``TOKUDB_LEAF_NODE_FULL_EVICTIONS_BYTES``
 The number of bytes freed by evicting full leaf nodes from the cache.

``TOKUDB_LEAF_NODE_FULL_EVICTIONS``
 The number of times a full leaf node was evicted from the cache.

``TOKUDB_LEAF_NODE_PARTIAL_EVICTIONS_BYTES``
 The number of bytes freed by evicting partitions of leaf nodes from the cache.

``TOKUDB_LEAF_NODE_PARTIAL_EVICTIONS``
 The number of times a partition of a leaf node was evicted from the cache.

``TOKUDB_LEAF_NODES_CREATED``
 Number of leaf nodes created.

``TOKUDB_LEAF_NODES_DESTROYED``
 Number of leaf nodes destroyed.

``TOKUDB_LEAF_NODES_FLUSHED_CHECKPOINT_BYTES``
 Number of bytes of leaf nodes flushed to disk for checkpoint.

``TOKUDB_LEAF_NODES_FLUSHED_CHECKPOINT``
 Number of leaf nodes flushed to disk for checkpoint.

``TOKUDB_LEAF_NODES_FLUSHED_CHECKPOINT_SECONDS``
 Number of seconds waiting for IO when writing leaf nodes flushed to disk for checkpoint.

``TOKUDB_LEAF_NODES_FLUSHED_CHECKPOINT_UNCOMPRESSED BYTES``
 Number of uncompressed bytes of leaf nodes flushed to disk for checkpoint.

``TOKUDB_LEAF_NODES_FLUSHED_NOT_CHECKPOINT_BYTES``
 Number of bytes of leaf nodes flushed to disk, not for checkpoint.

``TOKUDB_LEAF_NODES_FLUSHED_NOT_CHECKPOINT``
 Number of leaf nodes flushed to disk, not for checkpoint.

``TOKUDB_LEAF_NODES_FLUSHED_NOT_CHECKPOINT_SECONDS``
 Number of seconds waiting for IO when writing leaf nodes flushed to disk, not for checkpoint.

``TOKUDB_LEAF_NODES_FLUSHED_NOT_CHECKPOINT_UNCOMPRESSED_BYTES``
 Number of bytes of leaf nodes flushed to disk, not for checkpoint.

``TOKUDB_LEAF_SERIALIZATION_TO_MEMORY_SECONDS``
 Total time, in seconds, spent serializing leaf nodes.

``TOKUDB_LOADER_NUM_CREATED``
 This is the number of times one of our internal objects, a loader, has been created.

``TOKUDB_LOADER_NUM_CURRENT``
 This is the number of loaders that currently exist.

``TOKUDB_LOADER_NUM_MAX``
 This is the maximum number of loaders that ever existed simultaneously.

``TOKUDB_LOCKTREE_ESCALATION_NUM``
 Number of times the locktree needed to run lock escalation to reduce its memory footprint.

``TOKUDB_LOCKTREE_ESCALATION_SECONDS``
 Total number of seconds spent performing locktree escalation.

``TOKUDB_LOCKTREE_LATEST_POST_ESCALATION_MEMORY_SIZE``
 Size of the locktree, in bytes, after most current locktree escalation.

``TOKUDB_LOCKTREE_LONG_WAIT_COUNT``
 Number of lock waits greater than 1 second in duration.

``TOKUDB_LOCKTREE_LONG_WAIT_ESCALATION_COUNT``
 Number of times that a client thread had to wait on lock escalation and the wait time was greater than 1 second.

``TOKUDB_LOCKTREE_LONG_WAIT_ESCALATION_TIME``
 Total time, in microseconds, of the long waits for lock escalation to free up memory.

``TOKUDB_LOCKTREE_LONG_WAIT_TIME``
 Total time, in microseconds, of the long waits.

``TOKUDB_LOCKTREE_MEMORY_SIZE``
 Count, in bytes, that the locktree is currently using.

``TOKUDB_LOCKTREE_MEMORY_SIZE_LIMIT``
 Maximum number of bytes that the locktree is allowed to use.

``TOKUDB_LOCKTREE_OPEN_CURRENT``
 Number of locktrees currently open.

``TOKUDB_LOCKTREE_PENDING_LOCK_REQUESTS``
 Number of requesters waiting for a lock grant.

``TOKUDB_LOCKTREE_STO_ELIGIBLE_NUM``
 Number of locktrees eligible for "single transaction optimizations".

``TOKUDB_LOCKTREE_STO_ENDED_NUM``
 Total number of times a "single transaction optimization" was ended early due to another transaction starting.

``TOKUDB_LOCKTREE_STO_ENDED_SECONDS``
 Total number of seconds ending "single transaction optimizations".

``TOKUDB_LOCKTREE_TIMEOUT_COUNT``
 Count of the number of times that a lock request timed out.

``TOKUDB_LOCKTREE_WAIT_COUNT``
 Number of times that a lock request could not be acquired because of a conflict with some other transaction.

``TOKUDB_LOCKTREE_WAIT_ESCALATION_COUNT``
 When the sum of the sizes of locks taken reaches the lock tree limit, we run lock escalation on a background thread. The clients threads need to wait for escalation to consolidate locks and free up memory. This counter counts the number of times a client thread has to wait on lock escalation.

``TOKUDB_LOCKTREE_WAIT_ESCALATION_TIME``
 Total time, in microseconds, that a client thread spent waiting for lock escalation to free up memory.

``TOKUDB_LOCKTREE_WAIT_TIME``
 Total time, in microseconds, spend by some client waiting for a lock conflict to be resolved.

``TOKUDB_LOGGER_WAIT_LONG``
 Number of times a logger write operation required 100ms or more.

``TOKUDB_LOGGER_WRITES_BYTES``
 Number of bytes the logger has written to disk.

``TOKUDB_LOGGER_WRITES``
 Number of times the logger has written to disk.

``TOKUDB_LOGGER_WRITES_SECONDS``
 Number of seconds waiting for IO when writing logs to disk.

``TOKUDB_LOGGER_WRITES_UNCOMPRESSED_BYTES``
 Number of uncompressed the logger has written to disk.

``TOKUDB_MEM_ESTIMATED_MAXIMUM_MEMORY_FOOTPRINT``
 Maximum memory footprint of the storage engine, the max value of (used - freed).

``TOKUDB_MESSAGES_FLUSHED_FROM_H1_TO_LEAVES_BYTES``
 How many bytes of messages flushed from h1 nodes to leaves.

``TOKUDB_MESSAGES_IGNORED_BY_LEAF_DUE_TO_MSN``
 The number of messages that were ignored by a leaf because it had already been applied.

``TOKUDB_MESSAGES_INJECTED_AT_ROOT_BYTES``
 How many bytes of messages injected at root (for all trees).

``TOKUDB_MESSAGES_INJECTED_AT_ROOT``
 How many messages injected at root.

``TOKUDB_MESSAGES_IN_TREES_ESTIMATE_BYTES``
 How many bytes of messages currently in trees (estimate).

``TOKUDB_NONLEAF_COMPRESSION_TO_MEMORY_SECONDS``
 Total time, in seconds, spent compressing non leaf nodes.

``TOKUDB_NONLEAF_DECOMPRESSION_TO_MEMORY_SECONDS``
 Total time, in seconds, spent decompressing non leaf nodes.

``TOKUDB_NONLEAF_DESERIALIZATION_TO_MEMORY_SECONDS``
 Total time, in seconds, spent deserializing non leaf nodes.

``TOKUDB_NONLEAF_NODE_COMPRESSION_RATIO``
 Ratio of uncompressed bytes (in-memory) to compressed bytes (on-disk) for nonleaf nodes.

``TOKUDB_NONLEAF_NODE_FULL_EVICTIONS_BYTES``
 The number of bytes freed by evicting full nonleaf nodes from the cache.

``TOKUDB_NONLEAF_NODE_FULL_EVICTIONS``
 The number of times a full nonleaf node was evicted from the cache.

``TOKUDB_NONLEAF_NODE_PARTIAL_EVICTIONS_BYTES``
 The number of bytes freed by evicting partitions of nonleaf nodes from the cache.

``TOKUDB_NONLEAF_NODE_PARTIAL_EVICTIONS``
 The number of times a partition of a nonleaf node was evicted from the cache.

``TOKUDB_NONLEAF_NODES_CREATED``
 Number of nonleaf nodes created.

``TOKUDB_NONLEAF_NODES_DESTROYED``
 Number of nonleaf nodes destroyed.

``TOKUDB_NONLEAF_NODES_FLUSHED_TO_DISK_CHECKPOINT_BYTES``
 Number of bytes of nonleaf nodes flushed to disk for checkpoint.

``TOKUDB_NONLEAF_NODES_FLUSHED_TO_DISK_CHECKPOINT``
 Number of nonleaf nodes flushed to disk for checkpoint.

``TOKUDB_NONLEAF_NODES_FLUSHED_TO_DISK_CHECKPOINT_SECONDS``
 Number of seconds waiting for IO when writing nonleaf nodes flushed to disk for checkpoint.

``TOKUDB_NONLEAF_NODES_FLUSHED_TO_DISK_CHECKPOINT_UNCOMPRESSED_BYTES``
 Number of uncompressed bytes of nonleaf nodes flushed to disk for checkpoint.

``TOKUDB_NONLEAF_NODES_FLUSHED_TO_DISK_NOT_CHECKPOINT_BYTES``
 Number of bytes of nonleaf nodes flushed to disk, not for checkpoint.

``TOKUDB_NONLEAF_NODES_FLUSHED_TO_DISK_NOT_CHECKPOINT``
 Number of nonleaf nodes flushed to disk, not for checkpoint.

``TOKUDB_NONLEAF_NODES_FLUSHED_TO_DISK_NOT_CHECKPOINT_SECONDS``
 Number of seconds waiting for IO when writing nonleaf nodes flushed to disk, not for check- point.

``TOKUDB_NONLEAF_NODES_FLUSHED_TO_DISK_NOT_CHECKPOINT_UNCOMPRESSED_BYTES``
 Number of uncompressed bytes of nonleaf nodes flushed to disk, not for checkpoint.

``TOKUDB_NONLEAF_SERIALIZATION_TO_MEMORY_SECONDS``
 Total time, in seconds, spent serializing non leaf nodes.

``TOKUDB_OVERALL_NODE_COMPRESSION_RATIO``
 Ratio of uncompressed bytes (in-memory) to compressed bytes (on-disk) for all nodes.

``TOKUDB_PIVOTS_FETCHED_FOR_PREFETCH_BYTES``
 Number of bytes of pivot nodes fetched by a prefetch thread.

``TOKUDB_PIVOTS_FETCHED_FOR_PREFETCH``
 Number of pivot nodes fetched by a prefetch thread.

``TOKUDB_PIVOTS_FETCHED_FOR_PREFETCH_SECONDS``
 Number seconds waiting for IO when fetching pivot nodes by a prefetch thread.

``TOKUDB_PIVOTS_FETCHED_FOR_QUERY_BYTES``
 Number of bytes of pivot nodes fetched for queries.

``TOKUDB_PIVOTS_FETCHED_FOR_QUERY``
 Number of pivot nodes fetched for queries.

``TOKUDB_PIVOTS_FETCHED_FOR_QUERY_SECONDS``
 Number of seconds waiting for IO when fetching pivot nodes for queries.

``TOKUDB_PIVOTS_FETCHED_FOR_WRITE_BYTES``
 Number of bytes of pivot nodes fetched for writes.

``TOKUDB_PIVOTS_FETCHED_FOR_WRITE``
 Number of pivot nodes fetched for writes.

``TOKUDB_PIVOTS_FETCHED_FOR_WRITE_SECONDS``
 Number of seconds waiting for IO when fetching pivot nodes for writes.

``TOKUDB_PROMOTION_H1_ROOTS_INJECTED_INTO``
 Number of times a message stopped at a root with height 1.

``TOKUDB_PROMOTION_INJECTIONS_AT_DEPTH_0``
 Number of times a message stopped at depth 0.

``TOKUDB_PROMOTION_INJECTIONS_AT_DEPTH_1``
 Number of times a message stopped at depth 1.

``TOKUDB_PROMOTION_INJECTIONS_AT_DEPTH_2``
 Number of times a message stopped at depth 2.

``TOKUDB_PROMOTION_INJECTIONS_AT_DEPTH_3``
 Number of times a message stopped at depth 3.

``TOKUDB_PROMOTION_INJECTIONS_LOWER_THAN_DEPTH_3``
 Number of times a message was promoted past depth 3.

``TOKUDB_PROMOTION_LEAF_ROOTS_INJECTED_INTO``
 Number of times a message stopped at a root with height 0.

``TOKUDB_PROMOTION_ROOTS_SPLIT``
 Number of times the root split during promotion.

``TOKUDB_PROMOTION_STOPPED_AFTER_LOCKING_CHILD``
 Number of times a message stopped before a child which had been locked.

``TOKUDB_PROMOTION_STOPPED_AT_HEIGHT_1``
 Number of times a message stopped because it had reached height 1.

``TOKUDB_PROMOTION_STOPPED_CHILD_LOCKED_OR_NOT_IN_MEMORY``
 Number of times a message stopped because it could not cheaply get access to a child.

``TOKUDB_PROMOTION_STOPPED_CHILD_NOT_FULLY_IN_MEMORY``
 Number of times a message stopped because it could not cheaply get access to a child.

``TOKUDB_PROMOTION_STOPPED_NONEMPTY_BUFFER``
 Number of times a message stopped because it reached a nonempty buffer.

``TOKUDB_TXN_ABORTS``
 This is the total number of transactions that have been aborted.

``TOKUDB_TXN_BEGIN``
 This is the number of transactions that have been started.

``TOKUDB_TXN_BEGIN_READ_ONLY``
 Number of read only transactions started.

``TOKUDB_TXN_COMMITS``
 This is the total number of transactions that have been committed.
