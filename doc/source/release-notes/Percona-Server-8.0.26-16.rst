.. _8.0.26-16:

================================================================================
*Percona Server for MySQL* 8.0.26-16
================================================================================

:Date: October 20, 2021
:Installation: `Installing Percona Server for MySQL <https://www.percona.com/doc/percona-server/8.0/installation.html>`_

`Percona Server for MySQL <https://www.percona.com/software/mysql-database/percona-server>`_ 8.0.26-16
includes all the features and bug fixes available in the
`MySQL 8.0.26 Community Edition <https://dev.mysql.com/doc/relnotes/mysql/8.0/en/news-8-0-26.html>`__.
in addition to enterprise-grade features developed by Percona.

Percona Server for MySQL® is a free, fully compatible, enhanced and open
source drop-in replacement for any MySQL database. It provides superior
performance, scalability and instrumentation.

Percona Server for MySQL is trusted by thousands of enterprises to provide
better performance and concurrency for their most demanding workloads, and
delivers greater value to MySQL server users with optimized performance,
greater performance scalability and availability, enhanced backups and
increased visibility. `Commercial support contracts are available
<https://www.percona.com/services/support/mysql-support>`__.

Release Highlights
=================================================

We have integrated a ZenFS RocksDB plugin to Percona Server for MySQL. This
plugin places files on a raw zoned block device (ZBD) using the MyRocks File
System interface. For more information, see :ref:`zenfs`.

The following list are some of the bug fixes for MySQL 8.0.26, provided by Oracle, and included in Percona Server for MySQL:

* :mysqlbug:`104575`: In the PERFORMANCE_SCHEMA.Threads table, the ``srv_purge_thread`` and ``srv_worker_thread`` values are duplicated.
* :mysqlbug:`104387`: When using REGEX comparison, a CHARACTER_SET_MISMATCH error is thrown.
* :mysqlbug:`104576`: Accessing the second index in a partition table with many columns can create a high CPU load.

Find the full list of bug fixes and changes in `MySQL 8.0.26 Release Notes <https://dev.mysql.com/doc/relnotes/mysql/8.0/en/news-8-0-26.html>`__.

New Features
=================================================

* :jirabug:`PS-7757`: Integrate ZenFS RocksDB plugin into Percona Server
* :jirabug:`PS-7777`: Document RocksDB variable ``rocksdb_manual_compaction_bottommost_level``.
* :jirabug:`PS-7765`: Document RocksDB variable ``rocksdb_fault_injection_options``

Deprecated Features
=================================================

The TokuDB Storage Engine was declared deprecated for Percona Server for MySQL. Starting with Percona Server 8.0.26-16, the plugins are available in the binary builds and packages but are disabled. The plugins will be removed from the binary builds and packages in a future version.

New options have been added to enable the plugins if they are needed to migrate the data to another storage engine.

The instructions on how to enable the plugins and more information are available at the beginning of each TokuDB topic.


Improvements
===============================================

* :jirabug:`PS-7526`: Fix the unexpected quoting and dropping of comments in DROP TABLE commands
* :jirabug:`PS-7706`: Add options to explicitly enable TokuDB and TokuDB Backup that are ``FALSE`` by default.


Bugs Fixed
===============================================

* :jirabug:`PS-1344`: LP #1160436: Fix if the ``log_slow_statement`` is called unconditionally
* :jirabug:`PS-1346`: LP #1163232: Fix an anomaly with the ``opt_log_slow_slave_statements``.
* :jirabug:`PS-7500`: Fix the ``SELECT COUNT(*)`` is slow in MyRocks.
* :jirabug:`PS-7742`: Fix when enabling binary log encryption breaks the basic replication setup on Percona Server for MySQL.
* :jirabug:`PS-7746`: Fix for a possible double call to `free_share()` in ha_innobase::open()
* :jirabug:`PS-7778`: Fix for denied commands when triggers with ``DEFINER`` are used.
* :jirabug:`PS-1955`: LP #1088529: Update the ``log_slow_verbosity`` help text to add the missing the "minimal", "standard", and "full" options
* :jirabug:`PS-2433`: LP #1234346: Include a timestamp in the slow query log file when initializing a new file
* :jirabug:`PS-7790`: Fix that disallows certain roles the ability to bypass the ProcFS access boundary with `..` instead of ``/proc`` or ``/sys``.
* :jirabug:`PS-7784`: Fix that reset the status variables ``procfs_access_violations`` and ``procfs_queries``
* :jirabug:`PS-7785`: Fix that reset the default value for ``procfs_files_spec`` which had the same value.
* :jirabug:`PS-7788`: Fix improves wildcard globbing in ``proc_files_spec``.
* :jirabug:`PS-7917`: Fix for installing the TokuDB storage engine with ps-admin.
