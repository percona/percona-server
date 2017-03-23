.. rn:: 5.6.35-81.0

============================
|Percona Server| 5.6.35-81.0
============================

Percona is glad to announce the release of |Percona Server| 5.6.35-81.0 on
March 24th, 2017 (Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.35-81.0/>`_
and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.6.35
<http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-35.html>`_, including
all the bug fixes in it, |Percona Server| 5.6.35-80.1 is the current GA release
in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source
and free, all the details of the release can be found in the `5.6.35-81.0
milestone at Launchpad
<https://launchpad.net/percona-server/+milestone/5.6.35-81.0>`_.

New Features
============

 |Percona Server| has implemented new :command:`mysqldump`
 :option:`--order-by-primary-desc` option. This feature tells ``mysqldump``
 to take the backup by descending primary key order (``PRIMARY KEY DESC``)
 which can be useful if storage engine is using reverse order column for
 a primary key.

Bugs Fixed
==========

 When :variable:`innodb_ft_result_cache_limit` was exceeded by internal memory
 allocated by |InnoDB| during the FT scan not all memory was released which
 could lead to server assertion. Bug fixed :bug:`1634932` (upstream
 :mysqlbug:`83648`).

 Log tracking initialization did not find last valid bitmap data correctly,
 potentially resulting in needless redo log retracking or hole in the tracked
 LSN range. Bug fixed :bug:`1658055`.

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

 A long-running binary log commit would block ``SHOW STATUS``, which in turn
 could block a number of of other operations such as client connects and
 disconnects. Bug fixed  :bug:`1646100`.

 It was impossible to use column compression dictionaries with partitioned
 |InnoDB| tables. Bug fixed :bug:`1653104`.

 Diagnostics for OpenSSL errors have been improved. Bug fixed :bug:`1660339`
 (upstream :mysqlbug:`75311`).

 When ``DuplicateWeedout`` strategy was used for joins, use was not reported in
 the query plan info output extension for the slow query log. Bug fixed
 :bug:`1592694`.

Other bugs fixed: :bug:`1650321`, :bug:`1650322`, :bug:`1654501`,
:bug:`1663251`, :bug:`1666213`, :bug:`1652912`, :bug:`1659548`, :bug:`1663452`,
:bug:`1670834`, :bug:`1672871`, :bug:`1626545`, :bug:`1644174`, :bug:`1658006`,
:bug:`1658021`, :bug:`1659218`, :bug:`1659746`, :bug:`1660239`, :bug:`1660243`,
:bug:`1660255`, :bug:`1660348`, :bug:`1662163` upstream (:mysqlbug:`81467`),
:bug:`1664219`, :bug:`1664473`, :bug:`1671076`, and :bug:`1671123`.
