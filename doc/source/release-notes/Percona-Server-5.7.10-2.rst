.. rn:: 5.7.10-2

=========================
|Percona Server| 5.7.10-2
=========================

Percona is glad to announce the second Release Candidate release of |Percona
Server| 5.7.10-2 on February 5th, 2016 (Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.7/Percona-Server-5.7.10-2rc2/>`_
and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.7.10
<http://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-10.html>`_, including
all the bug fixes in it, |Percona Server| 5.7.10-2 is the current Release
Candidate release in the |Percona Server| 5.7 series. All of Percona's
software is open-source and free, all the details of the release can be found
in the `5.7.10-2 milestone at Launchpad
<https://launchpad.net/percona-server/+milestone/5.7.10-2rc2>`_

New Features
============

 Complete list of changes between |Percona Server| 5.6 and 5.7 can be seen in
 :ref:`changed_in_57`.

 5.7 binlog group commit algorithm is now supported in TokuDB as well.

 New TokuDB index statistics reporting has been implemented to be compatible
 with the changes implemented in upstream 5.7. Following the InnoDB example,
 the default value for :variable:`tokudb_cardinality_scale_percent` has been
 changed from ``50%`` to ``100%``. Implementing this also addresses a server
 crash deep in the optimizer code.

Known Issues
============

 In |Percona Server| 5.7 `super_read_only
 <https://www.percona.com/doc/percona-server/5.6/management/super_read_only.html>`_
 feature has been replaced with upstream implementation. There are currently
 two known issues compared to |Percona Server| 5.6 implementation:

   * Bug :mysqlbug:`78963`, :variable:`super_read_only` aborts ``STOP SLAVE``
     if variable :variable:`relay_log_info_repository` is set to ``TABLE``
     which could lead to a server crash in Debug builds.

   * Bug :mysqlbug:`79328`, :variable:`super_read_only` set as a server option
     has no effect.

 InnoDB crash recovery might fail if :variable:`innodb_flush_method` is set
 to ``ALL_O_DIRECT``. The workaround is to set this variable to a different
 value before starting up the crashed instance (bug :bug:`1529885`).

Bugs Fixed
==========

 Clustering secondary index could not be created on a partitioned TokuDB
 table. Bug fixed :bug:`1527730` (:tokubug:`720`).

 :ref:`toku_backup` was failing to compile with |Percona Server| 5.7. Bug fixed
 :backupbug:`123`.

 Granting privileges to a user authenticating with :ref:`pam_plugin` could lead
 to a server crash. Bug fixed :bug:`1521474`.

 TokuDB status variables were missing from |Percona Server| :rn:`5.7.10-1`.
 Bug fixed :bug:`1527364` (:tokubug:`923`).

 Attempting to rotate the audit log file would result in audit log file name
 :file:`foo.log.%u` (literally) instead of a numeric suffix. Bug fixed
 :bug:`1528603`.

 Adding an index to an InnoDB temporary table while
 :variable:`expand_fast_index_creation` was enabled could lead to server
 assertion. Bug fixed :bug:`1529555`.

 TokuDB would not be upgraded on *Debian*/*Ubuntu* distributions while
 performing an upgrade from |Percona Server| 5.6 to |Percona Server| 5.7 even
 if explicitly requested. Bug fixed :bug:`1533580`.

 Server would assert when both TokuDB and InnoDB tables were used within
 one transaction on a replication slave which has binary log enabled and slave
 updates logging disabled. Bug fixed :bug:`1534249` (upstream bug
 :mysqlbug:`80053`).

 `MeCab Full-Text Parser Plugin
 <https://dev.mysql.com/doc/refman/5.7/en/fulltext-search-mecab.html>`_ has not
 been included in the previous release. Bug fixed :bug:`1534617`.

 Fixed server assertion caused by ``Performance Schema`` memory key mix-up in
 ``SET STATEMENT ... FOR ...`` statements. Bug fixed :bug:`1534874`.

 Service name on *CentOS* 6 has been renamed from ``mysqld`` back to ``mysql``.
 This change requires manual service restart after being upgraded from |Percona
 Server| :rn:`5.7.10-1`. Bug fixed :bug:`1542332`.

 Setting the :variable:`innodb_sched_priority_purge` (available only in debug
 builds) while purge threads were stopped would cause a server crash. Bug fixed
 :bug:`1368552`.

 Enabling TokuDB with ``ps_tokudb_admin`` script inside the Docker container
 would cause an error due to insufficient privileges even when running as root.
 In order for this script to be used inside docker containers this error has
 been changed to a warning that a check is impossible. Bug
 fixed :bug:`1520890`.

 Write-heavy workload with a small buffer pool could lead to a deadlock when
 free buffers are exhausted. Bug fixed :bug:`1521905`.

 InnoDB status will start printing negative values for spin rounds per wait,
 if the wait number, even though being accounted as a signed 64-bit integer,
 will not fit into a signed 32-bit integer. Bug fixed :bug:`1527160` (upstream
 :mysqlbug:`79703`).

 |Percona Server| 5.7 couldn't be restarted after TokuDB has been installed
 with ``ps_tokudb_admin`` script. Bug fixed :bug:`1527535`.

 Fixed memory leak when :variable:`utility_user` is enabled. Bug fixed
 :bug:`1530918`.

 Page cleaner worker threads were not instrumented for ``Performance Schema``.
 Bug fixed :bug:`1532747` (upstream bug :mysqlbug:`79894`).

 Busy server was preferring LRU flushing over flush list flushing too strongly
 which could lead to performance degradation. Bug fixed :bug:`1534114`.

 :file:`libjemalloc.so.1` was missing from binary tarball. Bug fixed
 :bug:`1537129`.

 When ``cmake/make/make_binary_distribution`` workflow was used to produce
 binary tarballs it would produce tarballs with ``mysql-...`` naming instead of
 ``percona-server-...``. Bug fixed :bug:`1540385`.

 Added proper memory cleanup if for some reason a table is unable to be opened
 from a dead closed state. This prevents an assertion from happening the next
 time the table is attempted to be opened. Bug fixed :tokubug:`917`.

 Variable :variable:`tokudb_support_xa` has been modified to prevent setting it
 to anything but ``ON``/``ENABLED`` and to print a SQL warning anytime an
 attempt is made to change it, just like :variable:`innodb_support_xa`. Bug
 fixed :tokubug:`928`.

Other bugs fixed: :bug:`1179451`, :bug:`1534246`, :bug:`1524763`,
:bug:`1525109` (upstream :mysqlbug:`79569`), :bug:`1530102`, :tokubug:`897`,
:tokubug:`898`, :tokubug:`899`, :tokubug:`900`, :tokubug:`901`, :tokubug:`902`,
:tokubug:`903`, :tokubug:`905`, :tokubug:`906`, :tokubug:`907`, :tokubug:`908`,
:tokubug:`909`, :tokubug:`910`, :tokubug:`911`, :tokubug:`912`, :tokubug:`913`,
:tokubug:`915`, :tokubug:`919`, and :tokubug:`904`.
