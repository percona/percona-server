.. rn:: 8.0.15-6

================================================================================
|Percona Server| |release|
================================================================================

|Percona| announces the release of |Percona Server| |release| on |date|
(downloads are available `here
<https://www.percona.com/downloads/Percona-Server-8.0/>`__ and from the `Percona
Software Repositories
<https://www.percona.com/doc/percona-server/8.0/installation.html#installing-from-binaries>`__).

This release includes fixes to bugs found in previous releases of |Percona
Server| 8.0.

|Percona Server| |release| is now the current GA release in the 8.0
series. All of |Percona|’s software is open-source and free.

Percona Server for MySQL 8.0 includes all the `features available in MySQL 8.0
Community Edition
<https://dev.mysql.com/doc/refman/8.0/en/mysql-nutshell.html>`__ in addition to
enterprise-grade features developed by Percona.  For a list of highlighted
features from both MySQL 8.0 and Percona Server for MySQL 8.0, please see the
`GA release announcement
<https://www.percona.com/blog/2018/12/21/announcing-general-availability-of-percona-server-for-mysql-8-0/>`__.

.. note::

   If you are upgrading from 5.7 to 8.0, please ensure that you read the
   `upgrade guide
   <https://www.percona.com/doc/percona-server/8.0/upgrading_guide.html>`__ and
   the document `Changed in Percona Server for MySQL 8.0
   <https://www.percona.com/doc/percona-server/8.0/changed_in_version.html>`__.

New Features
================================================================================

- The server part of MyRocks cross-engine consistent physical backups has been
  implemented by introducing :variable:`rocksdb_disable_file_deletions` and 
  :variable:`rocksdb_create_temporary_checkpoint` session variables. These
  variables are intended to be used by backup tools. Prolonged use or
  other misuse can have serious side effects to the server instance.

- RocksDB WAL file information can now be seen in the
  :table:`performance_schema.log_status` :ref:`table <log_status>`.
  

Bugs Fixed
================================================================================

- :ref:`backup_locks` was blocking DML for RocksDB. Bug fixed :psbug:`5583`.

.. |release| replace:: 8.0.15-6
.. |date| replace:: May 07, 2019
