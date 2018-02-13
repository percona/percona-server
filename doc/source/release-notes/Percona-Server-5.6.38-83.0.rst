.. rn:: 5.6.38-83.0

============================
|Percona Server| 5.6.38-83.0
============================

Percona is glad to announce the release of |Percona Server| 5.6.38-83.0 on
December 8, 2017 (Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.38-83.0/>`_
and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.6.38
<http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-38.html>`_, including
all the bug fixes in it, |Percona Server| 5.6.38-83.0 is the current GA release
in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source
and free, all the details of the release can be found in the `5.6.38-83.0
milestone at Launchpad
<https://launchpad.net/percona-server/+milestone/5.6.38-83.0>`_.

New Features
============

 |Percona Server| has implemented :ref:`innodb_fragmentation_count`.

 |Percona Server| has implemented support for :ref:`aio_page_requests`.
 This feature was ported from a *Facebook MySQL* patch.

 |Percona Server| has implemented |TokuDB| :ref:`integration
 <tokudb_performance_schema>` with ``PERFORMANCE_SCHEMA``.

 |Percona Server| packages are now available for *Ubuntu 17.10 (Artful)*.

Bugs Fixed
==========

 Dynamic row format feature to support ``BLOB/VARCHAR`` in ``MEMORY`` tables
 requires all the key columns to come before any ``BLOB`` columns. This
 requirement however was not enforced, allowing creating MEMORY tables in
 unsupported column configurations, which then crashed or lose data in usage.
 Bug fixed :bug:`1731483`.

 If an I/O syscall returned an error during the server shutdown with
 :ref:`threadpool` enabled, a mutex could be left locked. Bug fixed
 :bug:`1702330` (*Daniel Black*).

 After fixing bug :bug:`1668602`, bug :bug:`1539504`, and bug :bug:`1313901`,
 ``CREATE/DROP TEMPORARY TABLE`` statements were forbidden incorrectly in
 transactional contexts, including function and trigger calls, even when
 they required no binary logging at all. Bug fixed :bug:`1711781`.

 Running ``ANALYZE TABLE`` while a long-running query is accessing the same
 table in parallel could lead to a situation where new queries on the same
 table are blocked in a ``Waiting for table flush`` state.
 Fixed by stopping ``ANALYZE TABLE`` flushing affected |InnoDB| and |TokuDB|
 tables from the table definition cache. Bug fixed :bug:`1704195`
 (upstream :mysqlbug:`87065`).

 MyRocks additions to the 5.6 *mysqldump* output have `been removed
 <https://blueprints.launchpad.net/percona-server/+spec/remove-rocksdb-from-5.6-mysqldump>`_.

 ``CREATE TABLE ... LIKE ...`` did not use source ``row_format`` on target
 |TokuDB| table. Bug fixed :tdbbug:`76`.

 |TokuDB| would encode already encoded database name for a directory name. Bug
 fixed :tdbbug:`74`.

Other bugs fixed: :bug:`1670902` (upstream :mysqlbug:`85352`), :bug:`1670902`
(upstream :mysqlbug:`85352`), :bug:`1729241`, :tdbbug:`83`, :tdbbug:`80`, and
:tdbbug:`75`.
