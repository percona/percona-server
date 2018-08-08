.. rn:: 5.7.17-12

==========================
|Percona Server| 5.7.17-12
==========================

Percona is glad to announce the GA (Generally Available) release of |Percona
Server| 5.7.17-12 on March 24th, 2017 (Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.7/Percona-Server-5.7.17-12/>`_
and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.7.17
<http://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-17.html>`_, including
all the bug fixes in it, |Percona Server| 5.7.17-12 is the current GA release
in the |Percona Server| 5.7 series. All of |Percona|'s software is open-source
and free, all the details of the release can be found in the `5.7.17-12
milestone at
Launchpad <https://launchpad.net/percona-server/+milestone/5.7.17-12>`_

New Features
============

 |Percona Server| has implemented new :command:`mysqldump`
 :option:`--order-by-primary-desc` option. This feature tells ``mysqldump``
 to take the backup by descending primary key order (``PRIMARY KEY DESC``)
 which can be useful if storage engine is using reverse order column family
 for a primary key.

 :command:`mysqldump` will now detect when MyRocks is installed and available
 by seeing if there is a session variable named
 :variable:`rocksdb_skip_fill_cache` and setting it to ``1`` if it exists.

 :command:`mysqldump` will now automatically enable session variable
 :variable:`rocksdb_bulk_load` if it is supported by target server.

Bugs Fixed
==========

 If the variable :variable:`thread_handling` was set to ``pool-of-threads`` in
 the |MySQL| configuration file, server couldn't be gracefully shut down by a
 ``SIGTERM`` signal. Bug fixed :bug:`1537554`.

 When :variable:`innodb_ft_result_cache_limit` was exceeded by internal memory
 allocated by |InnoDB| during the FT scan not all memory was released which
 could lead to server assertion. Bug fixed :bug:`1634932` (upstream
 :mysqlbug:`83648`).

 Executing the ``FLUSH LOGS`` on a read-only slave with a user that doesn't
 have the ``SUPER`` privilege would result in ``Error 1290``. Bug fixed
 :bug:`1652852` (upstream :mysqlbug:`84350`).

 ``FLUSH LOGS`` was disabled with :variable:`read_only` and
 :variable:`super_read_only` variables. Bug fixed :bug:`1654682` (upstream
 :mysqlbug:`84437`).

 If ``SHOW BINLOGS`` or ``PERFORMANCE_SCHEMA.GLOBAL_STATUS`` query, and a
 transaction commit would run in parallel, they could deadlock. Bug fixed
 :bug:`1657128`.

 A long-running binary log commit would block ``SHOW STATUS``, which in turn
 could block a number of of other operations such as client connects and
 disconnects. Bug fixed  :bug:`1646100`.

 Log tracking initialization did not find last valid bitmap data correctly. Bug
 fixed :bug:`1658055`.

 A query using range scan with a complex range condition could lead to a server
 crash. Bug fixed :bug:`1660591` (upstream :mysqlbug:`84736`).

 Race condition between buffer pool page optimistic access and eviction could
 lead to a server crash. Bug fixed :bug:`1664280`.

 If :ref:`audit_log_plugin` was unable to create file pointed by
 :variable:`audit_log_file`, server would crash during the startup. Bug fixed
 :bug:`1666496`.

 A ``DROP TEMPORARY TABLE ...``  for a table created by a ``CREATE TEMPORARY
 TABLE ... SELECT ...`` would get logged in the binary log on a disconnect
 with mixed mode replication. Bug fixed :bug:`1671013`.

 |TokuDB| did not use index with even if cardinality was good. Bug fixed
 :bug:`1671152`.

 Row-based replication events were not reflected in ``Rows_updated`` fields in
 the :ref:`user_stats` ``INFORMATION_SCHEMA`` tables. Bug fixed :bug:`995624`.

 When ``DuplicateWeedout`` strategy was used for joins, use was not reported in
 the query plan info output extension for the slow query log. Bug fixed
 :bug:`1592694`.

 It was impossible to use column compression dictionaries with partitioned
 |InnoDB| tables. Bug fixed :bug:`1653104`.

 Diagnostics for OpenSSL errors have been improved. Bug fixed :bug:`1660339`
 (upstream :mysqlbug:`75311`).

Other bugs fixed: :bug:`1665545`, :bug:`1650321`, :bug:`1654501`,
:bug:`1663251`, :bug:`1659548`, :bug:`1663452`, :bug:`1670834`, :bug:`1672871`,
:bug:`1626545`, :bug:`1658006`, :bug:`1658021`, :bug:`1659218`, :bug:`1659746`,
:bug:`1660239`, :bug:`1660243`, :bug:`1660348`, :bug:`1662163` (upstream
:mysqlbug:`81467`), :bug:`1664219`, :bug:`1664473`, :bug:`1671076`, and
:bug:`1671123`.

