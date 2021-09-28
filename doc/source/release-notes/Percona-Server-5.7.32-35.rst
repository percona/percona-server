.. rn:: 5.7.32-35

================================================================================
*Percona Server for MySQL* 5.7.32-35
================================================================================

:Date: November 24, 2020
:Installation: `Installing Percona Server for MySQL <https://www.percona.com/doc/percona-server/5.7/installation.html>`_

`Percona Server for MySQL <https://www.percona.com/software/mysql-database/percona-server>`_ 5.7.32-35
includes all the features and bug fixes available in
`MySQL 5.7.32 Community Edition <https://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-32.html>`_
in addition to enterprise-grade features developed by Percona.

New Features
================================================================================

* :jirabug:`PS-7238`: Backport Data Masking plugin to 5.7 (Thanks to user Surenda Kumar Gupta for reporting this issue)



Bugs Fixed
================================================================================

* :jirabug:`PS-7346`: Correct the buffer calculation for the audit plugin used when large queries are executed(PS-5395).
* :jirabug:`PS-7232`: Modify Multithreaded Replica to correct the exhausted slave_transaction_retries when replica has `slave_preserve_commit_order` enabled (Upstream :mysqlbug:`99440`)
* :jirabug:`PS-7231`: Modify `Slave_transaction::retry_transaction()` to call `mysql_errno()` only when `thd->is_error()` is true
* :jirabug:`PS-7304`: Correct package to include coredumper.a as a dependency of libperconaserverclient20-dev (Thanks to user Martin for reporting this issue)
* :jirabug:`PS-7289`: Restrict innodb encryption threads to 255 and add min/max values
* :jirabug:`PS-7270`: Fix admin_port to accept non-proxied connections when proxy_protocol_networks='*'

