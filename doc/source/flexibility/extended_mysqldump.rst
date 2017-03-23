.. _extended_mysqldump:

======================
Extended ``mysqldump``
======================

.. _mysqldump_ignore_create_error:

Ignoring missing tables in mysqldump
====================================

In case table name was changed during the :command:`mysqldump` process
taking place, :command:`mysqldump` would stop with error:

.. code-block:: text

   Couldn't execute 'show create table testtable'
   Table 'testdb.tabletest' doesn't exist (1146)\n")

This could happen if :command:`mysqldump` was taking a backup of a working
slave and during that process table name would get changed. This error happens
because :command:`mysqldump` takes the list of the tables at the beginning of
the dump process but the ``SHOW CREATE TABLE`` happens just before the table is
being dumped.

With this option :command:`mysqldump` will still show error to ``stderr``, but
it will continue to work and dump the rest of the tables.

.. _mysqldump_backup_locks:

Backup Locks support
====================

In |Percona Server| :rn:`5.6.16-64.0` ``mysqldump`` has been extended with a
new option, :option:`lock-for-backup` (disabled by default). When used together
with the :option:`--single-transaction` option, the option makes ``mysqldump``
issue ``LOCK TABLES FOR BACKUP`` before starting the dump operation to prevent
unsafe statements that would normally result in an inconsistent backup.

More information can be found on the :ref:`backup_locks` feature documentation.

.. _mysqldump_compressed_columns:

Compressed Columns support
==========================

In |Percona Server| :rn:`5.6.33-79.0` :command:`mysqldump` has been extended to
support :ref:`compressed_columns` feature. More information about the new
options can be found on the :ref:`compressed_columns` feature page.

.. _mysqldump_order_by_primary_desc:

Taking backup by descending primary key order
=============================================

In :rn:`5.6.35-81.0` new :option:`--order-by-primary-desc` has been
implemented. This feature tells ``mysqldump`` to take the backup by
descending primary key order (``PRIMARY KEY DESC``) which can be useful if
storage engine is using reverse order column for a primary key.

Version Specific Information
============================

  * :rn:`5.6.5-60.0`
    :command:`mysqldump` option :option:`--ignore-create-error` introduced

  * :rn:`5.6.16-64.0`
    :command:`mysqldump` has been extended with :ref:`backup_locks` support
    options

  * :rn:`5.6.33-79.0`
    :command:`mysqldump` has been extended with :ref:`compressed_columns`
    support options

  * :rn:`5.6.35-81.0`
    :command:`mysqldump` option :option:`--order-by-primary-desc` introduced
