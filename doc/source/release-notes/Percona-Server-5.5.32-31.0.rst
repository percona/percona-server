.. rn:: 5.5.32-31.0

==============================
 |Percona Server| 5.5.32-31.0 
==============================

Percona is glad to announce the release of |Percona Server| 5.5.32-31.0 on July 2nd, 2013. Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.32-31.0/>`_ and from the :doc:`Percona Software Repositories </installation>`.

Based on `MySQL 5.5.32 <http://dev.mysql.com/doc/relnotes/mysql/5.5/en/news-5-5-32.html>`_, including all the bug fixes in it, |Percona Server| 5.5.32-31.0 is now the current stable release in the 5.5 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.5.32-31.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.32-31.0>`_. 

New Features
============

 |Percona Server| has implemented support for supplementary groups for :ref:`pam_plugin`.

Bugs Fixed
==========

 Prevented a race condition that could lead to a server crash when querying the :table:`INFORMATION_SCHEMA.INNODB_BUFFER_PAGE` table. Bug fixed :bug:`1072573`.

 |Percona Server| wouldn't start if the :ref:`changed_page_tracking` was enabled and variable :variable:`innodb_flush_method` was set to ``ALL_O_DIRECT``. Bug fixed :bug:`1131949`.

 Fixed the upstream bug :mysqlbug:`68970` that, in |Percona Server|, would cause small tablespaces to expand too fast around 500KB tablespace size. Bug fixed :bug:`1169494`.

 Query to the :table:`INNODB_CHANGED_PAGES` table would cause server to stop with an I/O error if a bitmap file in the middle of requested LSN range was missing. Bug fixed :bug:`1179974`.

 Server would crash if an :table:`INNODB_CHANGED_PAGES` query is issued that has an empty LSN range and thus does not need to read any bitmap files. Bug fixed :bug:`1184427`.

 Querying :table:`INFORMATION_SCHEMA.PARTITIONS` could cause key distribution statistics for partitioned tables to be reset to those corresponding to the last partition. Fixed the upstream bug :mysqlbug:`69179`. Bug fixed :bug:`1192354`.

 Incorrect schema definition for the :ref:`user_stats` tables in ``INFORMATION_SCHEMA`` (:table:`CLIENT_STATISTICS`, :table:`INDEX_STATISTICS`, :table:`TABLE_STATISTICS`, :table:`THREAD_STATISTICS`, and :table:`USER_STATISTICS`) led to the maximum counter values being limited to 32-bit signed integers. Fixed so that these values can be 64-bit unsigned integers now. Bug fixed :bug:`714925`.

 Fixed the upstream bug :mysqlbug:`42415` that would cause ``UPDATE/DELETE`` statements with the ``LIMIT`` clause to be unsafe for Statement Based Replication even when ``ORDER BY`` primary key was present. Fixed by implementing an algorithm to do more elaborate analysis on the nature of the query to determine whether the query will cause uncertainty for replication or not. Bug fixed :bug:`1132194`.

 When an upgrade was performed between major versions (e.g. by uninstalling a 5.1 RPM and then installing a 5.5 one), ``mysql_install_db`` was still called on the existing data directory which lead to re-creation of the ``test`` database. Bug fixed :bug:`1169522`.

 :ref:`changed_page_tracking` used to hold the log system mutex for the log reads needlessly, potentially limiting performance on write-intensive workloads. Bug fixed :bug:`1171699`.

 The RPM installer script had the :term:`datadir` hardcoded to :file:`/var/lib/mysql` instead of using ``my_print_defaults`` function to get the correct :term:`datadir` info. Bug fixed :bug:`1181753`.

 Missing path separator between the directory and file name components in a bitmap file name could stop the server starting if the :variable:`innodb_data_home_dir` variable didn't have the path separator at the end. Bug fixed :bug:`1181887`.

 Fixed the upstream bug :mysqlbug:`68354` that could cause server to crash when performing update or join on ``Federated`` and ``MyISAM`` tables with one row, due to a bug in the ``Federated`` storage engine. Bug fixed :bug:`1182572`.

 A warning is now returned if a bitmap file I/O error occurs after an :table:`INNODB_CHANGED_PAGES` query started returning data to indicate an incomplete result set. Bug fixed :bug:`1185040`.

 Under very rare circumstances, deleting a zero-size bitmap file at the right moment would make server stop with an I/O error if changed page tracking is enabled. Bug fixed :bug:`1184517`.
 
 Fixed the compiler warnings caused by :ref:`atomic_fio` when building |Percona Server| on non-Linux platforms. Bug fixed :bug:`1189429`.

 The :table:`INNODB_CHANGED_PAGES` table couldn't be queried if the log tracker wasn't running. Bug fixed :bug:`1185304`.

 Transaction objects are now allocated calling ``calloc()`` directly instead of using |InnoDB| heap allocation. This may improve write performance for high levels of concurrency. Bug fixed :bug:`1185686`.

Other bugs fixed: bug fixed :bug:`1099764`, bug fixed :bug:`1132412`, bug fixed :bug:`1191395`, bug fixed :bug:`1079688`, bug fixed :bug:`1132422`, bug fixed :bug:`1153651`, bug fixed :bug:`1160951`, bug fixed :bug:`1183583`, bug fixed :bug:`1133266`.
