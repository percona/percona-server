.. _8.0.22-13:

================================================================================
*Percona Server for MySQL* 8.0.22-13
================================================================================

:Date: December 14, 2020
:Installation: `Installing Percona Server for MySQL <https://www.percona.com/doc/percona-server/8.0/installation.html>`_

`Percona Server for MySQL <https://www.percona.com/software/mysql-database/percona-server>`_ 8.0.22-13
includes all the features and bug fixes available in
`MySQL 8.0.22 Community Edition <https://dev.mysql.com/doc/relnotes/mysql/8.0/en/news-8-0-22.html>`_
in addition to enterprise-grade features developed by Percona.

New Features
================================================================================

* :jirabug:`PS-7162`: Implement user-defined functions for Point-in-time Recovery in PXC operator



Improvements
================================================================================

* :jirabug:`PS-7348`: Create a set of C++ classes/macros that would simplify the creation of new user-defined functions



Bugs Fixed
================================================================================

* :jirabug:`PS-7346`: Correct the buffer calculation for the audit plugin used when large queries are executed(PS-5395).
* :jirabug:`PS-7300`: Modify Session temporary tablespace truncation on connection disconnect to reduce high CPU usage (Upstream :mysqlbug:`98869`)
* :jirabug:`PS-7304`: Correct package to include coredumper.a as a dependency of libperconaserverclient20-dev (Thanks to user Martin for reporting this issue)
* :jirabug:`PS-7236`: Correct grouping by GROUP BY processing with timezone (Thanks to user larrabee for reporting this issue) (Upstream :mysqlbug:`101105`)
* :jirabug:`PS-7286`: Modify to check for boundaries for encryption_key_id
* :jirabug:`PS-7317`: Add explicit_default_counter=10000 to innodb.table_encrypt_* MTR tests


