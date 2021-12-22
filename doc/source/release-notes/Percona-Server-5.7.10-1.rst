.. rn:: 5.7.10-1

=========================
|Percona Server| 5.7.10-1
=========================

Percona is glad to announce the first Release Candidate release of |Percona
Server| 5.7.10-1 on December 14th, 2015 (Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.7/Percona-Server-5.7.10-1rc1/>`_
and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.7.10
<http://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-10.html>`_, including
all the bug fixes in it, |Percona Server| 5.7.10-1 is the current Release
Candidate release in the |Percona Server| 5.7 series. All of Percona's
software is open-source and free, all the details of the release can be found
in the `5.7.10-1 milestone at Launchpad
<https://launchpad.net/percona-server/+milestone/5.7.10-1rc1>`_

This release contains all the bug fixes from latest |Percona Server| 5.6
release (currently |Percona Server| `5.6.27-76.0
<http://www.percona.com/doc/percona-server/5.6/release-notes/Percona-Server-5.6.27-76.0.html>`_).

New Features
============

 |Percona Server| 5.7.10-1 is not available on *RHEL* 5 family of Linux
 distributions and *Debian* 6 (squeeze).

 Complete list of changes between |Percona Server| 5.6 and 5.7 can be seen in
 :ref:`changed_in_57`.

Known issues
============

 `MeCab Full-Text Parser Plugin
 <https://dev.mysql.com/doc/refman/5.7/en/fulltext-search-mecab.html>`_  has
 not been included in this release.

 :ref:`pam_plugin` currently isn't working correctly.

 Variables :variable:`innodb_show_verbose_locks` and
 :variable:`innodb_show_locks_held` are not working correctly.

 In |Percona Server| 5.7 `super_read_only
 <https://www.percona.com/doc/percona-server/5.6/management/super_read_only.html>`_
 feature has been replaced with upstream implementation. There are currently
 two known issues compared to |Percona Server| 5.6 implementation:

  * Bug :mysqlbug:`78963`, :variable:`super_read_only` aborts ``STOP SLAVE`` if
    variable :variable:`relay_log_info_repository` is set to ``TABLE`` which
    could lead to a server crash in Debug builds.

  * Bug :mysqlbug:`79328`, :variable:`super_read_only` set as a server option
    has no effect.

 Using primary key with a ``BLOB`` in TokuDB table could lead to a server
 crash (:tokubug:`916`).

 Using XA transactions with TokuDB could lead to a server crash
 (:tokubug:`900`).

 :ref:`toku_backup` has not been included in this release.

Bugs Fixed
==========

 Running ``ALTER TABLE`` without specifying the storage engine (without
 ``ENGINE=`` clause) or ``OPTIMIZE TABLE`` when
 :variable:`enforce_storage_engine` was enabled could lead to unrequested and
 unexpected storage engine changes. If done for a system table, it would
 circumvent regular system table storage engine compatibility checks,
 resulting in crashes or otherwise broken server operation. Bug fixed
 :bug:`1488055`.

 Some transaction deadlocks did not increase the
 :table:`INFORMATION_SCHEMA.INNODB_METRICS` ``lock_deadlocks`` counter. Bug
 fixed :bug:`1466414` (upstream :mysqlbug:`77399`).

 Removed excessive locking during the buffer pool resize when checking whether
 AHI is enabled. Bug fixed :bug:`1525215` (upstream :mysqlbug:`78894`).

 Removed unnecessary code in InnoDB error monitor thread. Bug fixed
 :bug:`1521564` (upstream :mysqlbug:`79477`).

 With :ref:`expanded_innodb_fast_index_creation` enabled, DDL queries involving
 InnoDB temporary tables would cause later queries on the same tables to
 produce warnings that their indexes were not found in the index translation
 table. Bug fixed :bug:`1233431`.

Other bugs fixed: :bug:`371752` (upstream :mysqlbug:`45379`), :bug:`1441362`
(upstream :mysqlbug:`56155`), :bug:`1385062` (upstream :mysqlbug:`74810`),
:bug:`1519201` (upstream :mysqlbug:`79391`), :bug:`1515602`, :bug:`1506697`
(upstream :mysqlbug:`57552`), :bug:`1501089` (upstream :mysqlbug:`75239`),
:bug:`1447527` (upstream :mysqlbug:`75368`), :bug:`1384658` (upstream
:mysqlbug:`74619`), :bug:`1384656` (upstream :mysqlbug:`74584`), and
:bug:`1192052`.
