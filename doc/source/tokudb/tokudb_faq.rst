.. _tokudb_faq:

==========================
Frequently Asked Questions
==========================

.. Important:: 

   Starting with :ref:`8.0.28-19`, the TokuDB storage engine is no longer supported. We have removed the storage engine from the installation packages and disabled the storage engine in our binary builds.

   Starting with :ref:`8.0.26-16`, the binary builds and packages include but disable the TokuDB storage engine plugins. The ``tokudb_enabled`` option and the ``tokudb_backup_enabled`` option control the state of the plugins and have a default setting of ``FALSE``. The result of attempting to load the plugins are the plugins fail to initialize and print a deprecation message.

   We recommend :ref:`migrate-myrocks`. To enable the plugins to migrate to another storage engine, set the ``tokudb_enabled`` and ``tokudb_backup_enabled`` options to ``TRUE`` in your ``my.cnf`` file and restart your server instance. Then, you can load the plugins.

   The TokuDB Storage Engine was `declared as deprecated <https://www.percona.com/doc/percona-server/8.0/release-notes/Percona-Server-8.0.13-3.html>`__ in Percona Server for MySQL 8.0. For more information, see the Percona blog post: `Heads-Up: TokuDB Support Changes and Future Removal from Percona Server for MySQL 8.0 <https://www.percona.com/blog/2021/05/21/tokudb-support-changes-and-future-removal-from-percona-server-for-mysql-8-0/>`__.

This section contains frequently asked questions regarding *TokuDB* and related software. 

.. contents::
   :local:
   :depth: 1

Transactional Operations
------------------------

.. rubric:: What transactional operations does TokuDB support?

*TokuDB* supports ``BEGIN TRANSACTION``, ``END TRANSACTION``, ``COMMIT``, ``ROLLBACK``, ``SAVEPOINT``, and ``RELEASE SAVEPOINT``. 

TokuDB and the File System
--------------------------

.. rubric:: How can I determine which files belong to the various tables and indexes in my schemas?

The :ref:`tokudb_file_map` plugin lists all Fractal Tree Indexes and their corresponding data files. The ``internal_file_name`` is the actual file name (in the data folder).

.. code-block:: sql

   mysql> SELECT * FROM information_schema.tokudb_file_map;

   +--------------------------+---------------------------------------+---------------+-------------+------------------------+
   | dictionary_name          | internal_file_name                    | table_schema  | table_name  | table_dictionary_name  |
   +--------------------------+---------------------------------------+---------------+-------------+------------------------+
   | ./test/tmc-key-idx_col2  | ./_test_tmc_key_idx_col2_a_14.tokudb  | test          | tmc         | key_idx_col2           |
   | ./test/tmc-main          | ./_test_tmc_main_9_14.tokudb          | test          | tmc         | main                   |
   | ./test/tmc-status        | ./_test_tmc_status_8_14.tokudb        | test          | tmc         | status                 |
   +--------------------------+---------------------------------------+---------------+-------------+------------------------+

.. _tokudb_full_disks:

Full Disks
----------

.. rubric:: What happens when the disk system fills up?

The disk system may fill up during bulk load operations, such as ``LOAD DATA IN FILE`` or ``CREATE INDEX``, or during incremental operations like ``INSERT``.

In the bulk case, running out of disk space will cause the statement to fail with ``ERROR 1030 (HY000): Got error 1 from storage engine``. The temporary space used by the bulk loader will be released. If this happens, you can use a separate physical disk for the temporary files (for more information, see :ref:`tokudb_tmp_dir`). If server runs out of free space *TokuDB* will assert the server to prevent data corruption to existing data files.

Otherwise, disk space can run low during non-bulk operations. When available space is below a user-configurable reserve (5% by default) inserts are prevented and transactions that perform inserts are aborted. If the disk becomes completely full then *TokuDB* will freeze until some disk space is made available.

Details about the disk system:

* There is a free-space reserve requirement, which is a user-configurable parameter given as a percentage of the total space in the file system. The default reserve is five percent. This value is available in the global variable :ref:`tokudb_fs_reserve_percent`. We recommend that this reserve be at least half the size of your physical memory.

  *TokuDB* polls the file system every five seconds to determine how much free space is available. If the free space dips below the reserve, then further table inserts are prohibited. Any transaction that attempts to insert rows will be aborted. Inserts are re-enabled when twice the reserve is available in the file system (so freeing a small amount of disk storage will not be sufficient to resume inserts). Warning messages are sent to the system error log when free space dips below twice the reserve and again when free space dips below the reserve.

  Even with inserts prohibited it is still possible for the file system to become completely full. For example this can happen because another storage engine or another application consumes disk space.

* If the file system becomes completely full, then *TokuDB* will freeze. It will not crash, but it will not respond to most SQL commands until some disk space is made available. When *TokuDB* is frozen in this state, it will still respond to the following command:

 .. code-block:: mysql

    SHOW ENGINE TokuDB STATUS;

    Make disk space available will allow the storage engine to continue running, but inserts will still be prohibited until twice the reserve is free.

 .. note:: 
 
   Engine status displays a field indicating if disk free space is above twice the reserve, below twice the reserve, or below the reserve. It will also display a special warning if the disk is completely full.

* In order to make space available on this system you can:

  * Add some disk space to the filesystem.

  * Delete some non-TokuDB files manually.

  * If the disk is not completely full, you may be able to reclaim space by aborting any transactions that are very old. Old transactions can consume large volumes of disk space in the recovery log.

  * If the disk is not completely full, you can drop indexes or drop tables from your *TokuDB* databases.

  * Deleting large numbers of rows from an existing table and then closing the table may free some space, but it may not. Deleting rows may simply leave unused space (available for new inserts) inside *TokuDB* data files rather than shrink the files (internal fragmentation).

The fine print:

* The *TokuDB* storage engine can use up to three separate file systems simultaneously, one each for the data, the recovery log, and the error log. All three are monitored, and if any one of the three falls below the relevant threshold then a warning message will be issued and inserts may be prohibited.

* Warning messages to the error log are not repeated unless available disk space has been above the relevant threshold for at least one minute. This prevents excess messages in the error log if the disk free space is fluctuating around the limit.

* Even if there are no other storage engines or other applications running, it is still possible for *TokuDB* to consume more disk space when operations such as row delete and query are performed, or when checkpoints are taken. This can happen because *TokuDB* can write cached information when it is time-efficient rather than when inserts are issued by the application, because operations in addition to insert (such as delete) create log entries, and also because of internal fragmentation of *TokuDB* data files.

* The :ref:`tokudb_fs_reserve_percent` variable can not be changed once the system has started. It can only be set in :file:`my.cnf` or on the mysqld command line.

Backup
------

.. rubric:: How do I back up a system with TokuDB tables?

Taking backups with :ref:`toku_backup`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

*TokuDB* is capable of performing online backups with :ref:`toku_backup`. To perform a backup, execute ``backup to '/path/to/backup';``. This will create backup of the server and return when complete. The backup can be used by another server using a copy of the binaries on the source server. You can view the progress of the backup by executing ``SHOW PROCESSLIST;``. *TokuBackup* produces a copy of your running *MySQL* server that is consistent at the end time of the backup process. The thread copying files from source to destination can be throttled by setting the :ref:`tokudb_backup_throttle` server variable. For more information check :ref:`toku_backup`.

  The following conditions apply:

  * Currently, *TokuBackup* only supports tables using the *TokuDB* storage engine and the *MyISAM* tables in the ``mysql`` database. 

    .. warning:: You must disable *InnoDB* asynchronous IO if backing up *InnoDB* tables via *TokuBackup* utility. Otherwise you will have inconsistent, unrecoverable backups. The appropriate setting is :ref:`innodb_use_native_aio` to ``0``.

  * Transactional storage engines (*TokuDB* and *InnoDB*) will perform recovery on the backup copy of the database when it is first started.

  * Tables using non-transactional storage engines (*MyISAM*) are not locked during the copy and may report issues when starting up the backup. It is best to avoid operations that modify these tables at the end of a hot backup operation (adding/changing users, stored procedures, etc.).

  * The database is copied locally to the path specified in :file:`/path/to/backup`. This folder must exist, be writable, be empty, and contain enough space for a full copy of the database.

  * *TokuBackup* always makes a backup of the *MySQL* ``datadir`` and optionally the :ref:`tokudb_data_dir`, :ref:`tokudb_log_dir`, and the binary log folder. The latter three are only backed up separately if they are not the same as or contained in the *MySQL* ``datadir``. None of these three folders can be a parent of the *MySQL* ``datadir``.

  * A folder is created in the given backup destination for each of the source folders.

  * No other directory structures are supported. All *InnoDB*, *MyISAM*, and other storage engine files must be within the *MySQL* ``datadir``.

  * *TokuBackup* does not follow symbolic links.

Other options for taking backups
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  *TokuDB* tables are represented in the file system with dictionary files, log files, and metadata files. A consistent copy of all of these files must be made during a backup. Copying the files while they may be modified by a running *MySQL* may result in an inconsistent copy of the database.

  LVM snapshots may be used to get a consistent snapshot of all of the *TokuDB* files. The LVM snapshot may then be backed up at leisure.

  The ``SELECT INTO OUTFILE`` statement or :program:`mysqldump` application may also be used to get a logical backup of the database.

.. rubric:: References

The MySQL 5.5 reference manual describes several backup methods and strategies. In addition, we recommend reading the backup and recovery chapter in the following book:

*High Performance MySQL, 3rd Edition*, by Baron Schwartz, Peter Zaitsev, and Vadim Tkachenko, Copyright 2012, O'Reilly Media.

.. rubric:: Cold Backup
 
When *MySQL* is shut down, a copy of the *MySQL* data directory, the *TokuDB* data directory, and the *TokuDB* log directory can be made. In the simplest configuration, the *TokuDB* files are stored in the *MySQL* data directory with all of other *MySQL* files. One merely has to back up this directory.

.. rubric:: Hot Backup using mylvmbackup

The :program:`mylvmbackup` utility, located on `Launchpad <https://launchpad.net/>`_, works with *TokuDB*. It does all of the magic required to get consistent copies of all of the *MySQL* tables, including *MyISAM* tables, *InnoDB* tables, etc., creates the LVM snapshots, and backs up the snapshots.

.. rubric:: Logical Snapshots

A logical snapshot of the databases uses a SQL statements to retrieve table rows and restore them. When used within a transaction, a consistent snapshot of the database can be taken. This method can be used to export tables from one database server and import them into another server.

The ``SELECT INTO OUTFILE`` statement is used to take a logical snapshot of a database. The ``LOAD DATA INFILE`` statement is used to load the table data. Please see the *MySQL* 5.6 reference manual for details.

.. note:: Please do not use the :program:`mysqlhotcopy` to back up *TokuDB* tables. This script is incompatible with *TokuDB*.

Missing Log Files
-----------------

.. rubric:: What do I do if I delete my logs files or they are otherwise missing?

You'll need to recover from a backup. It is essential that the log files be present in order to restart the database.

Isolation Levels
----------------

.. rubric:: What is the default isolation level for TokuDB?

It is repeatable-read (MVCC).

.. rubric:: How can I change the isolation level?

*TokuDB* supports repeatable-read, serializable, read-uncommitted and read-committed isolation levels (other levels are not supported). *TokuDB* employs pessimistic locking, and aborts a transaction when a lock conflict is detected.

To guarantee that lock conflicts do not occur, use repeatable-read, read-uncommitted or read- committed isolation level.

Lock Wait Timeout Exceeded
--------------------------

.. rubric:: Why do my *MySQL* clients get lock timeout errors for my update queries? And what should my application do when it gets these errors?

Updates can get lock timeouts if some other transaction is holding a lock on the rows being updated for longer than the *TokuDB* lock timeout. You may want to increase the this timeout.

If an update deadlocks, then the transaction should abort and retry.

For more information on diagnosing locking issues, see :ref:`Lock Visualization in TokuDB <tokudb_lock_visualization>`.

Row Size
--------

.. rubric:: What is the maximum row size?

The maximum row size is 32 MiB.

NFS & CIFS
----------

.. rubric:: Can the data directories reside on a disk that is NFS or CIFS mounted?

Yes, we do have customers in production with NFS & CIFS volumes today. However, both of these disk types can pose a challenge to performance and data integrity due to their complexity. If you're seeking performance, the switching infrastructure and protocols of a traditional network were not conceptualized for low response times and can be very difficult to troubleshoot. If you're concerned with data integrity, the possible data caching at the NFS level can cause inconsistencies between the logs and data files that may never be detected in the event of a crash. If you are thinking of using a NFS or CIFS mount, we would recommend that you use synchronous mount options, which are available from the NFS mount man page, but these settings may decrease performance. For further discussion please look `here <http://www.mysqlperformanceblog.com/2010/07/30/storing-mysql-binary-logs-on-nfs-volume/>`_.

Using Other Storage Engines
---------------------------

.. rubric:: Can the MyISAM and InnoDB Storage Engines be used?

*MyISAM* and *InnoDB* can be used directly in conjunction with *TokuDB*. Please note that you should not overcommit memory between *InnoDB* and *TokuDB*. The total memory assigned to both caches must be less than physical memory.

.. rubric:: Can the Federated Storage Engines be used?

The Federated Storage Engine can also be used, however it is disabled by default in *MySQL*. It can be enabled by either running mysqld with ``--federated`` as a command line parameter, or by putting ``federated`` in the ``[mysqld]`` section of the :file:`my.cnf` file.

For more information see the *MySQL* 5.6 Reference Manual: `FEDERATED Storage Engine <http://dev.mysql.com/doc/refman/5.6/en/federated-storage-engine.html>`_.

Using MySQL Patches with TokuDB
-------------------------------

.. rubric:: Can I use MySQL source code patches with TokuDB?

Yes, but you need to apply Percona patches as well as your patches to *MySQL* to build a binary that works with the Percona Fractal Tree library. 

Truncate Table vs Delete from Table
-----------------------------------

.. rubric:: Which is faster, TRUNCATE TABLE or DELETE FROM TABLE?

Use ``TRUNCATE TABLE`` whenever possible. A table truncation runs in constant time, whereas a ``DELETE FROM TABLE`` requires a row-by-row deletion and thus runs in time linear to the table size.

Foreign Keys
------------

.. rubric:: Does TokuDB enforce foreign key constraints?

No, *TokuDB* ignores foreign key declarations.

Dropping Indexes
----------------

.. rubric:: Is dropping an index in TokuDB hot?

No, the table is locked for the amount of time it takes the file system to delete the file associated with the index.
