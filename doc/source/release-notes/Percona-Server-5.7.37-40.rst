.. _PS-5.7.37-40:

================================================
*Percona Server for MySQL* 5.7.37-40
================================================
:Date: March 31, 2022

`Percona Server for MySQL <https://www.percona.com/software/mysql-database/percona-server>`_ 5.7.37-40
includes all the features and bug fixes available in `MySQL 5.7.37 Community Edition <https://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-37.html>`__ in addition to enterprise-grade features developed by Percona.

Percona Server for MySQL is a free, fully compatible, enhanced, and open source drop-in replacement for any MySQL database. It delivers more value to MySQL server users with optimized performance, greater performance scalability and availability, enhanced backups, and increased visibility. `Commercial support contracts are available <https://www.percona.com/services/support/mysql-support>`__.
 
.. contents::
   :local:

Release Highlights
=================================================

The following lists a number of the notable updates and fixes for MySQL 5.7.37, provided by Oracle, and included in Percona Server for MySQL:

* The performance on debug builds has been improved by optimizing `buf_validate()` function in the *InnoDB* sources.
* Fix for when a query using an index that differs from the primary key of partitioned table results in excessive CPU load.

Find the complete list of bug fixes and changes in `MySQL 5.7.37 Release Notes <https://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-37.html>`__.

Improvements
=================================================

* :jirabug:`PS-7792`: Allows setting an empty `MASTER_USER` user name if you always provide user credentials when using the `START_SLAVE` statement. This method requires user intervention to restart the replica.

Bugs Fixed
=================================================

* :jirabug:`PS-7929`: Fix for when the row locks were duplicated when inserting an existing row into a table within the same transaction.
* :jirabug:`PS-8007`: *Percona Server for MySQL* can fail to start if the server starts before the network mounts the datadir or a local mount of the datadir.
* :jirabug:`PS-7856`: A partition table update caused a server exit.
* :jirabug:`PS-7890`: When the server was started with the `--loose-rocksdb_persistent_cache_size_mb` option, the RocksDB engine plugin installation failed. 

Packaging Notes
=================================================

* Red Hat Enterprise Linux 6 (and derivative Linux distributions) are no longer supported.
* Debian 9 is no longer supported.

Known issues
=================================================

* The RPM packages for Red Hat Enterprise Linux 7 (and compatible derivatives) do not support TLSv1.3, as it requires OpenSSL 1.1.1, which is currently not available on this platform.

Useful links
=================================================

* To install Percona Server for MySQL 5.7, follow the instructions in `Installing Percona Server for MySQL <https://www.percona.com/doc/percona-server/5.7/installation.html>`_ .
* To upgrade Percona Server for MySQL from 5.6 to 5.7, follow the instructions in `Percona Server In-Place Upgrading Guide: From 5.6 to 5.7 <https://www.percona.com/doc/percona-server/5.7/upgrading_guide_56_57.html>`__.
* The GitHub location for `Percona Server <https://github.com/percona/percona-server>`__.
* To contribute to the Percona Server for MySQL documentation, review the `Documentation Contribution Guide <https://github.com/percona/percona-server/blob/8.0/doc/source/contributing.md>`__.


