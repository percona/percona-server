.. _8.0.26-17:

================================================================================
*Percona Server for MySQL* 8.0.26-17
================================================================================

:Date: January 26, 2022
:Installation: `Installing Percona Server for MySQL <https://www.percona.com/doc/percona-server/8.0/installation.html>`_

`Percona Server for MySQL <https://www.percona.com/software/mysql-database/percona-server>`_ 8.0.26-17
includes all the features and bug fixes available in the
`MySQL 8.0.26 Community Edition <https://dev.mysql.com/doc/relnotes/mysql/8.0/en/news-8-0-26.html>`__.
in addition to enterprise-grade features developed by Percona.

*Percona Server for MySQL* 8.0.26-17 is now the current GA release in the 8.0 series.

Percona Server for MySQL® is a free, fully-compatible, enhanced, and open
source drop-in replacement for any MySQL database. It provides superior
performance, scalability, and instrumentation.

Percona Server for MySQL is trusted by thousands of enterprises to provide
better performance and concurrency for their most demanding workloads. It
delivers a greater value to MySQL server users with optimized performance,
greater performance scalability, and availability, enhanced backups and
increased visibility. `Commercial support contracts are available
<https://www.percona.com/services/support/mysql-support>`__.

Release Highlights
=================================================

Percona integrates a ZenFS RocksDB plugin to Percona Server for MySQL. This
plugin places files on a raw zoned block device (ZBD) using the MyRocks File
System interface. Percona provides a binary release for `Debian 11.1 <https://www.debian.org/releases/bullseye/debian-installer/index>`__. `Other Linux distributions <https://zonedstorage.io/docs/distributions/linux/>`__ are adding support for ZenFS, but Percona does not offer installation packages for those distributions yet. The ``libzbd`` package is now linked statically to the RocksDB storage engine.

The following dependency libraries are updated to newer versions:

* ZenFS v1.0.0
* libzbd v2.0.1

For more information, see :ref:`zenfs`.

The following list has a number of the bug fixes for MySQL 8.0.26, provided by Oracle, and included in Percona Server for MySQL:

* :mysqlbug:`104575`: Fix for when, in the PERFORMANCE_SCHEMA.Threads table, the ``srv_purge_thread`` and ``srv_worker_thread`` values are duplicated.
* :mysqlbug:`104387`: Fix for when using a REGEX comparison, a CHARACTER_SET_MISMATCH error is thrown.
* :mysqlbug:`104576`: Fix for a high CPU load being created when accessing the second index in a partition table with many columns.

Find the full list of bug fixes and changes in `MySQL 8.0.26 Release Notes <https://dev.mysql.com/doc/relnotes/mysql/8.0/en/news-8-0-26.html>`__.


Deprecated Features
=================================================

The TokuDB Storage Engine was declared deprecated for Percona Server for MySQL. Starting with Percona Server 8.0.26-16, the plugins are available in binary builds and packages but are disabled. The plugins will be removed from the binary builds and packages in a future version.

New options have been added to enable the plugins if they are needed to migrate the data to another storage engine.

The instructions on enabling the plugins and more information are available at the beginning of each TokuDB topic in the Percona Server fo MySQL documenation.

Bugs Fixed
==================================================

* The ``libzbd`` user library is statically linked into ``ha_rocksdb.so``. This linking allows the creation of a single binary package and requires the 5.9 kernel and higher.

* Fix for ZenFS issues with ``sysbench`` using either Debian 11 or the latest ``libzbd`` user library.

* Fix a sporadic [aborting on BG write error] assertion in rocksdb.bloomfilter3.

* Fix when the ``WITH_ZENFS_UTILITY`` CMake option is set to ``ON``. Added logic to the RocksDB *CMakeLists.txt* to ensure the ``libgflags`` library is installed on the system.

* Fix for the tests that rely on the ``du`` system utility. The ``du`` utility results must be converted to computations based on the ``zenfs`` list output.

* Fix for the zenfs ``mkfs`` to allow the command to accept a pre-existing aux_path. 
