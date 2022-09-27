.. _tokudb_troubleshooting:

======================
TokuDB Troubleshooting
======================

.. Important:: 

   Starting with :ref:`8.0.28-19`, the TokuDB storage engine is no longer supported. We have removed the storage engine from the installation packages and disabled the storage engine in our binary builds.

   Starting with :ref:`8.0.26-16`, the binary builds and packages include but disable the TokuDB storage engine plugins. The ``tokudb_enabled`` option and the ``tokudb_backup_enabled`` option control the state of the plugins and have a default setting of ``FALSE``. The result of attempting to load the plugins are the plugins fail to initialize and print a deprecation message.

   We recommend :ref:`migrate-myrocks`. To enable the plugins to migrate to another storage engine, set the ``tokudb_enabled`` and ``tokudb_backup_enabled`` options to ``TRUE`` in your ``my.cnf`` file and restart your server instance. Then, you can load the plugins.

   The TokuDB Storage Engine was `declared as deprecated <https://www.percona.com/doc/percona-server/8.0/release-notes/Percona-Server-8.0.13-3.html>`__ in Percona Server for MySQL 8.0. For more information, see the Percona blog post: `Heads-Up: TokuDB Support Changes and Future Removal from Percona Server for MySQL 8.0 <https://www.percona.com/blog/2021/05/21/tokudb-support-changes-and-future-removal-from-percona-server-for-mysql-8-0/>`__.

.. contents::
   :local:
   :depth: 1

.. _tokudb_known_issues:

Known Issues
===============================================================================

**Replication and binary logging**: *TokuDB* supports binary logging and replication, with one restriction. *TokuDB* does not implement a lock on the auto-increment function, so concurrent insert statements with one or more of the statements inserting multiple rows may result in a non-deterministic interleaving of the auto-increment values. When running replication with these concurrent inserts, the auto-increment values on the replica table may not match the auto-increment values on the source table. Note that this is only an issue with Statement Based Replication (SBR), and not Row Based Replication (RBR).

For more information about auto-increment and replication, see the *MySQL*
Reference Manual: `AUTO_INCREMENT handling in InnoDB
<http://dev.mysql.com/doc/refman/8.0/en/innodb-auto-increment-handling.html>`_.

In addition, when using the ``REPLACE INTO`` or ``INSERT IGNORE`` on tables with no secondary indexes or tables where secondary indexes are subsets of the primary, the session variable :ref:`tokudb_pk_insert_mode` controls whether row based replication will work.

**Uninformative error message**: The ``LOAD DATA INFILE`` command can sometimes
 produce ``ERROR 1030 (HY000): Got error 1 from storage engine``. The message
 should say that the error is caused by insufficient disk space for the
 temporary files created by the loader.

**Transparent Huge Pages**: *TokuDB* will refuse to start if transparent huge
 pages are enabled. Transparent huge page support can be disabled by issuing the
 following as root:

.. code-block:: console

   # echo never > /sys/kernel/mm/redhat_transparent_hugepage/enabled

.. note::

   The previous command needs to be executed after every reboot, because it
   defaults to ``always``.

**XA behavior vs. InnoDB**: *InnoDB* forces a deadlocked XA transaction to
 abort, *TokuDB* does not.

**Disabling the unique checks**: For tables with unique keys, every insertion
into the table causes a lookup by key followed by an insertion, if the key is
not in the table. This greatly limits insertion performance. If one knows by
design that the rows being inserted into the table have unique keys, then one
can disable the key lookup prior to insertion.

If your primary key is an auto-increment key, and none of your secondary keys
are declared to be unique, then setting ``unique_checks=OFF`` will provide
limited performance gains. On the other hand, if your primary key has a lot of
entropy (it looks random), or your secondary keys are declared unique and have
a lot of entropy, then disabling unique checks can provide a significant
performance boost.

If :ref:`unique_checks` is disabled when the primary key is not unique,
secondary indexes may become corrupted. In this case, the indexes should be
dropped and rebuilt. This behavior differs from that of *InnoDB*, in which
uniqueness is always checked on the primary key, and setting
:ref:`unique_checks` to off turns off uniqueness checking on secondary
indexes only. Turning off uniqueness checking on the primary key can provide
large performance boosts, but it should only be done when the primary key is
known to be unique.

**Group Replication**: *TokuDB* storage engine doesn't support `Group Replication
<https://dev.mysql.com/doc/refman/8.0/en/group-replication.html>`_.

As of 8.0.17, InnoDB supports `multi-valued indexes <https://dev.mysql.com/doc/refman/8.0/en/create-index.html#create-index-multi-valued>`__. TokuDB does not support this feature.

As of 8.0.17, InnoDB supports the `Clone Plugin <https://dev.mysql.com/doc/refman/8.0/en/clone-plugin.html>`__ and the Clone Plugin API. TokuDB tables do not support either of these features.

.. _tokudb_lock_visualization:

Lock Visualization in TokuDB
================================================================================

*TokuDB* uses key range locks to implement serializable transactions, which are
acquired as the transaction progresses. The locks are released when the
transaction commits or aborts (this implements two phase locking).

*TokuDB* stores these locks in a data structure called the lock tree. The lock
tree stores the set of range locks granted to each transaction. In addition, the
lock tree stores the set of locks that are not granted due to a conflict with
locks granted to some other transaction. When these other transactions are
retired, these pending lock requests are retried. If a pending lock request is
not granted before the lock timer expires, then the lock request is aborted.

Lock visualization in *TokuDB* exposes the state of the lock tree with tables in
the information schema. We also provide a mechanism that may be used by a
database client to retrieve details about lock conflicts that it encountered
while executing a transaction.

The ``TOKUDB_TRX`` table
--------------------------------------------------------------------------------

The :ref:`TOKUDB_TRX` table in the ``INFORMATION_SCHEMA`` maps *TokuDB*
transaction identifiers to *MySQL* client identifiers. This mapping allows one
to associate a *TokuDB* transaction with a *MySQL* client operation.

The following query returns the *MySQL* clients that have a live *TokuDB*
transaction:

.. code-block:: mysql

   SELECT * FROM INFORMATION_SCHEMA.TOKUDB_TRX,
   INFORMATION_SCHEMA.PROCESSLIST
   WHERE trx_mysql_thread_id = id;

The ``TOKUDB_LOCKS`` table
--------------------------------------------------------------------------------

The :ref:`tokudb_locks` table in the information schema contains the set of
locks granted to *TokuDB* transactions.

The following query returns all of the locks granted to some *TokuDB*
transaction:

.. code-block:: mysql

   SELECT * FROM INFORMATION_SCHEMA.TOKUDB_LOCKS;

The following query returns the locks granted to some *MySQL* client:

.. code-block:: mysql

   SELECT id FROM INFORMATION_SCHEMA.TOKUDB_LOCKS,
   INFORMATION_SCHEMA.PROCESSLIST
   WHERE locks_mysql_thread_id = id;

The ``TOKUDB_LOCK_WAITS`` table
--------------------------------------------------------------------------------

The :ref:`tokudb_lock_waits` table in the information schema contains the set
of lock requests that are not granted due to a lock conflict with some other
transaction.

The following query returns the locks that are waiting to be granted due to a
lock conflict with some other transaction:

.. code-block:: mysql

   SELECT * FROM INFORMATION_SCHEMA.TOKUDB_LOCK_WAITS;

Supporting explicit DEFAULT value expressions as of 8.0.13-3
--------------------------------------------------------------------------------

TokuDB does not support `explicit DEFAULT value expressions <https://dev.mysql.com/doc/refman/8.0/en/data-type-defaults.html>`__ as of verion 8.0.13-3.


The :ref:`tokudb_lock_timeout_debug` session variable
--------------------------------------------------------------------------------

The :ref:`tokudb_lock_timeout_debug` session variable controls how lock
timeouts and lock deadlocks seen by the database client are reported.

The following values are available:

:0: No lock timeouts or lock deadlocks are reported.

:1: A JSON document that describes the lock conflict is stored in the
    :ref:`tokudb_last_lock_timeout` session variable

:2: A JSON document that describes the lock conflict is printed to the *MySQL*
    error log.

    *Supported since 7.5.5*: In addition to the JSON document describing the lock conflict, the following lines are printed to the MySQL error log:

    * A line containing the blocked thread id and blocked SQL
    * A line containing the blocking thread id and the blocking SQL.

:3: A JSON document that describes the lock conflict is stored in the :ref:`tokudb_last_lock_timeout` session variable and is printed to the *MySQL* error log.

    *Supported since 7.5.5*: In addition to the JSON document describing the lock conflict, the following lines are printed to the *MySQL* error log:

    * A line containing the blocked thread id and blocked SQL
    * A line containing the blocking thread id and the blocking SQL.

The :ref:`tokudb_last_lock_timeout` session variable
--------------------------------------------------------------------------------

The :ref:`tokudb_last_lock_timeout` session variable contains a JSON
document that describes the last lock conflict seen by the current *MySQL*
client. It gets set when a blocked lock request times out or a lock deadlock is
detected. The :ref:`tokudb_lock_timeout_debug` session variable should have
bit ``0`` set (decimal ``1``).

.. rubric:: Example

Suppose that we create a table with a single column that is the primary key.

.. code-block:: mysql

 mysql> SHOW CREATE TABLE table;

 Create Table: CREATE TABLE ‘table‘ (
 ‘id‘ int(11) NOT NULL,
 PRIMARY KEY (‘id‘)) ENGINE=TokuDB DEFAULT CHARSET=latin1

Suppose that we have 2 *MySQL* clients with ID's 1 and 2 respectively. Suppose
that *MySQL* client 1 inserts some values into ``table``. *TokuDB* transaction
51 is created for the insert statement. Since autocommit is disabled,
transaction 51 is still live after the insert statement completes, and we can
query the :ref:`tokudb_locks` table in information schema to see the locks
that are held by the transaction.

.. code-block:: mysql

   mysql> SET AUTOCOMMIT=OFF;
   mysql> INSERT INTO table VALUES (1),(10),(100);

.. admonition:: Output

   .. code-block:: mysql

      Query OK, 3 rows affected (0.00 sec)
      Records: 3  Duplicates: 0  Warnings: 0

.. code-block:: mysql

   mysql> SELECT * FROM INFORMATION_SCHEMA.TOKUDB_LOCKS;

.. admonition:: Output

   .. code-block:: mysql

      +--------------+-----------------------+---------------+----------------+-----------------+--------------------+------------------+-----------------------------+
      | locks_trx_id | locks_mysql_thread_id | locks_dname   | locks_key_left | locks_key_right | locks_table_schema | locks_table_name | locks_table_dictionary_name |
      +--------------+-----------------------+---------------+----------------+-----------------+--------------------+------------------+-----------------------------+
      |           51 |                     1 | ./test/t-main | 0001000000     | 0001000000      | test               | t                | main                        |
      |           51 |                     1 | ./test/t-main | 000a000000     | 000a000000      | test               | t                | main                        |
      |           51 |                     1 | ./test/t-main | 0064000000     | 0064000000      | test               | t                | main                        |
      +--------------+-----------------------+---------------+----------------+-----------------+--------------------+------------------+-----------------------------+

.. code-block:: mysql

   mysql> SELECT * FROM INFORMATION_SCHEMA.TOKUDB_LOCK_WAITS;

.. admonition:: Output

   .. code-block:: mysql

      Empty set (0.00 sec)

The keys are currently hex dumped.

Now we switch to the other *MySQL* client with ID 2.

.. code-block:: mysql

   mysql> INSERT INTO table VALUES (2),(20),(100);

The insert gets blocked since there is a conflict on the primary key with value 100.

The granted *TokuDB* locks are:

.. code-block:: mysql

   SELECT * FROM INFORMATION_SCHEMA.TOKUDB_LOCKS;

.. admonition:: Output

   .. code-block:: mysql

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

.. admonition:: Error

   ERROR 1205 (HY000): Lock wait timeout exceeded; try restarting transaction

.. code-block:: mysql

   SELECT @@TOKUDB_LAST_LOCK_TIMEOUT;

.. admonition:: Output

   .. code-block:: mysql

      +---------------------------------------------------------------------------------------------------------------+
      | @@tokudb_last_lock_timeout                                                                                    |
      +---------------------------------------------------------------------------------------------------------------+
      | "mysql_thread_id":2, "dbname":"./test/t-main", "requesting_txnid":62, "blocking_txnid":51, "key":"0064000000" |
      +---------------------------------------------------------------------------------------------------------------+

.. code-block:: mysql

   ROLLBACK;

Since transaction 62 was rolled back, all of the locks taken by it are released.

.. code-block:: mysql

   SELECT * FROM INFORMATION_SCHEMA.TOKUDB_LOCKS;

.. admonition:: Output

   .. code-block:: mysql

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
--------------------------------------------------------------------------------

Engine status provides details about the inner workings of *TokuDB* and can be
useful in tuning your particular environment. The engine status can be
determined by running the following command:  :mysql:`SHOW ENGINE tokudb STATUS;`

The following is a reference of the table status statements:

.. list-table::
   :widths: 15 85
   :header-rows: 1

   * - Table Status
     - Description

   * - disk free space
     - This is a gross estimate of how much of your file system is available.
       Possible displays in this field are:

       * More than twice the reserve ("more than 10 percent of total file system
	 space")
       * Less than twice the reserve
       * Less than the reserve
       * File system is completely full

   * - time of environment creation
     - This is the time when the *TokuDB* storage engine was first started up.
       Normally, this is when ``mysqld`` was initially installed with *TokuDB*. If
       the environment was upgraded from *TokuDB* 4.x (4.2.0 or later), then this
       will be displayed as "Dec 31, 1969" on Linux hosts.

   * - time of engine startup
     - This is the time when the *TokuDB* storage engine started up. Normally, this
       is when ``mysqld`` started.

   * - time now
     - Current date/time on server.

   * - db opens
     - This is the number of times an individual PerconaFT dictionary file was
       opened. This is a not a useful value for a regular user to use for any purpose
       due to layers of open/close caching on top.

   * - db closes
     - This is the number of times an individual PerconaFT dictionary file was
       closed. This is a not a useful value for a regular user to use for any purpose
       due to layers of open/close caching on top.

   * - num open dbs now
     - This is the number of currently open databases.

   * - max open dbs
     - This is the maximum number of concurrently opened databases.

   * - period, in ms, that recovery log is automatically fsynced
     - ``fsync()`` frequency in milliseconds.

   * - dictionary inserts
     - This is the total number of rows that have been inserted into all primary and
       secondary indexes combined, when those inserts have been done with a separate
       recovery log entry per index. For example, inserting a row into a table with
       one primary and two secondary indexes will increase this count by three, if
       the inserts were done with separate recovery log entries.

   * - dictionary inserts fail
     - This is the number of single-index insert operations that failed.

   * - dictionary deletes
     - This is the total number of rows that have been deleted from all primary and
       secondary indexes combined, if those deletes have been done with a separate
       recovery log entry per index.

   * - dictionary deletes fail
     - This is the number of single-index delete operations that failed.

   * - dictionary updates
     - This is the total number of rows that have been updated in all primary and
       secondary indexes combined, if those updates have been done with a separate
       recovery log entry per index.

   * - dictionary updates fail
     - This is the number of single-index update operations that failed.

   * - dictionary broadcast updates
     - This is the number of broadcast updates that have been successfully performed.
       A broadcast update is an update that affects all rows in a dictionary.

   * - dictionary broadcast updates fail
     - This is the number of broadcast updates that have failed.

   * - dictionary multi inserts
     - This is the total number of rows that have been inserted into all primary and
       secondary indexes combined, when those inserts have been done with a single
       recovery log entry for the entire row. (For example, inserting a row into a
       table with one primary and two secondary indexes will normally increase this
       count by three).

   * - dictionary multi inserts fail
     - This is the number of multi-index insert operations that failed.

   * - dictionary multi deletes
     - This is the total number of rows that have been deleted from all primary and
       secondary indexes combined, when those deletes have been done with a single
       recovery log entry for the entire row.

   * - dictionary multi deletes fail
     - This is the number of multi-index delete operations that failed.

   * - dictionary updates multi
     - This is the total number of rows that have been updated in all primary and
       secondary indexes combined, if those updates have been done with a single
       recovery log entry for the entire row.

   * - dictionary updates fail multi
     - This is the number of multi-index update operations that failed.

   * - le: max committed xr
     - This is the maximum number of committed transaction records that were stored
       on disk in a new or modified row.

   * - le: max provisional xr
     - This is the maximum number of provisional transaction records that were stored
       on disk in a new or modified row.

   * - le: expanded
     - This is the number of times that an expanded memory mechanism was used to
       store a new or modified row on disk.

   * - le: max memsize
     - This is the maximum number of bytes that were stored on disk as a new or
       modified row. This is the maximum uncompressed size of any row stored in
       *TokuDB* that was created or modified since the server started.

   * - le: size of leafentries before garbage collection (during message application)
     - Total number of bytes of leaf nodes data before performing garbage collection
       for non-flush events.

   * - le: size of leafentries after garbage collection (during message application)
     - Total number of bytes of leaf nodes data after performing garbage collection
       for non-flush events.

   * - le: size of leafentries before garbage collection (outside message application)
     - Total number of bytes of leaf nodes data before performing garbage collection
       for flush events.

   * - le: size of leafentries after garbage collection (outside message application)
     - Total number of bytes of leaf nodes data after performing garbage collection
       for flush events.

   * - checkpoint: period
     - This is the interval in seconds between the end of an automatic checkpoint and
       the beginning of the next automatic checkpoint.

   * - checkpoint: footprint
     - Where the database is in the checkpoint process.

   * - checkpoint: last checkpoint began
     - This is the time the last checkpoint began. If a checkpoint is currently in
       progress, then this time may be later than the time the last checkpoint
       completed.

       .. note::

	  If no checkpoint has ever taken place, then this value will be ``Dec 31,
	  1969`` on Linux hosts.

   * - checkpoint: last complete checkpoint began
     - This is the time the last complete checkpoint started. Any data that changed
       after this time will not be captured in the checkpoint.

   * - checkpoint: last complete checkpoint ended
     - This is the time the last complete checkpoint ended.

   * - checkpoint: time spent during checkpoint (begin and end phases)
     - Time (in seconds) required to complete all checkpoints.

   * - checkpoint: time spent during last checkpoint (begin and end phases)
     - Time (in seconds) required to complete the last checkpoint.

   * - checkpoint: last complete checkpoint LSN
     - This is the Log Sequence Number of the last complete checkpoint.

   * - checkpoint: checkpoints taken
     - This is the number of complete checkpoints that have been taken.

   * - checkpoint: checkpoints failed
     - This is the number of checkpoints that have failed for any reason.

   * - checkpoint: waiters now
     - This is the current number of threads simultaneously waiting for the
       checkpoint-safe lock to perform a checkpoint.

   * - checkpoint: waiters max
     - This is the maximum number of threads ever simultaneously waiting for the
       checkpoint-safe lock to perform a checkpoint.

   * - checkpoint: non-checkpoint client wait on mo lock
     - The number of times a non-checkpoint client thread waited for the
       multi-operation lock.

   * - checkpoint: non-checkpoint client wait on cs lock
     - The number of times a non-checkpoint client thread waited for the
       checkpoint-safe lock.

   * - checkpoint: checkpoint begin time
     - Cumulative time (in microseconds) required to mark all dirty nodes as
       pending a checkpoint.

   * - checkpoint: long checkpoint begin time
     - The total time, in microseconds, of long checkpoint begins. A long checkpoint
       begin is one taking more than 1 second.

   * - checkpoint: long checkpoint begin count
     - The total number of times a checkpoint begin took more than 1 second.

   * - checkpoint: checkpoint end time
     - The time spent in checkpoint end operation in seconds.

   * - checkpoint: long checkpoint end time
     - The time spent in checkpoint end operation in seconds.

   * - checkpoint: long checkpoint end count
     - This is the count of end_checkpoint operations that exceeded 1 minute.

   * - cachetable: miss
     - This is a count of how many times the application was unable to access your
       data in the internal cache.

   * - cachetable: miss time
     - This is the total time, in microseconds, of how long the database has had to
       wait for a disk read to complete.

   * - cachetable: prefetches
     - This is the total number of times that a block of memory has been prefetched
       into the database's cache. Data is prefetched when the database's algorithms
       determine that a block of memory is likely to be accessed by the application.

   * - cachetable: size current
     - This shows how much of the uncompressed data, in bytes, is currently in the
       database's internal cache.

   * - cachetable: size limit
     - This shows how much of the uncompressed data, in bytes, will fit in the
       database's internal cache.

   * - cachetable: size writing
     - This is the number of bytes that are currently queued up to be written to
       disk.

   * - cachetable: size nonleaf
     - This shows the amount of memory, in bytes, the current set of non-leaf nodes
       occupy in the cache.

   * - cachetable: size leaf
     - This shows the amount of memory, in bytes, the current set of (decompressed)
       leaf nodes occupy in the cache.

   * - cachetable: size rollback
     - This shows the rollback nodes size, in bytes, in the cache.

   * - cachetable: size cachepressure
     - This shows the number of bytes causing cache pressure (the sum of buffers and
       work done counters), helps to understand if cleaner threads are keeping up
       with workload. It should really be looked at as more of a value to use in a
       ratio of cache pressure / cache table size. The closer that ratio evaluates to
       1, the higher the cache pressure.

   * - cachetable: size currently cloned data for checkpoint
     - Amount of memory, in bytes, currently used for cloned nodes. During the
       checkpoint operation, dirty nodes are cloned prior to
       serialization/compression, then written to disk. After which, the memory for
       the cloned block is returned for re-use.

   * - cachetable: evictions
     - Number of blocks evicted from cache.

   * - cachetable: cleaner executions
     - Total number of times the cleaner thread loop has executed.

   * - cachetable: cleaner period
     - *TokuDB* includes a cleaner thread that optimizes indexes in the background.
       This variable is the time, in seconds, between the completion of a group of
       cleaner operations and the beginning of the next group of cleaner operations.
       The cleaner operations run on a background thread performing work that does
       not need to be done on the client thread.

   * - cachetable: cleaner iterations
     - This is the number of cleaner operations that are performed every cleaner
       period.

   * - cachetable: number of waits on cache pressure
     - The number of times a thread was stalled due to cache pressure.

   * - cachetable: time waiting on cache pressure
     - Total time, in microseconds, waiting on cache pressure to subside.

   * - cachetable: number of long waits on cache pressure
     - The number of times a thread was stalled for more than 1 second due to cache
       pressure.

   * - cachetable: long time waiting on cache pressure
     - Total time, in microseconds, waiting on cache pressure to subside for more
       than 1 second.

   * - cachetable: client pool: number of threads in pool
     - The number of threads in the client thread pool.

   * - cachetable: client pool: number of currently active threads in pool
     - The number of currently active threads in the client thread pool.

   * - cachetable: client pool: number of currently queued work items
     - The number of currently queued work items in the client thread pool.

   * - cachetable: client pool: largest number of queued work items
     - The largest number of queued work items in the client thread pool.

   * - cachetable: client pool: total number of work items processed
     - The total number of work items processed in the client thread pool.

   * - cachetable: client pool: total execution time of processing work items
     - The total execution time of processing work items in the client thread pool.

   * - cachetable: cachetable pool: number of threads in pool
     - The number of threads in the cachetable thread pool.

   * - cachetable: cachetable pool: number of currently active threads in pool
     - The number of currently active threads in the cachetable thread pool.

   * - cachetable: cachetable pool: number of currently queued work items
     - The number of currently queued work items in the cachetable thread pool.

   * - cachetable: cachetable pool: largest number of queued work items
     - The largest number of queued work items in the cachetable thread pool.

   * - cachetable: cachetable pool: total number of work items processed
     - The total number of work items processed in the cachetable thread pool.

   * - cachetable: cachetable pool: total execution time of processing work items
     - The total execution time of processing work items in the cachetable thread
       pool.

   * - cachetable: checkpoint pool: number of threads in pool
     - The number of threads in the checkpoint thread pool.

   * - cachetable: checkpoint pool: number of currently active threads in pool
     - The number of currently active threads in the checkpoint thread pool.

   * - cachetable: checkpoint pool: number of currently queued work items
     - The number of currently queued work items in the checkpoint thread pool.

   * - cachetable: checkpoint pool: largest number of queued work items
     - The largest number of queued work items in the checkpoint thread pool.

   * - cachetable: checkpoint pool: total number of work items processed
     - The total number of work items processed in the checkpoint thread pool.

   * - cachetable: checkpoint pool: total execution time of processing work items
     - The total execution time of processing work items in the checkpoint thread
       pool.

   * - locktree: memory size
     - The amount of memory, in bytes, that the locktree is currently using.

   * - locktree: memory size limit
     - The maximum amount of memory, in bytes, that the locktree is allowed to use.

   * - locktree: number of times lock escalation ran
     - Number of times the locktree needed to run lock escalation to reduce its
       memory footprint.

   * - locktree: time spent running escalation (seconds)
     - Total number of seconds spent performing locktree escalation.

   * - locktree: latest post-escalation memory size
     - Size of the locktree, in bytes, after most current locktree escalation.

   * - locktree: number of locktrees open now
     - Number of locktrees currently open.

   * - locktree: number of pending lock requests
     - Number of requests waiting for a lock grant.

   * - locktree: number of locktrees eligible for the STO
     - Number of locktrees eligible for "Single Transaction Optimizations". ``STO``
       optimization are behaviors that can happen within the locktree when there is
       exactly one transaction active within the locktree. This is a not a useful
       value for a regular user to use for any purpose.

   * - locktree: number of times a locktree ended the STO early
     - Total number of times a "single transaction optimization" was ended early due
       to another trans- action starting.

   * - locktree: time spent ending the STO early (seconds)
     - Total number of seconds ending "Single Transaction Optimizations". ``STO``
       optimization are behaviors that can happen within the locktree when there is
       exactly one transaction active within the locktree. This is a not a useful
       value for a regular user to use for any purpose.

   * - locktree: number of wait locks
     - Number of times that a lock request could not be acquired because of a
       conflict with some other transaction.

   * - locktree: time waiting for locks
     - Total time, in microseconds, spend by some client waiting for a lock conflict
       to be resolved.

   * - locktree: number of long wait locks
     - Number of lock waits greater than 1 second in duration.

   * - locktree: long time waiting for locks
     - Total time, in microseconds, of the long waits.

   * - locktree: number of lock timeouts
     - Count of the number of times that a lock request timed out.

   * - locktree: number of waits on lock escalation
     - When the sum of the sizes of locks taken reaches the lock tree limit, we run
       lock escalation on a background thread. The clients threads need to wait for
       escalation to consolidate locks and free up memory. This counter counts the
       number of times a client thread has to wait on lock escalation.

   * - locktree: time waiting on lock escalation
     - Total time, in microseconds, that a client thread spent waiting for lock
       escalation to free up memory.

   * - locktree: number of long waits on lock escalation
     - Number of times that a client thread had to wait on lock escalation and the
       wait time was greater than 1 second.

   * - locktree: long time waiting on lock escalation
     - Total time, in microseconds, of the long waits for lock escalation to free up
       memory.

   * - ft: dictionary updates
     - This is the total number of rows that have been updated in all primary and
       secondary indexes combined, if those updates have been done with a separate
       recovery log entry per index.

   * - ft: dictionary broadcast updates
     - This is the number of broadcast updates that have been successfully performed.
       A broadcast update is an update that affects all rows in a dictionary.

   * - ft: descriptor set
     - This is the number of time a descriptor was updated when the entire dictionary
       was updated (for example, when the schema has been changed).

   * - ft: messages ignored by leaf due to msn
     - The number of messages that were ignored by a leaf because it had already been
       applied.

   * - ft: total search retries due to TRY AGAIN
     - Total number of search retries due to TRY AGAIN. Internal value that is no use
       to anyone other than a developer debugging a specific query/search issue.

   * - ft: searches requiring more tries than the height of the tree
     - Number of searches that required more tries than the height of the tree.

   * - ft: searches requiring more tries than the height of the tree plus three
     - Number of searches that required more tries than the height of the tree plus
       three.

   * - ft: leaf nodes flushed to disk (not for checkpoint)
     - Number of leaf nodes flushed to disk, not for checkpoint.

   * - ft: leaf nodes flushed to disk (not for checkpoint) (bytes)
     - Number of bytes of leaf nodes flushed to disk, not for checkpoint.

   * - ft: leaf nodes flushed to disk (not for checkpoint) (uncompressed bytes)
     - Number of bytes of leaf nodes flushed to disk, not for checkpoint.

   * - ft: leaf nodes flushed to disk (not for checkpoint) (seconds)
     - Number of seconds waiting for IO when writing leaf nodes flushed to disk, not
       for checkpoint.

   * - ft: nonleaf nodes flushed to disk (not for checkpoint)
     - Number of non-leaf nodes flushed to disk, not for checkpoint.

   * - ft: nonleaf nodes flushed to disk (not for checkpoint) (bytes)
     - Number of bytes of non-leaf nodes flushed to disk, not for checkpoint.

   * - ft: nonleaf nodes flushed to disk (not for checkpoint) (uncompressed bytes)
     - Number of uncompressed bytes of non-leaf nodes flushed to disk, not for
       checkpoint.

   * - ft: nonleaf nodes flushed to disk (not for checkpoint) (seconds)
     - Number of seconds waiting for I/O when writing non-leaf nodes flushed to disk,
       not for checkpoint.

   * - ft: leaf nodes flushed to disk (for checkpoint)
     - Number of leaf nodes flushed to disk for checkpoint.

   * - ft: leaf nodes flushed to disk (for checkpoint) (bytes)
     - Number of bytes of leaf nodes flushed to disk for checkpoint.

   * - ft: leaf nodes flushed to disk (for checkpoint) (uncompressed bytes)
     - Number of uncompressed bytes of leaf nodes flushed to disk for checkpoint.

   * - ft: leaf nodes flushed to disk (for checkpoint) (seconds)
     - Number of seconds waiting for IO when writing leaf nodes flushed to disk for
       checkpoint.

   * - ft: nonleaf nodes flushed to disk (for checkpoint)
     - Number of non-leaf nodes flushed to disk for checkpoint.

   * - ft: nonleaf nodes flushed to disk (for checkpoint) (bytes)
     - Number of bytes of non-leaf nodes flushed to disk for checkpoint.

   * - ft: nonleaf nodes flushed to disk (for checkpoint) (uncompressed bytes)
     - Number of uncompressed bytes of non-leaf nodes flushed to disk for checkpoint.

   * - ft: nonleaf nodes flushed to disk (for checkpoint) (seconds)
     - Number of seconds waiting for IO when writing non-leaf nodes flushed to disk
       for checkpoint.

   * - ft: uncompressed / compressed bytes written (leaf)
     - Ratio of uncompressed bytes (in-memory) to compressed bytes (on-disk) for leaf
       nodes.

   * - ft: uncompressed / compressed bytes written (nonleaf)
     - Ratio of uncompressed bytes (in-memory) to compressed bytes (on-disk) for
       non-leaf nodes.

   * - ft: uncompressed / compressed bytes written (overall)
     - Ratio of uncompressed bytes (in-memory) to compressed bytes (on-disk) for all
       nodes.

   * - ft: nonleaf node partial evictions
     - The number of times a partition of a non-leaf node was evicted from the cache.

   * - ft: nonleaf node partial evictions (bytes)
     - The number of bytes freed by evicting partitions of non-leaf nodes from the
       cache.

   * - ft: leaf node partial evictions
     - The number of times a partition of a leaf node was evicted from the cache.

   * - ft: leaf node partial evictions (bytes)
     - The number of bytes freed by evicting partitions of leaf nodes from the cache.

   * - ft: leaf node full evictions
     - The number of times a full leaf node was evicted from the cache.

   * - ft: leaf node full evictions (bytes)
     - The number of bytes freed by evicting full leaf nodes from the cache.

   * - ft: nonleaf node full evictions (bytes)
     - The number of bytes freed by evicting full non-leaf nodes from the cache.

   * - ft: nonleaf node full evictions
     - The number of times a full non-leaf node was evicted from the cache.

   * - ft: leaf nodes created
     - Number of created leaf nodes .

   * - ft: nonleaf nodes created
     - Number of created non-leaf nodes.

   * - ft: leaf nodes destroyed
     - Number of destroyed leaf nodes.

   * - ft: nonleaf nodes destroyed
     - Number of destroyed non-leaf nodes.

   * - ft: bytes of messages injected at root (all trees)
     - Amount of messages, in bytes, injected at root (for all trees).

   * - ft: bytes of messages flushed from h1 nodes to leaves
     - Amount of messages, in bytes, flushed from ``h1`` nodes to leaves.

   * - ft: bytes of messages currently in trees (estimate)
     - Amount of messages, in bytes, currently in trees (estimate).

   * - ft: messages injected at root
     - Number of messages injected at root node of a tree.

   * - ft: broadcast messages injected at root
     - Number of broadcast messages injected at root node of a tree.

   * - ft: basements decompressed as a target of a query
     - Number of basement nodes decompressed for queries.

   * - ft: basements decompressed for prelocked range
     - Number of basement nodes decompressed by queries aggressively.

   * - ft: basements decompressed for prefetch
     - Number of basement nodes decompressed by a prefetch thread.

   * - ft: basements decompressed for write
     - Number of basement nodes decompressed for writes.

   * - ft: buffers decompressed as a target of a query
     - Number of buffers decompressed for queries.

   * - ft: buffers decompressed for prelocked range
     - Number of buffers decompressed by queries aggressively.

   * - ft: buffers decompressed for prefetch
     - Number of buffers decompressed by a prefetch thread.

   * - ft: buffers decompressed for write
     - Number of buffers decompressed for writes.

   * - ft: pivots fetched for query
     - Number of pivot nodes fetched for queries.

   * - ft: pivots fetched for query (bytes)
     - Number of bytes of pivot nodes fetched for queries.

   * - ft: pivots fetched for query (seconds)
     - Number of seconds waiting for I/O when fetching pivot nodes for queries.

   * - ft: pivots fetched for prefetch
     - Number of pivot nodes fetched by a prefetch thread.

   * - ft: pivots fetched for prefetch (bytes)
     - Number of bytes of pivot nodes fetched by a prefetch thread.

   * - ft: pivots fetched for prefetch (seconds)
     - Number seconds waiting for I/O when fetching pivot nodes by a prefetch thread.

   * - ft: pivots fetched for write
     - Number of pivot nodes fetched for writes.

   * - ft: pivots fetched for write (bytes)
     - Number of bytes of pivot nodes fetched for writes.

   * - ft: pivots fetched for write (seconds)
     - Number of seconds waiting for I/O when fetching pivot nodes for writes.

   * - ft: basements fetched as a target of a query
     - Number of basement nodes fetched from disk for queries.

   * - ft: basements fetched as a target of a query (bytes)
     - Number of basement node bytes fetched from disk for queries.

   * - ft: basements fetched as a target of a query (seconds)
     - Number of seconds waiting for IO when fetching basement nodes from disk for
       queries.

   * - ft: basements fetched for prelocked range
     - Number of basement nodes fetched from disk aggressively.

   * - ft: basements fetched for prelocked range (bytes)
     - Number of basement node bytes fetched from disk aggressively.

   * - ft: basements fetched for prelocked range (seconds)
     - Number of seconds waiting for I/O when fetching basement nodes from disk
       aggressively.

   * - ft: basements fetched for prefetch
     - Number of basement nodes fetched from disk by a prefetch thread.

   * - ft: basements fetched for prefetch (bytes)
     - Number of basement node bytes fetched from disk by a prefetch thread.

   * - ft: basements fetched for prefetch (seconds)
     - Number of seconds waiting for I/O when fetching basement nodes from disk by a
       prefetch thread.

   * - ft: basements fetched for write
     - Number of basement nodes fetched from disk for writes.

   * - ft: basements fetched for write (bytes)
     - Number of basement node bytes fetched from disk for writes.

   * - ft: basements fetched for write (seconds)
     - Number of seconds waiting for I/O when fetching basement nodes from disk for
       writes.

   * - ft: buffers fetched as a target of a query
     - Number of buffers fetched from disk for queries.

   * - ft: buffers fetched as a target of a query (bytes)
     - Number of buffer bytes fetched from disk for queries.

   * - ft: buffers fetched as a target of a query (seconds)
     - Number of seconds waiting for I/O when fetching buffers from disk for queries.

   * - ft: buffers fetched for prelocked range
     - Number of buffers fetched from disk aggressively.

   * - ft: buffers fetched for prelocked range (bytes)
     - Number of buffer bytes fetched from disk aggressively.

   * - ft: buffers fetched for prelocked range (seconds)
     - Number of seconds waiting for I/O when fetching buffers from disk
       aggressively.

   * - ft: buffers fetched for prefetch
     - Number of buffers fetched from disk by a prefetch thread.

   * - ft: buffers fetched for prefetch (bytes)
     - Number of buffer bytes fetched from disk by a prefetch thread.

   * - ft: buffers fetched for prefetch (seconds)
     - Number of seconds waiting for I/O when fetching buffers from disk by a
       prefetch thread.

   * - ft: buffers fetched for write
     - Number of buffers fetched from disk for writes.

   * - ft: buffers fetched for write (bytes)
     - Number of buffer bytes fetched from disk for writes.

   * - ft: buffers fetched for write (seconds)
     - Number of seconds waiting for I/O when fetching buffers from disk for writes.

   * - ft: leaf compression to memory (seconds)
     - Total time, in seconds, spent compressing leaf nodes.

   * - ft: leaf serialization to memory (seconds)
     - Total time, in seconds, spent serializing leaf nodes.

   * - ft: leaf decompression to memory (seconds)
     - Total time, in seconds, spent decompressing leaf nodes.

   * - ft: leaf deserialization to memory (seconds)
     - Total time, in seconds, spent deserializing leaf nodes.

   * - ft: nonleaf compression to memory (seconds)
     - Total time, in seconds, spent compressing non leaf nodes.

   * - ft: nonleaf serialization to memory (seconds)
     - Total time, in seconds, spent serializing non leaf nodes.

   * - ft: nonleaf decompression to memory (seconds)
     - Total time, in seconds, spent decompressing non leaf nodes.

   * - ft: nonleaf deserialization to memory (seconds)
     - Total time, in seconds, spent deserializing non leaf nodes.

   * - ft: promotion: roots split
     - Number of times the root split during promotion.

   * - ft: promotion: leaf roots injected into
     - Number of times a message stopped at a root with height ``0``.

   * - ft: promotion: h1 roots injected into
     - Number of times a message stopped at a root with height ``1``.

   * - ft: promotion: injections at depth 0
     - Number of times a message stopped at depth ``0``.

   * - ft: promotion: injections at depth 1
     - Number of times a message stopped at depth ``1``.

   * - ft: promotion: injections at depth 2
     - Number of times a message stopped at depth ``2``.

   * - ft: promotion: injections at depth 3
     - Number of times a message stopped at depth ``3``.

   * - ft: promotion: injections lower than depth 3
     - Number of times a message was promoted past depth ``3``.

   * - ft: promotion: stopped because of a nonempty buffer
     - Number of times a message stopped because it reached a nonempty buffer.

   * - ft: promotion: stopped at height 1
     - Number of times a message stopped because it had reached height ``1``.

   * - ft: promotion: stopped because the child was locked or not at all in memory
     - Number of times promotion was stopped because the child node was locked or not
       at all in memory. This is a not a useful value for a regular user to use for
       any purpose.

   * - ft: promotion: stopped because the child was not fully in memory
     - Number of times promotion was stopped because the child node was not at all in
       memory. This is a not a useful value for a normal user to use for any purpose.

   * - ft: promotion: stopped anyway, after locking the child
     - Number of times a message stopped before a child which had been locked.

   * - ft: basement nodes deserialized with fixed-keysize
     - The number of basement nodes deserialized where all keys had the same size,
       leaving the basement in a format that is optimal for in-memory workloads.

   * - ft: basement nodes deserialized with variable-keysize
     - The number of basement nodes deserialized where all keys did not have the same
       size, and thus ineligible for an in-memory optimization.

   * - ft: promotion: succeeded in using the rightmost leaf shortcut
     - Rightmost insertions used the rightmost-leaf pin path, meaning that the
       Fractal Tree index detected and properly optimized rightmost inserts.

   * - ft: promotion: tried the rightmost leaf shortcut but failed (out-of-bounds)
     - Rightmost insertions did not use the rightmost-leaf pin path, due to the
       insert not actually being into the rightmost leaf node.

   * - ft: promotion: tried the rightmost leaf shortcut but failed (child reactive)
     - Rightmost insertions did not use the rightmost-leaf pin path, due to the
       leaf being too large (needed to split).

   * - ft: cursor skipped deleted leaf entries
     - Number of leaf entries skipped during search/scan because the result of
       message application and reconciliation of the leaf entry MVCC stack reveals
       that the leaf entry is deleted in the current transactions view. It is a good
       indicator that there might be excessive garbage in a tree if a range scan
       seems to take too long.

   * - ft flusher: total nodes potentially flushed by cleaner thread
     - Total number of nodes whose buffers are potentially flushed by cleaner thread.

   * - ft flusher: height-one nodes flushed by cleaner thread
     - Number of nodes of height one whose message buffers are flushed by cleaner
       thread.

   * - ft flusher: height-greater-than-one nodes flushed by cleaner thread
     - Number of nodes of height > 1 whose message buffers are flushed by cleaner
       thread.

   * - ft flusher: nodes cleaned which had empty buffers
     - Number of nodes that are selected by cleaner, but whose buffers are empty.

   * - ft flusher: nodes dirtied by cleaner thread
     - Number of nodes that are made dirty by the cleaner thread.

   * - ft flusher: max bytes in a buffer flushed by cleaner thread
     - Max number of bytes in message buffer flushed by cleaner thread.

   * - ft flusher: min bytes in a buffer flushed by cleaner thread
     - Min number of bytes in message buffer flushed by cleaner thread.

   * - ft flusher: total bytes in buffers flushed by cleaner thread
     - Total number of bytes in message buffers flushed by cleaner thread.

   * - ft flusher: max workdone in a buffer flushed by cleaner thread
     - Max workdone value of any message buffer flushed by cleaner thread.

   * - ft flusher: min workdone in a buffer flushed by cleaner thread
     - Min workdone value of any message buffer flushed by cleaner thread.

   * - ft flusher: total workdone in buffers flushed by cleaner thread
     - Total workdone value of message buffers flushed by cleaner thread.

   * - ft flusher: times cleaner thread tries to merge a leaf
     - The number of times the cleaner thread tries to merge a leaf.

   * - ft flusher: cleaner thread leaf merges in progress
     - The number of cleaner thread leaf merges in progress.

   * - ft flusher: cleaner thread leaf merges successful
     - The number of times the cleaner thread successfully merges a leaf.

   * - ft flusher: nodes dirtied by cleaner thread leaf merges
     - The number of nodes dirtied by the "flush from root" process to merge a leaf node.

   * - ft flusher: total number of flushes done by flusher threads or cleaner threads
     - Total number of flushes done by flusher threads or cleaner threads.

   * - ft flusher: number of in memory flushes
     - Number of in-memory flushes.

   * - ft flusher: number of flushes that read something off disk
     - Number of flushes that had to read a child (or part) off disk.

   * - ft flusher: number of flushes that triggered another flush in child
     - Number of flushes that triggered another flush in the child.

   * - ft flusher: number of flushes that triggered 1 cascading flush
     - Number of flushes that triggered 1 cascading flush.

   * - ft flusher: number of flushes that triggered 2 cascading flushes
     - Number of flushes that triggered 2 cascading flushes.

   * - ft flusher: number of flushes that triggered 3 cascading flushes
     - Number of flushes that triggered 3 cascading flushes.

   * - ft flusher: number of flushes that triggered 4 cascading flushes
     - Number of flushes that triggered 4 cascading flushes.

   * - ft flusher: number of flushes that triggered 5 cascading flushes
     - Number of flushes that triggered 5 cascading flushes.

   * - ft flusher: number of flushes that triggered over 5 cascading flushes
     - Number of flushes that triggered more than 5 cascading flushes.

   * - ft flusher: leaf node splits
     - Number of leaf nodes split.

   * - ft flusher: nonleaf node splits
     - Number of non-leaf nodes split.

   * - ft flusher: leaf node merges
     - Number of times leaf nodes are merged.

   * - ft flusher: nonleaf node merges
     - Number of times non-leaf nodes are merged.

   * - ft flusher: leaf node balances
     - Number of times a leaf node is balanced.

   * - hot: operations ever started
     - This variable shows the number of hot operations started (``OPTIMIZE TABLE``).
       This is a not a useful value for a regular user to use for any purpose.

   * - hot: operations successfully completed
     - The number of hot operations that have successfully completed (``OPTIMIZE
       TABLE``). This is a not a useful value for a regular user to use for any
       purpose.

   * - hot: operations aborted
     - The number of hot operations that have been aborted (``OPTIMIZE TABLE``).
       This is a not a useful value for a regular user to use for any purpose.

   * - hot: max number of flushes from root ever required to optimize a tree
     - The maximum number of flushes from the root ever required to optimize a tree.

   * - txn: begin
     - This is the number of transactions that have been started.

   * - txn: begin read only
     - Number of read only transactions started.

   * - txn: successful commits
     - This is the total number of transactions that have been committed.

   * - txn: aborts
     - This is the total number of transactions that have been aborted.

   * - logger: next LSN
     - This is the next unassigned Log Sequence Number. It will be assigned to the
       next entry in the recovery log.

   * - logger: writes
     - Number of times the logger has written to disk.

   * - logger: writes (bytes)
     - Number of bytes the logger has written to disk.

   * - logger: writes (uncompressed bytes)
     - Number of uncompressed the logger has written to disk.

   * - logger: writes (seconds)
     - Number of seconds waiting for I/O when writing logs to disk.

   * - logger: number of long logger write operations
     - Number of times a logger write operation required 100ms or more.

   * - indexer: number of indexers successfully created
     - This is the number of times one of our internal objects, a indexer, has been
       created.

   * - indexer: number of calls to toku_indexer_create_indexer() that failed
     - This is the number of times a indexer was requested but could not be created.

   * - indexer: number of calls to indexer->build() succeeded
     - This is the total number of times that indexes were created using a indexer.

   * - indexer: number of calls to indexer->build() failed
     - This is the total number of times that indexes were unable to be created using a indexer

   * - indexer: number of calls to indexer->close() that succeeded
     - This is the number of indexers that successfully created the requested index(es).

   * - indexer: number of calls to indexer->close() that failed
     - This is the number of indexers that were unable to create the requested index(es).

   * - indexer: number of calls to indexer->abort()
     - This is the number of indexers that were aborted.

   * - indexer: number of indexers currently in existence
     - This is the number of indexers that currently exist.

   * - indexer: max number of indexers that ever existed simultaneously
     - This is the maximum number of indexers that ever existed simultaneously.

   * - loader: number of loaders successfully created
     - This is the number of times one of our internal objects, a loader, has been
       created.

   * - loader: number of calls to toku_loader_create_loader() that failed
     - This is the number of times a loader was requested but could not be created.

   * - loader: number of calls to loader->put() succeeded
     - This is the total number of rows that were inserted using a loader.

   * - loader: number of calls to loader->put() failed
     - This is the total number of rows that were unable to be inserted using a
       loader.

   * - loader: number of calls to loader->close() that succeeded
     - This is the number of loaders that successfully created the requested table.

   * - loader: number of calls to loader->close() that failed
     - This is the number of loaders that were unable to create the requested table.

   * - loader: number of calls to loader->abort()
     - This is the number of loaders that were aborted.

   * - loader: number of loaders currently in existence
     - This is the number of loaders that currently exist.

   * - loader: max number of loaders that ever existed simultaneously
     - This is the maximum number of loaders that ever existed simultaneously.

   * - memory: number of malloc operations
     - Number of calls to ``malloc()``.

   * - memory: number of free operations
     - Number of calls to ``free()``.

   * - memory: number of realloc operations
     - Number of calls to ``realloc()``.

   * - memory: number of malloc operations that failed
     - Number of failed calls to ``malloc()``.

   * - memory: number of realloc operations that failed
     - Number of failed calls to ``realloc()``.

   * - memory: number of bytes requested
     - Total number of bytes requested from memory allocator library.

   * - memory: number of bytes freed
     - Total number of bytes allocated from memory allocation library that have been
       freed (used - freed = bytes in use).

   * - memory: largest attempted allocation size
     - Largest number of bytes in a single successful ``malloc()`` operation.

   * - memory: size of the last failed allocation attempt
     - Largest number of bytes in a single failed ``malloc()`` operation.

   * - memory: number of bytes used (requested + overhead)
     - Total number of bytes allocated by memory allocator library.

   * - memory: estimated maximum memory footprint
     - Maximum memory footprint of the storage engine,
       the max value of (used - freed).

   * - memory: mallocator version
     - Version string from in-use memory allocator.

   * - memory: mmap threshold
     - The threshold for malloc to use mmap.

   * - filesystem: ENOSPC redzone state
     - The state of how much disk space exists with respect to the red zone value.
       Redzone is space greater than :ref:`tokudb_fs_reserve_percent` and less
       than full disk.

       Valid values are:

       :0: Space is available
       :1: Warning, with 2x of redzone value. Operations are allowed, but engine
	   status prints a warning.
       :2: In red zone, insert operations are blocked
       :3: All operations are blocked

   * - filesystem: threads currently blocked by full disk
     - This is the number of threads that are currently blocked because they are
       attempting to write to a full disk. This is normally zero. If this value is
       non-zero, then a warning will appear in the "disk free space" field.

   * - filesystem: number of operations rejected by enospc prevention (red zone)
     - This is the number of database inserts that have been rejected because the
       amount of disk free space was less than the reserve.

   * - filesystem: most recent disk full
     - This is the most recent time when the disk file system was entirely full. If
       the disk has never been full, then this value will be ``Dec 31, 1969`` on
       Linux hosts.

   * - filesystem: number of write operations that returned ENOSPC
     - This is the number of times that an attempt to write to disk failed because
       the disk was full. If the disk is full, this number will continue increasing
       until space is available.

   * - filesystem: fsync time
     - This the total time, in microseconds, used to fsync to disk.

   * - filesystem: fsync count
     - This is the total number of times the database has flushed the operating
       system's file buffers to disk.

   * - filesystem: long fsync time
     - This the total time, in microseconds, used to fsync to disk when the operation
       required more than 1 second.

   * - filesystem: long fsync count
     - This is the total number of times the database has flushed the operating
       system's file buffers to disk and this operation required more than 1 second.

   * - context: tree traversals blocked by a full fetch
     - Number of times node ``rwlock`` contention was observed while pinning nodes
       from root to leaf because of a full fetch.

   * - context: tree traversals blocked by a partial fetch
     - Number of times node ``rwlock`` contention was observed while pinning nodes
       from root to leaf because of a partial fetch.

   * - context: tree traversals blocked by a full eviction
     - Number of times node ``rwlock`` contention was observed while pinning nodes
       from root to leaf because of a full eviction.

   * - context: tree traversals blocked by a partial eviction
     - Number of times node ``rwlock`` contention was observed while pinning nodes
       from root to leaf because of a partial eviction.

   * - context: tree traversals blocked by a message injection
     - Number of times node ``rwlock`` contention was observed while pinning nodes
       from root to leaf because of message injection.

   * - context: tree traversals blocked by a message application
     - Number of times node ``rwlock`` contention was observed while pinning nodes
       from root to leaf because of message application (applying fresh ancestors
       messages to a basement node).

   * - context: tree traversals blocked by a flush
     - Number of times node ``rwlock`` contention was observed while pinning nodes
       from root to leaf because of a buffer flush from parent to child.

   * - context: tree traversals blocked by a the cleaner thread
     - Number of times node ``rwlock`` contention was observed while pinning nodes
       from root to leaf because of a cleaner thread.

   * - context: tree traversals blocked by something uninstrumented
     - Number of times node ``rwlock`` contention was observed while pinning nodes
       from root to leaf because of something uninstrumented.

   * - context: promotion blocked by a full fetch (should never happen)
     - Number of times node ``rwlock`` contention was observed within promotion
       (pinning nodes from root to the buffer to receive the message) because of a
       full fetch.

   * - context: promotion blocked by a partial fetch (should never happen)
     - Number of times node ``rwlock`` contention was observed within promotion
       (pinning nodes from root to the buffer to receive the message) because of a
       partial fetch.

   * - context: promotion blocked by a full eviction (should never happen)
     - Number of times node ``rwlock`` contention was observed within promotion
       (pinning nodes from root to the buffer to receive the message) because of a
       full eviction.

   * - context: promotion blocked by a partial eviction (should never happen)
     - Number of times node ``rwlock`` contention was observed within promotion
       (pinning nodes from root to the buffer to receive the message) because of a
       partial eviction.

   * - context: promotion blocked by a message injection
     - Number of times node ``rwlock`` contention was observed within promotion
       (pinning nodes from root to the buffer to receive the message) because of
       message injection.

   * - context: promotion blocked by a message application
     - Number of times node ``rwlock`` contention was observed within promotion
       (pinning nodes from root to the buffer to receive the message) because of
       message application (applying fresh ancestors messages to a basement node).

   * - context: promotion blocked by a flush
     - Number of times node ``rwlock`` contention was observed within promotion
       (pinning nodes from root to the buffer to receive the message) because of a
       buffer flush from parent to child.

   * - context: promotion blocked by the cleaner thread
     - Number of times node ``rwlock`` contention was observed within promotion
       (pinning nodes from root to the buffer to receive the message) because of a
       cleaner thread.

   * - context: promotion blocked by something uninstrumented
     - Number of times node ``rwlock`` contention was observed within promotion
       (pinning nodes from root to the buffer to receive the message) because of
       something uninstrumented.

   * - context: something uninstrumented blocked by something uninstrumented
     - Number of times node ``rwlock`` contention was observed for an uninstrumented
       process because of something uninstrumented.

   * - handlerton: primary key bytes inserted
     - Total number of bytes inserted into all primary key indexes.
