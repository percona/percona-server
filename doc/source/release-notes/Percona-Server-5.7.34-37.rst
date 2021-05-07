.. _PS-5.7.34-37:

================================================================================
*Percona Server for MySQL* 5.7.34-37
================================================================================
:Date: May 26, 2021
:Installation: `Installing Percona Server for MySQL <https://www.percona.com/doc/percona-server/5.7/installation.html>`_

`Percona Server for MySQL <https://www.percona.com/software/mysql-database/percona-server>`_ 5.7.34-37
includes all the features and bug fixes available in
`MySQL 5.7.34 Community Edition <https://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-34.html>`_
in addition to enterprise-grade features developed by Percona.


Bugs Fixed
================================================================================

* :jirabug:`PS-4497`: Incorrect option error message for mysqlbinlog
* :jirabug:`PS-7498`: Prevent the replication coordinator thread stuck in Waiting until MASTER_DELAY seconds after master executed event while handling partial relay log transactions. (Upstream :mysqlbug:`102647`)
* :jirabug:`PS-7578`: Replication failure with UPDATE when replica server has a PK and the source does not. (Upstream :mysqlbug:`102628`)
* :jirabug:`PS-7593`: When changing the tx-isolation on a session, after a transaction has executed, the change is not honored and may cause a service failure. (Upstream :mysqlbug:`102831`)
* :jirabug:`PS-7657`: An update query executed against partition tables with compressed columns can cause an unexpected server exit.

