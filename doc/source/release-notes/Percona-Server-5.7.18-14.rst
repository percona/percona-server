.. rn:: 5.7.18-14

==========================
|Percona Server| 5.7.18-14
==========================

Percona is glad to announce the GA (Generally Available) release of |Percona
Server| 5.7.18-14 on May 12, 2017 (Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.7/Percona-Server-5.7.18-14/>`_
and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.7.18
<http://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-18.html>`_, including
all the bug fixes in it, |Percona Server| 5.7.18-14 is the current GA release
in the |Percona Server| 5.7 series. All of Percona's software is open-source
and free, all the details of the release can be found in the `5.7.18-14
milestone at
Launchpad <https://launchpad.net/percona-server/+milestone/5.7.18-14>`_

New Features
============

 |Percona Server| 5.7 packages are now available for Ubuntu 17.04 (*Zesty
 Zapus*).

 |Percona Server| now supports :ref:`prefix_index_queries_optimization`. This
 feature was ported from a Facebook MySQL patch.

 |Percona Server| has implemented support for :ref:`gap_locks_detection` for
 transactional storage engines, like *MyRocks*, that do not support gap locks.
 This feature was ported from a Facebook MySQL patch.

 :variable:`tokudb_dir_cmd` can now be used to :ref:`edit the TokuDB
 <editing_tokudb_files_with_tokudb_dir_cmd>` directory map. This feature is
 currently considered Experimental.

Bugs Fixed
==========

 Deadlock could occur in I/O-bound workloads when server was using several
 small buffer pool instances in combination with small redo log files and
 variable :variable:`innodb_empty_free_list_algorithm` set to ``backoff``
 algorithm. Bug fixed  :bug:`1651657`.

 Fixed a memory leak in :ref:`toku_backup`. Bug fixed :bug:`1669005`.

 :ref:`compressed_columns` could not be added to a partitioned table by using
 ``ALTER TABLE``. Bug fixed :bug:`1671492`.

 Fixed a memory leak that happened in case of failure to create
 a multi-threaded slave worker thread. Bug fixed :bug:`1675716`.

 In-Place upgrade from |Percona Server| 5.6 to 5.7 by using standalone packages
 would fail if :file:`/var/lib/mysql` wasn't defined as the
 :variable:`datadir`. Bug fixed :bug:`1687276`.

 Combination of using any audit API-using plugin, like :ref:`audit_log_plugin`
 and :ref:`response_time_distribution`, with multi-byte collation connection
 and ``PREPARE`` statement with a parse error could lead to a server crash. Bug
 fixed :bug:`1688698` (upstream :mysqlbug:`86209`).

 Fix for a :bug:`1433432` bug caused a performance regression due to suboptimal
 LRU manager thread flushing heuristics. Bug fixed :bug:`1631309`.

 Creating :ref:`compressed_columns` in MyISAM tables by specifying partition
 engines would not result in error. Bug fixed :bug:`1631954`.

 It was not possible to configure basedir as a symlink. Bug fixed
 :bug:`1639735`.

 Replication slave did not report ``Seconds_Behind_Master`` correctly when
 running in multi-threaded slave mode. Bug fixed :bug:`1654091`
 (upstream :mysqlbug:`84415`).

 ``DROP TEMPORARY TABLE`` would create a transaction in binary log on a
 read-only server. Bug fixed :bug:`1668602` (upstream :mysqlbug:`85258`).

 Processing GTIDs in the relay log that were already been executed were causing
 write/fsync amplification. Bug fixed :bug:`1669928` (upstream
 :mysqlbug:`85141`).

 Text/BLOB fields were not handling sorting of the empty string consistently
 between InnoDB and filesort. Bug fixed :bug:`1674867` (upstream
 :mysqlbug:`81810`) by porting a Facebook patch for MySQL.

 InnoDB adaptive hash index was using a partitioning algorithm which would
 produce uneven distribution when the server contained many tables with an
 identical schema. Bug fixed :bug:`1679155` (upstream :mysqlbug:`81814`).

 For plugin variables that are signed numbers, doing a ``SHOW VARIABLES`` would
 always show an unsigned number. Fixed by porting a Facebook patch for MySQL.

Other bugs fixed: :bug:`1629250` (upstream :mysqlbug:`83245`), :bug:`1660828`
(upstream :mysqlbug:`84786`), :bug:`1664519` (upstream :mysqlbug:`84940`),
:bug:`1674299`, :bug:`1670588` (upstream :mysqlbug:`84173`), :bug:`1672389`,
:bug:`1674507`, :bug:`1675623`, :bug:`1650294`, :bug:`1659224`, :bug:`1662908`,
:bug:`1669002`, :bug:`1671473`, :bug:`1673800`, :bug:`1674284`, :bug:`1676441`,
:bug:`1676705`, :bug:`1676847` (upstream :mysqlbug:`85671`), :bug:`1677130`
(upstream :mysqlbug:`85678`), :bug:`1677162`, :bug:`1677943`, :bug:`1678692`,
:bug:`1680510` (upstream :mysqlbug:`85838`), :bug:`1683993`, :bug:`1684012`,
:bug:`1684078`, :bug:`1684264`, :bug:`1687386`, :bug:`1687432`, :bug:`1687600`,
and :bug:`1674281`.
