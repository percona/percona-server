.. rn:: 5.5.46-37.6

==============================
 |Percona Server| 5.5.46-37.5
==============================

Percona is glad to announce the release of |Percona Server| 5.5.46-37.5 on November 5th, 2015. Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.46-37.5/>`_ and from the :doc:`Percona Software Repositories </installation>`.

Based on `MySQL 5.5.46 <http://dev.mysql.com/doc/relnotes/mysql/5.5/en/news-5-5-46.html>`_, including all the bug fixes in it, |Percona Server| 5.5.46-37.5 is now the current stable release in the 5.5 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.5.46-37.5 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.46-37.5>`_. 

New Features
============

 |Percona Server| is now available for *Ubuntu* 15.10 (Wily).

Bugs Fixed
==========

 Manipulating the :variable:`innodb_track_redo_log_now` variable dynamically would crash the server if it was started without :variable:`innodb_track_changed_pages` enabled. This variable is available on debug builds only. Bug fixed :bug:`1368530`.

 A potential crash in handling corrupted tables with :variable:`innodb_corrupt_table_action` ``warn`` or ``salvage`` values has been fixed. Bug fixed :bug:`1426610`.

 If the user had duplicate pid-file options in config files when running ``yum upgrade``, the upgrade would stop with error because it would think it found the duplicate pid while it was the same pid specified twice.  Bug fixed :bug:`1454917`.

 On some filesystems server would not start if :ref:`changed_page_tracking` feature was enabled and :variable:`innodb_flush_method` variable was set to ``O_DIRECT``. Bugs fixed :bug:`1500720` and :bug:`1498891`.

 When :ref:`user_stats` are enabled, executing any statement of the ``SHOW`` family with non-empty result, would bump :table:`USER_STATISTICS` ``ROWS_FETCHED`` column values erroneously. Bug fixed :bug:`1510953`.

 Fixed the conflicting meta packages between 5.1, 5.5, and 5.6 release series in *Debian* and *Ubuntu* distributions. ``percona-server-server`` and ``percona-server-client`` meta packages now point to the latest 5.6 release. Bug fixed :bug:`1292517`.

 :table:`INNODB_CHANGED_PAGES` table was unavailable with non-default :variable:`innodb_data_home_dir` setting if the variable had a trailing slash. Bug fixed :bug:`1364315`.

 ``UPDATE`` statement could crash the server with :ref:`innodb_fake_changes_page` enabled. Bug fixed :bug:`1395706`. 

 Changing :variable:`innodb_fake_changes` variable value in the middle of a transaction would have an immediate effect, that is, making part of the transaction run with fake changes enabled and the rest with fake changes disabled, resulting in a broken transaction. Fixed by making any :variable:`innodb_fake_changes` value changes becoming effective at the start of the next transaction instead of the next statement. Bug fixed :bug:`1395579`.

 Startup would fail due to a small hard-coded timeout value in the init script for the pid file to appear. This has been fixed by creating default file for *Debian* init script timeout parameters in :file:`etc/default/mysql`. Bug fixed :bug:`1434022`.

 |Percona Server| would fail to install on *CentOS* 7 if ``mariadb-devel`` package was already installed. Bug fixed :bug:`1499721`.

 The upstream bug :mysqlbug:`76627` was not fixed for the ``ALL_O_DIRECT`` case. Bug fixed :bug:`1500741`.

Other bugs fixed: :bug:`1512301`, :bug:`1160960`, and :bug:`1497942`. 
