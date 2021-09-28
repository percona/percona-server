.. rn:: 5.7.13-6

===========================
 |Percona Server| 5.7.13-6
===========================

Percona is glad to announce the GA (Generally Available) release of |Percona
Server| 5.7.13-6 on July 6th, 2016 (Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.7/Percona-Server-5.7.13-6/>`_
and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.7.13
<http://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-13.html>`_, including
all the bug fixes in it, |Percona Server| 5.7.13-6 is the current GA release in
the |Percona Server| 5.7 series. All of Percona's software is open-source and
free, all the details of the release can be found in the `5.7.13-6 milestone at
Launchpad <https://launchpad.net/percona-server/+milestone/5.7.13-6>`_

New Features
============

 TokuDB MTR suite is now part of the default MTR suite in |Percona Server|
 5.7.

Bugs Fixed
==========

 Querying the :table:`GLOBAL_TEMPORARY_TABLES` table would cause server crash
 if temporary table owning threads would execute new queries. Bug fixed
 :bug:`1581949`.

 ``IMPORT TABLESPACE`` and undo tablespace truncate could get stuck
 indefinitely with a writing workload in parallel. Bug fixed :bug:`1585095`.

 Requesting to flush the whole of the buffer pool with doublewrite parallel
 buffer wasn't working correctly. Bug fixed :bug:`1586265`.

 :ref:`audit_log_plugin` would hang when trying to write log record of
 :variable:`audit_log_buffer_size` length. Bug fixed :bug:`1588439`.

 Audit log in ``ASYNC`` mode could skip log records which don't fit into log
 buffer. Bug fixed :bug:`1588447`.

 In order to support :variable:`innodb_flush_method` being set to
 ``ALL_O_DIRECT``, the log I/O buffers were aligned to
 :variable:`innodb_log_write_ahead_size`. This missed that the variable is
 dynamic and could still cause server to crash. Bug fixed :bug:`1597143`.

 InnoDB tablespace import would fail when trying to import a table with
 different data directory. Bug fixed :bug:`1548597` (upstream
 :mysqlbug:`76142`).

 :ref:`audit_log_plugin` was truncating SQL queries to 512 bytes. Bug fixed
 :bug:`1557293`.

 ``mysqlbinlog`` did not free the existing connection before opening a new
 remote one. Bug fixed :bug:`1587840` (upstream :mysqlbug:`81675`).

 Fixed a memory leak in ``mysqldump``. Bug fixed :bug:`1588845` (upstream
 :mysqlbug:`81714`).

 Transparent Huge Pages check will now only happen if
 :variable:`tokudb_check_jemalloc` option is set. Bugs fixed :tokubug:`939` and
 :ftbug:`713`.

 Logging in ``ydb`` environment validation functions now print more useful
 context. Bug fixed :ftbug:`722`.

Other bugs fixed: :bug:`1541698` (upstream :mysqlbug:`80261`), :bug:`1587426`
(upstream, :mysqlbug:`81657`), :bug:`1589431`, :tokubug:`956`, :tokubug:`964`,

