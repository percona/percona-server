.. _extended_mysqldump:

======================
Extended ``mysqldump``
======================

.. _mysqldump_backup_locks:

Backup Locks support
====================

In |Percona Server| :rn:`5.7.10-1` ``mysqldump`` was extended with a new option,
:option:`lock-for-backup` (disabled by default). When used together with the
:option:`--single-transaction` option, the option makes ``mysqldump`` issue
``LOCK TABLES FOR BACKUP`` before starting the dump operation to prevent unsafe
statements that would normally result in an inconsistent backup.

More information can be found on the :ref:`backup_locks` feature documentation.

.. _mysqldump_compressed_columns:

Compressed Columns support
==========================

In |Percona Server| :rn:`5.7.17-11` :command:`mysqldump` was extended to
support :ref:`compressed_columns` feature. More information about the new
options can be found on the :ref:`compressed_columns` feature page.

.. _mysqldump_order_by_primary_desc:

Taking backup by descending primary key order
=============================================

In :rn:`5.7.17-12`, a new option :option:`--order-by-primary-desc` was
implemented. This feature tells ``mysqldump`` to take the backup by descending
primary key order (``PRIMARY KEY DESC``) which can be useful if storage engine
is using reverse order column for a primary key.

RocksDB support
===============

:command:`mysqldump` will now detect when MyRocks is installed and available
by seeing if there is a session variable named
:variable:`rocksdb_skip_fill_cache` and setting it to ``1`` if it exists.

:command:`mysqldump` will now automatically enable session variable
:variable:`rocksdb_bulk_load` if it is supported by target server.

Version Specific Information
============================

  * :rn:`8.0.12-1`: The feature was ported from |Percona Server| 5.7

  * :rn:`5.7.10-1`
    :command:`mysqldump` was extended with :ref:`backup_locks` support
    options

  * :rn:`5.7.17-11`
    :command:`mysqldump` was extended with :ref:`compressed_columns`
    support options

  * :rn:`5.7.17-12`
    The :command:`mysqldump` option :option:`--order-by-primary-desc` introduced
