.. _extended_mysqldump:

================================================================================
Extended ``mysqldump``
================================================================================

.. _mysqldump_backup_locks:

Backup Locks support
================================================================================

When used together with the :ref:`--single-transaction` option, the
:ref:`lock-for-backup` option makes ``mysqldump`` issue ``LOCK
TABLES FOR BACKUP`` before starting the dump operation to prevent
unsafe statements that would normally result in an inconsistent
backup.

More information can be found on the :ref:`backup_locks` feature documentation.

.. _mysqldump_compressed_columns:

Compressed Columns support
================================================================================

:command:`mysqldump` supports the :ref:`compressed_columns` feature. More
information about the relevant options can be found on the
:ref:`compressed_columns` feature page.

.. _mysqldump_order_by_primary_desc:

Taking backup by descending primary key order
================================================================================

:ref:`--order-by-primary-desc` tells ``mysqldump`` to take the backup by
descending primary key order (``PRIMARY KEY DESC``) which can be useful if
the storage engine is using the reverse order column for a primary key.

RocksDB support
================================================================================

:command:`mysqldump` detects when MyRocks is installed and available.
If there is a session variable named
:ref:`rocksdb_skip_fill_cache` :command:`mysqldump` sets it to **1**.

:command:`mysqldump` will now automatically enable session the variable
:ref:`rocksdb_bulk_load` if it is supported by the target server.

Version Specific Information
================================================================================

* `8.0.12-1`: The feature was ported from *Percona Server for MySQL* 5.7

