.. rn:: 5.6.32-78.0

============================
|Percona Server| 5.6.32-78.0
============================

Percona is glad to announce the release of |Percona Server| 5.6.32-78.0 on
August 22th, 2016 (Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.32-78.0/>`_
and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.6.32
<http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-32.html>`_, including
all the bug fixes in it, |Percona Server| 5.6.32-78.0 is the current GA release
in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source
and free, all the details of the release can be found in the `5.6.32-78.0
milestone at Launchpad
<https://launchpad.net/percona-server/+milestone/5.6.32-78.0>`_.

New Features
============

 |Percona Server| :ref:`audit_log_plugin` now supports filtering by :ref:`user
 <filtering_by_user>` and :ref:`sql_command <filtering_by_sql_command_type>`.

 |Percona Server| now supports `tree map file block allocation strategy 
 <https://www.percona.com/blog/2016/08/17/improve-tokudbperconaft-fragmented-data-file-performance/>`_
 for |TokuDB|.

Bugs Fixed
==========

 Fixed potential cardinality ``0`` issue for |TokuDB| tables if ``ANALYZE
 TABLE`` finds only deleted rows and no actual logical rows before it times
 out. Bug fixed :bug:`1607300` (:tokubug:`1006`, :ftbug:`732`).

 |TokuDB| ``database.table.index`` names longer than 256 characters could cause
 server crash if :ref:`background analyze table
 <tokudb_background_analyze_table>` status was checked while running. Bug fixed
 :tokubug:`1005`.

 :ref:`pam_plugin` would abort authentication while checking UNIX user group
 membership if there were more than a thousand members. Bug fixed
 :bug:`1608902`.

 If ``DROP DATABASE`` would fail to delete some of the tables in the database,
 the partially-executed command is logged in the binlog as ``DROP TABLE t1, t2,
 ...``  for the tables for which drop succeeded. A slave might fail to
 replicate such ``DROP TABLE`` statement if there exist foreign key
 relationships to any of the dropped tables and the slave has a different
 schema from master. Fix by checking, on the master, whether any of the
 database to be dropped tables participate in a Foreign Key relationship, and
 fail the ``DROP DATABASE`` statement immediately. Bug fixed :bug:`1525407`
 (upstream :mysqlbug:`79610`).

 :ref:`pam_plugin` didn't support spaces in the UNIX user group names. Bug
 fixed :bug:`1544443`.

 Due to security reasons ``ld_preload`` libraries can now only be loaded from
 the system directories (:file:`/usr/lib64`, :file:`/usr/lib`) and the MySQL
 installation base directory.

 |Percona Server| 5.6 could not be built with the
 ``-DMYSQL_MAINTAINER_MODE=ON`` option. Bug fixed :bug:`1590454`.

 In the client library, any EINTR received during network I/O was not handled
 correctly. Bug fixed :bug:`1591202` (upstream :mysqlbug:`82019`).

 The included :file:`.gitignore` in the percona-server source distribution had
 a line ``*.spec``, which means someone trying to check in a copy of the
 percona-server source would be missing the spec file required to build the
 RPMs. Bug fixed :bug:`1600051`.

 :ref:`audit_log_plugin` did not transcode queries. Bug fixed :bug:`1602986`.

 LeakSanitizer-enabled build failed to bootstrap server for MTR. Bug fixed
 :bug:`1603978` (upstream :mysqlbug:`81674`).

 Fixed ``MYSQL_SERVER_PUBLIC_KEY`` connection option memory leak. Bug fixed
 :bug:`1604419`.

 The fix for bug :bug:`1341067` added a call to free some of the heap memory
 allocated by OpenSSL. This is not safe for repeated calls if OpenSSL is
 linked twice through different libraries and each is trying to free the same.
 Bug fixed :bug:`1604676`.

 If the changed page bitmap redo log tracking thread stops due to any reason,
 then shutdown will wait for a long time for the log tracker thread to quit,
 which it never does. Bug fixed :bug:`1606821`.

 :ref:`audit_log_plugin` events did not report the default database. Bug fixed
 :bug:`1435099`.

 Canceling the :ref:`tokudb_background_analyze_table` job twice or while it was
 in the queue could lead to server assertion. Bug fixed :tokubug:`1004`.

 Fixed various spelling errors in comments and function names. Bug fixed
 :ftbug:`728` (*Otto Kekäläinen*).

 Implemented set of fixes to make PerconaFT build and run on the AArch64
 (64-bit ARMv8) architecture. Bug fixed :ftbug:`726` (*Alexey Kopytov*).

Other bugs fixed: :bug:`1603073`, :bug:`1604323`, :bug:`1604364`,
:bug:`1604462`, :bug:`1604774`, :bug:`1606782`, :bug:`1607224`, :bug:`1607359`,
:bug:`1607606`, :bug:`1607607`, :bug:`1607671`, :bug:`1608385`, :bug:`1608437`,
:bug:`1608845`, :bug:`1609422`, :bug:`1610858`, :bug:`1612084`, :bug:`1612551`,
:bug:`1455430`, :bug:`1455432`, :bug:`1610242`, :tokubug:`998`,
:tokubug:`1003`, :ftbug:`729`, and :ftbug:`730`.
