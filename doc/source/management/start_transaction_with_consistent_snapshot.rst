.. _start_transaction_with_consistent_snapshot:

============================================
 Start transaction with consistent snapshot
============================================

*Percona Server for MySQL* has ported *MariaDB* `enhancement <https://mariadb.com/kb/en/enhancements-for-start-transaction-with-consistent/>`_ for ``START TRANSACTION WITH CONSISTENT SNAPSHOTS`` feature to *MySQL* 5.6 group commit implementation. This enhancement makes binary log positions consistent with *InnoDB* transaction snapshots.

This feature is quite useful to obtain logical backups with correct positions without running a ``FLUSH TABLES WITH READ LOCK``. Binary log position can be obtained by two newly implemented status variables: :ref:`Binlog_snapshot_file` and :ref:`Binlog_snapshot_position`. After starting a transaction using the ``START TRANSACTION WITH CONSISTENT SNAPSHOT``, these two variables will provide you with the binlog position corresponding to the state of the database of the consistent snapshot so taken, irrespectively of which other transactions have been committed since the snapshot was taken.

.. _snapshot_cloning:

Snapshot Cloning
================

The *Percona Server for MySQL* implementation extends the ``START TRANSACTION WITH CONSISTENT SNAPSHOT`` syntax with the optional ``FROM SESSION`` clause:

.. code-block:: mysql

  START TRANSACTION WITH CONSISTENT SNAPSHOT FROM SESSION <session_id>;

When specified, all participating storage engines and binary log instead of creating a new snapshot of data (or binary log coordinates), create a copy of the snapshot which has been created by an active transaction in the specified session. ``session_id`` is the session identifier reported in the ``Id`` column of ``SHOW PROCESSLIST``.

Currently snapshot cloning is only supported by *XtraDB* and the binary log. As with the regular ``START TRANSACTION WITH CONSISTENT SNAPSHOT``, snapshot clones can only be created with the ``REPEATABLE READ`` isolation level.

For *XtraDB*, a transaction with a cloned snapshot will only see data visible or changed by the donor transaction. That is, the cloned transaction will see no changes committed by transactions that started after the donor transaction, not even changes made by itself. Note that in case of chained cloning the donor transaction is the first one in the chain. For example, if transaction A is cloned into transaction B, which is in turn cloned into transaction C, the latter will have read view from transaction A (i.e. the donor transaction). Therefore, it will see changes made by transaction A, but not by transaction B.

mysqldump
=========

``mysqldump`` has been updated to use new status variables automatically when they are supported by the server and both :ref:`--single-transaction` and :ref:`--master-data` are specified on the command line. Along with the ``mysqldump`` improvements introduced in :ref:`backup_locks` there is now a way to generate ``mysqldump`` backups that are guaranteed to be consistent without using ``FLUSH TABLES WITH READ LOCK`` even if :ref:`--master-data` is requested.

System Variables
================

.. _have_snapshot_cloning:

.. rubric:: ``have_snapshot_cloning``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - Yes
   * - Config file
     - No
   * - Scope
     - Global
   * - Dynamic
     - No
   * - Data type
     - Boolean

This server variable is implemented to help other utilities detect if the server supports the ``FROM SESSION`` extension. When available, the snapshot cloning feature and the syntax extension to ``START TRANSACTION WITH CONSISTENT SNAPSHOT`` are supported by the server, and the variable value is always ``YES``.

Status Variables
================

.. _Binlog_snapshot_file:

.. rubric:: ``Binlog_snapshot_file``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Scope
     - Global
   * - Data type
     - String

.. _Binlog_snapshot_position:

.. rubric:: ``Binlog_snapshot_position``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Scope
     - Global
   * - Data type
     - Numeric

These status variables are only available when the binary log is enabled globally.

Other Reading
=============
* `MariaDB Enhancements for START TRANSACTION WITH CONSISTENT SNAPSHOT <https://mariadb.com/kb/en/enhancements-for-start-transaction-with-consistent/>`_
