.. rn:: 5.6.39-83.1

=============================
|Percona Server| 5.6.39-83.1
=============================

Percona is glad to announce the release of |Percona Server| 5.6.39-83.1 on
February 13, 2018. Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.39-83.1/>`_
and from the :doc:`Percona Software Repositories </installation>`.

Based on `MySQL 5.6.39
<http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-39.html>`_, including
all the bug fixes in it, |Percona Server| 5.6.39-83.1 is now the current
stable release in the 5.6 series. All of |Percona|'s software is open-source
and free.

Bugs Fixed
==========

With :variable:`innodb_large_prefix` set to ``1``, Blackhole storage engine
was incompatible with InnoDB keys longer than 1000 bytes, thus adding new
indexes would cause replication errors on the slave. Bug fixed :psbug:`1126`
(upstream :mysqlbug:`53588`).

Intermediary slave with Blackhole storage engine couldn't record updates
from master to its own binary log in case master has
:variable:`binlog_rows_query_log_events` option enabled. Bug fixed
:psbug:`1119` (upstream :mysqlbug:`88057`).

A build error on FreeBSD caused by fixing the bug :psbug:`255` was present.
Bug fixed :psbug:`2284`.

Server queries that contained JSON special characters and were logged by
``audit_log`` plugin in JSON format caused invalid output due to lack of
escaping. Bug fixed :psbug:`1115`.

Compilation warnings fixed in sql_planner.cc module. Bug fixed :psbug:`3632`
(upstream :mysqlbug:`77637`).

A memory leak fixed in PFS unit test. Bug fixed :psbug:`1806` (upstream
:mysqlbug:`89384`).

A GCC 7 warning fix introduced regression in |Percona Server|
:rn:`5.6.38-83.0` that lead to a wrong SQL query built to access the remote
server when Federated storage engine was used. Bug fixed :psbug:`1134`.

|Percona Server| now uses *Travis CI* for additional tests. Bug fixed
:psbug:`3777`.

Other bugs fixed: :psbug:`257`, :psbug:`1090` (upstream :mysqlbug:`78048`),
:psbug:`1127`, and :psbug:`2415`.

This release also contains fixes for the following CVE issues: CVE-2018-2562,
CVE-2018-2573, CVE-2018-2583, CVE-2018-2590, CVE-2018-2591, CVE-2018-2612,
CVE-2018-2622, CVE-2018-2640, CVE-2018-2645, CVE-2018-2647, CVE-2018-2665,
CVE-2018-2668, CVE-2018-2696, CVE-2018-2703, CVE-2017-3737.

TokuDB Changes
==============

A memory leak was fixed in the PerconaFT library, caused by not destroying PFS
key objects on shutdown. Bug fixed :jirabug:`TDB-98`.

A clang-format configuration was added to PerconaFT and TokuDB. Bug fixed
:jirabug:`TDB-104`.

Other bugs fixed: :jirabug:`TDB-48`, :jirabug:`TDB-78`, :jirabug:`TDB-93`,
and :jirabug:`TDB-99`.


