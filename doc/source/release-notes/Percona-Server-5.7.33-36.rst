.. _5.7.33-36:

================================================================================
*Percona Server for MySQL* 5.7.33-36
================================================================================

:Date: March 2, 2021
:Installation: `Installing Percona Server for MySQL <https://www.percona.com/doc/percona-server/5.7/installation.html>`_

`Percona Server for MySQL <https://www.percona.com/software/mysql-database/percona-server>`_ 5.7.33-36
includes all the features and bug fixes available in
`MySQL 5.7.33 Community Edition <https://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-33.html>`_
in addition to enterprise-grade features developed by Percona.

New Features
================================================================================

* :jirabug:`PS-5364`: Update keyring_vault plugin to support KV Secrets Engine Version 2 (kv-v2) (Thanks to user aprokofyev for reporting this issue)
* :jirabug:`PS-7447`: Backport variable innodb_buffer_pool_in_core_file and processing (Upstream :mysqlbug:`101825`)
* :jirabug:`PS-7459`: Backport of InnoDB: Group purging of rows by table ID (`WL#9387 <https://dev.mysql.com/worklog/task/?id=9387>`_) to PS 5.7.33



Bugs Fixed
================================================================================

* :jirabug:`PS-1956`: Change data type for some microsecond times for the slow query log to 64-bit
* :jirabug:`PS-7499`: Improve error log when MyRocks fails with rocksdb_validate_tables=1
* :jirabug:`PS-5112`: Backport fix for :jirabug:`PS-5027` from 8.0 to 5.7
* :jirabug:`PS-7492`: Update slow log formatting for tmp tables related stats
* :jirabug:`PS-7498`: Prevent the replication co-ordinator thread getting stuck due to MASTER_DELAY while handling partial relay log transactions. (Upstream :mysqlbug:`102647`)



