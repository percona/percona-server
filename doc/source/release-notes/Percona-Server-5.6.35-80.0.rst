.. rn:: 5.6.35-80.0

============================
|Percona Server| 5.6.35-80.0
============================

Percona is glad to announce the release of |Percona Server| 5.6.35-80.0 on
February 8th, 2017 (Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.35-80.0/>`_
and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.6.35
<http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-35.html>`_, including
all the bug fixes in it, |Percona Server| 5.6.35-80.0 is the current GA release
in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source
and free, all the details of the release can be found in the `5.6.35-80.0
milestone at Launchpad
<https://launchpad.net/percona-server/+milestone/5.6.35-80.0>`_.

New Features
============

 :ref:`innodb_kill_idle_trx` feature has been re-implemented by setting a
 connection socket read timeout value instead of periodically scanning the
 internal |InnoDB| transaction list. This makes the feature applicable to any
 transactional storage engine, such as |TokuDB|, and, in future, *MyRocks*.
 This re-implementation is also addressing some existing bugs, including server
 crashes: :bug:`1166744`, :bug:`1179136`, :bug:`907719`, and :bug:`1369373`.

Bugs Fixed
==========

 Logical row counts for |TokuDB| tables could get inaccurate over time. Bug
 fixed :bug:`1651844` (:ftbug:`732`).

 Repeated execution of ``SET STATEMENT ... FOR <SELECT FROM view>`` could lead
 to a server crash. Bug fixed :bug:`1392375`.

 ``CREATE TEMPORARY TABLE`` would create a transaction in binary log on a read
 only server. Bug fixed :bug:`1539504` (upstream :mysqlbug:`83003`).

 If temporary tables from ``CREATE TABLE ... AS SELECT`` contained compressed
 attributes it could lead to server crash. Bug fixed :bug:`1633957`.

 Using :ref:`per_query_variable_statement` with subquery temporary tables could
 cause a memory leak. Bug fixed :bug:`1635927`.

 Fixed new compilation warnings with GCC 6. Bugs fixed :bug:`1641612` and
 :bug:`1644183`.

 A server could crash if a bitmap write I/O error happens in the background log
 tracking thread while a ``FLUSH CHANGED_PAGE_BITMAPS`` is executing
 concurrently. Bug fixed :bug:`1651656`.

 |TokuDB| was using wrong function to calculate free space in data files. Bug
 fixed :bug:`1656022` (:tokubug:`1033`).

 ``CONCURRENT_CONNECTIONS`` column in the :table:`USER_STATISTICS` table was
 showing incorrect values. Bug fixed :bug:`728082`.

 |InnoDB| index dives did not detect some of the concurrent tree changes, which
 could return bogus estimates. Bug fixed :bug:`1625151` (upstream
 :mysqlbug:`84366`).

 :table:`INFORMATION_SCHEMA.INNODB_CHANGED_PAGES` queries would needlessly read
 potentially incomplete bitmap data past the needed LSN range. Bug fixed
 :bug:`1625466`.

 |Percona Server| ``cmake`` compiler would always attempt to build *RocksDB*
 even if ``-DWITHOUT_ROCKSDB=1`` argument was specified. Bug fixed
 :bug:`1638455`.

 Adding ``COMPRESSED`` attributes to |InnoDB| special tables fields (like
 ``mysql.innodb_index_stats`` and ``mysql.innodb_table_stats``) could lead
 to server crashes. Bug fixed :bug:`1640810`.

 Lack of free pages in the buffer pool is not diagnosed with
 :variable:`innodb_empty_free_list_algorithm` set to ``backoff`` (which is the
 default). Bug fixed :bug:`1657026`.

 ``mysqld_safe`` now limits the use of ``rm`` and ``chown`` to avoid privilege
 escalation. ``chown`` can now be used only for :file:`/var/log` directory. Bug
 fixed :bug:`1660265`. Thanks to Dawid Golunski (https://legalhackers.com).

 Renaming a |TokuDB| table to a non-existent database with
 :variable:`tokudb_dir_per_db` enabled would lead to a server crash. Bug fixed
 :tokubug:`1030`.

 :ref:`tokudb_read_free_replication` optimization could not be used for
 |TokuDB| partition tables. Bug fixed :tokubug:`1012`.

Other bugs fixed: :bug:`1486747` (upstream :mysqlbug:`76872`), :bug:`1633988`,
:bug:`1638198` (upstream :mysqlbug:`82823`), :bug:`1638897`, :bug:`1646384`,
:bug:`1647530`, :bug:`1647741`, :bug:`1651121`, :bug:`1156772`, :bug:`1644569`,
:bug:`1644583`, :bug:`1648389`, :bug:`1648737`, :bug:`1650247`, :bug:`1650256`,
:bug:`1650324`, :bug:`1650450`, :bug:`1655587`, and :bug:`1647723`.
