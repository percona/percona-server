.. rn:: 5.6.31-77.0

============================
|Percona Server| 5.6.31-77.0
============================

Percona is glad to announce the release of |Percona Server| 5.6.31-77.0 on July
7th, 2016 (Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.31-77.0/>`_
and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.6.31
<http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-31.html>`_, including
all the bug fixes in it, |Percona Server| 5.6.31-77.0 is the current GA release
in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source
and free, all the details of the release can be found in the `5.6.31-77.0
milestone at Launchpad
<https://launchpad.net/percona-server/+milestone/5.6.31-77.0>`_.

New Features
============

 |Percona Server| has implemented protocol support for :ref:`TLS 1.1 and TLS
 1.2 <extended_tls_support>`. This implementation turns off TLS v1.0 support by
 default.

 |TokuDB| MTR suite is now part of the default MTR suite in |Percona Server|
 5.6.

Bugs Fixed
==========

 Querying the :table:`GLOBAL_TEMPORARY_TABLES` table would cause server crash
 if temporary table owning threads would execute new queries. Bug fixed
 :bug:`1581949`.

 :ref:`audit_log_plugin` would hang when trying to write log record of
 :variable:`audit_log_buffer_size` length. Bug fixed :bug:`1588439`.

 Audit log in ``ASYNC`` mode could skip log records which don't fit into log
 buffer. Bug fixed :bug:`1588447`.

 The :variable:`innodb_log_block_size` feature attempted to diagnose the
 situation where the logs have been created with a log block value that differs
 from the current :variable:`innodb_log_block_size` setting. But this
 diagnostics came too late, and a misleading error ``No valid checkpoints
 found`` was produced first, aborting the startup. Bug fixed :bug:`1155156`.

 Some transaction deadlocks did not increase the
 :table:`INFORMATION_SCHEMA.INNODB_METRICS` ``lock_deadlocks`` counter. Bug
 fixed :bug:`1466414` (upstream :mysqlbug:`77399`).

 |InnoDB| tablespace import would fail when trying to import a table with
 different data directory. Bug fixed :bug:`1548597` (upstream
 :mysqlbug:`76142`).

 :ref:`audit_log_plugin` was truncating SQL queries to 512 bytes. Bug fixed
 :bug:`1557293`.

 Regular user extra port connection would fail if :variable:`max_connections`
 plus one ``SUPER`` user were already connected on the main port, even if it
 connecting would not violate the :variable:`extra_max_connections`. Bug fixed
 :bug:`1583147`.

 The error log warning ``Too many connections`` was only printed for connection
 attempts when :variable:`max_connections` plus one ``SUPER`` have connected.
 If the extra ``SUPER`` is not connected, the warning was not printed for a
 non-SUPER connection attempt. Bug fixed :bug:`1583553`.

 ``mysqlbinlog`` did not free the existing connection before opening a new
 remote one. Bug fixed :bug:`1587840` (upstream :mysqlbug:`81675`).

 Fixed memory leaks in ``mysqltest``. Bugs fixed :bug:`1582718` and
 :bug:`1588318`.

 Fixed memory leaks in ``mysqlcheck``. Bug fixed :bug:`1582741`.

 Fixed memory leak in ``mysqlbinlog``. Bug fixed :bug:`1582761` (upstream
 :mysqlbug:`78223`).

 Fixed memory leaks in ``mysqldump``. Bug fixed :bug:`1587873` and
 :bug:`1588845` (upstream :mysqlbug:`81714`).

 Fixed memory leak in non-existing defaults file handling. Bug fixed
 :bug:`1588344`.

 Fixed memory leak in ``mysqlslap``. Bug fixed :bug:`1588361`.

 Transparent Huge Pages check will now only happen if
 :variable:`tokudb_check_jemalloc` option is set. Bugs fixed :tokubug:`939` and
 :ftbug:`713`.

 Logging in ``ydb`` environment validation functions now prints more useful
 context. Bug fixed :ftbug:`722`.

Other bugs fixed: :bug:`1588386`, :bug:`1529885`, :bug:`1541698` (upstream
:mysqlbug:`80261`), :bug:`1582681`, :bug:`1583589`, :bug:`1587426` (upstream,
:mysqlbug:`81657`), :bug:`1589431`, :tokubug:`956`, and :tokubug:`964`.
