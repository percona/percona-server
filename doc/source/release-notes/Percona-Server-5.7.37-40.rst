.. _PS-5.7.37-40:

================================================
*Percona Server for MySQL* 5.7.37-40
================================================
:Date: March, 2022
:Installation: `Installing Percona Server for MySQL <https://www.percona.com/doc/percona-server/5.7/installation.html>`_

`Percona Server for MySQL <https://www.percona.com/software/mysql-database/percona-server>`_ 5.7.37-40
includes all the features and bug fixes available in `MySQL 5.7.37 Community Edition <https://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-37.html>`__ in addition to enterprise-grade features developed by Percona.

Percona Server for MySQL is a free, fully compatible, enhanced, and open source drop-in replacement for any MySQL database. It delivers more value to MySQL server users with optimized performance, greater performance scalability and availability, enhanced backups, and increased visibility. `Commercial support contracts are available <https://www.percona.com/services/support/mysql-support>`__.
 
Release Highlights
=================================================

The following lists a number of the notable updates and fixes for MySQL 5.7.37, provided by Oracle, and included in Percona Server for MySQL:

* The GnuPG build key has been updated. The GnuPG build key is used to sign MySQL downloadable packages. The systems configured to use repo.mysql.com may report a signature verification error when upgrading using *apt* or *yum* to MySQL 5.7.37 and higher or to MySQL 8.0.28 and higher. To resolve this issue, use one of the following methods:

    - Reinstall the MySQL APT or YUM repository setup package from https://dev.mysql.com/downloads/.

    - Download the MySQL GnuPG public key and add it to your system GPG keyring.

    - Find MySQL APT repository instructions in `Appendix A: Adding and Configuring the MySQL APT Repository Manually <https://dev.mysql.com/doc/mysql-apt-repo-quick-guide/en/#repo-qg-apt-repo-manual-setup>`__. 

    - Find MySQL YUM repository instructions in `Upgrading MySQL with the MySQL Yum Repository <https://dev.mysql.com/doc/mysql-yum-repo-quick-guide/en/#repo-qg-yum-upgrading>`__. 

* The performance on debug builds has been improved by optimazing `buf_validate()` function in the *InnoDB* sources.
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
  
Upgrading
--------------------

To upgrade Percona Server for MySQL from 5.6 to 5.7, follow the instructions in `Percona Server In-Place Upgrading Guide: From 5.6 to 5.7 <https://www.percona.com/doc/percona-server/5.7/upgrading_guide_56_57.html>`__ .

Contact Us
--------------------

The `Documentation Contribution Guide <https://github.com/percona/percona-server/blob/8.0/doc/source/contributing.md>`__ describes the methods available to contribute to the Percona Server for MySQL documentation.

For free technical help, visit the Percona `Community Forum <https://forums.percona.com/c/mysql-mariadb/percona-server-for-mysql-5-7/18>`__.

To report bugs or submit feature requests, open a `JIRA <https://jira.percona.com/projects/PS/issues>`__ ticket.

For paid `support https://www.percona.com/services/support and `managed <https://www.percona.com/services/managed-services>`__ or `consulting services <https://www.percona.com/services/consulting>`__, contact `Percona Sales <https://www.percona.com/about-percona/contact>`__.



