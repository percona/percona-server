.. rn:: 5.6.27-75.0

==============================
 |Percona Server| 5.6.27-75.0 
==============================

Percona is glad to announce the release of |Percona Server| 5.6.27-75.0 on November 5th, 2015 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.27-75.0/>`_ and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.6.27 <http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-27.html>`_, including all the bug fixes in it, |Percona Server| 5.6.27-75.0 is the current GA release in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.6.27-75.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.6.27-75.0>`_.

New Features
============

 |Percona Server| is now available for *Ubuntu* 15.10 (Wily).

 |TokuDB| MTR tests have been integrated into |Percona Server|.
 
 Linux thread ID is now available in the :table:`PROCESSLIST` table. 

 |Percona Server| has now re-enabled savepoints in triggers and stored functions.

 Variables :variable:`innodb_buffer_pool_populate` and :variable:`numa_interleave` have been mapped to the upstream implementation of the new `innodb_numa_interleave <http://dev.mysql.com/doc/refman/5.6/en/innodb-parameters.html#sysvar_innodb_numa_interleave>`_ option.

Bugs Fixed
==========

 Fixed transactional inconsistency with rollback in |TokuDB|. Rolling back a large transaction with many inserts/updates/deletes could result in some of the changes being committed rather than rolled back. Bug fixed :bug:`1512455`.

 Variable :variable:`tokudb_backup_exclude` was not excluding files correctly. Bug fixed :bug:`1512457`.

 |TokuDB| could crash under load if transaction-isolation level ``READ-COMMITTED`` was used. Bug fixed :bug:`1501633`.

 |TokuDB| thread pool names were missing in the ``SHOW ENGINE tokudb STATUS`` which caused duplicate entries. Bug fixed :bug:`1512452`.

 Manipulating the :variable:`innodb_track_redo_log_now` variable dynamically would crash the server if it was started without :variable:`innodb_track_changed_pages` enabled. This variable is available on debug builds only. Bug fixed :bug:`1368530`.

 If the user had duplicate pid-file options in config files when running ``yum upgrade``, the upgrade would stop with error because it would think it found the duplicate pid while it was the same pid specified twice. Bug fixed :bug:`1454917`.

 On some filesystems server would not start if :ref:`changed_page_tracking` feature was enabled and :variable:`innodb_flush_method` variable was set to ``O_DIRECT``. Bugs fixed :bug:`1500720` and :bug:`1498891`.

 When :ref:`user_stats` are enabled, executing any statement of the ``SHOW`` family with non-empty result, would bump :table:`USER_STATISTICS` ``ROWS_FETCHED`` column values erroneously. Bug fixed :bug:`1510953`.

 A write operation with :variable:`innodb_fake_changes` enabled could cause a server assertion if it followed the pessimistic B-tree update path internally. Bug fixed :bug:`1192898`.

 An online DDL operation could have caused server crash with fake changes enabled. Bug fixed :bug:`1226532`.

 Fixed the conflicting meta packages between 5.1, 5.5, and 5.6 release series in *Debian* and *Ubuntu* distributions. ``percona-server-server`` and ``percona-server-client`` meta packages now point to the latest 5.6 release. Bug fixed :bug:`1292517`.

 :table:`INNODB_CHANGED_PAGES` table was unavailable with non-default :variable:`innodb_data_home_dir` setting if the variable had a trailing slash. Bug fixed :bug:`1364315`.

 Changing :variable:`innodb_fake_changes` variable value in the middle of a transaction would have an immediate effect, that is, making part of the transaction run with fake changes enabled and the rest with fake changes disabled, resulting in a broken transaction. Fixed by making any :variable:`innodb_fake_changes` value changes becoming effective at the start of the next transaction instead of the next statement. Bug fixed :bug:`1395579`.

 ``UPDATE`` statement could crash the server with :ref:`innodb_fake_changes_page` enabled. Bug fixed :bug:`1395706`. 

 Startup would fail due to a small hard-coded timeout value in the init script for the pid file to appear. This has been fixed by creating default file for *Debian* init script timeout parameters in :file:`etc/default/mysql`. Bug fixed :bug:`1434022`.

 :file:`CMakeLists.txt` for ``tokudb-backup-plugin`` was missing Valgrind dependency. Bug fixed :bug:`1494283`.

 |Percona Server| would fail to install on *CentOS* 7 if ``mariadb-devel`` package was already installed. Bug fixed :bug:`1499721`.

 Fixed suboptimal :ref:`innodb_fake_changes_page` handling in online ``ALTER`` storage engine API. Bug fixed :bug:`1204422`.

 The upstream bug :mysqlbug:`76627` was not fixed for the ``ALL_O_DIRECT`` case. Bug fixed :bug:`1500741`.

 Fixed multiple |TokuDB| clang build issues. Bug fixed :bug:`1512449`.

Other bugs fixed: :bug:`1204443`, :bug:`1384632`, :bug:`1475117`, :bug:`1512301`, :bug:`1452397`, :bug:`1160960`, :bug:`1495965`, and :bug:`1497942`.
