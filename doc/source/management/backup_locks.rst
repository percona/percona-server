.. _backup_locks:

==============
 Backup Locks
==============

|Percona Server| has implemented this feature to be a lightweight alternative to ``FLUSH TABLES WITH READ LOCK`` for both physical and logical backups. Three new statements are now available: ``LOCK TABLES FOR BACKUP``, ``LOCK BINLOG FOR BACKUP`` and ``UNLOCK BINLOG``.

``LOCK TABLES FOR BACKUP``
---------------------------

``LOCK TABLES FOR BACKUP`` uses a new MDL lock type to block updates to non-transactional tables and DDL statements for all tables. If there is an active ``LOCK TABLES FOR BACKUP`` lock then all DDL statements and all updates to MyISAM, CSV, MEMORY, ARCHIVE, TokuDB, and MyRocks tables will be blocked in the ``Waiting for backup lock`` status, visible in ``PERFORMANCE_SCHEMA`` or ``PROCESSLIST``.

``LOCK TABLES FOR BACKUP`` has no effect on ``SELECT`` queries for all mentioned storage engines. Against InnoDB, Blackhole and Federated tables, the ``LOCK TABLES FOR BACKUP`` is not applicable to the ``INSERT``, ``REPLACE``, ``UPDATE``, ``DELETE`` statements: Blackhole tables obviously have no relevance to backups, and Federated tables are ignored by both logical and physical backup tools. 

Unlike ``FLUSH TABLES WITH READ LOCK``, ``LOCK TABLES FOR BACKUP`` does not flush tables, i.e. storage engines are not forced to close tables and tables are not expelled from the table cache. As a result, ``LOCK TABLES FOR BACKUP`` only waits for conflicting statements to complete (i.e. DDL and updates to non-transactional tables). It never waits for SELECTs, or UPDATEs to InnoDB tables to complete, for example.

If an "unsafe" statement is executed in the same connection that is holding a ``LOCK TABLES FOR BACKUP`` lock, it fails with the following error: :: 

 ERROR 1880 (HY000): Can't execute the query because you have a conflicting backup lock

 UNLOCK TABLES releases the lock acquired by LOCK TABLES FOR BACKUP.

``LOCK BINLOG FOR BACKUP``
---------------------------

``LOCK BINLOG FOR BACKUP`` uses another new MDL lock type to block all operations that might change either binary log position or ``Exec_Master_Log_Pos`` or ``Exec_Gtid_Set`` (i.e. source binary log coordinates corresponding to the current SQL thread state on a replication replica) as reported by ``SHOW MASTER``/``SLAVE STATUS``. More specifically, a commit will only be blocked if the binary log is enabled (both globally, and for connection with sql_log_bin), or if commit is performed by a replica thread and would advance ``Exec_Master_Log_Pos`` or ``Executed_Gtid_Set``. Connections that are currently blocked on the global binlog lock can be identified by the ``Waiting for binlog lock`` status in ``PROCESSLIST``.

.. _backup-safe_binlog_information:

``LOCK TABLES FOR BACKUP`` flushes the current binary log coordinates to InnoDB. Thus, under active ``LOCK TABLES FOR BACKUP``, the binary log coordinates in InnoDB are consistent with its redo log and any non-transactional updates (as the latter are blocked by ``LOCK TABLES FOR BACKUP``). It is planned that this change will enable |Percona XtraBackup| to avoid issuing the more invasive ``LOCK BINLOG FOR BACKUP`` command under some circumstances.

``UNLOCK BINLOG``
------------------

``UNLOCK BINLOG`` releases the ``LOCK BINLOG FOR BACKUP`` lock, if acquired by the current connection. The intended use case for |Percona XtraBackup| is: :: 

  LOCK TABLES FOR BACKUP
  ... copy .frm, MyISAM, CSV, etc. ...
  LOCK BINLOG FOR BACKUP
  UNLOCK TABLES
  ... get binlog coordinates ...
  ... wait for redo log copying to finish ...
  UNLOCK BINLOG

Privileges
----------

Both ``LOCK TABLES FOR BACKUP`` and ``LOCK BINLOG FOR BACKUP`` require the ``RELOAD`` privilege. The reason for that is to have the same requirements as ``FLUSH TABLES WITH READ LOCK``.

Interaction with other global locks
-----------------------------------

Both ``LOCK TABLES FOR BACKUP`` and ``LOCK BINLOG FOR BACKUP`` have no effect if the current connection already owns a ``FLUSH TABLES WITH READ LOCK`` lock, as it's a more restrictive lock. If ``FLUSH TABLES WITH READ LOCK`` is executed in a connection that has acquired ``LOCK TABLES FOR BACKUP`` or ``LOCK BINLOG FOR BACKUP``, ``FLUSH TABLES WITH READ LOCK`` fails with an error.

If the server is operating in the read-only mode (i.e. :variable:`read_only` set to ``1``), statements that are unsafe for backups will be either blocked or fail with an error, depending on whether they are executed in the same connection that owns ``LOCK TABLES FOR BACKUP`` lock, or other connections.

MyISAM index and data buffering
-------------------------------

MyISAM key buffering is normally write-through, i.e. by the time each update to a MyISAM table is completed, all index updates are written to disk. The only exception is delayed key writing feature which is disabled by default. 

When the global system variable :variable:`delay_key_write` is set to ``ALL``, key buffers for all MyISAM tables are not flushed between updates, so a physical backup of those tables may result in broken MyISAM indexes. To prevent this, ``LOCK TABLES FOR BACKUP`` will fail with an error if ``delay_key_write`` is set to ``ALL``. An attempt to set :variable:`delay_key_write` to ``ALL`` when there's an active backup lock will also fail with an error. 

Another option to involve delayed key writing is to create MyISAM tables with the DELAY_KEY_WRITE option and set the :variable:`delay_key_write` variable to ``ON`` (which is the default). In this case, ``LOCK TABLES FOR BACKUP`` will not be able to prevent stale index files from appearing in the backup. Users are encouraged to set :variable:`delay_key_writes` to ``OFF`` in the configuration file, :file:`my.cnf`, or repair MyISAM indexes after restoring from a physical backup created with backup locks.

MyISAM may also cache data for bulk inserts, e.g. when executing multi-row INSERTs or ``LOAD DATA`` statements. Those caches, however, are flushed between statements, so have no effect on physical backups as long as all statements updating MyISAM tables are blocked.

mysqldump
---------

``mysqldump`` has also been extended with a new option, :option:`lock-for-backup` (disabled by default). When used together with the :option:`--single-transaction` option, the option makes ``mysqldump`` issue ``LOCK TABLES FOR BACKUP`` before starting the dump operation to prevent unsafe statements that would normally result in an inconsistent backup.

When used without the :option:`single-transaction` option, :option:`lock-for-backup` is automatically converted to :option:`lock-all-tables`.

Option :option:`lock-for-backup` is mutually exclusive with :option:`lock-all-tables`, i.e. specifying both on the command line will lead to an error. 

If the backup locks feature is not supported by the target server, but :option:`lock-for-backup` is specified on the command line, ``mysqldump`` aborts with an error.

|Percona XtraBackup| provides the `--backup-locks <https://www.percona.com/doc/percona-xtrabackup/2.4/innobackupex/innobackupex_option_reference.html#cmdoption-innobackupex-backup-locks>`_ option. If you disable this option, ``Flush Table with Read Lock`` is used on the backup stage.

Version Specific Information
============================

  * :rn:`5.7.10-1`
        Feature ported from |Percona Server| 5.6

System Variables
================

.. variable:: have_backup_locks

     :cli: Yes
     :conf: No
     :scope: Global
     :dyn: No
     :vartype: Boolean
     :default: YES

This is a server variable implemented to help other utilities decide what locking strategy can be implemented for a server. When available, the backup locks feature is supported by the server and the variable value is always ``YES``.

.. variable:: have_backup_safe_binlog_info

     :cli: Yes
     :conf: No
     :scope: Global
     :dyn: No
     :vartype: Boolean
     :default: YES

This is a server variable implemented to help other utilities decide if ``LOCK BINLOG FOR BACKUP`` can be avoided in some cases. When the necessary server-side functionality is available, this server system variable exists and its value is always ``YES``.

Status Variables
================

.. variable:: Com_lock_tables_for_backup

     :vartype: Numeric
     :scope: Global/Session

.. variable:: Com_lock_binlog_for_backup

     :vartype: Numeric
     :scope: Global/Session

.. variable:: Com_unlock_binlog

     :vartype: Numeric
     :scope: Global/Session

These status variables indicate the number of times the corresponding statements have been executed.

Client Command Line Parameter
=============================

.. option:: lock-for-backup

     :cli: Yes
     :scope: Global
     :dyn: No
     :vartype: String
     :default: Off

When used together with the :option:`--single-transaction` option, the option makes ``mysqldump`` issue ``LOCK TABLES FOR BACKUP`` before starting the dump operation to prevent unsafe statements that would normally result in an inconsistent backup.

